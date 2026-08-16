#include <math.h>
#include <Bridge.h>
#include <Logger.h>
#include <enums.h>
#include <NetworkValue.h>
#include <modbus/Modbus.h>
#include <modbus/ModbusTypes.h>
#include <modbus/ModbusTCP.h>
#include "PressureProfile.h"
#include "config-modbus.h"
#include "config.h"
#include "PHApp.h"
#include <ArduinoLog.h>

#ifdef ENABLE_PROFILE_PRESSURE

const char *PressureProfile::MODBUS_PREFIX = "PProf";

PressureProfile::PressureProfile(Component *owner, short slot, ushort componentId, ushort modbusBaseAddress)
    : PlotBase(owner, componentId, modbusBaseAddress, PlotType::Pressure),
      _previousStatus(PlotStatus::IDLE),
      _lastLoopExecutionMs(0),
      _lastLogMs(0),
      _targetRegisters(PRESSURE_PROFILE_MAX_TARGET_REGS, 0)
{
    setSlot(slot);
    name = "PressureProfile_" + String(this->id) + "_Slot_" + String(slot);
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    max = 100; // Pressure profiles always use 100% scale for PressCylinder SP
}

short PressureProfile::setup()
{
    PlotBase::setup();
    return E_OK;
}

short PressureProfile::loop()
{
    PlotBase::loop();
    uint32_t now = millis();

    PlotStatus currentStatus = getCurrentStatus();
    if (currentStatus == PlotStatus::IDLE && _previousStatus == PlotStatus::IDLE)
    {
        return E_OK;
    }

    m_status.update(currentStatus);

    if (currentStatus == PlotStatus::RUNNING)
    {
        m_duration.update(getDuration() / 1000);
        m_remaining.update(getRemainingTime() / 1000);
    }

    auto applyPressure = [&](uint16_t pressureValue)
    {
        if (currentStatus != PlotStatus::RUNNING)
        {
            return;
        }

        if (modbusTCP == nullptr || modbusTCP->modbusServer == nullptr)
        {
            L_WARN("PressureProfile: Apply pressure failed - Modbus not initialized");
            return;
        }

        for (uint16_t targetRegAddr : _targetRegisters)
        {
            if (targetRegAddr == 0 || pressureValue == 0)
                continue;
            ModbusMessage resp;
            ModbusMessage req;
            Error err = SUCCESS;
            req.setMessage(static_cast<uint8_t>(1), static_cast<uint8_t>(E_FN_CODE::FN_WRITE_HOLD_REGISTER), targetRegAddr, pressureValue);
            resp = modbusTCP->modbusServer->localRequest(req);
            if ((err = resp.getError()) != SUCCESS)
            {
                Log.errorln("Error writing pressure %d to slave %d, register %d: %d", pressureValue, slaveId, targetRegAddr, err);
            }
        }
    };

    auto resetOutputs = [&]()
    {
        m_currentValue.update(0);
        m_elapsed.update(0);
        applyPressure(0);
    };

    auto holdCurrentPressure = [&]()
    {
        // For paused/finished states: maintain current pressure, only reset display values
        m_currentValue.update(getValue(getElapsedMs()));
        // Don't call applyPressure(0) - keep current setpoint
    };

    auto handleRunning = [&]()
    {
        uint32_t currentElapsedMs = getElapsedMs();
        m_elapsed.update(currentElapsedMs / 1000);

        if (now - _lastLoopExecutionMs < PRESSURE_PROFILE_LOOP_INTERVAL_MS)
        {
            return;
        }
        _lastLoopExecutionMs = now;

        int16_t currentPressure = getValue(getElapsedMs());
        if (currentPressure >= 0)
        {
            m_currentValue.update(currentPressure, E_PRIORITY::E_PRIORITY_MEDIUM);
            applyPressure(static_cast<uint16_t>(currentPressure));
        }
    };

    // --- State Machine ---
    switch (currentStatus)
    {
    case PlotStatus::INITIALIZING:
        // resetOutputs();
        break;

    case PlotStatus::RUNNING:
        handleRunning();
        break;

    case PlotStatus::IDLE:
        m_currentValue.update(0);
        m_elapsed.update(0);
        // resetOutputs(); // Avoid writing 0 to SP when IDLE to allow manual control
        break;

    case PlotStatus::PAUSED:
        holdCurrentPressure();
        break;

    case PlotStatus::FINISHED:
        if (_previousStatus != PlotStatus::FINISHED)
        {
            onFinished();
        }
        holdCurrentPressure();
        break;

    case PlotStatus::STOPPED:
        if (_previousStatus != PlotStatus::STOPPED)
        {
            onStop();
        }
        resetOutputs();
        break;
    }

    if (currentStatus != _previousStatus)
    {
        _previousStatus = currentStatus;
    }

    return E_OK;
}

void PressureProfile::sample()
{
    _initializeControlPoints();
    stop();
    _durationMs = 1740000; // Matches the duration in the JSON file (29 minutes)

    // Point 0
    _controlPoints[0].x = 0;
    _controlPoints[0].y = 700;

    // Point 1
    _controlPoints[1].x = 590;
    _controlPoints[1].y = 1000;

    // Point 2
    _controlPoints[2].x = 1000;
    _controlPoints[2].y = 1000;

    // Point 3
    _controlPoints[3].x = 1000;
    _controlPoints[3].y = 0;

    _numControlPoints = 4;
    max = 100;
}
bool PressureProfile::load(const JsonObject &config)
{
    PlotBase::load(config);
    _initializeControlPoints();
    JsonArray pointsArray = config["controlPoints"].as<JsonArray>();
    _numControlPoints = min((uint8_t)pointsArray.size(), (uint8_t)MAX_CONTROL_POINTS);
    if (_numControlPoints < 2)
    {
        Log.warningln("%s: Not enough control points (found %d). Using default sample profile.", name.c_str(), _numControlPoints);
        sample();
    }
    else
    {
        for (uint8_t i = 0; i < _numControlPoints; ++i)
        {
            JsonObject pointObj = pointsArray[i].as<JsonObject>();
            uint16_t x_from_json = pointObj["x"].as<uint16_t>();
            uint16_t y_scaled = pointObj["y"].as<uint16_t>();
            _controlPoints[i].x = x_from_json;
            _controlPoints[i].y = y_scaled;
        }
    }

    // --- Load Target Registers (PID Affinity) ---
    clearTargetRegisters();
    if (config["targetRegisters"].is<JsonArray>())
    {
        JsonArray targetArray = config["targetRegisters"].as<JsonArray>();
        uint8_t numTargets = min((uint8_t)targetArray.size(), (uint8_t)PRESSURE_PROFILE_MAX_TARGET_REGS);
        if (numTargets > 0)
        {
            for (uint8_t i = 0; i < numTargets; ++i)
            {
                uint16_t targetReg = targetArray[i].as<uint16_t>();
                if (targetReg > 0)
                {
                    setTargetRegister(i, targetReg);
                }
            }
        }
        if (targetArray.size() > PRESSURE_PROFILE_MAX_TARGET_REGS)
        {
            Log.warningln("%s: Config specified %d targets, but limited to %d.", name.c_str(), targetArray.size(), PRESSURE_PROFILE_MAX_TARGET_REGS);
        }
    }

    if (config["enabled"].is<bool>())
    {
        enable(config["enabled"].as<bool>());
    }

    // Pressure profiles must always use max=100 for proper percentage scaling
    // Override any max value from JSON configuration
    max = 100;

    return true;
}

short PressureProfile::status()
{
    uint32_t duration = getDuration();
    uint32_t remaining = getRemainingTime();
    PlotStatus status = getCurrentStatus();
    int16_t pressure = getValue(getElapsedMs());

    Log.noticeln("  Status: %d (%s)", (int)status,
                 status == PlotStatus::IDLE ? "IDLE" : status == PlotStatus::RUNNING ? "RUNNING"
                                                   : status == PlotStatus::PAUSED    ? "PAUSED"
                                                                                     : "FINISHED");
    Log.noticeln("  Duration: %lu ms", duration);
    Log.noticeln("  Elapsed: %lu ms", duration - remaining);
    Log.noticeln("  Remaining: %lu ms", remaining);
    Log.noticeln("  Current Pressure (scaled): %d", pressure);
    return E_OK;
}

const char *PressureProfile::getOwnPrefix() const
{
    return PressureProfile::MODBUS_PREFIX;
}

#endif // ENABLE_PROFILE_PRESSURE
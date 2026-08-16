#include <math.h>
#include <Bridge.h>
#include <Logger.h>
#include <enums.h>
#include <NetworkValue.h>
#include <modbus/Modbus.h>
#include <modbus/ModbusTypes.h>
#include <modbus/ModbusTCP.h>
#include <modbus/Modbus.h>
#include "TemperatureProfile.h"
#include "config-modbus.h"
#include "config.h"
#include "PHApp.h"
#include <ArduinoLog.h>
#include <components/OmronE5.h>

#ifdef ENABLE_PROFILE_TEMPERATURE

const char *TemperatureProfile::MODBUS_PREFIX = "TProf";

TemperatureProfile::TemperatureProfile(Component *owner, short slot, ushort componentId, ushort modbusBaseAddress)
    : PlotBase(owner, componentId, modbusBaseAddress, PlotType::Temperature),
      slaveId(1), // Default slaveId to 1
      _lastLoopExecutionMs(0),
      _lastLogMs(0),
      _targetRegisters(TEMP_PROFILE_MAX_TARGET_REGS, 0),
      _signalPlotSlotId(-1),
      _pressureProfileSlotId(-1)

{
    setSlot(slot);
    name = "TempProfile_" + String(this->id) + "_Slot_" + String(slot);
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
}

short TemperatureProfile::setup()
{
    return PlotBase::setup();
}

short TemperatureProfile::loop()
{
    PlotBase::loop();
    uint32_t now = millis();

    PlotStatus currentStatus = getCurrentStatus();
    if (currentStatus == PlotStatus::IDLE && _previousStatus == PlotStatus::IDLE)
    {
        return E_OK;
    }
    bool currentEnabledState = this->enabled();

    auto applyTemperature = [&](uint16_t tempValue)
    {
        if (modbusTCP == nullptr || modbusTCP->modbusServer == nullptr)
        {
            L_WARN("TemperatureProfile: Apply temperature failed - Modbus not initialized");
            return;
        }

        for (uint16_t targetRegAddr : _targetRegisters)
        {
            if (targetRegAddr == 0)
                continue;

            int16_t offset = getTargetOffset(targetRegAddr);
            int16_t finalTemp = (int16_t)tempValue + offset;
            ModbusMessage resp;
            ModbusMessage req;
            Error err = SUCCESS;
            req.setMessage(static_cast<uint8_t>(1), static_cast<uint8_t>(E_FN_CODE::FN_WRITE_HOLD_REGISTER), targetRegAddr, (uint16_t)finalTemp);
            resp = modbusTCP->modbusServer->localRequest(req);
            if ((err = resp.getError()) != SUCCESS)
            {
                L_ERROR("Error writing temperature %d (base %d + off %d) to slave %d, register %d: %d", finalTemp, tempValue, offset, slaveId, targetRegAddr, err);
            }
        }
    };

    auto handleRunning = [&]()
    {
        // --- Lag Compensation ---
        if (now - _lastLoopExecutionMs < TEMPERATURE_PROFILE_LOOP_INTERVAL_MS)
        {
            return;
        }
        _lastLoopExecutionMs = now;
        int16_t currentTemp = getValue(getElapsedMs());
        if (currentTemp >= 0)
        {
            applyTemperature(static_cast<uint16_t>(currentTemp));
        }
    };

    // Check if initializing to reset cache if needed, or do it on start.
    // Ideally we clear _omron on stop/start to handle config changes, but it's unlikely to change at runtime.

    // --- State Machine ---
    switch (currentStatus)
    {
    case PlotStatus::INITIALIZING:
        // Do nothing, wait for state to change
        break;

    case PlotStatus::RUNNING:
        handleRunning();
        break;

    case PlotStatus::IDLE:
    case PlotStatus::PAUSED:
        break;

    case PlotStatus::FINISHED:
        if (_previousStatus != PlotStatus::FINISHED)
        {
            onFinished();
        }
        break;

    case PlotStatus::STOPPED:
        if (_previousStatus != PlotStatus::STOPPED)
        {
            onStop();
        }
        break;
    }

    if (currentStatus != _previousStatus)
    {
        _previousStatus = currentStatus;
    }
    return E_OK;
}

void TemperatureProfile::sample()
{
    _initializeControlPoints();
    _durationMs = 1000 * 10 * 20;
    // Point 0: Start at 20% (scaled) temp at time 0
    _controlPoints[0].x = 0;   // 0% time
    _controlPoints[0].y = 200; // 20% of PROFILE_SCALE

    // Point 1: Ramp up linearly to 80% temp by 30% time (18 seconds)
    _controlPoints[1].x = 500; // 30% time (scaled)
    _controlPoints[1].y = 800; // 80% temp (scaled)

    // Point 2: Hold at 80% temp until 70% time (42 seconds)
    _controlPoints[2].x = 700; // 70% time (scaled)
    _controlPoints[2].y = 400; // 80% temp (scaled) - same as previous

    // Point 3: Ramp down linearly to 30% temp by 90% time (54 seconds)
    _controlPoints[3].x = 900; // 90% time (scaled)
    _controlPoints[3].y = 300; // 30% temp (scaled)

    // Point 4: End at 25% temp at 100% time (60 seconds)
    _controlPoints[4].x = 1000; // 100% time (Use macro)
    _controlPoints[4].y = 200;  // 25% temp (scaled)
    _numControlPoints = 5;
    max = 150;
}
bool TemperatureProfile::load(const JsonObject &config)
{
    PlotBase::load(config);
    _initializeControlPoints();
    JsonArray pointsArray = config["controlPoints"].as<JsonArray>();
    _numControlPoints = min((uint8_t)pointsArray.size(), (uint8_t)MAX_CONTROL_POINTS);
    if (_numControlPoints < 2)
    {
        Log.warningln("%s: Load failed - Not enough control points (found %d, need at least 2). Using empty profile.", name.c_str(), _numControlPoints);
        _numControlPoints = 0; // Need at least 2 points for interpolation
        return false;          // Consider it a failure if not enough points
    }
    for (uint8_t i = 0; i < _numControlPoints; ++i)
    {
        JsonObject pointObj = pointsArray[i].as<JsonObject>();
        int16_t x_from_json = pointObj["x"].as<int16_t>();
        int16_t y_scaled = pointObj["y"].as<int16_t>();
        _controlPoints[i].x = x_from_json;
        _controlPoints[i].y = y_scaled;
    }

    // --- Load Target Registers (PID Affinity) ---
    clearTargetRegisters();
    if (config["targetRegisters"].is<JsonArray>())
    {
        JsonArray targetArray = config["targetRegisters"].as<JsonArray>();
        uint8_t numTargets = min((uint8_t)targetArray.size(), (uint8_t)TEMP_PROFILE_MAX_TARGET_REGS);
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
        if (targetArray.size() > TEMP_PROFILE_MAX_TARGET_REGS)
        {
            Log.warningln("%s: Config specified %d targets, but limited to %d.", name.c_str(), targetArray.size(), TEMP_PROFILE_MAX_TARGET_REGS);
        }
    }

    if (config["signalPlot"].is<int>())
    {
        _signalPlotSlotId = config["signalPlot"].as<short>();
    }
    else
    {
        _signalPlotSlotId = -1;
    }

    if (config["pressureProfile"].is<int>())
    {
        _pressureProfileSlotId = config["pressureProfile"].as<short>();
    }
    else
    {
        _pressureProfileSlotId = -1;
    }

    if (config["enabled"].is<bool>())
    {
        enable(config["enabled"].as<bool>());
    }

    // --- Load Overrides ---
    clearTargetOffsets();
    if (config["overrides"].is<JsonObject>())
    {
        JsonObject overrides = config["overrides"].as<JsonObject>();
        if (overrides["sp"].is<JsonArray>())
        {
            JsonArray spOverrides = overrides["sp"].as<JsonArray>();
            for (JsonVariant v : spOverrides)
            {
                if (v.is<JsonObject>())
                {
                    JsonObject o = v.as<JsonObject>();
                    if (o["targetRegister"].is<uint16_t>() && o["offset"].is<int16_t>())
                    {
                        setTargetOffset(o["targetRegister"].as<uint16_t>(), o["offset"].as<int16_t>());
                    }
                }
            }
        }
    }

    return true;
}
short TemperatureProfile::info()
{
    uint32_t duration = getDuration();
    uint32_t remaining = getRemainingTime();
    PlotStatus status = getCurrentStatus();
    int16_t temp = getValue(getElapsedMs());

    Log.noticeln("  Status: %d (%s)", (int)status,
                 status == PlotStatus::IDLE ? "IDLE" : status == PlotStatus::INITIALIZING ? "INITIALIZING"
                                                   : status == PlotStatus::RUNNING        ? "RUNNING"
                                                   : status == PlotStatus::PAUSED         ? "PAUSED"
                                                                                          : "FINISHED");
    Log.noticeln("  Duration: %lu ms", duration);
    Log.noticeln("  Elapsed: %lu ms", duration - remaining);
    Log.noticeln("  Remaining: %lu ms", remaining);
    Log.noticeln("  Current Temp (scaled): %d", temp);
    return E_OK;
}

const char *TemperatureProfile::getOwnPrefix() const
{
    return "TProf";
}

void TemperatureProfile::setTargetOffset(uint16_t reg, int16_t offset)
{
    for (auto &to : _targetOffsets)
    {
        if (to.targetReg == reg)
        {
            to.offset = offset;
            return;
        }
    }
    _targetOffsets.push_back({reg, offset});
}

int16_t TemperatureProfile::getTargetOffset(uint16_t reg) const
{
    for (const auto &to : _targetOffsets)
    {
        if (to.targetReg == reg)
        {
            return to.offset;
        }
    }
    return 0;
}

void TemperatureProfile::clearTargetOffsets()
{
    _targetOffsets.clear();
#ifdef NUM_OMRON_DEVICES
    _targetOffsets.reserve(NUM_OMRON_DEVICES);
#endif
}

void TemperatureProfile::resolveLinkedProfiles()
{
    PHApp *app = (PHApp *)owner;
    if (!app)
    {
        return;
    }

#ifdef ENABLE_PROFILE_PRESSURE
    if (_pressureProfileSlotId >= 0 && _pressureProfileSlotId < PROFILE_PRESSURE_COUNT)
    {
        PressureProfile *p = app->pressureProfiles[_pressureProfileSlotId];
        if (p)
        {
            addPlot(p);
        }
    }
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    if (_signalPlotSlotId >= 0 && _signalPlotSlotId < PROFILE_SIGNAL_PLOT_COUNT)
    {
        SignalPlot *s = app->signalPlots[_signalPlotSlotId];
        if (s)
        {
            addPlot(s);
        }
    }
#endif
}

void TemperatureProfile::onStart()
{
    resolveLinkedProfiles();
    PlotBase::onStart();
}

#endif // ENABLE_PROFILE_TEMPERATURE

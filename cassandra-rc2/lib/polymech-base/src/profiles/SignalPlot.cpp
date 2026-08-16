#include "SignalPlot.h"

#include <Logger.h>         // Added for logging
#include <macros.h>         // Added for LOW_WORD/HIGH_WORD
#include "config-modbus.h"  // Added for MB_HREG_SIGNAL_PLOT_BASE
#include <enums.h>          // Added for E_OK, etc.
#include <modbus/Modbus.h>  // Added for ModbusError
#include "Arduino.h"        // For pinMode, digitalWrite, analogWrite
#include "CommandMessage.h" // For CommandMessage
#include "Bridge.h"         // For Bridge component
#include "PHApp.h"          // For PHApp component
#include <ArduinoLog.h>

#ifdef ENABLE_FEEDBACK_BUZZER
#include <components/FeedbackBuzzer.h>
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT

SignalPlot::SignalPlot(Component *owner, ushort slot, ushort componentId, ushort modbusAddress)
    : PlotBase(owner, componentId, modbusAddress, PlotType::Signal),
      _numControlPoints(0)
{
    setSlot(slot);
    name = "SignalPlot_" + String(this->id) + "_Slot_" + String(slot);
    for (int i = 0; i < MAX_SIGNAL_POINTS; ++i)
    {
        _controlPoints[i] = {};
    }
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    disable(); // Disable by default
}

short SignalPlot::setup()
{
    PlotBase::setup();
    return E_OK;
}

short SignalPlot::loop()
{
    PlotBase::loop();

    // Update all relevant NetworkValues from the base class
    PlotStatus currentStatus = getCurrentStatus();
    m_status.update(currentStatus);
    m_duration.update(getDuration() / 1000);       // Assuming duration is in seconds for modbus
    m_remaining.update(getRemainingTime() / 1000); // Same for remaining

    uint32_t currentElapsedMs = getElapsedMs();
    m_elapsed.update(currentElapsedMs / 1000); // Same for elapsed

    // Since SignalPlot doesn't have a value curve, we'll just update with 0
    m_currentValue.update(0);

    if (!isRunning() || !enabled())
    {
        return E_OK;
    }

    for (uint8_t i = 0; i < _numControlPoints; ++i)
    {
        S_SignalControlPoint &cp = _controlPoints[i];
        uint32_t absoluteTriggerMs = static_cast<uint32_t>(((uint64_t)cp.time * (uint64_t)getDuration()) / 1000);
        if (cp.state == E_SIGNAL_STATE::STATE_NONE && currentElapsedMs >= absoluteTriggerMs)
        {
            executeControlPointAction(i);
        }
    }
    return E_OK;
}

bool SignalPlot::load(const JsonObject &config)
{
    PlotBase::load(config);
    _numControlPoints = 0; // Reset count before loading new points
    for (int i = 0; i < MAX_SIGNAL_POINTS; ++i)
    {
        _controlPoints[i] = {}; // Clear existing points
    }

    if (!config["controlPoints"].is<JsonArray>())
    {
        _numControlPoints = 0;
        return true; // No points to load is considered a failure for a plot that needs points.
    }

    JsonArray pointsArray = config["controlPoints"].as<JsonArray>();

    _numControlPoints = min((uint8_t)pointsArray.size(), (uint8_t)MAX_SIGNAL_POINTS);
    if (_numControlPoints < 1)
    {
        _numControlPoints = 0;
        return true;
    }

    for (uint8_t i = 0; i < _numControlPoints; ++i)
    {
        JsonObject pointObj = pointsArray[i].as<JsonObject>();

        _controlPoints[i].id = pointObj["id"].as<uint8_t>();
        _controlPoints[i].time = pointObj["time"].as<uint32_t>();
        _controlPoints[i].name = pointObj["name"].as<String>();
        _controlPoints[i].description = pointObj["description"].as<String>();
        _controlPoints[i].type = static_cast<E_SIGNAL_TYPE>(pointObj["type"].as<uint8_t>());
        _controlPoints[i].arg_0 = pointObj["arg_0"].as<uint16_t>();
        if (pointObj["arg_1"].is<int>())
        {
            _controlPoints[i].arg_1 = pointObj["arg_1"].as<uint16_t>();
        }
        else
        {
            _controlPoints[i].arg_1 = 0;
            // Log.warningln("%s: CP id %d, type %d - 'arg_1' not found in JSON. Defaulting to 0.",
            //               name.c_str(), _controlPoints[i].id, static_cast<int>(_controlPoints[i].type));
        }

        if (pointObj["arg_2"].is<uint16_t>())
        {
            _controlPoints[i].arg_2 = pointObj["arg_2"].as<uint16_t>();
        }
        else
        {
            _controlPoints[i].arg_2 = 0;
        }

        if (!pointObj["user"].isNull())
        {
            // ... user field handling ...
        }

        if (pointObj["state"].is<uint8_t>())
        {
            _controlPoints[i].state = static_cast<E_SIGNAL_STATE>(pointObj["state"].as<uint8_t>());
        }
        else
        {
            _controlPoints[i].state = E_SIGNAL_STATE::STATE_NONE;
        }
    }
    return true;
}

const S_SignalControlPoint *SignalPlot::findActivePoint(uint32_t elapsedMs) const
{
    const S_SignalControlPoint *lastApplicablePoint = nullptr;
    // Iterate backwards assuming points are sorted by time ascending
    // This is more efficient as we likely hit the correct segment sooner.
    for (int i = _numControlPoints - 1; i >= 0; --i)
    {
        uint32_t pointTimeMs = static_cast<uint32_t>(((uint64_t)_controlPoints[i].time * (uint64_t)getDuration()) / 1000);
        if (pointTimeMs <= elapsedMs)
        {
            // Found the latest point at or before the elapsed time
            lastApplicablePoint = &_controlPoints[i];
            break; // Since sorted, this is the correct one
        }
    }
    return lastApplicablePoint;
}

E_SIGNAL_STATE SignalPlot::getState(E_SIGNAL_STATE defaultState) const
{
    if (!_running || _numControlPoints == 0)
    {
        return defaultState;
    }

    uint32_t elapsedMs = getElapsedMs();
    const S_SignalControlPoint *activePoint = findActivePoint(elapsedMs);

    if (activePoint != nullptr)
    {
        return activePoint->state;
    }
    else
    {
        // No point defined at or before the current time
        return defaultState;
    }
}

int16_t SignalPlot::getUserValue(int16_t defaultValue) const
{
    if (!_running || _numControlPoints == 0)
    {
        return defaultValue;
    }

    uint32_t elapsedMs = getElapsedMs();
    const S_SignalControlPoint *activePoint = findActivePoint(elapsedMs);

    if (activePoint != nullptr)
    {
        return *((int16_t *)activePoint->user);
    }
    else
    {
        // No point defined at or before the current time
        return defaultValue;
    }
}

bool SignalPlot::getCurrentControlPointInfo(uint8_t &outId, uint32_t &outTimeMs, int16_t &outValue, int16_t &outUser) const
{
    if (!_running || _numControlPoints == 0)
    {
        return false;
    }

    uint32_t elapsedMs = getElapsedMs();
    const S_SignalControlPoint *activePoint = findActivePoint(elapsedMs);

    if (activePoint != nullptr)
    {
        outId = activePoint->id;
        outTimeMs = static_cast<uint32_t>(((uint64_t)activePoint->time * (uint64_t)getDuration()) / 1000);
        outValue = static_cast<int16_t>(activePoint->state); // Cast enum state to int16_t
        outUser = *((int16_t *)activePoint->user);
        return true;
    }
    else
    {
        // No point defined at or before the current time
        return false;
    }
}

short SignalPlot::start()
{
    PlotBase::start();
    for (uint8_t i = 0; i < _numControlPoints; ++i)
    {
        _controlPoints[i].state = E_SIGNAL_STATE::STATE_NONE;
    }
    return E_OK;
}

void SignalPlot::executeControlPointAction(uint8_t cpIndex)
{
    if (cpIndex >= _numControlPoints)
    {
        return;
    }
    S_SignalControlPoint &cp = _controlPoints[cpIndex];

    if (cp.state == E_SIGNAL_STATE::STATE_ON)
    {
        return; // Already executed, skip
    }

    switch (cp.type)
    {
    case E_SIGNAL_TYPE::MB_WRITE_COIL:
    {
        if (modbusTCP == nullptr || modbusTCP->modbusServer == nullptr)
        {
            L_ERROR("SignalPlot::executeControlPointAction: modbusTCP or modbusServer is nullptr");
            return;
        }
        uint16_t coilAddress = static_cast<uint16_t>(cp.arg_0);
        uint16_t coilValue = (cp.arg_1 != 0) ? 0xFF00 : 0x0000; // 0xFF00 for ON, 0x0000 for OFF
        ModbusMessage req;
        // Using MB_GATEWAY_ESP_DEFAULT_SLAVE_ID for local requests to the ESP's own server instance.
        // Adjust if target is an external slave.
        req.setMessage(1, static_cast<uint8_t>(E_FN_CODE::FN_WRITE_COIL), coilAddress, coilValue);
        ModbusMessage resp = modbusTCP->modbusServer->localRequest(req);
        cp.state = E_SIGNAL_STATE::STATE_ON;
        break;
    }
    case E_SIGNAL_TYPE::GPIO_WRITE:
    {
        uint8_t pin = static_cast<uint8_t>(cp.arg_0);
        E_GpioWriteMode gpioMode = static_cast<E_GpioWriteMode>(cp.arg_1);
        int16_t value = cp.arg_2;
        pinMode(pin, OUTPUT);
        if (gpioMode == E_GpioWriteMode::DIGITAL)
        {
            digitalWrite(pin, (value != 0) ? HIGH : LOW);
            cp.state = E_SIGNAL_STATE::STATE_ON;
        }
        else if (gpioMode == E_GpioWriteMode::ANALOG_PWM)
        {
            if (value < 0)
                value = 0;
            if (value > 255)
                value = 255;
            analogWrite(pin, value);
            cp.state = E_SIGNAL_STATE::STATE_ON;
        }
        else
        {
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
        }
        break;
    }
    case E_SIGNAL_TYPE::CALL_METHOD:
    {
        if (!cp.user)
        {
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            break;
        }

        String cmdStr = String((char *)cp.user);

        Bridge *bridge = static_cast<Bridge *>(owner->byId(COMPONENT_KEY_MB_BRIDGE));
        if (!bridge)
        {
            return;
        }

        CommandMessage msg;
        if (!msg.parse(cmdStr))
        {
            return;
        }

        bridge->onMessage(msg.id, msg.verb, msg.flags, msg.payload, this);
        return;
    }
    case E_SIGNAL_TYPE::DISPLAY_MESSAGE:
    {
#ifdef ENABLE_WEBSOCKET
        String message = cp.description;
        if (message.isEmpty())
        {
            message = cp.name;
        }
        PHApp *phApp = static_cast<PHApp *>(owner);
        if (phApp)
        {
            JsonDocument doc;
            doc["message"] = message;
            doc["id"] = cp.id;
            doc["type"] = "user_message";
            phApp->broadcast(BROADCAST_USER_MESSAGE, doc);
        }
        cp.state = E_SIGNAL_STATE::STATE_ON;
#endif
        break;
    }
    case E_SIGNAL_TYPE::PAUSE:
    {
        pause();
        cp.state = E_SIGNAL_STATE::STATE_ON;
        break;
    }
    case E_SIGNAL_TYPE::MB_WRITE_HOLDING_REGISTER:
    {
#ifdef ENABLE_MODBUS_TCP
        if (modbusTCP == nullptr || modbusTCP->modbusServer == nullptr)
        {
            L_ERROR("SignalPlot::executeControlPointAction: modbusTCP or modbusServer is nullptr");
            return;
        }

        uint16_t targetRegAddr = static_cast<uint16_t>(cp.arg_0);
        uint16_t value = static_cast<uint16_t>(cp.arg_1);

        ModbusMessage resp;
        ModbusMessage req;
        Error err;
        req.setMessage(1, static_cast<uint8_t>(E_FN_CODE::FN_WRITE_HOLD_REGISTER), targetRegAddr, value);
        resp = modbusTCP->modbusServer->localRequest(req);
        cp.state = E_SIGNAL_STATE::STATE_ON;
#endif
        break;
    }
    case E_SIGNAL_TYPE::STOP_PIDS:
    {
        PlotBase *parent = getParent();
        if (parent)
        {
            PHApp *phApp = static_cast<PHApp *>(owner);
            if (phApp)
            {
                phApp->stopPids(parent->getSlot());
            }
        }
        cp.state = E_SIGNAL_STATE::STATE_ON;
        L_INFO("SignalPlot::stopPids");
        break;
    }
    case E_SIGNAL_TYPE::START_PIDS:
    {
        PlotBase *parent = getParent();
        if (parent)
        {
            PHApp *phApp = static_cast<PHApp *>(owner);
            if (phApp)
            {
                phApp->startPids(parent->getSlot());
            }
        }
        cp.state = E_SIGNAL_STATE::STATE_ON;
        break;
    }
#ifdef ENABLE_FEEDBACK_BUZZER
    case E_SIGNAL_TYPE::BUZZER_OFF:
    case E_SIGNAL_TYPE::BUZZER_SOLID:
    case E_SIGNAL_TYPE::BUZZER_SLOW_BLINK:
    case E_SIGNAL_TYPE::BUZZER_FAST_BLINK:
    case E_SIGNAL_TYPE::BUZZER_LONG_BEEP_SHORT_PAUSE:
    {
        PHApp *phApp = static_cast<PHApp *>(owner);
        if (phApp && phApp->feedbackBuzzer_0)
        {
            FeedbackBuzzer::E_BuzzerMode mode;
            switch (cp.type)
            {
            case E_SIGNAL_TYPE::BUZZER_OFF:
                mode = FeedbackBuzzer::MODE_OFF;
                break;
            case E_SIGNAL_TYPE::BUZZER_SOLID:
                mode = FeedbackBuzzer::MODE_SOLID;
                break;
            case E_SIGNAL_TYPE::BUZZER_SLOW_BLINK:
                mode = FeedbackBuzzer::MODE_SLOW_BLINK;
                break;
            case E_SIGNAL_TYPE::BUZZER_FAST_BLINK:
                mode = FeedbackBuzzer::MODE_FAST_BLINK;
                break;
            case E_SIGNAL_TYPE::BUZZER_LONG_BEEP_SHORT_PAUSE:
                mode = FeedbackBuzzer::MODE_LONG_BEEP_SHORT_PAUSE;
                break;
            default:
                cp.state = E_SIGNAL_STATE::STATE_ERROR;
                return;
            }
            uint32_t duration_ms = (cp.arg_0 > 0) ? cp.arg_0 : 0;
            phApp->feedbackBuzzer_0->setMode(mode, duration_ms);
            cp.state = E_SIGNAL_STATE::STATE_ON;
        }
        else
        {
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
        }
        break;
    }
#endif
    case E_SIGNAL_TYPE::IFTTT_WEBHOOK:
    {
#ifdef ENABLE_IFTTT
        PHApp *phApp = static_cast<PHApp *>(owner);
        if (phApp && phApp->iftttWebhook)
        {
            phApp->iftttWebhook->triggerEvent(cp.description);
            cp.state = E_SIGNAL_STATE::STATE_ON;
        }
        else
        {
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
        }
#else
        cp.state = E_SIGNAL_STATE::STATE_ERROR;
#endif
        break;
    }
    case E_SIGNAL_TYPE::CALL_FUNCTION:
    case E_SIGNAL_TYPE::CALL_REST:
    case E_SIGNAL_TYPE::USER_DEFINED:
        cp.state = E_SIGNAL_STATE::STATE_ERROR;
        break;
    default:
        cp.state = E_SIGNAL_STATE::STATE_ERROR;
        break;
    }
}

// --- Getters for control points ---
const S_SignalControlPoint *SignalPlot::getControlPoints() const
{
    return _controlPoints;
}

uint8_t SignalPlot::getNumControlPoints() const
{
    return _numControlPoints;
}

#endif
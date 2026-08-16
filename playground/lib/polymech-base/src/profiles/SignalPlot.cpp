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

SignalPlot::SignalPlot(Component *owner, ushort slot, ushort componentId)
    : PlotBase(owner, componentId),
      _numControlPoints(0),
      slot(slot),        // Initialize slot
      modbusTCP(nullptr), // Initialize modbusTCP
      _elapsedValue(this, componentId, "SigPlot_Elapsed")
{
    name = "SignalPlot_" + String(this->id) + "_Slot_" + String(slot);
    for (int i = 0; i < MAX_SIGNAL_POINTS; ++i)
    {
        _controlPoints[i] = {};
    }
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    const uint16_t tcpBaseAddr = MB_HREG_SIGNAL_PLOT_BASE + (slot * SIGNAL_PLOT_REGISTER_COUNT);
    
    _elapsedValue.enableFeature(E_NetworkValueFeatureFlags::E_NVFF_ALL);
    _elapsedValue.name = "SigPlot Elapsed";
    _elapsedValue.initModbus(
        tcpBaseAddr + static_cast<uint16_t>(SignalPlotRegisterOffset::ELAPSED_LW),
        1,
        this->id,
        this->slaveId,
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        _elapsedValue.name.c_str(),
        this->name.c_str()
    );
    
    // Notify on interval step of 100ms.
    _elapsedValue.initNotify(0, 100, NetworkValue_ThresholdMode::INTERVAL_STEP,
        [](const uint32_t &oldValue, const uint32_t &newValue) { });
    
    _modbusBlocks[static_cast<uint16_t>(SignalPlotRegisterOffset::STATUS)] =
        INIT_MODBUS_BLOCK(SignalPlotRegisterOffset::STATUS, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SigPlot Status", name.c_str());
    _modbusBlocks[static_cast<uint16_t>(SignalPlotRegisterOffset::DURATION_LW)] =
        INIT_MODBUS_BLOCK(SignalPlotRegisterOffset::DURATION_LW, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SigPlot Duration LW", name.c_str());
    _modbusBlocks[static_cast<uint16_t>(SignalPlotRegisterOffset::DURATION_HW)] =
        INIT_MODBUS_BLOCK(SignalPlotRegisterOffset::DURATION_HW, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SigPlot Duration HW", name.c_str());
    _modbusBlocks[static_cast<uint16_t>(SignalPlotRegisterOffset::COMMAND)] =
        INIT_MODBUS_BLOCK(SignalPlotRegisterOffset::COMMAND, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_WRITE_ONLY, "SigPlot Command", name.c_str());
    _modbusBlocks[static_cast<uint16_t>(SignalPlotRegisterOffset::ENABLE_CMD)] =
        INIT_MODBUS_BLOCK(SignalPlotRegisterOffset::ENABLE_CMD, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, "SigPlot Enable", name.c_str());    
    _modbusBlocks[static_cast<uint16_t>(SignalPlotRegisterOffset::ELAPSED_LW)] = _elapsedValue.getRegisterInfo();
    
    _modbusBlockView = {_modbusBlocks, SIGNAL_PLOT_REGISTER_COUNT};
    disable(); // Disable by default
}

short SignalPlot::setup()
{
    return E_OK;
}

short SignalPlot::loop()
{
    PlotBase::loop();

    if (!isRunning() || !enabled() || modbusTCP == nullptr || modbusTCP->modbusServer == nullptr)
    {
        return E_OK;
    }
    uint32_t currentElapsedMs = getElapsedMs();
    _elapsedValue.update(getElapsedMs());
    uint32_t now = millis();
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

    // Load name if present
    if (config["name"].is<String>())
    {
        this->name = config["name"].as<String>();
        Log.verboseln("%s: Loaded name from config: %s", this->name.c_str(), this->name.c_str());
    }
    else
    {
        // Name might have been set in constructor, or keep default if not in config
        Log.verboseln("%s: 'name' not found in config or not a string, using existing name: %s", this->name.c_str(), this->name.c_str());
    }

    // Load duration if present (expected in milliseconds from JSON)
    if (config["duration"].is<uint32_t>())
    {
        this->_durationMs = config["duration"].as<uint32_t>(); // Assign directly as ms
    }

    // Slot is set by constructor and typically not overridden by JSON load().
    _numControlPoints = 0; // Reset count before loading new points
    for (int i = 0; i < MAX_SIGNAL_POINTS; ++i)
    {
        _controlPoints[i] = {}; // Clear existing points
    }

    if (!config["controlPoints"].is<JsonArray>())
    {
        Log.warningln("%s: 'controlPoints' array not found or not an array in config. No points loaded.", name.c_str());
        _numControlPoints = 0;
        return false; // No points to load is considered a failure for a plot that needs points.
    }

    JsonArray pointsArray = config["controlPoints"].as<JsonArray>();

    _numControlPoints = min((uint8_t)pointsArray.size(), (uint8_t)MAX_SIGNAL_POINTS);
    if (_numControlPoints < 1)
    {
        Log.warningln("%s: 'controlPoints' array is empty. No points loaded.", name.c_str());
        _numControlPoints = 0;
        return false;
    }

    for (uint8_t i = 0; i < _numControlPoints; ++i)
    {
        JsonObject pointObj = pointsArray[i].as<JsonObject>();

        _controlPoints[i].id = pointObj["id"].as<uint8_t>();
        _controlPoints[i].time = pointObj["time"].as<uint32_t>();
        _controlPoints[i].name = pointObj["name"].as<String>();
        _controlPoints[i].description = pointObj["description"].as<String>();

        _controlPoints[i].type = static_cast<E_SIGNAL_TYPE>(pointObj["type"].as<int16_t>());

        _controlPoints[i].arg_0 = pointObj["arg_0"].as<int16_t>();
        // Always load arg_1 directly from JSON.
        // JSON must provide the correct value (e.g., 0 or 1 for MB_WRITE_COIL).
        if (pointObj["arg_1"].is<int>())
        {
            _controlPoints[i].arg_1 = pointObj["arg_1"].as<int16_t>();
        }
        else
        {
            _controlPoints[i].arg_1 = 0; // Default if not present, or log warning
            Log.warningln("%s: CP id %d, type %d - 'arg_1' not found in JSON. Defaulting to 0.",
                          name.c_str(), _controlPoints[i].id, static_cast<int>(_controlPoints[i].type));
        }

        if (pointObj["arg_2"].is<int>())
        {
            _controlPoints[i].arg_2 = pointObj["arg_2"].as<int16_t>();
        }
        else
        {
            _controlPoints[i].arg_2 = 0; // Default if not present
        }

        // user field logic remains as is (currently skipped for assignment)
        if (!pointObj["user"].isNull())
        {
            // ... user field handling ...
        }

        // Load the actual execution state for this point, if provided in JSON.
        // This is distinct from the coil value that might have been in jsonState before.
        if (pointObj["state"].is<int>())
        {
            _controlPoints[i].state = static_cast<E_SIGNAL_STATE>(pointObj["state"].as<int16_t>());
        }
        else
        {
            _controlPoints[i].state = E_SIGNAL_STATE::STATE_NONE; // Default initial state for execution tracking
        }
    }

    // Data is assumed sorted by time, no sorting needed here.

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

// --- Modbus Overrides Implementation ---

uint16_t SignalPlot::mb_tcp_base_address() const
{
    return MB_HREG_SIGNAL_PLOT_BASE + (this->slot * SIGNAL_PLOT_REGISTER_COUNT);
}

ModbusBlockView *SignalPlot::mb_tcp_blocks() const
{
    return const_cast<ModbusBlockView *>(&_modbusBlockView);
}

void SignalPlot::mb_tcp_register(ModbusTCP *manager)
{
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS))
        return;

    const_cast<SignalPlot *>(this)->modbusTCP = manager;
    uint16_t instanceBaseAddr = mb_tcp_base_address();
    ModbusBlockView *blocksView = mb_tcp_blocks();
    Component *thiz = const_cast<SignalPlot *>(this);
    for (int i = 0; i < blocksView->count; ++i)
    {
        MB_Registers info = blocksView->data[i];
        info.componentId = this->id;
        manager->registerModbus(thiz, info);
    }
}

short SignalPlot::mb_tcp_read(MB_Registers *reg)
{
    uint32_t val32 = 0;
    short requestedAddress = reg->startAddress; // This is the absolute address from Modbus manager
    uint16_t instanceBaseAddr = mb_tcp_base_address();
    short offset = requestedAddress - instanceBaseAddr;
    // Define offsets for the NetworkValue part. These are relative to the component's base address.
    const uint16_t ELAPSED_LW_OFFSET = static_cast<uint16_t>(SignalPlotRegisterOffset::ELAPSED_LW);
    //const uint16_t ELAPSED_HW_OFFSET = static_cast<uint16_t>(SignalPlotRegisterOffset::ELAPSED_HW);

    // First, check if it's for the NetworkValue
    if (offset == ELAPSED_LW_OFFSET)
    {
        return (uint16_t)LOW_WORD(_elapsedValue.getValue());
    }
    //if (offset == ELAPSED_HW_OFFSET)
    //{
    //    return (uint16_t)HIGH_WORD(_elapsedValue.getValue());
    //}

    // Then check the regular registers
    if (offset < 0 || offset >= SIGNAL_PLOT_REGISTER_COUNT)
    {
        // Log.warningln("%s: Read request for address %d outside instance block. Offset: %d", name.c_str(), requestedAddress, offset);
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }

    SignalPlotRegisterOffset regOffset = static_cast<SignalPlotRegisterOffset>(offset);
    switch (regOffset)
    {
    case SignalPlotRegisterOffset::STATUS:
        return (short)getCurrentStatus();
    case SignalPlotRegisterOffset::DURATION_LW:
        val32 = getDuration();
        return (uint16_t)LOW_WORD(val32);
    case SignalPlotRegisterOffset::DURATION_HW:
        val32 = getDuration();
        return (uint16_t)HIGH_WORD(val32);
    case SignalPlotRegisterOffset::COMMAND:   // Write-only
        return MODBUS_ERROR_ILLEGAL_FUNCTION; // Or appropriate error
    case SignalPlotRegisterOffset::ENABLE_CMD:
        return enabled() ? 1 : 0;
    default:
        Log.errorln("%s: Read default case for offset %d (addr %d). Logic error.", name.c_str(), offset, requestedAddress);
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }
}

short SignalPlot::mb_tcp_write(MB_Registers *reg, short value)
{
    short requestedAddress = reg->startAddress; // Absolute address
    uint16_t instanceBaseAddr = mb_tcp_base_address();
    short offset = requestedAddress - instanceBaseAddr;

    // Define offsets for the NetworkValue part. These are relative to the component's base address.
    const uint16_t ELAPSED_LW_OFFSET = static_cast<uint16_t>(SignalPlotRegisterOffset::ELAPSED_LW);
    //const uint16_t ELAPSED_HW_OFFSET = static_cast<uint16_t>(SignalPlotRegisterOffset::ELAPSED_HW);

    // Protect NetworkValue from being written
    if (offset == ELAPSED_LW_OFFSET)
    {
        return MODBUS_ERROR_ILLEGAL_FUNCTION; // It's read-only
    }
    //if (offset == ELAPSED_HW_OFFSET)
    //{
    //    return MODBUS_ERROR_ILLEGAL_FUNCTION; // It's read-only
    //}

    if (offset < 0 || offset >= SIGNAL_PLOT_REGISTER_COUNT)
    {
        Log.warningln("%s: Write req for addr %d OUTSIDE instance block. Offset: %d", name.c_str(), requestedAddress, offset);
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }

    SignalPlotRegisterOffset regOffset = static_cast<SignalPlotRegisterOffset>(offset);
    switch (regOffset)
    {
    case SignalPlotRegisterOffset::COMMAND:
        Log.infoln("%s: Received command via Modbus (Offset %d, Addr %d): %d", name.c_str(), offset, requestedAddress, value);
        switch (static_cast<SignalPlotCommand>(value))
        {
        case SignalPlotCommand::START:
            start();
            break;
        case SignalPlotCommand::STOP:
            stop();
            break;
        case SignalPlotCommand::PAUSE:
            pause();
            break;
        case SignalPlotCommand::RESUME:
            resume();
            break;
        default:
            Log.warningln("%s: Invalid command value %d for COMMAND register.", name.c_str(), value);
            return MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        return E_OK;
    case SignalPlotRegisterOffset::ENABLE_CMD:
        Log.infoln("%s: Received enable/disable (Offset %d, Addr %d): %d", name.c_str(), offset, requestedAddress, value);
        if (value)
        {
            enable();
        }
        else
        {
            disable();
        }
        return E_OK;
    default:
        Log.errorln("%s: Write default case for offset %d (Addr %d). Read-only or Logic error.", name.c_str(), offset, requestedAddress);
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS; // Or ILLEGAL_FUNCTION if it's a read-only reg
    }
}

void SignalPlot::start()
{
    PlotBase::start();
    for (uint8_t i = 0; i < _numControlPoints; ++i)
    {
        _controlPoints[i].state = E_SIGNAL_STATE::STATE_NONE;
    }
    Log.infoln("%s: Started. Control point states reset.", name.c_str());
}

void SignalPlot::executeControlPointAction(uint8_t cpIndex)
{
    if (cpIndex >= _numControlPoints)
    {
        Log.errorln("%s: Invalid control point index %d in executeControlPointAction.", name.c_str(), cpIndex);
        return;
    }
    S_SignalControlPoint &cp = _controlPoints[cpIndex];
    switch (cp.type)
    {
    case E_SIGNAL_TYPE::MB_WRITE_COIL:
    {
        if (modbusTCP == nullptr || modbusTCP->modbusServer == nullptr)
        {
            Log.errorln("%s: Modbus not available for MB_WRITE_COIL at CP id %d.", name.c_str(), cp.id);
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            return;
        }
        uint16_t coilAddress = static_cast<uint16_t>(cp.arg_0);
        uint16_t coilValue = (cp.arg_1 != 0) ? 0xFF00 : 0x0000; // 0xFF00 for ON, 0x0000 for OFF
        ModbusMessage req;
        // Using MB_GATEWAY_ESP_DEFAULT_SLAVE_ID for local requests to the ESP's own server instance.
        // Adjust if target is an external slave.
        req.setMessage(1, static_cast<uint8_t>(E_FN_CODE::FN_WRITE_COIL), coilAddress, coilValue);

        Log.infoln("%s: CP id %d: MB_WRITE_COIL Addr: %u, Value: %s", name.c_str(), cp.id, coilAddress, (coilValue == 0xFF00 ? "ON" : "OFF"));

        ModbusMessage resp = modbusTCP->modbusServer->localRequest(req);
        Error err = resp.getError();

        if (err == SUCCESS)
        {
            cp.state = E_SIGNAL_STATE::STATE_ON; // Mark as executed successfully
            Log.verboseln("%s: CP id %d: MB_WRITE_COIL success.", name.c_str(), cp.id);
        }
        else
        {
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            ModbusError me(err);
            Log.errorln("%s: CP id %d: MB_WRITE_COIL Error: %02d - %s", name.c_str(), cp.id, err, (const char *)me);
        }
        break;
    }
    case E_SIGNAL_TYPE::GPIO_WRITE:
    {
        uint8_t pin = static_cast<uint8_t>(cp.arg_0);
        E_GpioWriteMode gpioMode = static_cast<E_GpioWriteMode>(cp.arg_1);
        int16_t value = cp.arg_2;

        Log.infoln("%s: CP id %d: GPIO_WRITE Pin: %u, Mode: %d, Value: %d",
                   name.c_str(), cp.id, pin, static_cast<int16_t>(gpioMode), value);

        pinMode(pin, OUTPUT);

        if (gpioMode == E_GpioWriteMode::DIGITAL)
        {
            digitalWrite(pin, (value != 0) ? HIGH : LOW);
            cp.state = E_SIGNAL_STATE::STATE_ON;
            Log.verboseln("%s: CP id %d: digitalWrite(%u, %s) success.",
                          name.c_str(), cp.id, pin, (value != 0 ? "HIGH" : "LOW"));
        }
        else if (gpioMode == E_GpioWriteMode::ANALOG_PWM)
        {
            if (value < 0)
                value = 0;
            if (value > 255)
                value = 255;
            analogWrite(pin, value);
            cp.state = E_SIGNAL_STATE::STATE_ON;
            Log.verboseln("%s: CP id %d: analogWrite(%u, %d) attempt. (Note: ESP32 PWM may need ledcSetup)",
                          name.c_str(), cp.id, pin, value);
        }
        else
        {
            Log.warningln("%s: CP id %d: GPIO_WRITE Unknown mode %d.", name.c_str(), cp.id, static_cast<int16_t>(gpioMode));
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
        }
        break;
    }
    case E_SIGNAL_TYPE::CALL_METHOD:
    {
        if (!cp.user)
        {
            Log.errorln("%s: CP id %d: CALL_METHOD has null user pointer.", name.c_str(), cp.id);
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            break;
        }

        String cmdStr = String((char *)cp.user);
        
        Bridge *bridge = static_cast<Bridge *>(owner->byId(COMPONENT_KEY_MB_BRIDGE));
        if (!bridge)
        {
            Log.errorln("%s: CP id %d: Bridge component not found.", name.c_str(), cp.id);
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            break;
        }

        CommandMessage msg;
        if (!msg.parse(cmdStr))
        {
            Log.errorln("%s: CP id %d: Failed to parse command string: %s", name.c_str(), cp.id, cmdStr.c_str());
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            break;
        }

        short result = bridge->onMessage(msg.id, msg.verb, msg.flags, msg.payload, this);
        if (result == E_OK)
        {
            cp.state = E_SIGNAL_STATE::STATE_ON;
            Log.verboseln("%s: CP id %d: CALL_METHOD executed successfully.", name.c_str(), cp.id);
        }
        else
        {
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            Log.errorln("%s: CP id %d: CALL_METHOD failed with error %d.", name.c_str(), cp.id, result);
        }
        break;
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
        else
        {
            Log.errorln("%s: CP id %d: DISPLAY_MESSAGE: PHApp component not found.", name.c_str(), cp.id);
        }
        Log.infoln("%s: CP id %d: DISPLAY_MESSAGE: %s", name.c_str(), cp.id, message.c_str());
        cp.state = E_SIGNAL_STATE::STATE_ON;
        #endif
        break;
    }
    case E_SIGNAL_TYPE::PAUSE:
    {
        Log.infoln("%s: CP id %d: PAUSE", name.c_str(), cp.id);
        pause();
        cp.state = E_SIGNAL_STATE::STATE_ON;
        break;
    }
    case E_SIGNAL_TYPE::MB_WRITE_HOLDING_REGISTER:
    {
#ifdef ENABLE_MODBUS_TCP
        if (modbusTCP == nullptr || modbusTCP->modbusServer == nullptr)
        {
            Log.errorln("%s: Modbus not available for MB_WRITE_HOLDING_REGISTER at CP id %d.", name.c_str(), cp.id);
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            break;
        }

        uint16_t targetRegAddr = static_cast<uint16_t>(cp.arg_0);
        uint16_t value = static_cast<uint16_t>(cp.arg_1);

        ModbusMessage resp;
        ModbusMessage req;
        Error err;

        req.setMessage(1, static_cast<uint8_t>(E_FN_CODE::FN_WRITE_HOLD_REGISTER), targetRegAddr, value);
        Log.infoln("%s: CP id %d: MB_WRITE_HOLDING_REGISTER Addr: %u, Value: %u", name.c_str(), cp.id, targetRegAddr, value);
        resp = modbusTCP->modbusServer->localRequest(req);

        if ((err = resp.getError()) == SUCCESS)
        {
            cp.state = E_SIGNAL_STATE::STATE_ON; // Mark as executed successfully
            Log.infoln("%s: CP id %d: MB_WRITE_HOLDING_REGISTER success.", name.c_str(), cp.id);
        }
        else
        {
            cp.state = E_SIGNAL_STATE::STATE_ERROR;
            ModbusError me(err);
            Log.errorln("%s: CP id %d: MB_WRITE_HOLDING_REGISTER Error writing %u to slave 1, register %u: %02d - %s",
                        name.c_str(), cp.id, value, targetRegAddr, err, (const char *)me);
        }
#endif
        break;
    }
    case E_SIGNAL_TYPE::CALL_FUNCTION:
    case E_SIGNAL_TYPE::CALL_REST:
    case E_SIGNAL_TYPE::USER_DEFINED:
        Log.warningln("%s: CP id %d: Signal type %d not yet implemented in loop.", name.c_str(), cp.id, static_cast<int16_t>(cp.type));
        cp.state = E_SIGNAL_STATE::STATE_ERROR;
        break;
    default:
        Log.errorln("%s: CP id %d: Unknown signal type %d.", name.c_str(), cp.id, static_cast<int16_t>(cp.type));
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

short SignalPlot::onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user, Component *src)
{
    return this->owner->onMessage(id, verb, flags, user, src);
}
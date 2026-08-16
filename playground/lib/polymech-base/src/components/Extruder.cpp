#include "Extruder.h"
#include <Arduino.h>

Extruder::Extruder(Component *owner, SAKO_VFD *vfd, Pos3Analog *joystick, POT *speedPot, POT *overloadPot)
    : Component(EXTRUDER_COMPONENT_NAME, COMPONENT_KEY_EXTRUDER, Component::COMPONENT_DEFAULT, owner),
      _vfd(vfd),
      _joystick(joystick),
      _speedPot(speedPot),
      _overloadPot(overloadPot),
      _currentState(ExtruderState::IDLE),
      _lastJoystickDirection(Pos3Analog::E_POS3_DIRECTION::MIDDLE),
      _currentSpeedPotValue(100),
      _currentOverloadPotValue(100),
      _calculatedExtrudingSpeedHz(EXTRUDER_SPEED_MEDIUM_HZ),
      _calculatedOverloadThresholdPercent(EXTRUDER_OVERLOAD_POT_MAX_TORQUE_PERCENT),
      _lastStateChangeTimeMs(0),
      _jammedStartTimeMs(0),
      _lastVfdReadTimeMs(0),
      _joystickHoldStartTimeMs(0),
      _modbusCommandRegisterValue(static_cast<short>(ExtruderModbusCommand::NO_COMMAND)),
      _operationStartTimeMs(0),
      _currentMaxOperationTimeMs(0)
{
    if (!_vfd)
    {
        Log.errorln("[%s] ERROR: VFD pointer is null!", name.c_str());
        return;
    }
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
}

short Extruder::init()
{
    if (!_vfd)
    {
        Log.errorln("[%s] ERROR: Cannot initialize - VFD pointer is null!", name.c_str());
        return E_INVALID_PARAMETER;
    }
    Log.infoln("[%s] init() called. Setting to default state.", name.c_str());

    _updatePotValues();
    _vfdStop();
    _transitionToState(ExtruderState::IDLE);

    _lastJoystickDirection = Pos3Analog::E_POS3_DIRECTION::MIDDLE;
    _lastStateChangeTimeMs = millis();
    _jammedStartTimeMs = 0;
    _lastVfdReadTimeMs = 0;
    _joystickHoldStartTimeMs = 0;
    _operationStartTimeMs = 0;
    _currentMaxOperationTimeMs = 0;
    _modbusCommandRegisterValue = static_cast<short>(ExtruderModbusCommand::NO_COMMAND);

    return E_OK;
}

short Extruder::setup()
{
    Component::setup();
    return this->init();
}

void Extruder::_transitionToState(ExtruderState newState)
{
    if (_currentState != newState)
    {
        Log.verboseln("[%s] State transition: %d -> %d", name.c_str(), static_cast<uint8_t>(_currentState), static_cast<uint8_t>(newState));
        _currentState = newState;
        _lastStateChangeTimeMs = millis();
        if (newState != ExtruderState::JAMMED)
        {
            _jammedStartTimeMs = 0;
        }
        if (newState == ExtruderState::IDLE ||
            newState == ExtruderState::STOPPING ||
            newState == ExtruderState::JAMMED ||
            newState == ExtruderState::RESETTING_JAM ||
            newState == ExtruderState::EXTRUDING_AUTO)
        {
            _joystickHoldStartTimeMs = 0;
        }

        if (newState == ExtruderState::EXTRUDING_MANUAL || newState == ExtruderState::EXTRUDING_AUTO)
        {
            _operationStartTimeMs = millis();
            float currentSpeedHzForCalc = _calculatedExtrudingSpeedHz;

            if (currentSpeedHzForCalc > 0 && EXTRUDER_SPEED_MEDIUM_HZ > 0)
            {
                _currentMaxOperationTimeMs = static_cast<unsigned long>(
                    EXTRUDER_MAX_RUN_TIME_MEDIUM_SPEED_MS *
                    (static_cast<float>(EXTRUDER_SPEED_MEDIUM_HZ) / currentSpeedHzForCalc));
                Log.verboseln("[%s] New operation. Max time: %lu ms for speed %.2f (0.01Hz)", name.c_str(), _currentMaxOperationTimeMs, currentSpeedHzForCalc);
            }
            else
            {
                _currentMaxOperationTimeMs = EXTRUDER_MAX_RUN_TIME_MEDIUM_SPEED_MS;
                Log.warningln("[%s] Operation speed for max time calc is zero or invalid. Defaulting max time to %lu ms.", name.c_str(), _currentMaxOperationTimeMs);
            }
        }
        else
        {
            _operationStartTimeMs = 0;
            _currentMaxOperationTimeMs = 0;
        }
    }
}

void Extruder::_updatePotValues()
{
    _currentSpeedPotValue = _speedPot ? _speedPot->getValue() : 100;
    float multiplier = EXTRUDER_SPEED_POT_MIN_MULTIPLIER +
                       (EXTRUDER_SPEED_POT_MAX_MULTIPLIER - EXTRUDER_SPEED_POT_MIN_MULTIPLIER) *
                           (_currentSpeedPotValue / 100.0f);
    _calculatedExtrudingSpeedHz = static_cast<float>(EXTRUDER_SPEED_MEDIUM_HZ) * multiplier;

    _currentOverloadPotValue = _overloadPot ? _overloadPot->getValue() : 100;
    _calculatedOverloadThresholdPercent = static_cast<uint8_t>(map(_currentOverloadPotValue, 0, 100, EXTRUDER_OVERLOAD_POT_MIN_TORQUE_PERCENT, EXTRUDER_OVERLOAD_POT_MAX_TORQUE_PERCENT));
    _calculatedOverloadThresholdPercent = constrain(_calculatedOverloadThresholdPercent, EXTRUDER_OVERLOAD_POT_MIN_TORQUE_PERCENT, EXTRUDER_OVERLOAD_POT_MAX_TORQUE_PERCENT);
}

void Extruder::_vfdStartForward(uint16_t frequencyCentiHz)
{
    if (!_vfd)
        return;
    Log.verboseln("[%s] VFD Start Forward: %d (0.01Hz units)", name.c_str(), frequencyCentiHz);
    _vfd->setFrequency(frequencyCentiHz);
    _vfd->run();
}

void Extruder::_vfdStartReverse(uint16_t frequencyCentiHz)
{
    if (!_vfd)
        return;
    Log.verboseln("[%s] VFD Start Reverse: %d (0.01Hz units)", name.c_str(), frequencyCentiHz);
    _vfd->setFrequency(frequencyCentiHz);
    _vfd->reverse();
}

void Extruder::_vfdStop()
{
    if (!_vfd)
        return;
    Log.verboseln("[%s] VFD Stop", name.c_str());
    _vfd->stop();
}

void Extruder::_vfdResetJam()
{
    if (!_vfd)
        return;
    Log.verboseln("[%s] VFD Resetting Fault/Jam", name.c_str());
    _vfd->resetFault();
}

void Extruder::_checkVfdForJam()
{
    if (!_vfd)
        return;
    if (!(_currentState == ExtruderState::EXTRUDING_MANUAL || _currentState == ExtruderState::EXTRUDING_AUTO))
        return;

    short currentTorque = _vfd->getTorque();

    if (_vfd->hasFault())
    {
        Log.errorln("[%s] JAMMED! VFD reports fault code %d.", name.c_str(), _vfd->getFaultCode());
        _transitionToState(ExtruderState::JAMMED);
        return;
    }

    if (currentTorque > _calculatedOverloadThresholdPercent)
    {
        if (_jammedStartTimeMs == 0)
        {
            _jammedStartTimeMs = millis();
            Log.warningln("[%s] High torque detected (%d%% > %d%%). Monitoring for jam.", name.c_str(), currentTorque, _calculatedOverloadThresholdPercent);
        }
        else if (millis() - _jammedStartTimeMs > 1000)
        {
            Log.errorln("[%s] JAMMED! Torque %d%% > %d%% for >%lums.", name.c_str(), currentTorque, _calculatedOverloadThresholdPercent, 1000);
            _transitionToState(ExtruderState::JAMMED);
        }
    }
    else
    {
        if (_jammedStartTimeMs != 0)
        {
            Log.infoln("[%s] Torque (%d%%) normalized below threshold (%d%%). Jam monitor reset.", name.c_str(), currentTorque, _calculatedOverloadThresholdPercent);
        }
        _jammedStartTimeMs = 0;
    }
}

short Extruder::loop()
{
    Component::loop();
    if (!_vfd)
    {
        return E_INVALID_PARAMETER;
    }
    return E_OK;
    _updatePotValues();
    Pos3Analog::E_POS3_DIRECTION currentJoystickDir = _joystick ? static_cast<Pos3Analog::E_POS3_DIRECTION>(_joystick->getValue()) : Pos3Analog::E_POS3_DIRECTION::MIDDLE;
    if ((_currentState == ExtruderState::EXTRUDING_MANUAL || _currentState == ExtruderState::EXTRUDING_AUTO) &&
        _operationStartTimeMs > 0 && _currentMaxOperationTimeMs > 0)
    {
        if (now - _operationStartTimeMs > _currentMaxOperationTimeMs)
        {
            Log.warningln("[%s] MAX OPERATION TIME (%lu ms) EXCEEDED for state %d! Stopping.",
                          name.c_str(), _currentMaxOperationTimeMs, static_cast<int>(_currentState));
            _transitionToState(ExtruderState::STOPPING);
        }
    }

    if ((_currentState == ExtruderState::EXTRUDING_MANUAL || _currentState == ExtruderState::EXTRUDING_AUTO) &&
        (now - _lastVfdReadTimeMs > EXTRUDER_VFD_READ_INTERVAL_MS))
    {
        _checkVfdForJam();
        _lastVfdReadTimeMs = now;
    }

    switch (_currentState)
    {
    case ExtruderState::IDLE:
        _handleIdleState();
        break;
    case ExtruderState::EXTRUDING_MANUAL:
        _handleExtrudingManualState();
        break;
    case ExtruderState::EXTRUDING_AUTO:
        _handleExtrudingAutoState();
        break;
    case ExtruderState::STOPPING:
        _handleStoppingState();
        break;
    case ExtruderState::JAMMED:
        _handleJammedState();
        break;
    case ExtruderState::RESETTING_JAM:
        _handleResettingJamState();
        break;
    default:
        Log.warningln("[%s] Unknown state: %d. Transitioning to IDLE.", name.c_str(), static_cast<uint8_t>(_currentState));
        _transitionToState(ExtruderState::IDLE);
        break;
    }
    _lastJoystickDirection = currentJoystickDir;
    return E_OK;
}

void Extruder::_handleIdleState()
{
    Pos3Analog::E_POS3_DIRECTION joyDir = _joystick ? static_cast<Pos3Analog::E_POS3_DIRECTION>(_joystick->getValue()) : Pos3Analog::E_POS3_DIRECTION::MIDDLE;
    if (joyDir == Pos3Analog::E_POS3_DIRECTION::UP && _lastJoystickDirection != Pos3Analog::E_POS3_DIRECTION::UP)
    {
        Log.verboseln("[%s] Joystick UP from IDLE. Starting EXTRUDING_MANUAL.", name.c_str());
        _joystickHoldStartTimeMs = millis();
        _vfdStartForward(static_cast<uint16_t>(_calculatedExtrudingSpeedHz));
        _transitionToState(ExtruderState::EXTRUDING_MANUAL);
    }
    if (_vfd->isRunning() && _joystickHoldStartTimeMs == 0)
    {
        Log.warningln("[%s] VFD running in IDLE state unexpectedly. Stopping.", name.c_str());
        _vfdStop();
    }
}

void Extruder::_handleExtrudingManualState()
{
    Pos3Analog::E_POS3_DIRECTION joyDir = _joystick ? static_cast<Pos3Analog::E_POS3_DIRECTION>(_joystick->getValue()) : Pos3Analog::E_POS3_DIRECTION::MIDDLE;

    if (joyDir == Pos3Analog::E_POS3_DIRECTION::UP)
    {
        if (_joystickHoldStartTimeMs > 0 && (millis() - _joystickHoldStartTimeMs > EXTRUDER_AUTO_MODE_HOLD_DURATION_MS))
        {
            Log.infoln("[%s] Joystick held UP for auto mode. Transitioning to EXTRUDING_AUTO.", name.c_str());
            _transitionToState(ExtruderState::EXTRUDING_AUTO);
        }
    }
    else
    {
        Log.infoln("[%s] Joystick released/moved from UP during EXTRUDING_MANUAL. Stopping.", name.c_str());
        _transitionToState(ExtruderState::STOPPING);
    }
}

void Extruder::_handleExtrudingAutoState()
{
    Pos3Analog::E_POS3_DIRECTION joyDir = _joystick ? static_cast<Pos3Analog::E_POS3_DIRECTION>(_joystick->getValue()) : Pos3Analog::E_POS3_DIRECTION::MIDDLE;

    if (joyDir != Pos3Analog::E_POS3_DIRECTION::UP)
    {
        Log.infoln("[%s] Joystick moved from UP during EXTRUDING_AUTO. Aborting to STOPPING.", name.c_str());
        _transitionToState(ExtruderState::STOPPING);
    }
}

void Extruder::_handleStoppingState()
{
    _vfdStop();
    _joystickHoldStartTimeMs = 0;
    _transitionToState(ExtruderState::IDLE);
}

void Extruder::_handleJammedState()
{
    _vfdStop();
    _joystickHoldStartTimeMs = 0;
    Pos3Analog::E_POS3_DIRECTION joyDir = _joystick ? static_cast<Pos3Analog::E_POS3_DIRECTION>(_joystick->getValue()) : Pos3Analog::E_POS3_DIRECTION::MIDDLE;
    if (joyDir == Pos3Analog::E_POS3_DIRECTION::MIDDLE && _lastJoystickDirection != Pos3Analog::E_POS3_DIRECTION::MIDDLE)
    {
        Log.infoln("[%s] Jammed: Joystick moved to MIDDLE. Ready for reset attempt.", name.c_str());
        _transitionToState(ExtruderState::RESETTING_JAM);
    }
}

void Extruder::_handleResettingJamState()
{
    Pos3Analog::E_POS3_DIRECTION joyDir = _joystick ? static_cast<Pos3Analog::E_POS3_DIRECTION>(_joystick->getValue()) : Pos3Analog::E_POS3_DIRECTION::MIDDLE;
    if (joyDir == Pos3Analog::E_POS3_DIRECTION::MIDDLE)
    {
    }
    else if (joyDir != Pos3Analog::E_POS3_DIRECTION::MIDDLE && _lastJoystickDirection == Pos3Analog::E_POS3_DIRECTION::MIDDLE)
    {
        Log.infoln("[%s] Resetting Jam: Joystick moved from MIDDLE. Returning to IDLE. Manual VFD reset might be needed.", name.c_str());
        _vfdResetJam();
        _transitionToState(ExtruderState::IDLE);
    }
}

short Extruder::info()
{
    Log.verboseln("--- Extruder Info (ID: %d, Name: %s) ---", id, name.c_str());
    Log.verboseln("State: %d, LastJoy: %d, CurrentJoy: %d",
                  static_cast<uint8_t>(_currentState),
                  static_cast<int>(_lastJoystickDirection),
                  static_cast<int>(_joystick->getValue()));
    Log.verboseln("SpeedPOT: %d (-> %.2f 0.01Hz), OverloadPOT: %d (-> %d%% Torque)",
                  _currentSpeedPotValue, _calculatedExtrudingSpeedHz,
                  _currentOverloadPotValue, _calculatedOverloadThresholdPercent);
    uint16_t freq = 0;
    bool freqValid = _vfd->getFrequency(freq);
    Log.verboseln("VFD: Running=%s, Fault=%s, FreqSet=%.2fHz, Torque=%d%%",
                  _vfd->isRunning() ? "YES" : "NO",
                  _vfd->hasFault() ? "YES" : "NO",
                  freqValid ? (freq / 100.0f) : -1.0f,
                  _vfd->getTorque());
    return E_OK;
}

short Extruder::debug()
{
    return info();
}

short Extruder::serial_register(Bridge *b)
{
    if (!b)
        return E_INVALID_PARAMETER;
    b->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&Extruder::info);
    b->registerMemberFunction(id, this, C_STR("extrude"), (ComponentFnPtr)&Extruder::cmd_extrude);
    b->registerMemberFunction(id, this, C_STR("stop"), (ComponentFnPtr)&Extruder::cmd_stop);
    return E_OK;
}

short Extruder::cmd_extrude()
{
    Log.infoln("[%s] cmd_extrude received.", name.c_str());
    if (_currentState == ExtruderState::IDLE)
    {
        Log.infoln("[%s] Initiating EXTRUDING_AUTO from command.", name.c_str());
        _vfdStartForward(static_cast<uint16_t>(_calculatedExtrudingSpeedHz));
        _transitionToState(ExtruderState::EXTRUDING_AUTO);
        return E_OK;
    }
    else
    {
        Log.warningln("[%s] cmd_extrude ignored. Current state is %d (not IDLE).", name.c_str(), static_cast<int>(_currentState));
        return 1;
    }
}

short Extruder::cmd_stop()
{
    Log.infoln("[%s] cmd_stop received.", name.c_str());
    if (_currentState != ExtruderState::IDLE && _currentState != ExtruderState::STOPPING)
    {
        Log.infoln("[%s] Initiating STOPPING from command. Current state: %d", name.c_str(), static_cast<int>(_currentState));
        _transitionToState(ExtruderState::STOPPING);
        return E_OK;
    }
    else
    {
        Log.infoln("[%s] cmd_stop: Already IDLE or STOPPING. No action taken.", name.c_str());
        return E_OK;
    }
}

ModbusBlockView *Extruder::mb_tcp_blocks() const
{
    static MB_Registers blocks[EXTRUDER_MB_BLOCK_COUNT] = {
        {static_cast<uint16_t>(EXTRUDER_MB_BASE_ADDRESS + EXTRUDER_MB_STATE_OFFSET),
         1,
         E_FN_CODE::FN_READ_HOLD_REGISTER,
         MB_ACCESS_READ_ONLY,
         static_cast<uint16_t>(id),
         EXTRUDER_MB_STATE_OFFSET,
         "Extruder State",
         EXTRUDER_COMPONENT_NAME},
        {static_cast<uint16_t>(EXTRUDER_MB_BASE_ADDRESS + EXTRUDER_MB_COMMAND_OFFSET),
         1,
         E_FN_CODE::FN_READ_HOLD_REGISTER,
         MB_ACCESS_READ_WRITE,
         static_cast<uint16_t>(id),
         EXTRUDER_MB_COMMAND_OFFSET,
         "Extruder Command (0:None,2:Extrude,3:Stop,4:Info)",
         EXTRUDER_COMPONENT_NAME}};
    static ModbusBlockView blockView = {blocks, EXTRUDER_MB_BLOCK_COUNT};
    return &blockView;
}

void Extruder::mb_tcp_register(ModbusTCP *mgr)
{
    if (!mgr)
        return;
    ModbusBlockView *blocksView = mb_tcp_blocks();
    Component *thiz = const_cast<Extruder *>(this);
    for (int i = 0; i < blocksView->count; ++i)
    {
        mgr->registerModbus(thiz, blocksView->data[i]);
    }
}

short Extruder::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;

    uint16_t address = reg->startAddress;

    if (address == (EXTRUDER_MB_BASE_ADDRESS + EXTRUDER_MB_STATE_OFFSET))
    {
        return static_cast<short>(_currentState);
    }
    else if (address == (EXTRUDER_MB_BASE_ADDRESS + EXTRUDER_MB_COMMAND_OFFSET))
    {
        return _modbusCommandRegisterValue;
    }
    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
}

short Extruder::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    if (!reg)
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;

    uint16_t address = reg->startAddress;

    if (address == (EXTRUDER_MB_BASE_ADDRESS + EXTRUDER_MB_COMMAND_OFFSET))
    {
        ExtruderModbusCommand cmd = static_cast<ExtruderModbusCommand>(networkValue);
        Log.infoln("[%s] Modbus Command Register write. Address: %d, Value: %d (Cmd: %d)",
                   name.c_str(), address, networkValue, static_cast<int>(cmd));

        _modbusCommandRegisterValue = networkValue;

        short result = E_OK;
        switch (cmd)
        {
        case ExtruderModbusCommand::CMD_EXTRUDE:
            result = this->cmd_extrude();
            break;
        case ExtruderModbusCommand::CMD_STOP:
            result = this->cmd_stop();
            break;
        case ExtruderModbusCommand::CMD_INFO:
            result = this->info();
            break;
        case ExtruderModbusCommand::NO_COMMAND:
            Log.verboseln("[%s] Modbus NO_COMMAND received.", name.c_str());
            break;
        default:
            Log.warningln("[%s] Unknown Modbus command received: %d", name.c_str(), networkValue);
            result = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
            break;
        }

        if (cmd != ExtruderModbusCommand::NO_COMMAND && result == E_OK)
        {
            _modbusCommandRegisterValue = static_cast<short>(ExtruderModbusCommand::NO_COMMAND);
        }
        else if (result != E_OK && result != MODBUS_ERROR_ILLEGAL_DATA_VALUE)
        {
            _modbusCommandRegisterValue = static_cast<short>(ExtruderModbusCommand::NO_COMMAND);
        }

        return (result == E_OK || result == 1) ? E_OK : result;
    }
    else if (address == (EXTRUDER_MB_BASE_ADDRESS + EXTRUDER_MB_STATE_OFFSET))
    {
        Log.warningln("[%s] mb_tcp_write: Attempt to write to read-only State register %d", name.c_str(), address);
        return MODBUS_ERROR_ILLEGAL_FUNCTION;
    }

    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
}

short Extruder::reset()
{
    Log.infoln("[%s] reset() called. Stopping VFD, clearing faults, and re-initializing.", name.c_str());
    _vfdStop();
    _vfdResetJam();
    return this->init();
}

// ... rest of the file remains unchanged ...
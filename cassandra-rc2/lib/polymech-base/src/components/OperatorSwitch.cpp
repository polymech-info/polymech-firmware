#include <App.h>
#include <Bridge.h>
#include <modbus/Modbus.h>

#include "./OperatorSwitch.h"

// Helper function to convert State enum to string for logging
const char *stateToString(OperatorSwitch::State state)
{
    switch (state)
    {
    case OperatorSwitch::State::IDLE:
        return "IDLE";
    case OperatorSwitch::State::STOP_PRESSED:
        return "STOP_PRESSED";
    case OperatorSwitch::State::CYCLE_PRESSED:
        return "CYCLE_PRESSED";
    case OperatorSwitch::State::BOTH_PRESSED:
        return "BOTH_PRESSED";
    case OperatorSwitch::State::STOP_HELD:
        return "STOP_HELD";
    case OperatorSwitch::State::CYCLE_HELD:
        return "CYCLE_HELD";
    case OperatorSwitch::State::BOTH_HELD:
        return "BOTH_HELD";
    default:
        return "UNKNOWN";
    }
}

OperatorSwitch::OperatorSwitch(
    Component *owner,
    ushort _pinStop,
    ushort _pinCycle,
    short _id,
    ushort _modbusAddress)
    : NetworkComponent(_modbusAddress, "OperatorSwitch", _id, Component::COMPONENT_DEFAULT, owner),
      buttons{{_pinStop, false, 0, 0}, {_pinCycle, false, 0, 0}},
      m_stopCoil(this, _id, "StopCoil"),
      m_cycleCoil(this, _id, "CycleCoil"),
      m_state(this, _id, "State")
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
}

short OperatorSwitch::setup()
{
    NetworkComponent::setup();
    for (int i = 0; i < NB_BUTTONS; ++i)
    {
        pinMode(buttons[i].pin, INPUT_PULLUP);
    }

    const uint16_t baseAddr = mb_tcp_base_address();

    m_state.initNotify(State::IDLE, (State)1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_state.initModbus(baseAddr + (ushort)E_MB_Offset::STATE, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, "State", this->name.c_str());
    registerBlock(m_state.getRegisterInfo());

    m_stopCoil.initNotify(false, true, NetworkValue_ThresholdMode::DIFFERENCE);
    m_stopCoil.initModbus(baseAddr + (ushort)E_MB_Offset::COIL_STOP, 1, this->id, this->slaveId, FN_READ_COIL, "StopCoil", this->name.c_str());
    registerBlock(m_stopCoil.getRegisterInfo());

    m_cycleCoil.initNotify(false, true, NetworkValue_ThresholdMode::DIFFERENCE);
    m_cycleCoil.initModbus(baseAddr + (ushort)E_MB_Offset::COIL_CYCLE, 1, this->id, this->slaveId, FN_READ_COIL, "CycleCoil", this->name.c_str());
    registerBlock(m_cycleCoil.getRegisterInfo());
    return E_OK;
}

unsigned long OperatorSwitch::getPressDuration() const
{
    if (pressStartTime > 0)
    {
        return now - pressStartTime;
    }
    return 0;
}

short OperatorSwitch::loop()
{
    Component::loop();

    // Update individual button states and timestamps
    for (int i = 0; i < NB_BUTTONS; ++i)
    {
        bool currentlyPressed = (digitalRead(buttons[i].pin) == LOW);
        if (currentlyPressed && !buttons[i].pressed)
        {
            buttons[i].pressTime = now;
        }
        else if (!currentlyPressed && buttons[i].pressed)
        {
            buttons[i].releaseTime = now;
        }
        buttons[i].pressed = currentlyPressed;
    }

    bool stopPressed = buttons[BUTTON_STOP].pressed;
    bool cyclePressed = buttons[BUTTON_CYCLE].pressed;

    m_stopCoil.update(stopPressed);
    m_cycleCoil.update(cyclePressed);

    State rawInputState = State::IDLE;
    if (stopPressed && cyclePressed)
    {
        rawInputState = State::BOTH_PRESSED;
    }
    else if (stopPressed)
    {
        rawInputState = State::STOP_PRESSED;
    }
    else if (cyclePressed)
    {
        rawInputState = State::CYCLE_PRESSED;
    }

    State currentState = m_state.getValue();
    State newState = currentState;

    unsigned long pressDuration = 0;

    switch (currentState)
    {
    case State::IDLE:
        if (rawInputState != State::IDLE)
        {
            unsigned long mostRecentPressTime = 0;
            if (stopPressed)
                mostRecentPressTime = max(mostRecentPressTime, buttons[BUTTON_STOP].pressTime);
            if (cyclePressed)
                mostRecentPressTime = max(mostRecentPressTime, buttons[BUTTON_CYCLE].pressTime);

            if ((now - mostRecentPressTime >= PRESS_TIME_MS))
            {
                newState = rawInputState;
                pressStartTime = mostRecentPressTime;
                holdEventTriggered = false;
            }
        }
        break;

    case State::STOP_PRESSED:
    case State::CYCLE_PRESSED:
    case State::BOTH_PRESSED:
        if (rawInputState == State::IDLE)
        {
            newState = State::IDLE;
            pressStartTime = 0;
        }
        else if (rawInputState != currentState)
        {
            newState = State::IDLE; // Reset and re-evaluate
            pressStartTime = 0;
        }
        else if (!holdEventTriggered && (now - pressStartTime >= HOLD_TIME_MS))
        {
            if (currentState == State::STOP_PRESSED)
                newState = State::STOP_HELD;
            else if (currentState == State::CYCLE_PRESSED)
                newState = State::CYCLE_HELD;
            else if (currentState == State::BOTH_PRESSED)
                newState = State::BOTH_HELD;

            if (newState != currentState)
            {
                holdEventTriggered = true;
            }
        }
        break;

    case State::STOP_HELD:
        if (!stopPressed)
            newState = State::IDLE;
        break;
    case State::CYCLE_HELD:
        if (!cyclePressed)
            newState = State::IDLE;
        break;
    case State::BOTH_HELD:
        if (!stopPressed || !cyclePressed)
            newState = State::IDLE;
        break;
    default:
        break;
    }

    if (newState != currentState)
    {
        m_state.update(newState);
        notifyStateChange();
    }

    return E_OK;
}

short OperatorSwitch::info(short val0, short val1)
{
    Log.traceln(F("OperatorSwitch Info: State=%d, StopPin=%d, CyclePin=%d, Duration=%lums"),
                (ushort)getState(), m_stopCoil.getValue(), m_cycleCoil.getValue(), getPressDuration());
    return E_OK;
}

void OperatorSwitch::notifyStateChange()
{
    Component::notifyStateChange();
}

short OperatorSwitch::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    return E_INVALID_PARAMETER; // All registers are read-only from Modbus perspective
}

short OperatorSwitch::mb_tcp_read(MB_Registers *reg)
{
    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED)
    {
        return result;
    }

    uint16_t address = reg->startAddress;

    if (address == (_baseAddress + (ushort)E_MB_Offset::STATE))
    {
        return (ushort)m_state.getValue();
    }
    else if (address == (_baseAddress + (ushort)E_MB_Offset::COIL_STOP))
    {
        return m_stopCoil.getValue() ? 1 : 0;
    }
    else if (address == (_baseAddress + (ushort)E_MB_Offset::COIL_CYCLE))
    {
        return m_cycleCoil.getValue() ? 1 : 0;
    }
    return 0;
}

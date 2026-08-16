#include <App.h>
#include <Bridge.h>
#include <modbus/Modbus.h>

#include "./PushButton.h"

// Helper function to convert State enum to string for logging
const char *stateToString(PushButton::State state)
{
    switch (state)
    {
    case PushButton::State::IDLE:
        return "IDLE";
    case PushButton::State::PRESSED:
        return "PRESSED";
    case PushButton::State::HELD:
        return "HELD";
    default:
        return "UNKNOWN";
    }
}

PushButton::PushButton(
    Component *owner,
    ushort _pin,
    short _id,
    ushort _modbusAddress)
    : NetworkComponent(_modbusAddress, "PushButton", _id, Component::COMPONENT_DEFAULT, owner),
      button{_pin, false, 0},
      m_state(this, _id, "State")
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
}

short PushButton::setup()
{
    NetworkComponent::setup();

    pinMode(button.pin, INPUT_PULLUP);

    const uint16_t baseAddr = mb_tcp_base_address();

    m_state.initNotify(State::IDLE, (State)1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_state.initModbus(baseAddr + (ushort)E_MB_Offset::STATE, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, "State", this->name.c_str());
    registerBlock(m_state.getRegisterInfo());

    return E_OK;
}

unsigned long PushButton::getPressDuration() const
{
    if (pressStartTime > 0)
    {
        return now - pressStartTime;
    }
    return 0;
}

short PushButton::loop()
{
    Component::loop();

    // Update button state
    bool currentlyPressed = (digitalRead(button.pin) == LOW);
    if (currentlyPressed && !button.pressed)
    {
        button.pressTime = now;
    }
    button.pressed = currentlyPressed;

    State currentState = m_state.getValue();
    State newState = currentState;

    switch (currentState)
    {
    case State::IDLE:
        if (currentlyPressed)
        {
            if ((now - button.pressTime >= PRESS_TIME_MS))
            {
                newState = State::PRESSED;
                pressStartTime = button.pressTime;
                holdEventTriggered = false;
            }
        }
        break;

    case State::PRESSED:
        if (!currentlyPressed)
        {
            newState = State::IDLE;
            pressStartTime = 0;
        }
        else if (!holdEventTriggered && (now - pressStartTime >= HOLD_TIME_MS))
        {
            newState = State::HELD;
            holdEventTriggered = true;
        }
        break;

    case State::HELD:
        if (!currentlyPressed)
        {
            newState = State::IDLE;
            pressStartTime = 0;
        }
        break;
    }

    if (newState != currentState)
    {
        m_state.update(newState, E_PRIORITY::E_PRIORITY_HIGHEST);
        Log.verboseln("PushButton::loop, state: %s", stateToString(m_state.getValue()));
        notifyStateChange();
    }

    return E_OK;
}

short PushButton::info(short val0, short val1)
{
    return E_OK;
}

void PushButton::notifyStateChange()
{
    Component::notifyStateChange();
}

short PushButton::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    return E_INVALID_PARAMETER; // All registers are read-only from Modbus perspective
}

short PushButton::mb_tcp_read(MB_Registers *reg)
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
    return 0;
}

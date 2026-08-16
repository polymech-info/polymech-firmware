#include <App.h>
#include <Bridge.h>
#include <modbus/Modbus.h>

#include "./OperatorSwitch.h"

OperatorSwitch::OperatorSwitch(
    Component *owner,
    ushort _pinStop,
    ushort _pinCycle,
    short _id,
    ushort _modbusAddress)
    : Component("OperatorSwitch", _id, Component::COMPONENT_DEFAULT, owner),
      pinStop(_pinStop),
      pinCycle(_pinCycle),
      modbusAddr(_modbusAddress),
      currentState(State::IDLE),
      lastState(State::IDLE)
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

    modbusBlocks[0] = INIT_MODBUS_BLOCK_TCP(
        this->modbusAddr, 
        (ushort)E_MB_Offset::STATE, 
        E_FN_CODE::FN_READ_HOLD_REGISTER, 
        MB_ACCESS_READ_ONLY,
        "State", 
        "OperatorSwitch"
    );

    modbusView.data = modbusBlocks;
    modbusView.count = (ushort)E_MB_Offset::COUNT;
}

short OperatorSwitch::setup()
{
    Component::setup();
    pinMode(pinStop, INPUT_PULLUP);
    pinMode(pinCycle, INPUT_PULLUP);
    return E_OK;
}

short OperatorSwitch::loop()
{
    Component::loop();

    if (now - lastReadMs < READ_INTERVAL_MS) {
        return E_OK;
    }
    lastReadMs = now;

    State rawState = readPins();

    // Debouncing logic
    if (rawState != proposedState) {
        proposedState = rawState;
        confirmCount = 0;
    } else {
        confirmCount++;
    }

    if (confirmCount >= DEBOUNCE_COUNT) {
        // A stable state is confirmed
        if (currentState != rawState) {
            lastState = currentState;
            currentState = rawState;
            pressStartTime = now;
            holdEventTriggered = false;
            notifyStateChange();
        }

        // Hold detection logic
        if (currentState != State::IDLE && !holdEventTriggered) {
            if (getPressDuration() >= HOLD_TIME_MS) {
                lastState = currentState;
                if (currentState == State::STOP_PRESSED) {
                    currentState = State::STOP_HELD;
                } else if (currentState == State::CYCLE_PRESSED) {
                    currentState = State::CYCLE_HELD;
                }
                holdEventTriggered = true;
                notifyStateChange();
            }
        }
    }
    return E_OK;
}

OperatorSwitch::State OperatorSwitch::readPins()
{
    bool stopPressed = (digitalRead(pinStop) == LOW);
    bool cyclePressed = (digitalRead(pinCycle) == LOW);

    if (stopPressed) return State::STOP_PRESSED;
    if (cyclePressed) return State::CYCLE_PRESSED;
    
    return State::IDLE;
}

unsigned long OperatorSwitch::getPressDuration() const
{
    if (currentState == State::IDLE) {
        return 0;
    }
    return now - pressStartTime;
}


void OperatorSwitch::notifyStateChange()
{
    Component::notifyStateChange();
    Log.verboseln("OperatorSwitch State Change: %s -> %s", C_STR(lastState), C_STR(currentState));
    // Here you can send messages to the owner/app if needed
    // owner->onMessage(id, verb, flags, user, this);
}


short OperatorSwitch::info(short val0, short val1)
{
    Log.verboseln("OperatorSwitch::info - ID: %d, State: %d, LastState: %d, PressDuration: %lu ms",
                  id, (ushort)currentState, (ushort)lastState, getPressDuration());
    return E_OK;
}


short OperatorSwitch::mb_tcp_read(MB_Registers *reg)
{
    if (reg->startAddress == modbusAddr + (ushort)E_MB_Offset::STATE) {
        return (short)currentState;
    }
    return 0;
}

short OperatorSwitch::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    // Write operations are not supported for this component
    return E_INVALID_PARAMETER;
}

void OperatorSwitch::mb_tcp_register(ModbusTCP *manager)
{
    ModbusBlockView *blocksView = mb_tcp_blocks();
    Component *thiz = const_cast<OperatorSwitch *>(this);
    for (int i = 0; i < blocksView->count; ++i) {
        MB_Registers info = blocksView->data[i];
        manager->registerModbus(thiz, info);
    }
}

ModbusBlockView *OperatorSwitch::mb_tcp_blocks() const
{
    return const_cast<ModbusBlockView *>(&modbusView);
}

short OperatorSwitch::serial_register(Bridge *bridge)
{
    Component::serial_register(bridge);
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&OperatorSwitch::info);
    return E_OK;
} 
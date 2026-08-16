#ifndef OPERATOR_SWITCH_H
#define OPERATOR_SWITCH_H

#include <ArduinoLog.h>
#include "config.h"
#include <App.h>
#include <Component.h>
#include <modbus/ModbusTCP.h> 
#include "config-modbus.h"
#include <stdint.h>

class Bridge;

class OperatorSwitch : public Component
{
public:
    enum class State : ushort {
        IDLE = 0,
        STOP_PRESSED,
        CYCLE_PRESSED,
        STOP_HELD,
        CYCLE_HELD,
        UNKNOWN
    };

    enum class E_MB_Offset : ushort {
        STATE = 0,
        COUNT
    };

private:
    const ushort pinStop;
    const ushort pinCycle;
    const ushort modbusAddr;
    
    State currentState;
    State lastState;
    
    // Timing for hold detection
    unsigned long pressStartTime = 0;
    bool holdEventTriggered = false;

    // Debouncing data
    unsigned long lastReadMs = 0;
    State proposedState = State::IDLE;
    ushort confirmCount = 0;
    bool useDebouncing = true;

    // Modbus definitions
    MB_Registers modbusBlocks[(ushort)E_MB_Offset::COUNT];
    ModbusBlockView modbusView;

    // Private helpers
    State readPins();

public:
    OperatorSwitch(
        Component *owner,
        ushort _pinStop,
        ushort _pinCycle,
        short _id,
        ushort _modbusAddress);

    short setup() override;
    short loop() override;
    short info(short val0 = 0, short val1 = 0) override;
    short debug() override { return info(0, 0); }

    State getState() const { return currentState; }
    ushort getValue() const { return static_cast<ushort>(getState()); }
    unsigned long getPressDuration() const;

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;
    void mb_tcp_register(ModbusTCP *manager) override;
    ModbusBlockView *mb_tcp_blocks() const override;
    short serial_register(Bridge *bridge) override;

protected:
    void notifyStateChange() override;

    // Configuration
    static constexpr ushort DEBOUNCE_COUNT = 3;
    static constexpr ushort READ_INTERVAL_MS = 25;
    static constexpr ushort HOLD_TIME_MS = 1500;
};

#endif // OPERATOR_SWITCH_H 
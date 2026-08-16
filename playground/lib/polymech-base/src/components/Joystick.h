#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <ArduinoLog.h>
#include "config.h"
#include <App.h>
#include <Component.h>
#include <modbus/ModbusTCP.h> 
#include "config-modbus.h"
#include <stdint.h>

class Bridge;

class Joystick : public Component
{
public:
    enum class E_POSITION : ushort {
        CENTER = 0,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        UNKNOWN
    };

    enum class E_MODE : ushort {
        LOCAL = 0,
        REMOTE = 1
    };

    enum class E_REGISTER : ushort {
        POSITION = 0,
        MODE = 1,
        OVERRIDE = 2
    };

private:
    const ushort pinUp;
    const ushort pinDown;
    const ushort pinLeft; 
    const ushort pinRight;
    const ushort modbusAddr;
    
    E_POSITION currentPosition;
    E_POSITION lastPosition;
    E_MODE mode;
    E_POSITION overridePosition;
    
    // Position timing
    unsigned long positionStartMs = 0;
    
    // Debouncing data
    unsigned long lastReadMs = 0;
    E_POSITION proposedPosition = E_POSITION::CENTER;
    ushort confirmCount = 0;
    bool useDebouncing = true;

    // Modbus definitions
    MB_Registers modbusBlocks[3];
    ushort modbusBlockCount = 0;
    ModbusBlockView modbusView;

    // Private helpers
    E_POSITION readPinsPosition();

public:
    Joystick(
        Component *owner,
        ushort _pinUp,
        ushort _pinDown,
        ushort _pinLeft,
        ushort _pinRight, 
        ushort _modbusAddress = 100);

    short setup() override;
    short loop() override;
    short reset();
    short info(short val0 = 0, short val1 = 0) override;
    short debug() override { return info(0, 0); }

    E_POSITION getPosition() const { return mode == E_MODE::LOCAL ? currentPosition : overridePosition; }
    ushort getValue() const { return static_cast<ushort>(getPosition()); }
    E_POSITION getLastPosition() const { return lastPosition; }
    unsigned long getHoldingTime() const { return now - positionStartMs; }
    E_MODE getMode() const { return mode; }
    
    void setMode(E_MODE _mode) { mode = _mode; notifyStateChange(); }
    void setOverridePosition(E_POSITION pos) { overridePosition = pos; if (mode == E_MODE::REMOTE) notifyStateChange(); }

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
};

#endif // JOYSTICK_H 
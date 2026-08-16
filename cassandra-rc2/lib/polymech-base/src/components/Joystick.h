#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <ArduinoLog.h>
#include "config.h"
#include <App.h>
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"
#include "config-modbus.h"
#include <stdint.h>

#define JOYSTICK_MB_COUNT 4 // m_enabled + position, mode, override

class Bridge;

class Joystick : public NetworkComponent<JOYSTICK_MB_COUNT>
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

    enum E_MB_Offset : ushort {
        POSITION = E_NVC_USER + 0,
        MODE = E_NVC_USER + 1,
        OVERRIDE = E_NVC_USER + 2
    };

private:
    const ushort pinUp;
    const ushort pinDown;
    const ushort pinLeft; 
    const ushort pinRight;
    
    E_POSITION currentPosition;
    E_POSITION lastPosition;
    
    // Position timing
    unsigned long positionStartMs = 0;
    
    // Debouncing data
    unsigned long lastReadMs = 0;
    E_POSITION proposedPosition = E_POSITION::CENTER;
    ushort confirmCount = 0;
    bool useDebouncing = true;

    // Network Values
    NetworkValue<E_POSITION> m_position;
    NetworkValue<E_MODE> m_mode;
    NetworkValue<E_POSITION> m_overridePosition;

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

    E_POSITION getPosition() const { return m_mode.getValue() == E_MODE::LOCAL ? currentPosition : m_overridePosition.getValue(); }
    ushort getValue() const { return static_cast<ushort>(getPosition()); }
    E_POSITION getLastPosition() const { return lastPosition; }
    unsigned long getHoldingTime() const { return now - positionStartMs; }
    E_MODE getMode() const { return m_mode.getValue(); }
    
    void setMode(E_MODE _mode) { m_mode.update(_mode); }
    void setOverridePosition(E_POSITION pos) { m_overridePosition.update(pos); }

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;
    short serial_register(Bridge *bridge) override;

protected:
    void notifyStateChange() override;

    // Configuration
    static constexpr ushort DEBOUNCE_COUNT = 3;
    static constexpr ushort READ_INTERVAL_MS = 25;
};

#endif // JOYSTICK_H 
#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <ArduinoLog.h>
#include "config.h"
#include <App.h>
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"
#include "config-modbus.h"
#include <stdint.h>

#define PUSHBUTTON_MB_COUNT 2 // m_enabled + state

class Bridge;

class PushButton : public NetworkComponent<PUSHBUTTON_MB_COUNT>
{
public:
    enum class State : ushort
    {
        IDLE = 0,
        PRESSED,
        HELD
    };

    enum class E_MB_Offset : ushort
    {
        STATE = E_NVC_USER + 1,
    };

private:
    static constexpr ushort NB_BUTTONS = 1;

    struct ButtonInfo
    {
        const ushort pin;
        bool pressed;
        unsigned long pressTime;
    };
    ButtonInfo button;

    NetworkValue<State> m_state;

    // Timing for hold detection
    unsigned long pressStartTime = 0;
    bool holdEventTriggered = false;

public:
    PushButton(
        Component *owner,
        ushort _pin,
        short _id,
        ushort _modbusAddress);

    short setup() override;
    short loop() override;
    short info(short val0 = 0, short val1 = 0) override;
    short debug() override { return info(0, 0); }

    State getState() const { return m_state.getValue(); }
    ushort getValue() const { return static_cast<ushort>(getState()); }
    unsigned long getPressDuration() const;

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;

protected:
    void notifyStateChange() override;

    // Configuration
    static constexpr ushort PRESS_TIME_MS = 50;
    static constexpr ushort HOLD_TIME_MS = 1500;
};

#endif // PUSHBUTTON_H
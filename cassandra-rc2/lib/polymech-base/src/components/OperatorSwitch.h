#ifndef OPERATOR_SWITCH_H
#define OPERATOR_SWITCH_H

#include <ArduinoLog.h>
#include "config.h"
#include <App.h>
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"
#include "config-modbus.h"
#include <stdint.h>

#define OPERATOR_SWITCH_MB_COUNT 4 // m_enabled + state, stop_coil, cycle_coil

class Bridge;

class OperatorSwitch : public NetworkComponent<OPERATOR_SWITCH_MB_COUNT>
{
public:
    enum class State : ushort
    {
        IDLE = 0,
        STOP_PRESSED,
        CYCLE_PRESSED,
        BOTH_PRESSED,
        STOP_HELD,
        CYCLE_HELD,
        BOTH_HELD,
        UNKNOWN
    };

    enum class E_MB_Offset : ushort
    {
        STATE = E_NVC_USER + 0,
        COIL_STOP,
        COIL_CYCLE,
    };

private:
    static constexpr ushort NB_BUTTONS = 2;
    enum ButtonIndex
    {
        BUTTON_STOP = 0,
        BUTTON_CYCLE
    };
    struct ButtonInfo
    {
        const ushort pin;
        bool pressed;
        unsigned long pressTime;
        unsigned long releaseTime;
    };
    ButtonInfo buttons[NB_BUTTONS];

    bool stateOverride = false;

    NetworkValue<bool> m_stopCoil;
    NetworkValue<bool> m_cycleCoil;
    NetworkValue<State> m_state;

    // Timing for hold detection
    unsigned long pressStartTime = 0;
    bool holdEventTriggered = false;

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

    State getState() const { return m_state.getValue(); }
    ushort getValue() const { return static_cast<ushort>(getState()); }
    unsigned long getPressDuration() const;

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;

protected:
    void notifyStateChange() override;

    // Configuration
    static constexpr ushort PRESS_TIME_MS = 80;
    static constexpr ushort HOLD_TIME_MS = 2500;
};

#endif // OPERATOR_SWITCH_H
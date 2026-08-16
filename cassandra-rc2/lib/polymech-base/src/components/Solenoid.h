#ifndef SOLENOID_H
#define SOLENOID_H

#include <ArduinoLog.h>
#include <App.h>
#include <enums.h>
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"
#include "StringUtils.h"
#include "config.h"
#include "config-modbus.h"


#define SOLENOID_MB_COUNT 2 // m_enabled, m_state

using SolenoidValue = NetworkValue<bool>;

class Solenoid : public NetworkComponent<SOLENOID_MB_COUNT>
{
public:
    enum E_MB_Offset {
        MB_OFS_COIL_STATE = E_NVC_USER + 0,
    };
private:
    unsigned long lastOnTime;
    unsigned long lastOffTime;
    uint32_t lastOnDurationS;
    uint32_t lastOffDurationS;
    uint32_t _activation_count;

public:
    const short pin;
    SolenoidValue m_state;
    XString<100> stringTest;

    Solenoid(
        Component *owner,
        short _pin,
        short _id,
        short _modbusAddress);

    short setup() override;
    short setValue(bool newValue);
    short setValueCmd(short arg1, short arg2);
    bool getValue() const;

    uint32_t getActivationCount() const;
    void resetActivationCount();

    uint32_t getCurrentOnDurationMs() const;

    uint32_t getLastOnDurationS() const { return lastOnDurationS; }
    uint32_t getLastOffDurationS() const { return lastOffDurationS; }

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;
    short loop() override;
};

#endif 
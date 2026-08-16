#ifndef RELAY_H
#define RELAY_H

#include <ArduinoLog.h>
#include <App.h>
#include <enums.h>
#include "config.h"
#include "config-modbus.h"
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"

#define RELAY_MB_COUNT 3 // m_enabled, m_state, m_frequency

class Bridge;
class Relay : public NetworkComponent<RELAY_MB_COUNT>
{
public:
    enum E_MB_Offset {
        MB_OFS_COIL_STATE = E_NVC_USER + 0,
        MB_OFS_HR_FREQUENCY = E_NVC_USER + 1,
    };

private:
    unsigned long lastToggleTime;

public:
    const short pin;
    NetworkValue<bool> m_state;
    NetworkValue<uint32_t> m_frequency;
    
    Relay(
        Component *owner,
        short _pin,
        short _id,
        short _modbusAddress);

    short setup() override;
    short setValue(bool newValue);
    short setValueCmd(short arg1, short arg2);
    bool getValue() const;

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;
    short loop() override;
};

#endif

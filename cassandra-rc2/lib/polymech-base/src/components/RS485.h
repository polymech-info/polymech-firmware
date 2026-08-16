#ifndef RS485_H
#define RS485_H

#include <Component.h>
#include <ArduinoLog.h>
#include <ModbusClientRTU.h>
#include <modbus/ModbusRTU.h>
#include <modbus/ModbusTypes.h>

#include "config-modbus.h" // application modbus config

class ModbusTCP;
class ModbusBlockView;

class RS485 : public Component
{
public:
    RS485(Component *owner);
    virtual ~RS485();

    short setup() override;
    short loop() override;
    short onMessage(int originId, E_CALLS reason, E_MessageFlags flags, void *data = nullptr, Component *sender = nullptr) override;

    short mb_tcp_read(short address) override;
    short mb_tcp_write(short address, short value) override;

    short mb_tcp_read(MB_Registers *reg);
    short mb_tcp_write(MB_Registers *reg, short value);
    ushort mb_tcp_error(MB_Registers *reg);
    void mb_tcp_register(ModbusTCP *manager) override;

    ModbusRTU modbus;      // RTU Master instance
    Manager deviceManager; // Manages RTU slave devices
    ModbusTCP *manager;

private:
    static RS485 *instance;
    static void staticRtuRegisterChangeCallback(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue);
    void handleRtuRegisterChange(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue);
    unsigned long lastLoopTime = 0;
};
#endif // RS485_H
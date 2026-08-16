#ifndef NETWORK_VALUE_TEST_H
#define NETWORK_VALUE_TEST_H

#include <Component.h>
#include <NetworkValue.h>
#include <array>
#include <modbus/ModbusTypes.h>
#include <components/commons.h>
#include <modbus/NetworkComponent.h>

class ModbusTCP;

// Define a type alias for the specific compositions we need.
using BoolNetValue = NetworkValue<bool>;
using IntNetValue = NetworkValue<uint16_t>;
using IntArrayNetValue = NetworkValue<std::array<uint16_t, 5>>;

#define NUM_NV_TEST_VALUES 3
#define NETWORK_VALUE_TEST_NC 1
#define NV_TEST_ARRAY_SIZE 5

class NetworkValueTest : public maybe<NETWORK_VALUE_TEST_NC, NetworkComponent<3>> {
public:
    NetworkValueTest(Component* owner, ushort id, ushort modbusBaseAddress);
    virtual ~NetworkValueTest() = default;
    

    short setup() override;
    void init();
    short loop() override;

    // Modbus
    void mb_tcp_register(ModbusTCP* manager) override;
    ModbusBlockView* mb_tcp_blocks() const override;
    short mb_tcp_read(MB_Registers *reg) override;
    short mb_tcp_write(MB_Registers *reg, short value) override;
    uint16_t mb_tcp_base_address() const override;
    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user, Component *src) override;

private:
    BoolNetValue m_boolValue;
    IntNetValue m_intValue;
    IntArrayNetValue m_arrayValue;

    uint32_t m_lastToggleMs;
    
    ushort m_modbusBaseAddress;
    mutable MB_Registers m_modbusBlocks[NUM_NV_TEST_VALUES];
    mutable ModbusBlockView m_modbusBlockView;
};

#endif // NETWORK_VALUE_TEST_H 
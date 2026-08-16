#ifndef NETWORK_VALUE_TEST_PB_H
#define NETWORK_VALUE_TEST_PB_H

#include <Component.h>
#include <NetworkValue.h>
#include <array>
#include <modbus/ModbusTypes.h>
#include <components/commons.h>
#include <net/NetworkComponent.h>

#define NUM_NV_TEST_PB_VALUES 5 // 1 for m_enabled + 4 custom
#define NV_TEST_PB_ARRAY_SIZE 5

#ifndef ENABLE_MODBUS_FOR_NETWORKVALUETESTPB
    #define ENABLE_MODBUS_FOR_NETWORKVALUETESTPB 0
#endif

#ifndef ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB
    #define ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB 0
#endif

#if (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 1)
    #include <net/NC_ModbusTCP.h>
#endif
#if (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 1)
    #include <net/NC_ModbusRTU.h>
#endif

// Base NComponent
using BaseNComponent = NComponent<NUM_NV_TEST_PB_VALUES>;

// Conditionally apply mixins
#if (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 1) && (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 1)
    using NetworkValueTestPBBase = WithModbusRTU<WithModbus<BaseNComponent, NUM_NV_TEST_PB_VALUES>, NUM_NV_TEST_PB_VALUES>;
#elif (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 1)
    using NetworkValueTestPBBase = WithModbus<BaseNComponent, NUM_NV_TEST_PB_VALUES>;
#elif (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 1)
    using NetworkValueTestPBBase = WithModbusRTU<BaseNComponent, NUM_NV_TEST_PB_VALUES>;
#else
    using NetworkValueTestPBBase = BaseNComponent;
#endif


// Enable Protobuf for NetworkValue
#ifndef NETWORKVALUE_ENABLE_PROTOBUF
    #define NETWORKVALUE_ENABLE_PROTOBUF 1
#endif

// Define a type alias for the specific compositions we need.
using BoolNetValue = NetworkValue<bool>;
using IntNetValue = NetworkValue<int>;
using IntArrayNetValue = NetworkValue<std::array<int, 5>>;
using BoolArrayNetValue = NetworkValue<std::array<bool, 5>>;

class NetworkValueTestPB : public NetworkValueTestPBBase {
public:
    NetworkValueTestPB(Component* owner, ushort id, ushort modbusBaseAddress);
    virtual ~NetworkValueTestPB() = default;
    
    short setup() override;
    short loopNetwork() override;

#if (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 1)
    short mb_tcp_read(MB_Registers *reg) override;
    short mb_tcp_write(MB_Registers *reg, short value) override;
#endif

#if (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 1)
    short mb_rtu_read(MB_Registers *reg) override;
    short mb_rtu_write(MB_Registers *reg, short value) override;
#endif

private:
    BoolNetValue m_boolValue;
    IntNetValue m_intValue;
    IntArrayNetValue m_arrayValue;
    BoolArrayNetValue m_boolArrayValue;

    uint32_t m_lastToggleMs;
};

#endif // NETWORK_VALUE_TEST_PB_H 
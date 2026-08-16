#include "NetworkValueTestPB.h"
#include "modbus/ModbusTCP.h"
#include "enums.h"
#include "error_codes.h"
#include "config.h"

// Bring in the Protobuf feature
#ifdef ENABLE_NETWORK_VALUE_TEST_PB

#include "NetworkValuePB.h"

// Local offsets, starting from the USER position defined in the base class
#define NV_TEST_BOOL_OFFSET E_NVC_USER + 0
#define NV_TEST_INT_OFFSET E_NVC_USER + 1
#define NV_TEST_ARRAY_OFFSET E_NVC_USER + 2
#define NV_TEST_BOOL_ARRAY_OFFSET E_NVC_USER + 7 // size of int array is 5, so 2+5=7

NetworkValueTestPB::NetworkValueTestPB(Component* owner, ushort id, ushort modbusBaseAddress)
#if (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 1) && (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 1)
    : NetworkValueTestPBBase(modbusBaseAddress, modbusBaseAddress, "NV_TestPB", id, COMPONENT_DEFAULT, owner),
#elif (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 1) || (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 1)
    : NetworkValueTestPBBase(modbusBaseAddress, "NV_TestPB", id, COMPONENT_DEFAULT, owner),
#else
    : NetworkValueTestPBBase("NV_TestPB", id, COMPONENT_DEFAULT, owner),
#endif
      m_boolValue(this, id, "NV_Bool_PB", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
      m_intValue(this, id, "NV_Int_PB", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
      m_arrayValue(this, id, "NV_Array_PB", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
      m_boolArrayValue(this, id, "NV_BoolArray_PB", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
      m_lastToggleMs(0)
{
    addNetworkValue(&m_boolValue);
    addNetworkValue(&m_intValue);
    addNetworkValue(&m_arrayValue);
    addNetworkValue(&m_boolArrayValue);
}

short NetworkValueTestPB::setup() {
    NetworkValueTestPBBase::setup(); // Call base class setup

#if (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 1)
    const uint16_t tcpBaseAddr = this->mb_tcp_base_address();
    m_boolValue.initModbus(tcpBaseAddr + NV_TEST_BOOL_OFFSET, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_COIL, "NV Test Bool TCP", this->name.c_str());
    this->registerTCPBlock(m_boolValue.getRegisterInfo());
    m_intValue.initModbus(tcpBaseAddr + NV_TEST_INT_OFFSET, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_HOLD_REGISTER, "NV Test Int TCP", this->name.c_str());
    this->registerTCPBlock(m_intValue.getRegisterInfo());
    m_arrayValue.initModbus(tcpBaseAddr + NV_TEST_ARRAY_OFFSET, NV_TEST_PB_ARRAY_SIZE, this->id, this->slaveId, E_FN_CODE::FN_WRITE_MULT_REGISTERS, "NV Test Array TCP", this->name.c_str());
    this->registerTCPBlock(m_arrayValue.getRegisterInfo());
    m_boolArrayValue.initModbus(tcpBaseAddr + NV_TEST_BOOL_ARRAY_OFFSET, NV_TEST_PB_ARRAY_SIZE, this->id, this->slaveId, E_FN_CODE::FN_WRITE_MULT_COILS, "NV Test Bool Array TCP", this->name.c_str());
    this->registerTCPBlock(m_boolArrayValue.getRegisterInfo());
#endif

#if (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 1)
    const uint16_t rtuBaseAddr = this->mb_rtu_base_address();
    m_boolValue.initModbus(rtuBaseAddr + NV_TEST_BOOL_OFFSET, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_COIL, "NV Test Bool RTU", this->name.c_str());
    this->registerRTUBlock(m_boolValue.getRegisterInfo());
    m_intValue.initModbus(rtuBaseAddr + NV_TEST_INT_OFFSET, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_HOLD_REGISTER, "NV Test Int RTU", this->name.c_str());
    this->registerRTUBlock(m_intValue.getRegisterInfo());
    m_arrayValue.initModbus(rtuBaseAddr + NV_TEST_ARRAY_OFFSET, NV_TEST_PB_ARRAY_SIZE, this->id, this->slaveId, E_FN_CODE::FN_WRITE_MULT_REGISTERS, "NV Test Array RTU", this->name.c_str());
    this->registerRTUBlock(m_arrayValue.getRegisterInfo());
    m_boolArrayValue.initModbus(rtuBaseAddr + NV_TEST_BOOL_ARRAY_OFFSET, NV_TEST_PB_ARRAY_SIZE, this->id, this->slaveId, E_FN_CODE::FN_WRITE_MULT_COILS, "NV Test Bool Array RTU", this->name.c_str());
    this->registerRTUBlock(m_boolArrayValue.getRegisterInfo());
#endif

#if (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 0) && (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 0)
    m_boolValue.initNotify(false, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_intValue.initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
#endif
    
    // The initNotify for array types is called unconditionally.
    std::array<int, NV_TEST_PB_ARRAY_SIZE> initialArray;
    for (size_t i = 0; i < NV_TEST_PB_ARRAY_SIZE; ++i) {
        initialArray[i] = i;
    }
    m_arrayValue.initNotify(initialArray, initialArray, NetworkValue_ThresholdMode::DIFFERENCE);

    // Configure and register the boolean array NetworkValue
    std::array<bool, NV_TEST_PB_ARRAY_SIZE> initialBoolArray;
    initialBoolArray.fill(false);
    m_boolArrayValue.initNotify(initialBoolArray, initialBoolArray, NetworkValue_ThresholdMode::DIFFERENCE);

    return E_OK;
}

short NetworkValueTestPB::loopNetwork() {
    if (millis() - m_lastToggleMs > 5000) {
        m_lastToggleMs = millis();        
        
        bool newBool = !m_boolValue.getValue();
        if (m_boolValue.update(newBool)) {
           //  L_INFO("NV_TestPB: m_boolValue updated to: %d", newBool);
        }
        
        int newInt = m_intValue.getValue() + 1;
        if (m_intValue.update(newInt, E_PRIORITY::E_PRIORITY_HIGHEST)) {
          //  L_INFO("NV_TestPB: m_intValue updated to: %d", newInt);
        }
        
        std::array<int, NV_TEST_PB_ARRAY_SIZE>& currentArray = m_arrayValue.getValueRef();
        // Increment all elements in the array by 1
        for(size_t i = 0; i < currentArray.size(); ++i) {
           currentArray[i]++;
        }
        
        if (m_arrayValue.update(currentArray)) {
           // L_INFO("NV_TestPB: m_arrayValue updated.");
        }
        
        // Create a new array and populate it with toggled values
        std::array<bool, NV_TEST_PB_ARRAY_SIZE> newBoolArray;
        const auto& currentBoolArray = m_boolArrayValue.getValue();
        for(size_t i = 0; i < NV_TEST_PB_ARRAY_SIZE; ++i) {
           newBoolArray[i] = !currentBoolArray[i];
        }

        if (m_boolArrayValue.update(newBoolArray)) {
           // L_INFO("NV_TestPB: m_boolArrayValue updated.");
        }
    }
    return E_OK;
}

#if (ENABLE_MODBUS_FOR_NETWORKVALUETESTPB == 1)
short NetworkValueTestPB::mb_tcp_read(MB_Registers* reg) {
    short result = NetworkValueTestPBBase::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }

    uint16_t address = reg->startAddress;
    short offset = address - this->mb_tcp_base_address();

    if (offset == NV_TEST_BOOL_OFFSET) return m_boolValue.getValue() ? 1 : 0;
    if (offset == NV_TEST_INT_OFFSET) return m_intValue.getValue();
    
    if (offset >= NV_TEST_ARRAY_OFFSET && offset < (NV_TEST_ARRAY_OFFSET + NV_TEST_PB_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_ARRAY_OFFSET;
        return m_arrayValue.getValue()[array_index];
    }
    if (offset >= NV_TEST_BOOL_ARRAY_OFFSET && offset < (NV_TEST_BOOL_ARRAY_OFFSET + NV_TEST_PB_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_BOOL_ARRAY_OFFSET;
        return m_boolArrayValue.getValue()[array_index] ? 1 : 0;
    }
    
    L_ERROR("NV_TestPB: Read from unhandled address offset: %d", offset);
    return 0xFFFF; // Error
}

short NetworkValueTestPB::mb_tcp_write(MB_Registers* reg, short value) {
    short result = NetworkValueTestPBBase::mb_tcp_write(reg, value);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }
    
    uint16_t address = reg->startAddress;
    short offset = address - this->mb_tcp_base_address();
    
    if (offset == NV_TEST_BOOL_OFFSET) {
        m_boolValue.update(value != 0);
        return E_OK;
    }
    if (offset == NV_TEST_INT_OFFSET) {
        m_intValue.update(value);
        return E_OK;
    }
    
    if (offset >= NV_TEST_ARRAY_OFFSET && offset < (NV_TEST_ARRAY_OFFSET + NV_TEST_PB_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_ARRAY_OFFSET;
        std::array<int, NV_TEST_PB_ARRAY_SIZE> currentArray = m_arrayValue.getValue();
        currentArray[array_index] = value;
        m_arrayValue.update(currentArray);
        return E_OK;
    }
    if (offset >= NV_TEST_BOOL_ARRAY_OFFSET && offset < (NV_TEST_BOOL_ARRAY_OFFSET + NV_TEST_PB_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_BOOL_ARRAY_OFFSET;
        std::array<bool, NV_TEST_PB_ARRAY_SIZE> currentBoolArray = m_boolArrayValue.getValue();
        currentBoolArray[array_index] = (value != 0);
        m_boolArrayValue.update(currentBoolArray);
        return E_OK;
    }
    
    L_ERROR("NV_TestPB: Write to unhandled address offset: %d", offset);
    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
} 
#endif

#if (ENABLE_MODBUSRTU_FOR_NETWORKVALUETESTPB == 1)
short NetworkValueTestPB::mb_rtu_read(MB_Registers* reg) {
    short result = NetworkValueTestPBBase::mb_rtu_read(reg);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }

    uint16_t address = reg->startAddress;
    short offset = address - this->mb_rtu_base_address();

    if (offset == NV_TEST_BOOL_OFFSET) return m_boolValue.getValue() ? 1 : 0;
    if (offset == NV_TEST_INT_OFFSET) return m_intValue.getValue();
    
    if (offset >= NV_TEST_ARRAY_OFFSET && offset < (NV_TEST_ARRAY_OFFSET + NV_TEST_PB_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_ARRAY_OFFSET;
        return m_arrayValue.getValue()[array_index];
    }
    if (offset >= NV_TEST_BOOL_ARRAY_OFFSET && offset < (NV_TEST_BOOL_ARRAY_OFFSET + NV_TEST_PB_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_BOOL_ARRAY_OFFSET;
        return m_boolArrayValue.getValue()[array_index] ? 1 : 0;
    }
    
    L_ERROR("NV_TestPB: RTU Read from unhandled address offset: %d", offset);
    return 0xFFFF; // Error
}

short NetworkValueTestPB::mb_rtu_write(MB_Registers* reg, short value) {
    short result = NetworkValueTestPBBase::mb_rtu_write(reg, value);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }
    
    uint16_t address = reg->startAddress;
    short offset = address - this->mb_rtu_base_address();
    
    if (offset == NV_TEST_BOOL_OFFSET) {
        m_boolValue.update(value != 0);
        return E_OK;
    }
    if (offset == NV_TEST_INT_OFFSET) {
        m_intValue.update(value);
        return E_OK;
    }
    
    if (offset >= NV_TEST_ARRAY_OFFSET && offset < (NV_TEST_ARRAY_OFFSET + NV_TEST_PB_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_ARRAY_OFFSET;
        std::array<int, NV_TEST_PB_ARRAY_SIZE> currentArray = m_arrayValue.getValue();
        currentArray[array_index] = value;
        m_arrayValue.update(currentArray);
        return E_OK;
    }
    if (offset >= NV_TEST_BOOL_ARRAY_OFFSET && offset < (NV_TEST_BOOL_ARRAY_OFFSET + NV_TEST_PB_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_BOOL_ARRAY_OFFSET;
        std::array<bool, NV_TEST_PB_ARRAY_SIZE> currentBoolArray = m_boolArrayValue.getValue();
        currentBoolArray[array_index] = (value != 0);
        m_boolArrayValue.update(currentBoolArray);
        return E_OK;
    }
    
    L_ERROR("NV_TestPB: RTU Write to unhandled address offset: %d", offset);
    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
}
#endif

#endif

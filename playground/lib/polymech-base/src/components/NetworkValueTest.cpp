#include "NetworkValueTest.h"
#include "modbus/ModbusTCP.h"
#include "enums.h"
#include "error_codes.h"

#define NV_TEST_BOOL_OFFSET 0
#define NV_TEST_INT_OFFSET 1
#define NV_TEST_ARRAY_OFFSET 2
#define NV_TEST_ARRAY_SIZE 5

NetworkValueTest::NetworkValueTest(Component* owner, ushort id, ushort modbusBaseAddress)
    : NetworkComponent<3>("NV_Test", id, COMPONENT_DEFAULT, owner),
      INIT_NETWORK_VALUE(m_boolValue, "NV_Bool"),
      INIT_NETWORK_VALUE(m_intValue, "NV_Int"),
      INIT_NETWORK_VALUE(m_arrayValue, "NV_Array"),
      m_lastToggleMs(0),
      m_modbusBaseAddress(modbusBaseAddress)
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    init();
}

void NetworkValueTest::init()
{
    // Configure the features for the boolean NetworkValue
    m_boolValue.initNotify(
        false, 1, NetworkValue_ThresholdMode::DIFFERENCE,
        [](const bool& oldValue, const bool& newValue) {
            
        }
    );
    m_boolValue.initModbus(
        m_modbusBaseAddress + NV_TEST_BOOL_OFFSET, 1, this->id, this->slaveId,
        E_FN_CODE::FN_WRITE_COIL, "NV Test Bool", this->name.c_str()
    );

    // Configure the features for the integer NetworkValue
    m_intValue.initNotify(
        0, 1, NetworkValue_ThresholdMode::DIFFERENCE,
        [](const uint16_t& oldValue, const uint16_t& newValue) {
           // Log.infoln("  [CB] Int changed from %d to %d", oldValue, newValue);
        }
    );
    m_intValue.initModbus(
        m_modbusBaseAddress + NV_TEST_INT_OFFSET, 1, this->id, this->slaveId,
        E_FN_CODE::FN_WRITE_HOLD_REGISTER, "NV Test Int", this->name.c_str()
    );

    // Configure the features for the array NetworkValue
    std::array<uint16_t, NV_TEST_ARRAY_SIZE> initialArray;
    initialArray.fill(0);
    m_arrayValue.initNotify(
        initialArray, initialArray, NetworkValue_ThresholdMode::DIFFERENCE,
        [](const std::array<uint16_t, NV_TEST_ARRAY_SIZE>& oldValue, const std::array<uint16_t, NV_TEST_ARRAY_SIZE>& newValue) {
            // Log.infoln("  [CB] Array changed from %d to %d", oldValue[0], newValue[0]);
        }
    );
    m_arrayValue.initModbus(
        m_modbusBaseAddress + NV_TEST_ARRAY_OFFSET, NV_TEST_ARRAY_SIZE, this->id, this->slaveId,
        E_FN_CODE::FN_WRITE_MULT_REGISTERS, "NV Test Array", this->name.c_str()
    );    
    // Populate the Modbus block array
    m_modbusBlocks[0] = m_boolValue.getRegisterInfo();
    m_modbusBlocks[1] = m_intValue.getRegisterInfo();
    m_modbusBlocks[2] = m_arrayValue.getRegisterInfo();    
    // Create the view
    m_modbusBlockView = {m_modbusBlocks, NUM_NV_TEST_VALUES};
}


short NetworkValueTest::setup() {
    
    return E_OK;
}

short NetworkValueTest::loop() {
    Component::loop();
    // Every 2 seconds, toggle the boolean and increment the integer and array values
    if (millis() - m_lastToggleMs > 2000) {
        m_lastToggleMs = millis();
        
        bool newBool = !m_boolValue.getValue();
        //Log.infoln("NV_Test: Updating bool to %d", newBool);
        m_boolValue.update(newBool);
        
        uint16_t newInt = m_intValue.getValue() + 1;
        //Log.infoln("NV_Test: Updating int to %d", newInt);
        m_intValue.update(newInt);       
        
        // Modify the array in place
        std::array<uint16_t, NV_TEST_ARRAY_SIZE>& currentArray = m_arrayValue.getValueRef();
        for(size_t i = 0; i < currentArray.size(); ++i) {
           currentArray[i] += (i + 1);
           //Log.infoln("NV_Test: Updating array[%d] to %d", i, currentArray[i]);
        }
        //Log.infoln("NV_Test: Updating array[0] to %d", currentArray[0]);
        m_arrayValue.update(currentArray);
    }

    return E_OK;
}

// Modbus Implementation
uint16_t NetworkValueTest::mb_tcp_base_address() const {
    return m_modbusBaseAddress;
}

ModbusBlockView* NetworkValueTest::mb_tcp_blocks() const {
    return const_cast<ModbusBlockView*>(&m_modbusBlockView);
}

void NetworkValueTest::mb_tcp_register(ModbusTCP* manager) {
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS)) return;

    ModbusBlockView* blocksView = mb_tcp_blocks();
    Component* thiz = const_cast<NetworkValueTest*>(this);

    for (int i = 0; i < blocksView->count; ++i) {
        MB_Registers info = blocksView->data[i];
        manager->registerModbus(thiz, info);
    }
}

short NetworkValueTest::mb_tcp_read(MB_Registers* reg) {
    // Log.infoln("NV_Test: mb_tcp_read called for reg->startAddress: %d, count: %d, name: %s, type: %d", reg->startAddress, reg->count, reg->name, reg->type);
    uint16_t address = reg->startAddress;
    short offset = address - m_modbusBaseAddress;

    if (offset == NV_TEST_BOOL_OFFSET) {
        return m_boolValue.getValue() ? 1 : 0;
    }
    if (offset == NV_TEST_INT_OFFSET) {
        return m_intValue.getValue();
    }
    if (offset >= NV_TEST_ARRAY_OFFSET && offset < (NV_TEST_ARRAY_OFFSET + NV_TEST_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_ARRAY_OFFSET;
        return m_arrayValue.getValue()[array_index];
    }

    Log.errorln("NV_Test: Read from unhandled address offset: %d", offset);
    return 0xFFFF; // Error
}

short NetworkValueTest::mb_tcp_write(MB_Registers* reg, short value) {
    uint16_t address = reg->startAddress;
    short offset = address - m_modbusBaseAddress;
    
    if (offset == NV_TEST_BOOL_OFFSET) {
        m_boolValue.update(value != 0);
        return E_OK;
    }
    if (offset == NV_TEST_INT_OFFSET) {
        m_intValue.update(value);
        return E_OK;
    }
    if (offset >= NV_TEST_ARRAY_OFFSET && offset < (NV_TEST_ARRAY_OFFSET + NV_TEST_ARRAY_SIZE)) {
        int array_index = offset - NV_TEST_ARRAY_OFFSET;
        std::array<uint16_t, NV_TEST_ARRAY_SIZE> currentArray = m_arrayValue.getValue();
        currentArray[array_index] = value;
        m_arrayValue.update(currentArray);
        return E_OK;
    }
    
    Log.errorln("NV_Test: Write to unhandled address offset: %d", offset);
    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
} 

short NetworkValueTest::onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user, Component *src) 
{
    return this->owner->onMessage(id, verb, flags, user, src);
}
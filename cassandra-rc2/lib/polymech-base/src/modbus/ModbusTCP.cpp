#include "ModbusTCP.h"
#include <Component.h>
#include <enums.h>
#include <ArduinoLog.h>
#include <modbus/ModbusTypes.h>
#include "config.h"

#define MODBUS_TCP_SERVER_ID 1
// #define MODBUS_TCP_STRICT


ModbusTCP::ModbusTCP(Component *owner, ModbusServerTCPasync *modbusServerInstance)
    : Component("ModbusTCP", COMPONENT_KEY_MB_Manager, Component::COMPONENT_DEFAULT, owner),
      modbusServer(modbusServerInstance)
{
    addressMappings.setStorage(mappingStorage);
    if (!owner)
    {
        Log.fatalln("ModbusTCP Error: Owner pointer is null. Cannot function without an owner to look up components.");
        disable();
    }
    if (!modbusServer)
    {
        Log.fatalln("ModbusTCP Error: Modbus server instance is null.");
        disable();
    }
}

short ModbusTCP::setup()
{
    if (!modbusServer)
    {
        Log.errorln("ModbusTCP::setup - Modbus server instance is null!");
        return E_INVALID_PARAMETER;
    }
    auto readCoilsWorker = [this](ModbusMessage request) -> ModbusMessage
    {
        ModbusMessage response;
        uint16_t address = 0;
        uint16_t count = 0;
        request.get(2, address);
        request.get(4, count);
        if (count == 0 || count > 2000)
        {
            Log.warningln("ModbusTCP: Read Coil FC=01 Addr=%d Count=%d - Invalid count", address, count);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_VALUE);
            return response;
        }

        uint8_t byteCount = (count + 7) / 8;
        response.add(request.getServerID(), request.getFunctionCode(), byteCount);
        uint8_t currentByte = 0;
        uint8_t bitPos = 0;
        bool anyError = false; // Flag if any address causes an issue

        for (uint16_t i = 0; i < count; ++i)
        {
            uint16_t currentAddr = address + i;
            bool coilValue = false; // Default to OFF
            MB_Registers *reg = this->findMappingForAddress(currentAddr, FN_READ_COIL);
            Component *target = nullptr;

            if (reg)
            {
                // Basic validation (can be expanded)
                if (false) // Placeholder: Add stricter type check if needed based on component capabilities
                {
                    // Log error if needed
                    anyError = true;
                }
                else
                {
                    target = owner->byId(reg->componentId);
                    if (target)
                    {
                        short result = target->mb_tcp_read(reg);
                        // Assuming component returns 0 for OFF, non-zero for ON
                        // Check for component specific errors if needed
                        coilValue = (result != 0);
                    }
                    else
                    {
                        Log.verboseln("ModbusTCP: Read Coil FC=01 Addr=%d - Component ID %d not found.", currentAddr, reg->componentId);
                        anyError = false; // Don't flag missing mapping as a hard error for the response
                    }
                }
            }
            else
            {
                // L_WARN("ModbusTCP: Read Coil FC=01 Addr=%d - No mapping found.", currentAddr);
                anyError = false; // Don't flag missing mapping as a hard error for the response
            }

            if (coilValue)
            {
                currentByte |= (1 << bitPos);
            }

            bitPos++;
            if (bitPos == 8)
            {
                response.add(currentByte);
                currentByte = 0;
                bitPos = 0;
            }
        }
        // Add the last partial byte if necessary
        if (bitPos != 0)
        {
            response.add(currentByte);
        }

        // If anyError was true, we logged warnings but still returned data.
        // Depending on strictness, could set a response error here instead.
        // For now, we follow the pattern of logging but returning data.

        return response;
    };

    // READ_HOLDING_REGISTERS worker (FC=03)
    auto readHregsWorker = [this](ModbusMessage request) -> ModbusMessage
    {
        ModbusMessage response;
        uint16_t address = 0;
        uint16_t count = 0;
        request.get(2, address);
        request.get(4, count);
        if (count == 0 || count > 125)
        {
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_VALUE);
            return response;
        }
        response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(count * 2));
        //L_INFO("ModbusTCP: Read HReg FC=03 Addr=%d Count=%d", address, count);
        for (uint16_t i = 0; i < count; ++i)
        {
            uint16_t currentAddr = address + i;
            MB_Registers *reg = this->findMappingForAddress(currentAddr, FN_READ_HOLD_REGISTER);
            if (reg)
            {
                uint16_t regValue = 0;
                Component *target = owner->byId(reg->componentId);
                if(!target){
                    //L_WARN("ModbusTCP: Read HReg FC=03 Addr=%d - Component ID %d not found. Setting to owner %d", currentAddr, reg->componentId, owner->id);
                    // @todo: byId should return owner if not found (may fuck it up :)
                    target = owner;
                }
                if (target)
                {
                    // L_INFO("ModbusTCP: Read HReg FC=03 Addr=%d - Component ID %d found. Reading value. - Component Name: %s", currentAddr, reg->componentId, owner->name.c_str());
                    regValue = (uint16_t)target->mb_tcp_read(reg);
                    response.add(regValue);
                }
                else
                {
                    response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_ADDRESS);
                }
            }else{
                // L_WARN("ModbusTCP: Read HReg FC=03 Addr=%d - No mapping found.", currentAddr);
                #ifdef MODBUS_TCP_STRICT
                    response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_ADDRESS);
                #else
                    response.add(0xFFFF);
                #endif
            }
        }
        return response;
    };

    // WRITE_SINGLE_COIL worker (FC=05)
    auto writeSingleCoilWorker = [this](ModbusMessage request) -> ModbusMessage
    {
        ModbusMessage response;
        uint16_t address = 0;
        uint16_t valueCode = 0;
        request.get(2, address);
        request.get(4, valueCode);
        bool value = (valueCode == 0xFF00); // 0xFF00 for ON, 0x0000 for OFF

        MB_Registers *reg = this->findMappingForAddress(address, FN_WRITE_COIL);
        if (!reg)
        {
            L_WARN("ModbusTCP: Write Coil FC=05 Addr=%d - No mapping found.", address);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_ADDRESS);
            return response;
        }

        if (!(reg->type == E_FN_CODE::FN_WRITE_COIL || reg->type == E_FN_CODE::FN_WRITE_MULT_COILS) || reg->access == E_ModbusAccess::MB_ACCESS_READ_ONLY)
        {
            // Log.warningln("ModbusTCP: Write Coil FC=05 Addr=%d - Invalid type (%d) or access (%d) in mapping.", address, reg->type, reg->access);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_ADDRESS);
            return response;
        }

        Component *target = owner->byId(reg->componentId);
        if (!target)
        {
            Log.warningln("ModbusTCP: Write Coil FC=05 Addr=%d - Component ID %d not found.", address, reg->componentId);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::SERVER_DEVICE_FAILURE); // Internal error
            return response;
        }

        short result = target->mb_tcp_write(reg, value ? 1 : 0);
        if (result == E_OK)
        {
            response = request;
        }
        else
        {
            Log.errorln("ModbusTCP: Write Coil FC=05 Addr=%d - Component write failed with code %d.", address, result);
            response.setError(request.getServerID(), request.getFunctionCode(), this->mapErrorToModbusException(result));
        }
        return response;
    };

    // WRITE_SINGLE_REGISTER worker (FC=06)
    auto writeSingleHregWorker = [this](ModbusMessage request) -> ModbusMessage
    {
        ModbusMessage response;
        uint16_t address = 0;
        uint16_t value = 0;
        request.get(2, address);
        request.get(4, value);

        MB_Registers *reg = this->findMappingForAddress(address, FN_WRITE_HOLD_REGISTER);
        if (!reg)
        {
            // @todo: add error handling for this
            // L_WARN("ModbusTCP: Write HReg FC=06 Addr=%d - No mapping found.", address);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_ADDRESS);
            return response;
        }

        if (!(reg->type == E_FN_CODE::FN_WRITE_HOLD_REGISTER || reg->type == E_FN_CODE::FN_WRITE_MULT_REGISTERS) || reg->access == E_ModbusAccess::MB_ACCESS_READ_ONLY)
        {
            // @todo: add error handling for this
            // L_WARN("ModbusTCP: Write HReg FC=06 Addr=%d - Invalid type (%d) or access (%d) in mapping.", address, reg->type, reg->access);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_ADDRESS);
            return response;
        }

        Component *target = owner->byId(reg->componentId);
        if (!target)
        {
            Log.warningln("ModbusTCP: Write HReg FC=06 Addr=%d - Component ID %d not found.", address, reg->componentId);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::SERVER_DEVICE_FAILURE); // Internal error
            return response;
        }
        short result = target->mb_tcp_write(reg, (short)value);
        if (result == E_OK)
        {
            response = request;
        }
        else
        {
            Log.errorln("ModbusTCP: Write HReg FC=06 Addr=%d - Component write failed with code %d.", address, result);
            Modbus::Error modbusError = this->mapErrorToModbusException(result);
            response.setError(request.getServerID(), request.getFunctionCode(), modbusError);
        }
        return response;
    };

    // WRITE_MULTIPLE_COILS worker (FC=15, 0x0F)
    auto writeMultiCoilsWorker = [this](ModbusMessage request) -> ModbusMessage
    {
        ModbusMessage response;
        uint16_t address = 0;
        uint16_t count = 0;
        uint8_t byteCount = 0;
        request.get(2, address);
        request.get(4, count);
        request.get(6, byteCount);

        if (count == 0 || count > 1968 || byteCount != (count + 7) / 8)
        {
            Log.warningln("ModbusTCP: Write Multi Coil FC=15 Addr=%d Count=%d ByteCount=%d - Invalid params.", address, count, byteCount);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_VALUE);
            return response;
        }

        bool anyWriteFailed = false; // Track actual component write failures
        uint8_t bitPos = 0;
        uint8_t coilDataByte = 0;
        uint8_t dataIndex = 7; // Start of coil data in request

        for (uint16_t i = 0; i < count; ++i)
        {
            // Read the byte containing the current coil data if needed
            if (bitPos == 0)
            {
                if (!request.get(dataIndex++, coilDataByte))
                {
                    Log.errorln("ModbusTCP: Write Multi Coil FC=15 Addr=%d Count=%d - Failed to read data byte at index %d.", address, count, dataIndex - 1);
                    response.setError(request.getServerID(), request.getFunctionCode(), Modbus::SERVER_DEVICE_FAILURE); // Or ILLEGAL_DATA_VALUE
                    return response;                                                                                    // Abort on data read error
                }
            }

            uint16_t currentAddr = address + i;
            bool value = (coilDataByte >> bitPos) & 1;

            MB_Registers *reg = this->findMappingForAddress(currentAddr, FN_WRITE_MULT_COILS);
            Component *target = nullptr;
            short writeResult = E_NOT_FOUND;
            if (reg)
            {
                if ((reg->type == E_FN_CODE::FN_WRITE_COIL || reg->type == E_FN_CODE::FN_WRITE_MULT_COILS) && reg->access != E_ModbusAccess::MB_ACCESS_READ_ONLY)
                {
                    target = owner->byId(reg->componentId);
                    if (target)
                    {
                        writeResult = target->mb_tcp_write(reg, value ? 1 : 0);
                        if (writeResult != E_OK)
                        {
                            Log.errorln("ModbusTCP: Write Multi Coil FC=15 Addr=%d - Component write failed with code %d.", currentAddr, writeResult);
                            anyWriteFailed = true; // Mark failure only if component write fails
                        }
                    }
                    else
                    {
                        Log.warningln("ModbusTCP: Write Multi Coil FC=15 Addr=%d - Component ID %d not found.", currentAddr, reg->componentId);
                    }
                }
                else
                {
                    L_WARN("ModbusTCP: Write Multi Coil FC=15 Addr=%d - Invalid type (%d) or access (%d).", currentAddr, reg->type, reg->access);
                }
            }
            else
            {
                L_WARN("ModbusTCP: Write Multi Coil FC=15 Addr=%d - No mapping found.", currentAddr);
            }

            bitPos = (bitPos + 1) % 8;
        }

        if (!anyWriteFailed)
        {
            // Standard success response: function code, start address, number of coils written
            response.add(request.getServerID(), request.getFunctionCode());
            response.add(address);
            response.add(count);
        }
        else
        {
            // Report a general failure if any part failed
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::SERVER_DEVICE_FAILURE);
        }
        return response;
    };

    // WRITE_MULTIPLE_REGISTERS worker (FC=16, 0x10)
    auto writeMultiHregsWorker = [this](ModbusMessage request) -> ModbusMessage
    {
        ModbusMessage response;
        uint16_t address = 0;
        uint16_t count = 0;
        uint8_t byteCount = 0;
        request.get(2, address);
        request.get(4, count);
        request.get(6, byteCount);

        if (count == 0 || count > 123 || byteCount != count * 2)
        {
            Log.warningln("ModbusTCP: Write Multi HReg FC=16 Addr=%d Count=%d ByteCount=%d - Invalid params.", address, count, byteCount);
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::ILLEGAL_DATA_VALUE);
            return response;
        }

        bool anyWriteFailed = false; // Track actual component write failures
        uint8_t dataIndex = 7;       // Start of register data in request

        for (uint16_t i = 0; i < count; ++i)
        {
            uint16_t currentAddr = address + i;
            uint16_t value = 0;
            // Read the register value from the request
            if (!request.get(dataIndex, value))
            {
                Log.errorln("ModbusTCP: Write Multi HReg FC=16 Addr=%d Count=%d - Failed to read data value at index %d.", address, count, dataIndex);
                response.setError(request.getServerID(), request.getFunctionCode(), Modbus::SERVER_DEVICE_FAILURE); // Or ILLEGAL_DATA_VALUE
                return response;                                                                                    // Abort on data read error
            }
            dataIndex += 2;

            MB_Registers *reg = this->findMappingForAddress(currentAddr, FN_WRITE_MULT_REGISTERS);
            Component *target = nullptr;
            short writeResult = E_NOT_FOUND; // Default to error

            if (reg)
            {
                if ((reg->type == E_FN_CODE::FN_WRITE_HOLD_REGISTER || reg->type == E_FN_CODE::FN_WRITE_MULT_REGISTERS) && reg->access != E_ModbusAccess::MB_ACCESS_READ_ONLY)
                {
                    target = owner->byId(reg->componentId);
                    if (target)
                    {
                        writeResult = target->mb_tcp_write(reg, (short)value);
                        if (writeResult != E_OK)
                        {
                            Log.errorln("ModbusTCP: Write Multi HReg FC=16 Addr=%d - Component write failed with code %d.", currentAddr, writeResult);
                            anyWriteFailed = true; // Mark failure only if component write fails
                        }
                    }
                    else
                    {
                        Log.warningln("ModbusTCP: Write Multi HReg FC=16 Addr=%d - Component ID %d not found.", currentAddr, reg->componentId);
                    }
                }
                else
                {
                    L_WARN("ModbusTCP: Write Multi HReg FC=16 Addr=%d - Invalid type (%d) or access (%d).", currentAddr, reg->type, reg->access);
                }
            }
            else
            {
                L_WARN("ModbusTCP: Write Multi HReg FC=16 Addr=%d - No mapping found.", currentAddr);
            }
        }

        if (!anyWriteFailed)
        {
            // Standard success response: function code, start address, number of registers written
            response.add(request.getServerID(), request.getFunctionCode());
            response.add(address);
            response.add(count);
        }
        else
        {
            // General failure
            response.setError(request.getServerID(), request.getFunctionCode(), Modbus::SERVER_DEVICE_FAILURE);
        }
        return response;
    };

    // --- Register Workers with eModbus ---
    // Assuming SERVER_ID 1 for simplicity. Get from config or constructor if needed.
    const uint8_t SERVER_ID = 1;
    modbusServer->registerWorker(SERVER_ID, Modbus::READ_COIL, readCoilsWorker);
    modbusServer->registerWorker(SERVER_ID, Modbus::READ_HOLD_REGISTER, readHregsWorker);
    modbusServer->registerWorker(SERVER_ID, Modbus::WRITE_COIL, writeSingleCoilWorker);
    modbusServer->registerWorker(SERVER_ID, Modbus::WRITE_HOLD_REGISTER, writeSingleHregWorker);
    modbusServer->registerWorker(SERVER_ID, Modbus::WRITE_MULT_COILS, writeMultiCoilsWorker);
    modbusServer->registerWorker(SERVER_ID, Modbus::WRITE_MULT_REGISTERS, writeMultiHregsWorker);    
    Log.infoln("ModbusTCP::setup - eModbus workers registered.");
    return Component::setup();
}

short ModbusTCP::loop() { return Component::loop(); }

/**
 * @brief Registers a component's Modbus address range with the manager.
 */
bool ModbusTCP::registerModbus(Component *component, const MB_Registers &info)
{
    if (!component)
    {
        Log.errorln("ModbusTCP::registerComponent - Null component pointer.");
        return false;
    }

    if (info.startAddress < 0 || info.count <= 0)
    {
        Log.warningln("registerModbus - Component ID %d (%s) provided invalid block (Address: %d, Count: %d). Skipping block ! %s @ %d ",
                      component->id, component->name.c_str(), info.startAddress, info.count,info.name,info.slaveId);
        
        return false;
    }

    if (addressMappings.size() >= MAX_MODBUS_COMPONENTS)
    {
        Log.errorln("ModbusTCP::registerModbus - Max Modbus components/mappings reached (%d). Cannot register block for Component ID %d (%s).",
                    MAX_MODBUS_COMPONENTS, component->id, component->name.c_str());
        return false;
    }
    MB_Registers infoWithId = info;
    infoWithId.componentId = component->id;
    addressMappings.push_back(infoWithId);
    return true;
}
/**
 * @brief Finds the component registered to handle a specific Modbus address.
 * @param address The Modbus address.
 * @return Pointer to the component, or nullptr if no component handles this address or owner is invalid.
 */
Component *ModbusTCP::findComponentForAddress(short address)
{
    if (!owner)
    {
        Log.errorln("ModbusTCP::findComponentForAddress - Owner is null, cannot look up component.");
        return nullptr;
    }
    for (const auto &info : addressMappings)
    {
        if (address >= info.startAddress && address < (info.startAddress + info.count))
        {
            Component *target = owner->byId(info.componentId);
            if (target)
            {
                return target;
            }
        }
    }
    return nullptr;
}

/**
 * @brief Maps internal error codes to Modbus exception codes.
 * @param internalError The error code returned by a component's mb_tcp_write.
 * @return Corresponding Modbus::Error code.
 */
Modbus::Error ModbusTCP::mapErrorToModbusException(short internalError)
{
    // Extend this mapping as needed
    switch (internalError)
    {
    case E_OK:
        // return Modbus::EX_SUCCESS; // Should not happen if called on error
        return Modbus::SUCCESS; // Use SUCCESS instead of EX_SUCCESS
    case E_INVALID_PARAMETER:
        // Could be address or value depending on context
        return Modbus::ILLEGAL_DATA_ADDRESS; // Or ILLEGAL_DATA_VALUE
    // Add mappings for other relevant internal error codes
    // case E_SOME_OTHER_ERROR:
    //     return Modbus::SLAVE_DEVICE_FAILURE;
    default:
        // General failure for unmapped errors
        return Modbus::SERVER_DEVICE_FAILURE;
    }
}

/**
 * @brief Gets the internal list of Modbus address mappings.
 */
const Vector<MB_Registers> &ModbusTCP::getAddressMappings() const
{
    return addressMappings;
}

// Implementation for the pure virtual function from Component
// ModbusTCP manager itself doesn't define blocks this way, so return empty view.
ModbusBlockView *ModbusTCP::mb_tcp_blocks() const
{
    static ModbusBlockView blockView = {nullptr, 0};
    return &blockView;
}

/**
 * @brief Finds the MB_Registers mapping definition for a specific Modbus address and function code (const version).
 * @param address The Modbus address.
 * @param fc The Modbus function code (determines allowed type/access).
 * @return Const pointer to the mapping definition, or nullptr if no suitable mapping is found.
 */
MB_Registers *ModbusTCP::findMappingForAddress(short address, E_FN_CODE fc)
{
    for (const auto &info : addressMappings)
    {
        if (address >= info.startAddress && address < (info.startAddress + info.count))
        {
            return (MB_Registers *)&info;
        }
    }
    return nullptr;
}

short ModbusTCP::getConnectedClients() const 
{
    if (!modbusServer) {
        return 0;
    }
    return (short)modbusServer->activeClients();
}
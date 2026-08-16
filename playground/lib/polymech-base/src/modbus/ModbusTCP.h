#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <Component.h>
#include <enums.h>
#include "config-modbus.h"
#include <ArduinoLog.h>
#include <Vector.h>
#include <ModbusServerTCPasync.h>
#include "modbus/ModbusTypes.h"

class PHApp;

/**
 * @brief Manages Modbus TCP communication (acting as a Server/Slave).
 *
 * This class listens for Modbus TCP requests, finds the target component,
 * and uses the Component's network interface (read/mb_tcp_write) to process the request.
 */
class ModbusTCP : public Component
{
public:
    /**
     * @brief Constructor for ModbusTCP.
     * @param owner The owning component (typically PHApp).
     * @param modbusServerInstance A pointer to the actual Modbus server library instance.
     */
    ModbusTCP(Component *owner, ModbusServerTCPasync *modbusServerInstance);

    virtual ~ModbusTCP() = default; 
    /**
     * @brief Initializes the Modbus TCP server and sets up callbacks.
     */
    short setup() override;
    /**
     * @brief Handles Modbus TCP communication polling/tasks (if any needed).
     * Must be called regularly in the main loop.
     */
    short loop() override;
    /**
     * @brief Registers a component's Modbus address range with the manager.
     * @param component Pointer to the component to register.
     * @param info MB_Registers struct describing the address block.
     * @return True if registration was successful, false otherwise.
     */
    bool registerModbus(Component* component, const MB_Registers& info);
    /**
     * @brief Finds the component registered to handle a specific Modbus address.
     * @param address The Modbus address.
     * @return Pointer to the component, or nullptr if no component handles this address.
     */
    Component* findComponentForAddress(short address);
    /**
     * @brief Finds the MB_Registers mapping definition for a specific Modbus address and function code.
     * @param address The Modbus address.
     * @param fc The Modbus function code (determines allowed type/access).
     * @return Pointer to the mapping definition, or nullptr if no suitable mapping is found.
     */
    MB_Registers* findMappingForAddress(short address, E_FN_CODE fc);
    /**
     * @brief Maps internal error codes to Modbus exception codes.
     * @param internalError The error code returned by a component's mb_tcp_write.
     * @return Corresponding Modbus::Error code.
     */
    Modbus::Error mapErrorToModbusException(short internalError);
    /**
     * @brief Gets the internal list of Modbus address mappings.
     * Primarily for use by REST server or diagnostics.
     * @return A constant reference to the vector of MB_Registers mappings.
     */
    const Vector<MB_Registers>& getAddressMappings() const;
    Vector<MB_Registers> addressMappings;

    // Implement pure virtual function inherited from Component
    ModbusBlockView* mb_tcp_blocks() const override;

    ModbusServerTCPasync *modbusServer;
    
    /**
     * @brief Gets the number of currently connected Modbus TCP clients.
     * @return Number of connected clients.
     */
    short getConnectedClients() const;
    
private:
    MB_Registers mappingStorage[MAX_MODBUS_COMPONENTS];
};

#endif
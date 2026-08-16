#include "RS485.h"
#include <modbus/ModbusRTU.h> 
#include <ArduinoLog.h>
#include <Component.h> 
#include <RTUutils.h> 
#include <enums.h>    
#include <macros.h>   
#include <modbus/ModbusTypes.h>
#include <vector>
#include <algorithm>
#include <net/commons.h>


#include "RS485Devices.h" // registerApplicationDevices
#include "PHApp.h"

RS485* RS485::instance = nullptr;

RS485::RS485(Component *owner)
    : Component("RS485", COMPONENT_KEY_RS485, COMPONENT_DEFAULT, owner),
      modbus(this, REDEPIN_MODBUS, 32)
{
    this->owner = owner;
    RS485::instance = this;
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
}
RS485::~RS485()
{
    L_INFO("RS485 component destroyed.");
}
short RS485::setup()
{
    Log.infoln(F("--- Setting up RS485 Interface ---"));
    pinMode(REDEPIN_MODBUS, OUTPUT);   // Use define from ModbusTypes.h or similar
    digitalWrite(REDEPIN_MODBUS, LOW); // Use define from ModbusTypes.h or similar
    RTUutils::prepareHardwareSerial(RS485_SERIAL_PORT);
    RS485_SERIAL_PORT.begin(MB_RTU_BAUDRATE, SERIAL_8N1, RXD1_PIN, TXD1_PIN);
    if (!RS485_SERIAL_PORT)
    { // Basic check if Serial port failed
        L_ERROR(F("RS485: Failed to begin Serial port!"));
        return E_SERIAL_INIT_FAILED;
    }
    // 3. Initialize Modbus RTU Master/Client
    MB_Error initResult = modbus.begin(RS485_SERIAL_PORT, MB_RTU_BAUDRATE);
    if (initResult == MB_Error::Success)
    {
        Log.infoln(F("RS485: ModbusRTU initialized successfully."));        
        modbus.setOnRegisterChangeCallback(RS485::staticRtuRegisterChangeCallback);
        modbus.setResponseCallback(Manager::responseCallback);
        modbus.setOnErrorCallback(Manager::staticOnError);
        deviceManager.setAsGlobalInstance();

#ifdef ENABLE_RS485_DEVICES
        RS485Devices::registerApplicationDevices(this);
        L_INFO(F("RS485: Application device registration triggered via RS485Devices."));
#endif
    }
    else
    {
        L_ERROR(F("RS485: ModbusRTU initialization failed! Error: %d"), static_cast<int>(initResult));
        return E_MODBUS_INIT_FAILED; // Use defined error code
    }
    deviceManager.initializeDevices(modbus);
    // deviceManager.printDeviceStatuses(modbus);
    #ifdef ENABLE_SETTINGS
    PHApp *phApp = (PHApp *)this->owner;
    modbus.setOperationTimeout(phApp->appSettings->get("OPERATION_TIMEOUT", (uint32_t)OPERATION_TIMEOUT));
    #endif

    return E_OK;
}
short RS485::loop()
{
    unsigned long now = millis();
    if (now - lastLoopTime >= RS485_LOOP_INTERVAL_MS)
    {
        lastLoopTime = now;
        modbus.process();
        deviceManager.processDevices(modbus);
    }
    return E_OK;
}
short RS485::mb_tcp_read(short address)
{
    // TODO: Implement TCP read delegation
    // 1. Find the RTU_Base* device responsible for this TCP address (requires mapping info)
    // 2. Call a method on that device (e.g., readTcpMappedValue(address)) to get the value.
    // Log.warningln(F("RS485::mb_tcp_read STUB for address %d"), address);
    return E_NOT_IMPLEMENTED;
}
short RS485::mb_tcp_write(short address, short value)
{
    // TODO: Implement TCP write delegation
    // 1. Find the RTU_Base* device responsible for this TCP address (requires mapping info).
    // 2. Call a method on that device (e.g., writeTcpMappedValue(address, value)).
    // Log.warningln(F("RS485::mb_tcp_write STUB for address %d, value %d"), address, value);
    return E_NOT_IMPLEMENTED;
}
short RS485::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
        return E_INVALID_PARAMETER;

    RTU_Base *targetDevice = deviceManager.getDeviceById(reg->slaveId);
    if (targetDevice)
    {
        return targetDevice->mb_tcp_read(reg);
    }
    else
    {
        L_ERROR(F("RS485::mb_tcp_read - Device not found for Slave ID %d (from MB_Registers for TCP %d)"), reg->slaveId, reg->startAddress);
        return (short)MB_Error::ServerDeviceFailure;
    }
}
short RS485::mb_tcp_write(MB_Registers *reg, short value)
{
    if (!reg || !reg->slaveId)
        return E_INVALID_PARAMETER;
    if (reg->slaveId == 0 || reg->slaveId > MAX_MODBUS_SLAVES)
    {
        Log.warningln(F("RS485::mb_tcp_write - Invalid Slave ID %d in MB_Registers for TCP Addr %d."), reg->slaveId, reg->startAddress);
        return (short)MB_Error::ServerDeviceFailure; // Indicate internal error
    }

    RTU_Base *targetDevice = deviceManager.getDeviceById(reg->slaveId);
    if (targetDevice)
    {
        return targetDevice->mb_tcp_write(reg, value);
    }
    else
    {
        L_ERROR(F("RS485::mb_tcp_write - Device not found for Slave ID %d (from MB_Registers for TCP %d)"), reg->slaveId, reg->startAddress);
        return (short)MB_Error::ServerDeviceFailure; // Device specified by mapping doesn't exist
    }
}
void RS485::mb_tcp_register(ModbusTCP *manager)
{
    if (!manager)
    {
        L_ERROR(F("RS485::mb_tcp_register: ModbusTCP manager is null!"));
        return;
    }

    // Check if this component itself should be registered (it acts as a gateway)
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS))
    {
        Log.warningln(F("RS485::mb_tcp_register: Gateway component lacks E_NCAPS_MODBUS flag. Skipping registration."));
        // Consider setting the flag in the constructor or setup if this registration should always happen.
        return;
    }

    L_INFO(F("RS485::mb_tcp_register: Registering TCP blocks for managed RTU devices (Gateway ID: %d)..."), this->id);

    // Get the array of device pointers and the size
    RTU_Base *const *devices = deviceManager.getDevices();
    int maxDevices = deviceManager.getMaxDevices();
    int totalRegisteredBlocks = 0;

    // Cast self to non-const for registerModbus call
    // Ensure this is safe and appropriate based on registerModbus's contract
    Component *thiz = const_cast<RS485 *>(this);

    // Iterate through the device array
    for (int i = 0; i < maxDevices; ++i)
    {
        const RTU_Base *device = devices[i];
        if (device)
        { // Check if the slot is not null

            // Get the block definitions from the device
            // ASSUMPTION: RTU_Base (or derived class) has mb_tcp_blocks()
            ModbusBlockView *deviceBlocks = device->mb_tcp_blocks(); // User confirmed this method name

            if (deviceBlocks && deviceBlocks->data && deviceBlocks->count > 0)
            { // Use ->data based on PHApp example
// Log device name if available, otherwise just ID
#ifdef RTU_BASE_HAS_DEVICE_NAME // Check if RTU_Base eventually gets deviceName
                Log.verboseln(F("RS485: Device ID %d (%s) provided %d TCP block(s). Registering with Gateway ID %d."),
                              device->slaveId, device->deviceName.c_str(), deviceBlocks->count, this->id);
#else
                // Log.infoln(F("RS485: Device ID %d provided %d TCP block(s). Registering with Gateway ID %d."),
                //              device->slaveId, deviceBlocks->count, this->id);
#endif

                // Iterate through the blocks provided by this specific device
                for (int blockIdx = 0; blockIdx < deviceBlocks->count; ++blockIdx)
                {
                    MB_Registers info = deviceBlocks->data[blockIdx]; // Use ->data

                    // CRITICAL: Associate the block with the GATEWAY component ID
                    info.componentId = this->id;

                    // Register this block with the TCP manager using the gateway component pointer
                    if(manager->registerModbus(thiz, info))
                    {
                        totalRegisteredBlocks++;
                    }
                    else
                    {
                        Log.warningln(F("RS485: FAILED to register TCP block for RTU Slave ID %d. Block: Addr=%d, Count=%d, Name='%s'"),
                                      device->slaveId, info.startAddress, info.count, info.name);
                    }
                }
                // Memory Management Note: If mb_tcp_blocks allocates, it needs freeing. Assume static/managed for now.
            }
            else
            {
                Log.verboseln(F("RS485: Device ID %d provided no TCP blocks."), device->slaveId);
            }
        }
    }

    L_INFO(F("RS485::mb_tcp_register: Finished. Registered %d total blocks from managed devices."), totalRegisteredBlocks);
}
ushort RS485::mb_tcp_error(MB_Registers *reg)
{
    if (!reg)
        return E_INVALID_PARAMETER;
    
    RTU_Base *device = deviceManager.getDeviceById(reg->slaveId);
    if (device)
    {
        return device->mb_tcp_error(reg);
    }
    else
    {
        L_ERROR(F("RS485::mb_tcp_error - Device not found for Slave ID %d (from MB_Registers for TCP %d)"), reg->slaveId, reg->startAddress);
        return E_INVALID_PARAMETER;
    }
}
void RS485::staticRtuRegisterChangeCallback(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue)
{
    if (RS485::instance) {
        RS485::instance->handleRtuRegisterChange(op, oldValue, newValue);
    }
}
void RS485::handleRtuRegisterChange(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue)
{
    deviceManager.handleRegisterChange(op, oldValue, newValue);
    RTU_Base *rtuDevice = deviceManager.getDeviceById(op.slaveId);
    if (!rtuDevice)
    {
        // @todo: add error handling for this
        //Log.warningln(F("[RS485::handleRtuRegisterChange] Device not found for Slave ID %d. Cannot translate for broadcast."), op.slaveId);
        return;
    }
    uint16_t tcpBaseAddress = rtuDevice->mb_tcp_base_address();
    uint16_t tcpOffset = rtuDevice->mb_tcp_offset_for_rtu_address(op.address);
    if (tcpBaseAddress == 0 || tcpOffset == 0)
    {
        // If no valid TCP mapping, do not broadcast this specific RTU change via TCP address based update.
        // This can happen for registers not exposed over TCP or if device has no TCP base.
        return;
    }
    uint16_t tcpAddress = tcpBaseAddress + tcpOffset;    
    MB_UpdateData update; 
    update.slaveId = op.slaveId; // Keep original RTU slave ID for context
    update.address = tcpAddress; // Use the calculated TCP address for the update
    update.value = newValue;
    update.functionCode = op.type; // Use the function code from the original RTU operation
    update.componentId = rtuDevice->id; // The component ID of the RTU device that changed
    update.priority = E_PRIORITY::E_PRIORITY_LOWEST;
    // Log before sending to owner
    Log.verboseln("RS485 (ID: %u) forwarding RTU register change from RTU device ID %u (Slave %u), OriginalAddr: %u -> TCPAddr: %u, NewVal: %u, FC: %u to its owner.",
                  this->id, rtuDevice->id, op.slaveId, op.address, tcpAddress, newValue, op.type);

    if (this->owner) {
        this->owner->onMessage(rtuDevice->id, E_CALLS::EC_USER, E_MessageFlags::E_MF_NONE, &update, rtuDevice);
    } else {
        Log.warningln("RS485 (ID: %u) has no owner to forward RTU register change.", this->id);
    }
}
short RS485::onMessage(int originId, E_CALLS reason, E_MessageFlags flags, void *data, Component *sender)
{
    Log.verboseln("RS485 (ID: %u) received onMessage from CompID: %d (Sender: %s), Reason: %d. Forwarding to owner.", 
                  this->id, originId, sender ? sender->name.c_str() : "UnknownSender", static_cast<int>(reason));

    if (this->owner)
    {
        this->owner->onMessage(originId, reason, flags, data, sender);
    }
    else
    {
        Log.warningln("RS485 (ID: %u) has no owner to forward message from CompID: %d.", this->id, originId);
    }
    // Base class Component::onMessage returns E_OK, so we can too if no other specific error.
    return E_OK; 
}

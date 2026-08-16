#include "PHApp.h"

#ifdef ENABLE_MODBUS_TCP

#include <modbus/ModbusTCP.h>     // For ModbusManager class
#include <ArduinoLog.h>           // For logging
#include <enums.h>                // For error codes like E_INVALID_PARAMETER
#include <ModbusServerTCPasync.h> // Include for ModbusServerTCPasync
#include <enums.h>
#include <modbus/ModbusTypes.h>

#include "config-modbus.h"        // Include centralized addresses (defines MODBUS_PORT)

extern ModbusServerTCPasync mb;

void PHApp::mb_tcp_register(ModbusTCP *manager) const
{
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS))
        return;
    ModbusBlockView *blocksView = mb_tcp_blocks();
    Component *thiz = const_cast<PHApp *>(this);
    for (int i = 0; i < blocksView->count; ++i)
    {
        MB_Registers info = blocksView->data[i];
        info.componentId = this->id;
        manager->registerModbus(thiz, info);
    }
}

ModbusBlockView *PHApp::mb_tcp_blocks() const
{
    static const MB_Registers kBlocks[] = {
        {MB_ADDR_SYSTEM_ERROR, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, static_cast<ushort>(this->id), 0, "PHApp: System Error","PHApp"},
        {MB_ADDR_APP_STATE, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "PHApp: App State","PHApp"},
        {MB_ADDR_RESET_CONTROLLER, 1, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "PHApp: Reset Controller","PHApp"},
        {MB_ADDR_ECHO_TEST, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "PHApp: Echo Test","PHApp"},
    };
    static ModbusBlockView blockView = {kBlocks, int(sizeof(kBlocks) / sizeof(kBlocks[0]))};
    return &blockView;
}

/**
 * @brief Handles Modbus read requests for PHApp's specific registers.
 */
short PHApp::mb_tcp_read(short address)
{
    switch (address)
    {
    case MB_ADDR_SYSTEM_ERROR:
        return (short)getLastError();
    case MB_ADDR_APP_STATE:
        return (short)_state;
    case MB_ADDR_ECHO_TEST:
        return (short)88;
    default:
        return 0;
    }
}
short PHApp::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    return mb_tcp_write(reg->startAddress, networkValue);
}
/**
 * @brief Handles Modbus write requests for PHApp's specific registers.
 */
short PHApp::mb_tcp_write(short address, short value)
{
    switch (address)
    {
    case MB_ADDR_RESET_CONTROLLER:
        reset(0, 0);
        return E_OK; 
    default:
        return E_INVALID_PARAMETER;
    }
}
short PHApp::loopModbus()
{
    return E_OK;
}
short PHApp::setupModbus()
{
    Log.infoln("Setting up Modbus TCP...");
    modbusManager = new ModbusTCP(this, &mb);
    if (!modbusManager)
    {
        Log.fatalln("Failed to create ModbusTCP!");
        return E_INVALID_PARAMETER;
    }
    components.push_back(modbusManager); // Add manager to component list for setup/loop calls
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    // --- Register Components with Modbus Manager ---
    Log.infoln("PHApp::setupModbus - Registering components with ModbusTCP...");
    mb_tcp_register(modbusManager);
    Log.infoln("--- End Modbus Mappings DUMP --- ");
    if (modbusManager->enabled())
    {
        mb.start(MODBUS_PORT, 10, 2000);
    }
    else
    {
        Log.warningln("PHApp::setupModbus - ModbusTCP not available or disabled. Skipping Modbus server start.");
    }
    return E_OK;
}

#endif // ENABLE_MODBUS_TCP
#include "PHApp.h"

#ifdef ENABLE_MODBUS_TCP

#include <modbus/ModbusTCP.h>     // For ModbusManager class
#include <ArduinoLog.h>           // For logging
#include <enums.h>                // For error codes like E_INVALID_PARAMETER
#include <ModbusServerTCPasync.h> // Include for ModbusServerTCPasync
#include <enums.h>
#include <modbus/ModbusTypes.h>
#include "config-modbus.h" // Include centralized addresses (defines MODBUS_PORT)
#include <esp_heap_caps.h>

extern ModbusServerTCPasync mb;

void PHApp::mb_tcp_register(ModbusTCP *manager)
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
        {MB_ADDR_SYSTEM_ERROR, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, static_cast<ushort>(this->id), 0, "System Error", "PHApp"},
        {MB_ADDR_APP_STATE, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "App State", "PHApp"},
        {MB_ADDR_SUB_STATE_0, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, static_cast<ushort>(this->id), 0, "Sub State 0", "PHApp"},
        {MB_ADDR_SUB_STATE_1, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, static_cast<ushort>(this->id), 0, "Sub State 1", "PHApp"},
        {MB_ADDR_IS_SLAVE, 1, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "Is Slave", "PHApp"},
        {MB_ADDR_ALL_OMRON_STOP, 1, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "All Omron Stop", "PHApp"},
        {MB_ADDR_ALL_OMRON_COM_WRITE, 1, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "All Omron Com Write", "PHApp"},
        {MB_ADDR_CMD_WRITE, 1, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "Write Command", "PHApp"},
        {MB_ADDR_RESET_CONTROLLER, 1, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "Reset Controller", "PHApp"},
        {MB_ADDR_ECHO_TEST, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "Echo Test", "PHApp"},
        {MB_ADDR_MEM_FREE_HEAP_KB, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, static_cast<ushort>(this->id), 0, "Free Heap", "PHApp"},
        {MB_ADDR_MEM_MAX_FREE_BLOCK_KB, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, static_cast<ushort>(this->id), 0, "Max Free Block)", "PHApp"},
        {MB_ADDR_MEM_FRAGMENTATION_PERCENT, 1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, static_cast<ushort>(this->id), 0, "Heap Fragmentationn", "PHApp"},
        {MB_ADDR_APP_WARMUP, 1, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "Warmup", "PHApp"},
        {MB_ADDR_APP_PID_LAG, 1, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, static_cast<ushort>(this->id), 0, "PID Lag", "PHApp"},
    };
    static ModbusBlockView blockView = {kBlocks, int(sizeof(kBlocks) / sizeof(kBlocks[0]))};
    return &blockView;
}

/**
 * @brief Handles Modbus read requests for PHApp's specific registers.
 */
short PHApp::mb_tcp_read(MB_Registers *reg)
{
    switch (reg->startAddress)
    {
    case MB_ADDR_SYSTEM_ERROR:
        return (short)getLastError();
    case MB_ADDR_APP_STATE:
        return (short)_state;
    case MB_ADDR_IS_SLAVE:
        return (short)(_is_slave ? 1 : 0);
    case MB_ADDR_ALL_OMRON_STOP:
        return (short)(_all_omron_stop ? 1 : 0);
    case MB_ADDR_ALL_OMRON_COM_WRITE:
        return (short)(_all_omron_com_write ? 1 : 0);
    case MB_ADDR_ECHO_TEST:
        return (short)88;
    case MB_ADDR_MEM_FREE_HEAP_KB:
        return (short)(ESP.getFreeHeap() / 1024);
    case MB_ADDR_MEM_MAX_FREE_BLOCK_KB:
        return (short)(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) / 1024);
    case MB_ADDR_MEM_FRAGMENTATION_PERCENT:
        return (short)getHeapFragmentation();
    case MB_ADDR_APP_WARMUP:
        return (short)(_app_warmup ? 1 : 0);
    case MB_ADDR_APP_PID_LAG:
        return (short)(_app_pid_lag ? 1 : 0);
    default:
        return 0;
    }
}
short PHApp::runCommand(short command)
{
    switch (command)
    {
    case COMMANDS::CMD_RESET:
        reset(0, 0);
        return E_OK;

    case COMMANDS::CMD_PRINT_SLAVES:
    {
        if (rs485)
        {
            // rs485->modbus.printSlaves();
            rs485->modbus.printSlaves();
        }
        else
        {
            L_ERROR("PHApp::runCommand - RS485 not initialized.");
        }
        return E_OK;
    }
    case COMMANDS::CMD_PRINT_QUEUE:
        if (rs485)
        {
            // rs485->modbus.printSlaves();
            rs485->modbus.printQueue();
        }
        else
        {
            L_ERROR("PHApp::runCommand - RS485 not initialized.");
        }
        return E_OK;

    case COMMANDS::CMD_PRINT_BACKOFF:
        if (rs485)
        {
            // rs485->modbus.printSlaves();
            rs485->modbus.printBackoffQueue();
        }
        else
        {
            L_ERROR("PHApp::runCommand - RS485 not initialized.");
        }
        return E_OK;
    default:
        return E_INVALID_PARAMETER;
    }

    return E_OK;
}

/**
 * @brief Handles Modbus write requests for PHApp's specific registers.
 */
short PHApp::mb_tcp_write(MB_Registers *reg, short value)
{
    L_INFO("PHApp::mb_tcp_write - Register: %d, Value: %d", reg->startAddress, value);
    switch (reg->startAddress)
    {
    case MB_ADDR_IS_SLAVE:
        return setSlave(value != 0, true);
    case MB_ADDR_ALL_OMRON_STOP:
        return setAllOmronStop(value != 0, true);
    case MB_ADDR_ALL_OMRON_COM_WRITE:
        return setAllOmronComWrite(value != 0, true);
    case MB_ADDR_RESET_CONTROLLER:
        reset(0, 0);
        return E_OK;
    case MB_ADDR_CMD_WRITE:
        return (short)runCommand(value);
    case MB_ADDR_APP_WARMUP:
        return setAppWarmup(value != 0, true);
    case MB_ADDR_APP_PID_LAG:
        return setAppPidLag(value != 0, true);
    default:
        return E_INVALID_PARAMETER;
    }
}
short PHApp::loopModbus()
{
    return E_OK;
}

short PHApp::getConnectedClients() const
{
    if (!modbusManager)
        return 0;
    return modbusManager->getConnectedClients();
}

short PHApp::setupModbus()
{
    modbusManager = new ModbusTCP(this, &mb);
    components.push_back(modbusManager);
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    mb_tcp_register(modbusManager);
    mb.start(MODBUS_PORT, 3, 1000);
    return E_OK;
}

#endif // ENABLE_MODBUS_TCP
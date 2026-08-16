#include "Plunger.h"
#include <Arduino.h>

ModbusBlockView *Plunger::mb_tcp_blocks() const
{
    static MB_Registers blocks[PLUNGER_MB_BLOCK_COUNT] = {
        {static_cast<uint16_t>(PLUNGER_MB_BASE_ADDRESS + PLUNGER_MB_STATE_OFFSET),
         1,
         E_FN_CODE::FN_READ_HOLD_REGISTER,
         MB_ACCESS_READ_ONLY,
         static_cast<uint16_t>(id),
         PLUNGER_MB_STATE_OFFSET,
         "Plunger State",
         PLUNGER_COMPONENT_NAME},
        {static_cast<uint16_t>(PLUNGER_MB_BASE_ADDRESS + PLUNGER_MB_COMMAND_OFFSET),
         1,
         E_FN_CODE::FN_READ_HOLD_REGISTER,
         MB_ACCESS_READ_WRITE,
         static_cast<uint16_t>(id),
         PLUNGER_MB_COMMAND_OFFSET,
         "Plunger Command (0:None,1:Home,2:Plunge,3:Stop,4:Info,5:Fill)",
         PLUNGER_COMPONENT_NAME}};
    static ModbusBlockView blockView = {blocks, PLUNGER_MB_BLOCK_COUNT};
    return &blockView;
}

void Plunger::mb_tcp_register(ModbusTCP *mgr)
{
    if (!mgr)
        return;
    ModbusBlockView *blocksView = mb_tcp_blocks();
    Component *thiz = const_cast<Plunger *>(this);
    for (int i = 0; i < blocksView->count; ++i)
    {
        mgr->registerModbus(thiz, blocksView->data[i]);
    }
}

short Plunger::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;

    uint16_t address = reg->startAddress;

    if (address == (PLUNGER_MB_BASE_ADDRESS + PLUNGER_MB_STATE_OFFSET))
    {
        return static_cast<short>(_currentState);
    }
    else if (address == (PLUNGER_MB_BASE_ADDRESS + PLUNGER_MB_COMMAND_OFFSET))
    {
        return _modbusCommandRegisterValue;
    }
    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
}

short Plunger::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    if (!reg)
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;

    uint16_t address = reg->startAddress;

    if (address == (PLUNGER_MB_BASE_ADDRESS + PLUNGER_MB_COMMAND_OFFSET))
    {
        E_PlungerCommand cmd = static_cast<E_PlungerCommand>(networkValue);
        _modbusCommandRegisterValue = networkValue;

        short result = E_OK;
        switch (cmd)
        {
        case E_PlungerCommand::CMD_HOME:
            result = this->cmd_home();
            break;
        case E_PlungerCommand::CMD_PLUNGE:
            result = this->cmd_plunge();
            break;
        case E_PlungerCommand::CMD_STOP:
            result = this->cmd_stop();
            break;
        case E_PlungerCommand::CMD_INFO:
            result = this->info();
            break;
        case E_PlungerCommand::CMD_FILL:
            result = this->cmd_fill();
            break;
        case E_PlungerCommand::CMD_REPLAY:
            result = this->cmd_replay();
            break;
        case E_PlungerCommand::NO_COMMAND:
            Log.verboseln("[%s] Modbus NO_COMMAND received.", name.c_str());
            break;
        default:
            Log.warningln("[%s] Unknown Modbus command received: %d", name.c_str(), networkValue);
            result = MODBUS_ERROR_ILLEGAL_DATA_VALUE;
            break;
        }

        if (cmd != E_PlungerCommand::NO_COMMAND && result == E_OK)
        {
            _modbusCommandRegisterValue = static_cast<short>(E_PlungerCommand::NO_COMMAND);
        }
        else if (result != E_OK && result != MODBUS_ERROR_ILLEGAL_DATA_VALUE)
        {
            _modbusCommandRegisterValue = static_cast<short>(E_PlungerCommand::NO_COMMAND);
        }

        return (result == E_OK || result == 1) ? E_OK : result;
    }
    else if (address == (PLUNGER_MB_BASE_ADDRESS + PLUNGER_MB_STATE_OFFSET))
    {
        Log.warningln("[%s] mb_tcp_write: Attempt to write to read-only State register %d", name.c_str(), address);
        return MODBUS_ERROR_ILLEGAL_FUNCTION;
    }

    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
}

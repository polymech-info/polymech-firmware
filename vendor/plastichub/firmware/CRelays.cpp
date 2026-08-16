#include "CRelays.h"

#ifdef NB_CONTROLLINO_RELAYS

#include "ModbusBridge.h"

void CRelays::print(){}

void CRelays::fromTCP()
{
    /*
    millis_t t = now;
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        switch (i)
        {
        case 0:
        {

            if (modbus->mb->R[MB_W_PID_1_SP] > 0)
            {
                //singlePID(states[i].slaveID, ku8MBWriteSingleRegister, OR_E5_SWR::OR_E5_SWR_SP, modbus->mb->R[MB_W_PID_1_SP]);
                modbus->mb->R[MB_W_PID_1_SP] = 0;
                modbus->print();
                states[i].lastWritten = t;
            }
            break;
        }
        case 1:
        {

            if (modbus->mb->R[MB_W_PID_2_SP] > 0)
            {
                //singlePID(states[i].slaveID, ku8MBWriteSingleRegister, OR_E5_SWR::OR_E5_SWR_SP, modbus->mb->R[MB_W_PID_2_SP]);
                modbus->mb->R[MB_W_PID_2_SP] = 0;
                states[i].lastWritten = t;
                return true;
            }
            break;
        }
        case 2:
        {

            if (modbus->mb->R[MB_W_PID_3_SP])
            {
                //singlePID(states[i].slaveID, ku8MBWriteSingleRegister, OR_E5_SWR::OR_E5_SWR_SP, modbus->mb->R[MB_W_PID_3_SP]);
                modbus->mb->R[MB_W_PID_3_SP] = 0;
                states[i].lastWritten = t;
            }
            break;
        }
        }
    }
    */
}

void CRelays::updateTCP()
{
/*
    modbus->mb->R[MB_R_PID_1_PV + MB_REGISTER_OFFSET] = states[0].pv;
    modbus->mb->R[MB_R_PID_2_PV + MB_REGISTER_OFFSET] = states[1].pv;
    modbus->mb->R[MB_R_PID_3_PV + MB_REGISTER_OFFSET] = states[2].pv;
    modbus->mb->R[MB_R_PID_1_SP + MB_REGISTER_OFFSET] = states[0].sp;
    modbus->mb->R[MB_R_PID_2_SP + MB_REGISTER_OFFSET] = states[1].sp;
    modbus->mb->R[MB_R_PID_3_SP + MB_REGISTER_OFFSET] = states[2].sp;
    */
}

short CRelays::setup()
{
    
}

short CRelays::loop()
{

/*
    if (millis() - startTS < 2000)
    {
        return;
    }

    if (modbus->qstate() != IDLE)
    {
        return;
    }

    if (!_did)
    {
        _did = true;
    }
    if (millis() - interval > OMRON_PID_UPDATE_INTERVAL)
    {
        fromTCP();

        interval = now;

        Query *nextCommand = modbus->nextQueryByState(QUERY_STATE::QUEUED);
        if (nextCommand != NULL)
        {
            nextCommand->state = QUERY_STATE::PROCESSING;
            modbus->nextWaitingTime = MODBUS_CMD_WAIT;
            print();
        }
    }
    */
}

short CRelays::debug(Stream *stream)
{
    //*stream << this->name << ":" << this->ok();
    return false;
}
short CRelays::info(Stream *stream)
{
    //*stream << this->name << "\n\t : " SPACE("Pin:" << MOTOR_IDLE_PIN);
    return false;
}

#endif
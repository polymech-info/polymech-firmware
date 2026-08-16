#include "OmronPID.h"
#include "ModbusBridge.h"

#include "./components/OmronE5.h"

bool printModbus = false;
bool printPIDS = false;
bool debugUpdate = false;
bool _updateState = true;
bool printMBErrors = true;
void OmronPID::testPIDs()
{
    // setAllSP(15);
    // runAll();
    // stopAll();
    // singlePIDW(2, OR_E5_SWR::OR_E5_SWR_SP, 300);
    // singlePIDW(1, 5000, 20);
    // singlePID(1, ku8MBWriteSingleRegister, 0, OR_E5_CMD::OR_E5_AT_EXCECUTE);
}

short OmronPID::read10_16(int slaveAddress, int addr, int prio = 0)
{
    Query *same = modbus->nextSame(QUEUED, slaveAddress, addr, ku8MBReadHoldingRegisters, 10);
    if (same != NULL && millis() - same->ts < 1000)
    {
        return;
    }
    if (modbus->numByState(DONE) < 10)
    {
        return;
    }

    if (modbus->numSame(QUEUED, slaveAddress, addr, ku8MBReadHoldingRegisters, 1) > 1)
    {
        return;
    }

    Query *next = modbus->nextQueryByState(DONE);
    if (next != NULL)
    {
        next->fn = ku8MBReadHoldingRegisters;
        next->slave = slaveAddress;
        next->value = 10;
        next->addr = addr;
        next->state = QUERY_STATE::QUEUED;
        next->ts = millis();
        next->prio = prio;
        next->owner = OMRON_PID;
        if (debugUpdate)
        {
            Serial.println(next->slave);
        }
        return E_OK;
    }
    else
    {
        Serial.print("Buffer full");
    }
    return E_QUERY_BUFFER_END;
}
void OmronPID::updateState()
{
    if (!_updateState)
    {
        return;
    }
    OmronState *next = nextToUpdate();
    if (next != NULL)
    {

        modbus->nextWaitingTime = MODBUS_READ_WAIT;
        if (debugUpdate)
        {
            Serial.println("Update slave :");
            Serial.println(next->slaveID);
        }
        next->flags = OmronState::FLAGS::UPDATING;
        read10_16(next->slaveID, 0);
    }
}
short OmronPID::rawResponse(short size, uint8_t rxBuffer[])
{

    Query *current = modbus->nextQueryByState(PROCESSING, OMRON_PID);

    /*
    Serial.print("\n\t  Incoming: ");
    current->print();
    Serial.println(" :: ");
    */
    /*
    for (int i = 0; i < size; i++)
    {
        Serial.print(rxBuffer[i], HEX);
        Serial.print(" : ");
    }

    Serial.print("\n\t  Incoming size : ");
    Serial.print(size);
    Serial.print("\n");
*/
    if (current)
    {
        switch (current->fn)
        {
        case ku8MBWriteSingleRegister:
        {

            if (size == 5 && rxBuffer[1] == OR_E5_RESPONSE_CODE::OR_COMMAND_ERROR)
            {
                Serial.print("------ \n Command Error: ");
                Serial.print(rxBuffer[2]);
                Serial.print(" : ");
                switch (rxBuffer[2])
                {
                case OR_E5_ERROR::VARIABLE_ADDRESS_ERROR:
                {
                    Serial.println(OR_E_MSG_INVALID_ADDRESS);
                    break;
                }
                case OR_E5_ERROR::VARIABLE_RANGE_ERROR:
                {
                    Serial.println(OR_E_MSG_INVALID_RANGE);
                    break;
                }
                case OR_E5_ERROR::VARIABLE_OPERATION_ERROR:
                {
                    Serial.println(OR_E_MSG_OPERATION_ERROR);
                    break;
                }
                }
                Serial.println("\n------");
                return rxBuffer[2];
            }

            if (size == 8 && (rxBuffer[0] != current->slave || rxBuffer[2] != current->addr))
            {
                return OR_COMMAND_ERROR;
            }
            break;
        }
        }
    }
    return ERROR_OK;
}

OmronState *OmronPID::current()
{
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        if (states[i].flags == OmronState::FLAGS::UPDATING)
        {
            return &states[i];
        }
    }
    return NULL;
}

short OmronPID::responseFn(short error)
{

    Query *last = modbus->nextQueryByState(QUERY_STATE::PROCESSING, OMRON_PID);
    if (!last)
    {
        Serial.println("nothing to process !");
        return;
    }
    OmronState *state = pidBySlave(last->slave);
    if (last->fn == ku8MBWriteSingleRegister)
    {
        last->reset();
        if (state)
        {
            state->flags = OmronState::FLAGS::UPDATED;
        }
        return;
    }

    if (state)
    {
        if (state->flags != OmronState::FLAGS::UPDATING)
        {
        }

        if (state->slaveID != last->slave)
        {
            Serial.println("mismatch::wrong slave id -------");
            last->print();
            last->reset();
            return;
        }

        state->lastUpdated = millis();
        state->statusHigh = modbus->ModbusSlaveRegisters[2];
        state->statusLow = modbus->ModbusSlaveRegisters[3];
        state->pv = modbus->ModbusSlaveRegisters[1];
        state->sp = modbus->ModbusSlaveRegisters[5];
        state->flags = OmronState::FLAGS::UPDATED;
        state->ready = true;
        if (printPIDS)
        {
            Serial.print("Updated SlaveID: ");
            Serial.print(state->slaveID);
            Serial.println("");
            print();
        }
        last->reset();
        updateTCP();
    }
    else
    {
        Serial.print("Invalid current PID: ");
        Serial.println(last->slave);
    }
}
void OmronPID::print()
{

    printStates();
}
void OmronPID::fromTCP()
{
    millis_t t = now;
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        switch (i)
        {
        case 0:
        {

            if (modbus->mb->R[MB_W_PID_1_SP] > 0)
            {
                singlePID(states[i].slaveID, ku8MBWriteSingleRegister, OR_E5_SWR::OR_E5_SWR_SP, modbus->mb->R[MB_W_PID_1_SP]);
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
                singlePID(states[i].slaveID, ku8MBWriteSingleRegister, OR_E5_SWR::OR_E5_SWR_SP, modbus->mb->R[MB_W_PID_2_SP]);
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
                singlePID(states[i].slaveID, ku8MBWriteSingleRegister, OR_E5_SWR::OR_E5_SWR_SP, modbus->mb->R[MB_W_PID_3_SP]);
                modbus->mb->R[MB_W_PID_3_SP] = 0;
                states[i].lastWritten = t;
            }
            break;
        }
        }
    }
}

void OmronPID::updateTCP()
{

    modbus->mb->R[MB_R_PID_1_PV + MB_REGISTER_OFFSET] = states[0].pv;
    modbus->mb->R[MB_R_PID_2_PV + MB_REGISTER_OFFSET] = states[1].pv;
    modbus->mb->R[MB_R_PID_3_PV + MB_REGISTER_OFFSET] = states[2].pv;

    modbus->mb->R[MB_R_PID_1_SP + MB_REGISTER_OFFSET] = states[0].sp;
    modbus->mb->R[MB_R_PID_2_SP + MB_REGISTER_OFFSET] = states[1].sp;
    modbus->mb->R[MB_R_PID_3_SP + MB_REGISTER_OFFSET] = states[2].sp;
}

short OmronPID::queryResponse(short error)
{
    Query *last = modbus->nextQueryByState(QUERY_STATE::PROCESSING);
    if (last)
    {
        last->state = QUERY_STATE::DONE;
    }
}

int OmronPID::singlePIDW(int slave, int addr, int value)
{
    singlePID(slave, ku8MBWriteSingleRegister, addr, value);
}
int OmronPID::singlePID(int slave, short fn, int addr, int value)
{
    Query *same = modbus->nextSame(QUEUED, slave, addr, fn, value);
    if (modbus->numByState(DONE) < 2 && fn != ku8MBWriteSingleRegister)
    {
        return false;
    }
    if (modbus->numSame(QUEUED, slave, addr, fn, value) > 1)
    {
        return false;
    }

    OmronState *pid = pidBySlave(slave);
    if (pid)
    {
        Query *next = modbus->nextQueryByState(DONE);
        if (next)
        {
            next->fn = fn;
            next->slave = pid->slaveID;
            next->value = value;
            next->addr = addr;
            next->state = QUERY_STATE::QUEUED;
            if (fn == ku8MBWriteSingleRegister)
            {
                next->prio = MB_QUERY_TYPE_CMD;
            }
            return E_OK;
        }
    }
    else
    {
        Serial.println("No such PID");
        return E_NO_SUCH_PID;
    }
}

int OmronPID::eachPIDW(int addr, int value)
{
    return eachPID(ku8MBWriteSingleRegister, addr, value);
}

int OmronPID::eachPID(short fn, int addr, int value)
{
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        Query *next = modbus->nextQueryByState(DONE);
        if (next)
        {
            next->fn = fn;
            next->slave = states[i].slaveID;
            next->value = value;
            next->addr = addr;
            next->state = QUERY_STATE::QUEUED;
        }
        else
        {
            Serial.println("no buffer free");
        }
    }
}
OmronState *OmronPID::pidBySlave(int slave)
{
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        if (states[i].slaveID == slave)
        {
            return &states[i];
        }
    }
    return NULL;
}

void OmronPID::stopAll()
{
    eachPID(ku8MBWriteSingleRegister, 0, OR_E5_CMD::OR_E5_STOP);
}

void OmronPID::runAll()
{
    eachPID(ku8MBWriteSingleRegister, 0, OR_E5_CMD::OR_E5_RUN);
}

void OmronPID::setAllSP(int sp)
{
    eachPID(ku8MBWriteSingleRegister, OR_E5_SWR::OR_E5_SWR_SP, sp);
}

short OmronPID::setup()
{
    statusLight.off();
}

// for manual testing
bool did = false;

short OmronPID::loop()
{

    if (millis() - startTS < 2000)
    {
        return;
    }

    statusLight.loop();

    if (modbus->qstate() != IDLE)
    {
        return;
    }

    if (!did)
    {
        testPIDs();
        did = true;
    }
    if (millis() - interval > OMRON_PID_UPDATE_INTERVAL)
    {
        fromTCP();

        updateState();

        interval = now;

        Query *nextCommand = modbus->nextQueryByState(QUERY_STATE::QUEUED);
        if (nextCommand != NULL)
        {
            if (printModbus)
            {
                modbus->print();
            }

            nextCommand->state = QUERY_STATE::PROCESSING;
            modbus->nextWaitingTime = MODBUS_CMD_WAIT;
            modbus->onMessage = (AddonRxFn)&OmronPID::rawResponse;
            modbus->onError = (AddonFnPtr)&OmronPID::onError;
            if (debugUpdate)
            {
                Serial.print("query slave : ");
                Serial.print(nextCommand->slave);
                Serial.print(" qid: ");
                Serial.print(nextCommand->id);
                Serial.print(" ts: ");
                Serial.print(nextCommand->ts);
                Serial.print(" fn: ");
                Serial.print(nextCommand->fn);
                Serial.println("----");
            }
            modbus->query(nextCommand->slave, nextCommand->fn, nextCommand->addr, nextCommand->value, this, (AddonFnPtr)&OmronPID::responseFn);

            print();
            if (!isRunning())
            {
                statusLight.setBlink(false);
                statusLight.off();
                return;
            }
            if (isHeatingUp())
            {
                statusLight.setBlink(true);
            }
            else
            {
                statusLight.setBlink(false);
                statusLight.on();
            }
        }
    }
}

short OmronPID::onError(short error)
{
    if (printMBErrors)
    {
        Serial.print("Omron PID :: onError ");
        if (error == 255)
        {
            Serial.println("Timeout");
        }
        else
        {
            Serial.println(error);
        }
    }
    Query *last = modbus->nextQueryByState(QUERY_STATE::PROCESSING, OMRON_PID);
    if (last)
    {
        last->reset();
    }
    else
    {
        Serial.println("Omron PID :: onError - can't find last query! ");
    }
    resetStates();
}
void OmronPID::resetStates()
{
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        states[i].flags = OmronState::FLAGS::UPDATED;
    }
}
OmronState *OmronPID::nextToUpdate()
{
    OmronState *oldest = NULL;
    bool isUpdating = false;
    millis_t t = millis();
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        if (states[i].flags == OmronState::FLAGS::UPDATING)
        {
            continue;
        }

        if (!oldest)
        {
            oldest = &states[i];
        }

        /*
        if (&states[i] != oldest && states[i].lastUpdated < oldest->lastUpdated)
        {
            oldest = &states[i];
        }
        */

        if (millis() - states[i].lastUpdated > OMRON_PID_UPDATE_INTERVAL * 2)
        {
            oldest = &states[i];
        }

        if (states[i].flags == OmronState::FLAGS::UPDATING)
        {
            isUpdating = true;
        }
    }

    if (isUpdating)
    {
        return NULL;
    }

    return oldest;
}

bool OmronPID::isHeatingUp()
{
    bool ret = false;
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        if (states[i].isHeating())
        {
            return true;
        }
    }
    return ret;
}
bool OmronPID::isRunning()
{
    bool ret = false;
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        if (states[i].isRunning())
        {
            return true;
        }
    }
    return ret;
}
void OmronPID::printStates()
{
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        states[i].print();
    }
}

OmronState *OmronPID::nextToWrite()
{
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        if (millis() - states[i].lastWritten > OMRON_PID_WRITE_INTERVAL)
        {
            return &states[i];
        }
    }
    return NULL;
}

short OmronPID::debug(Stream *stream)
{
    //*stream << this->name << ":" << this->ok();
    return false;
}
short OmronPID::info(Stream *stream)
{
    //*stream << this->name << "\n\t : " SPACE("Pin:" << MOTOR_IDLE_PIN);
    return false;
}

void OmronPID::initPIDS()
{
    for (short i = 0; i < NB_OMRON_PIDS; i++)
    {
        states[i].slaveID = slaveStart + i;
        states[i].idx = i;
        states[i].lastUpdated = millis();
        states[i].lastWritten = millis();
        states[i].flags = OmronState::FLAGS::UPDATED;
    }
}

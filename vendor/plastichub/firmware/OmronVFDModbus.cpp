#include "OmronVFD.h"
#include "ModbusBridge.h"
#include "./components/OmronMX2.h"
#include "app.h"

////////////////////////////////////////////////////////////////////////////
//
// Modbus
bool didTest = true;
bool debugReceive = false;
bool debugSend = false;
bool debugFilter = true;
bool debugMultiRegs = false;
bool debugModQueries = false;
bool printErrors = true;

short OmronVFD::onError(short error)
{
    if (printErrors)
    {
        Serial.print("Omron VFD onError : ");
        if (error == 255)
        {
            Serial.println("Timeout");
        }
        else
        {
            Serial.println(error);
        }
    }
    Query *last = modbus->nextQueryByState(QUERY_STATE::PROCESSING, OMRON_VFD);
    if (last)
    {
        last->reset();
    }
    else
    {
        Serial.println("Omron VDF :: onError - can't find last query! ");
    }
}
void OmronVFD::modbusLoop()
{

    if (!didTest)
    {
        didTest = true;
        doTest();
    }

    updateState();

    if (millis() - interval > OMRON_MX2_LOOP_INTERVAL)
    {
        interval = now;
        if (ready)
        {
            fromTCP();
        }
        Query *nextCommand = modbus->nextQueryByState(QUERY_STATE::QUEUED, OMRON_VFD);
        if (nextCommand)
        {
            if (modbus->qstate() != IDLE)
            {
                return;
            }

            nextCommand->state = QUERY_STATE::PROCESSING;
            modbus->nextWaitingTime = MODBUS_CMD_WAIT;
            modbus->onMessage = (AddonRxFn)&OmronVFD::rawResponse;
            modbus->onError = (AddonFnPtr)&OmronVFD::onError;

            if (debugSend)
            {
                if (now - debugTS > OMRON_MX2_DEBUG_INTERVAL)
                {
                    debugTS = now;
                    Serial.print("next to send ");
                    Serial.print(nextCommand->id);
                    Serial.print(" | ");
                    Serial.print(nextCommand->ts);
                    Serial.print(" | Addr=");
                    Serial.print(nextCommand->addr);
                    Serial.print(" | Value=");
                    Serial.print(nextCommand->value);
                    Serial.print(" | FN=");
                    Serial.print(nextCommand->fn);
                    Serial.print("\n");

                    if (debugModQueries)
                    {
                        modbus->print();
                    }
                }
            }

            modbus->query(nextCommand->slave, nextCommand->fn, nextCommand->addr, nextCommand->value, this, (AddonFnPtr)&OmronVFD::queryResponse);
            return;
        }
    }
}

short OmronVFD::ping()
{

    Query *next = modbus->nextQueryByState(DONE);
    if (next)
    {
        next->fn = ku8MBLinkTestOmronMX2Only;
        next->slave = slaveAddress;
        next->value = 1234;
        next->addr = 0;
        next->state = QUERY_STATE::QUEUED;
        next->ts = millis();
        return E_OK;
    }
    return E_QUERY_BUFFER_END;
}

short OmronVFD::rawResponse(short size, uint8_t rxBuffer[])
{
    if (!size)
    {
        return E_OK;
    }
    if (debugReceive)
    {
        Serial.print("\nIncoming:");
        Serial.print(size);
        Serial.print("::\t");

        for (int i = 0; i < size; i++)
        {
            Serial.print(rxBuffer[i], HEX);
            Serial.print(" : ");
        }
        Serial.print("\n");
    }
    if (size == 5)
    {
        Serial.println("Error");
    }

    return ERROR_OK;
}

short OmronVFD::responseFn(short error)
{
}

short OmronVFD::queryResponse(short error)
{
    Query *last = modbus->nextQueryByState(QUERY_STATE::PROCESSING, OMRON_VFD);
    if (last)
    {
        long first = modbus->ModbusSlaveRegisters[0];
        if (last->prio == MB_QUERY_TYPE_STATUS_POLL)
        {

            states[0].state = modbus->ModbusSlaveRegisters[3];
            states[0].status = modbus->ModbusSlaveRegisters[2];
            states[0].FC = modbus->ModbusSlaveRegisters[0];
            ready = true;
            updateTCP();
            if (debugMultiRegs)
            {
                Serial.print(" - regs : \n ");
                for (int i = 0; i < 5; i++)
                {
                    Serial.print(" - ");
                    Serial.print(modbus->ModbusSlaveRegisters[i]);
                    Serial.print("\n");
                }
            }
        }

        last->addr = 0;
        last->value = 0;
        last->slave = 0;
        last->ts = 0;
        last->prio = 0;
        last->state = QUERY_STATE::DONE;
    }
    else
    {
        Serial.println("state error, had nothing to process");
    }
}

void OmronVFD::updateTCP()
{

    modbus->mb->R[MB_R_VFD_STATUS] = states[0].status;
    modbus->mb->R[MB_R_VFD_STATE] = states[0].state;
    modbus->mb->R[MB_R_FREQ_TARGET] = states[0].FC;
    // fromTCP();
}

void OmronVFD::fromTCP()
{
    if (modbus->mb->R[MB_W_VFD_RUN] == 1)
    {
        onStart();
        write_Bit(MX2_START, 1);
        modbus->mb->R[MB_W_VFD_RUN] = 0;
    }
    if (modbus->mb->R[MB_W_VFD_RUN] == 2)
    {
        onStop();
        write_Bit(MX2_START, 0);
        modbus->mb->R[MB_W_VFD_RUN] = 0;
    }

    if (modbus->mb->R[MB_W_VFD_RUN] == 2)
    {
        onStop();
        modbus->mb->R[MB_W_VFD_RUN] = 0;
        write_Bit(MX2_START, 0);
    }

    if (modbus->mb->R[MB_W_DIRECTION] > 0)
    {
        switch (modbus->mb->R[MB_W_DIRECTION])
        {
        case 1:
            forward();
            break;
        case 2:
            reverse();
            break;
        default:
            stop();
            break;
        }
        modbus->mb->R[MB_W_DIRECTION] = 0;
    }

    if (states[0].state == OMRON_STATE_DECELERATING || states[0].state == OMRON_STATE_ACCELERATING)
    {
        status.setBlink(true);
    }

    if (states[0].state == OMRON_STATE_RUNNING)
    {
        status.setBlink(false);
        status.on();
    }

    if (states[0].state == OMRON_STATE_STOPPED)
    {
        status.setBlink(false);
        status.off();
    }

    if (modbus->mb->R[MB_W_FREQ_TARGET] > 0)
    {
        setTargetFreq(modbus->mb->R[MB_W_FREQ_TARGET]);
        modbus->mb->R[MB_W_FREQ_TARGET] = 0;
    }
}

uint16_t OmronVFD::write_Single(uint16_t addr, unsigned int data)
{

    Query *next = modbus->nextQueryByState(DONE);
    if (next)
    {
        next->fn = ku8MBWriteSingleRegister;
        next->slave = slaveAddress;
        // modbus->setDebugSend(true);
        next->value = data;
        next->addr = addr;
        next->state = QUERY_STATE::QUEUED;
        next->ts = millis();
        next->owner = OMRON_VFD;
        next->prio = MB_QUERY_TYPE_CMD;
        return E_OK;
    }
    return E_QUERY_BUFFER_END;
}

uint16_t OmronVFD::write_Bit(uint16_t addr, int on)
{
    Query *same = modbus->nextSame(QUEUED, slaveAddress, addr, ku8MBWriteSingleCoil, on);
    if (same && millis() - same->ts < 300)
    {
    }

    Query *next = modbus->nextQueryByState(DONE);
    if (next)
    {
        next->fn = ku8MBWriteSingleCoil;
        next->slave = slaveAddress;
        next->addr = addr;
        // modbus->setDebugSend(true);
        next->value = on;
        next->state = QUERY_STATE::QUEUED;
        next->ts = millis();
        next->owner = OMRON_VFD;
        next->prio = MB_QUERY_TYPE_CMD;
        return E_OK;
    }
    return E_QUERY_BUFFER_END;
}

short OmronVFD::readSingle_16(int addr, int prio = 0)
{

    Query *same = modbus->nextSame(QUEUED, slaveAddress, addr, ku8MBReadHoldingRegisters, 1);
    if (same && millis() - same->ts < OMRON_MX2_SAME_REQUEST_INTERVAL)
    {
        return;
    }
    if (modbus->numByState(DONE) < MODBUS_QUEUE_MIN_FREE)
    {
        return;
    }

    if (modbus->numSame(QUEUED, slaveAddress, addr, ku8MBReadHoldingRegisters, 1) > 1)
    {
        return;
    }

    Query *next = modbus->nextQueryByState(DONE);
    if (next)
    {
        next->fn = ku8MBReadHoldingRegisters;
        next->slave = slaveAddress;
        next->value = 1;
        next->addr = addr;
        next->state = QUERY_STATE::QUEUED;
        next->ts = millis();
        next->prio = prio;
        next->owner = OMRON_VFD;
        return E_OK;
    }

    return E_QUERY_BUFFER_END;
}

short OmronVFD::read_16(int addr, int num, int prio = 0)
{

    Query *same = modbus->nextSame(QUEUED, slaveAddress, addr, ku8MBReadHoldingRegisters, 1);

    if (same && millis() - same->ts < OMRON_MX2_SAME_REQUEST_INTERVAL)
    {
        return;
    }

    if (modbus->numByState(DONE) < MODBUS_QUEUE_MIN_FREE)
    {
        return;
    }

    if (modbus->numSame(QUEUED, slaveAddress, addr, ku8MBReadHoldingRegisters, 1) > 1)
    {
        return;
    }

    Query *next = modbus->nextQueryByState(DONE);
    if (next)
    {
        next->fn = ku8MBReadHoldingRegisters;
        next->slave = slaveAddress;
        next->value = num;
        next->addr = addr;
        next->state = QUERY_STATE::QUEUED;
        next->ts = millis();
        next->prio = prio;
        next->owner = OMRON_VFD;
        return E_OK;
    }
    return E_QUERY_BUFFER_END;
}

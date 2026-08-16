#include <Streaming.h>
#include "./Addon.h"
#include "ModbusRtu.h"
#include "ModbusBridge.h"
#include "config.h"

#include <SPI.h>
#include <Ethernet.h>

#define RS485Serial 3

#define MasterModbusAdd 0
#define SlaveModbusAdd 1

Modbus master(MasterModbusAdd, RS485Serial);

modbus_t ModbusQuery[1];
uint16_t ModbusSlaveRegisters[8];
millis_t WaitingTime;

int _state = IDLE;

bool debugQuery = true;

void ModbusBridge::setDebugSend(bool debug)
{
    master.debugSend = debug;
}

Modbus *ModbusBridge::modbus()
{
    return &master;
}
// Modbus TCP
short ModbusBridge::setup()
{
    master.begin(MODBUS_RS485_BAUDRATE, MODBUS_RS485_PORT);
    master.setTimeOut(MODBUS_RS485_TIMEOUT);
    WaitingTime = millis() + nextWaitingTime;
    _state = IDLE;

    Ethernet.begin(MB_MAC, MB_IP, MB_GATEWAY, MB_SUBNET);

    for (uchar i = 0; i < MAX_QUERY_BUFFER; i++)
    {
        queries[i].reset();
        queries[i].id = i;
    }

    for (uchar i = 0; i < 50; i++)
    {
        mb->R[i] = MODBUS_TCP_DEFAULT_REGISTER_VALUE;
    }

    startTS = millis();
}

Query *ModbusBridge::nextByPrio(uchar state, int prio)
{

    Query *oldest;
    for (int i = 0; i < MAX_QUERY_BUFFER; i++)
    {
        if (!oldest)
        {
            oldest = &queries[i];
        }

        if (queries[i].state == state && queries[i].prio == prio)
        {
            oldest = &queries[i];
        }
    }
    return oldest;
}

bool didm = false;
Query *ModbusBridge::nextQueryByState(uchar state = DONE, int owner = -1)
{

    if (owner > 0)
    {
        Query *q = nextQueryByOwner(state, owner);
        if (q != NULL)
        {
            return q;
        }
    }

    millis_t t = millis();
    Query *oldest = NULL;

    for (int i = 0; i < MAX_QUERY_BUFFER; i++)
    {
        if (queries[i].state == state)
        {

            if (_state == QUEUED && t - queries[i].ts > 1000 * 10)
            {
                queries[i].reset();
                continue;
            }

            if (_state == PROCESSING && t - queries[i].ts > 1000 * 10)
            {
                queries[i].reset();
                continue;
            }

            if (queries[i].ts == 0)
            {
                queries[i].ts = t;
            }

            if (queries[i].prio != MB_QUERY_TYPE_CMD && t - queries[i].ts > 1000)
            {
                return &queries[i];
            }
            
            if (!oldest)
            {
                oldest = &queries[i];
            }

            if (queries[i].ts > oldest->ts)
            {
                oldest = &queries[i];
            }
        }
    }
    return oldest;
}

Query *ModbusBridge::nextQueryByOwner(uchar state = DONE, int owner = -1)
{
    millis_t t = millis();
    Query *oldest = NULL;
    for (int i = 0; i < MAX_QUERY_BUFFER; i++)
    {
        if (queries[i].state == state && queries[i].owner == owner)
        {

            if (_state == QUEUED && t - queries[i].ts > 1000 * 10)
            {
                queries[i].reset();
                continue;
            }

            if (_state == PROCESSING && t - queries[i].ts > 1000 * 10)
            {
                queries[i].reset();
                Serial.println("reset processing");
                continue;
            }
            if (queries[i].ts == 0)
            {
                queries[i].ts = t;
            }

            if (queries[i].prio == MB_QUERY_TYPE_CMD && t - queries[i].ts > 300)
            {
                return &queries[i];
            }

            if (!oldest)
            {
                oldest = &queries[i];
            }

            if (queries[i].ts > oldest->ts)
            {
                oldest = &queries[i];
            }
        }
    }
    return oldest;
}

Query *ModbusBridge::nextSame(uchar state, short slave, int addr, short fn, int value)
{
    millis_t t = millis();
    Query *oldest;
    for (int i = 0; i < MAX_QUERY_BUFFER; i++)
    {
        if (queries[i].state == state)
        {
            Query *q = &queries[i];
            if (q->addr == addr && q->fn == fn && q->value == value && q->slave == slave)
            {
                if (!oldest)
                {
                    oldest = &queries[i];
                }
                if (queries[i].ts > oldest->ts)
                {
                    oldest = &queries[i];
                }
            }
        }
    }
    return oldest;
}
int ModbusBridge::numSame(uchar state, short slave, int addr, short fn, int value)
{
    int num = 0;
    for (int i = 0; i < MAX_QUERY_BUFFER; i++)
    {
        if (queries[i].state == state)
        {
            Query *q = &queries[i];
            if (q->addr == addr && q->fn == fn && q->value == value && q->slave == slave)
            {
                num++;
            }
        }
    }
    return num;
}
int ModbusBridge::numByState(int state = DONE)
{
    int num = 0;
    for (int i = 0; i < MAX_QUERY_BUFFER; i++)
    {
        if (queries[i].state == state)
        {
            num++;
        }
    }
    return num;
}
void ModbusBridge::print()
{
    Serial.print("----- Queries : --- ");
    Serial.print("Proccessing : ");
    Serial.print(numByState(PROCESSING));
    Serial.print(" | QUEUED : ");
    Serial.print(numByState(QUEUED));
    Serial.print(" | DONE: ");
    Serial.print(numByState(DONE));

    Serial.print(" | ADDR: ");
    Serial.print(addr);
    Serial.print(" | FN: ");
    Serial.print(fn);
    Serial.print(" | NOW : ");
    Serial.print(millis());

    Serial.print("-----\n");

    for (int i = 0; i < MAX_QUERY_BUFFER; i++)
    {
        Serial.print(" - ");
        Serial.print(queries[i].id);
        Serial.print(". \t ");
        queries[i].print();
        Serial.print("\n");
    }
}
short ModbusBridge::qstate()
{
    return _state;
}
short ModbusBridge::loop()
{
    loop_test();
    mb->Run();
    if (mb->R[9])
    {
        print();
        mb->R[9] = 0;
    }
}

short ModbusBridge::query(int slave, short function, long start, int coils, Addon *_addon, AddonFnPtr _mPtr)
{
    if (_state != IDLE)
    {
        return WAITING;
    }

    addr = 0;
    id = slave;
    fn = function;
    addr = start;
    nb = coils;
    owner = _addon;
    updatedPtr = _mPtr;

    if (debugQuery)
    {
        Serial.print("\n --------------Modbus QUERY --------- SLAVE :  ");
        Serial.print(id);
        Serial.print(" | FN : ");
        Serial.print(fn);
        Serial.print(" | NB : ");
        Serial.print(coils);
        Serial.print(" | Address : ");
        Serial.print(addr, HEX);

        Serial.print(" | STATE : ");
        Serial.print(_state);

        Serial.print(" | OWNER : ");
        Serial.println(owner->id);
        Serial.println(" \n ");
    }

    _state = WAITING;

    return E_OK;
}

short ModbusBridge::loop_test()
{
    switch (_state)
    {

    case IDLE:
    {
        return;
    }

    case WAITING:
    {
        if (millis() > WaitingTime)
        {
            _state++; // set to query state
        }
        break;
    }
    case QUERY:
    {
        ModbusQuery[0].u8id = id;                      // slave address
        ModbusQuery[0].u8fct = fn;                     // function code (this one is registers read)
        ModbusQuery[0].u16RegAdd = addr;               // start address in slave
        ModbusQuery[0].u16CoilsNo = nb;                // number of elements (coils or registers) to read
        ModbusQuery[0].au16reg = ModbusSlaveRegisters; // pointer to a memory array in the CONTROLLINO
        master.query(ModbusQuery[0]);                  // send query (only once)
        _state++;                                      // set to RESPONSE
        break;
    }
    case RESPONSE:
    {
        master.poll(); // check incoming messages
        if (master.getState() == COM_IDLE)
        {

            int errors = master.getErrCnt();
            if (errors)
            {

                if (owner && onError != NULL)
                {
                    (owner->*onError)(master.getLastError());
                    master.clearError();
                }
                else
                {
                    Serial.print("ModbusBridge:: Have Errors : ");
                    Serial.println(master.getLastError());
                }
                _state = IDLE;
                return;
            }

            long onMessageError = 0;
            if (owner && onMessage)
            {
                onMessageError = (owner->*onMessage)(master.rxSize, master.rxBuffer);
            }

            short ret = (owner->*updatedPtr)(onMessageError);
            WaitingTime = millis() + nextWaitingTime;

            if (TEST(debug_flags, DEBUG_RECEIVE) && onMessageError == ERROR_OK)
            {
            }

            /*
            Serial.print("--------------Modbus RESPONSE --------- FN :  ");
            Serial.print(fn);
            Serial.print(" | NB : ");
            Serial.print(nb);

            Serial.print(" | SLAVE : ");
            Serial.print(id);

            Serial.print(" | Address : ");
            Serial.println(addr);
            */

            _state = IDLE;
        }
        break;
    }
    }
}

short ModbusBridge::debug(Stream *stream)
{
    // *stream << this->name << ":";
    return false;
}

short ModbusBridge::info(Stream *stream)
{
    // *stream << this->name << "\n\t";
}
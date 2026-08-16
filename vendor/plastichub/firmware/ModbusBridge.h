#ifndef MODBUS_BRIDGE_H
#define MODBUS_BRIDGE_H

#include "Addon.h"
#include <Controllino.h>
#include "Mudbus.h"

#define MAX_QUERY_BUFFER 20
class Modbus;

// query struct
class Query
{
public:
    int slave;
    long addr;
    long value;
    int state;
    long fn;
    millis_t ts;
    int id;
    int prio;
    int owner;

    long printNumber(int number)
    {
        Serial.print("   ");
        int spaces = 0;
        if (number < 1000)
        {
            spaces = 1;
        }
        if (number < 100)
        {
            spaces = 2;
        }
        if (number < 10)
        {
            spaces = 3;
        }
        for (int i = 0; i < spaces; i++)
        {
            Serial.print(" ");
        }
        Serial.print(number);
    }

    void print()
    {
        Serial.print("SLAVE: ");
        Serial.print(slave);
        Serial.print(" \t | Address: ");
        // Serial.print(addr, HEX);
        printNumber(addr);
        Serial.print(" \t | VALUE:  ");
        printNumber(value);
        Serial.print(" \t | STATE ");
        if (state == DONE)
        {
            Serial.print("Done       ");
        }
        if (state == PROCESSING)
        {
            Serial.print("Processsing");
        }
        if (state == QUEUED)
        {
            Serial.print("Queued     ");
        }
        Serial.print(" \t | FN: ");
        Serial.print(fn);
        Serial.print(" \t | PRIO: ");
        Serial.print(prio);
        Serial.print(" | ");

        Serial.print(" \t | OWNER: ");
        Serial.print(owner);
        Serial.print(" | ");
    }
    Query()
    {
        reset();
    }
    void reset()
    {
        state = DONE;
        fn = 0;
        ts = 0;
        value = 0;
        slave = 0;
        addr = 0;
        prio = 0;
        owner = 0;
    }
};

class ModbusBridge : public Addon
{

public:
    ModbusBridge() : Addon("ModbusBridge", 50, ADDON_NORMAL),
                     mb(new Mudbus())
    {
        setFlag(DEBUG);
        debug_flags = 0;
        debug_flags = 1 << DEBUG_RECEIVE;
        nextWaitingTime = 1000;
    }

    uint16_t ModbusSlaveRegisters[8];

    // Addon std implementation
    short debug(Stream *stream);
    short info(Stream *stream);
    short setup();
    short loop();
    short loop_test();

    // current query
    short id;
    short fn;
    short addr;
    int nb;

    long debug_flags;
    short queryState();
    short query(int slave, short function, long start, int coils, Addon *_addon, AddonFnPtr _mPtr);
    short qstate();

    // 0x6 callback
    AddonFnPtr updatedPtr;
    // on Error
    AddonFnPtr onError;

    // on RawMessage
    AddonRxFn onMessage;

    // callback owner
    Addon *owner;

    int nextWaitingTime;

    Mudbus *mb;

    // Modbus query / commands
    Query *nextQueryByState(uchar state = DONE, int owner = -1);
    Query *nextQueryByOwner(uchar state = DONE, int owner = -1);

    Query *nextByPrio(uchar state, int prio);

    Query *nextSame(uchar state, short slave, int addr, short fn, int value);

    int numSame(uchar state, short slave, int addr, short fn, int value);
    int numByState(int state);
    void print();
    enum FLAGS
    {
        DEBUG_RECEIVE = 1,
        DEBUG_SEND = 2,
    };

    Query queries[MAX_QUERY_BUFFER];
    millis_t startTS;
    Modbus *modbus();
    void setDebugSend(bool debug);
    ////////////////////////////////////////////////////////////////
    //
    // TCP Gateway
    //
};

#endif
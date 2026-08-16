#ifndef ADDON_H
#define ADDON_H

#include <WString.h>
#include <Vector.h>

#include "enums.h"
#include "common/macros.h"
#include "common/timer.h"

#define ADDON_NORMAL 1 << LOOP | 1 << INFO | 1 << SETUP
#ifdef HAS_STATES
#define ADDON_STATED ADDON_NORMAL | 1 << STATE
#else
#define ADDON_STATED ADDON_NORMAL
#endif

class Stream;
class App;

class Addon
{

public:
    const String name;
    const short id;
    millis_t now;
    millis_t last;
    millis_t dt;
    Addon(String _name, short _id) : name(_name),
                                     id(_id),
                                     now(0),
                                     last(0),
                                     dt(0)
    {
        flags = ADDON_NORMAL;
    }

    Addon(String _name, short _id, short _flags) : name(_name),
                                                   id(_id),
                                                   flags(_flags)
    {
    }

    virtual short debug(Stream *stream);
    virtual short info(Stream *stream);
    virtual short setup(){ return 0; };
    virtual short loop(){ return 0; };
    virtual short ok(){ return 0; };
    virtual bool pause(){ return 0; };
    virtual bool resume(){ return 0; };
    virtual bool destroy(){ return 0; };
    virtual String state() { return ""; };

    int flags;
    void setFlag(uchar flag);
    bool hasFlag(uchar flag);
    void clearFlag(uchar flag);
    void enable();
    void disable();
    bool enabled();
};

typedef Vector<Addon *> Addons;
Addon *byId(Addons addons, uchar id);
typedef short (Addon::*AddonFnPtr)(short);
typedef short (Addon::*AddonRxFn)(short size, uint8_t rxBuffer[]);

#endif
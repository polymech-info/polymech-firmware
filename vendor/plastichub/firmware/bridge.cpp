#include "bridge.h"
#include <Vector.h>
#include <Streaming.h>
#include "constants.h"

typedef struct
{
    short key;
    char *value;
} TAddons;

const TAddons addonsDict[]{};

static VSL *_instance;
class SKeyVal;
SKeyVal *clazzMaps[20];
Vector<SKeyVal *> clazzes;

class SKeyValS
{
public:
    void *instance;
    ushort id;
    SKeyValS() {}
    SKeyValS(void *_instance, ushort _id) : instance(_instance),
                                            id(_id) {}
};

char *getAddonName(short key)
{
    for (uchar i = 0; i < sizeof(addonsDict) / sizeof(TAddons); ++i)
    {
        if (addonsDict[i].key == key)
        {
            return addonsDict[i].value;
        }
    }
    return NULL;
}
short getAddonKey(String name)
{
    for (uchar i = 0; i < sizeof(addonsDict) / sizeof(TAddons); ++i)
    {
        if (name.equals(String(addonsDict[i].value)))
        {
            return addonsDict[i].value;
        }
    }
    return -1;
}

void VSL::init()
{
    _instance = new VSL();
    clazzes.setStorage(clazzMaps);
}

VSL *VSL::instance()
{
    if (!_instance)
    {
        VSL::init();
    }
    return _instance;
}

SKeyVal *VSL::hasMethod(ushort id, String methodName)
{
    uchar s = clazzes.size();
    for (uchar i = 0; i < s; i++)
    {

        SKeyVal *val = clazzes.at(i);
        if (val->key == id && val->methodName.equals(methodName))
        {
            return val;
        }
    }
    return NULL;
}
void VSL::debug()
{
#ifdef BRIDGE_DEBUG
    uchar s = clazzes.size();
    for (uchar i = 0; i < s; i++)
    {
        SKeyVal *val = clazzes.at(i);
        Serial.print(val->className);
        Serial.print(":");
        Serial.print(val->methodName);
        Serial.print("\n");
    }
#endif
}

SKeyVal *VSL::registerMemberFunction(ushort id, Addon *clazz, char *method, AddonFnPtr ptr, char *ret)
{
    SKeyVal *meth = hasMethod(id, method);
    if (meth)
    {
#ifdef BRIDGE_DEBUG
        Serial << "Register class member: "
               << SPACE(name << "::" << method)
               << "already registered! \n";
#endif
    }
    else
    {
#ifdef BRIDGE_DEBUG
        if (!getAddonKey(name))
        {
            Serial.println("invalid addon key");
        }
#endif
        meth = new SKeyVal(id, clazz, method, ptr);
        //Serial << "Register member method:"
        //       << SPACE(meth->className << "::" << meth->methodName)
        //       << "\n";
        clazzes.push_back(meth);
    }
}

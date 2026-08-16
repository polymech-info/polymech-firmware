#ifndef BRDIGE_H
#define BRIDGE_H

#include "Addon.h"
#include <WString.h>
class App;
class SKeyVal;
class SKeyValS;

class SKeyVal
{
public:
    short key;
    void *instance;
    String methodName;
    AddonFnPtr mPtr;
    SKeyVal() {}
    SKeyVal(ushort _key, void *_instance, String _methodName, AddonFnPtr _mPtr) : key(_key),
                                                                                  instance(_instance),
                                                                                  methodName(_methodName),
                                                                                  mPtr(_mPtr) {}
};
class VSL
{
public:
    SKeyVal *registerMemberFunction(
        ushort id,
        Addon *clazz,
        char *method,
        AddonFnPtr ptr,
        char *ret);

    static void init();
    static VSL *instance();
    SKeyVal *VSL::hasMethod(ushort id, String method);
    void debug();
};

#define REGISTER_CLASS_MEMBER_FN(id, inst, methodName, method, ret)                 \
    {                                                                               \
        VSL::instance()->registerMemberFunction(id, inst, methodName, method, "2"); \
    }
#endif

namespace Bridge
{

static const char *START_STR = "<<";
static const char *END_STR = ">>";
static const char RESPONSE_DEL = ';';
static const char *CreateResponse(short id, short error, short ret)
{
    static char response[1024] = {'\0'};
    snprintf(response, sizeof(response), "%s%d%c%d%c%d%s\r", START_STR, id, RESPONSE_DEL, error, RESPONSE_DEL, ret, END_STR);
    return response;
}
static const char *CreateResponse(short id, short error, const char *ret)
{
    static char response[1024] = {'\0'};
    snprintf(response, sizeof(response), "%s%d%c%d%c%s%s\r", START_STR, id, RESPONSE_DEL, error, RESPONSE_DEL, ret, END_STR);
    return response;
}

enum ECALLS
{
    EC_COMMAND = 1,
    EC_METHOD = 2,
    EC_FUNC = 3,
    EC_USER = 10
};

enum MessageFlags
{
    NEW = 1,
    PROCESSING = 2,
    PROCESSED = 3,
    DEBUG = 4,
    RECEIPT = 5,
    STATE = 6
};
} // namespace Bridge

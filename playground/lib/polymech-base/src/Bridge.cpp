#include "macros.h"
#include <ArduinoLog.h>
#include "Bridge.h"
#include <Vector.h>
#include <Streaming.h>
#include "constants.h"
#include <StringUtils.h>
#include "./enums.h"
#include "config.h"

#define BRIDGE_DEBUG_REGISTER
#define BRIDGE_DEBUG_CALL_METHOD
#define NB_PAYLOAD_ELEMENTS 3

class SComponentInfo;
SComponentInfo *componentsArray[MAX_COMPONENTS];
Vector<SComponentInfo *> componentList;

Bridge::Bridge(Component *_owner) : Component("Bridge", COMPONENT_KEY_MB_BRIDGE, Component::COMPONENT_DEFAULT, _owner)
{
    componentList.setStorage(componentsArray);
}
short Bridge::setup()
{
    return E_OK;
}

SComponentInfo *Bridge::hasMethod(ushort id, String methodName)
{
    uchar s = componentList.size();
    for (uchar i = 0; i < s; i++)
    {
        SComponentInfo *val = componentList.at(i);
        if (val->key == id && val->methodName.equals(methodName))
        {
            return val;
        }
    }
    return NULL;
}
short Bridge::debug()
{
    return E_OK;
}
short Bridge::list()
{
    uchar s = componentList.size();
    Log.verboseln("Bridge::list - Registered methods: %d", s);
    for (uchar i = 0; i < s; i++)
    {
        SComponentInfo *val = componentList.at(i);
        Log.verboseln("\tRegistered Method:  %d::%s::%s", val->key, ((Component *)val->instance)->name.c_str(), val->methodName.c_str());
    }
    return E_OK;
}

SComponentInfo *Bridge::registerMemberFunction(ushort id, Component *clazz, char *method, ComponentFnPtr ptr)
{
#ifdef DISABLE_BRIDGE
    // #pragma message("Bridge::registerMemberFunction : Bridge is disabled!")
    return NULL;
#endif
#ifndef DISABLE_BRIDGE
    if (componentList.size() > MAX_COMPONENTS)
    {
        Log.errorln("Bridge::registerMemberFunction : Max components reached : %d", MAX_COMPONENTS);
        return NULL;
    }
    SComponentInfo *meth = hasMethod(id, method);
    if (meth)
    {
#ifdef BRIDGE_DEBUG_REGISTER
        Log.verboseln("Register class member: %s::%s already registered!", clazz, method);
#endif
    }
    else
    {
        meth = new SComponentInfo(id, clazz, method, ptr);
        componentList.push_back(meth);
    }
    return meth;
#endif
    return NULL;
}

const Vector<SComponentInfo *> &Bridge::getComponentList() const
{
    return componentList;
}

short Bridge::onMessage(int id, E_CALLS verb, E_MessageFlags flags, String user, Component *src)
{

#ifdef BRIDGE_DEBUG_CALL_METHOD
    Log.verboseln("Bridge::onMessage: %d::%d::%d::%s - User:  %s", id, verb, flags, src->name.c_str(), user.c_str());
#endif

    if (strlen(user.c_str()) == 0)
    {
        Log.verboseln("Bridge::onMessage: %d : invalid payload", id);
        return E_NOT_FOUND;
    }

    switch (verb)
    {
    case E_CALLS::EC_USER:
    case E_CALLS::EC_NONE:
    case E_CALLS::EC_COMMAND:
    case E_CALLS::EC_FUNC:
    {
        return E_OK;
    }
    case E_CALLS::EC_METHOD:
    {
        char *strings[NB_PAYLOAD_ELEMENTS];
        char *ptr = NULL;
        byte index = 0;
        ptr = strtok(C_STR(user.c_str()), CC_STR(Bridge::METHOD_DELIMITER));
        while (ptr != NULL && index <= NB_PAYLOAD_ELEMENTS)
        {
            strings[index] = ptr;
            index++;
            ptr = strtok(NULL, CC_STR(Bridge::METHOD_DELIMITER));
        }
        char *_method = strings[0];
        if (strlen(_method) == 0)
        {
            Log.errorln("Bridge::onMessage: %d : invalid method name", id);
            return E_NOT_FOUND;
        }

        SComponentInfo *method = hasMethod(id, _method);
        if (method)
        {
            short arg0 = convertTo<short>(CC_STR(strings[1]));
            short arg1 = convertTo<short>(CC_STR(strings[2]));
            Component *component = (Component *)method->instance;
            ComponentFnPtr ptr = method->mPtr;
            short ret = (component->*ptr)(arg0, arg1);
#ifdef BRIDGE_DEBUG_CALL_METHOD
            Log.verboseln("Called method: %s(%d)::%s with : %d | %d = %d", component->name.c_str(), id, _method, arg0, arg1, ret);
#endif
            return ret;
        }
        else
        {
#ifdef BRIDGE_DEBUG_CALL_METHOD
            Log.errorln("Method not found: %d::%s - register size %d", id, _method, componentList.size());
            list();
#endif
        }
        return E_OK;
    }
    }
    return E_NOT_FOUND;
}

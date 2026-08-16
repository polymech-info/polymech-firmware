#ifndef BRIDGE_H
#define BRIDGE_H

#include <vector>

#include <Component.h>
#include <WString.h>
#include <xtypes.h>
#include <enums.h>
#include <macros.h>

#include <enums.h>

#include <Streaming.h>

class SComponentInfo;

class SComponentInfo
{
public:
    short key;
    void *instance;
    String methodName;
    ComponentFnPtr mPtr;
    SComponentInfo() {}
    SComponentInfo(ushort _key, void *_instance, String _methodName, ComponentFnPtr _mPtr) : key(_key),
                                                                                      instance(_instance),
                                                                                      methodName(_methodName),
                                                                                      mPtr(_mPtr) {}
};

class Bridge : public Component
{
public:
    Bridge(Component *_owner);
    
    SComponentInfo *registerMemberFunction(
        ushort id,
        Component *clazz,
        char *method,
        ComponentFnPtr ptr);

    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, String user, Component *src);
    const std::vector<SComponentInfo *> &getComponentList() const;

    // Component implementation
    short debug();
    short setup();    

    // --- Methods for ModbusManager ---

    /**
     * @brief Retrieves a list of all registered component instances.
     * NOTE: This requires careful memory management. Consider returning pointers or references.
     * This current implementation returns pointers stored in the internal vector.
     * @return A vector of Component pointers.
     */
    std::vector<Component*> getAllComponents();

    static constexpr char *METHOD_DELIMITER = C_STR(":");

private:
    SComponentInfo *hasMethod(ushort id, String methodName);
    short list();    
};
#endif


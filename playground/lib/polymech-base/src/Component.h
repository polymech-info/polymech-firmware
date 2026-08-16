#ifndef COMPONENT_H
#define COMPONENT_H

#include <WString.h>
#include <ArduinoLog.h>
#include <Vector.h>
#include "./enums.h"
#include "constants.h"
#include "error_codes.h"
#include "macros.h"
#include "xtypes.h"

class Bridge;
class ModbusTCP;
class ModbusBlockView;
class MB_Registers;
class RS485;
/**
 * @brief The Component class represents a generic component.
 */
class Component
{
public:
    /**
     * @brief The default run flags for a component.
     */
    static const int COMPONENT_DEFAULT = 1 << OBJECT_RUN_FLAGS::E_OF_LOOP | 1 << OBJECT_RUN_FLAGS::E_OF_SETUP;

    /**
     * @brief The default ID for a component.
     */
    static const ushort COMPONENT_NO_ID = 0;

    /**
     * @brief The type of the component.
     */
    ushort type = COMPONENT_TYPE::COMPONENT_TYPE_UNKOWN;

    /**
     * @brief Default constructor for the Component class.
     */
    Component() : name("NO_NAME"), id(0),
                  flags(OBJECT_RUN_FLAGS::E_OF_NONE),
                  nFlags(OBJECT_NET_CAPS::E_NCAPS_NONE),
                  owner(nullptr),
                  slaveId(0) {}

    /**
     * @brief Constructor for the Component class with a specified name.
     * @param _name The name of the component.
     */
    Component(String _name) : name(_name), id(COMPONENT_NO_ID),
                              flags(OBJECT_RUN_FLAGS::E_OF_NONE),
                              nFlags(OBJECT_NET_CAPS::E_NCAPS_NONE),
                              owner(nullptr),
                              slaveId(0) {}

    /**
     * @brief Constructor for the Component class with a specified name and ID.
     * @param _name The name of the component.
     * @param _id The ID of the component.
     */
    Component(String _name, ushort _id) : name(_name),
                                          id(_id),
                                          flags(OBJECT_RUN_FLAGS::E_OF_NONE),
                                          nFlags(OBJECT_NET_CAPS::E_NCAPS_NONE),
                                          owner(nullptr),
                                          slaveId(0) {}

    /**
     * @brief Constructor for the Component class with a specified name, ID, and flags.
     * @param _name The name of the component.
     * @param _id The ID of the component.
     * @param _flags The run flags for the component.
     */
    Component(String _name, short _id, int _flags) : name(_name),
                                                     id(_id),
                                                     flags(_flags),
                                                     nFlags(OBJECT_NET_CAPS::E_NCAPS_NONE),
                                                     owner(nullptr),
                                                     slaveId(0)
    {
    }

    /**
     * @brief Constructor for the Component class with a specified name, ID, flags, and owner.
     * @param _name The name of the component.
     * @param _id The ID of the component.
     * @param _flags The run flags for the component.
     * @param _owner The owner of the component.
     */
    Component(String _name, ushort _id, uint16_t _flags, Component *_owner, uint16_t featureFlags = E_NetworkValueFeatureFlags::E_NVFF_ALL) : nFlags(OBJECT_NET_CAPS::E_NCAPS_NONE),
                                                                              name(_name),
                                                                              id(_id),
                                                                              flags(_flags),
                                                                              owner(_owner),
                                                                              slaveId(0),
                                                                              fFlags(featureFlags)
    {
    }

    /**
     * @brief Destructor for the Component class.
     */
    virtual ~Component() = default;

    /**
     * @brief Virtual function to destroy the component.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short destroy() { return E_OK; };

    /**
     * @brief Virtual function to debug the component.
     * @param stream The stream to output the debug information to.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short debug() { return E_OK; };

    /**
     * @brief Virtual function to debug the component.
     * @param stream The stream to output the debug information to.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short debug(short val0, short val1) { return E_OK; };

    /**
     * @brief Virtual function to display information about the component.
     * @param stream The stream to output the information to.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short info() { return E_OK; };

    /**
     * @brief Virtual function to display information about the component.
     * @param stream The stream to output the information to.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short info(short val0, short val1) { return E_OK; };

    /**
     * @brief Virtual function to set up the component.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short setup() { return E_OK; };

    /**
     * @brief Virtual function being called after all components have been setup.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short onRun() { return E_OK; };

    /**
     * @brief Virtual function to run the component in a loop.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short loop()
    {
        _loop_start_time_us = micros();
        // Derived classes will call this base implementation or implement their own logic
        // Execution of derived class loop happens here
        // Then, the duration is calculated in the derived class if it overrides this, or after App::loop() in PHApp
        return E_OK;
    };

    /**
     * @brief Checks if the component has a specific flag.
     * @param flag The flag to check.
     * @return True if the component has the flag, false otherwise.
     */
    bool hasFlag(byte flag)
    {
        return TEST(flags, flag);
    }

    /**
     * @brief Sets a specific flag for the component.
     * @param flag The flag to set.
     */
    void setFlag(byte flag)
    {
        SBI(flags, flag);
    }

    /**
     * @brief Sets a specific flag for the component.
     * @param flag The flag to set.
     */
    short toggleFlag(short flag, short value)
    {
        if (value)
        {
            SBI(flags, flag);
        }
        else
        {
            CBI(flags, flag);
        }
        return flags;
    }

    /**
     * @brief Clears a specific flag for the component.
     * @param flag The flag to clear.
     */
    void clearFlag(byte flag)
    {
        CBI(flags, flag);
    }

    /**
     * @brief Enables the component.
     */
    void enable()
    {
        clearFlag(OBJECT_RUN_FLAGS::E_OF_DISABLED);
    }

    /**
     * @brief Disables the component.
     */
    void disable()
    {
        setFlag(OBJECT_RUN_FLAGS::E_OF_DISABLED);
    }

    /**
     * @brief Checks if the component is enabled.
     * @return True if the component is enabled, false otherwise.
     */
    bool enabled()
    {
        return !hasFlag(OBJECT_RUN_FLAGS::E_OF_DISABLED);
    }

    void enableFeature(E_NetworkValueFeatureFlags flag)
    {
        SBI(fFlags, flag);
    }
    void disableFeature(E_NetworkValueFeatureFlags flag)
    {
        CBI(fFlags, flag);
    }
    bool hasFeature(E_NetworkValueFeatureFlags flag) const { return TEST(fFlags, flag); }

    /**
     * @brief The name of the component.
     */
    String name;

    /**
     * @brief The ID of the component.
     */
    const ushort id;

    /**
     * @brief The run flags for the component.
     */
    uint16_t flags;

    /**
     * @brief The network capabilities of the component.
     */
    uint16_t nFlags;

    /**
     * @brief The feature flags for the component.
     */
    uint16_t fFlags;

    /**
     * @brief The owner of the component.
     */
    Component *owner;

    /**
     * @brief The current time in milliseconds.
     */
    millis_t now;

    /**
     * @brief The last tick time in milliseconds.
     */
    millis_t last;

    /**
     * @brief Start time of the last loop execution in microseconds.
     */
    uint64_t _loop_start_time_us;

    /**
     * @brief Duration of the last loop execution in microseconds.
     */
    uint64_t _loop_duration_us;

    //////////////////////////////////////////
    //
    //  Component Hierarchy / Lookup
    //

    /**
     * @brief Virtual method to retrieve a component managed by this component (or its children) by ID.
     * The base implementation returns nullptr.
     * Owners like PHApp should override this to provide actual lookup.
     * @param id The ID of the component to find.
     * @return Pointer to the component if found, nullptr otherwise.
     */
    virtual Component *getComponent(short id) { return nullptr; }

    //////////////////////////////////////////
    //
    //  Messaging
    //  @todo: extract to a separate class

    /**
     * @brief Handles incoming messages.
     *
     * This function is called when a message is received by the component.
     * It processes the message and returns a short value indicating the status of the operation.
     *
     * @param id The ID of the message.
     * @param verb The type of operation to be performed.
     * @param flags The flags associated with the message.
     * @param user A pointer to user-defined data.
     * @param src The source component that sent the message.
     * @return A short value indicating the status of the operation.
     */
    virtual short onMessage(int id, E_CALLS verb, E_MessageFlags flags, String user, Component *src)
    {
        if (this->owner)
        {
            return this->owner->onMessage(id, verb, flags, user, src);
        }
        return E_OK;
    };

    /**
     * @brief Handles incoming messages with a generic void* payload.
     *
     * @param id The ID of the message.
     * @param verb The type of operation to be performed.
     * @param flags The flags associated with the message.
     * @param user A pointer to user-defined data (nullptr if not provided).
     * @param src The source component that sent the message (nullptr if not provided).
     * @return A short value indicating the status of the operation.
     */
    virtual short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user = nullptr, Component *src = nullptr)
    {
        return E_OK;
    };
    /**
     * @brief Handles errors.
     * @param id The ID of the error.
     * @param error The error code.
     * @return The error code indicating the success or failure of the operation.
     */
    virtual short onError(short id, short error) { return E_OK; };

    /**
     * @brief Handles responses.
     * @param id The ID of the response.
     * @param response The response code.
     * @return The response code indicating the success or failure of the operation.
     */
    virtual short onResponse(short id, short response) { return E_OK; };

    //////////////////////////////////////////
    //
    //  Binding

    /**
     * Registers methods for the component with the specified bridge.
     * This method should be overridden by derived classes to provide custom method registration logic.
     *
     * @param bridge The bridge to register methods with.
     * @return The status code indicating the success or failure of the method registration.
     */
    virtual short serial_register(Bridge *bridge) { return E_OK; }

    /**
     * @brief Sets a specific network capability flag for the component.
     * @param flag The network capability flag to set (from OBJECT_NET_CAPS).
     */
    void setNetCapability(OBJECT_NET_CAPS flag)
    {
        SBI(nFlags, flag);
    }

    /**
     * @brief Checks if the component has a specific network capability flag.
     * @param flag The network capability flag to check (from OBJECT_NET_CAPS).
     * @return True if the component has the capability, false otherwise.
     */
    bool hasNetCapability(OBJECT_NET_CAPS flag) const
    {
        return TEST(nFlags, flag);
    }

    /**
     * @brief Clears a specific network capability flag for the component.
     * @param flag The network capability flag to clear (from OBJECT_NET_CAPS).
     */
    void clearNetCapability(OBJECT_NET_CAPS flag)
    {
        CBI(nFlags, flag);
    }

    //////////////////////////////////////////
    //
    //  Network Interface (Modbus, Serial, CAN, etc.)
    //

    /**
     * @brief Called by a network manager (e.g., ModbusTCP) to write a value to this component.
     * Derived classes should implement this to handle incoming network writes specific to their function.
     * @param address The specific Modbus address being written to within the component's range.
     * @param value The value received from the network.
     * @return E_OK on success, or an appropriate error code.
     */
    virtual short mb_tcp_write(short address, short value)
    {
        return 0;
    };

    /**
     * @brief Variant of mb_tcp_write accepting MB_Registers context.
     * @param reg The MB_Registers block associated with this write request.
     * @param value The value received from the network.
     * @return E_OK on success, or an appropriate error code.
     */
    virtual short mb_tcp_write(MB_Registers *reg, short value)
    {
        return 0;
    };

    /**
     * @brief Called by a network manager (e.g., ModbusTCP) to read a value from this component.
     * Derived classes should implement this to provide their current state to the network.
     * @param address The specific Modbus address being read within the component's range.
     * @return The current value for the given address, or potentially an error indicator.
     */
    virtual short mb_tcp_read(short address)
    {
        return 0;
    }

    /**
     * @brief Variant of mb_tcp_read accepting MB_Registers context.
     * @param reg The MB_Registers block associated with this read request.
     * @return The current value for the register block, or potentially an error indicator.
     */
    virtual short mb_tcp_read(MB_Registers *reg)
    {
        return 0;
    }

    /**
     * @brief Get the last error code
     */
    virtual ushort mb_tcp_error(MB_Registers *reg) { return 0; }

    /**
     * @brief Called during setup to allow the component to register its Modbus blocks.
     *
     * Derived classes should override this. It's recommended to call mb_tcp_blocks()
     * inside this function, iterate through the returned view, add the runtime
     * component ID to each MB_Registers struct, and then register it with the manager.
     *
     * @param manager Pointer to the ModbusTCP instance.
     */
    virtual void mb_tcp_register(ModbusTCP *manager)
    {
        // Base implementation does nothing.
    }

    /**
     * @brief Gets a view of the static Modbus block definitions for this component type.
     *
     * @note The componentId field within the returned MB_Registers structs may not be
     *       populated, as the definitions are typically static/constexpr.
     *       Use mb_tcp_register to handle registration with the correct runtime ID.
     *
     * @return A ModbusBlockView describing the blocks handled by this component type.
     *         Default implementation returns an empty view {nullptr, 0}.
     */
    virtual ModbusBlockView *mb_tcp_blocks() const { return nullptr; }

    /**
     * @brief Gets the base Modbus TCP address allocated for this RTU device instance.
     * @return The base TCP address for this device instance.
     */
    virtual uint16_t mb_tcp_base_address() const { return 0; }

    /**
     * @brief The Modbus slave ID for this component (satisfies the Modbus interfaces)
     */
    ushort slaveId;

    /**
     * @brief The RS485 interface for this component.
     * @todo: move to feature
     */
    RS485 *rs485;

protected:
    /**
     * @brief Called by derived classes when their internal state changes in a way that should be reflected on the network.
     * The base class (or a network manager observing this) should handle queuing the update.
     */
    virtual void notifyStateChange()
    {
        // Base implementation could potentially interact with a NetworkManager singleton/instance
        // Log.verboseln("Component::notifyStateChange - ID %d", id);
    }

public:
    //////////////////////////////////////////
    //
    //  Component Hierarchy / Lookup
    virtual Component *byId(ushort id) { return nullptr; }

    /**
     * @brief Gets the duration of the last loop execution in microseconds.
     * @return The loop duration in microseconds.
     */
    uint64_t getLoopDurationUs() const { return _loop_duration_us; }
};

/**
 * @brief Function pointer type for component member functions.
 */
typedef short (Component::*ComponentFnPtr)(short, short);
/**
 * @brief Function pointer type for component member functions with variable arguments.
 */
typedef short (Component::*ComponentVarArgsFnPtr)(...);

typedef short (Component::*ComponentRxFn)(short size, uint8_t rxBuffer[]);

#endif

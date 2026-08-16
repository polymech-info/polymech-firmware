# `ModbusTCP` as a Feature for `NetworkComponent`

This document outlines a design to refactor `NetworkComponent` to make network protocols like Modbus TCP modular, opt-in features. This approach increases flexibility, reduces the memory footprint for components that don't need network capabilities, and provides a clear path for adding other protocols (e.g., Serial, CAN) in the future.

## 1. Motivation

Currently, `NetworkComponent` is tightly coupled with Modbus TCP. Its constructor requires a Modbus `baseAddress`, and it contains member variables and methods specific to Modbus. This design has a few drawbacks:

-   **Lack of Modularity**: Every component inheriting from `NetworkComponent` carries the overhead of Modbus, even if unused.
-   **Limited Extensibility**: Adding new network protocols is difficult without modifying the base `NetworkComponent` and `Component` classes.
-   **Inconsistency**: `NetworkValue` already uses a flexible feature system (for logging, notifications, etc.), which has proven effective. Aligning `NetworkComponent` with a similar pattern would create a more consistent and robust architecture.

The goal is to evolve `NetworkComponent` into a lean, protocol-agnostic base class and provide network capabilities via composable mixins, starting with Modbus TCP.

## 2. Proposed Design

The proposed solution is inspired by the `Persistent` mixin and involves two main steps:
1.  Refactor `NetworkComponent` to remove hard-coded Modbus TCP logic.
2.  Create a `WithModbus` mixin class that encapsulates the extracted Modbus TCP functionality.

This approach ensures that existing component implementations remain source-compatible.

### 2.1. Refactored `NetworkComponent` (Core)

The core `NetworkComponent` will be simplified to a generic manager for `NetworkValue`s, without any knowledge of specific network protocols.

**`lib/polymech-base/src/modbus/NetworkComponent.h` (Proposed Changes)**
```cpp
#ifndef NETWORK_COMPONENT_H
#define NETWORK_COMPONENT_H

#include "Component.h"
#include <vector>
#include "NetworkValue.h"

// The template parameter N (number of NetworkValues) is kept for consistency.
template <size_t N = 20>
class NetworkComponent : public Component {
protected:
    std::vector<NetworkValueBase *> _networkValues;
    NetworkValue<bool> m_enabled;

public:
    // The constructor no longer takes a Modbus-specific 'baseAddress'.
    template <typename... Args>
    NetworkComponent(Args &&... args)
        : Component(std::forward<Args>(args)...),
          m_enabled(this, this->id, "Enabled", /*...initial values...*/),
          // No modbus blocks are initialized here.
    {
        _networkValues.reserve(N);
        // Default capability is removed. It will be added by the feature mixin.
        // setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS); 
        addNetworkValue(&m_enabled);
    }

    virtual ~NetworkComponent() = default; // Destructor simplifies.

    // Generic setup remains, but Modbus-specific calls are gone.
    short setup() override {
        Component::setup();
        // The m_enabled.initModbus() call is removed.
        return E_OK;
    }

    // Generic loop logic.
    short loop() override {
        Component::loop();
        if (!this->enabled()) {
            return E_OK;
        }
        return loopNetwork();
    }

    virtual short loopNetwork() { return E_OK; }

    void addNetworkValue(NetworkValueBase *nv) {
        if (nv) {
            _networkValues.push_back(nv);
        }
    }

    // onMessage can be simplified if it only contained protocol-specific logic.
    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user, Component *src) override {
        // If the Protobuf logic is also moved to a feature, this can be cleaner.
        if (verb == E_CALLS::EC_PROTOBUF_UPDATE && user != nullptr) {
            return this->owner->onMessage(id, verb, flags, user, src);
        }
        return Component::onMessage(id, verb, flags, user, src);
    }
};

// Convenience macros for NetworkValues can be moved to the Modbus mixin
// if they are only used in that context.
// #define INIT_NETWORK_VALUE(...)
// #define SETUP_NETWORK_VALUE(...)

#endif // NETWORK_COMPONENT_H
```
The virtual functions for Modbus (`mb_tcp_write`, `mb_tcp_read`, etc.) remain in the base `Component` class with default implementations. The refactored `NetworkComponent` simply inherits them without providing an implementation.

### 2.2. The `WithModbus` Mixin

A new header will define the `WithModbus` mixin. This class inherits from its template parameter `TBase` (which will be a `NetworkComponent` instantiation) and re-introduces all the Modbus TCP logic.

**`lib/polymech-base/src/mixins/WithModbus.h` (New File)**
```cpp
#ifndef MIXIN_WITH_MODBUS_H
#define MIXIN_WITH_MODBUS_H

#include "modbus/NetworkComponent.h"
#include "modbus/ModbusTCP.h"

template <class TBase, size_t N>
class WithModbus : public TBase {
protected:
    uint16_t _baseAddress;
    MB_Registers* _modbusBlocks;
    mutable ModbusBlockView _modbusBlockView;
    size_t _nextIndex;
    ModbusTCP* modbusTCP;

public:
    template <typename... Args>
    WithModbus(uint16_t baseAddress, Args&&... args)
        : TBase(std::forward<Args>(args)...),
          _baseAddress(baseAddress),
          _modbusBlocks(new MB_Registers[N]()),
          _modbusBlockView{_modbusBlocks, static_cast<int>(N)},
          _nextIndex(0),
          modbusTCP(nullptr)
    {
        // Add the Modbus capability flag.
        this->setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    }

    virtual ~WithModbus() {
        delete[] _modbusBlocks;
    }

    // The Modbus-specific parts of setup() are now here.
    short setup() override {
        TBase::setup();
        this->m_enabled.initModbus(_baseAddress + E_NVC_ENABLED, 1, this->id, this->slaveId, FN_WRITE_COIL, "Enabled", this->name.c_str());
        this->m_enabled.initNotify(true, true, NetworkValue_ThresholdMode::DIFFERENCE);
        registerBlock(this->m_enabled.getRegisterInfo());
        return E_OK;
    }

    // All Modbus-related methods are implemented here.
    MB_Registers* registerBlock(const MB_Registers& reg) {
        if (_nextIndex >= N) { /* ... error handling ... */ return nullptr; }
        _modbusBlocks[_nextIndex] = reg;
        return &_modbusBlocks[_nextIndex++];
    }

    // --- Component virtual overrides ---
    ModbusBlockView* mb_tcp_blocks() const override {
        _modbusBlockView.count = _nextIndex;
        return const_cast<ModbusBlockView*>(&_modbusBlockView);
    }

    void mb_tcp_register(ModbusTCP* manager) override {
        if (!this->hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS)) return;
        this->modbusTCP = manager;
        // ... (rest of the original implementation)
    }

    uint16_t mb_tcp_base_address() const override { return _baseAddress; }

    short mb_tcp_read(MB_Registers *reg) override {
        if (reg->startAddress == (_baseAddress + E_NVC_ENABLED)) {
            return this->enabled() ? 1 : 0;
        }
        // Chain up to allow base class to handle other reads if needed.
        return TBase::mb_tcp_read(reg); 
    }

    short mb_tcp_write(MB_Registers *reg, short value) override {
        if (reg->startAddress == (_baseAddress + E_NVC_ENABLED)) {
            this->enable(value != 0);
            this->m_enabled.update(value != 0);
            return E_OK;
        }
        return TBase::mb_tcp_write(reg, value);
    }
};

// Helper macros can be defined here as they are specific to the Modbus context
#define SETUP_NETWORK_VALUE(nv_member, reg_offset_enum, fn_code, desc, ...) \
    do { \
        (nv_member).initNotify(__VA_ARGS__); \
        (nv_member).initModbus( \
            _baseAddress + static_cast<uint16_t>(reg_offset_enum), \
            1, \
            this->id, \
            this->slaveId, \
            fn_code, \
            desc, \
            this->name.c_str() \
        ); \
        registerBlock((nv_member).getRegisterInfo()); \
    } while(0)

#endif // MIXIN_WITH_MODBUS_H
```

## 3. Usage and Compatibility

To maintain source compatibility, a component that previously inherited from `NetworkComponent` will now inherit from the `WithModbus` mixin, which wraps the core `NetworkComponent`.

### Example: `Relay` Component

**`Relay.h` (Before)**
```cpp
#include "modbus/NetworkComponent.h"
//...
class Relay : public NetworkComponent<RELAY_MB_COUNT> { /*...*/ };
```

**`Relay.h` (After)**
```cpp
#include "modbus/NetworkComponent.h" // The new lean version
#include "mixins/WithModbus.h"       // The new mixin
//...

// Define a type that represents the classic NetworkComponent behavior.
template<size_t N>
using ModbusNetworkComponent = WithModbus<NetworkComponent<N>, N>;

// Relay's inheritance changes, but the rest of its interface and implementation
// remains IDENTICAL. The constructor signature doesn't change.
class Relay : public ModbusNetworkComponent<RELAY_MB_COUNT> {
public:
    // ... same as before
    Relay(
        Component *owner,
        short _pin,
        short _id,
        short _modbusAddress); // Signature is compatible!
    // ...
};
```
The constructor in `Relay.cpp` will call the `WithModbus` constructor, which has the exact same signature as the old `NetworkComponent` constructor, ensuring full compatibility.

## 4. Future Extensibility

This design provides a clear template for adding other network protocols. For instance, to add a "Serial" feature:

1.  Create a `WithSerial` mixin class similar to `WithModbus`.
2.  Implement the serial communication logic and necessary `Component` virtual overrides within it.
3.  A component could then use `WithSerial<NetworkComponent<...>>` or even compose features: `WithModbus<WithSerial<NetworkComponent<...>>>`.

This modular approach ensures that `NetworkComponent` remains a lightweight and stable base, while features can be developed and composed independently. 
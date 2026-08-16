# Component System Documentation

This document describes the component-based architecture used in the firmware.

## Overview

The firmware utilizes a modular design where different functionalities (hardware interfaces, communication managers, etc.) are implemented as separate "Components". This promotes code reusability, organization, and easier maintenance.

## Core Concepts

1.  **`Component` Base Class (`src/Component.h`):**
    *   All functional units inherit from the base `Component` class.
    *   Provides common properties like `name`, `id` (unique identifier from `COMPONENT_KEY` enum in `src/enums.h`), `owner` (pointer to the parent component, usually `PHApp`), `flags` (for controlling behavior like setup/loop execution), and `nFlags` (for network capabilities).
    *   Defines standard virtual methods that can be overridden by derived classes:
        *   `setup()`: Called once during application initialization.
        *   `loop()`: Called repeatedly in the main application loop.
        *   `info()`: Used for printing component status/information (often called via serial command).
        *   `debug()`: Used for printing debugging information.
        *   `onRegisterMethods(Bridge* bridge)`: Used to register methods that can be called via the `Bridge` (e.g., through serial commands).
        *   `readNetworkValue(short address)` / `writeNetworkValue(short address, short value)`: Interface for network managers (like `ModbusManager`) to interact with the component's data.
        *   `notifyStateChange()`: Intended to be called by components when their state changes, although currently not actively used by managers.

2.  **`App` / `PHApp` (`src/App.h`, `src/PHApp.h`):**
    *   `App` is the base application class, inheriting from `Component`.
    *   `PHApp` is the specific application implementation for this project.
    *   `PHApp` acts as the central orchestrator and owner of most components.
    *   It maintains a `Vector<Component*> components` list to hold pointers to all active components.
    *   **Initialization (`PHApp::setup()`):**
        *   Instantiates necessary components (like `Bridge`, `SerialMessage`, `ModbusManager`, `Relay`, etc.).
        *   Adds component pointers to the `components` vector using `components.push_back()`. **Crucially, `components.setStorage(componentsArray);` must be called in the `App` constructor (`App.cpp`) for the vector to function correctly.**
        *   Calls `App::setup()`, which iterates through the `components` vector and calls the `setup()` method of each component (if the `E_OF_SETUP` flag is set).
        *   Registers components with managers (e.g., `modbusManager->registerComponentAddress(...)`).
        *   Registers component methods with the `Bridge` (`registerComponents(bridge)`).
    *   **Main Loop (`PHApp::loop()`):**
        *   Calls `App::loop()`, which iterates through the `components` vector and calls the `loop()` method of each component (if the `E_OF_LOOP` flag is set).

3.  **Configuration (`src/config.h`, `src/features.h`, `src/config_secrets.h`, `src/config-modbus.h`):**
    *   Hardware pin assignments (e.g., `MB_RELAY_0`, `STATUS_WARNING_PIN`) are typically defined in `config.h`.
    *   Feature flags (e.g., `ENABLE_MODBUS_TCP`, `HAS_STATUS`) used for conditional compilation are often in `features.h` or `config_adv.h`.
    *   Sensitive information like WiFi credentials should be in `config_secrets.h` (which should not be committed to version control).
    *   Modbus addresses are centralized in `config-modbus.h`.
    *   Component IDs are defined in `src/enums.h`.

## Adding a New Component

Let's illustrate with a hypothetical example: adding a simple Temperature Sensor component.

1.  **Define Configuration:**
    *   Add pin definition to `config.h`: `#define TEMP_SENSOR_PIN 34`
    *   Add component ID to `enums.h` (`COMPONENT_KEY` enum): `COMPONENT_KEY_TEMP_SENSOR_0 = 800,`
    *   (Optional) Add Modbus address to `config-modbus.h`: `#define MB_IREG_TEMP_SENSOR_0 800`
    *   (Optional) Add feature flag to `features.h`: `#define HAS_TEMP_SENSOR_0`

2.  **Create Component Class (`src/TempSensor.h`):**
    ```cpp
    #ifndef TEMP_SENSOR_H
    #define TEMP_SENSOR_H

    #include "Component.h"
    #include "enums.h"
    #include <ArduinoLog.h>
    // Include any specific sensor libraries if needed

    class Bridge;

    class TempSensor : public Component {
    private:
        const short pin;
        float currentValue;
        short modbusAddress;

    public:
        TempSensor(Component* owner, short _pin, short _id, short _modbusAddr)
            : Component("TempSensor", _id, COMPONENT_DEFAULT, owner),
              pin(_pin),
              currentValue(0.0),
              modbusAddress(_modbusAddr)
        {
            // Enable Modbus capability if needed
            setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
            // Ensure loop() is called
            // setFlag(OBJECT_RUN_FLAGS::E_OF_LOOP); // Already in COMPONENT_DEFAULT
        }

        short setup() override {
            Component::setup();
            // Initialize sensor library, set pin mode, etc.
            // pinMode(pin, INPUT); // Example
            Log.verboseln("TempSensor::setup - ID: %d, Pin: %d, Modbus Addr: %d", id, pin, modbusAddress);
            return E_OK;
        }

        short loop() override {
            Component::loop();
            // Read sensor value periodically
            // currentValue = analogRead(pin); // Simplified example
            // Add proper timing logic (e.g., read every second)
            return E_OK;
        }

        short info(short arg1 = 0, short arg2 = 0) override {
             Log.verboseln("TempSensor::info - ID: %d, Pin: %d, Value: %F, Modbus Addr: %d", id, pin, currentValue, modbusAddress);
             return E_OK;
        }

        // --- Network Interface ---
        short readNetworkValue(short address) override {
            if (address == modbusAddress) {
                // Convert float to Modbus register format (e.g., integer degrees * 10)
                return (short)(currentValue * 10.0);
            }
            return 0; // Not handled
        }

        // Optional: Implement writeNetworkValue if temperature could be set (e.g., for simulation)
        // virtual short writeNetworkValue(short address, short value) override { ... }

        // Register methods for serial commands
        short onRegisterMethods(Bridge* bridge) override {
             Component::onRegisterMethods(bridge);
             bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&TempSensor::info);
             // Add other methods if needed
             return E_OK;
        }
    };

    #endif // TEMP_SENSOR_H
    ```

3.  **Instantiate and Register in `PHApp`:**
    *   **`PHApp.h`:**
        *   Include `TempSensor.h` (likely within `#ifdef HAS_TEMP_SENSOR_0`).
        *   Declare a pointer: `TempSensor* tempSensor_0;`.
    *   **`PHApp.cpp` (`PHApp::setup()`):**
        ```cpp
        // ... other includes ...
        #ifdef HAS_TEMP_SENSOR_0
        #include "TempSensor.h"
        #endif

        // ... inside PHApp::setup() ...

        #ifdef HAS_TEMP_SENSOR_0
        tempSensor_0 = new TempSensor(this,
                                      TEMP_SENSOR_PIN,
                                      COMPONENT_KEY_TEMP_SENSOR_0,
                                      MB_IREG_TEMP_SENSOR_0);
        components.push_back(tempSensor_0);
        #else
        tempSensor_0 = nullptr;
        #endif

        // ... later, after App::setup() ...

        // --- Register Components with Modbus Manager ---
        #if defined(ENABLE_MODBUS_TCP)
        if (modbusManager) {
            // ... other registrations ...
            #ifdef HAS_TEMP_SENSOR_0
            if (tempSensor_0) modbusManager->registerComponentAddress(tempSensor_0, MB_IREG_TEMP_SENSOR_0, 1);
            #endif
        }
        #endif

        // ... registerComponents(bridge) will handle calling TempSensor::onRegisterMethods ...
        ```

4.  **Build and Test:** Recompile (`npm run build`), upload (`npm run upload`), and test using serial commands (`npm run send -- "<<800;2;64;info:0:0>>"`) and Modbus tools (`python scripts/modbus_read_registers.py --address 800 --ip-address <IP>`). 
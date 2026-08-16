# NetworkComponent Architecture

## 1. Overview

The `NetworkComponent` class serves as a generic, network-aware base class for components that need to expose values over Modbus and other network protocols. It is designed to reduce boilerplate by providing default implementations for common network-related tasks, including Modbus registration, message handling, and a standardized enable/disable mechanism.

## 2. Core Features

### a. Default "Enabled" Coil

By default, every `NetworkComponent` automatically registers a `NetworkValue<bool>` named `m_enabled`.

-   **Modbus Address**: This coil is always located at `mb_tcp_base_address() + 0`.
-   **Functionality**: Writing to this coil via Modbus will enable or disable the component's main loop logic by setting the component's internal `E_OF_DISABLED` flag. This provides a standardized way to remotely control component activity.

### b. `loopNetwork()` Method

Derived components should place their primary operational logic inside the `loopNetwork()` method, not the standard `loop()` method.

The base `NetworkComponent::loop()` method will first check the state of the `m_enabled` coil. If the component is disabled, the `loop()` method will exit early, preventing `loopNetwork()` from being called. This ensures that the enabled state is always respected.

### c. Centralized Network Methods

`NetworkComponent` provides default implementations for several key virtual methods from the `Component` base class:

-   `mb_tcp_register()`: Automatically iterates through the component's registered Modbus blocks and registers them with the `ModbusTCP` manager.
-   `mb_tcp_blocks()`: Returns the view of the component's Modbus blocks.
-   `mb_tcp_read()` / `mb_tcp_write()`: Automatically handle read/write requests for the default `m_enabled` coil.
-   `onMessage()`: Provides a default handler that forwards Protobuf messages (`EC_PROTOBUF_UPDATE`) to the component's owner, which is typically the `RestServer` for broadcasting.

Derived classes should not need to override these methods unless they need to add new behavior.

## 3. Required Implementation by Subclasses

To use `NetworkComponent`, a derived class **must** implement the following:

-   A constructor that calls the `NetworkComponent` base constructor.
-   `uint16_t mb_tcp_base_address() const override`: Must return the unique starting Modbus address for this component instance.
-   `setup()`: Must call `NetworkComponent::setup()` and then register its own `NetworkValue` instances and Modbus blocks.
-   `loopNetwork()`: Contains the component's primary operational logic.
-   `mb_tcp_read(MB_Registers* reg) override`: Must first call `on_mb_tcp_read(reg, &value)`. If this returns `true`, the value has been handled by the base class. Otherwise, the method should handle its own registers.
-   `mb_tcp_write(MB_Registers* reg, short value) override`: Must first call `on_mb_tcp_write(reg, value)`. If this returns `true`, the request has been handled. Otherwise, the method should handle its own registers.

This architecture ensures that components are lean and focused on their specific logic, while the `NetworkComponent` base class handles the common, error-prone networking boilerplate. 
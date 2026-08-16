# NetworkValue and NetworkComponent Enhancements

## 1. Overview

This document outlines a plan to enhance the `NetworkComponent` and `NetworkValue` classes to provide more granular, runtime control over individual `NetworkValue` instances and their specific features (Modbus, Protobuf, Logging).

## 2. Motivation

Currently, the `NetworkComponent` provides a single `m_enabled` coil that toggles the entire component's `loopNetwork()` method on or off. While useful, this is an all-or-nothing approach.

There is a need for more fine-grained control, allowing a user to selectively enable or disable not just the `NetworkValue` itself, but also its specific features. This would be useful for:

-   **Debugging**: Selectively enabling logging for a single problematic `NetworkValue`.
-   **Performance**: Disabling high-frequency Protobuf or Modbus updates for values that are not currently being monitored by any client, without disabling the value's core logic.
-   **Bandwidth Management**: Remotely toggling which values are broadcast over the network to reduce traffic.

## 3. Proposed Feature: Feature Control Masks

I propose adding a series of new, default Modbus registers to the `NetworkComponent`.

-   **Location**: These registers will start at `mb_tcp_base_address() + 1`.
-   **Type**: They will be `uint16_t` holding registers.
-   **Functionality**: Each register will act as a bitmask for a specific feature. Each bit in a mask will correspond to a `NetworkValue` instance within the `_networkValues` vector.

### a. New Registers

| Offset | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| +0 | `ENABLED` | (Coil) Master switch for the component's `loopNetwork()`. | `1` (On) |
| +1 | `ENABLED_MASK` | (Holding) Bitmask to enable/disable entire `NetworkValue` instances. | `0xFFFF` |
| +2 | `MODBUS_MASK` | (Holding) Bitmask to enable/disable the Modbus feature for each `NetworkValue`. | `0xFFFF` |
| +3 | `PROTOBUF_MASK`| (Holding) Bitmask to enable/disable the Protobuf feature for each `NetworkValue`. | `0xFFFF` |
| +4 | `LOGGING_MASK` | (Holding) Bitmask to enable/disable the Logging feature for each `NetworkValue`. | `0xFFFF` |

### b. `NetworkComponent` Implementation

1.  **New `NetworkValue` Members**: New `NetworkValue<uint16_t>` members will be added for `m_enabled_mask`, `m_modbus_mask`, `m_protobuf_mask`, and `m_logging_mask`.
2.  **Initialization**: In `setup()`, these new `NetworkValue`s will be initialized and registered as individual holding registers at their respective offsets. Their default value will be `0xFFFF`.
3.  **Update Logic**: An `onUpdate` callback will be implemented for each mask. When a mask is written to, its callback will iterate through the `_networkValues` vector and apply the corresponding feature toggle.

### c. `NetworkValue` Implementation

The `NetworkValue::update()` method will be modified to respect the component's top-level enabled flag before proceeding with any change detection or notifications.

## 4. Implementation Steps

1.  **Create Documentation**: Write this plan. (Done)
2.  **Modify `NetworkComponent.h`**:
    -   Add the new `NetworkValue<uint16_t>` members for each mask.
    -   Update `setup()` to register the new Modbus holding registers individually.
    -   Implement the `onUpdate` callbacks for each mask.
3.  **Modify `NetworkValue.h`**:
    -   Add the `if (!this->enabled()) return false;` guard at the beginning of the `update()` method.
4.  **Verify & Test**: Run the build and update the `NetworkValueTestPB` component to verify the new functionality.

## 5. Edge Cases and Limitations

-   **Maximum NetworkValues**: The use of a `uint16_t` for the bitmasks limits a single `NetworkComponent` to a maximum of 16 `NetworkValue` instances.
-   **Vector Order**: The masks rely on the fixed order of `NetworkValue` instances in the `_networkValues` vector. This order must not change after initialization.
-   **Component vs. NetworkValue State**: Bit 0 of the `ENABLED_MASK` controls the entire component's `loopNetwork()` method. Other bits in `ENABLED_MASK` control individual `NetworkValue` instances. The feature-specific masks only control their respective features and do not stop the `NetworkValue` from updating its internal state.
## 6. Performance Considerations (ESP32)

Based on the provided device metrics (Free Heap: ~46 KB, Max Free Block: ~34 KB), memory is a significant constraint. Let's analyze the impact of this new feature.

### a. Memory (RAM) Impact

-   **Per `NetworkComponent`**:
    -   Each new feature mask adds one `NetworkValue<uint16_t>` instance. A `NetworkValue` object has a baseline size of around 40-50 bytes.
    -   Adding 4 new masks per `NetworkComponent` will consume approximately **160-200 bytes** of RAM.
-   **System-Wide**:
    -   With **30 components**, the total RAM increase would be `30 * 200 bytes = 6000 bytes`, or approximately **5.9 KB**.
    -   This represents a **~13%** increase relative to the 46 KB of free heap.

#### Detailed Memory Breakdown

The total memory footprint is a sum of the `NetworkValue` objects themselves and the storage for their `MB_Registers` structs within the parent `NetworkComponent`.

| Item | Size per Item (Bytes) | Notes |
| :--- | :--- | :--- |
| `MB_Registers` struct | ~32 | Includes `ushort` fields, pointers, and compiler padding. |
| `NetworkValue` overhead | ~24 | Includes v-table pointer, owner, flags, callbacks, etc. |
| **Total per `NetworkValue`** | **~56** | |

With this baseline, we can calculate a more precise per-component and system-wide impact. This example assumes a component with 10 user-defined `NetworkValue`s, in addition to the new feature masks.

| Item | Count (per Component) | Total Size (Bytes) |
| :--- | :--- | :--- |
| **`NetworkValue` Objects** | | |
| `m_enabled_mask` | 1 | 56 |
| `m_modbus_mask` | 1 | 56 |
| `m_protobuf_mask` | 1 | 56 |
| `m_logging_mask` | 1 | 56 |
| **`_modbusBlocks` Array** | | |
| For user `NetworkValue`s | 10 | 320 |
| For mask `NetworkValue`s | 4 | 128 |
| **Total Per Component** | | **~672** |
| | | |
| **Total System-Wide** | **30 Components** | **~20,160 (~19.7 KB)** |

-   A **~20 KB** increase represents a **~43%** reduction in the available 46 KB of free heap. This is a very significant impact and must be carefully considered.

### b. CPU Impact

-   **Idle State**: When no masks are being changed, the CPU impact is negligible. It amounts to a few extra `if` checks in the `mb_tcp_write` handler per incoming write request.
-   **On Mask Update**: When a mask register is written to, the `onUpdate` callback will execute. This involves a loop of up to 16 iterations (the number of `NetworkValue` instances). Inside the loop, it performs a bitwise check and calls `enable()` or `enableFeature()`. This is a very fast operation, likely taking only a few microseconds to complete.

### c. Conclusion

The primary impact of this feature is on RAM. A ~16 KB increase is a major architectural cost. While the feature offers significant flexibility, the memory overhead may be too high for the target device. The "Consolidated Mask Register" optimization should be considered the primary implementation path to mitigate this.

## 7. Optimizations and Future Work

-   **Consolidated Mask Register**: **(HIGHLY RECOMMENDED)** To significantly reduce the memory footprint, the four separate `uint16_t` masks should be consolidated into a single `std::array<uint16_t, 4>`. This would require only one `NetworkValue` instance and one Modbus block (`FN_WRITE_MULT_REGISTERS`) to update all masks at once, reducing the per-component RAM overhead from over 500 bytes to around 120 bytes.
-   **Dynamic Instantiation**: For very large or dynamic systems, the static `NetworkComponent<N>` template could be replaced with a version that uses a dynamically allocated `Vector` for `_modbusBlocks`. This would increase memory management complexity but allow for a variable number of `NetworkValue` instances.
-   **Read-Only Masks**: To provide a read-only view of the current feature states, additional read-only registers could be exposed. These would be updated internally whenever a mask is changed, providing a clear "live" view of the configuration for clients without allowing them to be written to directly.
### Radical Optimizations (Advanced)

For systems requiring extreme memory optimization, the `NetworkComponent` and `NetworkValue` architecture could be fundamentally redesigned.

-   **Centralized `NetworkValueRegistry`**: Instead of each `NetworkComponent` managing its own list of `NetworkValue` instances and Modbus blocks, a single, static `NetworkValueRegistry` could be implemented.
    -   **Memory Savings**: This would eliminate the `std::vector _networkValues` and `MB_Registers* _modbusBlocks` from every `NetworkComponent` instance, saving hundreds of bytes per component.
    -   **Implementation**: Components would register their `NetworkValue`s with this central registry during their `setup()` phase. The `ModbusTCP` manager would then interact directly with the registry instead of individual components.

-   **Flyweight Pattern for Modbus Data**: The `MB_Registers` struct could be replaced with a more memory-efficient flyweight object.
    -   **How it Works**: The central registry would store a single, canonical copy of the static Modbus metadata (name, group, function code, etc.) for each registered address. The `NetworkValue` would only need to store a pointer or an ID to this shared data, rather than a full copy.
    -   **Memory Savings**: This would further reduce the per-`NetworkValue` overhead from ~56 bytes to closer to ~24-32 bytes.

-   **Callback-Based Read/Write**: The virtual `mb_tcp_read` and `mb_tcp_write` methods could be replaced with a callback system.
    -   **How it Works**: When registering a `NetworkValue`, the component would provide lambdas or function pointers for getting and setting its value. The central registry would store these pointers. When a Modbus request arrives, the registry would directly invoke the appropriate callback.
    -   **Performance**: This would eliminate the overhead and indirection of virtual function calls through the component hierarchy, resulting in faster request handling.

-   **Combined Approach**: A combination of all three radical optimizations would yield the most significant memory and performance gains, but would require a substantial refactoring of the current component architecture. 
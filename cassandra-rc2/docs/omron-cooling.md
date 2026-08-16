# Task List: Omron E5 Cooling Integration

This document outlines the steps required to add cooling monitoring and control capabilities to the `OmronE5` C++ class (`src/components/OmronE5.h` and `src/components/OmronE5.cpp`), referencing registers defined in `docs/omron/omron.h`.

## 1. Verify/Implement Cooling Status Check (`isCooling`)

*   **Goal:** Provide a boolean method to check if the cooling output is currently active.
*   **Changes:**
    *   **`OmronE5.h`:** Ensure the `bool isCooling() const;` method declaration exists (it likely does).
    *   **`OmronE5.cpp`:**
        *   Verify the implementation of `isCooling()`.
        *   It should check the appropriate bit within the status registers (`_currentStatusLow`, `_currentStatusHigh`) that indicates cooling output status.
        *   **Reference Register:** The specific bit needs confirmation from the **H175 Manual Section 5-2 (Status Monitor Table)**. The placeholder `OR_E5_S1_Control_OutputCloseOutput` is used in the current code; verify its correctness for indicating cooling output state. The status data comes from reading `E_STATUS_1_REGISTER` (0x2001) and potentially higher words like `E_STATUS_3_REGISTER` (0x2407), which are read as part of the base status block.
*   **Registers Involved:** Primarily Status Registers (e.g., `0x2001`, `0x2406`, `0x2407`, `0x2408`, `0x2409`) read during the main data poll.

## 2. Add Cooling Manipulated Value (MV) Monitoring (Optional)

*   **Goal:** Allow the application to read the current cooling output level (percentage).
*   **Considerations:** This requires reading a register likely *not* in the default read block (0x0000-0x0005). Decide if raw data or scaled float is needed. Reading extra registers increases bus traffic.
*   **Changes (If implemented):**
    *   **`OmronE5.h`:**
        *   Add a getter method declaration, e.g.:
            ```c++
            // Option A: Scaled Float
            // bool getCoolingMV(float& value) const;

            // Option B: Raw 32-bit Value (reading two 16-bit registers)
            bool getCoolingMVRaw(uint32_t& value) const;
            ```
        *   If using Option B, add private members:
            ```c++
            // uint16_t _currentCoolingMVLow = 0;
            // uint16_t _currentCoolingMVHigh = 0;
            // bool _coolingMvValid = false;
            ```
    *   **`OmronE5.cpp`:**
        *   **`setup()`:** Modify `addMandatoryReadBlock` or add a new one to include the Cooling MV registers. **Note:** `E_MV_MONITOR_COOL_REGISTER` (0x2005) is a 4-byte value, requiring reading two consecutive 16-bit addresses (0x2005 and 0x2006). Adjust the read block accordingly.
        *   **`onRegisterUpdate()`:** Add `else if` conditions to update the internal state (`_currentCoolingMVLow`, `_currentCoolingMVHigh`, `_coolingMvValid`) when data for the new registers (e.g., 0x2005, 0x2006) arrives.
        *   **Implement Getter:** Write the logic for `getCoolingMV` or `getCoolingMVRaw`, combining the low/high words if necessary and potentially scaling the value.
*   **Registers Involved:**
    *   `E_MV_MONITOR_COOL_REGISTER` (0x2005) - Requires reading **0x2005** (Low Word) and **0x2006** (High Word).
    *   Alternatively: `E_OPERATION_MV_MONITOR_COOL_REGISTER` (0x2606) - Requires reading **0x2606** (Low Word) and **0x2607** (High Word).

## 3. Add Cooling Configuration Setters (Optional)

*   **Goal:** Allow runtime configuration of cooling parameters (if not pre-configured on the device).
*   **Considerations:** Increases complexity. Requires adding Modbus write capabilities for these specific registers.
*   **Changes (If implemented):**
    *   **`OmronE5.h`:** Add public method declarations for desired setters (see examples below).
    *   **`OmronE5.cpp`:**
        *   **`setup()`:** For each setter added, call `addOutputRegister()` targeting the specific register address and function code (likely `FN_WRITE_HOLD_REGISTER`). Note that 4-byte registers often require writing to the low-word address.
        *   **Implement Setter Methods:** Implement the logic for each setter, likely calling `this->setOutputRegisterValue()` with the correct address and value (potentially requiring scaling).
*   **Example Setters & Registers Involved:**
    *   `setControlType(E_CONTROL_HEAT_AND_COOL)` -> Writes to `E_CONTROL_TYPE_REGISTER` (**0x2D11**)
    *   `setOperationMode(E_OPERATION_DIRECT)` -> Writes to `E_DIRECT_REVERSE_OP_REGISTER` (**0x2D12**)
    *   `assignOutputToCooling(outputNumber)` -> Writes `E_OUTPUT_ASSIGN_CONTROL_COOL` (2) to `E_CONTROL_OUTPUT_1_ASSIGN_REGISTER` (**0x2E06**) or `E_CONTROL_OUTPUT_2_ASSIGN_REGISTER` (**0x2E07**)
    *   `setCoolingPID(p, i, d)` -> Writes scaled values to `E_PROP_BAND_COOL_REGISTER` (**0x2701**), `E_INTEGRAL_TIME_COOL_REGISTER` (**0x2702**), `E_DERIVATIVE_TIME_COOL_REGISTER` (**0x2703**)
    *   `setDeadBand(value)` -> Writes scaled value to `E_DEAD_BAND_REGISTER` (**0x2704**)

## 4. Modbus TCP Exposure (Optional)

*   **Goal:** Expose new cooling status/values/settings via the Modbus TCP interface.
*   **Changes (If implemented):**
    *   **`OmronE5.cpp`:**
        *   Add new enum members to `E_OmronTcpOffset` for each value to be exposed (e.g., `COOLING_MV`, `COOLING_P_PARAM`).
        *   Increment `OMRON_TCP_BLOCK_COUNT` accordingly (in `.h` and `.cpp` constructor).
        *   Add `INIT_MODBUS_BLOCK` calls in the constructor for the new offsets.
        *   Add `case` statements in `mb_tcp_read()` to handle reads for the new cooling values/parameters (retrieving data from internal state or getters).
        *   Add `case` statements in `mb_tcp_write()` to handle writes for the new cooling settings (calling the corresponding setter methods).

## Review and Testing

*   Thoroughly test all added functionality, paying close attention to:
    *   Correct register addresses and function codes.
    *   Correct handling of 4-byte values (reading/writing low/high words).
    *   Data scaling and unit conversions.
    *   Impact on Modbus bus timing and traffic. 
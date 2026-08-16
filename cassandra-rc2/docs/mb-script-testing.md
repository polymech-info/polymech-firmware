# Modbus Logic Engine (MB_SCRIPT) Functional Testing Strategy

## 1. Objective

To verify the functional correctness of the `ModbusLogicEngine` component (enabled by the `ENABLE_MB_SCRIPT` define). This includes:
*   Correct configuration of logic rules via Modbus.
*   Accurate evaluation of rule conditions based on Modbus register/coil values.
*   Proper execution of defined actions (writing registers/coils, calling component methods).
*   Correct reporting of rule status, trigger timestamps, and trigger counts via Modbus.
*   Handling of various error conditions.

## 2. Scope

This strategy focuses on black-box functional testing from the perspective of a Modbus client interacting with the device. It does not cover unit testing of individual `ModbusLogicEngine` methods or performance/stress testing (which may have separate test plans).

## 3. Approach

Testing will primarily utilize the existing `npm` scripts which wrap Python helper scripts to interact with the device over Modbus TCP.

*   **Configuration:** Rules will be configured by writing to the specific Modbus holding registers allocated to the Logic Engine (`MODBUS_LOGIC_RULES_START` and subsequent offsets defined in `src/ModbusLogicEngine.h`).
*   **Triggering:** Rule conditions will be triggered by writing appropriate values to the source Modbus holding registers or coils specified in the rule's condition.
*   **Verification:**
    *   Action outcomes will be verified by reading the target Modbus holding registers or coils.
    *   Rule status, timestamps, and trigger counts will be verified by reading the corresponding status registers for the rule.
    *   Internal behavior and potential errors can be monitored using device logs (`npm run build:monitor` or `npm run debug:serial`).
*   **No New Scripts:** This strategy aims to use only the pre-existing `npm` scripts for Modbus interaction.

## 4. Tools

*   **Modbus Read/Write:**
    *   `npm run modbus:read:holding -- --address <addr>`
    *   `npm run modbus:write:holding -- --address <addr> --value <val>`
    *   `npm run modbus:read:coil -- --address <addr>`
    *   `npm run modbus:write:coil -- --address <addr> --value <0|1>`
*   **Logging:**
    *   `npm run build:monitor` (Live serial monitoring)
    *   `npm run debug:serial` (Fetch buffered logs via REST API)
*   **Reference:**
    *   `src/ModbusLogicEngine.h` (for register offsets, enums)
    *   `src/config-modbus.h` (for `MODBUS_LOGIC_RULES_START` address)

## 5. Prerequisites

*   Firmware compiled with `#define ENABLE_MB_SCRIPT` in `config.h` and flashed to the ESP32.
*   Device connected to the network and accessible via its IP address or mDNS name (`modbus-esp32.local` by default).
*   Python environment configured correctly to run the scripts in the `scripts/` directory.
*   Knowledge of the Modbus register mapping for the Logic Engine (starting address + offsets).

## 6. Test Cases

Let `RULE_START = MODBUS_LOGIC_RULES_START`.
Let `RULE_0_BASE = RULE_START`.
Let `RULE_1_BASE = RULE_START + LOGIC_ENGINE_REGISTERS_PER_RULE`.
Offsets are defined in `ModbusLogicEngineOffsets`.

*(Note: Choose suitable, otherwise unused Modbus addresses for source/target registers/coils for testing)*

**TC 1: Basic Rule - Write Holding Register**
1.  **Configure:**
    *   Write `1` to `RULE_0_BASE + ENABLED`.
    *   Write `3` (REG_HOLDING) to `RULE_0_BASE + COND_SRC_TYPE`.
    *   Write `2000` to `RULE_0_BASE + COND_SRC_ADDR`.
    *   Write `0` (EQUAL) to `RULE_0_BASE + COND_OPERATOR`.
    *   Write `123` to `RULE_0_BASE + COND_VALUE`.
    *   Write `3` (WRITE_HOLDING_REGISTER) to `RULE_0_BASE + COMMAND_TYPE`.
    *   Write `2001` to `RULE_0_BASE + COMMAND_TARGET`.
    *   Write `456` to `RULE_0_BASE + COMMAND_PARAM1` (Value to write).
    *   Write `0` to `RULE_0_BASE + COMMAND_PARAM2` (Unused).
    *   Write `0` to `RULE_0_BASE + FLAGS`.
2.  **Trigger:** Write `123` to Modbus address `2000`.
3.  **Verify:**
    *   Read address `2001`. Expected: `456`.
    *   Read `RULE_0_BASE + LAST_STATUS`. Expected: `0` (MB_Error::Success).
    *   Read `RULE_0_BASE + TRIGGER_COUNT`. Expected: `1`.

**TC 2: Condition Operator - Not Equal**
1.  **Configure:** Similar to TC 1, but:
    *   Write `1` (NOT_EQUAL) to `RULE_0_BASE + COND_OPERATOR`.
    *   Write `123` to `RULE_0_BASE + COND_VALUE`.
2.  **Trigger 1:** Write `123` to address `2000`.
3.  **Verify 1:** Read address `2001`. Expected: *Unchanged* from previous state (action should *not* run).
4.  **Trigger 2:** Write `124` to address `2000`.
5.  **Verify 2:**
    *   Read address `2001`. Expected: `456`.
    *   Read `RULE_0_BASE + LAST_STATUS`. Expected: `0` (MB_Error::Success).
    *   Read `RULE_0_BASE + TRIGGER_COUNT`. Expected: Incremented.
    *   *(Repeat for other operators: `<`, `<=`, `>`, `>=`)*

**TC 3: Source Type - Coil**
1.  **Configure:** Similar to TC 1, but:
    *   Write `2` (REG_COIL) to `RULE_0_BASE + COND_SRC_TYPE`.
    *   Write `100` (Test Coil Addr) to `RULE_0_BASE + COND_SRC_ADDR`.
    *   Write `1` to `RULE_0_BASE + COND_VALUE` (Condition is Coil ON).
2.  **Trigger:** Write `1` to Coil address `100`.
3.  **Verify:**
    *   Read address `2001`. Expected: `456`.
    *   Read `RULE_0_BASE + LAST_STATUS`. Expected: `0` (MB_Error::Success).

**TC 4: Action Type - Write Coil**
1.  **Configure:** Similar to TC 1, but:
    *   Write `2` (WRITE_COIL) to `RULE_0_BASE + COMMAND_TYPE`.
    *   Write `101` to `RULE_0_BASE + COMMAND_TARGET`.
    *   Write `1` to `RULE_0_BASE + COMMAND_PARAM1` (Value: ON).
    *   Write `0` to `RULE_0_BASE + COMMAND_PARAM2` (Unused).
2.  **Trigger:** Write `123` to address `2000`.
3.  **Verify:**
    *   Read Coil address `101`. Expected: `1`.
    *   Read `RULE_0_BASE + LAST_STATUS`. Expected: `0` (MB_Error::Success).

**TC 5: Action Type - Call Component Method (Requires Setup)**
*   **Prerequisite:** A method must be registered with the `ModbusLogicEngine` that has a verifiable side-effect readable via Modbus. Example: A simple method in `PHApp` that increments a counter stored in a Modbus register (`e.g., address 3000`).
    ```cpp
    // In PHApp.h (or a test component)
    short testMethod(short p1, short p2) {
        testMethodCounter += p1; // Use p1 (arg1 from rule)
        Log.infoln("Test Method Called! Arg1=%d, Counter: %d", p1, testMethodCounter);
        return E_OK;
    }
    uint16_t testMethodCounter = 0;

    // In PHApp::setup() or where ModbusLogicEngine is initialized
    logicEngine->registerMethod(this->id, 1, // Use app ID and method ID 1
        std::bind(&PHApp::testMethod, this, std::placeholders::_1, std::placeholders::_2));

    // In PHApp::readNetworkValue()
    if (address == 3000) return testMethodCounter;
    ```
1.  **Configure:** Similar to TC 1, but:
    *   Write `100` (CALL_COMPONENT_METHOD) to `RULE_0_BASE + COMMAND_TYPE`.
    *   Write `app->id` to `RULE_0_BASE + COMMAND_TARGET` (Component ID).
    *   Write `1` to `RULE_0_BASE + COMMAND_PARAM1` (Method ID).
    *   Write `5` to `RULE_0_BASE + COMMAND_PARAM2` (Argument 1).
2.  **Initial Read:** Read address `3000`. Note the value (e.g., `X`).
3.  **Trigger:** Write `123` to address `2000`.
4.  **Verify:**
    *   Read address `3000`. Expected: `X + 5`.
    *   Read `RULE_0_BASE + LAST_STATUS`. Expected: `0` (MB_Error::Success).
    *   Check logs (`build:monitor`) for "Test Method Called! Arg1=5".

**TC 6: Rule Enable/Disable**
1.  **Configure:** Configure rule as in TC 1.
2.  **Disable:** Write `0` to `RULE_0_BASE + ENABLED`.
3.  **Trigger:** Write `123` to address `2000`.
4.  **Verify (Disabled):** Read address `2001`. Expected: *Unchanged*. Trigger count should *not* increment.
5.  **Enable:** Write `1` to `RULE_0_BASE + ENABLED`.
6.  **Trigger:** Write `123` to address `2000`.
7.  **Verify (Enabled):** Read address `2001`. Expected: `456`. Trigger count *should* increment.

**TC 7: Error - Invalid Condition Source Address**
1.  **Configure:** Similar to TC 1, but:
    *   Write `9999` (Invalid/Unregistered address) to `RULE_0_BASE + COND_SRC_ADDR`.
2.  **Trigger:** Let the engine loop run.
3.  **Verify:** Read `RULE_0_BASE + LAST_STATUS`. Expected: `2` (MB_Error::IllegalDataAddress).

**TC 8: Error - Invalid Action Target Address (Write)**
1.  **Configure:** Similar to TC 1, but:
    *   Write `9998` (Invalid/Unregistered address) to `RULE_0_BASE + COMMAND_TARGET`.
2.  **Trigger:** Write `123` to address `2000`.
3.  **Verify:** Read `RULE_0_BASE + LAST_STATUS`. Expected: `4` (MB_Error::ServerDeviceFailure).

**TC 9: Error - Invalid Action Target Method (Call)**
1.  **Configure:** Similar to TC 5, but:
    *   Write `99` (Non-existent Method ID) to `RULE_0_BASE + COMMAND_PARAM1`.
2.  **Trigger:** Write `123` to address `2000`.
3.  **Verify:** Read `RULE_0_BASE + LAST_STATUS`. Expected: `2` (MB_Error::IllegalDataAddress).

**TC 10: Status/Counter Reset**
1.  **Configure & Trigger:** Perform TC 1.
2.  **Verify Count:** Read `RULE_0_BASE + TRIGGER_COUNT`. Expected: `1` (or current count).
3.  **Reset Counter:** Write `0` to `RULE_0_BASE + TRIGGER_COUNT`.
4.  **Verify Reset:** Read `RULE_0_BASE + TRIGGER_COUNT`. Expected: `0`.
5.  **Trigger Again:** Write `123` to address `2000`.
6.  **Verify Increment:** Read `RULE_0_BASE + TRIGGER_COUNT`. Expected: `1`.

**TC 11: Debug Flag**
1.  **Configure:** Configure as in TC 1, but:
    *   Write `RULE_FLAG_DEBUG` (value 1) to `RULE_0_BASE + FLAGS`.
2.  **Trigger:** Write `123` to address `2000`.
3.  **Verify:**
    *   Check logs (`build:monitor`). Expected: Verbose logs like "MLE Eval [0]: ...", "MLE Action [0]: ...".
    *   Read address `2001`. Expected: `456`.

**TC 12: Receipt Flag**
1.  **Configure:** Configure as in TC 1, but:
    *   Write `RULE_FLAG_RECEIPT` (value 2) to `RULE_0_BASE + FLAGS`.
2.  **Trigger:** Write `123` to address `2000`.
3.  **Verify:**
    *   Check logs (`build:monitor`). Expected: Info log "MLE: Rule 0 action successful.".
    *   Read address `2001`. Expected: `456`.

**TC 13: Debug + Receipt Flags**
1.  **Configure:** Configure as in TC 1, but:
    *   Write `RULE_FLAG_DEBUG | RULE_FLAG_RECEIPT` (value 3) to `RULE_0_BASE + FLAGS`.
2.  **Trigger:** Write `123` to address `2000`.
3.  **Verify:**
    *   Check logs (`build:monitor`). Expected: Both verbose debug logs and the receipt log.
    *   Read address `2001`. Expected: `456`. 
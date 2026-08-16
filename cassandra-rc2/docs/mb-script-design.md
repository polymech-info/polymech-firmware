# Modbus Logic Engine (MB_SCRIPT) Design

## 1. Overview

The Modbus Logic Engine (`ModbusLogicEngine` class, enabled by `#define ENABLE_MB_SCRIPT`) provides a way to implement simple automation rules directly on the device, configured and monitored via Modbus. It allows users to define rules based on the state of Modbus registers or coils (conditions) and trigger actions like writing to other registers/coils or calling internal component methods.

This enables reactive logic without requiring an external controller polling and writing values constantly.

## 2. Modbus Register Layout

Each logic rule occupies a contiguous block of Modbus holding registers. The number of rules is defined by `MAX_LOGIC_RULES` (default 8) and the number of registers per rule by `LOGIC_ENGINE_REGISTERS_PER_RULE` (currently 13). The starting address for the first rule is defined by `MODBUS_LOGIC_RULES_START`.

**Register Layout per Rule:**

```mermaid
graph TD
    subgraph Rule N
        Reg0[Offset 0: ENABLED (0/1)]
        Reg1[Offset 1: COND_SRC_TYPE (E_RegType)]
        Reg2[Offset 2: COND_SRC_ADDR]
        Reg3[Offset 3: COND_OPERATOR (ConditionOperator)]
        Reg4[Offset 4: COND_VALUE]
        Reg5[Offset 5: COMMAND_TYPE (CommandType)]
        Reg6[Offset 6: COMMAND_TARGET (Addr/CompID)]
        Reg7[Offset 7: COMMAND_PARAM1 (Value/MethodID)]
        Reg8[Offset 8: COMMAND_PARAM2 (Arg1)]
        Reg9[Offset 9: FLAGS (Debug/Receipt)]
        Reg10[Offset 10: LAST_STATUS (MB_Error)]
        Reg11[Offset 11: LAST_TRIGGER_TS (Lower 16bit)]
        Reg12[Offset 12: TRIGGER_COUNT]
    end

    style Reg0 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg1 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg2 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg3 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg4 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg5 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg6 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg7 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg8 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg9 fill:#f9f,stroke:#333,stroke-width:2px
    style Reg10 fill:#ccf,stroke:#333,stroke-width:2px
    style Reg11 fill:#ccf,stroke:#333,stroke-width:2px
    style Reg12 fill:#ccf,stroke:#333,stroke-width:2px

```

**Register Descriptions (Offsets defined in `ModbusLogicEngineOffsets`):**

*   **Configuration Registers (Read/Write via Modbus):**
    *   `ENABLED` (0): `0` = Disabled, `1` = Enabled.
    *   `COND_SRC_TYPE` (1): Type of the Modbus entity to check for the condition. Uses `RegisterState::E_RegType` enum values (e.g., `REG_HOLDING = 3`, `REG_COIL = 2`).
    *   `COND_SRC_ADDR` (2): Modbus address of the register/coil for the condition.
    *   `COND_OPERATOR` (3): Comparison operator to use. Uses `ConditionOperator` enum values (e.g., `EQUAL = 0`, `NOT_EQUAL = 1`, ...).
    *   `COND_VALUE` (4): The value to compare the source register/coil against.
    *   `COMMAND_TYPE` (5): The action to perform if the condition is met. Uses `CommandType` enum values (e.g., `WRITE_COIL = 2`, `WRITE_HOLDING_REGISTER = 3`, `CALL_COMPONENT_METHOD = 100`).
    *   `COMMAND_TARGET` (6): Target of the command.
        *   For `WRITE_*`: The Modbus address to write to.
        *   For `CALL_COMPONENT_METHOD`: The `Component::id` of the target component.
    *   `COMMAND_PARAM1` (7): First parameter for the command.
        *   For `WRITE_*`: The value to write.
        *   For `CALL_COMPONENT_METHOD`: The `methodId` registered with `ModbusLogicEngine::registerMethod`.
    *   `COMMAND_PARAM2` (8): Second parameter for the command.
        *   For `WRITE_*`: Unused.
        *   For `CALL_COMPONENT_METHOD`: The first argument (`arg1`) passed to the registered method.
    *   `FLAGS` (9): Bit flags for rule behavior (See Section 7).

*   **Status Registers (Read-Only via Modbus, except TRIGGER_COUNT reset):**
    *   `LAST_STATUS` (10): Result of the last evaluation/action attempt for this rule. Uses `MB_Error` enum values (e.g., `Success = 0`, `IllegalDataAddress = 2`, `ServerDeviceFailure = 4`).
    *   `LAST_TRIGGER_TS` (11): Timestamp (lower 16 bits of `millis()/1000`) of the last successful trigger.
    *   `TRIGGER_COUNT` (12): Counter of successful triggers. Can be reset by writing `0`.

## 3. Data Structures

*   **`LogicRule` Struct:**
    *   Holds the internal representation of a single rule.
    *   `config[9]`: An array storing the 9 configuration registers (Offset 0 to 8) read from/written to Modbus.
    *   `lastStatus` (type `RuleStatus`/`MB_Error`): Internal state variable reflecting the last status.
    *   `lastTriggerTimestamp` (type `uint32_t`): Internal state variable for the full timestamp.
    *   `triggerCount` (type `uint16_t`): Internal state variable for the trigger count.
    *   Helper methods (`isEnabled()`, `getCondSourceType()`, `getCommandType()`, `getFlags()`, etc.) provide convenient access to the values stored in the `config` array.
*   **`std::vector<LogicRule> rules`:** Member variable in `ModbusLogicEngine` holding all configured rules.
*   **`std::map<uint32_t, CallableMethod> callableMethods`:** Member variable storing methods registered via `registerMethod`, keyed by `(componentId << 16) | methodId`.

## 4. Core Logic Flow

```mermaid
sequenceDiagram
    participant ModbusClient
    participant ModbusManager
    participant ModbusLogicEngine as MLE
    participant TargetComponent

    Note over ModbusClient, MLE: Initialization
    PHApp->>MLE: constructor(app)
    PHApp->>MLE: setup()
    MLE->>MLE: rules.resize(MAX_LOGIC_RULES)
    Note over MLE: Methods registered via registerMethod()
    PHApp->>MLE: registerMethod(compID, methodID, function)
    MLE->>MLE: callableMethods.insert(...)

    loop Application Loop
        PHApp->>MLE: loop()
        alt Not Initialized or Interval Not Met
            MLE-->>PHApp: return E_OK
        else Rule Evaluation
            MLE->>MLE: For each rule in rules vector
            alt Rule Enabled?
                MLE->>MLE: evaluateCondition(rule)
                Note over MLE: Reads condition source
                MLE->>ModbusManager: findComponentForAddress(condAddr)
                ModbusManager-->>MLE: TargetComponent*
                MLE->>TargetComponent: readNetworkValue(condAddr)
                TargetComponent-->>MLE: currentValue (or error)
                Note over MLE: Performs comparison
                alt Condition Met?
                    MLE->>MLE: performAction(rule)
                    opt CommandType == WRITE_*
                        Note over MLE: Performs write action
                        MLE->>ModbusManager: findComponentForAddress(targetAddr)
                        ModbusManager-->>MLE: TargetComponent*
                        MLE->>TargetComponent: writeNetworkValue(targetAddr, value)
                        TargetComponent-->>MLE: E_OK (or error)
                        MLE->>MLE: updateRuleStatus(rule, MB_Error::Success / ServerDeviceFailure)
                    opt CommandType == CALL_COMPONENT_METHOD
                        Note over MLE: Performs method call
                        MLE->>MLE: callableMethods.find(key)
                        alt Method Found?
                           MLE->>TargetComponent: Execute registered std::function(arg1)
                           TargetComponent-->>MLE: E_OK (or error)
                           MLE->>MLE: updateRuleStatus(rule, MB_Error::Success / OpExecutionFailed)
                        else Method Not Found
                           MLE->>MLE: updateRuleStatus(rule, MB_Error::IllegalDataAddress)
                        end
                    end
                    MLE->>MLE: update timestamp & trigger count
                else Condition Not Met
                    MLE->>MLE: updateRuleStatus(rule, MB_Error::Success) if not error
                end
            else Rule Disabled
                MLE->>MLE: Skip rule
            end
        end
    end

    Note over ModbusClient, MLE: Configuration/Status Access
    ModbusClient->>ModbusManager: Read/Write Request (Holding Registers)
    ModbusManager->>MLE: readNetworkValue(addr) / writeNetworkValue(addr, val)
    MLE->>MLE: getRuleInfoFromAddress(addr)
    alt Read Request
        MLE->>MLE: Access rule.config[] or internal status
        MLE-->>ModbusManager: value / status
    else Write Request
        MLE->>MLE: rule.setConfigValue(offset, val) / rule.triggerCount = 0
        MLE-->>ModbusManager: E_OK / error
    end
    ModbusManager-->>ModbusClient: Response

```

*   **`setup()`:** Initializes the `rules` vector based on `MAX_LOGIC_RULES`.
*   **`loop()`:** Called periodically by the main application loop (`PHApp`).
    *   Checks if initialized and if the evaluation `loopInterval` has passed.
    *   Iterates through each `LogicRule` in the `rules` vector.
    *   If a rule `isEnabled()`, it calls `evaluateCondition()`.
    *   If `evaluateCondition()` returns `true` (condition met), it calls `performAction()`.
*   **`evaluateCondition()`:**
    *   Retrieves condition parameters (source type, address, operator, value) from the rule.
    *   Calls `readConditionSourceValue()` to get the current value of the source register/coil.
    *   Performs the comparison based on the `condOperator`.
    *   Updates `rule.lastStatus` (e.g., `MB_Error::IllegalDataAddress` on read failure, `MB_Error::IllegalDataValue` on invalid operator).
    *   Returns `true` if the condition is met, `false` otherwise (including errors).
*   **`performAction()`:**
    *   Retrieves command parameters (type, target, params) from the rule.
    *   Based on `commandType`:
        *   Calls `performWriteAction()` for `WRITE_*` commands.
        *   Calls `performCallAction()` for `CALL_COMPONENT_METHOD`.
    *   Updates `rule.lastStatus` based on the success/failure of the action (e.g., `MB_Error::Success`, `MB_Error::ServerDeviceFailure`, `MB_Error::OpExecutionFailed`, `MB_Error::IllegalFunction`).
    *   If successful, updates `rule.lastTriggerTimestamp` and increments `rule.triggerCount`.
    *   Returns `true` on success, `false` on failure.

## 5. Interaction with ModbusManager

The `ModbusLogicEngine` relies on the `ModbusManager` (accessed via the `PHApp* app` pointer) to interact with other components' Modbus values.

*   **`readConditionSourceValue()`:**
    *   Takes the source type (`RegisterState::E_RegType`) and address.
    *   Uses `app->modbusManager->findComponentForAddress(address)` to find the component responsible for that address.
    *   Calls the target component's `readNetworkValue(address)` method.
    *   Returns the value or indicates failure.
*   **`performWriteAction()`:**
    *   Takes the command type (`WRITE_COIL` or `WRITE_HOLDING_REGISTER`), target address, and value.
    *   Uses `app->modbusManager->findComponentForAddress(address)` to find the target component.
    *   Calls the target component's `writeNetworkValue(address, value)` method.
    *   Returns `true` if the write call returns `E_OK`, `false` otherwise.

## 6. Method Calling (`CALL_COMPONENT_METHOD`)

This command type allows rules to trigger C++ methods within other components.

*   **Registration:** Components (like `PHApp` or custom components) that want to expose methods to the Logic Engine must call `ModbusLogicEngine::registerMethod(componentId, methodId, method)`.
    *   `componentId`: The unique `Component::id` of the component exposing the method.
    *   `methodId`: A unique ID (within that component) for the specific method being exposed.
    *   `method`: A `std::function<short(short, short)>` object wrapping the actual C++ method (often created using `std::bind`). The function should accept two `short` arguments and return `E_OK` (or another error code).
    *   The engine stores this registration in the `callableMethods` map.
*   **Configuration:** A rule is configured with `COMMAND_TYPE = CALL_COMPONENT_METHOD`.
    *   `COMMAND_TARGET` is set to the `componentId`.
    *   `COMMAND_PARAM1` is set to the `methodId`.
    *   `COMMAND_PARAM2` is set to the value to be passed as the first argument (`arg1`) to the registered C++ method.
*   **Execution (`performCallAction()`):**
    *   Combines `componentId` (from Target) and `methodId` (from Param1) into a key.
    *   Looks up the key in the `callableMethods` map.
    *   If found, executes the stored `std::function`, passing `param2` (as `arg1`) and a dummy `0` (as `arg2`) to the bound C++ method.
    *   Updates status based on the return value of the C++ method (`MB_Error::Success` if `E_OK`, `MB_Error::OpExecutionFailed` otherwise) or `MB_Error::IllegalDataAddress` if the method was not found in the map.

*Note: This `std::function`-based registration is internal to the `ModbusLogicEngine` and separate from the `Bridge` mechanism used for serial commands.*

## 7. Flags (`FLAGS` Register - Offset 9)

The `FLAGS` register allows modifying rule behavior using bitmasks:

*   **`RULE_FLAG_DEBUG` (Bit 0 / Value 1):** If set, enables verbose logging (`Log.verboseln`) during the evaluation and action phases for this specific rule, showing details like addresses, values, and outcomes.
*   **`RULE_FLAG_RECEIPT` (Bit 1 / Value 2):** If set, logs an informational message (`Log.infoln`) whenever the rule's action is executed successfully.

These flags can be combined (e.g., value `3` enables both).

## 8. Modbus Interface (`read/writeNetworkValue`)

*   The `ModbusLogicEngine` implements `readNetworkValue` and `writeNetworkValue` as required by the `Component` base class.
*   These methods are called by `ModbusManager` when a Modbus client reads/writes registers within the engine's address range (`MODBUS_LOGIC_RULES_START` onwards).
*   They calculate the `ruleIndex` and `offset` from the requested Modbus address.
*   **Read:**
    *   If the offset corresponds to a configuration register (0-8), it returns the value from `rule.config[offset]`.
    *   If the offset is `FLAGS` (9), it returns the flag value via `getFlags()`.
    *   If the offset corresponds to a status register (10-12), it returns the value from the internal state variables (`rule.lastStatus`, `rule.lastTriggerTimestamp & 0xFFFF`, `rule.triggerCount`).
*   **Write:**
    *   If the offset corresponds to a configuration register (0-9, including FLAGS), it updates `rule.config[offset]` using `rule.setConfigValue()`.
    *   If the offset is `TRIGGER_COUNT` (12) and the value is `0`, it resets `rule.triggerCount`.
    *   Writes to read-only status registers (10, 11) are disallowed and return an error. 
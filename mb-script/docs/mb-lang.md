# Modbus Logic Language (mb-lang)

This document describes the design and usage of the simple logic engine configurable via Modbus TCP.

## Purpose

The Modbus Logic Engine allows users to define simple conditional automation rules directly by writing to specific Modbus holding registers. This enables basic automation sequences like "if sensor value X exceeds Y, then turn on relay Z" without modifying the core firmware code.

## Architecture

A dedicated component, `ModbusLogicEngine`, runs within the firmware.
- It exposes a block of Modbus Holding Registers for configuration and status.
- It periodically evaluates the enabled rules.
- It can read the state of other Modbus registers/coils.
- It can perform actions like writing to Modbus registers/coils or calling pre-defined methods on other firmware components.
- It updates status registers after each rule evaluation/action attempt.

## Configuration and Status

The engine supports a fixed number of logic rules, defined by `MAX_LOGIC_RULES` (e.g., 8). Each rule is configured and monitored using a block of `REGISTERS_PER_RULE` (now 13) consecutive Holding Registers.

- **Base Address:** `MODBUS_LOGIC_RULES_START` (Defined in `config-modbus.h`)
- **Rule N Address:** `MODBUS_LOGIC_RULES_START + (N * REGISTERS_PER_RULE)` where `N` is the rule index (0 to `MAX_LOGIC_RULES - 1`).

### Register Map per Rule (N)

| Offset | Register Name                 | Address Offset                    | R/W | Description                                                                                                | Notes                                                                       |
| :----- | :---------------------------- | :-------------------------------- | :-- | :--------------------------------------------------------------------------------------------------------- | :-------------------------------------------------------------------------- |
| +0     | `Rule_N_Enabled`              | `Base + (N*13) + 0`               | R/W | **Enable/Disable Rule:** 0 = Disabled, 1 = Enabled                                                         | Rules are ignored if disabled.                                              |
| +1     | `Rule_N_Cond_Src_Type`        | `Base + (N*13) + 1`               | R/W | **Condition Source Type:** 0 = Holding Register, 1 = Coil                                                  | Specifies what type of Modbus item the condition reads.                     |
| +2     | `Rule_N_Cond_Src_Addr`        | `Base + (N*13) + 2`               | R/W | **Condition Source Address:** Modbus address of the register/coil to read for the condition.               | The address of the data point to check.                                     |
| +3     | `Rule_N_Cond_Operator`        | `Base + (N*13) + 3`               | R/W | **Condition Operator:** 0=`==`, 1=`!=`, 2=`<`, 3=`<=`, 4=`>`, 5=`>=`                                          | The comparison to perform.                                                  |
| +4     | `Rule_N_Cond_Value`           | `Base + (N*13) + 4`               | R/W | **Condition Value:** The value to compare the source against.                                              | For Coils, use 0 for OFF and 1 for ON.                                      |
| +5     | `Rule_N_Action_Type`          | `Base + (N*13) + 5`               | R/W | **Action Type:** 0=None, 1=Write Holding Reg, 2=Write Coil, 3=Call Component Method                          | Specifies what to do if the condition is true.                              |
| +6     | `Rule_N_Action_Target`        | `Base + (N*13) + 6`               | R/W | **Action Target:** Modbus Address (for Write Reg/Coil) or Component ID (for Call Method)                   | Specifies *what* to act upon (Register Address, Coil Address, Component ID). |
| +7     | `Rule_N_Action_Param1`        | `Base + (N*13) + 7`               | R/W | **Action Parameter 1:** Value (for Write Reg), ON/OFF (for Write Coil, 0=OFF, 1=ON), Method ID (for Call) | Specifies *how* to act upon the target.                                     |
| +8     | `Rule_N_Action_Param2`        | `Base + (N*13) + 8`               | R/W | **Action Parameter 2:** Argument 1 for `Call Component Method`                                             | First argument for the component method. Ignored for Write actions.         |
| +9     | `Rule_N_Action_Param3`        | `Base + (N*13) + 9`               | R/W | **Action Parameter 3:** Argument 2 for `Call Component Method`                                             | Second argument for the component method. Ignored for Write actions.        |
| +10    | `Rule_N_Last_Status`          | `Base + (N*13) + 10`              | R   | **Last Action Status:** 0=Idle/OK, 1=Err Cond Read, 2=Err Action Write/Call, 3=Invalid Action Params        | Reports the outcome of the last trigger attempt. (See Status Codes below)   |
| +11    | `Rule_N_Last_Trigger_Timestamp` | `Base + (N*13) + 11`              | R   | **Last Trigger Timestamp:** System time (e.g., seconds since boot) when rule last triggered.               | 0 if never triggered. Wraps eventually.                                     |
| +12    | `Rule_N_Trigger_Count`        | `Base + (N*13) + 12`              | R   | **Trigger Count:** Number of times this rule's action has been successfully triggered.                    | Wraps eventually. Can be reset by writing 0 via Modbus.                    |

*Note: `Base` refers to `MODBUS_LOGIC_RULES_START`. R=Read-Only, W=Writeable (from Modbus perspective). Status registers (+10 to +12) are updated internally but the count (+12) can potentially be reset via write.*

### Status Codes (`Rule_N_Last_Status`)

| Code | Meaning                     |
| :--- | :-------------------------- |
| 0    | Idle / Action OK            |
| 1    | Error Reading Condition Src |
| 2    | Error Performing Action     |
| 3    | Invalid Action Parameters   |
| 4    | Component Method Call Failed|
| ...  | (Other specific errors TBD) |

## Actions

### 1. Write Holding Register

- **Action Type:** 1
- **Action Target:** Modbus address of the Holding Register.
- **Action Parameter 1:** Value to write.
- **Action Parameters 2 & 3:** Ignored.

### 2. Write Coil

- **Action Type:** 2
- **Action Target:** Modbus address of the Coil.
- **Action Parameter 1:** Value to write (0 for OFF, 1 for ON). Other non-zero values may also be interpreted as ON.
- **Action Parameters 2 & 3:** Ignored.

### 3. Call Component Method

- **Action Type:** 3
- **Action Target:** The numeric `Component ID`.
- **Action Parameter 1:** The numeric `Method ID`.
- **Action Parameter 2:** The first integer argument (`arg1`).
- **Action Parameter 3:** The second integer argument (`arg2`).

**Important:** Only specific, pre-registered methods can be called. Available Component/Method IDs need separate documentation.

## Execution Flow

1.  The `ModbusLogicEngine` loops periodically.
2.  For each rule `N` from 0 to `MAX_LOGIC_RULES - 1`:
    a.  Read `Rule_N_Enabled`. If 0, skip.
    b.  Read condition parameters.
    c.  Attempt to read the current value from `Cond_Src_Addr`.
    d.  If read fails, update `Rule_N_Last_Status` (e.g., to 1) and skip to the next rule.
    e.  Evaluate the condition.
    f.  If the condition is TRUE:
        i.  Read action parameters.
        ii. Attempt to perform the specified action.
        iii. Update `Rule_N_Last_Status` based on action success (0) or failure (e.g., 2, 3, 4).
        iv. If action was successful, increment `Rule_N_Trigger_Count` and update `Rule_N_Last_Trigger_Timestamp`.
    g.  If the condition is FALSE, potentially reset `Rule_N_Last_Status` to 0 (Idle), unless it holds an error state.

## Example Scenarios

*(Addresses updated for REGISTERS_PER_RULE = 13)*

### Example 1: Turn on Relay 5 if Register 200 >= 100

Assume:
- `MODBUS_LOGIC_RULES_START` = 1000
- Rule Index `N = 0` (`Base = 1000`)
- Relay 5 is mapped to Coil address 5
- Register 200

Write:

| Register Address | Value | Meaning                   |
| :--------------- | :---- | :------------------------ |
| 1000             | 1     | Rule 0: Enabled           |
| 1001             | 0     | Cond Src Type: Reg        |
| 1002             | 200   | Cond Src Addr: 200        |
| 1003             | 5     | Cond Operator: >= (5)     |
| 1004             | 100   | Cond Value: 100           |
| 1005             | 2     | Action Type: Write Coil   |
| 1006             | 5     | Action Target: Coil Addr 5|
| 1007             | 1     | Action Param 1: Value ON  |
| 1008             | 0     | Action Param 2: (Ignored) |
| 1009             | 0     | Action Param 3: (Ignored) |

Read Status (after trigger):

| Register Address | Value         | Meaning                     |
| :--------------- | :------------ | :-------------------------- |
| 1010             | 0             | Last Status: OK             |
| 1011             | e.g., 12345   | Last Trigger: Timestamp     |
| 1012             | e.g., 1       | Trigger Count: 1            |

### Example 2: Call `resetCounter()` Method on Component `StatsTracker` if Coil 10 is ON

Assume:
- Rule Index `N = 1` (`Base = 1000 + 13 = 1013`)
- Coil 10
- Component `StatsTracker` ID = 5
- Method `resetCounter` ID = 1 (takes no args)

Write:

| Register Address | Value | Meaning                       |
| :--------------- | :---- | :---------------------------- |
| 1013             | 1     | Rule 1: Enabled               |
| 1014             | 1     | Cond Src Type: Coil           |
| 1015             | 10    | Cond Src Addr: Coil 10        |
| 1016             | 0     | Cond Operator: == (0)         |
| 1017             | 1     | Cond Value: ON (1)            |
| 1018             | 3     | Action Type: Call Method      |
| 1019             | 5     | Action Target: Component ID 5 |
| 1020             | 1     | Action Param 1: Method ID 1   |
| 1021             | 0     | Action Param 2: Arg1 = 0      |
| 1022             | 0     | Action Param 3: Arg2 = 0      |

Read Status (after trigger):

| Register Address | Value         | Meaning                     |
| :--------------- | :------------ | :-------------------------- |
| 1023             | 0             | Last Status: OK             |
| 1024             | e.g., 12360   | Last Trigger: Timestamp     |
| 1025             | e.g., 1       | Trigger Count: 1            |

## Limitations & Considerations

- **Complexity:** Only simple, single conditions per rule. No `AND`/`OR`/`ELSE`.
- **Execution Order:** Rules evaluated sequentially.
- **Performance:** Rule evaluation takes time. Consider impact and execution frequency.
- **Timestamp:** The `Last_Trigger_Timestamp` resolution and potential for wrapping depend on the firmware implementation (e.g., `millis()` overflow, using seconds since boot).
- **Error Handling:** Status codes provide basic feedback. More detailed logging might be needed for complex debugging.
- **Method Availability:** Callable methods are fixed in firmware.
- **Concurrency:** Actions (especially method calls) might take time. The engine design needs to consider if rule evaluation should block or if actions run asynchronously (current design implies synchronous execution within the loop).


## Limitations & Considerations

- **Complexity:** Only simple, single conditions per rule are supported. No `AND`/`OR` or `ELSE` logic.
- **Execution Order:** Rules are evaluated sequentially. Be mindful of potential interactions if multiple rules modify the same target.
- **Performance:** Reading Modbus values and executing actions takes time. Complex rules or a large number of rules might impact overall system performance. Rule execution frequency should be considered.
- **Error Handling:** The current design doesn't explicitly define Modbus registers for rule execution status or errors. This could be added later.
- **Method Availability:** The list of callable methods (`Component ID`, `Method ID`) is fixed in the firmware and needs to be documented for users. 
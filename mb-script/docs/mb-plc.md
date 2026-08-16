# Comparison: Modbus Logic Engine (MB_SCRIPT) vs. PLC Ladder Logic

## 1. Introduction

This document compares the custom Modbus Logic Engine (`MB_SCRIPT`) implemented in this firmware with traditional PLC (Programmable Logic Controller) programming languages, primarily focusing on Ladder Logic (LD).

*   **Modbus Logic Engine (MB_SCRIPT):** A feature enabling simple, rule-based automation directly within the ESP32 firmware. Rules are configured and monitored via Modbus registers. It allows defining conditions based on Modbus values and triggering actions like writing Modbus values or calling internal C++ functions.
*   **PLC Ladder Logic (LD):** A graphical programming language widely used in industrial automation. It mimics the appearance of electrical relay logic diagrams, making it intuitive for electricians and technicians. It runs on dedicated PLC hardware in a cyclic scan execution model.

## 2. Core Concepts Comparison

| Feature                 | PLC Ladder Logic (Typical)                                     | Modbus Logic Engine (MB_SCRIPT)                                     |
| :---------------------- | :------------------------------------------------------------- | :------------------------------------------------------------------ |
| **Programming Model**   | Graphical (Relay Logic Diagram), Textual (ST, FBD often avail.) | Rule-Based (IF condition THEN action), Configuration via Registers |
| **Representation**      | Rungs, Contacts (NO/NC), Coils, Function Blocks (Timers, Cnt) | Blocks of Holding Registers defining rules                           |
| **Execution Model**     | Cyclic Scan (Read Inputs -> Solve Logic -> Write Outputs)      | Iterative `loop()` checks rules sequentially based on interval      |
| **Data Addressing**     | Standardized I/O points (%I, %Q), Memory (%M), Data Regs (%MW) | Modbus Addresses (Coils/Registers) managed by firmware components |
| **Built-in Functions**  | Timers (TON/TOF), Counters (CTU/CTD), Math, Compare, Move    | Basic Comparisons (==, !=, <, >...), Write Coil/Reg, Call Method |
| **Complex Logic**       | Subroutines, Jumps, Structured Text (ST), Function Blocks    | Requires registering custom C++ methods (`CallableMethod`)          |
| **Configuration Tool**  | Dedicated PLC IDE (e.g., TIA Portal, RSLogix)                | Standard Modbus Client/Master Software                              |
| **Deployment**          | Download program binary to PLC hardware                        | Write configuration values to Modbus registers                       |
| **Debugging/Monitor**   | Online monitoring in IDE, Forcing I/O, Status LEDs           | Reading Status registers via Modbus, Serial Logs (Debug/Receipt Flags) |
| **Hardware**            | Dedicated Industrial PLC Hardware                              | Runs on the ESP32 microcontroller within the firmware             |
| **Extensibility**       | Adding I/O modules, using advanced function blocks, libraries | Adding/Registering C++ methods, potentially modifying engine code |
| **Target User**         | Automation Engineers, Technicians, Electricians              | Firmware developers, System integrators familiar with Modbus      |

## 3. Detailed Comparison

### 3.1. Programming Model & Representation

*   **Ladder Logic:** Visual and intuitive for those familiar with electrical schematics. Logic flows left-to-right across rungs. Standard symbols for contacts, coils, timers, etc., are well-understood in the industry.
*   **MB_SCRIPT:** Abstract. Logic is defined by setting numerical values in specific Modbus registers according to predefined offsets (`ModbusLogicEngineOffsets`). Requires understanding the specific register map and enum values (`E_RegType`, `ConditionOperator`, `CommandType`, `MB_Error`). Less visual and more data-entry focused.

### 3.2. Execution

*   **Ladder Logic:** Typically deterministic cyclic scan. The PLC reads all inputs, executes the entire ladder program from top to bottom, and then updates all outputs in a predictable cycle time.
*   **MB_SCRIPT:** Executes within the ESP32's main `loop()`. Rules are checked sequentially within the `ModbusLogicEngine::loop()` call, which runs periodically based on `loopInterval`. Execution time can be influenced by other tasks running on the ESP32. It's event-driven based on Modbus value changes *observed* during rule evaluation, but the evaluation itself is interval-based.

### 3.3. Data Handling

*   **Ladder Logic:** Uses well-defined memory areas for inputs, outputs, internal bits, timers, counters, and data registers, accessed via standardized addressing.
*   **MB_SCRIPT:** Relies entirely on the existing Modbus address space managed by other firmware components (`ModbusManager`). Condition sources and action targets are arbitrary Modbus addresses. Internal rule state (status, timestamp, count) is exposed via dedicated status registers within the rule's block. Complex data manipulation requires C++ functions.

### 3.4. Capabilities & Functionality

*   **Ladder Logic:** Offers standard, pre-built function blocks for common automation tasks like timing delays, counting events, basic arithmetic, and data manipulation. More advanced PLCs include PID loops, communication protocols, motion control, etc.
*   **MB_SCRIPT:** Provides fundamental building blocks: compare Modbus values and trigger simple actions (write Modbus value, call internal function). It lacks built-in timers, counters, or complex math operations within the rule definition itself. Such functionality must be implemented in C++ and exposed via the `CALL_COMPONENT_METHOD` command.

### 3.5. Configuration & Deployment

*   **Ladder Logic:** Requires specialized, often vendor-specific, programming software. The compiled logic is downloaded as a binary program to the PLC. Configuration is done offline in the IDE.
*   **MB_SCRIPT:** Configured "online" by writing values to Modbus registers using any standard Modbus master tool. No separate compilation or download step for the logic itself is needed (only for the firmware containing the engine). This allows dynamic reconfiguration without reflashing firmware (though registered methods are fixed in firmware).

### 3.6. Debugging & Monitoring

*   **Ladder Logic:** IDEs provide powerful online monitoring tools, allowing visualization of rung state, live value tracing, and forcing of inputs/outputs for testing. PLCs often have hardware status LEDs.
*   **MB_SCRIPT:** Debugging relies on reading the `LAST_STATUS`, `LAST_TRIGGER_TS`, and `TRIGGER_COUNT` registers via Modbus. More detail requires enabling the `RULE_FLAG_DEBUG` and `RULE_FLAG_RECEIPT` flags and monitoring the device's serial output (`npm run build:monitor`). Less interactive than typical PLC debugging.

## 4. Strengths & Weaknesses

**MB_SCRIPT:**

*   **Strengths:**
    *   Simple rule structure, easy to understand if the register map is known.
    *   Configuration via standard Modbus tools - no specialized IDE needed.
    *   Runs directly on the ESP32, reducing need for external micro-controllers for simple logic.
    *   Extensible via C++ (`CALL_COMPONENT_METHOD`) for complex custom actions.
    *   Logic configuration can potentially be updated without firmware reflash.
*   **Weaknesses:**
    *   Limited built-in functions (no timers, counters, complex math within rules).
    *   Logic representation is abstract (register values) and not visual.
    *   Debugging relies heavily on Modbus reads and serial logs.
    *   Sequential rule execution might have timing implications compared to cyclic scan.
    *   Scalability limited by `MAX_LOGIC_RULES` and ESP32 resources.
    *   Complex logic requires C++ development and method registration.

**PLC Ladder Logic:**

*   **Strengths:**
    *   Visual and intuitive programming paradigm (especially LD).
    *   Industry standard, widely understood by technicians and engineers.
    *   Rich set of built-in standard functions (timers, counters, etc.).
    *   Mature, powerful IDEs with excellent online monitoring and debugging capabilities.
    *   Deterministic execution (typically).
    *   Highly scalable with modular hardware.
*   **Weaknesses:**
    *   Requires dedicated, often expensive, PLC hardware.
    *   Requires specialized, often expensive, vendor-specific IDE software.
    *   Less straightforward to integrate directly with custom C++ functions or device-specific hardware features compared to code running *on* the device.
    *   Configuration/updates typically require offline editing and download.

## 5. Use Cases

*   **MB_SCRIPT is suitable for:**
    *   Simple, reactive logic based directly on Modbus values on the device.
    *   Triggering predefined C++ functions based on Modbus conditions.
    *   Implementing basic interlocks or sequences configurable via Modbus.
    *   Situations where adding a full PLC is overkill or impractical.
    *   Environments where configuration via standard Modbus tools is preferred.
*   **PLC Ladder Logic is suitable for:**
    *   Complex industrial automation and control sequences.
    *   Applications requiring standard timing, counting, and process control functions.
    *   Systems needing robust, deterministic execution.
    *   Environments where technicians are primarily familiar with Ladder Logic.
    *   Large-scale systems with extensive I/O requirements.

## 6. Conclusion

The Modbus Logic Engine (`MB_SCRIPT`) provides a useful, lightweight mechanism for implementing simple, Modbus-driven automation rules directly on the ESP32 firmware. It leverages the existing Modbus infrastructure for configuration and interaction but relies on C++ for complex actions. It is not a replacement for a full PLC system using Ladder Logic, which offers a more comprehensive, standardized, and visually intuitive environment with richer built-in capabilities tailored for industrial control, albeit with the requirement of dedicated hardware and software. The choice depends on the complexity of the required logic, the target environment, user expertise, and integration needs. 
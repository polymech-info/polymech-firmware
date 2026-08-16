#ifndef MODBUS_LOGIC_ENGINE_H
#define MODBUS_LOGIC_ENGINE_H

#include "config.h" 

#ifdef ENABLE_MB_SCRIPT

#include <Component.h>
#include <ArduinoLog.h>
#include <vector>
#include <cstdint> // For uint16_t etc.
#include <modbus/ModbusTypes.h>

// Forward declarations
class PHApp; // Assuming PHApp provides access to other components/Modbus

// --- Configuration Constants (Define these in config-modbus.h or similar) ---
#ifndef MAX_LOGIC_RULES
#define MAX_LOGIC_RULES 8 // Default number of rules
#endif

#ifndef LOGIC_ENGINE_REGISTERS_PER_RULE
#define LOGIC_ENGINE_REGISTERS_PER_RULE 13 // Now 13 (removed Param3)
#endif

#ifndef MODBUS_LOGIC_RULES_START
#define MODBUS_LOGIC_RULES_START 1000 // Example starting address
#endif
// --- End Configuration Constants ---

// Define constants for rule structure offsets (matching mb-lang.md)
namespace ModbusLogicEngineOffsets {
    // --- Configuration (8 registers) ---
    const short ENABLED = 0;
    const short COND_SRC_TYPE = 1;
    const short COND_SRC_ADDR = 2;
    const short COND_OPERATOR = 3;
    const short COND_VALUE = 4;
    const short COMMAND_TYPE = 5;
    const short COMMAND_TARGET = 6;
    // Parameters below depend on COMMAND_TYPE:
    // - WRITE_HOLDING_REGISTER: PARAM1=Value
    // - WRITE_COIL: PARAM1=Value (0 or 1)
    // - CALL_COMPONENT_METHOD: PARAM1=MethodID, PARAM2=Arg1
    const short COMMAND_PARAM1 = 7;
    const short COMMAND_PARAM2 = 8;
    // Removed PARAM3

    // --- Status & Flags (Moved to end - 4 registers) ---
    const short FLAGS = 9;         // Moved
    const short LAST_STATUS = 10;    // Moved
    const short LAST_TRIGGER_TS = 11; // Moved
    const short TRIGGER_COUNT = 12;   // Moved
}

// --- Rule Flags (Bitmasks for FLAGS register) ---
#define RULE_FLAG_DEBUG   (1 << 0) // Enable verbose debug logging for this rule
#define RULE_FLAG_RECEIPT (1 << 1) // Enable logging upon successful trigger/action

// Define constants for condition operators
enum class ConditionOperator : uint16_t {
    EQUAL = 0,
    NOT_EQUAL = 1,
    LESS_THAN = 2,
    LESS_EQUAL = 3,
    GREATER_THAN = 4,
    GREATER_EQUAL = 5
};

// Define constants for command types
// Partially aligned with E_MB_OpType from ModbusTypes.h
enum class CommandType : uint16_t {
    NONE = 0,
    // Use standard op types where possible (Values from E_MB_OpType)
    WRITE_COIL = 2,             // Matches E_MB_OpType::MB_WRITE_COIL
    WRITE_HOLDING_REGISTER = 3, // Matches E_MB_OpType::MB_WRITE_REGISTER
    // Custom command type (value > standard op types)
    CALL_COMPONENT_METHOD = 100
};

// Type alias for rule status using standard Modbus errors
using RuleStatus = MB_Error;

// Structure to hold the configuration and state of a single rule
struct LogicRule {
    // --- Configuration (Set via Modbus) ---
    // Store config registers directly (Enabled to Param2)
    uint16_t config[LOGIC_ENGINE_REGISTERS_PER_RULE - 4]; // Size now 13-4 = 9

    // Helper accessors for configuration
    bool isEnabled() const { return config[ModbusLogicEngineOffsets::ENABLED] == 1; }
    RegisterState::E_RegType getCondSourceType() const { return static_cast<RegisterState::E_RegType>(config[ModbusLogicEngineOffsets::COND_SRC_TYPE]); }
    uint16_t getCondSourceAddr() const { return config[ModbusLogicEngineOffsets::COND_SRC_ADDR]; }
    ConditionOperator getCondOperator() const { return static_cast<ConditionOperator>(config[ModbusLogicEngineOffsets::COND_OPERATOR]); }
    uint16_t getCondValue() const { return config[ModbusLogicEngineOffsets::COND_VALUE]; }
    CommandType getCommandType() const { return static_cast<CommandType>(config[ModbusLogicEngineOffsets::COMMAND_TYPE]); }
    uint16_t getCommandTarget() const { return config[ModbusLogicEngineOffsets::COMMAND_TARGET]; }
    uint16_t getCommandParam1() const { return config[ModbusLogicEngineOffsets::COMMAND_PARAM1]; }
    uint16_t getCommandParam2() const { return config[ModbusLogicEngineOffsets::COMMAND_PARAM2]; }

    // --- Status & Flags (Read/Write via Modbus, updated internally) ---
    // Note: These are conceptually separate but stored contiguously in Modbus address space.
    //       The internal representation uses separate members for status/timestamp/count
    //       and reads/writes the FLAGS register (config[9]) via Modbus.
    uint16_t getFlags() const { return config[ModbusLogicEngineOffsets::FLAGS]; }
    bool isDebugEnabled() const { return (getFlags() & RULE_FLAG_DEBUG) != 0; }
    bool isReceiptEnabled() const { return (getFlags() & RULE_FLAG_RECEIPT) != 0; }

    void setConfigValue(short offset, uint16_t value) {
        // Size check updated for new register count (9 config registers)
        if (offset >= 0 && offset < (LOGIC_ENGINE_REGISTERS_PER_RULE - 4)) {
             config[offset] = value;
        } else {
             Log.warningln(F("MLE: Attempt to write invalid config offset %d"), offset);
        }
    }

    // Internal state members (not directly part of config array)
    RuleStatus lastStatus = RuleStatus::Success;
    uint32_t lastTriggerTimestamp = 0;
    uint16_t triggerCount = 0;

    LogicRule() {
        // Initialize configuration registers (0-8)
        for(int i = 0; i < (LOGIC_ENGINE_REGISTERS_PER_RULE - 4); ++i) {
            config[i] = 0;
        }
    }
};


class ModbusLogicEngine : public Component {
public:
    // Define the callable method signature: takes two shorts, returns a short (e.g., error code)
    // Using std::function allows storing lambdas, member functions, etc.
    // NOTE: This internal registration is used for CALL_COMPONENT_METHOD.
    // It is separate from the Bridge mechanism used for serial commands.
    using CallableMethod = std::function<short(short, short)>;

    // Constructor - Requires access to PHApp to interact with other components/Modbus
    ModbusLogicEngine(PHApp* ownerApp); // Pass PHApp reference

    virtual ~ModbusLogicEngine() = default;

    short setup() override;
    short loop() override;

    // Handle Modbus reads/writes for rule configuration and status
    short mb_tcp_read(short address) override;
    short mb_tcp_write(short address, short value) override;

    /**
     * @brief Registers a method that can be called by the logic engine.
     *
     * @param componentId A unique ID for the component exposing the method.
     * @param methodId A unique ID for the method within that component.
     * @param method The function object (lambda, bound member function) to call.
     * @return True if registration was successful, false otherwise (e.g., duplicate ID).
     */
    bool registerMethod(uint16_t componentId, uint16_t methodId, CallableMethod method);

private:
    PHApp* app; // Pointer to the main application instance
    bool initialized_; // Flag to track initialization state

    // Storage for all logic rules
    std::vector<LogicRule> rules;

    // Registry for callable methods: Map<(ComponentID << 16) | MethodID, Function>
    // Combining IDs into a single 32-bit key for the map.
    std::map<uint32_t, CallableMethod> callableMethods;

    // --- Internal Logic ---
    unsigned long lastLoopTime = 0;
    const unsigned long loopInterval = 100; // Evaluate rules every 100ms (adjust as needed)

    bool evaluateCondition(LogicRule& rule);
    bool performAction(LogicRule& rule);

    // --- Helper Methods ---
    // Reads the value required for a rule's condition
    // Updated signature to use RegisterState::E_RegType
    bool readConditionSourceValue(RegisterState::E_RegType type, uint16_t address, uint16_t& value);
    // Performs the write action (Register or Coil)
    // Updated signature to use CommandType
    bool performWriteAction(CommandType type, uint16_t address, uint16_t value);
    // Performs the method call action (now only 2 params)
    bool performCallAction(uint16_t componentId, uint16_t methodId, uint16_t arg1);

    // Helper to calculate rule index and offset from Modbus address
    bool getRuleInfoFromAddress(short address, int& ruleIndex, short& offset);

    // Helper to update status fields
    // Updated signature to use RuleStatus (MB_Error)
    void updateRuleStatus(LogicRule& rule, RuleStatus status);
};

#endif // ENABLE_MB_SCRIPT

#endif // MODBUS_LOGIC_ENGINE_H 
// Include config first to get the feature flag
#include "config.h" 

#ifdef ENABLE_MB_SCRIPT

#include "ModbusLogicEngine.h"
#include "PHApp.h" // Include PHApp to access other components/methods
#include "ModbusTCP.h" // To potentially interact with Modbus directly if needed
#include "config-modbus.h" // Assuming constants like MODBUS_LOGIC_RULES_START are here
#include "enums.h"       // For error codes like E_OK, component states etc.
#include <Arduino.h>    // For millis()
#include <cmath>        // For isnan, isinf if needed
#include <cstring>      // For strncpy
#include <ArduinoLog.h>

// Placeholder for Component ID - Define this properly elsewhere (e.g., enums.h)
#ifndef COMPONENT_ID_MLE
#define COMPONENT_ID_MLE 99
#endif

// --- Constructor ---
ModbusLogicEngine::ModbusLogicEngine(PHApp* ownerApp)
    // Using the most complete constructor signature found in error logs
    // Assuming 0 for default flags
    : Component("LogicEngine", COMPONENT_ID_MLE, 0, ownerApp), app(ownerApp), initialized_(false) { // Initialize internal state flag
    // Name, ID, Flags, Owner set in base initializer list
}

// --- Setup ---
short ModbusLogicEngine::setup() {
    rules.resize(MAX_LOGIC_RULES); // Allocate space for all rules

    // Optional: Load rules from persistent storage if implemented

    L_INFO(F("MLE: Initialized %d rules."), MAX_LOGIC_RULES);
    // Set internal initialization flag
    initialized_ = true;
    return E_OK;
}

// --- Loop (Rule Evaluation) ---
short ModbusLogicEngine::loop() {
    if (!initialized_) {
        return E_OK; // Not ready to run
    }

    unsigned long currentTime = millis();
    if (currentTime - lastLoopTime < loopInterval) {
        return E_OK; // Not time to evaluate yet
    }
    lastLoopTime = currentTime;

    //Log.verboseln(F("MLE: Evaluating rules..."));

    for (int i = 0; i < rules.size(); ++i) {
        LogicRule& rule = rules[i];

        if (!rule.isEnabled()) {
            continue; // Skip disabled rules
        }

        bool conditionMet = false;
        bool conditionEvalSuccess = evaluateCondition(rule);

        if (!conditionEvalSuccess) {
            // Error already logged and status updated in evaluateCondition
            if(rule.isDebugEnabled()) Log.verboseln(F("MLE: Rule %d condition eval FAILED."), i);
            continue; // Move to the next rule
        }

        conditionMet = conditionEvalSuccess; // If evaluation didn't fail, the result is stored

        if (conditionMet) {
            if(rule.isDebugEnabled()) Log.verboseln(F("MLE: Rule %d condition MET."), i);
            bool actionSuccess = performAction(rule);
            if (actionSuccess) {
                // Status updated in performAction
                // Timestamp and counter are also updated in performAction
                if(rule.isReceiptEnabled()) L_INFO(F("MLE: Rule %d action successful."), i);
            } else {
                // Error logged and status updated in performAction
                if(rule.isDebugEnabled()) Log.warningln(F("MLE: Rule %d action FAILED."), i);
            }
        } else {
            // Condition not met
            if(rule.isDebugEnabled()) Log.verboseln(F("MLE: Rule %d condition NOT met."), i);
             // Ensure status is reset to OK/Idle if it wasn't an error
             if (rule.lastStatus != RuleStatus::IllegalDataAddress &&
                 rule.lastStatus != RuleStatus::IllegalDataValue) {
                 updateRuleStatus(rule, RuleStatus::Success);
             }
        }
    } // End for each rule

    return E_OK;
}

// --- Evaluate Condition ---
bool ModbusLogicEngine::evaluateCondition(LogicRule& rule) {
    uint16_t currentValue = 0;
    uint16_t targetValue = rule.getCondValue();
    RegisterState::E_RegType sourceType = rule.getCondSourceType();
    uint16_t sourceAddr = rule.getCondSourceAddr();
    ConditionOperator op = rule.getCondOperator();

    // Read the source value
    bool readSuccess = readConditionSourceValue(sourceType, sourceAddr, currentValue);
    if (!readSuccess) {
        Log.warningln(F("MLE: Failed to read condition source (Type: %d, Addr: %u)"), (int)sourceType, sourceAddr);
        updateRuleStatus(rule, RuleStatus::IllegalDataAddress);
        return false; // Indicate evaluation failure
    }

    if (rule.isDebugEnabled()) {
        Log.verboseln(F("MLE Eval [%d]: SrcType=%d, SrcAddr=%u, Op=%d, Target=%u, Current=%u"), 
                      &rule - &rules[0], // Crude way to get index for logging
                      (int)sourceType, sourceAddr, (int)op, targetValue, currentValue);
    }

    // Perform comparison
    bool result = false;
    switch (op) {
        case ConditionOperator::EQUAL:          result = (currentValue == targetValue); break;
        case ConditionOperator::NOT_EQUAL:      result = (currentValue != targetValue); break;
        case ConditionOperator::LESS_THAN:      result = (currentValue < targetValue); break;
        case ConditionOperator::LESS_EQUAL:     result = (currentValue <= targetValue); break;
        case ConditionOperator::GREATER_THAN:   result = (currentValue > targetValue); break;
        case ConditionOperator::GREATER_EQUAL:  result = (currentValue >= targetValue); break;
        default:
            Log.warningln(F("MLE: Invalid condition operator (%d)"), (int)op);
            updateRuleStatus(rule, RuleStatus::IllegalDataValue);
            return false; // Indicate evaluation failure
    }

    // If we got here, evaluation itself succeeded, return the comparison result
    // Status will be updated later based on whether action is performed
    return result; 
}

// --- Perform Action (Renamed Command) ---
bool ModbusLogicEngine::performAction(LogicRule& rule) { // Renamed function
    CommandType commandType = rule.getCommandType(); // Renamed enum and accessor
    uint16_t target = rule.getCommandTarget();     // Renamed accessor
    uint16_t param1 = rule.getCommandParam1();     // Renamed accessor
    uint16_t param2 = rule.getCommandParam2();     // Renamed accessor
    bool success = false;

    if (rule.isDebugEnabled()) {
        Log.verboseln(F("MLE Action [%d]: CmdType=%d, Target=%u, P1=%u, P2=%u"),
                      &rule - &rules[0], // Crude index for logging
                      (int)commandType, target, param1, param2);
    }

    switch (commandType) {
        case CommandType::NONE:
            success = true; // No action is considered success
            break;
        case CommandType::WRITE_HOLDING_REGISTER:
        case CommandType::WRITE_COIL:
            success = performWriteAction(commandType, target, param1);
            if (!success) updateRuleStatus(rule, RuleStatus::ServerDeviceFailure);
            break;
        case CommandType::CALL_COMPONENT_METHOD:
            success = performCallAction(target, param1, 0);
             if (!success) updateRuleStatus(rule, RuleStatus::OpExecutionFailed);
            break;
        default:
            Log.warningln(F("MLE: Invalid command type (%d)"), (int)commandType);
            updateRuleStatus(rule, RuleStatus::IllegalFunction);
            success = false;
            break;
    }

    // Update timestamp and counter only on successful action execution
    if (success) {
         updateRuleStatus(rule, RuleStatus::Success);
         rule.lastTriggerTimestamp = millis() / 1000; // Store seconds since boot
         rule.triggerCount++;
    }

    return success;
}

// --- Read Condition Source Value ---
bool ModbusLogicEngine::readConditionSourceValue(RegisterState::E_RegType type, uint16_t address, uint16_t& value) {
    if (!app || !app->modbusManager) {
        L_ERROR(F("MLE: ModbusManager not available!"));
        return false;
    }

    // Find the component responsible for this address
    Component* targetComponent = app->modbusManager->findComponentForAddress(address);
    if (!targetComponent) {
        Log.warningln(F("MLE: No component found for read address %u"), address);
        // Optionally: Check if it's a global app address?
        if (app->modbusManager->findComponentForAddress(address) == static_cast<Component*>(app)){
             targetComponent = static_cast<Component*>(app);
        } else {
             return false; 
        }
    }

    short result = -1;
    switch (type) {
        case RegisterState::E_RegType::REG_HOLDING:
        case RegisterState::E_RegType::REG_INPUT:
            result = targetComponent->mb_tcp_read(address);
            break;
        case RegisterState::E_RegType::REG_COIL:
        case RegisterState::E_RegType::REG_DISCRETE_INPUT:
             result = targetComponent->mb_tcp_read(address);
            break;
        default:
            Log.warningln(F("MLE: Unsupported condition source type: %d"), (int)type);
            return false;
    }

    if (result == E_INVALID_PARAMETER || result < -1) { 
        Log.warningln(F("MLE: Read failed for address %u (Result: %d)"), address, result);
        return false;
    }
    
    // Handle potential E_NOT_IMPLEMENTED or other non-value returns if necessary
    if (result == E_NOT_IMPLEMENTED) {
         Log.warningln(F("MLE: Read not implemented for address %u"), address);
         return false;
    }

    value = (uint16_t)result;
    Log.verboseln(F("MLE: Read condition value %u from address %u"), value, address);
    return true;
}

// --- Perform Write Action ---
bool ModbusLogicEngine::performWriteAction(CommandType type, uint16_t address, uint16_t value) {
     if (!app || !app->modbusManager) {
        L_ERROR(F("MLE: ModbusManager not available!"));
        return false;
    }

    // Find the component responsible for this address
    Component* targetComponent = app->modbusManager->findComponentForAddress(address);
     if (!targetComponent) {
        Log.warningln(F("MLE: No component found for write address %u"), address);
        // Optionally: Check if it's a global app address?
         if (app->modbusManager->findComponentForAddress(address) == static_cast<Component*>(app)){
             targetComponent = static_cast<Component*>(app);
        } else {
             return false; 
        }
    }

    Log.verboseln(F("MLE: Writing value %u to address %u (Type: %d)"), value, address, (int)type);

    // Assuming mb_tcp_write handles both Registers and Coils
    // It should return E_OK on success
    short result = targetComponent->mb_tcp_write(address, value);

    if (result != E_OK) {
        Log.warningln(F("MLE: Write failed for address %u, value %u (Result: %d)"), address, value, result);
        return false;
    }

    return true;
}

// --- Perform Call Action ---
bool ModbusLogicEngine::performCallAction(uint16_t componentId, uint16_t methodId, uint16_t arg1) {
    uint32_t combinedId = ((uint32_t)componentId << 16) | methodId;
    auto it = callableMethods.find(combinedId);

    // Find the rule being processed - This requires passing the rule or index down
    int ruleIndex = -1; // Placeholder index
    // Logic to determine the correct ruleIndex based on loop context is needed here.
    // For now, just check if index is valid before accessing rules vector.
    if (ruleIndex < 0 || ruleIndex >= rules.size()) {
         L_ERROR(F("MLE: Invalid rule context in performCallAction!"));
         // Cannot update status without rule context
         return false; 
    }    
    LogicRule& rule = rules[ruleIndex]; // Now safe to access

    if (it == callableMethods.end()) {
        Log.warningln(F("MLE: Method not registered (CompID: %u, MethodID: %u)"), componentId, methodId);
        updateRuleStatus(rule, RuleStatus::IllegalDataAddress);
        return false;
    }

    Log.verboseln(F("MLE: Calling method (CompID: %u, MethodID: %u) with arg (%d)"), componentId, methodId, (short)arg1);
    // Pass only arg1 (param2 from rule) to the registered function, assuming it now takes only one argument
    // OR pass arg1 and a dummy second argument if the registered function still expects two.
    // Let's assume the registered CallableMethod now expects only one argument
    // TODO: Confirm signature of registered CallableMethod
    // short result = it->second((short)arg1, (short)arg2); // Old call
    short result = it->second((short)arg1, 0); // Pass arg1 and a dummy 0 for now

    if (result != E_OK) { // Check against generic E_OK from enums.h
        Log.warningln(F("MLE: Method call failed (CompID: %u, MethodID: %u, Result: %d)"), componentId, methodId, result);
        // Status is updated in performAction based on return
        return false;
    }

    return true;
}

// --- Register Method ---
bool ModbusLogicEngine::registerMethod(uint16_t componentId, uint16_t methodId, CallableMethod method) {
    uint32_t combinedId = ((uint32_t)componentId << 16) | methodId;
    auto result = callableMethods.insert({combinedId, method});

    if (result.second) {
        L_INFO(F("MLE: Registered method (CompID: %u, MethodID: %u)"), componentId, methodId);
    } else {
        Log.warningln(F("MLE: Failed to register duplicate method (CompID: %u, MethodID: %u)"), componentId, methodId);
    }
    return result.second; // Returns true if insertion took place
}

// --- Modbus Network Interface ---

// Helper to get rule index and offset from a Modbus address
bool ModbusLogicEngine::getRuleInfoFromAddress(short address, int& ruleIndex, short& offset) {
    if (address < MODBUS_LOGIC_RULES_START) {
        return false;
    }
    short relativeAddress = address - MODBUS_LOGIC_RULES_START;
    ruleIndex = relativeAddress / LOGIC_ENGINE_REGISTERS_PER_RULE;
    offset = relativeAddress % LOGIC_ENGINE_REGISTERS_PER_RULE;

    if (ruleIndex < 0 || ruleIndex >= MAX_LOGIC_RULES) {
        return false; // Address out of range
    }
    return true;
}

// Read Configuration/Status via Modbus
short ModbusLogicEngine::mb_tcp_read(short address) {
    int ruleIndex = -1;
    short offset = -1;

    if (!getRuleInfoFromAddress(address, ruleIndex, offset)) {
        return E_INVALID_PARAMETER;
    }

    const LogicRule& rule = rules[ruleIndex];

    // Read configuration registers (Offset 0 to 8)
    if (offset >= ModbusLogicEngineOffsets::ENABLED && offset <= ModbusLogicEngineOffsets::COMMAND_PARAM2) {
        return rule.config[offset];
    }
    // Read status/flag registers (Offset 9 to 12)
    else if (offset == ModbusLogicEngineOffsets::FLAGS) {
        return rule.getFlags(); // Use getFlags() to read from config array
    }
    else if (offset == ModbusLogicEngineOffsets::LAST_STATUS) {
        // Return MB_Error as short
        return static_cast<short>(rule.lastStatus);
    }
    else if (offset == ModbusLogicEngineOffsets::LAST_TRIGGER_TS) {
        // Timestamps might be 32-bit, Modbus registers are 16-bit.
        // Return lower 16 bits for simplicity, or implement 32-bit reading.
        return (short)(rule.lastTriggerTimestamp & 0xFFFF); // Return lower 16 bits
        // TODO: Implement reading upper 16 bits at offset + 1 if needed
    }
    else if (offset == ModbusLogicEngineOffsets::TRIGGER_COUNT) {
        return rule.triggerCount;
    }
    else {
        return E_INVALID_PARAMETER;
    }
}

// Write Configuration via Modbus
short ModbusLogicEngine::mb_tcp_write(short address, short value) {
    int ruleIndex = -1;
    short offset = -1;

    if (!getRuleInfoFromAddress(address, ruleIndex, offset)) {
        return E_INVALID_PARAMETER;
    }

    LogicRule& rule = rules[ruleIndex];

    // Allow writing to configuration registers (Offset 0 to 9, including FLAGS)
    if (offset >= ModbusLogicEngineOffsets::ENABLED && offset <= ModbusLogicEngineOffsets::FLAGS) {
        Log.verboseln(F("MLE: Setting Rule %d, Offset %d to %d"), ruleIndex, offset, value);
        rule.setConfigValue(offset, (uint16_t)value);
        // Optional: Persist rule change here if implementing storage
        return E_OK;
    }
    // Allow resetting trigger count (Offset 12)
    else if (offset == ModbusLogicEngineOffsets::TRIGGER_COUNT && value == 0) {
         L_INFO(F("MLE: Resetting trigger count for Rule %d"), ruleIndex);
         rule.triggerCount = 0;
         return E_OK;
    }
    // Disallow writing to other status registers directly (Offsets 10, 11)
    else if (offset == ModbusLogicEngineOffsets::LAST_STATUS || offset == ModbusLogicEngineOffsets::LAST_TRIGGER_TS) {
        Log.warningln(F("MLE: Attempt to write read-only status register (Rule %d, Offset %d)"), ruleIndex, offset);
        return E_INVALID_PARAMETER;
    }
    else {
        return E_INVALID_PARAMETER;
    }
}

// --- Helper to update status (and log changes) ---
void ModbusLogicEngine::updateRuleStatus(LogicRule& rule, RuleStatus newStatus) {
    if (rule.lastStatus != newStatus) {
        Log.verboseln(F("MLE: Rule Status changing from %d to %d"), static_cast<uint8_t>(rule.lastStatus), static_cast<uint8_t>(newStatus));
        rule.lastStatus = newStatus;
        // Optional: Log specific error messages based on the status code
    }
} 

#endif // ENABLE_MB_SCRIPT 
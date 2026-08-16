export enum ConditionOperator {
  EQUAL = 0,
  NOT_EQUAL = 1,
  LESS_THAN = 2,
  LESS_EQUAL = 3,
  GREATER_THAN = 4,
  GREATER_EQUAL = 5,
}

export enum CommandType {
  NONE = 0,
  WRITE_COIL = 2,
  WRITE_HOLDING_REGISTER = 3,
  CALL_COMPONENT_METHOD = 100,
}

// Using a string type for Modbus errors for more descriptive errors in TS
export type RuleStatus = string; 

export const RuleStatusNoError: RuleStatus = "Success";
// Add other MB_Error equivalents as needed, e.g.
// export const RuleStatusIllegalFunction: RuleStatus = "IllegalFunction"; 


export enum RegisterType {
  HOLDING_REGISTER = 0,
  COIL = 1,
  // Add INPUT_REGISTER and DISCRETE_INPUT if needed from RegisterState::E_RegType
}

export interface LogicRuleConfig {
  enabled: boolean;
  conditionSourceType: RegisterType;
  conditionSourceAddress: number;
  conditionOperator: ConditionOperator;
  conditionValue: number;
  commandType: CommandType;
  commandTarget: number;
  commandParam1: number;
  commandParam2: number;
  // Param3 removed as per C++
  flags: number;

  // ELSE action (optional)
  elseCommandType?: CommandType;
  elseCommandTarget?: number;
  elseCommandParam1?: number;
  elseCommandParam2?: number;
}

export interface LogicRuleStatus {
  lastStatus: RuleStatus;
  lastTriggerTimestamp: number; // seconds since boot
  triggerCount: number;
  lastEvalLogTimestamp?: number; // Added for throttling eval log
}

export interface LogicRule extends LogicRuleConfig, LogicRuleStatus {
  id: number; // Rule index
}

// --- Rule Flags (Bitmasks for FLAGS register) ---
export const RULE_FLAG_DEBUG = 1 << 0; // Enable verbose debug logging for this rule
export const RULE_FLAG_RECEIPT = 1 << 1; // Enable logging upon successful trigger/action

// Type for callable methods. Matches C++ std::function<short(short, short)>
// In TypeScript, this can be more flexible. For simplicity, using a similar structure.
// arg1 is param1 from rule (MethodID for CALL_COMPONENT_METHOD), arg2 is param2 (Arg1 for method)
export type CallableMethod = (arg1: number, arg2: number) => Promise<number>; // Returns a status code (0 for E_OK)

export interface ModbusRegisterValues {
  holdingRegisters: Map<number, number>;
  coils: Map<number, boolean>;
  // Add inputRegisters and discreteInputs if they become readable by the engine
}

// Configuration for the ModbusLogicEngine
export interface ModbusLogicEngineConfig {
  maxRules: number;
  modbusLogicRulesStartAddr: number;
  loopIntervalMs: number; // Interval for rule evaluation
}

// Default values from C++ and mb-lang.md
export const DEFAULT_MAX_LOGIC_RULES = 8;
export const DEFAULT_LOGIC_ENGINE_REGISTERS_PER_RULE = 17; // Increased by 4 for ELSE action
export const DEFAULT_MODBUS_LOGIC_RULES_START = 0;
export const DEFAULT_LOOP_INTERVAL_MS = 100; // C++ default is 100ms

// Namespaces for Modbus register offsets, similar to C++
export namespace ModbusLogicEngineOffsets {
    export const ENABLED = 0;
    export const COND_SRC_TYPE = 1;
    export const COND_SRC_ADDR = 2;
    export const COND_OPERATOR = 3;
    export const COND_VALUE = 4;
    export const COMMAND_TYPE = 5;
    export const COMMAND_TARGET = 6;
    export const COMMAND_PARAM1 = 7;
    export const COMMAND_PARAM2 = 8;
    // Removed PARAM3
    export const FLAGS = 9;
    export const LAST_STATUS = 10;
    export const LAST_TRIGGER_TS = 11;
    export const TRIGGER_COUNT = 12;
    // New offsets for ELSE action
    export const ELSE_COMMAND_TYPE = 13;
    export const ELSE_COMMAND_TARGET = 14;
    export const ELSE_COMMAND_PARAM1 = 15;
    export const ELSE_COMMAND_PARAM2 = 16;
}

// For Zod validation
import { z } from 'zod';

export const LogicRuleConfigSchema = z.object({
  id: z.number().int().min(0),
  enabled: z.boolean(),
  conditionSourceType: z.nativeEnum(RegisterType),
  conditionSourceAddress: z.number().int().min(0),
  conditionOperator: z.nativeEnum(ConditionOperator),
  conditionValue: z.number().int(),
  commandType: z.nativeEnum(CommandType),
  commandTarget: z.number().int().min(0),
  commandParam1: z.number().int(),
  commandParam2: z.number().int(),
  flags: z.number().int().min(0),
  lastStatus: z.string(), // RuleStatus
  lastTriggerTimestamp: z.number().int().min(0),
  triggerCount: z.number().int().min(0),
  lastEvalLogTimestamp: z.number().int().optional(),
  // Zod validation for ELSE fields
  elseCommandType: z.nativeEnum(CommandType).optional(),
  elseCommandTarget: z.number().int().min(0).optional(),
  elseCommandParam1: z.number().int().optional(),
  elseCommandParam2: z.number().int().optional(),
});

export type LogicRuleZod = z.infer<typeof LogicRuleConfigSchema>; 
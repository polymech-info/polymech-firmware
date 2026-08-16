import { Logger } from "tslog";
import { ZodError } from "zod";
import {
  LogicRule,
  LogicRuleConfig,
  ConditionOperator,
  CommandType,
  RuleStatus,
  RegisterType,
  ModbusRegisterValues,
  CallableMethod,
  LogicRuleConfigSchema,
  ModbusLogicEngineConfig,
  DEFAULT_MAX_LOGIC_RULES,
  DEFAULT_MODBUS_LOGIC_RULES_START,
  DEFAULT_LOOP_INTERVAL_MS,
  ModbusLogicEngineOffsets,
  RULE_FLAG_DEBUG,
  RULE_FLAG_RECEIPT,
  RuleStatusNoError,
  DEFAULT_LOGIC_ENGINE_REGISTERS_PER_RULE
} from "./types.js";
import { ModbusTCPServerWrapper } from "./ModbusTCPServerWrapper.js";

const E_OK = 0; // Assuming E_OK is 0 from enums.h
const E_INVALID_PARAMETER = -2; // Example, match actual error codes if known
const E_ILLEGAL_DATA_ADDRESS = "IllegalDataAddress";
const E_ILLEGAL_DATA_VALUE = "IllegalDataValue";
const E_SERVER_DEVICE_FAILURE = "ServerDeviceFailure";
const E_OP_EXECUTION_FAILED = "OperationExecutionFailed";
const E_ILLEGAL_FUNCTION = "IllegalFunction";

export class ModbusLogicEngine {
  private readonly log: Logger<any>;
  private rules: LogicRule[];
  private callableMethods: Map<number, CallableMethod>; // (ComponentID << 16) | MethodID
  private initialized: boolean = false;
  private lastLoopTime: number = 0;
  private loopIntervalId?: NodeJS.Timeout;
  private modbusServer?: ModbusTCPServerWrapper;

  // Configuration
  private readonly maxRules: number;
  private readonly modbusLogicRulesStartAddr: number;
  private readonly loopIntervalMs: number;

  // Simulated Modbus Data Store (for standalone operation)
  // In a real scenario, this would interact with a Modbus client/server.
  private modbusData: ModbusRegisterValues;

  constructor(
    config?: Partial<ModbusLogicEngineConfig>,
    parentLogger?: Logger<any>,
    modbusServer?: ModbusTCPServerWrapper
  ) {
    this.log = parentLogger ? parentLogger.getSubLogger({ name: "ModbusLogicEngine" }) : new Logger();
    this.log.info("Initializing ModbusLogicEngine...");
    this.modbusServer = modbusServer;

    this.maxRules = config?.maxRules ?? DEFAULT_MAX_LOGIC_RULES;
    this.modbusLogicRulesStartAddr = config?.modbusLogicRulesStartAddr ?? DEFAULT_MODBUS_LOGIC_RULES_START;
    this.loopIntervalMs = config?.loopIntervalMs ?? DEFAULT_LOOP_INTERVAL_MS;

    this.rules = new Array(this.maxRules).fill(null).map((_, i) => this.createDefaultRule(i));
    this.callableMethods = new Map<number, CallableMethod>();

    this.setupDefaultRules();

    // Initialize with empty Modbus data
    this.modbusData = {
      holdingRegisters: new Map<number, number>(),
      coils: new Map<number, boolean>(),
    };

    this.log.info(`Initialized ${this.maxRules} rules. Base Addr: ${this.modbusLogicRulesStartAddr}, Interval: ${this.loopIntervalMs}ms`);
  }

  public setModbusServer(server: ModbusTCPServerWrapper): void {
    this.modbusServer = server;
    this.log.info("ModbusTCPServerWrapper instance has been successfully set in ModbusLogicEngine.");
    if (this.modbusServer && this.modbusServer.getModbusGateway()) {
        this.log.info(`ModbusLogicEngine will now use Holding Registers buffer of length: ${this.modbusServer.getModbusGateway().holding.length} bytes.`);
        this.log.info(`ModbusLogicEngine will now use Coils buffer of length: ${this.modbusServer.getModbusGateway().coils.length} bytes.`);
    }
  }

  private createDefaultRule(id: number): LogicRule {
    return {
      id,
      enabled: false,
      conditionSourceType: RegisterType.HOLDING_REGISTER,
      conditionSourceAddress: 0,
      conditionOperator: ConditionOperator.EQUAL,
      conditionValue: 0,
      commandType: CommandType.NONE,
      commandTarget: 0,
      commandParam1: 0,
      commandParam2: 0,
      flags: 0,
      lastStatus: RuleStatusNoError,
      lastTriggerTimestamp: 0,
      triggerCount: 0,
      lastEvalLogTimestamp: 0,
      elseCommandType: CommandType.NONE,
      elseCommandTarget: 0,
      elseCommandParam1: 0,
      elseCommandParam2: 0,
    };
  }

  public setup(): Promise<void> {
    this.log.info("ModbusLogicEngine: Setting up...");
    // In a real application, this might load rules from persistent storage.
    this.initialized = true;
    this.log.info("ModbusLogicEngine: Setup complete.");
    return Promise.resolve();
  }

  public start(): void {
    if (!this.initialized) {
      this.log.warn("Engine not initialized. Call setup() first.");
      return;
    }
    if (this.loopIntervalId) {
        this.log.warn("Engine loop already started.");
        return;
    }
    this.log.info(`Starting ModbusLogicEngine loop with interval: ${this.loopIntervalMs}ms`);
    this.loopIntervalId = setInterval(() => this.loop(), this.loopIntervalMs);
  }

  public stop(): void {
    if (this.loopIntervalId) {
      this.log.info("Stopping ModbusLogicEngine loop.");
      clearInterval(this.loopIntervalId);
      this.loopIntervalId = undefined;
    }
  }

  private async loop(): Promise<void> {
    if (!this.initialized) {
      return; // Not ready
    }

    const currentTime = Date.now();
    // this.log.debug("ModbusLogicEngine: Evaluating rules...");

    for (let i = 0; i < this.rules.length; ++i) {
      const rule = this.rules[i];

      if (!rule.enabled) {
        continue; // Skip disabled rules
      }

      const isDebugEnabled = (rule.flags & RULE_FLAG_DEBUG) !== 0;
      const isReceiptEnabled = (rule.flags & RULE_FLAG_RECEIPT) !== 0;

      let conditionMet = false;
      let conditionEvalSuccess = false;
      try {
        const evalResult = await this.evaluateCondition(rule);
        conditionMet = evalResult.met;
        conditionEvalSuccess = evalResult.success;

        if (!conditionEvalSuccess) {
          if (isDebugEnabled) this.log.debug(`MLE: Rule ${i} condition eval FAILED internally.`);
          // Status already updated in evaluateCondition if specific error
          continue; // Move to the next rule
        }
      } catch (error) {
        this.log.error(`MLE: Rule ${i} condition evaluation error:`, error);
        this.updateRuleStatus(rule, E_ILLEGAL_DATA_ADDRESS); // Or a more generic error
        continue;
      }
      
      if (conditionMet) {
        if (isDebugEnabled) this.log.debug(`MLE: Rule ${i} (IF) condition MET.`);
        // Perform THEN action if defined
        if (rule.commandType !== CommandType.NONE) {
          try {
            const actionSuccess = await this.performAction(rule, rule.commandType, rule.commandTarget, rule.commandParam1, rule.commandParam2);
            if (actionSuccess) {
              rule.lastTriggerTimestamp = Math.floor(Date.now() / 1000);
              rule.triggerCount++;
              this.updateRuleStatus(rule, RuleStatusNoError);
              if (isReceiptEnabled) this.log.info(`MLE: Rule ${i} (THEN) action successful. Count: ${rule.triggerCount}`);
            } else {
              if (isDebugEnabled) this.log.warn(`MLE: Rule ${i} (THEN) action FAILED.`);
              // Status is updated in performAction for specific failures
            }
          } catch (error) {
            this.log.error(`MLE: Rule ${i} (THEN) action execution error:`, error);
            this.updateRuleStatus(rule, E_OP_EXECUTION_FAILED);
          }
        } else { // THEN action is CommandType.NONE
            if (isDebugEnabled) this.log.debug(`MLE: Rule ${i} (THEN) action is NONE, skipping.`);
            if (conditionEvalSuccess) { // Condition eval was fine, but no THEN action.
                this.updateRuleStatus(rule, RuleStatusNoError);
            }
        }
      } else { // Condition NOT met (IF was false)
        if (isDebugEnabled) this.log.debug(`MLE: Rule ${i} (IF) condition NOT MET.`);
        // Perform ELSE action if defined
        if (rule.elseCommandType !== undefined && rule.elseCommandType !== CommandType.NONE) {
          if (isDebugEnabled) this.log.debug(`MLE: Rule ${i} executing ELSE action (Type: ${CommandType[rule.elseCommandType]}, Target: ${rule.elseCommandTarget ?? 0}).`);
          try {
            const elseActionSuccess = await this.performAction(rule, rule.elseCommandType, rule.elseCommandTarget ?? 0, rule.elseCommandParam1 ?? 0, rule.elseCommandParam2 ?? 0);
            if (elseActionSuccess) {
              rule.lastTriggerTimestamp = Math.floor(Date.now() / 1000);
              rule.triggerCount++;
              this.updateRuleStatus(rule, RuleStatusNoError);
              if (isReceiptEnabled) this.log.info(`MLE: Rule ${i} (ELSE) action successful. Count: ${rule.triggerCount}`);
            } else {
              if (isDebugEnabled) this.log.warn(`MLE: Rule ${i} (ELSE) action FAILED.`);
              // Status is updated in performAction for specific failures
            }
          } catch (error) {
            this.log.error(`MLE: Rule ${i} (ELSE) action execution error:`, error);
            this.updateRuleStatus(rule, E_OP_EXECUTION_FAILED);
          }
        } else { // No ELSE action defined (elseCommandType is undefined or CommandType.NONE)
          if (isDebugEnabled) this.log.debug(`MLE: Rule ${i} (ELSE) action is NONE or undefined, skipping.`);
          if (conditionEvalSuccess) { // Condition eval was fine, but no ELSE action.
            this.updateRuleStatus(rule, RuleStatusNoError);
          }
        }
      }
    }
    this.lastLoopTime = currentTime;
  }

  private setupDefaultRules(): void {
    if (this.maxRules > 0 && this.rules.length > 0) {
        const rule0 = this.rules[0];
        rule0.enabled = true;
        rule0.conditionSourceType = RegisterType.HOLDING_REGISTER;
        rule0.conditionSourceAddress = 5;
        rule0.conditionOperator = ConditionOperator.GREATER_THAN;
        rule0.conditionValue = 10;
        rule0.commandType = CommandType.WRITE_HOLDING_REGISTER;
        rule0.commandTarget = 8; // Target address for the write action
        rule0.commandParam1 = 20; // Value to write
        rule0.commandParam2 = 0;  // Not used for this command type
        // rule0.flags = RULE_FLAG_DEBUG; // Optionally enable debug for this rule

        // Add ELSE action
        rule0.elseCommandType = CommandType.WRITE_HOLDING_REGISTER;
        rule0.elseCommandTarget = 8;    // Target address for the ELSE write action (same register)
        rule0.elseCommandParam1 = 10;   // Value to write for ELSE
        rule0.elseCommandParam2 = 0;    // Not used for this command type

        this.log.info(`Configured default rule 0: IF HR[5] > 10 THEN HR[8] = 20 ELSE HR[8] = 10`);
    } else {
        this.log.info("No rules available or maxRules is 0, skipping default rule setup.");
    }
  }

  private updateRuleStatus(rule: LogicRule, newStatus: RuleStatus): void {
    if (rule.lastStatus !== newStatus) {
      const isDebugEnabled = (rule.flags & RULE_FLAG_DEBUG) !== 0;
      if (isDebugEnabled || newStatus !== RuleStatusNoError) { // Log changes or any error
         this.log.debug(`MLE Rule ${rule.id}: Status changing from '${rule.lastStatus}' to '${newStatus}'`);
      }
      rule.lastStatus = newStatus;
    }
  }

  private async readConditionSourceValue(
    type: RegisterType,
    address: number
  ): Promise<{ value: number; success: boolean }> {
    let val: number | undefined;
    let success = false;

    if (!this.modbusServer) {
        this.log.warn("MLE readConditionSourceValue: ModbusTCPServerWrapper instance not available. Using internal simulated data store.");
        // Fallback to internal simulated data if server not provided
        switch (type) {
            case RegisterType.HOLDING_REGISTER:
                val = this.modbusData.holdingRegisters.get(address);
                break;
            case RegisterType.COIL:
                const coilVal = this.modbusData.coils.get(address);
                if (coilVal !== undefined) {
                val = coilVal ? 1 : 0; // Convert boolean to number for comparison
                }
                break;
            default:
                this.log.warn(`Unsupported condition source type: ${type}`);
                return { value: 0, success: false };
        }
        success = val !== undefined;
    } else {
        const gateway = this.modbusServer.getModbusGateway();
        switch (type) {
            case RegisterType.HOLDING_REGISTER:
                const requiredBytes = address * 2 + 2;
                if (requiredBytes <= gateway.holding.length) { // Check bounds for 16-bit read
                    try {
                        val = gateway.holding.readUInt16BE(address * 2);
                        success = true;
                    } catch (e: any) {
                        this.log.error(`MLE ReadHR: Addr=${address}, Error reading UInt16BE: ${e.message}`);
                        success = false; // Explicitly false on error
                    }
                } else {
                    this.log.warn(`MLE ReadHR: Addr=${address} is out of bounds for server holding registers (length: ${gateway.holding.length}, needed: ${requiredBytes}).`);
                    success = false; // Explicitly false if out of bounds
                }
                break;
            case RegisterType.COIL:
                const byteIndex = Math.floor(address / 8);
                const bitInByte = address % 8;
                if (byteIndex < gateway.coils.length) {
                    const byteValue = gateway.coils.readUInt8(byteIndex);
                    val = (byteValue >> bitInByte) & 1;
                    success = true;
                } else {
                    this.log.warn(`MLE: Address ${address} (byte ${byteIndex}) out of bounds for server coils.`);
                }
                break;
            default:
                this.log.warn(`Unsupported condition source type: ${type} when reading from server.`);
                break;
        }
    }

    if (!success || val === undefined) {
      this.log.warn(`MLE: Failed to read condition source (Type: ${type}, Addr: ${address}). Value not found or access failed.`);
      return { value: 0, success: false };
    }
    
    return { value: val, success: true };
  }

  private async evaluateCondition(rule: LogicRule): Promise<{ met: boolean; success: boolean }> {
    const { conditionSourceType, conditionSourceAddress, conditionOperator, conditionValue } = rule;
    const isDebugEnabled = (rule.flags & RULE_FLAG_DEBUG) !== 0;

    const readResult = await this.readConditionSourceValue(conditionSourceType, conditionSourceAddress);

    if (!readResult.success) {
      this.log.warn(`MLE Rule ${rule.id}: Failed to read condition source (Type: ${conditionSourceType}, Addr: ${conditionSourceAddress})`);
      this.updateRuleStatus(rule, E_ILLEGAL_DATA_ADDRESS);
      return { met: false, success: false };
    }

    const currentValue = readResult.value;

    if (isDebugEnabled) {
      const now = Date.now();
      if (!rule.lastEvalLogTimestamp || (now - rule.lastEvalLogTimestamp > 2500)) {
        this.log.debug(
          `MLE Eval Rule ${rule.id}: SrcType=${conditionSourceType}, SrcAddr=${conditionSourceAddress}, Op=${conditionOperator}, Target=${conditionValue}, Current=${currentValue}`
        );
        rule.lastEvalLogTimestamp = now;
      }
    }

    let result = false;
    switch (conditionOperator) {
      case ConditionOperator.EQUAL:          result = currentValue === conditionValue; break;
      case ConditionOperator.NOT_EQUAL:      result = currentValue !== conditionValue; break;
      case ConditionOperator.LESS_THAN:      result = currentValue < conditionValue;  break;
      case ConditionOperator.LESS_EQUAL:     result = currentValue <= conditionValue; break;
      case ConditionOperator.GREATER_THAN:   result = currentValue > conditionValue;  break;
      case ConditionOperator.GREATER_EQUAL:  result = currentValue >= conditionValue; break;
      default:
        this.log.warn(`MLE Rule ${rule.id}: Invalid condition operator (${conditionOperator})`);
        this.updateRuleStatus(rule, E_ILLEGAL_DATA_VALUE);
        return { met: false, success: false }; // Indicate evaluation failure
    }
    // If we got here, evaluation itself (not necessarily the condition result) succeeded.
    // Status will be updated by the loop based on whether action is performed.
    return { met: result, success: true };
  }

  private async performWriteAction(
    ruleId: number, // for logging
    type: CommandType,
    address: number,
    value: number
  ): Promise<boolean> {
    if (!this.modbusServer) {
        this.log.warn(`MLE Rule ${ruleId} performWriteAction: ModbusTCPServerWrapper instance not available. Using internal simulated data store.`);
        // Fallback to internal simulated data if server not provided
        try {
            if (type === CommandType.WRITE_HOLDING_REGISTER) {
                this.modbusData.holdingRegisters.set(address, value);
            } else if (type === CommandType.WRITE_COIL) {
                this.modbusData.coils.set(address, value !== 0); // Standard practice: 0 is OFF, non-zero is ON
            } else {
                this.log.warn(`MLE Rule ${ruleId}: performWriteAction called with invalid type ${type}`);
                return false;
            }
            return true;
        } catch (error) {
            this.log.error(`MLE Rule ${ruleId}: Write failed for address ${address}, value ${value}. Error:`, error);
            return false;
        }
    }

    const gateway = this.modbusServer.getModbusGateway();
    try {
      if (type === CommandType.WRITE_HOLDING_REGISTER) {
        if (address * 2 + 2 <= gateway.holding.length) { // Check bounds for 16-bit write
            gateway.holding.writeUInt16BE(value, address * 2);
            return true;
        } else {
            this.log.warn(`MLE Rule ${ruleId}: Address ${address} out of bounds for server holding registers.`);
            return false;
        }
      } else if (type === CommandType.WRITE_COIL) {
        const byteIndex = Math.floor(address / 8);
        const bitInByte = address % 8;
        if (byteIndex < gateway.coils.length) {
            let byteValue = gateway.coils.readUInt8(byteIndex);
            if (value !== 0) { // Set bit
                byteValue |= (1 << bitInByte);
            } else { // Clear bit
                byteValue &= ~(1 << bitInByte);
            }
            gateway.coils.writeUInt8(byteValue, byteIndex);
            return true;
        } else {
            this.log.warn(`MLE Rule ${ruleId}: Address ${address} (byte ${byteIndex}) out of bounds for server coils.`);
            return false;
        }
      } else {
        this.log.warn(`MLE Rule ${ruleId}: performWriteAction with server called with invalid type ${type}`);
        return false;
      }
    } catch (error) {
        this.log.error(`MLE Rule ${ruleId}: Server write failed for address ${address}, value ${value}. Error:`, error);
        return false;
    }
  }

  private async performCallAction(
    ruleId: number, // for logging
    componentId: number,
    methodId: number,
    arg1: number,
    // arg2: number // param2 from rule is arg1 for the method, C++ has param3 as arg2 for method, but it was removed.
                // The registered CallableMethod in TS is defined as (arg1, arg2) => Promise<number>
                // Here, commandParam1 is methodId, commandParam2 is arg1 for the method.
                // Let's assume the TS CallableMethod now takes only one user-defined arg (arg1 from rule.commandParam2)
                // and the second arg in its signature can be a dummy or an internal context if needed.
                // For now, we pass 0 as the second argument to match the C++ stub.
  ): Promise<boolean> {
    const combinedId = (componentId << 16) | methodId;
    const method = this.callableMethods.get(combinedId);

    if (!method) {
      this.log.warn(`MLE Rule ${ruleId}: Method not registered (CompID: ${componentId}, MethodID: ${methodId})`);
      return false;
    }

    this.log.debug(`MLE Rule ${ruleId}: Calling method (CompID: ${componentId}, MethodID: ${methodId}) with arg1: ${arg1}`);
    try {
      // Pass arg1 (from rule.commandParam2) as the first argument to the callable method.
      // Pass 0 as the second argument for now, as per the C++ stub for the simplified call.
      const result = await method(arg1, 0); 
      if (result !== E_OK) { // Check against generic E_OK
        this.log.warn(
          `MLE Rule ${ruleId}: Method call failed (CompID: ${componentId}, MethodID: ${methodId}, Result: ${result})`
        );
        return false;
      }
      this.log.debug(`MLE Rule ${ruleId}: Method call successful (CompID: ${componentId}, MethodID: ${methodId})`);
      return true;
    } catch (error) {
      this.log.error(
        `MLE Rule ${ruleId}: Method call exception (CompID: ${componentId}, MethodID: ${methodId}). Error:`, error
      );
      return false;
    }
  }

  private async performAction(
    rule: LogicRule, 
    commandType: CommandType, 
    commandTarget: number, 
    commandParam1: number, 
    commandParam2: number
  ): Promise<boolean> {
    const { id: ruleId, flags } = rule;
    const isDebugEnabled = (flags & RULE_FLAG_DEBUG) !== 0;
    let success = false;

    if (isDebugEnabled) {
      this.log.debug(
        `MLE Action Rule ${ruleId}: CmdType=${CommandType[commandType]}(${commandType}), Target=${commandTarget}, P1=${commandParam1}, P2=${commandParam2}`
      );
    }

    if (commandType === CommandType.NONE) {
        if (isDebugEnabled) this.log.debug(`MLE Action Rule ${ruleId}: commandType is NONE, considered successful no-op.`);
        if (rule.lastStatus === RuleStatusNoError || rule.lastStatus === (E_OK as unknown as string)) { 
             this.updateRuleStatus(rule, RuleStatusNoError);
        }
        return true; 
    }

    switch (commandType) {
        case CommandType.WRITE_HOLDING_REGISTER:
        case CommandType.WRITE_COIL:
            success = await this.performWriteAction(ruleId, commandType, commandTarget, commandParam1);
            if (!success) this.updateRuleStatus(rule, E_SERVER_DEVICE_FAILURE);
            break;
        case CommandType.CALL_COMPONENT_METHOD:
            success = await this.performCallAction(ruleId, commandTarget, commandParam1, commandParam2);
            if (!success) this.updateRuleStatus(rule, E_OP_EXECUTION_FAILED);
            break;
        default:
            this.log.warn(`MLE Rule ${ruleId}: Invalid command type (${commandType}) in performAction.`);
            this.updateRuleStatus(rule, E_ILLEGAL_FUNCTION);
            success = false;
            break;
    }
    return success;
  }

  public registerMethod(componentId: number, methodId: number, method: CallableMethod): boolean {
    const combinedId = (componentId << 16) | methodId;
    if (this.callableMethods.has(combinedId)) {
      this.log.warn(`Method already registered (CompID: ${componentId}, MethodID: ${methodId})`);
      return false;
    }
    this.callableMethods.set(combinedId, method);
    this.log.info(`Registered method (CompID: ${componentId}, MethodID: ${methodId})`);
    return true;
  }

  public async setModbusHoldingRegister(address: number, value: number): Promise<void> {
    if (this.modbusServer) {
        const gateway = this.modbusServer.getModbusGateway();
        if (address * 2 + 2 <= gateway.holding.length) {
            gateway.holding.writeUInt16BE(value, address * 2);
            this.log.debug(`Server Modbus: Set Holding Register ${address} = ${value}`);
        } else {
            this.log.warn(`Server Modbus: Address ${address} out of bounds for holding registers during set.`);
        }
        return;
    }
    this.modbusData.holdingRegisters.set(address, value);
    this.log.debug(`Simulated Modbus: Set Holding Register ${address} = ${value}`);
  }

  public async getModbusHoldingRegister(address: number): Promise<number | undefined> {
    if (this.modbusServer) {
        const gateway = this.modbusServer.getModbusGateway();
        if (address * 2 + 2 <= gateway.holding.length) {
            return gateway.holding.readUInt16BE(address * 2);
        }
        this.log.warn(`Server Modbus: Address ${address} out of bounds for holding registers during get.`);
        return undefined;
    }
    return this.modbusData.holdingRegisters.get(address);
  }

  public async setModbusCoil(address: number, value: boolean): Promise<void> {
    if (this.modbusServer) {
        const gateway = this.modbusServer.getModbusGateway();
        const byteIndex = Math.floor(address / 8);
        const bitInByte = address % 8;
        if (byteIndex < gateway.coils.length) {
            let byteValue = gateway.coils.readUInt8(byteIndex);
            if (value) {
                byteValue |= (1 << bitInByte);
            } else {
                byteValue &= ~(1 << bitInByte);
            }
            gateway.coils.writeUInt8(byteValue, byteIndex);
            this.log.debug(`Server Modbus: Set Coil ${address} = ${value}`);
        } else {
            this.log.warn(`Server Modbus: Address ${address} (byte ${byteIndex}) out of bounds for coils during set.`);
        }
        return;
    }
    this.modbusData.coils.set(address, value);
    this.log.debug(`Simulated Modbus: Set Coil ${address} = ${value}`);
  }

  public async getModbusCoil(address: number): Promise<boolean | undefined> {
    if (this.modbusServer) {
        const gateway = this.modbusServer.getModbusGateway();
        const byteIndex = Math.floor(address / 8);
        const bitInByte = address % 8;
        if (byteIndex < gateway.coils.length) {
            const byteValue = gateway.coils.readUInt8(byteIndex);
            return ((byteValue >> bitInByte) & 1) === 1;
        }
        this.log.warn(`Server Modbus: Address ${address} (byte ${byteIndex}) out of bounds for coils during get.`);
        return undefined;
    }
    return this.modbusData.coils.get(address);
  }

  public getRule(ruleIndex: number): Readonly<LogicRule> | undefined {
    if (ruleIndex < 0 || ruleIndex >= this.rules.length) {
      this.log.warn(`Attempted to get invalid rule index: ${ruleIndex}`);
      return undefined;
    }
    return { ...this.rules[ruleIndex] };
  }

  private getRuleInfoFromAddress(address: number): { ruleIndex: number; offset: number; valid: boolean } {
    if (address < this.modbusLogicRulesStartAddr) {
      return { ruleIndex: -1, offset: -1, valid: false };
    }
    const relativeAddress = address - this.modbusLogicRulesStartAddr;
    const ruleIndex = Math.floor(relativeAddress / DEFAULT_LOGIC_ENGINE_REGISTERS_PER_RULE);
    const offset = relativeAddress % DEFAULT_LOGIC_ENGINE_REGISTERS_PER_RULE;

    if (ruleIndex < 0 || ruleIndex >= this.maxRules) {
      return { ruleIndex: -1, offset: -1, valid: false };
    }
    return { ruleIndex, offset, valid: true };
  }

  // Simulates mb_tcp_read from C++
  public async mb_read(address: number): Promise<{ value: number; status: RuleStatus | number }> {
    const ruleInfo = this.getRuleInfoFromAddress(address);
    if (!ruleInfo.valid) {
      return { value: 0, status: E_INVALID_PARAMETER }; // Or specific Modbus error code for invalid address
    }

    const rule = this.rules[ruleInfo.ruleIndex];
    const offset = ruleInfo.offset;

    // Directly access LogicRule properties based on offset mapping
    // This part needs to be carefully mapped from mb-lang.md and C++ offsets
    switch (offset) {
      case ModbusLogicEngineOffsets.ENABLED:
        return { value: rule.enabled ? 1 : 0, status: E_OK };
      case ModbusLogicEngineOffsets.COND_SRC_TYPE:
        return { value: rule.conditionSourceType, status: E_OK };
      case ModbusLogicEngineOffsets.COND_SRC_ADDR:
        return { value: rule.conditionSourceAddress, status: E_OK };
      case ModbusLogicEngineOffsets.COND_OPERATOR:
        return { value: rule.conditionOperator, status: E_OK };
      case ModbusLogicEngineOffsets.COND_VALUE:
        return { value: rule.conditionValue, status: E_OK };
      case ModbusLogicEngineOffsets.COMMAND_TYPE:
        return { value: rule.commandType, status: E_OK };
      case ModbusLogicEngineOffsets.COMMAND_TARGET:
        return { value: rule.commandTarget, status: E_OK };
      case ModbusLogicEngineOffsets.COMMAND_PARAM1:
        return { value: rule.commandParam1, status: E_OK };
      case ModbusLogicEngineOffsets.COMMAND_PARAM2:
        return { value: rule.commandParam2, status: E_OK };
      case ModbusLogicEngineOffsets.FLAGS:
        return { value: rule.flags, status: E_OK };
      case ModbusLogicEngineOffsets.LAST_STATUS:
        // In C++, this is an MB_Error enum. Here we use string or a number code.
        // For simplicity, returning a numeric representation if possible or a fixed code.
        // This might need a mapping if status strings are used internally.
        // For now, let's assume we need to map it back to a number for Modbus.
        // This is a placeholder; a robust solution maps RuleStatus strings to specific numbers.
        // For now, let's assume we need to map it back to a number for Modbus.
        // This is a placeholder; a robust solution maps RuleStatus strings to specific numbers.
        return { value: rule.lastStatus === RuleStatusNoError ? 0 : 1, status: E_OK }; // Example mapping
      case ModbusLogicEngineOffsets.LAST_TRIGGER_TS:
        // Timestamps are 32-bit. Modbus registers are 16-bit.
        // C++ returns lower 16 bits. We can do the same or handle 32-bit reads if client supports it.
        return { value: rule.lastTriggerTimestamp & 0xFFFF, status: E_OK }; // Lower 16 bits
      // TODO: Add reading upper 16 bits at offset + 1 if implementing 32-bit Modbus reads
      case ModbusLogicEngineOffsets.TRIGGER_COUNT:
        return { value: rule.triggerCount, status: E_OK };
      // Add cases for ELSE action parameters
      case ModbusLogicEngineOffsets.ELSE_COMMAND_TYPE:
        return { value: rule.elseCommandType ?? CommandType.NONE, status: E_OK };
      case ModbusLogicEngineOffsets.ELSE_COMMAND_TARGET:
        return { value: rule.elseCommandTarget ?? 0, status: E_OK };
      case ModbusLogicEngineOffsets.ELSE_COMMAND_PARAM1:
        return { value: rule.elseCommandParam1 ?? 0, status: E_OK };
      case ModbusLogicEngineOffsets.ELSE_COMMAND_PARAM2:
        return { value: rule.elseCommandParam2 ?? 0, status: E_OK };
      default:
        this.log.warn(`MLE mb_read: Invalid offset ${offset} for rule ${ruleInfo.ruleIndex}`);
        return { value: 0, status: E_INVALID_PARAMETER }; // Or specific Modbus error code
    }
  }

  // Simulates mb_tcp_write from C++
  public async mb_write(address: number, value: number): Promise<RuleStatus | number> {
    const ruleInfo = this.getRuleInfoFromAddress(address);
    if (!ruleInfo.valid) {
      this.log.warn(`MLE mb_write: Invalid address ${address}`);
      return E_INVALID_PARAMETER; // Or specific Modbus error code
    }

    const rule = this.rules[ruleInfo.ruleIndex];
    const offset = ruleInfo.offset;

    this.log.debug(`MLE: Attempting to write Rule ${ruleInfo.ruleIndex}, Offset ${offset} (Modbus Addr ${address}) to Value ${value}`); // Enhanced initial log

    // Update LogicRule properties based on offset mapping
    try {
      let configChanged = false;
      switch (offset) {
        case ModbusLogicEngineOffsets.ENABLED:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - ENABLED changing from ${rule.enabled} to ${value === 1}`);
          rule.enabled = value === 1;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.COND_SRC_TYPE:
          if (Object.values(RegisterType).includes(value)) {
            this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - COND_SRC_TYPE changing from ${RegisterType[rule.conditionSourceType]} to ${RegisterType[value as RegisterType]}`);
            rule.conditionSourceType = value as RegisterType;
            configChanged = true;
          } else {
            this.log.warn(`MLE mb_write: Invalid CondSrcType value ${value} for rule ${ruleInfo.ruleIndex}`);
            return E_ILLEGAL_DATA_VALUE;
          }
          break;
        case ModbusLogicEngineOffsets.COND_SRC_ADDR:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - COND_SRC_ADDR changing from ${rule.conditionSourceAddress} to ${value}`);
          rule.conditionSourceAddress = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.COND_OPERATOR:
           if (Object.values(ConditionOperator).includes(value)) {
            this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - COND_OPERATOR changing from ${ConditionOperator[rule.conditionOperator]} to ${ConditionOperator[value as ConditionOperator]}`);
            rule.conditionOperator = value as ConditionOperator;
            configChanged = true;
          } else {
            this.log.warn(`MLE mb_write: Invalid CondOperator value ${value} for rule ${ruleInfo.ruleIndex}`);
            return E_ILLEGAL_DATA_VALUE;
          }
          break;
        case ModbusLogicEngineOffsets.COND_VALUE:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - COND_VALUE changing from ${rule.conditionValue} to ${value}`);
          rule.conditionValue = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.COMMAND_TYPE:
          if (Object.values(CommandType).includes(value)) {
            this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - COMMAND_TYPE changing from ${CommandType[rule.commandType]} to ${CommandType[value as CommandType]}`);
            rule.commandType = value as CommandType;
            configChanged = true;
          } else {
            this.log.warn(`MLE mb_write: Invalid CommandType value ${value} for rule ${ruleInfo.ruleIndex}`);
            return E_ILLEGAL_DATA_VALUE;
          }
          break;
        case ModbusLogicEngineOffsets.COMMAND_TARGET:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - COMMAND_TARGET changing from ${rule.commandTarget} to ${value}`);
          rule.commandTarget = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.COMMAND_PARAM1:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - COMMAND_PARAM1 changing from ${rule.commandParam1} to ${value}`);
          rule.commandParam1 = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.COMMAND_PARAM2:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - COMMAND_PARAM2 changing from ${rule.commandParam2} to ${value}`);
          rule.commandParam2 = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.FLAGS:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - FLAGS changing from ${rule.flags} to ${value}`);
          rule.flags = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.LAST_STATUS:
        case ModbusLogicEngineOffsets.LAST_TRIGGER_TS:
          this.log.warn(
            `MLE: Attempt to write to read-only status/timestamp register (Rule ${ruleInfo.ruleIndex}, Offset ${offset})`
          );
          return E_ILLEGAL_FUNCTION;
        case ModbusLogicEngineOffsets.TRIGGER_COUNT:
          if (value === 0) {
            this.log.warn(`MLE mb_write: Invalid TRIGGER_COUNT value ${value} for rule ${ruleInfo.ruleIndex}`);
            return E_ILLEGAL_DATA_VALUE;
          }
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - TRIGGER_COUNT changing from ${rule.triggerCount} to ${value}`);
          rule.triggerCount = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.ELSE_COMMAND_TYPE:
          if (Object.values(CommandType).includes(value)) {
            this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - ELSE_COMMAND_TYPE changing from ${CommandType[rule.elseCommandType ?? CommandType.NONE]} to ${CommandType[value as CommandType]}`);
            rule.elseCommandType = value as CommandType;
            configChanged = true;
          } else {
            this.log.warn(`MLE mb_write: Invalid ElseCommandType value ${value} for rule ${ruleInfo.ruleIndex}`);
            return E_ILLEGAL_DATA_VALUE;
          }
          break;
        case ModbusLogicEngineOffsets.ELSE_COMMAND_TARGET:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - ELSE_COMMAND_TARGET changing from ${rule.elseCommandTarget ?? 0} to ${value}`);
          rule.elseCommandTarget = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.ELSE_COMMAND_PARAM1:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - ELSE_COMMAND_PARAM1 changing from ${rule.elseCommandParam1 ?? 0} to ${value}`);
          rule.elseCommandParam1 = value;
          configChanged = true;
          break;
        case ModbusLogicEngineOffsets.ELSE_COMMAND_PARAM2:
          this.log.debug(`MLE Write: Rule ${ruleInfo.ruleIndex} - ELSE_COMMAND_PARAM2 changing from ${rule.elseCommandParam2 ?? 0} to ${value}`);
          rule.elseCommandParam2 = value;
          configChanged = true;
          break;
        default:
          this.log.warn(`MLE mb_write: Invalid offset ${offset} for rule ${ruleInfo.ruleIndex}`);
          return E_INVALID_PARAMETER; // Or specific Modbus error code
      }
      if (configChanged) {
        this.log.info(`MLE: Rule ${ruleInfo.ruleIndex}: Configuration changed.`);
      }
      return E_OK;
    } catch (error) {
      this.log.error(`MLE mb_write: Error writing to rule ${ruleInfo.ruleIndex}, offset ${ruleInfo.offset}:`, error);
      return E_SERVER_DEVICE_FAILURE; // Or specific Modbus error code
    }
  }
}
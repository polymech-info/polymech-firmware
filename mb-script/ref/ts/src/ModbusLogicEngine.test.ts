import { describe, it, expect, beforeEach, vi } from 'vitest';
import { ModbusLogicEngine } from './ModbusLogicEngine.js';
import {
  CommandType,
  ConditionOperator,
  RegisterType,
  ModbusLogicEngineOffsets,
  RuleStatusNoError,
  DEFAULT_MAX_LOGIC_RULES
} from './types.js';
import { Logger } from 'tslog';

// Suppress logger output during tests
vi.mock('tslog', async () => {
  const ActualTsLog = await vi.importActual<typeof import('tslog')>('tslog');
  class MockLogger extends ActualTsLog.Logger<any> {
    constructor(settings: any, logObj: any) {
      super({ ...settings, minLevel: 6 }, logObj); // 6 corresponds to "fatal", effectively silencing it
    }
    getSubLogger() {
        // Return a new instance of the mock to ensure sub-loggers are also silenced
        return new MockLogger({minLevel: 6}, {});
    }
  }
  return { Logger: MockLogger };
});

describe('ModbusLogicEngine', () => {
  let engine: ModbusLogicEngine;

  beforeEach(() => {
    // Re-initialize engine before each test to ensure isolation
    engine = new ModbusLogicEngine({ maxRules: DEFAULT_MAX_LOGIC_RULES });
    engine.setup(); // Initialize the engine
  });

  it('should initialize with default rules', () => {
    for (let i = 0; i < DEFAULT_MAX_LOGIC_RULES; i++) {
      const rule = engine.getRule(i);
      expect(rule).toBeDefined();
      expect(rule?.id).toBe(i);
      expect(rule?.enabled).toBe(false);
      expect(rule?.lastStatus).toBe(RuleStatusNoError);
    }
  });

  it('should allow writing and reading a rule configuration via Modbus methods', async () => {
    const ruleId = 0;
    const ruleBaseAddr = DEFAULT_MAX_LOGIC_RULES > 0 ? 1000 : 0; // Assuming start addr 1000 for this test

    // Enable rule
    let status = await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.ENABLED, 1);
    expect(status).toBe(0); // E_OK
    let readResult = await engine.mb_read(ruleBaseAddr + ModbusLogicEngineOffsets.ENABLED);
    expect(readResult.value).toBe(1);

    // Set condition: HR 100 == 50
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_SRC_TYPE, RegisterType.HOLDING_REGISTER);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_SRC_ADDR, 100);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_OPERATOR, ConditionOperator.EQUAL);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_VALUE, 50);

    // Set action: Write Coil 10 = 1 (ON)
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_TYPE, CommandType.WRITE_COIL);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_TARGET, 10);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_PARAM1, 1);

    const rule = engine.getRule(ruleId);
    expect(rule?.enabled).toBe(true);
    expect(rule?.conditionSourceType).toBe(RegisterType.HOLDING_REGISTER);
    expect(rule?.conditionSourceAddress).toBe(100);
    expect(rule?.conditionOperator).toBe(ConditionOperator.EQUAL);
    expect(rule?.conditionValue).toBe(50);
    expect(rule?.commandType).toBe(CommandType.WRITE_COIL);
    expect(rule?.commandTarget).toBe(10);
    expect(rule?.commandParam1).toBe(1);
  });

  it('should correctly evaluate a simple rule and perform action', async () => {
    const ruleId = 0;
    const ruleBaseAddr = 1000; // Assuming start addr 1000
    vi.useFakeTimers();

    // Configure Rule 0: IF HR_10 >= 5 THEN Write Coil_1 = ON
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.ENABLED, 1);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_SRC_TYPE, RegisterType.HOLDING_REGISTER);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_SRC_ADDR, 10);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_OPERATOR, ConditionOperator.GREATER_EQUAL);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_VALUE, 5);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_TYPE, CommandType.WRITE_COIL);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_TARGET, 1);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_PARAM1, 1);

    // Set initial Modbus values
    await engine.setModbusHoldingRegister(10, 3); // Condition not met
    await engine.setModbusCoil(1, false);

    engine.start(); // Start the engine's loop

    // Advance time for one loop cycle (default 100ms)
    await vi.advanceTimersByTimeAsync(110);
    
    let coilValue = await engine.getModbusCoil(1);
    expect(coilValue).toBe(false); // Condition not met, coil should remain OFF
    let ruleState = engine.getRule(ruleId);
    expect(ruleState?.triggerCount).toBe(0);

    // Change HR value to meet condition
    await engine.setModbusHoldingRegister(10, 7);

    // Advance time for another loop cycle
    await vi.advanceTimersByTimeAsync(110);

    coilValue = await engine.getModbusCoil(1);
    expect(coilValue).toBe(true); // Condition met, coil should be ON
    ruleState = engine.getRule(ruleId);
    expect(ruleState?.triggerCount).toBe(1);
    expect(ruleState?.lastStatus).toBe(RuleStatusNoError);

    engine.stop();
    vi.useRealTimers();
  });

  // Add more tests for different conditions, actions, edge cases, and error handling.
  // For example: test CALL_COMPONENT_METHOD, test different operators, test rule disabling, etc.

  it('should register and call a component method', async () => {
    const ruleId = 1;
    const ruleBaseAddr = 1000 + 13; // Rule 1, assuming 13 regs per rule
    const testComponentId = 5;
    const testMethodId = 2;
    const mockCallable = vi.fn(async (arg1: number, arg2: number) => {
      await engine.setModbusHoldingRegister(200, arg1 + arg2); // Simulate method action
      return 0; // E_OK
    });

    engine.registerMethod(testComponentId, testMethodId, mockCallable);

    // Configure Rule 1: IF Coil_2 IS ON THEN Call Component_5.Method_2 with Arg1=10
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.ENABLED, 1);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_SRC_TYPE, RegisterType.COIL);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_SRC_ADDR, 2);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_OPERATOR, ConditionOperator.EQUAL);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COND_VALUE, 1); // Coil is ON
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_TYPE, CommandType.CALL_COMPONENT_METHOD);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_TARGET, testComponentId);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_PARAM1, testMethodId);
    await engine.mb_write(ruleBaseAddr + ModbusLogicEngineOffsets.COMMAND_PARAM2, 10); // Arg1 for the method

    await engine.setModbusCoil(2, true); // Meet the condition
    await engine.setModbusHoldingRegister(200, 0); // Initial value for result check

    vi.useFakeTimers();
    engine.start();
    await vi.advanceTimersByTimeAsync(110);
    engine.stop();
    vi.useRealTimers();

    expect(mockCallable).toHaveBeenCalledTimes(1);
    // In performCallAction, we pass commandParam2 (10) as arg1, and 0 as arg2 to the callable.
    expect(mockCallable).toHaveBeenCalledWith(10, 0);
    const resultRegister = await engine.getModbusHoldingRegister(200);
    expect(resultRegister).toBe(10 + 0); // 10 (arg1 from rule) + 0 (hardcoded second arg in performCallAction)

    const ruleState = engine.getRule(ruleId);
    expect(ruleState?.triggerCount).toBe(1);
    expect(ruleState?.lastStatus).toBe(RuleStatusNoError);
  });

}); 
import { Logger, ILogObj } from "tslog";
import yargs from "yargs";
import { hideBin } from "yargs/helpers";
import { ModbusLogicEngine } from "./ModbusLogicEngine.js";
import { ModbusTCPServerWrapper } from "./ModbusTCPServerWrapper.js";
import {
  CommandType,
  ConditionOperator,
  RegisterType,
  RuleStatusNoError,
  ModbusLogicEngineOffsets,
  RULE_FLAG_DEBUG,
  DEFAULT_MODBUS_LOGIC_RULES_START,
  DEFAULT_LOGIC_ENGINE_REGISTERS_PER_RULE
} from "./types.js";

const mainLog: Logger<ILogObj> = new Logger({ name: "MainApp" });
const engine = new ModbusLogicEngine({}, mainLog);
let serverWrapper: ModbusTCPServerWrapper | null = null;

// Example: Register a simple callable method for testing
const COMP_ID_TEST = 1;
const METHOD_ID_TEST_ADD = 1;

engine.registerMethod(COMP_ID_TEST, METHOD_ID_TEST_ADD, async (arg1, arg2) => {
  mainLog.info(`Called TEST_METHOD_ADD with arg1=${arg1}, arg2=${arg2}`);
  // In a real scenario, arg2 might not be used if the method only expects one arg from the rule.
  // const sum = arg1 + arg2; // If method used both
  const sum = arg1 + 5; // Example: arg1 from rule, 5 is an internal value
  mainLog.info(`TEST_METHOD_ADD result: ${sum}`);
  // This method should interact with some component and return a status code (0 for E_OK)
  // For simulation, we can write the result to another Modbus register.
  await engine.setModbusHoldingRegister(500, sum); // Example: Write result to HR 500
  return 0; // Return E_OK
});

async function setupAndRunDemo() {
  await engine.setup();

  // --- Example 1 from mb-lang.md: Turn on Relay 5 if Register 200 >= 100 ---
  // Rule 0
  // Relay 5 is Coil 5
  // Register 200
  // MODBUS_LOGIC_RULES_START = 1000
  const rule0AddrBase = 1000;
  await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.ENABLED, 1);
  await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.COND_SRC_TYPE, RegisterType.HOLDING_REGISTER);
  await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.COND_SRC_ADDR, 200);
  await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.COND_OPERATOR, ConditionOperator.GREATER_EQUAL);
  await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.COND_VALUE, 100);
  await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.COMMAND_TYPE, CommandType.WRITE_COIL);
  await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.COMMAND_TARGET, 5); // Coil address 5
  await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.COMMAND_PARAM1, 1); // Value ON
  // await engine.mb_write(rule0AddrBase + ModbusLogicEngineOffsets.FLAGS, RULE_FLAG_DEBUG);

  // Set initial Modbus values for demo
  await engine.setModbusHoldingRegister(200, 90); // Initial value for HR 200 (condition not met)
  await engine.setModbusCoil(5, false); // Initial state for Coil 5 (OFF)
  mainLog.info("Initial: HR 200 = 90, Coil 5 should be OFF");

  // --- Example 2: Call resetCounter() Method on Component StatsTracker if Coil 10 is ON ---
  // Rule 1 (Base = 1000 + 13 = 1013)
  // Coil 10
  // Component StatsTracker ID = COMP_ID_TEST (1), Method resetCounter ID = METHOD_ID_TEST_ADD (1)
  const rule1AddrBase = 1013;
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.ENABLED, 1);
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.COND_SRC_TYPE, RegisterType.COIL);
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.COND_SRC_ADDR, 10);
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.COND_OPERATOR, ConditionOperator.EQUAL);
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.COND_VALUE, 1); // Condition: Coil ON
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.COMMAND_TYPE, CommandType.CALL_COMPONENT_METHOD);
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.COMMAND_TARGET, COMP_ID_TEST); // Component ID
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.COMMAND_PARAM1, METHOD_ID_TEST_ADD); // Method ID
  await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.COMMAND_PARAM2, 123); // Arg1 for the method
  // await engine.mb_write(rule1AddrBase + ModbusLogicEngineOffsets.FLAGS, RULE_FLAG_DEBUG);

  await engine.setModbusCoil(10, false); // Initial: Coil 10 is OFF (condition not met)
  await engine.setModbusHoldingRegister(500, 0); // For checking call method result

  engine.start(); // Start the rule evaluation loop

  // Simulate changes to Modbus values over time
  setTimeout(async () => {
    mainLog.info("\n---> SIMULATING HR 200 changing to 150 <--- After 2s");
    await engine.setModbusHoldingRegister(200, 150); // Condition for Rule 0 should now be met
  }, 2000);

  setTimeout(async () => {
    mainLog.info("\n---> SIMULATING Coil 10 changing to ON (true) <--- After 4s");
    await engine.setModbusCoil(10, true); // Condition for Rule 1 should now be met
  }, 4000);

  // Allow the engine to run for a bit
  setTimeout(() => {
    mainLog.info("\n--- Engine run finished (10s) ---");
    const rule0 = engine.getRule(0);
    const rule1 = engine.getRule(1);
    mainLog.info("Rule 0 final state:", rule0);
    mainLog.info("Rule 1 final state:", rule1);
    engine.getModbusCoil(5).then(val => mainLog.info("Final Coil 5 state:", val));
    engine.getModbusHoldingRegister(500).then(val => mainLog.info("Final HR 500 (from method call):", val));
    engine.stop();
    process.exit(0);
  }, 10000);
}

yargs(hideBin(process.argv))
  .command(
    "run",
    "Run the ModbusLogicEngine demo",
    () => {},
    async (argv) => {
      mainLog.info("Starting Modbus Logic Engine Demo...");
      await setupAndRunDemo();
    }
  )
  .command(
    "read-rule <ruleId>",
    "Read a specific rule configuration and status",
    (y) => y.positional("ruleId", { type: "number", describe: "The ID (index) of the rule to read", demandOption: true }),
    async (argv) => {
        const ruleId = argv.ruleId as number;
        if (ruleId === undefined || ruleId < 0 || ruleId >= (await engine.mb_read(0)).value ) { // A bit hacky to get maxRules
            mainLog.error(`Invalid ruleId: ${ruleId}. Must be between 0 and maxRules-1.`);
            // TODO: Get maxRules from engine config directly if exposed
            return;
        }
        const rule = engine.getRule(ruleId);
        if (rule) {
            mainLog.info(`Rule ${ruleId} Data:`, rule);
        } else {
            mainLog.error(`Rule ${ruleId} not found.`);
        }
    }
  )
  .command(
    "read-reg <address>",
    "Read a Modbus register (simulated, via engine's mb_read)",
    (y) => y.positional("address", { type: "number", describe: "The Modbus address to read", demandOption: true }),
    async (argv) => {
        const address = argv.address as number;
        const result = await engine.mb_read(address);
        mainLog.info(`Read Addr ${address}: Value=${result.value}, Status=${result.status}`);
    }
  )
  .command(
    "write-reg <address> <value>",
    "Write a Modbus register (simulated, via engine's mb_write)",
    (y) => y
        .positional("address", { type: "number", describe: "The Modbus address to write", demandOption: true })
        .positional("value", { type: "number", describe: "The value to write", demandOption: true }),
    async (argv) => {
        const address = argv.address as number;
        const value = argv.value as number;
        const status = await engine.mb_write(address, value);
        mainLog.info(`Write Addr ${address}, Value ${value}: Status=${status}`);
    }
  )
  .command(
    "serve",
    "Start the ModbusLogicEngine as a TCP server",
    (y) => y
        .option('port', { type: 'number', default: 502, describe: 'Port for Modbus TCP server' })
        .option('host', { type: 'string', default: '0.0.0.0', describe: 'Host for Modbus TCP server' })
        .option('unitId', { type: 'number', default: 1, describe: 'Modbus Server Unit ID' }),
    async (argv) => {
      mainLog.info(`Attempting to start Modbus Logic Engine TCP server on ${argv.host}:${argv.port} with Unit ID ${argv.unitId}...`);
      
      await engine.setup(); 
      // await setupDefaultRules(engine);
      engine.start(); 
      mainLog.info("ModbusLogicEngine initialized, default rules set, and its loop started.");
      serverWrapper = new ModbusTCPServerWrapper(engine, mainLog, { 
        host: argv.host as string,
        port: argv.port as number,
        unitId: argv.unitId as number 
      });
      engine.setModbusServer(serverWrapper);
      try {
        await serverWrapper.start();
        mainLog.info(`Modbus TCP server is listening on ${argv.host}:${argv.port}, Unit ID: ${argv.unitId}`);
        // Process will be kept alive by the listening server
      } catch (error) {
        mainLog.error("Failed to start Modbus TCP server:", error);
        engine.stop();
        process.exit(1);
      }

      

      // Graceful shutdown
      process.on('SIGINT', async () => {
        mainLog.info("SIGINT received, shutting down...");
        if (serverWrapper) {
          try {
            await serverWrapper.stop();
          } catch (e) {
            mainLog.error("Error stopping TCP server wrapper:", e);
          }
        }
        engine.stop();
        mainLog.info("Shutdown complete.");
        process.exit(0);
      });

      // Prevent script from exiting immediately if server start is very fast or has no async hold
      // The net.Server.listen() should keep it alive.
      // If it exits, it means listen() isn't holding or an error occurred before it could.
    }
  )
  .demandCommand(1, "Please specify a command.")
  .help()
  .strict()
  .parse();

// Default action if no command is given, or provide a hint.
if (!process.argv.slice(2).length && process.env.NODE_ENV !== 'test') {
  yargs.showHelp();
}

async function setupDefaultRules(engineInstance: ModbusLogicEngine) {
  mainLog.info("Setting up default rules for TCP server...");
  const rulesStartAddr = DEFAULT_MODBUS_LOGIC_RULES_START; // Use the constant from types

  // Rule 0: If Holding Register 100 >= 50, Write Coil 0 = ON
  const rule0Base = rulesStartAddr; // For Rule 0
  await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.ENABLED, 1);
  await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.COND_SRC_TYPE, RegisterType.HOLDING_REGISTER);
  await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.COND_SRC_ADDR, 100);
  await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.COND_OPERATOR, ConditionOperator.GREATER_EQUAL);
  await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.COND_VALUE, 50);
  await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.COMMAND_TYPE, CommandType.WRITE_COIL);
  await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.COMMAND_TARGET, 0); // Target Coil 0
  await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.COMMAND_PARAM1, 1); // Value ON
  // await engineInstance.mb_write(rule0Base + ModbusLogicEngineOffsets.FLAGS, RULE_FLAG_DEBUG);
  mainLog.info(`Rule 0 configured: HR 100 >= 50 -> Coil 0 ON. Base: ${rule0Base}`);

  // Rule 1: If Coil 1 is ON, Write Holding Register 101 = 1234
  const rule1Base = rulesStartAddr + DEFAULT_LOGIC_ENGINE_REGISTERS_PER_RULE; // For Rule 1
  await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.ENABLED, 1);
  await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.COND_SRC_TYPE, RegisterType.COIL);
  await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.COND_SRC_ADDR, 1);
  await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.COND_OPERATOR, ConditionOperator.EQUAL);
  await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.COND_VALUE, 1); // Value ON
  await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.COMMAND_TYPE, CommandType.WRITE_HOLDING_REGISTER);
  await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.COMMAND_TARGET, 101); // Target HR 101
  await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.COMMAND_PARAM1, 1234); // Value 1234
  //await engineInstance.mb_write(rule1Base + ModbusLogicEngineOffsets.FLAGS, 0); // No debug/receipt for this one
  mainLog.info(`Rule 1 configured: Coil 1 ON -> HR 101 = 1234. Base: ${rule1Base}`);

  // Initialize some values for testing these rules
  await engineInstance.setModbusHoldingRegister(100, 40); // Rule 0 condition initially false
  await engineInstance.setModbusCoil(0, false);          // Rule 0 target initially off
  await engineInstance.setModbusCoil(1, false);          // Rule 1 condition initially false
  await engineInstance.setModbusHoldingRegister(101, 0); // Rule 1 target initially 0
} 
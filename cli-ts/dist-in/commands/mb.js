import { log } from "../lib/logger.js";
import { ModbusClient, E_FN_CODE } from "../lib/modbus-client.js";
export const command = "mb";
export const describe = "Modbus TCP client";
export const builder = {
    host: {
        describe: "The target host",
        demandOption: false,
        type: "string",
        default: "192.168.1.250",
    },
    port: {
        describe: "The target port",
        demandOption: false,
        type: "number",
        default: 502,
    },
    fn: {
        describe: "Function code",
        demandOption: true,
        type: "number",
    },
    reg: {
        describe: "Register address",
        demandOption: true,
        type: "number",
    },
    value: {
        describe: "Value to write",
        demandOption: false,
        type: "number",
    },
    count: {
        describe: "Number of registers/coils to read",
        demandOption: false,
        type: "number",
        default: 1,
    },
    "slave-id": {
        describe: "Modbus slave/unit ID",
        demandOption: false,
        type: "number",
        default: 1,
    }
};
export async function handler(argv) {
    log.info(`Connecting to ${argv.host}:${argv.port}`);
    const client = new ModbusClient(argv.host, argv.port);
    log.info(`Function code: ${argv.fn}`);
    log.info(`Register address: ${argv.reg}`);
    log.info(`Value: ${argv.value}`);
    log.info(`Count: ${argv.count}`);
    log.info(`Slave ID: ${argv["slave-id"]}`);
    try {
        await client.connect();
        if (argv.value !== undefined) { // Write operation
            switch (argv.fn) {
                case E_FN_CODE.FN_WRITE_HOLD_REGISTER:
                    log.info(`Writing holding register ${argv.reg} with value ${argv.value} to slave ${argv["slave-id"]}`);
                    if (argv.reg !== 100) {
                        await client.writeHoldingRegister(argv.reg, argv.value, argv["slave-id"]);
                    }
                    else {
                        client.writeHoldingRegister(argv.reg, argv.value, argv["slave-id"]);
                        process.exit(0);
                    }
                    break;
                case E_FN_CODE.FN_WRITE_COIL:
                    log.info(`Writing coil ${argv.reg} with value ${argv.value} to slave ${argv["slave-id"]}`);
                    await client.writeCoil(argv.reg, argv.value > 0, argv["slave-id"]);
                    log.info(`Write successful.`);
                    break;
                default:
                    log.error(`Function code ${argv.fn} not supported for write operation.`);
            }
        }
        else { // Read operation
            switch (argv.fn) {
                case E_FN_CODE.FN_READ_HOLD_REGISTER: {
                    log.info(`Reading ${argv.count} holding register(s) from address ${argv.reg} on slave ${argv["slave-id"]}`);
                    const values = await client.readHoldingRegisters(argv.reg, argv.count, argv["slave-id"]);
                    process.stdout.write(JSON.stringify(values) + '\n');
                    break;
                }
                case E_FN_CODE.FN_READ_COIL: {
                    log.info(`Reading ${argv.count} coil(s) from address ${argv.reg} on slave ${argv["slave-id"]}`);
                    const values = await client.readCoils(argv.reg, argv.count, argv["slave-id"]);
                    process.stdout.write(JSON.stringify(values) + '\n');
                    break;
                }
                default:
                    log.error(`Function code ${argv.fn} not supported for read operation.`);
            }
        }
    }
    catch (error) {
        if (error instanceof Error) {
            log.error(`An error occurred: ${error.message}`);
        }
        else {
            log.error("An unknown error occurred.", error);
        }
    }
    finally {
        client.disconnect();
    }
}
//# sourceMappingURL=mb.js.map
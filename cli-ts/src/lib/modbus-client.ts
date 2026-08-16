import { log } from "./logger.js";
import net from "net";


export enum E_FN_CODE {
    FN_ANY_FUNCTION_CODE = 0x00, // Only valid for server to register function codes
    FN_READ_COIL = 0x01,
    FN_READ_DISCR_INPUT = 0x02,
    FN_READ_HOLD_REGISTER = 0x03,
    FN_READ_INPUT_REGISTER = 0x04,
    FN_WRITE_COIL = 0x05,
    FN_WRITE_HOLD_REGISTER = 0x06,
    FN_READ_EXCEPTION_SERIAL = 0x07,
    FN_DIAGNOSTICS_SERIAL = 0x08,
    FN_READ_COMM_CNT_SERIAL = 0x0B,
    FN_READ_COMM_LOG_SERIAL = 0x0C,
    FN_WRITE_MULT_COILS = 0x0F,
    FN_WRITE_MULT_REGISTERS = 0x10,
    FN_REPORT_SERVER_ID_SERIAL = 0x11,
    FN_READ_FILE_RECORD = 0x14,
    FN_WRITE_FILE_RECORD = 0x15,
    FN_MASK_WRITE_REGISTER = 0x16,
    FN_R_W_MULT_REGISTERS = 0x17,
    FN_READ_FIFO_QUEUE = 0x18,
    FN_ENCAPSULATED_INTERFACE = 0x2B,
    FN_USER_DEFINED_41 = 0x41,
    FN_USER_DEFINED_42 = 0x42,
    FN_USER_DEFINED_43 = 0x43,
    FN_USER_DEFINED_44 = 0x44,
    FN_USER_DEFINED_45 = 0x45,
    FN_USER_DEFINED_46 = 0x46,
    FN_USER_DEFINED_47 = 0x47,
    FN_USER_DEFINED_48 = 0x48,
    FN_USER_DEFINED_64 = 0x64,
    FN_USER_DEFINED_65 = 0x65,
    FN_USER_DEFINED_66 = 0x66,
    FN_USER_DEFINED_67 = 0x67,
    FN_USER_DEFINED_68 = 0x68,
    FN_USER_DEFINED_69 = 0x69,
    FN_USER_DEFINED_6A = 0x6A,
    FN_USER_DEFINED_6B = 0x6B,
    FN_USER_DEFINED_6C = 0x6C,
    FN_USER_DEFINED_6D = 0x6D,
    FN_USER_DEFINED_6E = 0x6E,
    FN_NONE = 0xFF,
}
export enum E_ModbusAccess {
    MB_ACCESS_NONE = 0,
    MB_ACCESS_READ_ONLY = 1,
    MB_ACCESS_WRITE_ONLY = 2,
    MB_ACCESS_READ_WRITE = 3
}
export interface RegisterData {
    // Modbus address
    address: number
    // Value of the register
    value: number;
    // Name of the register
    name: string;
    // Component id
    id: string;
    // Type of the register
    type: E_FN_CODE;
    // Flags of the register
    flags: number;
    // Group of the register
    group: string;
    // Component of the register
    component: string;
    // Error of the register
    error?: number;
    // Slave ID of the register
    slaveId?: number;
}

export class ModbusClient {
    private host: string;
    private port: number;
    private client: net.Socket;

    constructor(host: string, port: number) {
        this.host = host;
        this.port = port;
        this.client = new net.Socket();
    }

    connect(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.client.connect(this.port, this.host, () => {
                log.info("Connected to Modbus server");
                resolve();
            });

            this.client.on('error', (err) => {
                log.error("Connection error:", err);
                reject(err);
            });
        });
    }

    disconnect() {
        this.client.end();
        log.info("Disconnected from Modbus server");
    }

    private sendRequest(unitId: number, functionCode: number, data: Buffer): Promise<Buffer> {
        return new Promise((resolve, reject) => {
            const transactionId = Math.floor(Math.random() * 65535);
            const protocolId = 0; // Modbus protocol

            const pdu = Buffer.concat([Buffer.from([functionCode]), data]);
            const length = 1 + pdu.length;

            const header = Buffer.alloc(7);
            header.writeUInt16BE(transactionId, 0);
            header.writeUInt16BE(protocolId, 2);
            header.writeUInt16BE(length, 4);
            header.writeUInt8(unitId, 6);

            const request = Buffer.concat([header, pdu]);

            this.client.once('data', (response) => {
                const responseTransactionId = response.readUInt16BE(0);
                if (responseTransactionId !== transactionId) {
                    return reject(new Error("Transaction ID mismatch"));
                }
                const responseFunctionCode = response.readUInt8(7);
                if (responseFunctionCode > 0x80) {
                    const errorCode = response.readUInt8(8);
                    return reject(new Error(`Modbus Exception (FC: ${functionCode}): ${errorCode}`));
                }
                resolve(response.subarray(8));
            });

            this.client.write(request);
        });
    }

    async writeHoldingRegister(address: number, value: number, unitId: number = 1) {
        const data = Buffer.alloc(4);
        data.writeUInt16BE(address, 0);
        data.writeUInt16BE(value, 2);
        await this.sendRequest(unitId, E_FN_CODE.FN_WRITE_HOLD_REGISTER, data);
    }

    async writeCoil(address: number, value: boolean, unitId: number = 1) {
        const data = Buffer.alloc(4);
        data.writeUInt16BE(address, 0);
        data.writeUInt16BE(value ? 0xFF00 : 0x0000, 2);
        await this.sendRequest(unitId, E_FN_CODE.FN_WRITE_COIL, data);
    }

    async readHoldingRegisters(address: number, count: number, unitId: number = 1): Promise<number[]> {
        const data = Buffer.alloc(4);
        data.writeUInt16BE(address, 0);
        data.writeUInt16BE(count, 2);
        const response = await this.sendRequest(unitId, E_FN_CODE.FN_READ_HOLD_REGISTER, data);

        const byteCount = response[0];
        const values: number[] = [];
        for (let i = 0; i < byteCount / 2; i++) {
            values.push(response.readUInt16BE(1 + i * 2));
        }
        return values;
    }

    async readCoils(address: number, count: number, unitId: number = 1): Promise<boolean[]> {
        const data = Buffer.alloc(4);
        data.writeUInt16BE(address, 0);
        data.writeUInt16BE(count, 2);
        const response = await this.sendRequest(unitId, E_FN_CODE.FN_READ_COIL, data);

        const byteCount = response[0];
        const values: boolean[] = [];
        for (let i = 0; i < byteCount; i++) {
            for (let j = 0; j < 8; j++) {
                if (values.length < count) {
                    values.push((response[1 + i] & (1 << j)) !== 0);
                }
            }
        }
        return values;
    }
} 
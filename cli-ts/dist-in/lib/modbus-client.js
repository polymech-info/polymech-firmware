import { log } from "./logger.js";
import net from "net";
export var E_FN_CODE;
(function (E_FN_CODE) {
    E_FN_CODE[E_FN_CODE["FN_ANY_FUNCTION_CODE"] = 0] = "FN_ANY_FUNCTION_CODE";
    E_FN_CODE[E_FN_CODE["FN_READ_COIL"] = 1] = "FN_READ_COIL";
    E_FN_CODE[E_FN_CODE["FN_READ_DISCR_INPUT"] = 2] = "FN_READ_DISCR_INPUT";
    E_FN_CODE[E_FN_CODE["FN_READ_HOLD_REGISTER"] = 3] = "FN_READ_HOLD_REGISTER";
    E_FN_CODE[E_FN_CODE["FN_READ_INPUT_REGISTER"] = 4] = "FN_READ_INPUT_REGISTER";
    E_FN_CODE[E_FN_CODE["FN_WRITE_COIL"] = 5] = "FN_WRITE_COIL";
    E_FN_CODE[E_FN_CODE["FN_WRITE_HOLD_REGISTER"] = 6] = "FN_WRITE_HOLD_REGISTER";
    E_FN_CODE[E_FN_CODE["FN_READ_EXCEPTION_SERIAL"] = 7] = "FN_READ_EXCEPTION_SERIAL";
    E_FN_CODE[E_FN_CODE["FN_DIAGNOSTICS_SERIAL"] = 8] = "FN_DIAGNOSTICS_SERIAL";
    E_FN_CODE[E_FN_CODE["FN_READ_COMM_CNT_SERIAL"] = 11] = "FN_READ_COMM_CNT_SERIAL";
    E_FN_CODE[E_FN_CODE["FN_READ_COMM_LOG_SERIAL"] = 12] = "FN_READ_COMM_LOG_SERIAL";
    E_FN_CODE[E_FN_CODE["FN_WRITE_MULT_COILS"] = 15] = "FN_WRITE_MULT_COILS";
    E_FN_CODE[E_FN_CODE["FN_WRITE_MULT_REGISTERS"] = 16] = "FN_WRITE_MULT_REGISTERS";
    E_FN_CODE[E_FN_CODE["FN_REPORT_SERVER_ID_SERIAL"] = 17] = "FN_REPORT_SERVER_ID_SERIAL";
    E_FN_CODE[E_FN_CODE["FN_READ_FILE_RECORD"] = 20] = "FN_READ_FILE_RECORD";
    E_FN_CODE[E_FN_CODE["FN_WRITE_FILE_RECORD"] = 21] = "FN_WRITE_FILE_RECORD";
    E_FN_CODE[E_FN_CODE["FN_MASK_WRITE_REGISTER"] = 22] = "FN_MASK_WRITE_REGISTER";
    E_FN_CODE[E_FN_CODE["FN_R_W_MULT_REGISTERS"] = 23] = "FN_R_W_MULT_REGISTERS";
    E_FN_CODE[E_FN_CODE["FN_READ_FIFO_QUEUE"] = 24] = "FN_READ_FIFO_QUEUE";
    E_FN_CODE[E_FN_CODE["FN_ENCAPSULATED_INTERFACE"] = 43] = "FN_ENCAPSULATED_INTERFACE";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_41"] = 65] = "FN_USER_DEFINED_41";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_42"] = 66] = "FN_USER_DEFINED_42";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_43"] = 67] = "FN_USER_DEFINED_43";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_44"] = 68] = "FN_USER_DEFINED_44";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_45"] = 69] = "FN_USER_DEFINED_45";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_46"] = 70] = "FN_USER_DEFINED_46";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_47"] = 71] = "FN_USER_DEFINED_47";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_48"] = 72] = "FN_USER_DEFINED_48";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_64"] = 100] = "FN_USER_DEFINED_64";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_65"] = 101] = "FN_USER_DEFINED_65";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_66"] = 102] = "FN_USER_DEFINED_66";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_67"] = 103] = "FN_USER_DEFINED_67";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_68"] = 104] = "FN_USER_DEFINED_68";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_69"] = 105] = "FN_USER_DEFINED_69";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_6A"] = 106] = "FN_USER_DEFINED_6A";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_6B"] = 107] = "FN_USER_DEFINED_6B";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_6C"] = 108] = "FN_USER_DEFINED_6C";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_6D"] = 109] = "FN_USER_DEFINED_6D";
    E_FN_CODE[E_FN_CODE["FN_USER_DEFINED_6E"] = 110] = "FN_USER_DEFINED_6E";
    E_FN_CODE[E_FN_CODE["FN_NONE"] = 255] = "FN_NONE";
})(E_FN_CODE || (E_FN_CODE = {}));
export var E_ModbusAccess;
(function (E_ModbusAccess) {
    E_ModbusAccess[E_ModbusAccess["MB_ACCESS_NONE"] = 0] = "MB_ACCESS_NONE";
    E_ModbusAccess[E_ModbusAccess["MB_ACCESS_READ_ONLY"] = 1] = "MB_ACCESS_READ_ONLY";
    E_ModbusAccess[E_ModbusAccess["MB_ACCESS_WRITE_ONLY"] = 2] = "MB_ACCESS_WRITE_ONLY";
    E_ModbusAccess[E_ModbusAccess["MB_ACCESS_READ_WRITE"] = 3] = "MB_ACCESS_READ_WRITE";
})(E_ModbusAccess || (E_ModbusAccess = {}));
export class ModbusClient {
    host;
    port;
    client;
    constructor(host, port) {
        this.host = host;
        this.port = port;
        this.client = new net.Socket();
    }
    connect() {
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
    sendRequest(unitId, functionCode, data) {
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
    async writeHoldingRegister(address, value, unitId = 1) {
        const data = Buffer.alloc(4);
        data.writeUInt16BE(address, 0);
        data.writeUInt16BE(value, 2);
        await this.sendRequest(unitId, E_FN_CODE.FN_WRITE_HOLD_REGISTER, data);
    }
    async writeCoil(address, value, unitId = 1) {
        const data = Buffer.alloc(4);
        data.writeUInt16BE(address, 0);
        data.writeUInt16BE(value ? 0xFF00 : 0x0000, 2);
        await this.sendRequest(unitId, E_FN_CODE.FN_WRITE_COIL, data);
    }
    async readHoldingRegisters(address, count, unitId = 1) {
        const data = Buffer.alloc(4);
        data.writeUInt16BE(address, 0);
        data.writeUInt16BE(count, 2);
        const response = await this.sendRequest(unitId, E_FN_CODE.FN_READ_HOLD_REGISTER, data);
        const byteCount = response[0];
        const values = [];
        for (let i = 0; i < byteCount / 2; i++) {
            values.push(response.readUInt16BE(1 + i * 2));
        }
        return values;
    }
    async readCoils(address, count, unitId = 1) {
        const data = Buffer.alloc(4);
        data.writeUInt16BE(address, 0);
        data.writeUInt16BE(count, 2);
        const response = await this.sendRequest(unitId, E_FN_CODE.FN_READ_COIL, data);
        const byteCount = response[0];
        const values = [];
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
//# sourceMappingURL=modbus-client.js.map
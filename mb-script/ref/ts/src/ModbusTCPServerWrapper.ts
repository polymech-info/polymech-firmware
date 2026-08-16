import { ModbusLogicEngine } from "./ModbusLogicEngine.js";
import { Logger, ILogObj } from "tslog";
import Modbus from "jsmodbus"; // Main import from 'jsmodbus'
import { Server as NetServer, Socket as NetSocket } from 'net'; // Node.js net module
import {
    RuleStatusNoError,
    // We need to map our engine's internal status/errors to Modbus exceptions
} from "./types.js";

const E_OK_MB_STATUS_CODE = 0; // Internal status from engine.mb_read indicating success
const E_INVALID_PARAMETER_MB_STATUS_CODE = -2; // Example internal code

// Modbus Standard Exception Codes (subset)
const MB_EXCEPTION_ILLEGAL_FUNCTION = 0x01;
const MB_EXCEPTION_ILLEGAL_DATA_ADDRESS = 0x02;
const MB_EXCEPTION_ILLEGAL_DATA_VALUE = 0x03;
const MB_EXCEPTION_SLAVE_DEVICE_FAILURE = 0x04;

interface ModbusServerOptions {
    host?: string;
    port?: number;
    unitId?: number; // This is for our internal logic, jsmodbus server.TCP might not take it in constructor
    initialHoldingRegisters?: Buffer;
}

// Tentative types based on jsmodbus common patterns and the example
interface JsModbusRequest {
    address: number;
    count?: number; // For read requests
    value?: number | Buffer; // For write requests
    unitId: number; // unitId is usually part of the request PDU
    // other properties may exist
}

interface JsModbusResponseBody {
    forceUnitException: (errorCode: number) => void;
    valuesAsBuffer: Buffer; // For readHoldingRegisters
    address?: number; // For writeSingleRegister response echo
    value?: number;   // For writeSingleRegister response echo
    // other FCs will have different body structures (e.g., coils for readCoils)
    coils: boolean[];
    holdingRegisters: number[];
}

interface JsModbusResponse {
    body: JsModbusResponseBody;
    
}

type JsModbusSendFunction = (response: JsModbusResponse) => void;

export class ModbusTCPServerWrapper {
    private engine: ModbusLogicEngine;
    private logger: Logger<ILogObj>;
    private netServer: NetServer;
    private modbusTCPGateway: Modbus.ModbusTCPServer; // jsmodbus TCP server gateway
    private options: Required<ModbusServerOptions>;

    constructor(engine: ModbusLogicEngine, parentLogger: Logger<ILogObj>, options?: ModbusServerOptions) {
        this.engine = engine;
        this.logger = parentLogger.getSubLogger({ name: "ModbusTCPServerWrapper" });
        
        const initialHoldingRegisters = options?.initialHoldingRegisters ?? Buffer.alloc(1024); // Reduced default size

        this.options = {
            host: options?.host ?? "0.0.0.0",
            port: options?.port ?? 502,
            unitId: options?.unitId ?? 1, 
            initialHoldingRegisters: initialHoldingRegisters,
        };

        this.netServer = new NetServer();
        this.modbusTCPGateway = new Modbus.server.TCP(this.netServer, {
            holding: this.options.initialHoldingRegisters
        });

        this.logger.info(`Modbus TCP Server Gateway configured. Target: ${this.options.host}:${this.options.port}. Our server unit ID (for internal logic): ${this.options.unitId}`);

        this.setupNetServerEventHandlers();
        this.setupModbusGatewayRequestHandlers();

        //test values
        // Set first 5 coils to true
        for (let i = 0; i < 5; i++) { // Coils 0, 1, 2, 3, 4
            const byteIndex = Math.floor(i / 8); // Will be 0 for coils 0-4
            const bitInByte = i % 8;
            // Ensure coils buffer exists and is large enough for the byteIndex
            if (this.modbusTCPGateway.coils && byteIndex < this.modbusTCPGateway.coils.length) {
                this.modbusTCPGateway.coils[byteIndex] |= (1 << bitInByte);
            } else {
                this.logger.warn(`Cannot set coil ${i}: Coils buffer not initialized or too small at byteIndex ${byteIndex}.`);
            }
        }

        this.modbusTCPGateway.discrete.writeUInt16BE(0x5678, 0)

        // Set first 5 holding registers to 10
        for (let i = 0; i < 5; i++) { // Holding registers 0, 1, 2, 3, 4
            const byteOffset = i * 2;
            // Ensure holding buffer exists and is large enough for 2 bytes from byteOffset
            // this.modbusTCPGateway.holding is this.options.initialHoldingRegisters
            if (this.modbusTCPGateway.holding && this.modbusTCPGateway.holding.length >= byteOffset + 2) {
                this.modbusTCPGateway.holding.writeUInt16BE(10, byteOffset);
            } else {
                this.logger.warn(`Cannot set holding register ${i}: Holding buffer not initialized or too small for offset ${byteOffset}.`);
            }
        }

        this.modbusTCPGateway.input.writeUInt16BE(0xff00, 0)
        this.modbusTCPGateway.input.writeUInt16BE(0xff00, 2)

    }

    private setupNetServerEventHandlers() {
        this.netServer.on("connection", (socket: NetSocket) => {
            const clientAddress = socket.remoteAddress ? `${socket.remoteAddress}:${socket.remotePort}` : "unknown client";
            this.logger.info(`TCP Client connected: ${clientAddress}`);
            socket.on("error", (err: Error) => {
                this.logger.warn(`TCP Socket error from ${clientAddress}:`, err.message);
            });
            socket.on("close", (hadError: boolean) => {
                this.logger.info(`TCP Client disconnected: ${clientAddress}${hadError ? ' with error' : ''}`);
            });
        });
        this.netServer.on("error", (err: Error) => {
            this.logger.error("Node net.Server Error:", err);
        });
        this.netServer.on("close", () => {
            this.logger.info("Node net.Server closed (all connections ended).");
        });

        // jsmodbus server.TCP specific events
        this.modbusTCPGateway.on('connection' as any, (client: any) => { // Use 'as any' if types are problematic
            this.logger.info('Modbus client TCP connection handled by jsmodbus gateway.');
            // client here is likely the net.Socket already handled above by netServer.on('connection')
        });
         (this.modbusTCPGateway as any).on('error', (err: Error) => { // Cast to any if .on has type issues
            this.logger.error('Modbus Gateway Error:', err);
        });
    }

    private setupModbusGatewayRequestHandlers() {
        // Assuming this.modbusTCPGateway is an EventEmitter that emits FC events.
        // The types for request, response, send are critical here.

        (this.modbusTCPGateway as any).on("readHoldingRegisters", async (request: JsModbusRequest, response: JsModbusResponse, send: JsModbusSendFunction) => {
            // unitId check - respond only if the request is for our unitId
            if (request.unitId !== this.options.unitId) {
                this.logger.debug(`Ignoring request for different Unit ID: ${request.unitId}`);
                // jsmodbus might handle this automatically, or we might need to not respond / send specific error.
                // For now, we'll assume we should only process requests for our configured unitId.
                // However, a Modbus TCP server usually responds to any Unit ID on the TCP connection unless it's a gateway to serial.
                // The example doesn't show explicit unit ID filtering at this layer for TCP.
                // Let's proceed as if the request is for us, as unitID was passed to Modbus.server.TCP
            }

            const startAddress = request.address;
            const quantity = request.count!; // Assuming count is always present for read requests
            
            this.logger.debug(`FC03 ReadHR: Addr=${startAddress}, Qty=${quantity}, UnitID=${request.unitId}`);

            if (quantity === 0 || quantity > 125) {
                this.logger.warn(`Invalid quantity: ${quantity}`);
                response.body.forceUnitException(MB_EXCEPTION_ILLEGAL_DATA_VALUE);
                return send(response);
            }

            try {
                for (let i = 0; i < quantity; i++) {
                    const currentAddress = startAddress + i;
                    const result = await this.engine.mb_read(currentAddress);

                    if (typeof result.status === "number" && result.status !== E_OK_MB_STATUS_CODE) {
                        this.logger.warn(`Engine mb_read for addr ${currentAddress} failed: ${result.status}`);
                        let mbErrorCode = MB_EXCEPTION_SLAVE_DEVICE_FAILURE;
                        if (result.status === E_INVALID_PARAMETER_MB_STATUS_CODE) {
                            mbErrorCode = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS;
                        }
                        this.logger.warn(`FC03 ReadHR: Sending Modbus Exception ${mbErrorCode} for address ${currentAddress}`);
                        response.body.forceUnitException(mbErrorCode);
                        return send(response);
                    } else if (typeof result.status === "string" && result.status !== RuleStatusNoError) {
                         this.logger.warn(`Engine mb_read for addr ${currentAddress} failed: ${result.status}`);
                         this.logger.warn(`FC03 ReadHR: Sending Modbus Exception ${MB_EXCEPTION_SLAVE_DEVICE_FAILURE} for address ${currentAddress}`);
                         response.body.forceUnitException(MB_EXCEPTION_SLAVE_DEVICE_FAILURE);
                         return send(response);
                    }
                    response.body.valuesAsBuffer.writeUInt16BE(result.value, i * 2);
                }
                this.logger.trace(`FC03 Response for Addr=${startAddress}, Qty=${quantity} prepared.`);
                return send(response);
            } catch (error) {
                this.logger.error("Error processing readHoldingRegisters:", error);
                this.logger.warn(`FC03 ReadHR: Sending Modbus Exception ${MB_EXCEPTION_SLAVE_DEVICE_FAILURE} due to catch block.`);
                response.body.forceUnitException(MB_EXCEPTION_SLAVE_DEVICE_FAILURE);
                return send(response);
            }
        });

        (this.modbusTCPGateway as any).on("readCoils", async (request: JsModbusRequest, response: JsModbusResponse, send: JsModbusSendFunction) => {
            // Unit ID check (consistent with readHoldingRegisters logging)
            if (request.unitId !== this.options.unitId) {
                this.logger.debug(`FC01 ReadCoils: Ignoring request for different Unit ID: ${request.unitId}, processing for ours: ${this.options.unitId}`);
            }

            const startAddress = request.address;
            const quantity = request.count!; // Assuming count is always present for read requests

            this.logger.debug(`FC01 ReadCoils: Addr=${startAddress}, Qty=${quantity}, UnitID=${request.unitId}`);

            if (quantity === 0 || quantity > 2000) { // Modbus spec: 1 to 2000 coils
                this.logger.warn(`FC01 ReadCoils: Invalid quantity: ${quantity}`);
                response.body.forceUnitException(MB_EXCEPTION_ILLEGAL_DATA_VALUE);
                return send(response);
            }

            try {
                // Ensure response.body.coils is initialized and has the correct length
                response.body.coils = new Array(quantity);

                for (let i = 0; i < quantity; i++) {
                    const currentAddress = startAddress + i;
                    const result = await this.engine.mb_read(currentAddress); // Assuming engine.mb_read can handle coils

                    if (typeof result.status === "number" && result.status !== E_OK_MB_STATUS_CODE) {
                        this.logger.warn(`Engine mb_read for coil addr ${currentAddress} failed: ${result.status}`);
                        let mbErrorCode = MB_EXCEPTION_SLAVE_DEVICE_FAILURE;
                        if (result.status === E_INVALID_PARAMETER_MB_STATUS_CODE) {
                            mbErrorCode = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS;
                        }
                        this.logger.warn(`FC01 ReadCoils: Sending Modbus Exception ${mbErrorCode} for address ${currentAddress}`);
                        response.body.forceUnitException(mbErrorCode);
                        return send(response);
                    } else if (typeof result.status === "string" && result.status !== RuleStatusNoError) {
                         this.logger.warn(`Engine mb_read for coil addr ${currentAddress} failed: ${result.status}`);
                         this.logger.warn(`FC01 ReadCoils: Sending Modbus Exception ${MB_EXCEPTION_SLAVE_DEVICE_FAILURE} for address ${currentAddress}`);
                         response.body.forceUnitException(MB_EXCEPTION_SLAVE_DEVICE_FAILURE);
                         return send(response);
                    }
                    // Assuming result.value is 0 or 1 for coils. Convert to boolean.
                    response.body.coils[i] = result.value !== 0;
                }
                this.logger.trace(`FC01 Response for Addr=${startAddress}, Qty=${quantity} prepared.`);
                return send(response);
            } catch (error) {
                this.logger.error("Error processing readCoils:", error);
                this.logger.warn(`FC01 ReadCoils: Sending Modbus Exception ${MB_EXCEPTION_SLAVE_DEVICE_FAILURE} due to catch block.`);
                response.body.forceUnitException(MB_EXCEPTION_SLAVE_DEVICE_FAILURE);
                return send(response);
            }
        });

        (this.modbusTCPGateway as any).on("writeSingleRegister", async (request: JsModbusRequest, response: JsModbusResponse, send: JsModbusSendFunction) => {
            const address = request.address;
            const value = request.value as number; // Assuming value is a number for FC06
            this.logger.debug(`FC06 WriteSR: Addr=${address}, Value=${value}, UnitID=${request.unitId}`);

            // Add unitId check if necessary (as above)

            if (typeof value !== 'number') {
                this.logger.warn(`Invalid value type for WriteSingleRegister: ${typeof value}`);
                response.body.forceUnitException(MB_EXCEPTION_ILLEGAL_DATA_VALUE);
                return send(response);
            }
            
            try {
                const status = await this.engine.mb_write(address, value);
                if (typeof status === "number" && status !== E_OK_MB_STATUS_CODE) {
                    this.logger.warn(`Engine mb_write for addr ${address} value ${value} failed: ${status}`);
                    let mbErrorCode = MB_EXCEPTION_SLAVE_DEVICE_FAILURE;
                    if (status === E_INVALID_PARAMETER_MB_STATUS_CODE) {
                         mbErrorCode = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS;
                    } 
                    this.logger.warn(`FC06 WriteSR: Sending Modbus Exception ${mbErrorCode} for address ${address}`);
                    response.body.forceUnitException(mbErrorCode);
                    return send(response);
                } else if (typeof status === "string" && status !== RuleStatusNoError) {
                    this.logger.warn(`Engine mb_write for addr ${address} value ${value} failed: ${status}`);
                    this.logger.warn(`FC06 WriteSR: Sending Modbus Exception ${MB_EXCEPTION_SLAVE_DEVICE_FAILURE} for address ${address}`);
                    response.body.forceUnitException(MB_EXCEPTION_SLAVE_DEVICE_FAILURE);
                    return send(response);
                }
                
                response.body.address = address; 
                response.body.value = value;
                this.logger.trace(`FC06 Write to Addr=${address} Value=${value} successful.`);
                return send(response);
            } catch (error) {
                this.logger.error("Error processing writeSingleRegister:", error);
                this.logger.warn(`FC06 WriteSR: Sending Modbus Exception ${MB_EXCEPTION_SLAVE_DEVICE_FAILURE} due to catch block.`);
                response.body.forceUnitException(MB_EXCEPTION_SLAVE_DEVICE_FAILURE);
                return send(response);
            }
        });
    }

    public getModbusGateway(): Modbus.ModbusTCPServer {
        return this.modbusTCPGateway;
    }

    public start(): Promise<void> {
        return new Promise((resolve, reject) => {
            if (this.netServer.listening) {
                this.logger.warn("Server is already listening.");
                return resolve();
            }
            // Store the error handler to remove it later if listen succeeds
            const listenErrorHandler = (err: Error) => {
                this.logger.error(`Failed to start Modbus TCP server on ${this.options.host}:${this.options.port}:`, err);
                this.netServer.removeListener('error', listenErrorHandler); // Clean up listener
                reject(err);
            };
            this.netServer.once('error', listenErrorHandler); // Use once for listen-specific error

            this.netServer.listen({ host: this.options.host, port: this.options.port }, () => {
                this.logger.info(`Modbus TCP Server listening on ${this.options.host}:${this.options.port}`);
                this.netServer.removeListener('error', listenErrorHandler); // Clean up listener on success
                resolve();
            });
        });
    }

    public stop(): Promise<void> {
        return new Promise((resolve) => {
            if (!this.netServer.listening) {
                this.logger.warn("Server is not listening or already closed.");
                return resolve();
            }
            this.netServer.close((err?: Error) => {
                if (err) {
                    this.logger.error("Error while stopping Modbus TCP Server:", err);
                }
                this.logger.info("Modbus TCP Server stopped.");
                resolve();
            });
        });
    }
} 
import fs from 'fs';
import path from 'path';
import { client as ModbusClient } from 'jsmodbus';
import net from 'net';
import { fileURLToPath } from 'url';
import { Logger, ILogObj } from "tslog";
import WebSocket from 'ws';
import { SerialPort } from 'serialport';
import { ReadlineParser } from '@serialport/parser-readline';

// --- Logger Setup ---
const log: Logger<ILogObj> = new Logger({ name: "ModbusTest" });

// --- Configuration ---
export const ESP32_IP = '192.168.1.250'; // Store IP as constant
export const MODBUS_PORT = 502; // Default Modbus TCP port
export const WEBSOCKET_PATH = '/ws'; // WebSocket path
export const WEBSOCKET_URL = `ws://${ESP32_IP}${WEBSOCKET_PATH}`;
export const SERIAL_BAUD_RATE = 115200; // Baud rate from Python script
export const SERIAL_PORT_MANUAL: string | undefined = 'COM14'; // Define specific port here if needed
export let SERIAL_PORT_PATH: string | undefined = undefined; // To be auto-detected
export const REPORTS_DIR = './tests/reports';

// Helper to resolve __dirname in ESM
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Ensure reports directory exists
const reportsPath = path.resolve(__dirname, '..', REPORTS_DIR); // Resolve relative to project root
if (!fs.existsSync(reportsPath)) {
    fs.mkdirSync(reportsPath, { recursive: true });
    log.info(`Created reports directory: ${reportsPath}`);
}

// --- Serial Port Auto-Detection ---
// Basic auto-detection based on common ESP identifiers
async function findSerialPort(): Promise<string | undefined> {
    log.info("Attempting to auto-detect serial port...");
    try {
        const ports = await SerialPort.list();
        log.debug("Available ports:", ports.map(p => ({ path: p.path, manufacturer: p.manufacturer, pnpId: p.pnpId })));
        for (const port of ports) {
            const pnpId = port.pnpId || '';
            const manufacturer = port.manufacturer || '';
            // Match common ESP patterns (add more if needed)
            if (pnpId.includes('VID_10C4&PID_EA60') || // CP210x
                pnpId.includes('VID_303A&PID_1001') || // ESP32-S3 Internal
                manufacturer.toLowerCase().includes('espressif') ||
                manufacturer.toLowerCase().includes('silicon labs')) { 
                log.info(`Found potential ESP device: ${port.path}`);
                return port.path;
            }
        }
        log.warn("Could not automatically find ESP serial port.");
        // Optionally fallback to manual port if detection fails
        // return SERIAL_PORT_MANUAL;
        return undefined;
    } catch (error) {
        log.error("Error listing serial ports:", error);
        return undefined;
    }
}

// --- Serial Port Client Setup ---
export function createSerialClient(path: string): SerialPort {
    log.info(`Creating SerialPort client for path: ${path}, Baud: ${SERIAL_BAUD_RATE}`);
    // autoOpen: false means we call open() explicitly later
    const port = new SerialPort({ path, baudRate: SERIAL_BAUD_RATE, autoOpen: false });
    return port;
}

// Utility to connect Serial Port cleanly
export async function connectSerial(port: SerialPort): Promise<void> {
    log.info(`Attempting Serial connection to ${port.path}...`);
    return new Promise((resolve, reject) => {
        port.open((err) => {
            if (err) {
                log.error(`Serial connection error to ${port.path}:`, err);
                reject(err);
            } else {
                log.info(`Serial port ${port.path} opened successfully.`);
                resolve();
            }
        });
    });
}

// Utility to send a command and wait for response data (potentially multi-line)
// Uses ReadlineParser to get lines like the Python script
export async function sendSerialCommandAndReceive(port: SerialPort, command: string, timeout: number = 5000): Promise<string> {
    if (!command.endsWith('\n')) {
        command += '\n'; // Ensure newline termination
    }
    log.debug(`Sending Serial command: ${JSON.stringify(command)} to ${port.path}`);
    
    const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));
    let receivedData = '';
    let responseResolver: (value: string | PromiseLike<string>) => void;
    let responseRejecter: (reason?: any) => void;
    const responsePromise = new Promise<string>((resolve, reject) => {
        responseResolver = resolve;
        responseRejecter = reject;
    });

    const timer = setTimeout(() => {
        log.warn(`Serial response timeout (${timeout}ms) for command: ${JSON.stringify(command)}`);
        parser.off('data', onData);
        // Resolve with whatever data was received before timeout, or reject if empty?
        // For now, resolve with potentially partial data, could be changed to reject.
        responseResolver(receivedData); 
    }, timeout);

    const onData = (line: string) => {
        const trimmedLine = line.trim();
        log.debug(`Received Serial line: ${trimmedLine}`);
        receivedData += trimmedLine + '\n';
        // Simple approach: Keep collecting lines until timeout.
        // More sophisticated logic could be added here to detect end-of-response.
    };

    parser.on('data', onData);

    // Send the command
    port.write(command, (err) => {
        if (err) {
            log.error("Error writing to serial port:", err);
            clearTimeout(timer);
            parser.off('data', onData);
            responseRejecter(err);
        } else {
            log.debug(`Command ${JSON.stringify(command)} sent successfully.`);
            port.drain((drainErr) => { // Ensure data is sent before waiting for response
                if (drainErr) {
                     log.error("Error draining serial port:", drainErr);
                    // Continue anyway, maybe it still worked
                }
            });
        }
    });

    return responsePromise;
}


// Utility to disconnect Serial Port cleanly
export function disconnectSerial(port: SerialPort) {
    if (port && port.isOpen) {
        log.info(`Closing Serial connection to ${port.path}`);
        port.close((err) => {
            if (err) {
                log.error(`Error closing serial port ${port.path}:`, err);
            } else {
                log.info(`Serial port ${port.path} closed.`);
            }
        });
    } else {
        log.warn(`Serial port ${port.path} is not open.`);
    }
}

// --- Modbus TCP Client Setup ---
export function createModbusClient() {
    const socket = new net.Socket();
    // Use correct client factory for TCP
    const client = new ModbusClient.TCP(socket);

    // Optional: Add error handling for the socket
    socket.on('error', (err) => {
        log.error("Socket Error:", err);
        // Potentially close the socket or handle reconnection here
    });

    socket.on('close', () => {
        log.info('Socket closed');
    });

    return { socket, client };
}

// --- WebSocket Client Setup ---
export function createWebSocketClient(url: string = WEBSOCKET_URL): WebSocket {
    log.info(`Creating WebSocket client for URL: ${url}`);
    const ws = new WebSocket(url);
    return ws;
}

// --- Test Reporting ---
export function createReport(testName: string): fs.WriteStream {
    const now = new Date();
    // Format: YYYYMMDD-HHMMSS - Using a more standard timestamp
    const timestamp = now.toISOString().replace(/[:T.]/g, '-').split('-').slice(0, 3).join('') + '_' + now.toTimeString().split(' ')[0].replace(/:/g, '');
    const reportFileName = `${timestamp}-${testName.replace(/\s+/g, '_')}.md`; // Sanitize test name for filename
    const reportPath = path.join(reportsPath, reportFileName);

    const reportStream = fs.createWriteStream(reportPath, { encoding: 'utf8' });

    reportStream.write(`# Modbus E2E Test Report: ${testName}

`);
    reportStream.write(`*   **Test:** ${testName}
`);
    reportStream.write(`*   **Timestamp:** ${now.toISOString()}
`);
    // Distinguish target type in report
    if (testName.toLowerCase().includes('websocket') || testName.toLowerCase().includes('ws')) {
        reportStream.write(`*   **Target URL:** ${WEBSOCKET_URL}\n`);
    } else if (testName.toLowerCase().includes('serial')) {
        // Report the port actually being used (could be auto or manual)
        reportStream.write(`*   **Target Port:** ${SERIAL_PORT_PATH || SERIAL_PORT_MANUAL || 'Not Found'} @ ${SERIAL_BAUD_RATE}\n`);
    } else { // Default to Modbus TCP
        reportStream.write(`*   **Target IP:** ${ESP32_IP}:${MODBUS_PORT}\n`);
    }
    reportStream.write(`*   **Report File:** ${reportFileName}

`);
    reportStream.write(`## Test Log\n\n`);

    log.info(`Created report file: ${reportPath}`);
    return reportStream;
}

export function logToReport(reportStream: fs.WriteStream, message: string, level: keyof Logger<ILogObj> = 'info') {
    const timestamp = new Date().toISOString();
    const logMessage = `[${timestamp}] ${message}`;
    reportStream.write(logMessage + '\n');

    if (typeof log[level] === 'function') {
       (log[level] as (message: string) => void)(message);
    } else {
       log.info(message);
    }
}

export function closeReport(reportStream: fs.WriteStream) {
    logToReport(reportStream, "Test finished.", 'info');
    reportStream.end();
}

// Utility to connect and disconnect cleanly
export async function connectModbus(socket: net.Socket, host: string, port: number): Promise<void> {
    return new Promise((resolve, reject) => {
        socket.connect({ host, port }, () => {
            log.info(`Connected to Modbus server ${host}:${port}`);
            resolve();
        });
        socket.once('error', (err) => {
            log.error(`Connection error to ${host}:${port}:`, err);
            reject(err);
        });
    });
}

export function disconnectModbus(socket: net.Socket) {
    if (socket && !socket.destroyed) {
        socket.destroy();
        log.info('Disconnected from Modbus server.');
    }
}

// Utility to connect WebSocket cleanly
export async function connectWebSocket(ws: WebSocket): Promise<void> {
    log.info(`Attempting WebSocket connection to ${ws.url}...`);
    return new Promise((resolve, reject) => {
        const onOpen = () => {
            log.info(`WebSocket connected to ${ws.url}`);
            ws.off('error', onError); // Clean up error listener
            resolve();
        };
        const onError = (err: Error) => {
            log.error(`WebSocket connection error to ${ws.url}:`, err);
            ws.off('open', onOpen); // Clean up open listener
            reject(err);
        };
        
        ws.once('open', onOpen);
        ws.once('error', onError);
    });
}

// Utility to send a message and wait for a specific response type
export async function sendWsMessageAndReceive(ws: WebSocket, message: any, expectedResponseType: string | null = null, timeout: number = 5000): Promise<any> {
    const messageString = JSON.stringify(message);
    log.debug(`Sending WebSocket message: ${messageString}`);
    ws.send(messageString);

    return new Promise((resolve, reject) => {
        const timer = setTimeout(() => {
            log.error(`WebSocket response timeout (${timeout}ms) waiting for type '${expectedResponseType || 'any'}'`);
            ws.off('message', onMessage); // Clean up listener on timeout
            reject(new Error(`WebSocket response timeout after ${timeout}ms waiting for type '${expectedResponseType || 'any'}'`));
        }, timeout);

        const onMessage = (data: WebSocket.RawData) => {
            let receivedData: any;
            try {
                receivedData = JSON.parse(data.toString());
                log.debug(`Received WebSocket message: ${JSON.stringify(receivedData, null, 2)}`);
            } catch (err: any) {
                // Ignore parsing errors? Or reject? For now, log and continue listening.
                log.warn("Ignoring non-JSON WebSocket message:", data.toString(), err);
                return; 
            }

            // Check if the message type matches the expected type, if provided
            if (expectedResponseType === null || (receivedData && receivedData.type === expectedResponseType)) {
                clearTimeout(timer);
                ws.off('message', onMessage); // Clean up listener after finding the right message
                log.debug(`Accepted message with type '${receivedData?.type || 'unknown'}'`);
                resolve(receivedData);
            } else {
                // Log ignored messages if needed
                log.debug(`Ignoring message with type '${receivedData?.type || 'unknown'}', waiting for '${expectedResponseType}'`);
                // Keep listening for the correct message
            }
        };

        // Use .on instead of .once to handle multiple incoming messages until the expected one arrives
        ws.on('message', onMessage);
    });
}

// Utility to disconnect WebSocket cleanly
export function disconnectWebSocket(ws: WebSocket) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        log.info(`Closing WebSocket connection to ${ws.url}`);
        ws.close();
        log.info(`WebSocket connection closed.`);
    }
    else if (ws && ws.readyState !== WebSocket.CLOSED) {
        log.warn(`WebSocket connection to ${ws.url} is not open (state: ${ws.readyState}), terminating.`);
        ws.terminate(); // Force close if not open/closed
    }
}

// --- Auto-detect Serial Port on startup ---
(async () => {
    // Prioritize manual port if set, otherwise attempt auto-detection
    if (SERIAL_PORT_MANUAL) {
        log.info(`Using manually configured serial port: ${SERIAL_PORT_MANUAL}`);
        SERIAL_PORT_PATH = SERIAL_PORT_MANUAL;
    } else {
        SERIAL_PORT_PATH = await findSerialPort();
    }
})(); 
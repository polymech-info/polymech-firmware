import { describe, it, expect, beforeAll, afterAll } from 'vitest';
import fs from 'fs';
import { SerialPort } from 'serialport';
import { 
    SERIAL_PORT_PATH, 
    createSerialClient,
    connectSerial,
    disconnectSerial,
    sendSerialCommandAndReceive,
    createReport, 
    logToReport, 
    closeReport 
} from './commons';

// Test Configuration
const TEST_NAME = "Serial List Components";
const COMMAND_PAYLOAD = "<<1;2;64;list:1:0>>"; // Newline added by helper
const RESPONSE_TIMEOUT = 8000; // Allow 3 seconds for the list command response

describe(TEST_NAME, () => {
    let port: SerialPort;
    let reportStream: fs.WriteStream;
    let serialPath: string;

    beforeAll(async () => {
        reportStream = createReport(TEST_NAME);
        logToReport(reportStream, "Starting test setup...", 'debug');
        
        // Wait a moment for commons.ts async init (port detection/setting)
        // Adjust if COM port isn't ready immediately after script starts.
        if (!SERIAL_PORT_PATH) {
             logToReport(reportStream, "Waiting a bit for serial port path initialization...", 'debug');
             await new Promise(resolve => setTimeout(resolve, 500)); 
        }

        serialPath = SERIAL_PORT_PATH || ''; // Use the path from commons.ts

        if (!serialPath) {
            const errorMsg = "Serial port path (SERIAL_PORT_PATH) not initialized in commons.ts.";
            logToReport(reportStream, errorMsg, 'error');
            throw new Error(errorMsg);
        }
        logToReport(reportStream, `Using serial port from commons: ${serialPath}`, 'info');

        port = createSerialClient(serialPath); 
        logToReport(reportStream, "Serial client created.", 'debug');
        
        try {
            await connectSerial(port);
            logToReport(reportStream, "Serial port connected successfully during setup.", 'info');
            // Keep a small delay just in case
            logToReport(reportStream, "Waiting 1s for device stabilization...", 'debug');
            await new Promise(resolve => setTimeout(resolve, 1000)); 
        } catch (error) {
            logToReport(reportStream, `Serial connection failed during setup: ${error}`, 'error');
            throw new Error('Serial connection failed during setup');
        }
        logToReport(reportStream, "Test setup complete.", 'debug');
    });

    afterAll(() => {
        logToReport(reportStream, "Starting test teardown.", 'debug');
        disconnectSerial(port);
        closeReport(reportStream);
    });

    it('should receive a non-empty component list', async () => {
        logToReport(reportStream, `Sending command: ${COMMAND_PAYLOAD}`, 'info');
        
        try {
            const response = await sendSerialCommandAndReceive(port, COMMAND_PAYLOAD, RESPONSE_TIMEOUT);
            logToReport(reportStream, `Received raw response:\n---\n${response}\n---`, 'debug');

            // Basic assertion: Check if we received *something*
            expect(response).toBeDefined();
            expect(response.trim().length).toBeGreaterThan(0);

            // Optional: Add more specific checks, e.g., check for known component names
            // expect(response).toContain('System'); 
            // expect(response).toContain('RS485');

            logToReport(reportStream, `Assertion passed: Received non-empty response (length: ${response.trim().length}).`, 'info');

        } catch (error: any) {
            logToReport(reportStream, `Error during serial communication: ${error.message || error}`, 'error');
            if (error.stack) {
                logToReport(reportStream, `Stack trace: ${error.stack}`, 'debug');
            }
            expect.fail(`Test failed due to serial communication error: ${error}`);
        }
    }, RESPONSE_TIMEOUT + 1000); // Vitest timeout slightly longer than response timeout
}); 
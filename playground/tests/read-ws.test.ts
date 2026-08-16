import { describe, it, expect, beforeAll, afterAll } from 'vitest';
import fs from 'fs';
import WebSocket from 'ws';
import { 
    WEBSOCKET_URL,
    createWebSocketClient,
    connectWebSocket,
    disconnectWebSocket,
    sendWsMessageAndReceive,
    createReport, 
    logToReport, 
    closeReport 
} from './commons';

// Test Configuration
const TEST_NAME = "WebSocket Read Registers";
const COMMAND_PAYLOAD = { command: "get_registers", id: 0 };

describe(TEST_NAME, () => {
    let ws: WebSocket;
    let reportStream: fs.WriteStream;

    beforeAll(async () => {
        reportStream = createReport(TEST_NAME);
        logToReport(reportStream, "Creating WebSocket client...", 'debug');
        ws = createWebSocketClient(WEBSOCKET_URL);
        logToReport(reportStream, "WebSocket client created.", 'debug');
        // Connect during setup to ensure availability for tests
        try {
            await connectWebSocket(ws);
            logToReport(reportStream, "WebSocket connected successfully during setup.", 'info');
        } catch (error) {
            logToReport(reportStream, `WebSocket connection failed during setup: ${error}`, 'error');
            // Optionally throw error to prevent tests from running if connection is critical
            throw new Error('WebSocket connection failed during setup');
        }
        logToReport(reportStream, "Test setup complete.", 'debug');
    });

    afterAll(() => {
        logToReport(reportStream, "Starting test teardown.", 'debug');
        disconnectWebSocket(ws);
        closeReport(reportStream);
    });

    it('should receive a register list response', async () => {
        logToReport(reportStream, `Sending command: ${JSON.stringify(COMMAND_PAYLOAD)}`, 'info');
        
        try {
            // Specify the expected response type
            const response = await sendWsMessageAndReceive(ws, COMMAND_PAYLOAD, 'registers');
            logToReport(reportStream, `Received response: ${JSON.stringify(response, null, 2)}`, 'debug');

            // Assert response structure
            expect(response).toBeDefined();
            expect(response.type).toBe('registers');
            expect(response.data).toBeDefined();
            expect(Array.isArray(response.data), 'Expected response.data to be an array').toBe(true);

            logToReport(reportStream, `Assertion passed: Received response with type 'registers' and data array (length: ${response.data.length}).`, 'info');

        } catch (error: any) {
            logToReport(reportStream, `Error during WebSocket communication: ${error.message || error}`, 'error');
            if (error.stack) {
                logToReport(reportStream, `Stack trace: ${error.stack}`, 'debug');
            }
            expect.fail(`Test failed due to WebSocket communication error: ${error}`);
        }
    });
}); 
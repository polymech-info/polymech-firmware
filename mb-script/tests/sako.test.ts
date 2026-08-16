import { describe, it, expect, beforeAll, afterAll } from 'vitest';
import fs from 'fs';
import { 
    ESP32_IP, 
    MODBUS_PORT, 
    createModbusClient, 
    connectModbus, 
    disconnectModbus, 
    createReport, 
    logToReport, 
    closeReport 
} from './commons';
import type { Socket } from 'net';
import type ModbusTCPClient from 'jsmodbus/dist/modbus-tcp-client';

// Test Configuration
const COMPONENT_KEY_SAKO_VFD = 750; // From enums.h
const SAKO_MB_TCP_OFFSET = COMPONENT_KEY_SAKO_VFD * 10;
const SAKO_VFD_TCP_REG_RANGE = 16; // From SAKO_VFD.h
const SAKO_SLAVE_ID_FOR_TEST = 0; // Assuming slaveId 0 for the base SAKO VFD TCP block

const TEST_NAME = "SAKO VFD Read TCP Registers";
// The SAKO VFD exposes its registers at SAKO_MB_TCP_OFFSET + (slaveId * SAKO_VFD_TCP_REG_RANGE).
// The readable offsets are 1 to SAKO_VFD_TCP_REG_RANGE.
// So, for slaveId 0, the first register is at SAKO_MB_TCP_OFFSET + 1.
const START_ADDRESS = SAKO_MB_TCP_OFFSET + (SAKO_SLAVE_ID_FOR_TEST * SAKO_VFD_TCP_REG_RANGE) + 1;
const COUNT = SAKO_VFD_TCP_REG_RANGE; // Read the entire range of registers for one SAKO instance

describe(TEST_NAME, () => {
    let client: ModbusTCPClient;
    let socket: Socket;
    let reportStream: fs.WriteStream;

    beforeAll(() => {
        reportStream = createReport(TEST_NAME);
        const { socket: modbusSocket, client: modbusClient } = createModbusClient();
        socket = modbusSocket;
        client = modbusClient;
        logToReport(reportStream, "Test setup complete.", 'debug');
        logToReport(reportStream, `Targeting SAKO VFD registers: START_ADDRESS=${START_ADDRESS}, COUNT=${COUNT}`, 'info');
    });

    afterAll(() => {
        logToReport(reportStream, "Starting test teardown.", 'debug');
        disconnectModbus(socket);
        closeReport(reportStream);
    });

    it('should connect to the Modbus server', async () => {
        logToReport(reportStream, `Attempting connection to ${ESP32_IP}:${MODBUS_PORT}...`, 'info');
        await expect(connectModbus(socket, ESP32_IP, MODBUS_PORT)).resolves.toBeUndefined();
        logToReport(reportStream, "Connection successful.", 'info');
    });

    it(`should read ${COUNT} holding registers for SAKO VFD starting from address ${START_ADDRESS}`, async () => {
        logToReport(reportStream, `Attempting to read ${COUNT} registers from address ${START_ADDRESS}...`, 'info');
        
        try {
            const response = await client.readHoldingRegisters(START_ADDRESS, COUNT);
            logToReport(reportStream, `Raw response:\\n\`\`\`json\\n${JSON.stringify(response, null, 2)}\\n\`\`\``, 'debug');
            
            expect(response.response.body.values.length).toBe(COUNT);
            logToReport(reportStream, `Successfully read ${response.response.body.values.length} registers.`, 'info');
            
            // Log the read values
            const values = response.response.body.values;
            logToReport(reportStream, `Register values: [${values.join(', ')}]`, 'info');

            // TODO: Add more specific assertions based on expected SAKO VFD values if possible.
            // For now, we are just checking if the read was successful and the count is correct.
            // The default assertion "expect(values.every(val => val === 0)).toBe(true)" is removed
            // as SAKO VFD registers will likely not all be zero.

            logToReport(reportStream, "Assertion passed: Correct number of registers read.", 'info');

        } catch (error: any) {
            logToReport(reportStream, `Error reading SAKO VFD registers: ${error.message || error}`, 'error');
            if (error.stack) {
                logToReport(reportStream, `Stack trace: ${error.stack}`, 'debug');
            }
            expect.fail(`Failed to read SAKO VFD holding registers: ${error}`);
        }
    });
}); 
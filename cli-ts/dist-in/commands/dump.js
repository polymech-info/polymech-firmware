import { log } from "../lib/logger.js";
import path from "path";
import fs from "fs/promises";
import { sync as write } from "@polymech/fs/write";
import modbusApiService from "@polymech/client-ts/modbusApiService";
import { resolve } from "@polymech/commons/variables";
async function dumpModbusData(targetDir) {
    try {
        log.info(`Fetching all registers...`);
        const registers = await modbusApiService.getRegisters();
        const filePath = path.resolve(resolve("${POLYMECH_ROOT}/site2/src/content/resources/cassandra/registers.json"));
        await write(filePath, JSON.stringify(registers, null, 2));
        log.info(`Saved registers.json to ${filePath}`);
    }
    catch (error) {
        if (error instanceof Error) {
            log.error(`Failed to dump registers: ${error.message}`);
        }
        else {
            log.error(`Failed to dump registers:`, error);
        }
    }
    try {
        log.info(`Fetching all coils...`);
        const coils = await modbusApiService.getCoils();
        const filePath = path.resolve(resolve("${POLYMECH_ROOT}/site2/src/content/resources/cassandra/coils.json"));
        await write(filePath, JSON.stringify(coils, null, 2));
        log.info(`Saved coils.json to ${filePath}`);
    }
    catch (error) {
        if (error instanceof Error) {
            log.error(`Failed to dump coils: ${error.message}`);
        }
        else {
            log.error(`Failed to dump coils:`, error);
        }
    }
}
export const command = "dump";
export const describe = "Dumps JSON data from API endpoints to files.";
export const builder = {
    targethost: {
        describe: "The target host (e.g., http://192.168.1.250)",
        demandOption: true,
        type: "string",
        default: "http://192.168.1.250",
    },
    directory: {
        describe: "Target directory to save JSON files",
        default: "../cassandra-rc2/templates/cassandra/",
        type: "string",
    },
};
const endpoints = [
    { path: "/v1/profiles", filename: "default_profiles.json" },
    { path: "/v1/settings", filename: "settings.json" },
    { path: "/v1/signalplots", filename: "signals_plots.json" },
    { path: "/v1/fs?file=layout.json", filename: "layout.json" },
    { path: "/v1/network/settings", filename: "network.json" },
    { path: "/v1/state", filename: "state.json" },
    { path: "/v1/pressure-profiles", filename: "pressure_profiles.json" },
];
async function fetchEndpoint(baseUrl, endpoint) {
    const url = `${baseUrl}${endpoint}`;
    const response = await fetch(url, {
        method: 'GET',
        headers: {
            'Content-Type': 'application/json',
        },
    });
    if (!response.ok) {
        let errorBody = '';
        try {
            errorBody = await response.text();
        }
        catch (e) {
            // Ignore
        }
        throw new Error(`Failed to fetch ${endpoint}: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }
    return await response.json();
}
async function testConnection(baseUrl) {
    try {
        const response = await fetch(`${baseUrl}/v1/system/info`);
        return response.ok;
    }
    catch (error) {
        return false;
    }
}
export async function handler(argv) {
    log.info(`Dumping data from host: ${argv.targethost}`);
    let baseUrl = argv.targethost;
    baseUrl = baseUrl.endsWith('/') ? baseUrl.slice(0, -1) : baseUrl;
    if (!baseUrl.endsWith('/api')) {
        baseUrl = `${baseUrl}/api`;
    }
    try {
        modbusApiService.setBaseUrl(baseUrl);
        const isConnected = await testConnection(baseUrl);
        if (!isConnected) {
            log.error(`Failed to connect to ${argv.targethost}. Please check the host address and network connection.`);
            return;
        }
        log.info(`Successfully connected to ${argv.targethost}.`);
        const targetDir = path.resolve(process.cwd(), argv.directory);
        try {
            await fs.mkdir(targetDir, { recursive: true });
        }
        catch (error) {
            // Directory might already exist
        }
        for (const endpoint of endpoints) {
            try {
                log.info(`Fetching ${endpoint.path}...`);
                const data = await fetchEndpoint(baseUrl, endpoint.path);
                const filePath = path.join(targetDir, endpoint.filename);
                await write(filePath, JSON.stringify(data, null, 2));
                log.info(`Saved ${endpoint.filename} to ${filePath}`);
            }
            catch (error) {
                if (error instanceof Error) {
                    log.error(`Failed to dump ${endpoint.path}: ${error.message}`);
                }
                else {
                    log.error(`Failed to dump ${endpoint.path}:`, error);
                }
            }
        }
        await dumpModbusData(targetDir);
        log.info("Dump completed.");
    }
    catch (error) {
        if (error instanceof Error) {
            log.error(`An error occurred: ${error.message}`);
        }
        else {
            log.error("An unknown error occurred.", error);
        }
    }
}
//# sourceMappingURL=dump.js.map
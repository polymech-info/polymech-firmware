import { log } from "../lib/logger.js";
import path from "path";
import fs from "fs/promises";
import modbusApiService from "@polymech/client-ts/modbusApiService";
export const command = "restore";
export const describe = "Restores JSON data from files to API endpoints via filesystem upload.";
export const builder = {
    targethost: {
        describe: "The target host (e.g., http://192.168.1.250)",
        demandOption: true,
        type: "string",
        default: "http://192.168.1.250",
    },
    directory: {
        describe: "Source directory to read JSON files",
        default: "./data",
        type: "string",
    },
};
const endpoints = [
    { local: "default_profiles.json", remote: "profile_defaults.json" },
    { local: "settings.json", remote: "settings.json" },
    { local: "signals_plots.json", remote: "signal_plots.json" },
    { local: "layout.json", remote: "layout.json" },
    { local: "network.json", remote: "network.json" },
    { local: "state.json", remote: "state.json" },
    { local: "pressure_profiles.json", remote: "pressure_profiles.json" },
];
async function uploadFile(baseUrl, filename, content) {
    const url = `${baseUrl}/v1/fs`;
    const response = await fetch(url, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            filename: filename,
            content: content
        }),
    });
    if (!response.ok) {
        let errorBody = '';
        try {
            errorBody = await response.text();
        }
        catch (e) {
            // Ignore
        }
        throw new Error(`Failed to upload ${filename}: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }
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
    log.info(`Restoring data to host: ${argv.targethost}`);
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
        const sourceDir = path.resolve(process.cwd(), argv.directory);
        log.info(`Reading files from: ${sourceDir}`);
        try {
            await fs.access(sourceDir);
        }
        catch (error) {
            log.error(`Source directory does not exist: ${sourceDir}`);
            return;
        }
        for (const endpoint of endpoints) {
            try {
                const filePath = path.join(sourceDir, endpoint.local);
                log.info(`Reading ${endpoint.local}...`);
                let fileContent;
                try {
                    fileContent = await fs.readFile(filePath, 'utf-8');
                }
                catch (e) {
                    log.warn(`Skipping ${endpoint.local}: File not found.`);
                    continue;
                }
                log.info(`Uploading to /${endpoint.remote}...`);
                const uploadResult = await uploadFile(baseUrl, endpoint.remote, fileContent);
                log.info(`Successfully restored ${endpoint.local} as /${endpoint.remote}`);
            }
            catch (error) {
                if (error instanceof Error) {
                    log.error(`Failed to restore ${endpoint.local}: ${error.message}`);
                }
                else {
                    log.error(`Failed to restore ${endpoint.local}:`, error);
                }
            }
        }
        log.info("Restore completed.");
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
//# sourceMappingURL=restore.js.map
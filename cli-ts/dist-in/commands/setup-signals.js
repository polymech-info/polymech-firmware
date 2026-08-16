import { signalPlotsSchema } from "../schemas/signal-plots-schema.js";
import { log } from "../lib/logger.js";
import path from "path";
import fs from "fs/promises";
export const command = "setup-signals";
export const describe = "Sets up signal plots for a target host.";
export const builder = {
    targethost: {
        describe: "The target host (e.g., http://192.168.1.250)",
        demandOption: true,
        type: "string",
        default: "http://192.168.1.250",
    },
    file: {
        describe: "Path to the signal plots file",
        default: "signal_plots.json",
        type: "string",
    },
};
async function setSignalPlots(baseUrl, settings) {
    const url = `${baseUrl}/v1/signalplots`;
    const response = await fetch(url, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(settings),
    });
    if (!response.ok) {
        let errorBody = '';
        try {
            errorBody = await response.text();
        }
        catch (e) {
            // Ignore
        }
        throw new Error(`Failed to set signal plots: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }
    try {
        return await response.json();
    }
    catch (e) {
        return { success: true, message: 'Signal plots updated.' };
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
    log.info(`Setting up signal plots for host: ${argv.targethost}`);
    let baseUrl = argv.targethost;
    baseUrl = baseUrl.endsWith('/') ? baseUrl.slice(0, -1) : baseUrl;
    if (!baseUrl.endsWith('/api')) {
        baseUrl = `${baseUrl}/api`;
    }
    try {
        const isConnected = await testConnection(baseUrl);
        if (!isConnected) {
            log.error(`Failed to connect to ${argv.targethost}. Please check the host address and network connection.`);
            return;
        }
        log.info(`Successfully connected to ${argv.targethost}.`);
        const filePath = path.resolve(process.cwd(), argv.file);
        const fileContent = await fs.readFile(filePath, "utf-8");
        const settings = JSON.parse(fileContent);
        log.info("Validating signal plots...");
        const validationResult = signalPlotsSchema.safeParse(settings);
        if (!validationResult.success) {
            log.error("Invalid signal plots:", validationResult.error.flatten());
            return;
        }
        log.info(`Validation successful. Applying signal plots to ${baseUrl}`);
        const res = await setSignalPlots(baseUrl, validationResult.data);
        log.info(`Signal plots applied successfully: \n\n`, { message: res.message });
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
//# sourceMappingURL=setup-signals.js.map
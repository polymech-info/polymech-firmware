import { networkSettingsSchema } from "../schemas/network-schema.js";
import type { NetworkSettingsUpdatePayload } from "../types.js";
import { log } from "../lib/logger.js";
import path from "path";
import fs from "fs/promises";

export const command = "setup-network";
export const describe = "Sets up network settings for a target host.";

const networkOptions = Object.keys(networkSettingsSchema.shape).reduce((acc, key) => {
    acc[key] = {
        describe: `Overrides ${key} from the settings file`,
        type: 'string',
        demandOption: false,
    };
    return acc;
}, {} as { [key: string]: { describe: string; type: 'string'; demandOption: false } });


export const builder = {
    targethost: {
        describe: "The target host (e.g., http://192.168.1.250)",
        demandOption: true,
        type: "string",
        default: "http://192.168.1.250",
    },
    file: {
        describe: "Path to the network settings file",
        default: "network.json",
        type: "string",
    },
    ...networkOptions,
} as const;

async function setNetworkSettings(baseUrl: string, settings: NetworkSettingsUpdatePayload): Promise<any> {
    const url = `${baseUrl}/v1/network/settings`;
    log.info(`Setting network settings to ${url}`);
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
        } catch (e) {
            // Ignore
        }
        throw new Error(`Failed to set network settings: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
    }
    try {
        return await response.json();
    } catch (e) {
        return { success: true, message: 'Network settings updated.' };
    }
}

async function testConnection(baseUrl: string): Promise<boolean> {
    try {
        const response = await fetch(`${baseUrl}/v1/system/info`);
        return response.ok;
    } catch (error) {
        return false;
    }
}


export async function handler(argv: { targethost: string; file: string;[key: string]: any }) {
    log.info(`Setting up network for host: ${argv.targethost}`);
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
        const settingsFromFile: NetworkSettingsUpdatePayload = JSON.parse(fileContent);

        const cliOverrides = Object.keys(networkSettingsSchema.shape).reduce((acc, key) => {
            if (argv[key] !== undefined && argv[key] !== null) {
                acc[key as keyof NetworkSettingsUpdatePayload] = argv[key];
            }
            return acc;
        }, {} as Partial<NetworkSettingsUpdatePayload>);

        const settings = { ...settingsFromFile, ...cliOverrides };

        log.info("Validating network settings...", cliOverrides);
        const validationResult = networkSettingsSchema.safeParse(settings);

        if (!validationResult.success) {
            log.error("Invalid network settings:", validationResult.error.flatten());
            return;
        }

        log.info(`Validation successful. Applying settings to ${baseUrl}`);
        const res = await setNetworkSettings(baseUrl, validationResult.data);
        log.info(`Network settings applied successfully: \n\n`, { message: res.message });

    } catch (error) {
        if (error instanceof Error) {
            log.error(`An error occurred: ${error.message}`);
        } else {
            log.error("An unknown error occurred.", error);
        }
    }
} 
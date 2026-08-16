import { settingsSchema } from "../schemas/settings-schema.js";
import type { SettingsUpdatePayload } from "../types.js";
import { log } from "../lib/logger.js";
import path from "path";
import fs from "fs/promises";


export const command = "setup-settings";
export const describe = "Sets up settings for a target host.";

export const builder = {
  targethost: {
    describe: "The target host (e.g., http://192.168.1.250)",
    demandOption: true,
    type: "string",
    default: "http://192.168.1.250",
  },
  file: {
    describe: "Path to the settings file",
    default: "settings.json",
    type: "string",
  },
} as const;

async function setSettings(
  baseUrl: string,
  settings: SettingsUpdatePayload,
): Promise<any> {
  const url = `${baseUrl}/v1/settings`;
  const response = await fetch(url, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(settings),
  });

  if (!response.ok) {
    let errorBody = "";
    try {
      errorBody = await response.text();
    } catch (e) {
      // Ignore
    }
    throw new Error(
      `Failed to set settings: ${response.statusText} ${errorBody ? `- ${errorBody}` : ""
      }`,
    );
  }
  try {
    return await response.json();
  } catch (e) {
    return { success: true, message: "Settings updated." };
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
  log.info(`Setting up settings for host: ${argv.targethost}`);
  let baseUrl = argv.targethost;
  baseUrl = baseUrl.endsWith("/") ? baseUrl.slice(0, -1) : baseUrl;
  if (!baseUrl.endsWith("/api")) {
    baseUrl = `${baseUrl}/api`;
  }

  try {
    const isConnected = await testConnection(baseUrl);
    if (!isConnected) {
      log.error(
        `Failed to connect to ${argv.targethost}. Please check the host address and network connection.`,
      );
      return;
    }
    log.info(`Successfully connected to ${argv.targethost}.`);

    const filePath = path.resolve(process.cwd(), argv.file);
    const fileContent = await fs.readFile(filePath, "utf-8");
    const settings: SettingsUpdatePayload = JSON.parse(fileContent);

    log.info("Validating settings...");
    const validationResult = settingsSchema.safeParse(settings);

    if (!validationResult.success) {
      log.error("Invalid settings:", validationResult.error.flatten());
      return;
    }

    log.info(`Validation successful. Applying settings to ${baseUrl}`);
    const res = await setSettings(baseUrl, validationResult.data);
    log.info(`Settings applied successfully: \n\n`, { message: res.message });
  } catch (error) {
    if (error instanceof Error) {
      log.error(`An error occurred: ${error.message}`);
    } else {
      log.error("An unknown error occurred.", error);
    }
  }
} 
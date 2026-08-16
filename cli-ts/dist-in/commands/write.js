import { Logger } from "tslog";
import path from "path";
import fs from "fs/promises";
const log = new Logger();
export const command = "write";
export const describe = "Writes a JSON file to the device filesystem using the /fs API.";
export const builder = {
    targethost: {
        describe: "The target host (e.g., http://192.168.1.250)",
        demandOption: true,
        type: "string",
        default: "http://192.168.1.250",
    },
    file: {
        describe: "Path to the local JSON file to write",
        demandOption: true,
        type: "string",
    },
    filename: {
        describe: "Target filename on the device (defaults to basename of file)",
        type: "string",
    },
};
async function writeFileToDevice(baseUrl, filename, content) {
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
        throw new Error(`Failed to write file: ${response.statusText} ${errorBody ? `- ${errorBody}` : ''}`);
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
    log.info(`Writing file to device: ${argv.targethost}`);
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
        try {
            await fs.access(filePath);
        }
        catch (error) {
            log.error(`File not found: ${filePath}`);
            return;
        }
        const fileContent = await fs.readFile(filePath, "utf-8");
        log.info(`Read file content (${fileContent.length} bytes)`);
        let parsedJson;
        try {
            parsedJson = JSON.parse(fileContent);
            log.info(`JSON parsed successfully. Keys: ${Object.keys(parsedJson).join(', ')}`);
        }
        catch (error) {
            log.error(`Invalid JSON file: ${filePath}. Error: ${error}`);
            return;
        }
        const targetFilename = argv.filename || path.basename(argv.file);
        log.info(`Writing ${targetFilename} to device...`);
        log.info(`Payload preview: ${JSON.stringify({ filename: targetFilename, content: fileContent.substring(0, 200) + '...' })}`);
        const result = await writeFileToDevice(baseUrl, targetFilename, fileContent);
        if (result.success) {
            log.info(`File written successfully: ${result.message}`);
        }
        else {
            log.error(`Failed to write file: ${result.error}`);
        }
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
//# sourceMappingURL=write.js.map
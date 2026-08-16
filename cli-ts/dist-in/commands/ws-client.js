import WebSocket from 'ws';
import { performance } from 'perf_hooks';
import { decodeNetworkValue } from '../lib/pb-decoder.js';
import * as fs from 'fs';
import * as path from 'path';
import { JSONPath } from 'jsonpath-plus';
export const command = 'ws-client';
export const describe = 'Connects to the device WebSocket server.';
const queryPresets = {
    coils: '$[?(@.type=="coil_update")]',
    registers: '$[?(@.type=="register_update")]',
};
export const builder = (yargs) => yargs
    .option('url', {
    alias: 'u',
    type: 'string',
    description: 'WebSocket URL to connect to',
    default: 'ws://192.168.1.250/ws',
})
    .option('timeout', {
    alias: 't',
    type: 'number',
    description: 'Disconnect after N seconds. 0 for no timeout.',
    default: 0,
})
    .option('dest', {
    alias: 'd',
    type: 'string',
    description: 'Path to a JSON file to save messages to.',
    default: './last-ws.json'
})
    .option('query', {
    alias: 'q',
    type: 'string',
    description: 'JSONPath query to filter messages. Presets: coils, registers.',
    coerce: (arg) => queryPresets[arg] || arg,
})
    .option('verbose', {
    alias: 'v',
    type: 'boolean',
    description: 'Show full message payloads in the console.',
    default: false,
})
    .example('$0 ws-client -q coils', 'Show only coil update messages.')
    .example('$0 ws-client -q \'$[?(@.address==1003)]\'', '')
    .example('$0 ws-client -q \'$[?(@.data.address==1002)]\'', '')
    .example('$0 ws-client -q \'$[?(@.type=="register_update" && @.data.address==1002)]\'', '');
export const handler = (argv) => {
    const { url, timeout, dest, query, verbose } = argv;
    const destPath = path.resolve(dest);
    let messages = [];
    // Load existing messages if the file exists and is valid JSON
    try {
        if (fs.existsSync(destPath)) {
            const fileContent = fs.readFileSync(destPath, 'utf-8');
            const existingData = JSON.parse(fileContent);
            if (Array.isArray(existingData)) {
                messages = existingData;
                console.log(`Loaded ${messages.length} existing messages from ${destPath}`);
            }
        }
    }
    catch (e) {
        console.log(`Could not parse existing destination file ${destPath}. Starting fresh.`);
        messages = [];
    }
    const saveAndExit = () => {
        try {
            fs.writeFileSync(destPath, JSON.stringify(messages, null, 2));
            console.log(`\nMessages saved to ${destPath}`);
        }
        catch (error) {
            console.error(`\nFailed to write messages to ${destPath}:`, error);
        }
        process.exit(0);
    };
    const connect = () => {
        console.log(`Connecting to ${url}...`);
        const ws = new WebSocket(url);
        if (timeout > 0) {
            console.log(`Will disconnect in ${timeout} seconds.`);
            setTimeout(() => {
                console.log('Timeout reached. Disconnecting.');
                ws.close();
            }, timeout * 1000);
        }
        ws.on('open', () => {
            console.log('Connected to WebSocket server.');
        });
        ws.on('message', (data, isBinary) => {
            let messageData;
            if (isBinary) {
                const buffer = data;
                const startTime = performance.now();
                const decoded = decodeNetworkValue(buffer);
                const endTime = performance.now();
                const duration = (endTime - startTime) * 1000;
                messageData = decoded;
            }
            else {
                const jsonStr = data.toString();
                try {
                    const message = JSON.parse(jsonStr);
                    if (message.type === 'welcome') {
                        console.log(`Successfully connected. Client ID: ${message.clientId}`);
                        return; // Don't process welcome message further
                    }
                    messageData = message;
                }
                catch (e) {
                    messageData = { type: 'raw', payload: jsonStr };
                }
            }
            if (messageData) {
                messages.push(messageData);
                if (query) {
                    try {
                        const result = JSONPath({ path: query, json: [messageData] });
                        if (result.length > 0) {
                            if (verbose)
                                console.log(JSON.stringify(result[0], null, 2));
                        }
                    }
                    catch (e) {
                        // Ignore errors for messages that don't match the query structure
                    }
                }
                else {
                    if (verbose)
                        console.log(JSON.stringify(messageData, null, 2));
                }
            }
        });
        ws.on('close', () => {
            console.log('Disconnected.');
            saveAndExit();
        });
        ws.on('error', (error) => {
            console.error('WebSocket error:', error.message);
            ws.close();
        });
    };
    // Ensure file is saved on exit signals
    process.on('SIGINT', saveAndExit);
    process.on('SIGTERM', saveAndExit);
    connect();
};
//# sourceMappingURL=ws-client.js.map
import WebSocket from 'ws';
import { performance } from 'perf_hooks';
export const command = 'test-web';
export const describe = 'Simulates the web app behavior: connects to WebSocket and fetches registers via REST.';
export const builder = (yargs) => yargs
    .option('wsUrl', {
    alias: 'w',
    type: 'string',
    description: 'WebSocket URL to connect to',
    default: 'ws://192.168.1.250/ws',
})
    .option('apiUrl', {
    alias: 'a',
    type: 'string',
    description: 'Base API URL',
    default: 'http://192.168.1.250/api',
})
    .option('interval', {
    alias: 'i',
    type: 'number',
    description: 'Interval between register fetches in ms',
    default: 30,
})
    .option('payloadTest', {
    type: 'boolean',
    description: 'Run WebSocket payload size test',
    default: true,
})
    .option('payloadSize', {
    type: 'number',
    description: 'Size of payload to request in bytes',
    default: 1024,
})
    .option('period', {
    alias: 'p',
    type: 'number',
    description: 'Period between payload requests in ms',
    default: 150,
});
export const handler = async (argv) => {
    const { wsUrl, apiUrl, interval, payloadTest, payloadSize, period } = argv;
    console.log(`Starting test-web...`);
    console.log(`WebSocket URL: ${wsUrl}`);
    console.log(`API URL: ${apiUrl}`);
    // 1. Connect to WebSocket
    const ws = new WebSocket(wsUrl);
    ws.on('open', () => {
        console.log('WebSocket connected.');
    });
    ws.on('message', (data) => {
        // Just consume messages to keep the connection active and simulate traffic processing
        // console.log('WS Message received:', data.toString().length, 'bytes');
        if (payloadTest) {
            try {
                const msg = JSON.parse(data.toString());
                if (msg.type === 'test_payload') {
                    console.log(`Received test payload: ${msg.data.size} bytes requested, payload length: ${msg.data.payload.length}`);
                }
            }
            catch (e) {
                // ignore
            }
        }
    });
    ws.on('close', (code, reason) => {
        console.log(`WebSocket disconnected. Code: ${code}, Reason: ${reason.toString()}`);
        process.exit(0);
    });
    ws.on('error', (error) => {
        console.error('WebSocket error:', error.message);
        console.error('WebSocket readyState:', ws.readyState);
    });
    ws.on('ping', (data) => {
        console.log('WS received ping:', data.toString());
    });
    ws.on('pong', (data) => {
        console.log('WS received pong:', data.toString());
    });
    // 2. Fetch Registers via REST (simulating modbusApiService.ts)
    const fetchRegisters = async () => {
        console.log('Starting register fetch...');
        let currentPage = 0;
        let totalPages = 1;
        const pageSize = 10;
        let totalRegisters = 0;
        while (currentPage < totalPages) {
            const startTime = performance.now();
            try {
                const response = await fetch(`${apiUrl}/v1/registers?page=${currentPage}&pageSize=${pageSize}`);
                if (!response.ok) {
                    throw new Error(`HTTP error ${response.status}: ${response.statusText}`);
                }
                const data = await response.json();
                if (data.registers) {
                    totalRegisters += data.registers.length;
                }
                if (data.meta) {
                    totalPages = data.meta.totalPages;
                }
                else {
                    break;
                }
                const duration = performance.now() - startTime;
                console.log(`Fetched page ${currentPage}/${totalPages} in ${duration.toFixed(2)}ms`);
            }
            catch (error) {
                console.error(`Failed to fetch page ${currentPage}:`, error);
            }
            currentPage++;
            if (interval > 0) {
                await new Promise(resolve => setTimeout(resolve, interval));
            }
        }
        console.log(`Finished fetching ${totalRegisters} registers.`);
    };
    // await fetchRegisters();
    if (payloadTest) {
        ws.on('open', () => {
            console.log(`Starting periodic payload test (size: ${payloadSize}, period: ${period}ms)...`);
            setInterval(() => {
                if (ws.readyState === WebSocket.OPEN) {
                    console.log('Sending payload test... ' + payloadSize + ' bytes');
                    ws.send(JSON.stringify({ command: 'test_payload', size: payloadSize }));
                }
                else {
                    console.log('WebSocket is not open. Payload test skipped.');
                }
            }, period);
        });
    }
    else {
        // Start fetching registers immediately
    }
    // Keep process alive
    process.stdin.resume();
};
//# sourceMappingURL=test-web.js.map
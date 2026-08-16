import { 
    SystemInfo, 
    NetworkSettingsResponse, 
    NetworkSettingsUpdatePayload, 
    PlungerSettingsResponse, 
    PlungerSettingsUpdatePayload, 
    Profile, 
    ProfilesResponse, 
    ProfileSavePayload, 
    SignalPlotData, 
    RegisteredMethod, 
    SerialCommandPayload, 
    SerialCommandResponse,
    SSignalControlPoint
} from '../types';


class ModbusApiServiceMocked {
    private baseUrl: string = 'http://mocked.api/api';
    private isConnected: boolean = true;
    private mockDelayMs = 500;

    private mockSystemInfo: SystemInfo = {
        version: '1.0.0-mock',
        board: 'MockBoard Rev.M',
        uptime: 12345678,
        timestamp: Date.now(),
        freeHeapKb: 1024,
        maxFreeBlockKb: 512,
        cpuTicks: 98765432,
        loopDurationMs: 10
    };

    private mockCoils: CoilResponse[] = Array.from({ length: 100 }, (_, i) => ({
        address: i,
        value: Math.random() > 0.5,
        name: `MockCoil ${i}`,
        id: i,
        type: 1,
        access: 3,
        flags: 0,
        group: 'Default'
    }));

    private mockRegisters: RegisterResponse[] = Array.from({ length: 200 }, (_, i) => ({
        address: i,
        value: Math.floor(Math.random() * 1000),
        name: `MockRegister ${i}`,
        id: `mock_reg_${i}`,
        type: 3, 
        access: 3, 
        flags: 0,
        group: 'Default'
    }));
    
    private mockProfiles: Profile[] = [
        { 
            slot: 1, 
            name: "Mock Profile Alpha", 
            description: "A mock profile for testing",
            duration: 3600, 
            status: 1, 
            currentTemp: 25, 
            max: 100, 
            controlPoints: [{x:0, y:20}, {x:1800, y:80}, {x:3600, y:20}],
            targetRegisters: [] 
        },
        { 
            slot: 2, 
            name: "Mock Profile Beta", 
            description: "Another mock profile",
            duration: 7200, 
            status: 0, 
            currentTemp: 22, 
            max: 120, 
            controlPoints: [{x:0, y:25}, {x:3600, y:100}, {x:7200, y:25}],
            targetRegisters: [] 
        },
    ];

    private mockNetworkSettings: NetworkSettingsResponse = {
        sta_ssid: "MockWiFi",
        sta_local_ip: "192.168.1.100",
        sta_gateway: "192.168.1.1",
        sta_subnet: "255.255.255.0",
        sta_primary_dns: "8.8.8.8",
        sta_secondary_dns: "8.8.4.4",
        ap_ssid: "MockDeviceAP",
        ap_config_ip: "10.0.0.1",
        ap_config_gateway: "10.0.0.1",
        ap_config_subnet: "255.255.255.0"
    };

    private mockPlungerSettings: PlungerSettingsResponse = {
        speedSlowHz: 10,
        speedMediumHz: 50,
        speedFastHz: 100,
        speedFillPlungeHz: 80,
        speedFillHomeHz: 60,
        currentJamThresholdMa: 500,
        jammedDurationHomingMs: 1000,
        jammedDurationMs: 2000,
        autoModeHoldDurationMs: 500,
        maxUniversalJamTimeMs: 5000,
        fillJoystickHoldDurationMs: 300,
        fillPlungedWaitDurationMs: 200,
        fillHomedWaitDurationMs: 200,
        recordHoldDurationMs: 1000,
        maxRecordDurationMs: 60000,
        replayDurationMs: 60000,
        enablePostFlow: true,
        postFlowDurationMs: 5000,
        postFlowSpeedHz: 20,
        currentPostFlowMa: 300,
        postFlowStoppingWaitMs: 100,
        postFlowCompleteWaitMs: 100,
        defaultMaxOperationDurationMs: 120000
    };
    
    private mockSignalPlots: SignalPlotData[] = [
        {
            name: "Mock Signal Plot 1",
            duration: 60000,
            slot: 1,
            controlPoints: [
                { id: 1, time: 0, state: 0, type: 0, arg_0: 10, arg_1: 0, name: 'Start' } as SSignalControlPoint,
                { id: 2, time: 30000, state: 1, type: 1, arg_0: 50, arg_1: 10, name: 'Midpoint' } as SSignalControlPoint,
                { id: 3, time: 60000, state: 0, type: 0, arg_0: 10, arg_1: 0, name: 'End' } as SSignalControlPoint,
            ]
        }
    ];

    private mockRegisteredMethods: RegisteredMethod[] = [
        { id: 1, component: "MainApp", method: "reset" },
        { id: 2, component: "MainApp", method: "getStatus" },
        { id: 3, component: "Plunger", method: "moveHome" },
        { id: 4, component: "Plunger", method: "movePlunge" },
    ];


    constructor() {
        console.log('Using Mocked Modbus API Service');
    }

    private delay<T>(data: T): Promise<T> {
        return new Promise(resolve => setTimeout(() => resolve(data), this.mockDelayMs));
    }

    setBaseUrl(url: string): void {
        console.log(`[Mock] setBaseUrl called with: ${url}`);
        url = url.endsWith('/') ? url.slice(0, -1) : url;
        if (!url.endsWith('/api')) {
            url = `${url}/api`;
        }
        this.baseUrl = url;
    }

    getBaseUrl(): string {
        return this.baseUrl;
    }

    async testConnection(): Promise<boolean> {
        console.log('[Mock] testConnection called');
        this.isConnected = true; 
        return this.delay(this.isConnected);
    }

    async getSystemInfo(): Promise<SystemInfo> {
        console.log('[Mock] getSystemInfo called');
        return this.delay({ ...this.mockSystemInfo, uptime: this.mockSystemInfo.uptime + Math.floor(Math.random()*1000), timestamp: Date.now() });
    }

    async getCoils(start: number = 0, count: number = 500): Promise<CoilResponse[]> {
        console.log(`[Mock] getCoils called with start: ${start}, count: ${count}`);
        this.mockCoils = this.mockCoils.map(coil => ({...coil, value: Math.random() > 0.5}));
        const paginatedCoils = this.mockCoils.slice(start, start + count);
        return this.delay(paginatedCoils);
    }

    async getCoil(address: number): Promise<boolean> {
        console.log(`[Mock] getCoil called for address: ${address}`);
        const coil = this.mockCoils.find(c => c.address === address);
        if (coil) {
            return this.delay(coil.value);
        }
        throw new Error(`[Mock] Coil at address ${address} not found`);
    }

    async setCoil(address: number, value: boolean): Promise<CoilUpdateResponse> {
        console.log(`[Mock] setCoil called for address: ${address}, value: ${value}`);
        const coilIndex = this.mockCoils.findIndex(c => c.address === address);
        if (coilIndex !== -1) {
            this.mockCoils[coilIndex].value = value;
            return this.delay({ success: true, address, value });
        }
        throw new Error(`[Mock] Failed to set coil at address ${address}`);
    }

    async getRegisters(start: number = 0, count: number = 1000): Promise<RegisterResponse[]> {
        console.log(`[Mock] getRegisters called with start: ${start}, count: ${count}`);
        this.mockRegisters = this.mockRegisters.map(reg => ({...reg, value: Math.floor(Math.random() * 1000)}));
        const paginatedRegisters = this.mockRegisters.slice(start, start + count);
        return this.delay(paginatedRegisters);
    }

    async getRegister(address: number): Promise<number> {
        console.log(`[Mock] getRegister called for address: ${address}`);
        const register = this.mockRegisters.find(r => r.address === address);
        if (register) {
            return this.delay(register.value);
        }
        throw new Error(`[Mock] Register at address ${address} not found`);
    }

    async getLogs(): Promise<string[]> {
        console.log('[Mock] getLogs called');
        const mockLogs = [
            `[${new Date().toISOString()}] [Mock FW] System initialized.`,
            `[${new Date().toISOString()}] [Mock FW] Mock operation started.`,
            `[${new Date().toISOString()}] [Mock FW] Random value: ${Math.random()}`,
        ];
        return this.delay(mockLogs);
    }

    async setRegister(address: number, value: number): Promise<RegisterUpdateResponse> {
        console.log(`[Mock] setRegister called for address: ${address}, value: ${value}`);
        const regIndex = this.mockRegisters.findIndex(r => r.address === address);
        if (regIndex !== -1) {
            this.mockRegisters[regIndex].value = value;
            return this.delay({ success: true, address, value });
        }
        throw new Error(`[Mock] Failed to set register at address ${address}`);
    }

    getConnectionStatus(): boolean {
        console.log('[Mock] getConnectionStatus called');
        return this.isConnected;
    }

    async getNetworkSettings(): Promise<NetworkSettingsResponse> {
        console.log('[Mock] getNetworkSettings called');
        return this.delay(this.mockNetworkSettings);
    }

    async setNetworkSettings(settings: NetworkSettingsUpdatePayload): Promise<any> {
        console.log('[Mock] setNetworkSettings called with:', settings);
        this.mockNetworkSettings = { ...this.mockNetworkSettings, ...settings };
        return this.delay({ success: true, message: 'Network settings updated (mocked).' });
    }

    async getPlungerSettings(): Promise<PlungerSettingsResponse> {
        console.log('[Mock] getPlungerSettings called');
        return this.delay(this.mockPlungerSettings);
    }

    async setPlungerSettings(settings: PlungerSettingsUpdatePayload): Promise<any> {
        console.log('[Mock] setPlungerSettings called with:', settings);
        this.mockPlungerSettings = { ...this.mockPlungerSettings, ...settings };
        return this.delay({ success: true, message: 'Plunger settings updated (mocked).' });
    }

    async loadPlungerDefaults(): Promise<any> {
        console.log('[Mock] loadPlungerDefaults called');
        this.mockPlungerSettings = { 
            speedSlowHz: 10, speedMediumHz: 50, speedFastHz: 100, speedFillPlungeHz: 80, speedFillHomeHz: 60,
            currentJamThresholdMa: 500, jammedDurationHomingMs: 1000, jammedDurationMs: 2000, autoModeHoldDurationMs: 500,
            maxUniversalJamTimeMs: 5000, fillJoystickHoldDurationMs: 300, fillPlungedWaitDurationMs: 200,
            fillHomedWaitDurationMs: 200, recordHoldDurationMs: 1000, maxRecordDurationMs: 60000, replayDurationMs: 60000,
            enablePostFlow: true, postFlowDurationMs: 5000, postFlowSpeedHz: 20, currentPostFlowMa: 300,
            postFlowStoppingWaitMs: 100, postFlowCompleteWaitMs: 100, defaultMaxOperationDurationMs: 120000
        };
        return this.delay({ success: true, message: 'Plunger defaults loaded (mocked).' });
    }

    async getProfiles(): Promise<ProfilesResponse> {
        console.log('[Mock] getProfiles called');
        return this.delay({ profiles: this.mockProfiles });
    }

    async saveProfile(profileData: ProfileSavePayload): Promise<any> {
        console.log('[Mock] saveProfile called with:', profileData);
        if (profileData.slot !== undefined) {
            const index = this.mockProfiles.findIndex(p => p.slot === profileData.slot);
            if (index !== -1) {
                this.mockProfiles[index] = { ...this.mockProfiles[index], ...profileData, status: this.mockProfiles[index].status, currentTemp: this.mockProfiles[index].currentTemp };
            } else {
                const newSlot = Math.max(0, ...this.mockProfiles.map(p => p.slot)) + 1;
                this.mockProfiles.push({ ...profileData, slot: newSlot, status: 0, currentTemp: 20 } as Profile);
            }
        } else {
            const newSlot = Math.max(0, ...this.mockProfiles.map(p => p.slot)) + 1;
            this.mockProfiles.push({ ...profileData, slot: newSlot, status: 0, currentTemp: 20 } as Profile);
        }
        return this.delay({ success: true, message: `Profile '${profileData.name}' saved (mocked).` });
    }

    async testRelays(): Promise<RelayTestResponse> {
        console.log('[Mock] testRelays called');
        return this.delay({ success: true, message: 'Relay test successful (mocked).' });
    }
    
    async getSignalPlots(): Promise<SignalPlotData[]> {
        console.log('[Mock] getSignalPlots called');
        return this.delay(this.mockSignalPlots.map(plot => ({...plot, controlPoints: plot.controlPoints.map(cp => ({...cp})) })));
    }

    async saveSignalPlot(signalPlotData: SignalPlotData): Promise<any> {
        console.log('[Mock] saveSignalPlot called with:', signalPlotData);
        const index = this.mockSignalPlots.findIndex(p => p.slot === signalPlotData.slot);
        if (index !== -1) {
            this.mockSignalPlots[index] = signalPlotData;
        } else {
            this.mockSignalPlots.push(signalPlotData);
        }
        return this.delay({ success: true, message: `Signal plot '${signalPlotData.name}' saved (mocked).` });
    }

    async deleteSignalPlot(slot: number): Promise<any> {
        console.log(`[Mock] deleteSignalPlot called for slot: ${slot}`);
        this.mockSignalPlots = this.mockSignalPlots.filter(p => p.slot !== slot);
        return this.delay({ success: true, message: `Signal plot at slot ${slot} deleted (mocked).` });
    }

    async getRegisteredMethods(): Promise<RegisteredMethod[]> {
        console.log('[Mock] getRegisteredMethods called');
        return this.delay(this.mockRegisteredMethods);
    }

    async sendSerialCommand(payload: SerialCommandPayload): Promise<SerialCommandResponse> {
        const command = `<<${payload.componentId};${payload.callType};${payload.flags};${payload.method}:${payload.arg1 ?? 0}:${payload.arg2 ?? 0}>>`;
        console.log(`[Mock] sendSerialCommand called with command: ${command}`, payload);

        if (payload.method === 'reset' && payload.componentId === 1) {
            console.log('[Mock] Simulating system reset...');
            this.mockSystemInfo.uptime = 0; 
            return this.delay({ success: true, message: 'System reset (mocked).', result: { status: 'OK' }});
        }
        if (payload.method === 'getStatus' && payload.componentId === 1) {
            return this.delay({ success: true, message: 'Status OK (mocked).', result: { heap: this.mockSystemInfo.freeHeapKb, uptime: this.mockSystemInfo.uptime }});
        }

        return this.delay({ 
            success: true, 
            message: `Command '${command}' processed (mocked).`,
            result: { mockData: "some_result_from_mock" }
        });
    }
}

const modbusApiServiceMocked = new ModbusApiServiceMocked();
export default modbusApiServiceMocked;

import { 
    E_FN_CODE, 
    E_ModbusAccess,
    WsStatus,
    type CoilData,
    type RegisterData,
    type RegisterUpdatePayload,
    type CoilUpdatePayload,
    type StatusChangeCallback,   
    type LogMessageCallback,
    type RegisterDataCallback,
    type RegisterUpdateCallback,
    type CoilDataCallback,
    type CoilUpdateCallback,
    type PaginatedRegistersResponse

} from '../types';

// Using a hardcoded value for the mock, as @/constants might not resolve in all contexts for the mock file itself.
const MOCK_WS_RECONNECT_INTERVAL_MS = 5000;


class ModbusServiceMocked {
  private ws: WebSocket | null = null; 
  private wsUrl: string = '';
  private status: WsStatus = 'DISCONNECTED';
  private messageIdCounter: number = 0;
  private pendingRequests: Map<number, { resolve: Function; reject: Function; command: string; data?: any;}> = new Map();
  private reconnectTimer: NodeJS.Timeout | null = null;
  private isIntentionalDisconnect: boolean = false;
  private mockDelayMs = 300; 

  private mockCoils: CoilData[] = [];
  private mockRegisters: RegisterData[] = [];
  private mockLogs: string[] = [];

  private onStatusChange: StatusChangeCallback = () => {};
  private onLogMessage: LogMessageCallback = () => {};
  private onRegisterData: RegisterDataCallback = () => {};
  private onRegisterUpdate: RegisterUpdateCallback = () => {};
  private onCoilData: CoilDataCallback = () => {};
  private onCoilUpdate: CoilUpdateCallback = () => {};

  constructor() {
    console.log('Using Mocked Modbus WebSocket Service');
    this.initializeMockData();
  }

  private initializeMockData(): void {
    this.mockCoils = Array.from({ length: 50 }, (_, i) => ({
      address: i,
      value: Math.random() > 0.5,
      name: `MockWSCoil ${i}`,
      id: i,
      group: 'Default',
      type: E_FN_CODE.FN_READ_COIL,
      access: E_ModbusAccess.MB_ACCESS_READ_WRITE,
      flags: 0,
    }));

    this.mockRegisters = Array.from({ length: 100 }, (_, i) => ({
      address: i,
      value: Math.floor(Math.random() * 65535),
      name: `MockWSReg ${i}`,
      id: `mock_ws_reg_${i}`,
      type: E_FN_CODE.FN_READ_HOLD_REGISTER,
      access: E_ModbusAccess.MB_ACCESS_READ_WRITE,
      flags: 0,
      group: 'Sensors',
      component: 'TempSensor',
    }));
    this.mockLogs = ['[MockWS] Service Initialized'];
  }
  
  private delay(ms?: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms ?? this.mockDelayMs));
  }

  private updateStatus(newStatus: WsStatus): void {
    if (this.status !== newStatus) {
      this.status = newStatus;
      console.log(`[MockWS] Status changed to: ${newStatus}`);
      this.onStatusChange(newStatus);
      if (newStatus === 'CONNECTED') {
        this.sendWelcomeMessage();
      }
    }
  }

  private sendWelcomeMessage(): void {
    this.onLogMessage({ type: 'welcome', message: 'Welcome to Mock Modbus WebSocket Server!', timestamp: Date.now()});
  }
  
  private rejectPendingRequests(reason: string): void {
    this.pendingRequests.forEach((req) => {
      req.reject(new Error(`[MockWS] ${req.command} failed: ${reason}`));
    });
    this.pendingRequests.clear();
  }

  public async connect(
    wsUrl: string,
    onStatusChange: StatusChangeCallback,
    onLogMessage: LogMessageCallback,
    onRegisterData: RegisterDataCallback,
    onRegisterUpdate: RegisterUpdateCallback,
    onCoilData?: CoilDataCallback, 
    onCoilUpdate?: CoilUpdateCallback
  ): Promise<void> {
    console.log(`[MockWS] Connect requested to ${wsUrl}`);
    if (this.status === 'CONNECTED' || this.status === 'CONNECTING' || this.status === 'RECONNECTING') {
      if(this.status === 'CONNECTED') {
        console.log('[MockWS] Already connected.');
        return Promise.resolve();
      }
      console.log(`[MockWS] Connection attempt while status is ${this.status}.`);
    }

    this.wsUrl = wsUrl;
    this.onStatusChange = onStatusChange;
    this.onLogMessage = onLogMessage;
    this.onRegisterData = onRegisterData;
    this.onRegisterUpdate = onRegisterUpdate;
    if (onCoilData) this.onCoilData = onCoilData;
    if (onCoilUpdate) this.onCoilUpdate = onCoilUpdate;
    this.isIntentionalDisconnect = false;

    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
    return this.attemptConnection();
  }

  private async attemptConnection(): Promise<void> {
    this.updateStatus(this.status === 'ERROR' || this.status === 'DISCONNECTED' ? 'CONNECTING' : 'RECONNECTING');
    await this.delay(500);

    if (Math.random() > 0.1) { 
      this.updateStatus('CONNECTED');
      this.onLogMessage('[MockWS] Successfully connected.');
      this.pushFullCoilData();
      this.pushFullRegisterDataPaginated(); 
      return Promise.resolve();
    } else {
      this.onLogMessage('[MockWS] Connection attempt failed.');
      this.updateStatus('ERROR');
      if (!this.isIntentionalDisconnect) {
        this.scheduleReconnect();
      }
      return Promise.reject(new Error('[MockWS] Failed to connect (simulated)'));
    }
  }

  public disconnect(intentional: boolean = true): void {
    console.log(`[MockWS] Disconnect requested (intentional: ${intentional})`);
    this.isIntentionalDisconnect = intentional;
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
    this.rejectPendingRequests('WebSocket disconnected intentionally by client.');
    this.updateStatus('DISCONNECTED');
    this.ws = null; 
    this.onLogMessage('[MockWS] Disconnected.');
  }

  private scheduleReconnect(): void {
    if (this.reconnectTimer) clearTimeout(this.reconnectTimer);
    console.log(`[MockWS] Scheduling reconnect in ${MOCK_WS_RECONNECT_INTERVAL_MS / 1000}s`);
    this.reconnectTimer = setTimeout(() => {
      this.attemptConnection();
    }, MOCK_WS_RECONNECT_INTERVAL_MS);
  }

  public getConnectionStatus(): WsStatus {
    return this.status;
  }

  private pushFullCoilData(): void {
    if (this.status === 'CONNECTED' && this.onCoilData) {
      console.log('[MockWS] Pushing full coil data update.');
      this.onCoilData([...this.mockCoils]);
    }
  }

  private pushCoilUpdate(address: number, value: boolean): void {
    if (this.status === 'CONNECTED' && this.onCoilUpdate) {
      console.log(`[MockWS] Pushing coil update for address ${address}.`);
      this.onCoilUpdate({ address, value, message: 'Coil update received from server' });
    }
  }
  
  private async pushFullRegisterDataPaginated(pageSize: number = 20): Promise<void> {
    if (this.status !== 'CONNECTED' || !this.onRegisterData) return;

    console.log('[MockWS] Pushing paginated full register data.');
    const totalRegisters = this.mockRegisters.length;
    const totalPages = Math.ceil(totalRegisters / pageSize);

    for (let page = 0; page < totalPages; page++) {
        // Loop remains for conceptual pagination, actual data push is one full array.
    }
    this.onRegisterData([...this.mockRegisters]);
  }


  private pushRegisterUpdate(payload: RegisterUpdatePayload): void {
    if (this.status === 'CONNECTED' && this.onRegisterUpdate) {
      console.log(`[MockWS] Pushing register update for address ${payload.address}.`);
      this.onRegisterUpdate(payload);
    }
  }

  private pushLogMessage(message: string, level: string = 'Info'): void {
     if (this.status === 'CONNECTED' && this.onLogMessage) {
        this.onLogMessage({
            type: 'log_entry', 
            level: level,
            message: `[MockWS Server] ${message}`,
            timestamp: Date.now()
        });
    }
  }

  public registerCoilHandlers(onCoilData: CoilDataCallback, onCoilUpdate: CoilUpdateCallback): void {
    this.onCoilData = onCoilData;
    this.onCoilUpdate = onCoilUpdate;
  }

  async getRegisters(initialPageNum: number = 0, requestedPageSize: number = 20 ): Promise<RegisterData[]> {
    console.log(`[MockWS] getRegisters called (page: ${initialPageNum}, size: ${requestedPageSize})`);
    if (this.status !== 'CONNECTED') {
      throw new Error('[MockWS] Not connected. Cannot fetch registers.');
    }
    await this.delay();
    this.pushLogMessage('Full register list requested by client.');
    this.onRegisterData([...this.mockRegisters]); 
    return Promise.resolve([...this.mockRegisters]);
  }

  async writeRegister(address: number, value: number): Promise<void> {
    console.log(`[MockWS] writeRegister called for address ${address}, value ${value}`);
    if (this.status !== 'CONNECTED') {
      throw new Error('[MockWS] Not connected. Cannot write register.');
    }
    
    const messageId = this.messageIdCounter++;
    const promise = new Promise<void>((resolve, reject) => {
        this.pendingRequests.set(messageId, { resolve, reject, command: 'write_register', data: { address, value } });
    });

    await this.delay();

    const regIndex = this.mockRegisters.findIndex(r => r.address === address);
    if (regIndex !== -1) {
      this.mockRegisters[regIndex].value = value;
      this.pushLogMessage(`Register ${address} updated to ${value} by client request.`);
      this.pushRegisterUpdate({ slaveId: 0, address, value, fc: E_FN_CODE.FN_WRITE_HOLD_REGISTER });
      
      const request = this.pendingRequests.get(messageId);
      if (request) {
          request.resolve(); 
          this.pendingRequests.delete(messageId);
      }
      return Promise.resolve();
    } else {
      this.pushLogMessage(`Write to non-existent register ${address} failed.`);
      const request = this.pendingRequests.get(messageId);
       if (request) {
          request.reject(new Error(`[MockWS] Register ${address} not found.`));
          this.pendingRequests.delete(messageId);
      }
      throw new Error(`[MockWS] Register ${address} not found.`);
    }
  }
  
  async requestLogs(): Promise<void> {
    console.log('[MockWS] requestLogs called');
    if (this.status !== 'CONNECTED') {
       console.warn('[MockWS] Cannot request logs: Not connected.');
       return Promise.resolve();
    }
    await this.delay();
    this.pushLogMessage('Client requested logs. Sending current buffer.');
    if (this.onLogMessage) {
        this.onLogMessage({ type: 'logs', data: [...this.mockLogs, `[MockWS Server] Log buffer as of ${new Date().toLocaleTimeString()}`] });
    }
    return Promise.resolve();
  }
  
  async requestCoils(): Promise<void> {
    console.log('[MockWS] requestCoils called');
    if (this.status !== 'CONNECTED') {
      throw new Error('[MockWS] Not connected. Cannot request coils.');
    }
    await this.delay();
    this.pushLogMessage('Client requested full coil data.');
    this.pushFullCoilData();
    return Promise.resolve();
  }

  async writeCoil(address: number, value: boolean): Promise<void> {
    console.log(`[MockWS] writeCoil called for address ${address}, value ${value}`);
    if (this.status !== 'CONNECTED') {
      throw new Error('[MockWS] Not connected. Cannot write coil.');
    }

    const messageId = this.messageIdCounter++;
    const promise = new Promise<void>((resolve, reject) => {
        this.pendingRequests.set(messageId, { resolve, reject, command: 'write_coil', data: { address, value } });
    });
    
    await this.delay();

    const coilIndex = this.mockCoils.findIndex(c => c.address === address);
    if (coilIndex !== -1) {
      this.mockCoils[coilIndex].value = value;
      this.pushLogMessage(`Coil ${address} updated to ${value} by client request.`);
      this.pushCoilUpdate(address, value);

      const request = this.pendingRequests.get(messageId);
      if (request) {
          request.resolve();
          this.pendingRequests.delete(messageId);
      }
      return Promise.resolve();
    } else {
      this.pushLogMessage(`Write to non-existent coil ${address} failed.`);
       const request = this.pendingRequests.get(messageId);
       if (request) {
          request.reject(new Error(`[MockWS] Coil ${address} not found.`));
          this.pendingRequests.delete(messageId);
      }
      throw new Error(`[MockWS] Coil ${address} not found.`);
    }
  }

  public simulateRandomRegisterChange(): void {
    if (this.status === 'CONNECTED' && this.mockRegisters.length > 0) {
      const randomIndex = Math.floor(Math.random() * this.mockRegisters.length);
      const reg = this.mockRegisters[randomIndex];
      const oldValue = reg.value;
      reg.value = Math.floor(Math.random() * 65535);
      this.pushLogMessage(`Register ${reg.address} changed from ${oldValue} to ${reg.value} (simulated server-side change).`, 'Debug');
      this.pushRegisterUpdate({
        slaveId: 0,
        address: reg.address,
        value: reg.value,
        fc: reg.type,
      });
    }
  }

  public simulateRandomCoilChange(): void {
    if (this.status === 'CONNECTED' && this.mockCoils.length > 0) {
      const randomIndex = Math.floor(Math.random() * this.mockCoils.length);
      const coil = this.mockCoils[randomIndex];
      coil.value = !coil.value;
      this.pushLogMessage(`Coil ${coil.address} value flipped to ${coil.value} (simulated server-side change).`, 'Debug');
      this.pushCoilUpdate(coil.address, coil.value);
    }
  }
  
  public simulateNewLogFromServer(message: string, level: string = 'Info'): void {
    if (this.status === 'CONNECTED') {
      this.mockLogs.push(`[MockWS Server Log ${level}] ${message}`);
      this.pushLogMessage(message, level);
    }
  }
}

const modbusServiceMocked = new ModbusServiceMocked();
export default modbusServiceMocked;

// Example of how to use simulation methods (e.g., in a useEffect in a debug component)
// if (modbusServiceMocked.getConnectionStatus() === 'CONNECTED') {
//   setInterval(() => {
//     modbusServiceMocked.simulateRandomRegisterChange();
//     modbusServiceMocked.simulateRandomCoilChange();
//     if (Math.random() < 0.2) {
//        modbusServiceMocked.simulateNewLogFromServer(`Random event occurred: ${Math.random().toString(36).substring(7)}`, 'Trace');
//     }
//   }, 5000);
// } 
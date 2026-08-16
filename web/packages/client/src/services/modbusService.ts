import { WS_HEARTBEAT_INTERVAL_MS, WS_RECONNECT_INTERVAL_MS } from '../constants.js';

const getLogLevelConsoleColor = (level: string): string => {
  switch (level) {
    case 'Fatal':
    case 'Error':
      return 'color: #ed4e4c; font-weight: bold;';
    case 'Warning':
      return 'color: #d2c057;';
    case 'Info':
      return 'color: #2774f0;';
    case 'Debug':
      return 'color: #01c800;';
    case 'Verbose':
      return 'color: #a142f4;';
    case 'Trace':
      return 'color: #898989;';
    default:
      return 'color: #cfd0d0;';
  }
};

let currentLogGroup = '';

const logToConsole = (log: any, levelMap: { [key: number]: string }) => {
  const { level, message, name, id, timestamp } = log;
  const levelStr = (typeof level === 'number' ? levelMap[level] : level) || 'Unknown';

  if (levelStr === 'Silent') return;

  const time = new Date(timestamp || Date.now()).toLocaleTimeString('en-GB');
  const levelColor = getLogLevelConsoleColor(levelStr);
  const componentInfo = name ? `${name}${id !== undefined ? `#${id}` : ''}` : 'System';

  // Only create a new group if the component has changed
  if (componentInfo !== currentLogGroup) {
    if (currentLogGroup !== '') {
      console.groupEnd(); // Close previous group
    }
    console.group(`%c[${componentInfo}]`, 'color: #e59e66; font-weight: bold;');
    currentLogGroup = componentInfo;
  }

  console.log(
    `%c${time} %c[${levelStr.toUpperCase()}] %c${message}`,
    'color: grey;',
    levelColor,
    'color: inherit;'
  );
};

import {
  type RegisterData,
  type RegisterUpdatePayload,
  type CoilData,
  type StatusChangeCallback,
  type LogMessageCallback,
  type RegisterDataCallback,
  type RegisterUpdateCallback,
  type CoilDataCallback,
  type CoilUpdateCallback,
  type WsStatus,
  type PaginatedRegistersResponse,
  type SystemInfo,
  E_FN_CODE,
  BinaryWebSocketMessageType
} from '../types';

export type DisplayMessagePayload = {
  id: string | number;
  message: string;
  timestamp: number;
};

export type DisplayMessageCallback = (payload: DisplayMessagePayload) => void;
export type SystemInfoCallback = (info: SystemInfo) => void;
export type PendingOpsCallback = (operations: any[]) => void;
export type RTUQueueCallback = (queueData: any) => void;
export type RTUStatsCallback = (statsData: any) => void;

enum LogLevel {
  SILENT = 0,
  FATAL,
  ERROR,
  WARNING,
  INFO,
  TRACE,
  VERBOSE
};

const LOG_LEVEL_MAP: { [key in LogLevel]: string } = {
  [LogLevel.SILENT]: 'Silent',
  [LogLevel.FATAL]: 'Fatal',
  [LogLevel.ERROR]: 'Error',
  [LogLevel.WARNING]: 'Warning',
  [LogLevel.INFO]: 'Info',
  [LogLevel.TRACE]: 'Trace',
  [LogLevel.VERBOSE]: 'Verbose',
};

class ModbusService {
  private ws: WebSocket | null = null;
  private wsUrl: string = '';
  private status: WsStatus = 'DISCONNECTED';
  private messageId: number = 0;
  private pendingRequests: Map<number, {
    resolve: Function;
    reject: Function;
    command: string;
    data?: any;
  }> = new Map();
  private reconnectTimer: NodeJS.Timeout | null = null;
  private isIntentionalDisconnect: boolean = false;
  private isFetchingAllRegisters: boolean = false;
  private heartbeatTimer: NodeJS.Timeout | null = null;
  private lastMessageTimestamp: number = 0;

  // Callbacks for the UI layer
  private onStatusChange: StatusChangeCallback = () => { };
  private onLogMessage: LogMessageCallback = () => { };
  private onRegisterData: RegisterDataCallback = () => { };
  private onRegisterUpdate: RegisterUpdateCallback = () => { };
  private onCoilData: CoilDataCallback = () => { };
  private onCoilUpdate: CoilUpdateCallback = () => { };
  private onDisplayMessage: DisplayMessageCallback = () => { };
  private onSystemInfo: SystemInfoCallback = () => { };
  private onPendingOps: PendingOpsCallback = () => { };
  private onRTUQueue: RTUQueueCallback = () => { };
  private onRTUStats: RTUStatsCallback = () => { };

  constructor() { }

  // --- Connection Management ---
  public connect(
    wsUrl: string,
    onStatusChange: StatusChangeCallback,
    onLogMessage: LogMessageCallback,
    onRegisterData: RegisterDataCallback,
    onRegisterUpdate: RegisterUpdateCallback,
    onCoilData?: CoilDataCallback,
    onCoilUpdate?: CoilUpdateCallback,
    onDisplayMessage?: DisplayMessageCallback,
    onSystemInfo?: SystemInfoCallback,
  ): Promise<boolean> {
    if (this.ws && (this.status === 'CONNECTED' || this.status === 'CONNECTING' || this.status === 'RECONNECTING')) {
      if (this.status === 'CONNECTED') {
        console.log('WebSocket already connected.');
        return Promise.resolve(false);
      }
    }

    this.wsUrl = wsUrl;
    this.onStatusChange = onStatusChange;
    this.onLogMessage = onLogMessage;
    this.onRegisterData = onRegisterData;
    this.onRegisterUpdate = onRegisterUpdate;
    if (onCoilData) this.onCoilData = onCoilData;
    if (onCoilUpdate) this.onCoilUpdate = onCoilUpdate;
    if (onDisplayMessage) this.onDisplayMessage = onDisplayMessage;
    if (onSystemInfo) this.onSystemInfo = onSystemInfo;
    this.isIntentionalDisconnect = false;

    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }

    return this.attemptConnection().then(() => true);
  }

  private attemptConnection(): Promise<void> {
    return new Promise((resolve, reject) => {
      // If we already have a valid/active connection or an ongoing connection attempt, abort this attempt
      if (this.ws) {
        const isOpen = this.ws.readyState === WebSocket.OPEN && this.status === 'CONNECTED';
        const isConnecting = this.ws.readyState === WebSocket.CONNECTING && (this.status === 'CONNECTING' || this.status === 'RECONNECTING');
        if (isOpen || isConnecting) {
          console.log('[ModbusService] Existing WebSocket is in a valid state (', this.ws.readyState, '/', this.status, '). Aborting new connection attempt.');
          resolve();
          return;
        }
      }

      if (!this.wsUrl) {
        console.error('WebSocket URL is not set.');
        this.updateStatus('ERROR');
        reject(new Error('WebSocket URL is not set.'));
        return;
      }

      if (this.ws) {
        this.ws.onopen = null;
        this.ws.onmessage = null;
        this.ws.onerror = null;
        this.ws.onclose = null;
        if (this.ws.readyState !== WebSocket.CLOSED && this.ws.readyState !== WebSocket.CLOSING) {
          console.log('[ModbusService] Closing existing WebSocket before new attempt.');
          this.ws.close();
        }
        this.ws = null;
      }

      const connectingState = (this.status === 'DISCONNECTED' || this.status === 'ERROR') ? 'CONNECTING' : 'RECONNECTING';
      this.updateStatus(connectingState);
      console.log(`Attempting WebSocket connection to ${this.wsUrl}...`);

      try {
        const newWs = new WebSocket(this.wsUrl);
        newWs.binaryType = 'arraybuffer'; // Enable binary messages
        this.ws = newWs;

        newWs.onopen = () => {
          console.log('WebSocket Connected!');
          this.updateStatus('CONNECTED');
          if ((window as any).markAppAsReady) {
            (window as any).markAppAsReady();
          }
          if (this.reconnectTimer) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
          }
          this.startHeartbeat();
          resolve();
        };

        newWs.onmessage = (event) => {
          this.lastMessageTimestamp = Date.now();
          if (event.data instanceof ArrayBuffer) {
            this.handleBinaryMessage(event.data);
          } else {
            this.handleMessage(event.data);
          }
        };
        newWs.onerror = (event: Event) => {
          console.error('WebSocket Error:', event);
          this.onSocketClosed(newWs, reject, event);
        }

        newWs.onclose = (event: Event) => this.onSocketClosed(newWs, reject, event);

      } catch (error) {

        console.error(`Failed to create WebSocket: ${error instanceof Error ? error.message : String(error)}`);
        this.updateStatus('ERROR');
        this.ws = null;
        reject(error);
      }
    });
  }

  private onSocketClosed(wsInstance: WebSocket, reject: (reason?: any) => void, event: Event) {
    if (this.ws !== wsInstance) {
      return;
    }

    const wasConnected = this.status === 'CONNECTED';

    let reason = 'Connection closed';
    let code: number | undefined;

    if (event instanceof CloseEvent) {
      reason = event.reason || 'No reason specified';
      code = event.code;
      console.log(`WebSocket Disconnected: ${reason} (Code: ${code})`);
    } else {
      console.error('WebSocket Error:', event);
      reason = 'Connection error';
    }

    this.ws = null;

    this.stopHeartbeat();
    const rejectionReason = this.isIntentionalDisconnect
      ? 'WebSocket disconnected intentionally'
      : 'WebSocket disconnected';
    this.rejectPendingRequests(rejectionReason);


    if (this.isIntentionalDisconnect) {
      this.updateStatus('DISCONNECTED');
      if (!wasConnected) {
        reject(new Error('WebSocket connection intentionally disconnected before opening.'));
      }
    } else {
      this.updateStatus('RECONNECTING');
      console.log(`WebSocket connection lost. Retrying in ${WS_RECONNECT_INTERVAL_MS / 1000} seconds...`);
      if (this.reconnectTimer) clearTimeout(this.reconnectTimer);
      this.reconnectTimer = setTimeout(() => this.attemptConnection(), WS_RECONNECT_INTERVAL_MS);

      if (!wasConnected) {
        reject(new Error(`WebSocket connection failed: ${reason}${code ? ` (Code: ${code})` : ''}`));
      }
    }
  }

  disconnect(intentional: boolean = true): void {
    console.log(`Disconnecting WebSocket (intentional: ${intentional})`);
    this.isIntentionalDisconnect = intentional;
    this.stopHeartbeat();

    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }

    if (this.ws) {
      this.ws.close();
    } else {
      console.log('No WebSocket connection to disconnect.');
      if (intentional && this.status !== 'DISCONNECTED') {
        this.updateStatus('DISCONNECTED');
      }
    }
  }

  private updateStatus(newStatus: WsStatus): void {
    if (this.status !== newStatus) {
      this.status = newStatus;
      this.onStatusChange(newStatus);
    }
  }

  private rejectPendingRequests(reason: string): void {
    this.pendingRequests.forEach((req) => {
      req.reject(new Error(`${req.command} failed: ${reason}`));
    });
    this.pendingRequests.clear();
  }

  private startHeartbeat(): Promise<void> {

    this.stopHeartbeat();
    return;
    this.lastMessageTimestamp = Date.now();
    return new Promise((resolve, reject) => {
      this.heartbeatTimer = setInterval(async () => {
        if (Date.now() - this.lastMessageTimestamp > WS_HEARTBEAT_INTERVAL_MS) {
          console.error('Heartbeat timeout. Connection lost.');
          this.disconnect(false); // Trigger reconnect
        } else {
          try {
            await this.getSystemInfo();
            resolve();
          } catch (error) {
            console.error("Heartbeat get_sysinfo failed:", error);
            reject(error);
          }

        }
      }, WS_HEARTBEAT_INTERVAL_MS);
    });
  }

  private stopHeartbeat(): void {
    if (this.heartbeatTimer) {
      clearInterval(this.heartbeatTimer);
      this.heartbeatTimer = null;
    }
  }

  private handleBinaryMessage(buffer: ArrayBuffer): void {
    const view = new DataView(buffer);
    if (view.byteLength < 1) return;

    const type = view.getUint8(0);

    if (type === BinaryWebSocketMessageType.COIL_UPDATE) {
      if (view.byteLength < 10) return;
      // Offset 1: slaveId (UInt8)
      // Offset 2: fc (UInt8)
      // Offset 3: value (UInt8) - 0 or 1
      const value = view.getUint8(3);
      // Offset 4: componentId (UInt16)
      // Offset 6: address (UInt16)
      const address = view.getUint16(6, true);
      // Offset 8: count (UInt16)

      this.onCoilUpdate({
        address: address,
        value: value === 1,
        fc: E_FN_CODE.FN_WRITE_COIL
      });
    } else if (type === BinaryWebSocketMessageType.REGISTER_UPDATE) {
      if (view.byteLength < 12) return;
      // Offset 1: slaveId (UInt8)
      // Offset 2: fc (UInt8)
      // Offset 4: componentId (UInt16)
      // Offset 6: address (UInt16)
      const address = view.getUint16(6, true);
      // Offset 8: count (UInt16)
      // Offset 10: value (UInt16)
      const regValue = view.getUint16(10, true);

      // console.log("Register update:", address, regValue);
      this.onRegisterUpdate({
        address: address,
        value: regValue,
        fc: E_FN_CODE.FN_WRITE_HOLD_REGISTER // Default to Write Hold for updates
      } as RegisterUpdatePayload);
    }
  }

  // --- Message Handling ---
  private handleMessage(data: any): void {
    try {
      let response = null

      try {
        response = JSON.parse(data);
      } catch (error) {
        // console.error('Failed to parse WebSocket message or handle it:');
        return;
      }

      if (response.id !== undefined && this.pendingRequests.has(response.id)) {
        const request = this.pendingRequests.get(response.id)!;

        if (request.command === 'get_coils' || request.command === 'get_registers') {
          // These commands' promises are resolved by specific type handlers below,
          // not by an ID in the response directly providing the full data.
          // Keep them pending for now, or if server sends ID with data, handle here.
          // If 'get_coils' or 'get_registers' commands *do* return an ID with their primary data, 
          // and that ID is meant to resolve them, this logic needs adjustment.
          // Let's assume they are resolved by type handlers like 'coils' or 'registers' that don't echo the ID.
        } else {
          this.pendingRequests.delete(response.id);
          if (response.error) {
            console.error(`[handleMessage] Error response for command ${request.command} (ID: ${response.id}):`, response.error);
            request.reject(new Error(response.error));
          } else {
            request.resolve(response.data);
          }
        }
      }
      else if (response.type === 'sysinfo' && response.data) {
        this.onSystemInfo(response.data as SystemInfo);
        // Also resolve the promise for getSystemInfo if it was sent with an ID
        for (const [id, reqDetails] of this.pendingRequests.entries()) {
          if (reqDetails.command === 'get_sysinfo') {
            reqDetails.resolve(response.data);
            this.pendingRequests.delete(id);
            break;
          }
        }
      }
      else if (response.type === 'coils' && Array.isArray(response.data)) {
        this.onCoilData(response.data as CoilData[]);
        for (const [id, reqDetails] of this.pendingRequests.entries()) {
          if (reqDetails.command === 'get_coils') {
            reqDetails.resolve(response.data); // Resolve with the coil data
            this.pendingRequests.delete(id);
            break;
          }
        }
      }
      else if (response.type === 'registers' && response.meta && typeof response.meta.totalPages === 'number' && Array.isArray(response.data)) {
        let GRPendingRequestEntry;
        let GRPendingRequestId;

        for (const [id, reqDetails] of this.pendingRequests.entries()) {
          if (reqDetails.command === 'get_registers') {
            GRPendingRequestEntry = reqDetails;
            GRPendingRequestId = id;
            break;
          }
        }

        if (GRPendingRequestEntry && GRPendingRequestId !== undefined) {
          this.pendingRequests.delete(GRPendingRequestId);
          GRPendingRequestEntry.resolve(response);
        } else {
          console.warn('[handleMessage] Received a paginated "registers" message, but no active "get_registers" request was found in pendingRequests. Calling onRegisterData with this page only.');
          this.onRegisterData(response.data as RegisterData[]);
        }
      }
      else if (response.type === 'registers' && Array.isArray(response.data)) {
        this.onRegisterData(response.data as RegisterData[]);
      }
      else if (response.type === 'logs' && Array.isArray(response.data)) {
        response.data.forEach((log: any) => logToConsole(log, LOG_LEVEL_MAP));
        this.onLogMessage(response.data);
      }
      else if (response.type === 'log_entry' && response.data) {
        logToConsole(response.data, LOG_LEVEL_MAP);
        const { level, message, id, name } = response.data;
        if (typeof level === 'number' && typeof message === 'string') {
          const levelStr = LOG_LEVEL_MAP[level as LogLevel] || 'Unknown';
          if (levelStr === 'Silent') return;

          this.onLogMessage([{
            level: levelStr,
            message: message,
            id,
            name
          } as any]);
        } else {
          console.warn('Received log_entry with invalid data format:', response.data);
        }
      }
      else if (response.type === 'register_update' && response.data) {
        if (typeof response.data.address === 'number' && typeof response.data.fc === 'number') {
          // Handle read register update (FN_READ_HOLD_REGISTER) - ignore count field
          if ((response.data.fc === E_FN_CODE.FN_READ_HOLD_REGISTER || response.data.fc === E_FN_CODE.FN_WRITE_HOLD_REGISTER) && typeof response.data.value === 'number') {
            this.onRegisterUpdate(response.data as RegisterUpdatePayload);
          }
          // Handle multiple register update (FN_WRITE_MULT_REGISTERS)
          else if (response.data.fc === E_FN_CODE.FN_WRITE_MULT_REGISTERS && Array.isArray(response.data.values) && typeof response.data.count === 'number') {
            this.onRegisterUpdate(response.data as RegisterUpdatePayload);
          }
          else {
            console.warn('Invalid register_update data for FC', response.data.fc, ':', response.data);
          }
        }
      }
      else if (response.type === 'welcome') {
        console.log('Received welcome message from server:', response);
      }
      else if (response.type === 'write_response' && response.data?.id !== undefined) {
        const writeRequestId = response.data.id;
        if (this.pendingRequests.has(writeRequestId)) {
          const request = this.pendingRequests.get(writeRequestId)!;
          if (request.command === 'write_register') {
            this.pendingRequests.delete(writeRequestId);
            const success = response.data.success === undefined ? true : response.data.success;
            if (success) {
              request.resolve();
            } else {
              console.error(`Write command (ID: ${writeRequestId}) confirmed failed by write_response:`, response.data);
              request.reject(new Error(response.data.error || 'Write failed according to write_response'));
            }
          } else {
            console.warn(`Received write_response for non-write command ID ${writeRequestId}?`, request);
          }
        } else {
          console.warn(`Received write_response for unknown or already handled ID: ${writeRequestId}`);
        }
      }
      else if (response.type === 'coil_update' && response.data) {
        if (typeof response.data.address !== 'number') {
          console.warn('Received coil_update with invalid or missing address in data:', response.data);
          return;
        }

        // Handle single coil update (acknowledged by server)
        if (response.data.fc === E_FN_CODE.FN_WRITE_COIL && typeof response.data.value === 'boolean') {
          this.onCoilUpdate(response.data);
        }
        // Handle multiple coil updates
        else if (response.data.fc === E_FN_CODE.FN_WRITE_MULT_COILS && Array.isArray(response.data.values)) {
          this.onCoilUpdate(response.data);
        }
        // Handle write_coil promise resolution from older messages
        else {
          const { address, value, id: messageId } = response.data;
          let coilValueBoolean: boolean;

          if (typeof value === 'boolean') {
            coilValueBoolean = value;
          } else if (typeof value === 'number') {
            coilValueBoolean = value !== 0;
          } else {
            // If we are here, we can't process the update for the UI, but we might still resolve a promise.
            if (messageId !== undefined) {
              const pendingRequest = this.pendingRequests.get(messageId);
              if (pendingRequest?.command === 'write_coil') {
                this.pendingRequests.delete(messageId);
                const success = response.data.success === undefined ? !response.data.error : response.data.success;
                if (success) pendingRequest.resolve();
                else pendingRequest.reject(new Error(response.data.error || `Coil write to address ${address} failed`));
              }
            }
            return; // Exit as we can't update UI state
          }

          if (messageId !== undefined) {
            const pendingRequest = this.pendingRequests.get(messageId);
            if (pendingRequest?.command === 'write_coil' && pendingRequest.data?.address === address) {
              this.pendingRequests.delete(messageId);
              const success = response.data.success === undefined ? !response.data.error : response.data.success;
              if (success) {
                pendingRequest.resolve();
              } else {
                pendingRequest.reject(new Error(response.data.error || `Coil write to address ${address} failed`));
              }
            }
          }

          // Legacy broadcast for older message formats.
          this.onCoilUpdate({
            address: address,
            value: coilValueBoolean,
            fc: E_FN_CODE.FN_WRITE_COIL
          });
        }
      }
      else if (response.type === 'user_message' && response.data) {
        if (typeof response.data.id !== 'undefined' && typeof response.data.message === 'string') {
          this.onDisplayMessage(response.data as DisplayMessagePayload);
        } else {
          console.warn('Received display_message with invalid or missing id/message in data:', response.data);
        }
      }
      else if (response.type === 'pending_ops') {
        // Handle pending operations response
        const requestId = response.id;
        if (requestId && this.pendingRequests.has(requestId)) {
          const request = this.pendingRequests.get(requestId);
          this.pendingRequests.delete(requestId);
          request.resolve(response.data);
        } else {
          // This is an unsolicited update, call the callback
          if (response.data && Array.isArray(response.data)) {
            this.onPendingOps(response.data);
          }
        }
      }
      else if (response.type === 'rtu_client_queue') {
        // Handle RTU client queue response
        const requestId = response.id;
        if (requestId && this.pendingRequests.has(requestId)) {
          const request = this.pendingRequests.get(requestId);
          this.pendingRequests.delete(requestId);
          request.resolve(response);
        } else {
          // This is an unsolicited update, call the callback
          this.onRTUQueue(response);
        }
      }
      else if (response.type === 'rtu_stats') {
        // Handle RTU stats response
        const requestId = response.id;
        if (requestId && this.pendingRequests.has(requestId)) {
          const request = this.pendingRequests.get(requestId);
          this.pendingRequests.delete(requestId);
          request.resolve(response.data);
        } else {
          // This is an unsolicited update, call the callback
          if (response.data) {
            this.onRTUStats(response.data);
          }
        }
      }
      else {
        console.warn('Received unexpected WebSocket message format or type:', response);
      }
    } catch (error) {
      console.warn('Failed to parse WebSocket message or handle it:', data, error);
    }
  }
  private sendRequest<T>(command: string, payload: object = {}): Promise<T> {
    if (!this.ws || this.status !== 'CONNECTED') {
      const errorMsg = `[sendRequest] Cannot send command '${command}': WebSocket not connected or status is '${this.status}'. WS object: ${this.ws ? 'exists' : 'null'}.`;
      console.warn(errorMsg);
      return Promise.reject(new Error(errorMsg));
    }

    return new Promise((resolve, reject) => {
      const id = this.messageId++;
      const message = { ...payload, command, id };

      try {
        if (!this.ws) {
          const errorMsg = `[sendRequest] Critical Error: WebSocket is null just before send for command '${command}', ID: ${id}.`;
          console.error(errorMsg);
          return reject(new Error(errorMsg));
        }
        this.ws.send(JSON.stringify(message));
        this.pendingRequests.set(id, { resolve, reject, command, data: payload });
      } catch (error) {
        const errorMsg = `[sendRequest] WebSocket send error for command '${command}', ID: ${id}. Error: ${error instanceof Error ? error.message : String(error)}`;
        console.error(errorMsg, error);
        reject(new Error(errorMsg));
      }
    });
  }
  // --- Public API Methods ---

  public getConnectionStatus(): WsStatus {
    return this.status;
  }

  public async getSystemInfo(): Promise<SystemInfo> {
    return this.sendRequest<SystemInfo>('get_sysinfo');
  }

  public async getPendingOps(payload: object = {}): Promise<any> {
    return this.sendRequest<any>('get_pending_ops', payload);
  }

  public async getRTUClientQueue(payload: object = {}): Promise<any> {
    return this.sendRequest<any>('get_rtu_client_queue', payload);
  }

  public async getRTUStats(payload: object = {}): Promise<any> {
    return this.sendRequest<any>('get_rtu_stats', payload);
  }

  // Generic method for sending custom websocket commands
  public async sendCommand<T>(command: string, payload: object = {}): Promise<T> {
    return this.sendRequest<T>(command, payload);
  }

  public registerCoilHandlers(onCoilData: CoilDataCallback, onCoilUpdate: CoilUpdateCallback): void {
    this.onCoilData = onCoilData;
    this.onCoilUpdate = onCoilUpdate;
  }

  public registerDebugHandlers(onPendingOps?: PendingOpsCallback, onRTUQueue?: RTUQueueCallback, onRTUStats?: RTUStatsCallback): void {
    if (onPendingOps) this.onPendingOps = onPendingOps;
    if (onRTUQueue) this.onRTUQueue = onRTUQueue;
    if (onRTUStats) this.onRTUStats = onRTUStats;
  }

  async getRegisters(
    initialPageNum: number = 0,
    requestedPageSize: number = 20
  ): Promise<RegisterData[]> {
    if (this.isFetchingAllRegisters) {
      console.warn('[getRegisters] Aborted: A full register fetch is already in progress.');
      return;
    }
    if (this.status !== 'CONNECTED') {
      console.warn('[getRegisters] Aborted: WebSocket not connected.');
      throw new Error('WebSocket not connected. Cannot fetch registers.');
    }
    this.isFetchingAllRegisters = true;
    let allRegisters: RegisterData[] = [];
    let currentPageNum = initialPageNum;
    let actualPageSize = requestedPageSize;
    try {
      const firstPageRequestPayload = { page: currentPageNum, pageSize: requestedPageSize };
      const firstPageResponse = await this.sendRequest<PaginatedRegistersResponse>('get_registers', firstPageRequestPayload);

      if (!firstPageResponse || !firstPageResponse.meta ||
        typeof firstPageResponse.meta.page !== 'number') {
        console.error('[getRegisters] Invalid response structure for the first page (missing meta or data):', firstPageResponse);
        throw new Error('Failed to fetch initial page of registers: Invalid response structure from server.');
      }

      const meta = firstPageResponse.meta;
      allRegisters = allRegisters.concat(firstPageResponse.data);
      const totalPages = meta.totalPages;
      currentPageNum = meta.page;
      actualPageSize = meta.pageSize;
      for (let pageToFetch = currentPageNum + 1; pageToFetch < totalPages; pageToFetch++) {
        const nextPageRequestPayload = { page: pageToFetch, pageSize: actualPageSize };
        const nextPageResponse = await this.sendRequest<PaginatedRegistersResponse>('get_registers', nextPageRequestPayload);

        if (!nextPageResponse || !nextPageResponse.meta || typeof nextPageResponse.meta.page !== 'number' || !Array.isArray(nextPageResponse.data)) {
          console.error(`[getRegisters] Invalid response structure for page ${pageToFetch} (missing meta or data):`, nextPageResponse);
          throw new Error(`Failed to fetch page ${pageToFetch} of registers: Invalid response structure from server.`);
        }
        allRegisters = allRegisters.concat(nextPageResponse.data);
      }

      this.onRegisterData(allRegisters);
      return allRegisters;

    } catch (error) {
      console.error('[getRegisters] Error during paginated fetching:', error instanceof Error ? error.stack : String(error), error);
      throw error;
    } finally {
      this.isFetchingAllRegisters = false;
    }
  }

  async writeRegister(address: number, value: number): Promise<void> {
    return this.sendRequest<void>('write_register', { address, value });
  }

  async requestCoils(): Promise<void> {
    if (this.status !== 'CONNECTED') {
      throw new Error('WebSocket not connected');
    }
    await this.sendRequest('get_coils');
  }

  async writeCoil(address: number, value: boolean): Promise<void> {
    if (this.status !== 'CONNECTED') {
      throw new Error('WebSocket not connected');
    }
    return this.sendRequest<void>('write_coil', { address, value });
  }

  async writeMultipleCoils(address: number, values: boolean[]): Promise<void> {
    if (this.status !== 'CONNECTED') {
      throw new Error('WebSocket not connected');
    }
    return this.sendRequest<void>('write_multiple_coils', { address, values });
  }
}
const modbusService = new ModbusService();
export default modbusService;

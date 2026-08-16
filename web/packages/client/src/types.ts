export enum E_FN_CODE {
  FN_ANY_FUNCTION_CODE = 0x00, // Only valid for server to register function codes
  FN_READ_COIL = 0x01,
  FN_READ_DISCR_INPUT = 0x02,
  FN_READ_HOLD_REGISTER = 0x03,
  FN_READ_INPUT_REGISTER = 0x04,
  FN_WRITE_COIL = 0x05,
  FN_WRITE_HOLD_REGISTER = 0x06,
  FN_READ_EXCEPTION_SERIAL = 0x07,
  FN_DIAGNOSTICS_SERIAL = 0x08,
  FN_READ_COMM_CNT_SERIAL = 0x0B,
  FN_READ_COMM_LOG_SERIAL = 0x0C,
  FN_WRITE_MULT_COILS = 0x0F,
  FN_WRITE_MULT_REGISTERS = 0x10,
  FN_REPORT_SERVER_ID_SERIAL = 0x11,
  FN_READ_FILE_RECORD = 0x14,
  FN_WRITE_FILE_RECORD = 0x15,
  FN_MASK_WRITE_REGISTER = 0x16,
  FN_R_W_MULT_REGISTERS = 0x17,
  FN_READ_FIFO_QUEUE = 0x18,
  FN_ENCAPSULATED_INTERFACE = 0x2B,
  FN_USER_DEFINED_41 = 0x41,
  FN_USER_DEFINED_42 = 0x42,
  FN_USER_DEFINED_43 = 0x43,
  FN_USER_DEFINED_44 = 0x44,
  FN_USER_DEFINED_45 = 0x45,
  FN_USER_DEFINED_46 = 0x46,
  FN_USER_DEFINED_47 = 0x47,
  FN_USER_DEFINED_48 = 0x48,
  FN_USER_DEFINED_64 = 0x64,
  FN_USER_DEFINED_65 = 0x65,
  FN_USER_DEFINED_66 = 0x66,
  FN_USER_DEFINED_67 = 0x67,
  FN_USER_DEFINED_68 = 0x68,
  FN_USER_DEFINED_69 = 0x69,
  FN_USER_DEFINED_6A = 0x6A,
  FN_USER_DEFINED_6B = 0x6B,
  FN_USER_DEFINED_6C = 0x6C,
  FN_USER_DEFINED_6D = 0x6D,
  FN_USER_DEFINED_6E = 0x6E,
  FN_NONE = 0xFF,
}
export enum E_ModbusAccess {
  MB_ACCESS_NONE = 0,
  MB_ACCESS_READ_ONLY = 1,
  MB_ACCESS_WRITE_ONLY = 2,
  MB_ACCESS_READ_WRITE = 3
}
export interface RegisterData {
  // Modbus address
  address: number
  // Value of the register
  value: number;
  // Name of the register
  name: string;
  // Component id
  id: string;
  // Type of the register
  type: E_FN_CODE;
  // Flags of the register
  flags: number;
  // Group of the register
  group: string;
  // Component of the register
  component: string;
  // Error of the register
  error?: number;
  // Slave ID of the register
  slaveId?: number;
}
export interface PaginatedRegistersResponse {
  meta: {
    page: number;
    pageSize: number;
    totalRegisters: number;
    totalPages: number;
  };
  data: RegisterData[]; // Holds the registers for the current page
}

export interface RegisterUpdatePayload {
  slaveId: number;
  address: number;
  fc: number;
  // For single register updates (FC 3, 6)
  value?: number;
  // For multiple register updates (FC 16)
  values?: readonly number[];
  count?: number;
}
export interface CoilData {
  address: number;
  value: boolean;
  name: string;
  id: number;
  group: string;
  type: number; // Modbus function code (e.g., 1, 2, 5, 15 for coils/inputs)
  flags: number;
}
export interface CoilUpdatePayload {
  address: number;
  fc: number;
  // For single coil updates
  value?: boolean;
  // For multiple coil updates
  values?: readonly boolean[];
  count?: number;
}
export type WsStatus = 'DISCONNECTED' | 'CONNECTING' | 'CONNECTED' | 'ERROR' | 'RECONNECTING';

export interface LogEntry {
  logId: number;
  timestamp: number;
  level: string;
  message: string;
  id?: number;
  name?: string;
}

export type StatusChangeCallback = (status: WsStatus) => void;
export type LogMessageCallback = (logData: any[]) => void;
export type RegisterDataCallback = (registers: RegisterData[]) => void;
export type RegisterUpdateCallback = (update: RegisterUpdatePayload) => void;
export type CoilDataCallback = (coils: CoilData[]) => void;
export type CoilUpdateCallback = (update: CoilUpdatePayload) => void;


/**
 * Enumeration representing different call types.
 * Maps to the C++ E_CALLS enum.
 */
export enum ECalls {
  /** Call a global registered command */
  NONE = 0x00000000,
  /** Call a global registered command */
  COMMAND = 0x00000001,
  /** Call component method (See Bridge::registerMemberFunction & Component::serial_register) */
  METHOD = 0x00000002,
  /** Function call type */
  FUNC = 0x00000004,
  /** User-defined call type */
  USER = 0x00000008
}

/**
* Enumeration representing different message flags.
* Maps to the C++ E_MessageFlags enum.
*/
export enum EMessageFlags {
  /** Internal: no flags */
  NONE = 0x00000000,
  /** Internal: flag designating an unprocessed message */
  NEW = 0x00000001,
  /** Internal: Processing message flag */
  PROCESSING = 0x00000002,
  /** Internal: Processed message flag */
  PROCESSED = 0x00000004,
  /** Internal: Debug message during processing */
  DEBUG = 0x00000008,
  /** Sender: Instruct to send a receipt - Default:On */
  RECEIPT = 0x00000010,
  /** Sender: Instruct to return component state */
  STATE = 0x00000020
}
export interface SystemInfo {
  version: string;
  board: string;
  uptime: number;
  timestamp: number;
  freeHeapKb: number;
  maxFreeBlockKb: number;
  cpuTicks: number;
  loopDurationMs?: number;
  fragmentationPercent: number;
}

export interface NetworkSettingsResponse {
  sta_ssid: string;
  sta_local_ip: string;
  sta_gateway: string;
  sta_subnet: string;
  sta_primary_dns: string;
  sta_secondary_dns: string;
  ap_ssid: string;
  ap_config_ip: string;
  ap_config_gateway: string;
  ap_config_subnet: string;
  hostname: string;
}

export interface NetworkSettingsUpdatePayload {
  sta_ssid?: string;
  sta_password?: string;
  sta_local_ip?: string;
  sta_gateway?: string;
  sta_subnet?: string;
  sta_primary_dns?: string;
  sta_secondary_dns?: string;
  ap_ssid?: string;
  ap_password?: string;
  ap_config_ip?: string;
  ap_config_gateway?: string;
  ap_config_subnet?: string;
  hostname?: string;
}

export interface PlungerSettingsResponse {
  speedSlowHz: number;
  speedMediumHz: number;
  speedFastHz: number;
  speedFillPlungeHz: number;
  speedFillHomeHz: number;
  currentJamThresholdMa: number;
  jammedDurationHomingMs: number;
  jammedDurationMs: number;
  autoModeHoldDurationMs: number;
  maxUniversalJamTimeMs: number;
  fillJoystickHoldDurationMs: number;
  fillPlungedWaitDurationMs: number;
  fillHomedWaitDurationMs: number;
  recordHoldDurationMs: number;
  maxRecordDurationMs: number;
  replayDurationMs: number;
  enablePostFlow: boolean;
  postFlowDurationMs: number;
  postFlowSpeedHz: number;
  currentPostFlowMa: number;
  postFlowStoppingWaitMs: number;
  postFlowCompleteWaitMs: number;
  defaultMaxOperationDurationMs: number;
}

export interface PlungerSettingsUpdatePayload {
  speedSlowHz?: number;
  speedMediumHz?: number;
  speedFastHz?: number;
  speedFillPlungeHz?: number;
  speedFillHomeHz?: number;
  currentJamThresholdMa?: number;
  jammedDurationHomingMs?: number;
  jammedDurationMs?: number;
  autoModeHoldDurationMs?: number;
  maxUniversalJamTimeMs?: number;
  fillJoystickHoldDurationMs?: number;
  fillPlungedWaitDurationMs?: number;
  fillHomedWaitDurationMs?: number;
  recordHoldDurationMs?: number;
  maxRecordDurationMs?: number;
  replayDurationMs?: number;
  enablePostFlow?: boolean;
  postFlowDurationMs?: number;
  postFlowSpeedHz?: number;
  currentPostFlowMa?: number;
  postFlowStoppingWaitMs?: number;
  postFlowCompleteWaitMs?: number;
  defaultMaxOperationDurationMs?: number;
}

export interface ControlPoint {
  x: number;
  y: number;
}

export interface Profile {
  id: number;
  slot: number;
  name: string;
  description?: string;
  duration: number;
  status: number;
  currentTemp: number;
  max: number;
  controlPoints: ControlPoint[];
  targetRegisters: any[];
  overrides?: { sp?: { targetRegister: number; offset: number; }[] };
}

export interface ProfilesResponse {
  profiles: Profile[];
}

export interface PressureProfile {
  slot: number;
  name: string;
  description?: string;
  duration: number;
  status: number;
  max: number;
  enabled: boolean;
  signalPlot: number;
  controlPoints: ControlPoint[];
  targetRegisters: any[];
}

export interface PressureProfilesResponse {
  profiles: PressureProfile[];
}

export interface PressureProfileSavePayload {
  slot?: number;
  name: string;
  description?: string;
  duration: number;
  max: number;
  enabled?: boolean;
  signalPlot?: number;
  controlPoints: ControlPoint[];
  targetRegisters?: any[];
}

export interface SignalPlotData {
  name: string;
  duration: number;
  slot: number;
  controlPoints: SSignalControlPoint[];
}

export interface SSignalControlPoint {
  id: number;
  time: number;
  state: any; // Assuming ESignalState from types.ts, but keeping it simple here
  type: any;  // Assuming ESignalType from types.ts
  arg_0?: number;
  arg_1?: number;
  arg_2?: number;
  user?: any;
  name?: string;
  description?: string;
}



// Add this interface for the saveProfile payload
// Ensure ControlPoint is defined or imported if not already
// export interface ControlPoint { x: number; y: number; }
export interface ProfileSavePayload {
  slot?: number;
  name: string;
  description?: string;
  duration: number;
  max: number;
  controlPoints: ControlPoint[];
  targetRegisters?: any[];
  signalPlot?: number;
  pressureProfile?: number;
  children?: number[];
  overrides?: { sp?: { targetRegister: number; offset: number; }[] };
}

export interface CoilsArrayResponse {
  coils: CoilResponse[];
  meta: {
    page: number;
    pageSize: number;
    totalCoils: number;
    totalPages: number;
  };
}

export interface CoilResponse {
  address: number;
  value: boolean;
  name?: string;
  id?: any; // id is used as coil.id || coil.address in context
  type?: number;
  flags?: number;
  group?: string;
}

export interface CoilUpdateResponse {
  success: boolean;
  address: number;
  value: boolean;
}

export interface RegistersArrayResponse {
  registers: RegisterResponse[];
  meta: {
    page: number;
    pageSize: number;
    totalRegisters: number;
    totalPages: number;
  };
}

export interface RegisterResponse {
  address: number;
  value: number;
  name: string;
  id: string;
  type: number;
  flags: number;
  group: string;
}

export interface RegisterUpdateResponse {
  success: boolean;
  address: number;
  value: number;
}

export interface RelayTestResponse {
  success: boolean;
  message: string;
}

export interface RegisteredMethod {
  id: number;
  component: string;
  method: string;
}

export interface SerialCommandPayload {
  componentId: number;
  callType: ECalls;
  flags: EMessageFlags;
  method: string;
  arg1?: number;
  arg2?: number;
}

export interface SerialCommandResponse {
  success: boolean;
  message?: string;
  result?: any;
}

export interface ComponentFlags {
  run: number;
  network: number;
  feature: number;
}

export interface ComponentInfo {
  id: number;
  name: string;
  enabled: boolean;
  flags: ComponentFlags;
}

export interface ControllerConfig {
  slaveid: number;
  name: string;
  enabled: boolean;
}

export interface PartitionConfig {
  name: string;
  controllers: ControllerConfig[];
  startslaveid?: number;
  numcontrollers?: number;
}

export interface Setting {
  enabled: boolean;
  id: number;
  name: string;
  group: string;
  flags: number;
  parent: number;
  type: string;
  value: string | number | boolean;
}

export interface Settings {
  master: string;
  slaves: string[];
  partitions: PartitionConfig[];
  settings?: Setting[];
}

// Filesystem API types
export interface FileReadResponse {
  success: boolean;
  filename: string;
  content: string;
  error?: string;
}

export interface FileWriteRequest {
  filename: string;
  content: string;
}

export interface FileWriteResponse {
  success: boolean;
  message?: string;
  error?: string;
}

/**
 * Enumeration for Binary WebSocket Message Types
 * Matches BroadcastMessageType for binary frames in RestServer.cpp
 */
export enum BinaryWebSocketMessageType {
  COIL_UPDATE = 0x01,
  REGISTER_UPDATE = 0x02
}

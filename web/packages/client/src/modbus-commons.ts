// --- Type & Enum Definitions (Derived from C++ headers like ModbusTypes.h) ---

/** Modbus Function Codes */
export enum FnCode {
  /** Read Coils */
  ReadCoil = 0x01,
  /** Read Discrete Inputs */
  ReadDiscrInput = 0x02,
  /** Read Holding Registers */
  ReadHoldRegister = 0x03,
  /** Read Input Registers */
  ReadInputRegister = 0x04,
  /** Write Single Coil */
  WriteCoil = 0x05,
  /** Write Single Holding Register */
  WriteHoldRegister = 0x06,
  /** Read Exception Status (Serial Line only) */
  ReadExceptStatus = 0x07,
  /** Diagnostic (Serial Line only) */
  Diagnostic = 0x08,
  /** Get Com Event Counter (Serial Line only) */
  GetComEventCounter = 0x0B,
  /** Get Com Event Log (Serial Line only) */
  GetComEventLog = 0x0C,
  /** Write Multiple Coils */
  WriteMultCoils = 0x0F,
  /** Write Multiple Holding Registers */
  WriteMultRegisters = 0x10,
  /** Report Server ID (Serial Line only) */
  ReportServerID = 0x11,
  /** Read File Record (TCP only) */
  ReadFileRecord = 0x14,
  /** Write File Record (TCP only) */
  WriteFileRecord = 0x15,
  /** Mask Write Register */
  MaskWriteRegister = 0x16,
  /** Read/Multple Write Registers */
  ReadWriteMultRegisters = 0x17,
  /** Read FIFO Queue */
  ReadFIFOQueue = 0x18,
  /** Encapsulated Interface Transport (CANopen, etc.) */
  EncapsulatedInterfaceTransport = 0x2B,

  // Add other specific function codes if necessary, like MEI types (0x2B)
  // For example, Modbus Device Information
  ReadDeviceIdentification = 0x2B, // Sub-function 0x0E / Category 0x04

  // Client-internal / Virtual Function Codes - often used for queuing/state
  Internal_ReadBlocks = 0xFF01, // Represents a request to read predefined blocks
  Internal_WriteRegisters = 0xFF02, // Represents a request to write pending changes
}

/** Modbus Error Codes (Derived from MB_Error enum) */
export enum ModbusErrorCode {
  Success = 0x00,

  // Standard Modbus Exceptions (Returned by Server)
  IllegalFunction = 0x01, // The function code received in the query is not an allowable action for the server (or slave).
  IllegalDataAddress = 0x02, // The data address received in the query is not an allowable address for the server.
  IllegalDataValue = 0x03, // A value in the query data field is not an allowable value for the server.
  ServerDeviceFailure = 0x04, // An unrecoverable error occurred while the server was attempting to perform the requested action.
  Acknowledge = 0x05, // The server accepts the request, but a lengthy processing time is required. Server returns this response to let the client know that it is being processed.
  ServerDeviceBusy = 0x06, // The server is engaged in processing a previous detect transaction and cannot accept more process new request.
  NegativeAcknowledge = 0x07, // The server cannot perform the requested function. This code is used to indicate that a programmed communication path was not available.
  MemoryParityError = 0x08, // Extended file area problem occurred during the attempted read or records modification.
  GatewayPathUnavailable = 0x0A, // Specialized use in conjunction with function codes 20 and 21 and reference type 6, to indicate that the gateway was unable to allocate an internal communication path from the client to the server.
  GatewayTargetNoResp = 0x0B, // Specialized use in conjunction with function codes 20 and 21 and reference type 6, to indicate that the target device failed to respond.

  // Internal Operation/Queue Errors (Client-Side or Library Internal)
  OpNotReady = 0xF0, // Operation cannot be performed in current state (e.g., queue full, not connected)
  OpQueueFull = 0xF1, // The operation queue (e.g. in ModbusRTU or ModbusTCP manager) is full.
  OpClientQueueFull = 0xF2, // A specific client/device queue is full.
  OpExecutionFailed = 0xF3, // The operation could not be executed after scheduling (e.g., communication error persisted).
  OpInvalidParameter = 0xF4, // An operation was created with invalid parameters.
  OpRetrying = 0xF5, // The operation failed but is being retried.
  OpMaxRetriesExceeded = 0xF6, // The operation failed after exhausting all retries.

  // Communication Errors (eModbus Specific + Generic RTU/TCP)
  Timeout = 0xE0, // No response received within the expected time.
  InvalidServer = 0xE1, // Response received, but properties (slaveId, FC) don't match the request.
  CrcError = 0xE2, // CRC checksum mismatch (RTU).
  FcMismatch = 0xE3, // Function code in response does not match request.
  ServerIdMismatch = 0xE4, // Server/Slave ID in response does not match request.
  PacketLengthError = 0xE5, // Received packet length is inconsistent or invalid.
  ParameterCountError = 0xE6, // Received parameters count is wrong for the function code.
  ParameterLimitError = 0xE7, // Received parameters exceed limits (e.g. count too large).
  RequestQueueFull = 0xE8, // Alias for OpClientQueueFull? Or a distinct queue type? Using OpClientQueueFull for now.
  IllegalIpOrPort = 0xE9, // Invalid IP address or port for TCP client.
  IpConnectionFailed = 0xEA, // Failed to connect to the TCP server.
  TcpHeadMismatch = 0xEB, // Modbus TCP header structure mismatch.
  EmptyMessage = 0xEC, // Received an empty message.
  AsciiFrameError = 0xED, // ASCII frame start/end marker errors.
  AsciiCrcError = 0xEE, // ASCII LRC checksum mismatch.
  AsciiInvalidChar = 0xEF, // Invalid character received in ASCII mode.
  BroadcastError = 0xFD, // Cannot perform this operation on a broadcast address.
  UndefinedError = 0xFE, // Catch-all for errors not specifically defined.
  UnknownError = 0xFF, // A placeholder or default for unhandled cases.
}


/** Device State (Derived from E_DeviceState enum) */
export enum DeviceState {
  Uninitialized,
  Initializing,
  Idle,
  Running,
  Error,
}


/** Represents a single Modbus Register or Coil state *managed* by a device handler. */
export interface RegisterState {
  type: FnCode;
  address: number; // Modbus address
  value: number; // The current value (for coils/discr inputs, 0 = OFF, 1 = ON)
  priority: number; // Some user-defined priority
  // getBoolValue(): boolean; // Helper function if needed, or just check value === 1
}

/** Represents a Modbus operation being enqueued/processed. */
export interface ModbusOperation {
    slaveId: number;
    type: FnCode;
    address: number; // Start address
    count?: number; // Number of values for block operations
    value?: number | number[]; // Value for write operations (single or multiple)
    priority?: number; // Operation priority
    // any other relevant fields from the C++ struct
}

/** Represents a mandatory read block configured for a device. */
export interface ModbusReadBlock {
    startAddress: number;
    count: number;
    type: FnCode; // Only Read function codes allowed
    readInterval: number; // Interval in milliseconds
    lastReadTime: number; // Timestamp of the last read attempt
    isUsed: boolean; // Flag to indicate if this block is active/configured
    // setUsed(used: boolean): void; // C++ uses a method, but a property is simpler in TS interface
}


/**
 * Interface representing the necessary functions of the ModbusRTU manager
 * used by the extracted functions. This decouples the logic from a specific
 * manager implementation.
 */
export interface IModbusRTUManager {
  readRegister(slaveId: number, address: number): ModbusErrorCode;
  readCoil(slaveId: number, address: number): ModbusErrorCode;
  writeRegister(slaveId: number, address: number, value: number): ModbusErrorCode;
  writeCoil(slaveId: number, address: number, value: boolean): ModbusErrorCode; // Assumes writeCoil takes boolean

  readHoldingRegisters(slaveId: number, startAddress: number, count: number): ModbusErrorCode;
  readInputRegisters(slaveId: number, startAddress: number, count: number): ModbusErrorCode;
  readCoils(slaveId: number, startAddress: number, count: number): ModbusErrorCode;
  readDiscreteInputs(slaveId: number, startAddress: number, count: number): ModbusErrorCode;

  isRegisterSynchronized(slaveId: number, address: number): boolean;
  isCoilSynchronized(slaveId: number, address: number): boolean;

  hasPendingOperations(slaveId: number): boolean;
  isOperationAlreadyPending(op: ModbusOperation): boolean;

  // Add other methods called in read(), write() etc. if they were extracted
}

/**
 * Interface representing a basic logger function, mimicking ArduinoLog.
 */
export interface ILog {
    errorln(message: string, ...args: any[]): void;
    warningln(message: string, ...args: any[]): void;
    infoln(message: string, ...args: any[]): void;
    noticeln(message: string, ...args: any[]): void;
    traceln(message: string, ...args: any[]): void;
    // debugln might also be present
}

// Provide a default console logger if ILog is not provided
const defaultLogger: ILog = {
    errorln: (msg, ...args) => console.error(`[ERROR] ${msg}`, ...args),
    warningln: (msg, ...args) => console.warn(`[WARN ] ${msg}`, ...args),
    infoln: (msg, ...args) => console.info(`[INFO ] ${msg}`, ...args),
    noticeln: (msg, ...args) => console.log(`[NOTICE] ${msg}`, ...args), // console.log is often used for notice/info
    traceln: (msg, ...args) => console.debug(`[TRACE] ${msg}`, ...args), // console.debug or console.trace
}

// --- Extracted Functions ---

/**
 * Translates a Modbus Error Code enum value to a human-readable string.
 * @param error The ModbusErrorCode value.
 * @returns A string description of the error.
 */
export function modbusErrorToString(error: ModbusErrorCode): string {
  switch (error) {
    case ModbusErrorCode.Success: return "Success";
    case ModbusErrorCode.IllegalFunction: return "Illegal Function";
    case ModbusErrorCode.IllegalDataAddress: return "Illegal Data Address";
    case ModbusErrorCode.IllegalDataValue: return "Illegal Data Value";
    case ModbusErrorCode.ServerDeviceFailure: return "Server Device Failure";
    case ModbusErrorCode.Acknowledge: return "Acknowledge";
    case ModbusErrorCode.ServerDeviceBusy: return "Server Device Busy";
    case ModbusErrorCode.NegativeAcknowledge: return "Negative Acknowledge";
    case ModbusErrorCode.MemoryParityError: return "Memory Parity Error";
    case ModbusErrorCode.GatewayPathUnavailable: return "Gateway Path Unavailable";
    case ModbusErrorCode.GatewayTargetNoResp: return "Gateway Target Device Failed to Respond";

    case ModbusErrorCode.OpNotReady: return "Operation Not Ready";
    case ModbusErrorCode.OpQueueFull: return "ModbusRTU Operation Queue Full";
    case ModbusErrorCode.OpClientQueueFull: return "Client Operation Queue Full"; // Adjusted name
    case ModbusErrorCode.OpExecutionFailed: return "Operation Execution Failed";
    case ModbusErrorCode.OpInvalidParameter: return "Invalid Parameter";
    case ModbusErrorCode.OpRetrying: return "Operation Retrying";
    case ModbusErrorCode.OpMaxRetriesExceeded: return "Max Retries Exceeded";

    case ModbusErrorCode.Timeout: return "Timeout";
    case ModbusErrorCode.InvalidServer: return "Invalid Server Response";
    case ModbusErrorCode.CrcError: return "CRC Error"; // RTU
    case ModbusErrorCode.FcMismatch: return "Function Code Mismatch";
    case ModbusErrorCode.ServerIdMismatch: return "Server ID Mismatch";
    case ModbusErrorCode.PacketLengthError: return "Packet Length Error";
    case ModbusErrorCode.ParameterCountError: return "Parameter Count Error";
    case ModbusErrorCode.ParameterLimitError: return "Parameter Limit Error";
    // case ModbusErrorCode.RequestQueueFull: return "eModbus Request Queue Full"; // Same as OpClientQueueFull? Check enum vals
    case ModbusErrorCode.IllegalIpOrPort: return "Illegal IP or Port"; // TCP
    case ModbusErrorCode.IpConnectionFailed: return "IP Connection Failed"; // TCP
    case ModbusErrorCode.TcpHeadMismatch: return "TCP Header Mismatch"; // TCP
    case ModbusErrorCode.EmptyMessage: return "Empty Message Received";
    case ModbusErrorCode.AsciiFrameError: return "ASCII Frame Error"; // ASCII
    case ModbusErrorCode.AsciiCrcError: return "ASCII LRC Error"; // ASCII
    case ModbusErrorCode.AsciiInvalidChar: return "ASCII Invalid Character"; // ASCII
    case ModbusErrorCode.BroadcastError: return "Broadcast Error";

    case ModbusErrorCode.UndefinedError:
    default:
      return "Undefined or Unknown Error";
    case ModbusErrorCode.UnknownError:
        return "Unknown Error";
  }
}

/**
 * Translates a Device State enum value to a human-readable string.
 * @param state The DeviceState value.
 * @returns A string description of the state.
 */
export function deviceStateToString(state: DeviceState): string {
  switch (state) {
    case DeviceState.Uninitialized: return "Uninitialized";
    case DeviceState.Initializing: return "Initializing";
    case DeviceState.Idle: return "Idle";
    case DeviceState.Running: return "Running";
    case DeviceState.Error: return "Error";
    default: return "Unknown";
  }
}


/**
 * Checks if a given Modbus function code represents a write operation.
 * @param opType The function code.
 * @returns true if the type is a write function, false otherwise.
 */
export function isWriteOperation(opType: FnCode): boolean {
    switch (opType) {
        case FnCode.WriteCoil:
        case FnCode.WriteHoldRegister:
        case FnCode.WriteMultCoils:
        case FnCode.WriteMultRegisters:
        case FnCode.MaskWriteRegister: // Although a mask write, it modifies a register
        case FnCode.WriteFileRecord:
        case FnCode.ReadWriteMultRegisters: // Includes a write part
            return true;
        default:
            return false;
    }
}

/**
 * Filters out duplicate Modbus operations if they are already pending in the manager.
 * Note: This logic directly mimics the C++ filter's core logic. The C++ filter held a reference
 * to the manager; here, we pass the manager interface as a parameter.
 * @param manager The Modbus manager instance (needs `isOperationAlreadyPending`).
 * @param op The operation to filter.
 * @returns true if the operation should be allowed, false if it's a duplicate and should be dropped.
 */
export function filterDuplicateOperation(manager: IModbusRTUManager, op: ModbusOperation): boolean {
    // Use the manager method to check for pending duplicates
    if (manager.isOperationAlreadyPending(op)) {
        // Logging is part of the original logic, let's add a placeholder or optional logger
        // Log.traceln("Filter: Dropping duplicate operation (slave: %d, type: %d, address: %d)", op.slaveId, op.type, op.address);
        return false; // Filter out the operation
    }
    return true; // Let the operation through
}

// The RateLimitFilter logic is tied to the filter instance's state (lastOperationTime)
// and requires a time source (millis()). It's less suitable for a standalone pure function,
// but we can abstract the core check:
/**
 * Checks if enough time has passed since the last operation for rate limiting.
 * Note: This requires external tracking of the last operation time and current time.
 * @param currentTime The current timestamp (e.g., from performance.now() or Date.now()).
 * @param lastOperationTime The timestamp of the last allowed operation.
 * @param minInterval The minimum time required between operations in milliseconds.
 * @returns true if the operation is allowed based on rate limiting, false otherwise.
 */
export function isRateLimited(currentTime: number, lastOperationTime: number, minInterval: number): boolean {
    return (currentTime - lastOperationTime) < minInterval;
}


/**
 * Reads the state of a single register or coil from the Modbus device
 * by queuing a read operation using the manager.
 * @param manager The Modbus manager instance (needs `readRegister`, `readCoil`).
 * @param state The RegisterState defining what to read (type and address).
 * @param slaveId The Modbus slave ID.
 * @returns The ModbusErrorCode result of queuing the operation.
 */
export function readRegisterStateFromDevice(manager: IModbusRTUManager, state: RegisterState, slaveId: number): ModbusErrorCode {
  switch (state.type) {
    case FnCode.ReadInputRegister:
    case FnCode.ReadHoldRegister:
      return manager.readRegister(slaveId, state.address);
    case FnCode.ReadCoil:
    case FnCode.ReadDiscrInput:
      return manager.readCoil(slaveId, state.address);
    default:
      // The C++ code didn't have a default here, but it was a method on RegisterState
      // which implies valid types. In a standalone function handling any RegisterState,
      // a default is safer, though the C++ switch implies only read types are handled here.
      // Let's follow the C++ logic and return Specific errors for non-read types.
      if (isWriteOperation(state.type)) {
          // Should not call readFromDevice for write types
          return ModbusErrorCode.IllegalFunction;
      }
      return ModbusErrorCode.UndefinedError; // Should not happen based on switch cases
  }
}

/**
 * Attempts to write the current value of a single register or coil
 * to the Modbus device by queuing a write operation using the manager.
 * Note: Handles only specific write function codes.
 * @param manager The Modbus manager instance (needs `writeRegister`, `writeCoil`).
 * @param state The RegisterState defining what to write (type, address, value).
 * @param slaveId The Modbus slave ID.
 * @param log Optional logger instance for warnings.
 * @returns The ModbusErrorCode result of queuing the operation. Returns error for read-only types or unhandled types.
 */
export function writeRegisterStateToDevice(manager: IModbusRTUManager, state: RegisterState, slaveId: number, log: ILog = defaultLogger): ModbusErrorCode {
  switch (state.type) {
    case FnCode.WriteHoldRegister:
      return manager.writeRegister(slaveId, state.address, state.value);
    case FnCode.WriteCoil:
      // Assuming value is 0 or 1 for coils in RegisterState
      return manager.writeCoil(slaveId, state.address, state.value === 1);
    case FnCode.ReadInputRegister:
    case FnCode.ReadDiscrInput:
    case FnCode.ReadHoldRegister: // Though it's a read type, included in C++ switch as illegal here
    case FnCode.ReadCoil: // Though it's a read type, included in C++ switch as illegal here
      return ModbusErrorCode.IllegalDataAddress; // Attempted write to a read-only address/type
    default:
      log.warningln("Attempted writeToDevice with unhandled/read-only type: 0x%X", state.type); // Match C++ log style
      return ModbusErrorCode.IllegalFunction;
  }
}

/**
 * Prints the details and current state of a single register or coil.
 * Note: Uses the manager to get synchronization status. Relies on a logger.
 * @param manager The Modbus manager instance (needs `isRegisterSynchronized`, `isCoilSynchronized`).
 * @param state The RegisterState to print.
 * @param slaveId The Modbus slave ID.
 * @param log The logger instance to use for output.
 */
export function printRegisterState(manager: IModbusRTUManager, state: RegisterState, slaveId: number, log: ILog = defaultLogger): void {
  const syncStatus = (state.type === FnCode.ReadCoil || state.type === FnCode.ReadDiscrInput)
    ? manager.isCoilSynchronized(slaveId, state.address)
    : manager.isRegisterSynchronized(slaveId, state.address);
  const syncString = syncStatus ? "Synchronized" : "Not synchronized";

  switch (state.type) {
    case FnCode.ReadInputRegister:
      log.noticeln("Input Register %d: %d (Priority: %d, %s)",
                 state.address, state.value, state.priority, syncString);
      break;
    case FnCode.ReadHoldRegister:
      log.noticeln("Holding Register %d: %d (Priority: %d, %s)",
                 state.address, state.value, state.priority, syncString);
      break;
    case FnCode.ReadCoil:
      // Assuming value is 0 or 1 for coils in RegisterState
      log.noticeln("Coil %d: %s (Priority: %d, %s)",
                 state.address, (state.value === 1) ? "ON" : "OFF", state.priority, syncString);
      break;
    case FnCode.ReadDiscrInput:
      // Assuming value is 0 or 1 for discr inputs in RegisterState
      log.noticeln("Discrete Input %d: %s (Priority: %d, %s)",
                 state.address, (state.value === 1) ? "ON" : "OFF", state.priority, syncString);
      break;
    default:
      // This case in C++ also used isRegisterSynchronized, which might be incorrect for non-register types.
      // We'll keep the sync check based on type as done above, but use the generic print format.
      log.noticeln("Register Address %d: Value %d (Type: 0x%X, Priority: %d, %s)",
                 state.address, state.value, state.type, state.priority, syncString);
      break;
  }
}


/**
 * Finds a RegisterState object in an array by its Modbus address.
 * @param registers An array of RegisterState objects.
 * @param address The Modbus address to search for.
 * @returns The found RegisterState object or undefined if not found.
 */
export function findRegisterByAddress(registers: (RegisterState | null | undefined)[], address: number): RegisterState | undefined {
  // Iterate through the array, skipping null/undefined entries
  for (const regState of registers) {
    if (regState != null && regState.address === address) {
      return regState;
    }
  }
  return undefined; // Return undefined if no register is found with that address
}


/**
 * Attempts to set the 'value' property of a writable RegisterState found in an array by address.
 * Checks if the register type is writeable (Holding Register or Coil).
 * @param registers An array of RegisterState objects (potentially containing null/undefined).
 * @param address The Modbus address of the register to update.
 * @param value The new value to set.
 * @param log Optional logger instance for warnings.
 */
export function setWritableRegisterValue(registers: (RegisterState | null | undefined)[], address: number, value: number, log: ILog = defaultLogger): void {
  const register = findRegisterByAddress(registers, address);

  if (register) {
      if (register.type === FnCode.WriteHoldRegister || register.type === FnCode.WriteCoil) {
          // Note: C++ code used implicit write types FN_WRITE_HOLD_REGISTER and FN_WRITE_COIL
          // in the switch/if for writeToDevice. Assuming these are the types indicating writability
          // for the managed RegisterState objects.
          register.value = value;
          // The C++ didn't explicitly log success here, only warnings/errors
      } else {
          log.warningln("Register with address %d found, but cannot set value - not a Writable Holding/Coil type (0x%X).",
                        address, register.type); // Match C++ log format
      }
  } else {
    log.warningln("Writable register with address %d not found for setting value.", address); // Match C++ log format
  }
}


/**
 * Updates the value of a RegisterState object found in an array by address,
 * typically used when a response indicates a new value from the device.
 * @param registers An array of RegisterState objects (potentially containing null/undefined).
 * @param address The Modbus address of the register to update.
 * @param newValue The new value to set.
 * @returns true if a register was found and updated, false otherwise.
 */
export function updateRegisterValueInList(registers: (RegisterState | null | undefined)[], address: number, newValue: number): boolean {
    // Reuse find function
    const register = findRegisterByAddress(registers, address);

    if (register) {
        // C++ only updated if the value changed. Let's follow that logic.
        if (register.value !== newValue) {
            register.value = newValue;
            // The C++ code had a trace log if updated, but also mentioned it's normal
            // if the address isn't managed. The check for "updated" handles that.
            // Log.traceln("Device %d: Register address %d updated to %d", slaveId, address, newValue); // This log depends on slaveId, so keep it outside this function ideally
        }
        return true; // Register found (value might or might not have changed)
    } else {
        // The C++ had a commented-out trace log for unmanaged addresses.
        // For a pure function, absence of the register is just a return value.
        return false; // Register not found
    }
}

// --- Possible future extractions (less common/more complex state) ---

/*
// Logic from RTU_Base::read:
// This function orchestrates reading mandatory blocks based on intervals.
// It depends heavily on device state (IDLE/RUNNING) and the mandatoryReadBlocks list.
// Extracting it requires passing the device state, the block list, the manager,
// a time source (like `millis()` in C++), and a logger.
// It also modifies the device state.

export function performMandatoryReads(
    manager: IModbusRTUManager,
    slaveId: number,
    deviceState: DeviceState, // state is read
    mandatoryReadBlocks: ModbusReadBlock[], // list is iterated and modified (.lastReadTime)
    currentTime: number, // Needs current time to check intervals
    setDeviceState: (newState: DeviceState) => void, // Needs ability to change device state
    log: ILog = defaultLogger // Needs logger for warnings
): void {
    if (deviceState === DeviceState.Running) {
        // C++ had a check `if (state == RUNNING) return;` at the start of read().
        // This implies reads are only queued when NOT in the RUNNING state.
        // This seems counter-intuitive (you read WHILE running communications)
        // but let's follow the C++ source logic. However, later in the function
        // it transitions to RUNNING if reads are queued. This suggests `read()`
        // is called when IDLE to *initiate* reads and transition to RUNNING.
        // Let's refine the logic based on the state transitions observed in updateState.
        // The state machine transitions IDLE -> RUNNING *if* hasPendingOps becomes true.
        // The `read()` function *queues* operations if intervals pass.
        // So, `read()` should be callable while IDLE. If it queues an op, the state will
        // transition to RUNNING later (e.g., in `updateState`).
        // The initial check `if (state == RUNNING) return;` in C++ `read` seems incorrect
        // based on the `updateState` logic that RUNNING -> IDLE when ops drain,
        // and IDLE -> RUNNING when ops appear. If `read` couldn't be called in RUNNING,
        // how would periodic reads happen *while* the device is busy?
        // Let's *ignore* that initial check from the C++ `read` function as potentially buggy documentation.
        // Or maybe it means "don't queue *new* blocks if device is already heavily running with *other* tasks"?
        // Given the title "Extract all *common* functionality", complex state checks tied
        // to the specific `RTU_Base` state machine might not be the best candidates.
        // Let's consider extracting *just* the queuing logic per block.

        // --- Reconsidering extraction ---
        // The core logic of `read()` is:
        // 1. Iterate over defined read blocks.
        // 2. Check if block is 'used' and interval has passed.
        // 3. If so, call the appropriate manager read function (readHoldingRegisters etc.).
        // 4. Update the block's lastReadTime if queuing was successful.
        // 5. Track if *any* read was successfully queued.
        // 6. If reads were queued AND the device was IDLE, transition to RUNNING (this part is state-machine specific).

        // Let's extract the block processing loop part, but acknowledge it needs state/time context.
    }

    let readsQueued = false;
    for (const block of mandatoryReadBlocks) {
         if (!block.isUsed) { // isUsed is a property in our TS interface
            continue;
         }
        if (currentTime - block.lastReadTime >= block.readInterval) {
            let err: ModbusErrorCode = ModbusErrorCode.UndefinedError; // Initialize with a non-success default

            // Check for valid type before calling manager
            const isValidReadType = (type: FnCode) =>
                 type === FnCode.ReadHoldRegister ||
                 type === FnCode.ReadInputRegister ||
                 type === FnCode.ReadCoil ||
                 type === FnCode.ReadDiscrInput;

            if (!isValidReadType(block.type)) {
                log.warningln("Device %d: Mandatory read block has invalid type (0x%X). Skipping.", slaveId, block.type);
                // err remains UndefinedError or could be set to IllegalFunction
                err = ModbusErrorCode.IllegalFunction; // Match C++ logic slightly better
            } else {
                // Call the appropriate manager method
                switch (block.type) {
                    case FnCode.ReadHoldRegister:
                        err = manager.readHoldingRegisters(slaveId, block.startAddress, block.count);
                        break;
                    case FnCode.ReadInputRegister:
                        err = manager.readInputRegisters(slaveId, block.startAddress, block.count);
                        break;
                    case FnCode.ReadCoil:
                        err = manager.readCoils(slaveId, block.startAddress, block.count);
                        break;
                    case FnCode.ReadDiscrInput:
                        err = manager.readDiscreteInputs(slaveId, block.startAddress, block.count);
                        break;
                    // Default case handled by isValidReadType check above
                }
            }

            if (err === ModbusErrorCode.Success) {
                readsQueued = true;
                block.lastReadTime = currentTime; // Update timestamp only on successful queue
                log.traceln("Device %d: Queued read for block - Start: %d, Count: %d, Type: 0x%X",
                           slaveId, block.startAddress, block.count, block.type);
            } else {
                 // Only log if the error ISN'T that the queue was full (OpNotReady in C++ code, or OpQueueFull/OpClientQueueFull in our TS)
                 // The C++ check was `if (err != MB_Error::OpNotReady)`. Let's check for known queue errors.
                 if (err !== ModbusErrorCode.OpNotReady && err !== ModbusErrorCode.OpQueueFull && err !== ModbusErrorCode.OpClientQueueFull) {
                     log.warningln("Device %d: Failed to queue read for block - Start: %d, Count: %d. Error: %s",
                                   slaveId, block.startAddress, block.count, modbusErrorToString(err));
                 } else if (err !== ModbusErrorCode.OpNotReady) { // Check explicitly for the logging condition in C++
                     // Log warning if queue is full, matching C++ logic
                     log.warningln("Device %d: Read queue is full. Skipping read for block - Start: %d, Count: %d.",
                                    slaveId, block.startAddress, block.count);
                 }
            }
        }
    }

    // This state logic is specific to RTU_Base and might not be general common functionality
    // if (readsQueued && deviceState === DeviceState.Idle) {
    //     setDeviceState(DeviceState.Running);
    // }
}
*/

// The `write` function in C++ also iterates through registers checking their type
// and calls `writeToDevice` for specific types. This is similar to the `read` function's
// pattern of iterating and calling manager methods, but tied to the `registers` list.
// Both `read` and `write` in C++ are part of the `RTU_Base`'s internal state management.
// Extracting them fully would require passing the entire device state, its lists (`registers`, `mandatoryReadBlocks`),
// the manager, time source, and state change callback, making them less 'common' utility functions
// and more like methods operating on a device object, even if defined outside a class.
// The functions related to a *single* RegisterState (`readRegisterStateFromDevice`, `writeRegisterStateToDevice`, `printRegisterState`)
// and the list utilities (`findRegisterByAddress`, `setWritableRegisterValue`, `updateRegisterValueInList`)
// plus the pure mapping functions are better fits for "common functionality" extracted as standalone functions.
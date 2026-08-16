/**
 * @file modbus_types.ts
 * @brief TypeScript definitions for Modbus-related enums, structs, and types
 *        extracted from C++ header files (ModbusTypes.h, ModbusManager.h,
 *        ModbusRTU.h, ModbusLogicEngine.h, Modbus.h).
 *
 * Naming Conventions:
 * - Enums: Capitalized, prefixed with E_
 * - Structs: Capitalized, prefixed based on context (Modbus related -> MB_)
 * - Types (aliases, interfaces for views): Capitalized, prefixed with T_
 *
 * Grouping:
 * - Common: Types/Enums used across RTU and TCP contexts.
 * - RTU: Types/Enums specific to Modbus RTU implementation.
 * - TCP: Types/Enums specific to Modbus TCP implementation (less direct structs found).
 * - Logic Engine: Types/Enums specific to the Modbus Logic Engine feature.
 */

// --- Common Constants (derived from C++ headers or configuration) ---
// Note: Actual values may differ based on the specific config-modbus.h
export const MAX_ADDRESSES_PER_SLAVE = 128; // Assuming typical config value for array sizes in SlaveData
export const MAX_PENDING_OPERATIONS = 64;   // Assuming typical config value for operation queue size
export const MAX_MODBUS_SLAVES = 16;        // Assuming typical config value for Manager/ModbusRTU slave data array size
export const MAX_MODBUS_COMPONENTS = 32;    // Assuming typical config value for ModbusTCP mapping storage
export const MAX_READ_BLOCKS = 10;          // Assuming typical config value for RTU_Base read block storage
export const MAX_REGISTERS = 128;           // Assuming typical config value for RTU_Base register array size


// Bitmask flags for MB_OPERATION (derived from C++ TEST/SET_BIT_TO usage)
// Note: Bit positions assumed based on typical usage, verify with C++ macros if available
export const OP_USED_BIT = 0;         // Flag indicating if the operation slot is used
export const OP_HIGH_PRIORITY_BIT = 1; // Flag indicating high priority
export const OP_IN_PROGRESS_BIT = 2;   // Flag indicating operation is currently being processed
export const OP_BROADCAST_BIT = 3;     // Flag indicating if it's a broadcast operation
export const OP_SYNCHRONIZED_BIT = 4;  // Flag indicating internal state is synchronized with the intended operation


// Bitmask flags for MB_VALUE_ENTRY (derived from C++ CBI/SBI usage)
export const VALUE_USED_BIT = 0;       // Flag indicating if the value entry slot is used
export const VALUE_SYNCHRONIZED_BIT = 1; // Flag indicating value has been synchronized with the slave


// Bitmask flags for MB_READ_BLOCK
export const BLOCK_USED_BIT = 0;       // Flag indicating if the read block slot is used


// --- Common Enums ---

/**
 * @brief Modbus Function Codes.
 */
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
    FN_USER_DEFINED_44 = 4, // Note: C++ has 0x44
    FN_USER_DEFINED_45 = 5, // Note: C++ has 0x45
    FN_USER_DEFINED_46 = 6, // Note: C++ has 0x46
    FN_USER_DEFINED_47 = 7, // Note: C++ has 0x47
    FN_USER_DEFINED_48 = 8, // Note: C++ has 0x48
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

/**
 * @brief Modbus operation types (a simplified view of function codes).
 */
export enum E_MB_OP_TYPE {
    MB_READ_COIL,
    MB_READ_REGISTER,
    MB_WRITE_COIL,
    MB_WRITE_REGISTER,
    MB_WRITE_MULTIPLE_REGISTERS
}

/**
 * @brief Modbus operation status.
 */
export enum E_MB_OP_STATUS {
    MB_PENDING,
    MB_SUCCESS,
    MB_FAILED,
    MB_RETRYING
}

/**
 * @brief Modbus operation error codes - incorporates internal errors and eModbus errors.
 */
export enum E_MB_ERROR {
    // Standard Modbus Exception Codes (as defined by eModbus)
    Success = 0x00, // Also used for internal success
    IllegalFunction = 0x01,
    IllegalDataAddress = 0x02,
    IllegalDataValue = 0x03,
    ServerDeviceFailure = 0x04,
    Acknowledge = 0x05,
    ServerDeviceBusy = 0x06,
    NegativeAcknowledge = 0x07,
    MemoryParityError = 0x08,
    GatewayPathUnavailable = 0x0A,
    GatewayTargetNoResp = 0x0B,

    // Internal Operation/Queue Errors (remapped to avoid conflicts)
    OpNotReady = 0x10,           // Renamed from NotReady
    OpQueueFull = 0x11,          // Renamed from QueueFull
    OpClientQueueFull = 0x12,    // Renamed from ClientQueueFull
    OpExecutionFailed = 0x13,    // Renamed from ExecutionFailed
    OpInvalidParameter = 0x14,   // Renamed from InvalidParameter
    OpRetrying = 0x15,           // Renamed from Retrying
    OpMaxRetriesExceeded = 0x16, // Renamed from MaxRetriesExceeded

    // eModbus Specific Communication Errors
    Timeout = 0xE0,
    InvalidServer = 0xE1,
    CrcError = 0xE2, // RTU only
    FcMismatch = 0xE3,
    ServerIdMismatch = 0xE4,
    PacketLengthError = 0xE5,
    ParameterCountError = 0xE6,
    ParameterLimitError = 0xE7,
    RequestQueueFull = 0xE8, // eModbus client queue
    IllegalIpOrPort = 0xE9,
    IpConnectionFailed = 0xEA,
    TcpHeadMismatch = 0xEB,
    EmptyMessage = 0xEC,
    AsciiFrameError = 0xED,
    AsciiCrcError = 0xEE,
    AsciiInvalidChar = 0xEF,
    BroadcastError = 0xF0,
    UndefinedError = 0xFF // Other communication error
}

/**
 * @brief Filter types for operation identification without RTTI.
 */
export enum E_FILTER_TYPE {
    FILTER_DUPLICATE,
    FILTER_RATE_LIMIT,
    FILTER_PRIORITY,
    FILTER_LIFECYCLE,
    FILTER_CUSTOM
}

// --- Common Structs / Interfaces ---

/**
 * @brief Structure to hold Modbus registration details for a component.
 */
export interface MB_REGISTERS {
    /** Starting address (-1/0xFFFF indicates invalid/not set) */
    startAddress: number; // ushort
    /** Number of consecutive addresses */
    count: number; // ushort
    /** Slave ID of the device */
    slaveId: number; // ushort (Note: RTU_Base uses uint8_t, reconcile based on context)
    /** Type of Modbus object */
    type: E_FN_CODE; // E_FN_CODE
    /** Read/Write access (value likely from an E_ModbusAccess enum not provided) */
    access: number; // E_ModbusAccess (assumed number type in TS)
    /** ID of the owning component (MAY NOT BE SET in data returned by mb_tcp_blocks) */
    componentId: number; // ushort
    /** Optional descriptive name for the register block */
    name: string | null; // const char* (string in TS)
    /** Optional group name for the register block */
    group: string | null; // const char* (string in TS)
    // ComponentFnPtr? Callback pointer is C++ specific, omit or describe conceptually
    // writeCallbackFn: any; // Conceptual callback function pointer
}

/**
 * @brief Structure to hold a Modbus operation. Representing data structure for the operation queue.
 * Optimized memory layout in C++ not directly applicable to TS interface.
 */
export interface MB_OPERATION {
    timestamp: number; // unsigned long (millis())
    token: number;     // uint32_t, Unique token for tracking response
    address: number;   // uint16_t, Modbus address
    value: number;     // uint16_t, Value for write operations / first value for reads
    quantity: number;  // uint16_t, Number of registers/coils for read/write multiple
    slaveId: number;   // uint8_t
    retries: number;   // uint8_t, Retry counter
    /**
     * @brief Flags byte (e.g., used, high priority, in progress, broadcast, synchronized).
     * Use OP_... constants to check bits.
     */
    flags: number;     // uint8_t
    type: E_FN_CODE;   // E_FN_CODE
    status: E_MB_OP_STATUS; // E_MB_OpStatus

    // C++ uses bitwise flags, define these as conceptual properties or helpers if needed
    // isUsed(): boolean; // Check flags using OP_USED_BIT
    // isHighPriority(): boolean; // Check flags using OP_HIGH_PRIORITY_BIT
    // isInProgress(): boolean; // Check flags using OP_IN_PROGRESS_BIT
    // isBroadcast(): boolean; // Check flags using OP_BROADCAST_BIT
    // isSynchronized(): boolean; // Check flags using OP_SYNCHRONIZED_BIT
}

/**
 * @brief Structure to represent a register or coil value entry stored in cache (e.g., in MB_SLAVE_DATA).
 */
export interface MB_VALUE_ENTRY {
    lastUpdate: number; // unsigned long (millis())
    address: number;    // uint16_t
    value: number;      // uint16_t (0 or 1 for coils, actual value for registers)
    /**
     * @brief Flags byte (e.g., used, synchronized).
     * Use VALUE_... constants to check bits.
     */
    flags: number;      // uint8_t

    // C++ uses bitwise flags, define these as conceptual properties or helpers if needed
    // isUsed(): boolean; // Check flags using VALUE_USED_BIT
    // isSynchronized(): boolean; // Check flags using VALUE_SYNCHRONIZED_BIT
}


// --- Common Types (Aliases) ---

/**
 * @brief A non-owning view of a collection of MB_REGISTERS blocks.
 */
export interface T_MODBUS_BLOCK_VIEW {
    /** Pointer to the first block. Never null if count > 0. */
    data: MB_REGISTERS[] | null; // Representing pointer to array
    /** Total number of blocks in the view. */
    count: number; // int
}

/**
 * @brief Callback type for when a Modbus RTU response is received.
 */
export type T_RESPONSE_CALLBACK = (slaveId: number) => void; // uint8_t slaveId

/**
 * @brief Callback type for notification when a register/coil value changes.
 */
export type T_ON_REGISTER_CHANGE_CALLBACK = (op: MB_OPERATION, oldValue: number, newValue: number) => void; // const ModbusOperation &op, uint16_t oldValue, uint16_t newValue

/**
 * @brief Callback type for notification when a write operation completes.
 */
export type T_ON_WRITE_CALLBACK = (op: MB_OPERATION) => void; // const ModbusOperation &op

/**
 * @brief Callback type for notification when a Modbus error occurs.
 */
export type T_ON_ERROR_CALLBACK = (op: MB_OPERATION, errorCode: number, errorMessage: string | null) => void; // const ModbusOperation &op, int errorCode, const char *errorMessage

/**
 * @brief Callback type for checking if operation already exists (used by filters).
 */
export type T_OPERATION_EXISTS_CALLBACK = (op: MB_OPERATION, context: any | null) => boolean; // const ModbusOperation &op, void *context


// --- RTU Specific Enums ---

/**
 * @brief Device State for RTU_Base class.
 */
export enum E_DEVICE_STATE {
    UNINITIALIZED,
    INITIALIZING,
    IDLE,
    RUNNING,
    ERROR
}

/**
 * @brief Initialization state for ModbusRTU client.
 */
export enum E_INIT_STATE {
    INIT_NOT_STARTED,
    INIT_SERIAL_STARTED,
    INIT_CLIENT_STARTED,
    INIT_READY,
    INIT_FAILED
}

// --- RTU Specific Structs / Interfaces ---

/**
 * @brief Struct for passing RTU update data via onMessage(void*).
 * Used for synchronous message passing - data copied immediately by receiver.
 */
export interface MB_RTU_UPDATE_DATA {
    slaveId: number;       // uint8_t, Original RTU Slave ID
    address: number;      // uint16_t, RTU Address OR Calculated TCP Address (depending on context)
    value: number;         // uint16_t
    functionCode: E_FN_CODE; // E_FN_CODE
}

/**
 * @brief Structure to represent a Modbus slave's data cache (coils and registers) managed by ModbusRTU.
 */
export interface MB_SLAVE_DATA {
    /** Array of coil entries (size MAX_ADDRESSES_PER_SLAVE) */
    coils: MB_VALUE_ENTRY[];
    /** Array of register entries (size MAX_ADDRESSES_PER_SLAVE) */
    registers: MB_VALUE_ENTRY[];
    /** Number of used coil entries */
    coilCount: number;     // uint8_t
    /** Number of used register entries */
    registerCount: number; // uint8_t
}

/**
 * @brief Structure for defining mandatory read blocks in Modbus RTU (used by RTU_Base).
 */
export interface MB_READ_BLOCK {
    /** Starting address of the block */
    startAddress: number;      // uint16_t
    /** Number of registers/coils in the block */
    count: number;             // uint16_t
    /** Modbus function code for reading (e.g., FN_READ_HOLD_REGISTER) */
    type: E_FN_CODE;             // E_FN_CODE
    /** Minimum interval between reads (ms) */
    readInterval: number; // unsigned long
    /** Timestamp of the last read attempt (millis()) */
    lastReadTime: number; // unsigned long
    /**
     * @brief Status flags (e.g., used).
     * Use BLOCK_USED_BIT constant.
     */
    flags: number;              // uint8_t

    // conceptual helpers
    // isUsed(): boolean; // Check flags using BLOCK_USED_BIT
}

/**
 * @brief Register state information. Represents the state and configuration of a single Modbus register/coil managed by RTU_Base.
 * Note: This was a C++ class, extracting data members as a type interface.
 */
export interface T_REGISTER_STATE {
    /** Type of register/coil (e.g., FN_READ_HOLD_REGISTER, FN_READ_COIL) */
    type: E_FN_CODE; // E_FN_CODE (Note: C++ uses RegisterState::E_RegType here, mapping assumed to E_FN_CODE or similar)
    /** Modbus address */
    address: number;    // uint16_t
    /** Current value */
    value: number;      // uint16_t
    /** Priority level for operations related to this register */
    priority: number;   // uint8_t

    // C++ methods like getBoolValue/setBoolValue/readFromDevice/writeToDevice are implementation details and omitted from interface.
    // getBoolValue(): boolean;
    // setBoolValue(boolValue: boolean): void;
}


// --- TCP Specific Types ---
// No unique structs/enums specific only to TCP were found other than the class itself,
// which primarily uses the common types like MB_REGISTERS and T_MODBUS_BLOCK_VIEW.


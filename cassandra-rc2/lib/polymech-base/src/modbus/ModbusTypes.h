#ifndef MODBUS_TYPES_H
#define MODBUS_TYPES_H
#include <Arduino.h>
#include <ArduinoLog.h>
#include <Component.h>
#include <Vector.h>
#include <enums.h>
#include <net/commons.h>
#include "macros.h"
#include "constants.h"
#include "config-modbus.h"

enum E_FN_CODE : uint8_t
{
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
};

// Define Modbus operation types
enum E_MB_OpType : uint8_t
{
    MB_READ_COIL,
    MB_READ_REGISTER,
    MB_WRITE_COIL,
    MB_WRITE_REGISTER,
    MB_WRITE_MULTIPLE_REGISTERS
};

// Define operation status
enum E_MB_OpStatus : uint8_t
{
    MB_PENDING,
    MB_SUCCESS,
    MB_FAILED,
    MB_RETRYING
};

// Define operation error codes - incorporates internal errors and eModbus errors
enum class MB_Error : uint8_t
{
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
    BaudRateMismatch = 0xF0, // New error code for baud rate mismatch
    BroadcastError = 0xF1,
    UndefinedError = 0xFF // Other communication error
};

/**
 * @brief Structure to hold Modbus registration details for a component.
 */
struct MB_Registers
{
    ushort startAddress = -1;                 // Starting address (-1 indicates invalid/not set)
    ushort count = 0;                         // Number of consecutive addresses
    ushort slaveId = 0;                       // Slave ID of the device
    E_FN_CODE type = E_FN_CODE::FN_NONE;      // Type of Modbus object
    E_ModbusAccess access = MB_ACCESS_NONE;   // Read/Write access
    ushort componentId = 0;                   // ID of the owning component (MAY NOT BE SET in data returned by mb_tcp_blocks)
    const char *name = nullptr;               // Optional descriptive name for the register block
    const char *group = nullptr;              // Optional group name for the register block
    ComponentFnPtr writeCallbackFn = nullptr; // Optional component member function pointer for write callbacks
    void *userData = nullptr;                 // Optional user data pointer

    MB_Registers(ushort addr = 0xFFFF, // Use 0xFFFF for unsigned invalid marker
                 ushort ct = 0,
                 E_FN_CODE t = E_FN_CODE::FN_NONE,
                 E_ModbusAccess a = MB_ACCESS_NONE,
                 ushort cId = 0,
                 ushort sId = 0,
                 const char *n = nullptr,
                 const char *g = nullptr,
                 ComponentFnPtr cb = nullptr,
                 void *ud = nullptr)
        : startAddress(addr), count(ct), type(t), access(a), componentId(cId), slaveId(sId), name(n), group(g), writeCallbackFn(cb), userData(ud)
    {
    }
};

/**
 * @brief A non-owning view of a collection of MB_Registers blocks.
 */
struct ModbusBlockView
{
public:
    const MB_Registers *data; // Pointer to the first block. Never null if count > 0.
    int count;                // Total number of blocks in the view.
};

// Forward declarations
class ModbusRTU;
class ModbusOperation;

// Define filter types for type identification without RTTI
enum E_FilterType
{
    FILTER_DUPLICATE,
    FILTER_RATE_LIMIT,
    FILTER_PRIORITY,
    FILTER_LIFECYCLE,
    FILTER_CUSTOM
};

// Filter chain base class
class ModbusOperationFilter
{
public:
    ModbusOperationFilter() : nextFilter(nullptr) {}
    virtual ~ModbusOperationFilter() = default;

    // Returns true if operation should be queued, false if it should be dropped
    virtual bool filter(ModbusOperation &op) = 0;

    // Get the filter type
    virtual E_FilterType getType() const = 0;

    // Set the next filter in the chain
    void setNext(ModbusOperationFilter *next) { nextFilter = next; }

    // Get the next filter in the chain
    ModbusOperationFilter *getNext() const { return nextFilter; }

    // Process the operation through this filter and subsequent filters
    bool process(ModbusOperation &op)
    {
        if (!filter(op))
            return false;
        return nextFilter ? nextFilter->process(op) : true;
    }

    // Method to notify filter that an operation was executed
    // Used by filters that need to track operation status
    virtual void notifyOperationExecuted(const ModbusOperation &op) {}

    // Method to notify filter that an operation was completed
    // Used by filters that need to track operation status
    virtual void notifyOperationCompleted(const ModbusOperation &op) {}

private:
    ModbusOperationFilter *nextFilter;
};

// Structure to hold a Modbus operation
// Optimized memory layout: larger members first, booleans last
struct ModbusOperation
{
    unsigned long timestamp; // 4 bytes
    uint32_t token;          // 4 bytes
    uint16_t address;        // 2 bytes
    uint16_t value;          // 2 bytes
    uint16_t quantity;       // 2 bytes
    uint8_t slaveId;         // 1 byte
    uint8_t retries;         // 1 byte
    uint8_t flags;           // 1 byte for all boolean flags
    E_FN_CODE type;          // 1 byte enum
    E_MB_OpStatus status;    // 1 byte enum

    ModbusOperation()
        : timestamp(0), token(0), address(0), value(0), quantity(1),
          slaveId(0), retries(0), flags(0), type(E_FN_CODE::FN_READ_COIL), status(MB_PENDING) {}

    ModbusOperation(E_FN_CODE t, uint8_t s, uint16_t a, uint16_t v = 0, uint16_t q = 1, bool hp = false)
        : timestamp(millis()), token(0), address(a), value(v), quantity(q),
          slaveId(s), retries(0), flags(0), type(t), status(MB_PENDING)
    {
        if (hp)
            flags |= OP_HIGH_PRIORITY_BIT;
    }

    // Getter/setter methods for flags - Update to use TEST/SET_BIT_TO
    bool isUsed() const { return TEST(flags, OP_USED_BIT); }
    bool isHighPriority() const { return TEST(flags, OP_HIGH_PRIORITY_BIT); }
    bool isInProgress() const { return TEST(flags, OP_IN_PROGRESS_BIT); }
    bool isBroadcast() const { return TEST(flags, OP_BROADCAST_BIT); }
    bool isSynchronized() const { return TEST(flags, OP_SYNCHRONIZED_BIT); }

    void setUsed(bool value) { SET_BIT_TO(flags, OP_USED_BIT, value); }
    void setHighPriority(bool value) { SET_BIT_TO(flags, OP_HIGH_PRIORITY_BIT, value); }
    void setInProgress(bool value) { SET_BIT_TO(flags, OP_IN_PROGRESS_BIT, value); }
    void setBroadcast(bool value) { SET_BIT_TO(flags, OP_BROADCAST_BIT, value); }
    void setSynchronized(bool value) { SET_BIT_TO(flags, OP_SYNCHRONIZED_BIT, value); }
};

// Structure to represent a register or coil value entry
struct ModbusValueEntry
{
    unsigned long lastUpdate; // 4 bytes
    uint16_t address;         // 2 bytes
    uint16_t value;           // 2 bytes
    uint8_t flags;            // 1 byte for flags (used, synchronized)

    ModbusValueEntry()
        : lastUpdate(0), address(0), value(0), flags(0) {}

    ModbusValueEntry(uint16_t addr, uint16_t val, bool sync = true)
        : lastUpdate(millis()), address(addr), value(val), flags(0)
    {
        SBI(flags, OP_USED_BIT);
        if (sync)
            SBI(flags, OP_HIGH_PRIORITY_BIT);
    }
};

// Structure to represent a Modbus slave's data
struct SlaveData
{
    ModbusValueEntry coils[MAX_ADDRESSES_PER_SLAVE];
    ModbusValueEntry registers[MAX_ADDRESSES_PER_SLAVE];
    uint8_t coilCount;
    uint8_t registerCount;
    uint8_t slaveId;

    SlaveData() : coilCount(0), registerCount(0), slaveId(0)
    {
        clear();
    }

    void clear()
    {
        coilCount = 0;
        registerCount = 0;
        slaveId = 0;
        for (int i = 0; i < MAX_ADDRESSES_PER_SLAVE; ++i)
        {
            CBI(coils[i].flags, OP_USED_BIT);
            CBI(registers[i].flags, OP_USED_BIT);
            CBI(coils[i].flags, OP_HIGH_PRIORITY_BIT);
            CBI(registers[i].flags, OP_HIGH_PRIORITY_BIT);
        }
    }
};

// Add callback function type definition
typedef void (*ResponseCallback)(uint8_t slaveId);

// Callback function types for notifications
typedef void (*OnRegisterChangeCallback)(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue);
typedef void (*OnWriteCallback)(const ModbusOperation &op);
typedef void (*OnErrorCallback)(const ModbusOperation &op, int errorCode, const char *errorMessage);

// Struct for passing RTU update data via onMessage(void*)
// Used for synchronous message passing - data copied immediately by receiver.
struct MB_UpdateData
{
    uint8_t slaveId;  // Original RTU Slave ID
    uint16_t address; // RTU Address OR Calculated TCP Address (depending on context)
    uint16_t value;   // For single register writes
    uint16_t count;   // For multi-register writes
    void *userData;   // For multi-register data pointer
    uint16_t componentId;
    E_FN_CODE functionCode;
    E_PRIORITY priority;
};

// Struct for passing Protobuf update data via onMessage(void*)
struct PB_UpdateData
{
    uint8_t *data;
    size_t len;
    uint8_t componentId;
};

// Define empty callbacks for default behavior
inline void emptyRegisterChangeCallback(const ModbusOperation &, uint16_t, uint16_t) {}
inline void emptyWriteCallback(const ModbusOperation &) {}
inline void emptyErrorCallback(const ModbusOperation &, int, const char *) {}

// Function to convert Modbus Error enum to human-readable string
const char *modbusErrorToString(MB_Error error);

// Structure for defining mandatory read blocks
struct ModbusReadBlock
{
    uint16_t startAddress;      // Starting address of the block
    uint16_t count;             // Number of registers/coils in the block
    E_FN_CODE type;             // Modbus function code for reading (e.g., FN_READ_HOLD_REGISTER)
    unsigned long readInterval; // Minimum interval between reads (ms)
    unsigned long lastReadTime; // Timestamp of the last read attempt (millis())
    uint8_t flags;              // Status flags (e.g., used)

    ModbusReadBlock()
        : startAddress(0), count(0), type(E_FN_CODE::FN_NONE), readInterval(0), lastReadTime(0), flags(0) {}

    ModbusReadBlock(uint16_t start, uint16_t ct, E_FN_CODE t, unsigned long interval = 1000)
        : startAddress(start), count(ct), type(t), readInterval(interval), lastReadTime(0), flags(0)
    {
        SBI(flags, OP_USED_BIT);
    }

    bool isUsed() const { return TEST(flags, OP_USED_BIT); }
    void setUsed(bool value) { SET_BIT_TO(flags, OP_USED_BIT, value); }
};

// Base class for register states
class RegisterState
{
public:
    RegisterState(E_FN_CODE type, uint16_t address, uint16_t value = 0)
        : type(type), address(address), value(value),
          priority(PRIORITY_MEDIUM), dirty(true) {}

    E_FN_CODE type;
    uint16_t address;
    uint16_t value;
    uint8_t priority;
    bool dirty;

    bool getBoolValue() const
    {
        return (type == E_FN_CODE::FN_READ_COIL || type == E_FN_CODE::FN_READ_DISCR_INPUT) && value != 0;
    }

    void setBoolValue(bool boolValue)
    {
        // Only applicable for Coil types (writeable)
        if (type == E_FN_CODE::FN_WRITE_COIL)
        {
            value = boolValue ? 1 : 0; // Simplified - Modbus uses 0xFF00/0x0000 but internal might use 1/0
        }
    }

    // Read value from device
    MB_Error readFromDevice(ModbusRTU &manager, uint8_t slaveId);

    // Write value to device
    MB_Error writeToDevice(ModbusRTU &manager, uint8_t slaveId, bool forceWrite = false);

    // Print the current state
    void printState(ModbusRTU &manager, uint8_t slaveId);
};

class RTU_Base : public Component
{
public:
    // Device state enum
    typedef enum
    {
        UNINITIALIZED,
        INITIALIZING,
        IDLE,
        RUNNING,
        ERROR
    } E_DeviceState;

    // Constructor with device identification & owner
    RTU_Base(Component *owner, uint8_t _slaveId = 1, short _componentId = 1000)
        : Component("RTU_Base_Device", _componentId, COMPONENT_DEFAULT, owner),
          state(UNINITIALIZED),
          errorCount(0),
          lastResponseTime(0),
          responseTimeout(5000),
          lastSyncTime(0),
          syncInterval(10),
          componentId(_componentId),
          registerCount(0),
          lastErrorCode(0),
          mandatoryReadBlocks(readBlockStorage)
    {
        this->slaveId = _slaveId;
        // Initialize name more specifically if desired, e.g., after this->id is set by Component base
        // For now, Component base takes care of ID and a generic name if not overridden.
        // If a more specific name like "RTU_Device_sidXX_cidYY" is needed, do it after base construction.
        for (int i = 0; i < MAX_REGISTERS; ++i)
            registers[i] = nullptr;
    }

    virtual ~RTU_Base()
    {
        // Clean up register objects
        for (int i = 0; i < registerCount; i++)
        {
            if (registers[i] != nullptr)
            {
                delete registers[i];
                registers[i] = nullptr;
            }
        }
    }

    // Timing control
    unsigned long lastResponseTime;
    unsigned long responseTimeout;
    unsigned long lastSyncTime;
    unsigned long syncInterval;

    // Error tracking
    unsigned int errorCount;

    // Device identification
    // uint8_t slaveId; // REMOVED to resolve ambiguity with Component::slaveId

    // Component mapping
    uint16_t componentId;

    // State tracking
    E_DeviceState state;

    RegisterState *registers[MAX_REGISTERS];
    int registerCount;
    virtual const char *getStateString() const;

    virtual void setState(E_DeviceState newState);

    void handleResponseReceived();

    bool addInputRegister(uint16_t address, E_FN_CODE type, uint8_t priority = PRIORITY_MEDIUM)
    {
        if (registerCount >= MAX_REGISTERS)
        {
            Log.warningln("Max total registers (%d) reached for device %d", MAX_REGISTERS, slaveId);
            return false;
        }
        // Use appropriate E_FN_CODE for input register (assuming FN_READ_INPUT_REGISTER)
        RegisterState *reg = new RegisterState(type, address, 0);
        reg->priority = priority;
        reg->type = type;
        registers[registerCount++] = reg;
        return true;
    }

    // Add an output register using MAX_REGISTERS from constants.h
    bool addOutputRegister(uint16_t address, E_FN_CODE type, uint16_t defaultValue, uint8_t priority = PRIORITY_MEDIUM)
    {
        if (registerCount >= MAX_REGISTERS)
        {
            Log.warningln("Max total registers (%d) reached for device %d", MAX_REGISTERS, slaveId);
            return false;
        }
        // Use appropriate E_FN_CODE for holding register (assuming FN_WRITE_HOLD_REGISTER)
        RegisterState *reg = new RegisterState(type, address, defaultValue);
        reg->priority = priority;
        registers[registerCount++] = reg;
        return true;
    }

    // Update device state based on Modbus operations
    void updateState(ModbusRTU &manager);

    // Initialize state by writing output values to the device
    bool initialize(ModbusRTU &manager);

    // Read readable registers from device
    virtual void read(ModbusRTU &manager);

    // Get a pointer to a register by its address (const version)
    const RegisterState *getRegisterByAddress(uint16_t address) const;

    // Set a specific output register value by ADDRESS
    bool setOutputRegisterValue(uint16_t address, uint16_t value, bool force = false);

    virtual bool onRegisterUpdate(uint16_t address, uint16_t newValue);

    virtual void onError(ushort errorCode, const char *errorMessage);
    ushort mb_tcp_error(MB_Registers *reg) { return lastErrorCode; }

    // Write writable registers to the device (queues writes based on local state)
    virtual void write(ModbusRTU &manager);

    // Print current state values
    void printState();

    // Reset state
    virtual short reset() override;

    // Run a test sequence (Declaration might be removed if implementation is gone)
    // void runTestSequence(ModbusRTU& manager); // Keep commented/removed if impl is gone

    // --- Modbus TCP Mapping Support ---
    /**
     * @brief Gets the base Modbus TCP address allocated for this RTU device instance.
     *
     * This is the starting address in the TCP address space from which the
     * device's own TCP register offsets are calculated.
     *
     * @return The base TCP address for this device instance.
     *         Returns 0 if TCP mapping is not applicable or configured.
     */
    virtual uint16_t mb_tcp_base_address() const { return 0; } // Default: No TCP mapping

    /**
     * @brief Calculates the Modbus TCP offset corresponding to a given RTU address update.
     *
     * This function is intended to map an address from the RTU bus (which triggered an update)
     * back to the corresponding offset within the device's allocated TCP address block.
     *
     * @param rtuAddress The RTU register address that was updated.
     * @return The corresponding TCP offset (relative to mb_tcp_base_address()), or 0 if no direct mapping exists for broadcast.
     */
    virtual uint16_t mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const { return 0; } // Default: No mapping

    // --- End Modbus TCP Mapping Support ---

    // Add a mandatory read block definition
    ModbusReadBlock *addMandatoryReadBlock(uint16_t startAddress, uint16_t count, E_FN_CODE type, unsigned long interval = 300);

    virtual bool isHighPriority(const ModbusOperation &op) const { return false; }

    uint16_t lastErrorCode;
    uint16_t getLastErrorCode() const { return lastErrorCode; }

    bool triggerRTUWrite();

protected:
private:
    // Storage for the mandatory read blocks Vector
    ModbusReadBlock readBlockStorage[MAX_READ_BLOCKS];
    // Vector to hold mandatory read block definitions
    Vector<ModbusReadBlock> mandatoryReadBlocks;
};

// Callback type for checking if operation already exists
typedef bool (*OperationExistsCallback)(const ModbusOperation &op, void *context);

// Filter that removes duplicate operations
class DuplicateOperationFilter : public ModbusOperationFilter
{
public:
    // Update constructor to take ModbusRTU reference
    DuplicateOperationFilter(ModbusRTU *rtu);
    ~DuplicateOperationFilter();

    bool filter(ModbusOperation &op) override;

    // These methods aren't needed anymore as we'll directly check the queues
    void notifyOperationExecuted(const ModbusOperation &op) override {}
    void notifyOperationCompleted(const ModbusOperation &op) override {}

    // Get the filter type
    E_FilterType getType() const override { return FILTER_DUPLICATE; }

private:
    // Check if operation is already in the ModbusRTU queues
    bool isOperationAlreadyPending(const ModbusOperation &op) const;

    // Reference to ModbusRTU for queue access
    ModbusRTU *modbusRTU;

    // Timeout for considering operations as duplicates
    unsigned long operationTimeout;
};

// Filter that limits the rate of operations
class RateLimitFilter : public ModbusOperationFilter
{
public:
    RateLimitFilter(unsigned long minIntervalMs = FILTER_RATE_LIMIT_MIN_INTERVAL);

    bool filter(ModbusOperation &op) override;

    // Get the filter type
    E_FilterType getType() const override { return FILTER_RATE_LIMIT; }

private:
    unsigned long minInterval;
    unsigned long lastOperationTime;
};

// Filter that prioritizes operations based on type or address
class PriorityFilter : public ModbusOperationFilter
{
public:
    PriorityFilter();

    bool filter(ModbusOperation &op) override;

    // Get the filter type
    E_FilterType getType() const override { return FILTER_PRIORITY; }

    // Adjusts the priority based on operation properties
    // Returns true to continue processing, false to abort
    // This doesn't change filtering, but could modify the op
    bool adjustPriority(ModbusOperation &op);
};

// Filter that handles operation lifecycle (expiration, retries)
class OperationLifecycleFilter : public ModbusOperationFilter
{
public:
    OperationLifecycleFilter(unsigned long timeoutMs = OPERATION_TIMEOUT, uint8_t maxRetries = MAX_RETRIES)
        : timeout(timeoutMs), maxRetries(maxRetries) {}

    bool filter(ModbusOperation &op) override
    {
        if (op.retries >= maxRetries)
        {
            return false;
        }

        unsigned long now = millis();
        // Give in-progress operations a grace period to allow for callback execution
        // This prevents the operation from being removed before the client reports the timeout
        unsigned long effectiveTimeout = timeout;
        if (op.isInProgress())
        {
            effectiveTimeout += timeout; // 5 seconds grace period
        }

        if (now - op.timestamp > effectiveTimeout)
        {
            return false;
        }

        return true; // Let operation through
    }

    E_FilterType getType() const override { return FILTER_LIFECYCLE; }

private:
    unsigned long timeout;
    uint8_t maxRetries;
};

#endif // MODBUS_TYPES_H

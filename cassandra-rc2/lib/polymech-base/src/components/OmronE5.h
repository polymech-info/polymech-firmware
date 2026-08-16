#ifndef OMRON_E5_H
#define OMRON_E5_H

#include "config.h"

#ifdef ENABLE_RS485

#include <ArduinoLog.h>
#include <Component.h>
#include <modbus/ModbusRTU.h>
#include <modbus/ModbusComponent.h>
#include "components/OmronE5Types.h"
#include <xstatistics.h>
#include <NetworkValue.h>
#include "config-modbus.h"

using OmronValue = NetworkValue<uint16_t>;
using OmronBoolValue = NetworkValue<bool>;

#define HEATUP_DEADBAND 5
#define OMRON_RUN_STOP_THROTTLE_MS 250

#define OMRON_E5_READ_BLOCK_START_ADDR 0x0000
#define OMRON_E5_READ_BLOCK_REG_COUNT 6

enum class E_OmronRtuAddress : uint16_t
{
    PV = OMRON_E5_READ_BLOCK_START_ADDR + 1,
    STATUS_HIGH = OMRON_E5_READ_BLOCK_START_ADDR + 2,
    STATUS_LOW = OMRON_E5_READ_BLOCK_START_ADDR + 3,
    SP = OMRON_E5_READ_BLOCK_START_ADDR + 5
};

enum class E_ExecuteCommands : short
{
    INFO = 1,
    RESET_STATS = 2
};

enum class E_ERROR_OMRON_E5 : short
{
    E_OMRON_ERROR_NONE = 0,
    E_OMRON_ERROR_INVALID_PV = 1,
    E_OMRON_ERROR_MB_ERROR = 2,
};

enum class E_OmronTcpOffset : ushort
{
    PV = 1,
    STATUS_HIGH = 2,
    STATUS_LOW = 3,
    SP = 5,
    CMD_SP = 11,
    CMD_STOP = 12,
    CMD_EXECUTE = 13, // Write 1 for info, 2 for reset stats
    IS_HEATING = 14,  // Read-only status: 1 if heating, 0 otherwise
    IS_HEATUP = 16,   // Read-only status: 1 if PV < SP - Deadband
    CMD_EXECUTE_INFO = 17,
    ENABLED = 18,        // R/W Coil to enable/disable the component
    CMD_COMMS_WRITE = 19 // R/W Coil to enable/disable communications writing
};

class OmronE5 : public RTU_Base
{
public:
    // Calculate the number of TCP blocks based on conditional compilation
    static constexpr int OMRON_TCP_BLOCK_COUNT = 18;

    // Constructor
    OmronE5(Component *owner, uint8_t slaveId, millis_t readInterval = OMRON_E5_READ_BLOCK_INTERVAL);
    virtual ~OmronE5() = default;

    // --- Component Interface ---
    virtual short setup() override;
    virtual short loop() override;
    virtual short info() override;
    virtual void enable(bool enabled) override;

    // --- Modbus Register Update Notification ---
    virtual bool onRegisterUpdate(uint16_t address, uint16_t newValue) override; // Override base method

    // --- Getters for Specific Values ---
    bool getPV(uint16_t &value) const;
    bool getSP(uint16_t &value) const;
    bool getStatusLow(uint16_t &value) const;
    bool getStatusHigh(uint16_t &value) const;
    bool isRunning() const;
    bool isHeating() const;
    bool isCooling() const;
    bool isAutoTuning() const;
    bool isCommsWritingEnabled() const;
    bool isHeatup() const;
    uint32_t getTotalWh() const;
    // --- Setters (Optional - Implement if needed) ---
    bool setSP(uint16_t value);
    bool run(bool force = false);
    short stop() override;
    short stop(bool force);
    bool setCommsWriting(bool enabled);
    uint32_t getConsumption() const;

#ifdef ENABLE_COOLING
    // --- Cooling Specific Getters ---
    // Requires ENABLE_COOLING define in OmronE5.cpp and manual Omron device setup for cooling.
    /**
     * @brief Gets the raw 32-bit value of the Cooling Manipulated Variable (MV) monitor.
     * Requires reading registers 0x2005 (Low) and 0x2006 (High).
     * @param value Reference to store the combined 32-bit value.
     * @return True if the value is valid (has been successfully read), false otherwise.
     */
    bool getCoolingMVRaw(uint32_t &value) const;
    // --- Cooling State (Requires ENABLE_COOLING in .cpp & manual device setup) ---
    uint16_t _currentCoolingMVLow = 0;
    uint16_t _currentCoolingMVHigh = 0;
    bool _coolingMvValid = false;
#endif

    // --- Modbus Block Definitions ---
    virtual ModbusBlockView *mb_tcp_blocks() const override;
    virtual short mb_tcp_read(MB_Registers *reg) override;
    virtual short mb_tcp_write(MB_Registers *reg, short value) override;

    // --- Modbus TCP Mapping Overrides ---
    /**
     * @brief Gets the base Modbus TCP address allocated for this RTU device instance.
     * @return The base TCP address for this device instance.
     */
    uint16_t mb_tcp_base_address() const override;

    /**
     * @brief Calculates the Modbus TCP offset corresponding to a given RTU address update.
     * @param rtuAddress The RTU register address that was updated.
     * @return The corresponding TCP offset (relative to mb_tcp_base_address), or 0 if no direct mapping exists for broadcast.
     */
    uint16_t mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const override;

    /**
     * @brief Gets the current error state of the Omron E5 device.
     * @return The current error state.
     */
    E_ERROR_OMRON_E5 getError() { return _error; };
    /**
     * @brief Checks if the Omron E5 device has an error.
     * @return True if the device has an error, false otherwise.
     */
    bool hasError() { return _error != E_ERROR_OMRON_E5::E_OMRON_ERROR_NONE || errorCount > 0; };

    void onError(ushort errorCode, const char *errorMessage) override;

    // --- Public NetworkValue members ---
    OmronValue m_pv;
    OmronValue m_sp;
    OmronValue m_statusLow;
    OmronValue m_statusHigh;
    OmronBoolValue m_runState; // true = running, false = stopped
    OmronBoolValue m_enabled;
    OmronBoolValue m_commsWritingEnabled;

private:
    void _log_binary(const char *prefix, uint16_t value);
    millis_t _readInterval;

    // --- Local State Storage ---
    E_ERROR_OMRON_E5 _error;

    bool _initialSpSet;
    bool _statusLowReceived = false;
    bool _statusHighReceived = false;
    uint32_t _consumption = 2700;

    ModbusComponent<OMRON_TCP_BLOCK_COUNT> m_modbusHelper;

    bool _wasHeating = false;
    millis_t _heatOnStartTime = 0;
    uint32_t _totalWs = 0;

    millis_t _lastHeatingLoopTime = 0; // Timestamp for continuous Wh calculation
    /**
     * @brief Resets all runtime statistics and timestamps used by the truth collector.
     * Triggered by writing 2 to the CMD_EXECUTE register.
     */
    void _resetRuntimeStats();

private:
    void _sendOperationCommand(OR_E5_OPERATION_CODE code, uint8_t data);
    millis_t _lastRunStopCmdTime = 0;
};

#endif // ENABLE_RS485
#endif // OMRON_E5_H
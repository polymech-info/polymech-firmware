#include "config.h"

#include <Logger.h>
#include "error_codes.h"
#include "components/OmronE5.h"
#include "components/OmronE5Types.h"
#include <modbus/ModbusTypes.h>
#include <modbus/Modbus.h>
#include "RS485.h"
#include "pid_constants.h"
#include <xstatistics.h>

#define HEATUP_DEADBAND 15 // Fixed deadband value for heatup detection, percentage
#define OMRON_E5_SP_MAX 260   // Maximum allowed Set Point value
#define OMRON_E5_PV_SP_MAX_RANGE 400 // Maximum range for PV and SP in heatup check

// --- Cooling Feature Setup Instructions ---
// To enable cooling monitoring:
// 1. Uncomment the #define ENABLE_COOLING above.
// 2. Ensure the Omron E5 controller is configured for Heat/Cool control
//    (Register 0x2D11 = 1) and has an output assigned to cooling 
//    (e.g., Register 0x2E06 or 0x2E07 = 2) via its own settings menu
//    or separate configuration tool. This code only reads the cooling MV.
// --- End Setup Instructions ---

#ifdef ENABLE_RS485

OmronE5::OmronE5(Component* owner, uint8_t slaveId, millis_t readInterval) 
    : RTU_Base(owner, slaveId, COMPONENT_KEY::COMPONENT_KEY_PID + slaveId),
      _readInterval(readInterval),
      _pvValid(false),
      _spValid(false),
      _statusValid(false),
      _runStateValue(owner, id, "OmronE5 Run/Stop"),
      _runStateWrapper(
          INIT_COMPONENT_VALUE_WRAPPER(
              bool,
              E_OmronTcpOffset::CMD_STOP,
              E_FN_CODE::FN_WRITE_COIL,
              false,
              true,
              ValueWrapper<bool>::ThresholdMode::DIFFERENCE,
              [this](const bool& newValue, const bool& oldValue) {
                  Log.infoln("OmronE5[%d]::runStateWrapper - Run state changed from %d to %d", this->slaveId, oldValue, newValue);
              }
          )
      )
{
    _runStateValue.enableFeature(E_NetworkValueFeatureFlags::E_NVFF_ALL);
    _runStateValue.initNotify(
        false, 1, NetworkValue_ThresholdMode::DIFFERENCE,
        [this](const bool& oldValue, const bool& newValue) {
            Log.infoln("OmronE5[%d]::runStateValue - Run state changed from %d to %d", this->slaveId, oldValue, newValue);
        }
    );
    syncInterval = readInterval;
    type = COMPONENT_TYPE::COMPONENT_TYPE_PID;
    name = "OmronE5[" + String(slaveId) + "]";
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    const uint16_t tcpBaseAddr = mb_tcp_base_address();

    _modbusBlocks[0] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::PV, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "PV", name.c_str());
    _modbusBlocks[1] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::SP, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SP", name.c_str());
    _modbusBlocks[2] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::STATUS_LOW, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Status Low", name.c_str());
    _modbusBlocks[3] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::STATUS_HIGH, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Status High", name.c_str());
    _modbusBlocks[4] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::CMD_SP, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "SP CMD", name.c_str());
    _modbusBlocks[5] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::CMD_STOP, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, "Run/Stop Coil", name.c_str());
    _modbusBlocks[6] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::MEAN_ERROR, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Mean Error (0-100)", name.c_str());
    _modbusBlocks[7] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::HEAT_RATE, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Heat Rate (osc/min)", name.c_str());
    _modbusBlocks[8] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::WH_LOW, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Total Wh (Low)", name.c_str());
    _modbusBlocks[9] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::WH_HIGH, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Total Wh (High)", name.c_str());
    _modbusBlocks[10] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::PV_SP_LAG, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "PV SP Lag (tenths/min)", name.c_str());
    _modbusBlocks[11] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::CMD_EXECUTE, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "Execute Command", name.c_str());
    _modbusBlocks[12] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::TOTAL_COST_CENTS, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Total Cost (Cents)", name.c_str());
    _modbusBlocks[13] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::IS_HEATING, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Heating Status", name.c_str());
    _modbusBlocks[14] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::LONGEST_HEAT_60S, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Longest Heat (s)", name.c_str());
    _modbusBlocks[15] = INIT_MODBUS_BLOCK(E_OmronTcpOffset::IS_HEATUP, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Heatup Status", name.c_str());
// Add static_assert *after* potential conditional block definitions
static_assert(sizeof(_modbusBlocks)/sizeof(_modbusBlocks[0]) == OmronE5::OMRON_TCP_BLOCK_COUNT, "Mismatch in _modbusBlocks size and OMRON_TCP_BLOCK_COUNT");
    _modbusBlockView = {_modbusBlocks, OMRON_TCP_BLOCK_COUNT};
}

uint16_t OmronE5::mb_tcp_base_address() const
{
    // Calculate and return the base TCP address for this specific Omron E5 instance
    return OMRON_MB_TCP_OFFSET + (this->slaveId * OMRON_TCP_BLOCK_COUNT);
}

uint16_t OmronE5::mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const
{
    switch (rtuAddress)
    {
        case OMRON_E5_READ_BLOCK_START_ADDR + 1: // PV (likely 0x0001)
            return static_cast<uint16_t>(E_OmronTcpOffset::PV);
        case OMRON_E5_READ_BLOCK_START_ADDR + 2: // Status High (likely 0x0002)
            return static_cast<uint16_t>(E_OmronTcpOffset::STATUS_HIGH);
        case OMRON_E5_READ_BLOCK_START_ADDR + 3: // Status Low (likely 0x0003)
            return static_cast<uint16_t>(E_OmronTcpOffset::STATUS_LOW);
        case OMRON_E5_READ_BLOCK_START_ADDR + 5: // SP (likely 0x0005)
            return static_cast<uint16_t>(E_OmronTcpOffset::SP);
         #ifdef ENABLE_COOLING
        #endif
        default:
            // Return 0 or an invalid offset marker if the RTU address doesn't map directly
            // to a defined TCP offset for broadcast purposes.
            return 0;
    }
}

short OmronE5::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
    {
        Log.errorln(F("OmronE5[%d]::mb_tcp_read - Invalid MB_Registers pointer"), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure; // Or E_INVALID_PARAMETER
    }

    // Use the new virtual function to get the base address for this instance
    const uint16_t instanceBaseAddr = this->mb_tcp_base_address();
    if (instanceBaseAddr == 0) { // Handle cases where TCP mapping might not be configured
        Log.errorln(F("OmronE5[%d]::mb_tcp_read - TCP Base Address is 0, cannot process read."), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const short requestedAddress = reg->startAddress;
    // Calculate offset from the instance's base TCP address
    short offset = requestedAddress - instanceBaseAddr;

    // Check if the calculated offset is positive.
    // The switch statement's default case will handle specific unmapped offsets.
    if (offset < 1)
    {
        Log.warningln(F("OmronE5[%d]: Received read request for address %d which maps to a non-positive offset %d. Base: %d"),
                      this->slaveId, requestedAddress, offset, instanceBaseAddr);
        return (short)MB_Error::IllegalDataAddress;
    }

    // Offset is now guaranteed to be valid (1 to 16)
    uint16_t value = 0;
    bool success = false;
    // Cast the valid offset to the enum type for use in the switch
    E_OmronTcpOffset regOffset = static_cast<E_OmronTcpOffset>(offset);

    // Handle reads based on the specific register offset
    switch (regOffset)
    {
    case E_OmronTcpOffset::PV:
        success = getPV(value);
        break;
    case E_OmronTcpOffset::SP:
        success = getSP(value);
        break;
    case E_OmronTcpOffset::STATUS_LOW: 
        success = getStatusLow(value);
        break;
    case E_OmronTcpOffset::STATUS_HIGH:
        success = getStatusHigh(value);
        break;    
#ifdef ENABLE_TRUTH_COLLECTOR
    case E_OmronTcpOffset::MEAN_ERROR: // Offset 4: Scaled Mean Error
        {
            float meanError = getMeanError();
            if (_errorStats.count() == 0 || isnan(meanError)) {
                value = 0; // Return 0 if no data or NaN
            } else {
                const float MAX_ERROR_FOR_SCALING = 100.0f;
                // Clamp the error between 0 and MAX_ERROR_FOR_SCALING
                float clampedError = meanError < 0.0f ? 0.0f : (meanError > MAX_ERROR_FOR_SCALING ? MAX_ERROR_FOR_SCALING : meanError);
                // Scale to 0-100
                value = static_cast<uint16_t>((clampedError / MAX_ERROR_FOR_SCALING) * 100.0f);
            }
            success = true; // Reading the statistic is considered successful
        }
        break;
    case E_OmronTcpOffset::HEAT_RATE: // Offset 6: Heat Rate (osc/min)
        {
            float rate = getHeatRate();
            value = static_cast<uint16_t>(rate); // Cast float rate to integer
            success = true; // Reading the statistic is considered successful
        }
        break;
    case E_OmronTcpOffset::WH_LOW: // Offset 7: Total Wh Low Word
        {
            uint32_t totalWhInt = static_cast<uint32_t>(_totalWh);
            value = totalWhInt & 0xFFFF; // Lower 16 bits
            success = true;
        }
        break;
    case E_OmronTcpOffset::WH_HIGH: // Offset 8: Total Wh High Word
        {
            uint32_t totalWhInt = static_cast<uint32_t>(_totalWh);
            value = (totalWhInt >> 16) & 0xFFFF; // Upper 16 bits
            success = true;
        }
        break;
    case E_OmronTcpOffset::PV_SP_LAG: // Offset 9: PV SP Lag (Tenths/min)
        value = static_cast<uint16_t>(_pvSpLag); // Return the stored lag (cast from int16_t)
        success = true; // Reading the calculated value is considered successful
        break;
    case E_OmronTcpOffset::TOTAL_COST_CENTS: // Offset 10: Total Cost in Cents
        {
            float totalKWh = static_cast<float>(static_cast<uint32_t>(_totalWh)) / 1000.0f;
            float totalCostEuros = totalKWh * PRICE_PER_KWH_EUROS;
            // Convert to cents, ensuring it fits within uint16_t (max 65535 cents = 655.35 EUR)
            float totalCostCentsFloat = totalCostEuros * 100.0f;
            if (totalCostCentsFloat > 65535.0f) {
                value = 65535; // Clamp to max value if overflow
            } else if (totalCostCentsFloat < 0.0f) {
                value = 0; // Ensure non-negative
            } else {
                value = static_cast<uint16_t>(totalCostCentsFloat);
            }
            success = true; // Calculation is considered successful
        }
        break;
    case E_OmronTcpOffset::LONGEST_HEAT_60S: // Offset 15: Longest Heat duration in last 60s
        value = getLongestHeatDuration60s();
        success = true; // Reading the tracked value is successful
        break;
#endif
    case E_OmronTcpOffset::CMD_SP: // Offset 11: Read associated with write SP command (returns current SP)
        success = getSP(value); // Read the actual SP
        break;
    case E_OmronTcpOffset::CMD_STOP: // Offset 12: Read associated with Run/Stop command
        value = !isRunning();
        success = true; 
        break;
    case E_OmronTcpOffset::CMD_EXECUTE: // Offset 13: Reading this trigger always returns 0
        value = 0;
        success = true;
        break;
    case E_OmronTcpOffset::IS_HEATING: // Offset 14: Heating Status
        value = isHeating() ? 1 : 0;
        success = true;
        break;
    case E_OmronTcpOffset::IS_HEATUP: // Offset 16: Heatup Status
        value = isHeatup() ? 1 : 0;
        success = true; // Checking the status is always considered successful
        break;
    case E_OmronTcpOffset::CMD_EXECUTE_INFO: // Offset 17: Reading this trigger always returns 0
        value = 0;
        success = true;
        break;

    default:
        // This case handles offsets that are within the allocated range [0, OMRON_TCP_BLOCK_COUNT)
        // but are not explicitly mapped to a readable value in the E_OmronTcpOffset enum.
        // (e.g., offsets 0, 4, 6, 7, 8, 9, 10, 13, 14, 15 in a range of 16)
        Log.warningln(F("OmronE5[%d]: Read attempt on unhandled offset %d within TCP block (Address: %d)."), this->slaveId, offset, requestedAddress);
        // Treat unmapped but valid addresses within the block as illegal data addresses for read operations.
        return (short)MB_Error::IllegalDataAddress;
    }

    // Check if the underlying getter function succeeded (e.g., data was valid)
    if (!success)
    {
        // Log.warningln(F("OmronE5[%d]: Failed to get valid data for TCP Address: %d, Offset: %d (underlying data invalid or unavailable)."), this->slaveId, requestedAddress, offset);
        // Return a value indicating failure to retrieve valid data from the device.
        // 0xFFFF is a common way to signal an error/invalid state in Modbus register reads
        // when no specific Modbus error code perfectly fits.
        return 0xFFFF; // Indicate data not available/valid from the source
    }

    // Return the successfully retrieved value
    return value;
}

short OmronE5::mb_tcp_write(MB_Registers *reg, short value)
{
    if (!reg)
        return E_INVALID_PARAMETER;

    // Use the new virtual function to get the base address for this instance
    const uint16_t tcpBaseAddr = this->mb_tcp_base_address();
    if (tcpBaseAddr == 0) { // Handle cases where TCP mapping might not be configured
        Log.errorln(F("OmronE5[%d]::mb_tcp_write - TCP Base Address is 0, cannot process write."), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const short requestedTcpAddress = reg->startAddress;
    // Calculate offset from the instance's base TCP address
    short offset = requestedTcpAddress - tcpBaseAddr;

    // Use the calculated offset (cast to enum) for checks
    E_OmronTcpOffset regOffset = static_cast<E_OmronTcpOffset>(offset);

    bool commandSuccess = false;

    // Check against the enum offset directly
    if (regOffset == E_OmronTcpOffset::CMD_SP)
    {
        // Clamp the value before setting
        uint16_t clampedValue = (uint16_t)value > OMRON_E5_SP_MAX ? OMRON_E5_SP_MAX : (uint16_t)value;
        if (clampedValue != (uint16_t)value) {
            Log.warningln(F("OmronE5[%d]: Requested SP %d clamped to %d"), this->slaveId, value, clampedValue);
        }
        commandSuccess = setSP(clampedValue);
    }
    else if (regOffset == E_OmronTcpOffset::CMD_STOP)
    {
        if (value) // Value 1 = Stop, Value 0 = Run
        {
            Log.infoln(F("OmronE5[%d]::mb_tcp_write - Received stop command via TCP Coil (Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
            commandSuccess = stop();
        }
        else
        {
            Log.infoln(F("OmronE5[%d]::mb_tcp_write - Received run command via TCP Coil (Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
            commandSuccess = run();
        }
    }
    else if (regOffset == E_OmronTcpOffset::CMD_EXECUTE)
    {
        // Cast the incoming value to the command enum
        E_ExecuteCommands command = static_cast<E_ExecuteCommands>(value);

        switch (command)
        {
        case E_ExecuteCommands::INFO:
            Log.infoln(F("OmronE5[%d]::mb_tcp_write - Received info() command via TCP Register (Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
            this->info(); // Execute the info method
            commandSuccess = true; // Command execution was successful
            break;
#ifdef ENABLE_TRUTH_COLLECTOR
        case E_ExecuteCommands::RESET_STATS:
            Log.infoln(F("OmronE5[%d]::mb_tcp_write - Received reset stats command via TCP Register (Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
            _resetRuntimeStats(); // Execute the reset method
            commandSuccess = true; // Command execution was successful
            break;
#endif
        default:
            // Writing any other value to the execute register is not an error, just does nothing specific
            Log.warningln(F("OmronE5[%d]::mb_tcp_write - Received non-command value for CMD_EXECUTE (Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
            commandSuccess = true; // Writing a non-trigger value is also considered successful
            break;
        }
        // Note: No persistent state change or RTU write needed for CMD_EXECUTE
    }
    else if (regOffset == E_OmronTcpOffset::CMD_EXECUTE_INFO)
    {
        // Any write value to CMD_EXECUTE_INFO (Offset 17) triggers the info dump.
        Log.infoln(F("OmronE5[%d]::mb_tcp_write - Received info() command via TCP Register (CMD_EXECUTE_INFO, Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
        this->info(); // Execute the info method
        commandSuccess = true; // Command execution was successful
    }
    else
    {
        // Handle attempt to write to a non-writable/unhandled offset
        Log.warningln(F("OmronE5[%d]::mb_tcp_write - Attempt to write to unhandled or read-only TCP Address: %d (Offset: %d)"), this->slaveId, requestedTcpAddress, offset);
        return (short)MB_Error::IllegalDataAddress; // Or E_ACCESS_DENIED if it's read-only
    }

    if (!commandSuccess)
    {
        Log.errorln(F("OmronE5[%d]::mb_tcp_write - Failed to execute command for TCP Addr %d, Value %d"), this->slaveId, requestedTcpAddress, value);
        return (short)MB_Error::ServerDeviceFailure; // Indicate command execution failed
    }

    return (short)MB_Error::Success; // Return OK to ModbusTCP, actual write happens later
}

ModbusBlockView *OmronE5::mb_tcp_blocks() const
{
    return const_cast<ModbusBlockView*>(&_modbusBlockView);
}
short OmronE5::setup()
{
    // --- Mandatory Block 1: Core Status/PV/SP --- 
    ModbusReadBlock *block = addMandatoryReadBlock(
        OMRON_E5_READ_BLOCK_START_ADDR, // 0x0000
        OMRON_E5_READ_BLOCK_REG_COUNT,  // 6 registers (PV, StatusH, StatusL, SP)
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        _readInterval);
    if (!block)
    {
        Log.errorln(F("OmronE5[%d]: Failed to add mandatory read block 1!"), slaveId);
        return E_INVALID_PARAMETERS;
    }

#ifdef ENABLE_COOLING
    // --- Mandatory Block 2: Cooling MV Monitor --- 
    // Read E_MV_MONITOR_COOL_REGISTER (0x2005) - Requires reading 2 registers (0x2005, 0x2006)
    ModbusReadBlock *coolBlock = addMandatoryReadBlock(
        0x2005, // Start address for Cooling MV Low Word
        2,      // Read 2 registers (Low and High words)
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        _readInterval); // Use same interval for simplicity, adjust if needed
    if (coolBlock)
    {
        Log.infoln(F("OmronE5[%d]: Configured mandatory read block 2 (Cooling MV): Addr=0x%04X, Count=%d, Interval=%lu ms"),
                   slaveId, 0x2005, 2, _readInterval);
    }
    else
    {
        Log.errorln(F("OmronE5[%d]: Failed to add mandatory read block 2 (Cooling MV)!"), slaveId);
        // Decide if this is critical, potentially return error
        // return E_INVALID_PARAMETERS;
    }
#endif

    this->addOutputRegister(OR_E5_SWR_SP, E_FN_CODE::FN_WRITE_HOLD_REGISTER, 10, PRIORITY_MEDIUM);
    this->addOutputRegister(OR_E5_CMD_ADDRESS::OR_E5_CMD_STOP_RUN, E_FN_CODE::FN_WRITE_HOLD_REGISTER, 0, PRIORITY_MEDIUM);
    return E_OK;
}
short OmronE5::loop()
{
    bool currentlyRunning = isRunning();
    _runStateValue.update(!currentlyRunning);
    _runStateWrapper.update(!currentlyRunning);
    // Most of the work (scheduling reads/writes) is handled by the RS485->RTU_DeviceManager.
    // The RTU_Device base class updates its internal register map when reads complete.
    // This loop could be used for:
    // 1. Triggering specific writes based on application logic.
    // 2. Checking status flags from the read data and acting upon them.
    // 3. Implementing component-specific logic not directly tied to Modbus reads/writes.

#ifdef ENABLE_TRUTH_COLLECTOR
    millis_t now = millis();
    const millis_t WINDOW_DURATION_MS = 60000 * 5; 

    // --- PV Rate of Change Calculation ---
    if (_pvValid) {
        if (_hasPreviousPv) {
            millis_t deltaTime = now - _lastPvUpdateTime;
            if (deltaTime > 0) { // Avoid division by zero and ensure time has passed
                int32_t deltaPV = (int32_t)_currentPV - (int32_t)_previousPV; // Use int32_t for intermediate calculation
                // Calculate rate in Tenths of PV Unit per Minute
                float ratePerMinute = (static_cast<float>(deltaPV) * 60000.0f) / static_cast<float>(deltaTime);
                // Clamp to int16_t range
                if (ratePerMinute > 32767.0f) ratePerMinute = 32767.0f;
                if (ratePerMinute < -32768.0f) ratePerMinute = -32768.0f;
                _pvSpLag = static_cast<int16_t>(ratePerMinute);
            } // else deltaTime is 0, keep previous rate
        } else {
            _pvSpLag = 0; // No rate calculable yet
            _hasPreviousPv = true; // Mark that we now have a previous value for the next iteration
        }
        _previousPV = _currentPV;
        _lastPvUpdateTime = now;
    } else {
        // If PV is not valid, reset the rate calculation state
        _pvSpLag = 0;
        _hasPreviousPv = false;
    }

    // --- Error Statistics ---
    // Calculate and add error to statistics if PV and SP are valid
    if (_pvValid && _spValid) {
        float error = abs((int16_t)_currentPV - (int16_t)_currentSP); // Calculate absolute error
        _errorStats.add(error);
    }

    // --- Heating Interval Statistics and Wh Calculation ---
    bool heatingNow = isHeating();
    // Note: 'now' should be defined earlier in the #ifdef block, e.g., millis_t now = millis();

    // Track heating intervals for rate calculation
    if (!_wasHeating && heatingNow) { // Heating just started
        _heatOnStartTime = now;
    } else if (_wasHeating && !heatingNow) { // Heating just stopped
        millis_t duration = now - _heatOnStartTime;
        if (duration > 0) { // Avoid adding zero or negative duration
             _heatingIntervalStats.add(duration);
        }
    }

    // --- Longest Heating Duration in 60s Window ---
    // Check if the 60s window needs to reset
    if (now - _windowStartTime >= WINDOW_DURATION_MS) {
        _windowStartTime = now; // Reset window start time
        _maxHeatDurationInWindowSecs = 0; // Reset max duration for the new window
    }

    if (heatingNow) {
        if (_currentHeatStartTime == 0) { // Just started heating in this continuous block
            _currentHeatStartTime = now;
        }
        // Calculate the duration of the *current* continuous heating period so far
        millis_t currentDurationMs = now - _currentHeatStartTime;
        uint16_t currentDurationSecs = (currentDurationMs + 500) / 1000; // Round to nearest second

        // Update the maximum duration within the current window if this is longer
        if (currentDurationSecs > _maxHeatDurationInWindowSecs) {
            _maxHeatDurationInWindowSecs = currentDurationSecs;
        }
    } else {
        // Heating is off, reset the start time for the next continuous block
        _currentHeatStartTime = 0;
    }

    // Continuous Wh calculation (based on original logic)
    if (heatingNow) {
        // Only calculate Wh delta if _lastHeatingLoopTime is valid (meaning it was heating last loop too)
        if (_lastHeatingLoopTime != 0) {
            millis_t deltaMs = now - _lastHeatingLoopTime;
            if (deltaMs > 0) { // Avoid division by zero or negative time
                float deltaHours = static_cast<float>(deltaMs) / 3600000.0f;
                float deltaWh = static_cast<float>(_consumption) * deltaHours;
                _totalWh += deltaWh;
            }
        }
        // Always update _lastHeatingLoopTime when heating is active
        _lastHeatingLoopTime = now;
    } else {
        // Reset when heating stops, so Wh calculation restarts correctly next time heating begins
        _lastHeatingLoopTime = 0;
    }

    // Update state for next iteration *after* all calculations using _wasHeating
    _wasHeating = heatingNow; // Store current state for next iteration
#endif
    
    // Example: Check staleness based on lastResponseTime (public member of RTU_Base)
    // if (lastResponseTime > 0 && (millis() - lastResponseTime > (_readInterval * 2))) {
    //     Log.warningln(F("OmronE5[%d]: Data might be stale (last response %lu ms ago)."), slaveId, millis() - lastResponseTime);
    // }

    return 0; // Return non-zero to indicate an error
}
short OmronE5::info()
{
    uint16_t pv, sp, statusL, statusH;
    bool pvOk = getPV(pv);
    bool spOk = getSP(sp);
    bool statusLOk = getStatusLow(statusL);
    bool statusHOk = getStatusHigh(statusH);

    Log.infoln(F("--- OmronE5[%d] Info ---"), slaveId);
    Log.infoln(F("  State: %s"), getStateString());
    Log.infoln(F("  Last Response: %lu ms ago"), (lastResponseTime > 0) ? (millis() - lastResponseTime) : 0);
    Log.infoln(F("  Error Count: %u"), errorCount);

    Log.infoln(F("  PV: %s (%d)"), pvOk ? "OK" : "Error/Missing", pvOk ? pv : 0);
    Log.infoln(F("  SP: %s (%d)"), spOk ? "OK" : "Error/Missing", spOk ? sp : 0);
    Log.infoln(F("  Status Low: %s (0x%04X)"), statusLOk ? "OK" : "Error/Missing", statusLOk ? statusL : 0);
    Log.infoln(F("  Status High: %s (0x%04X)"), statusHOk ? "OK" : "Error/Missing", statusHOk ? statusH : 0);
#ifdef ENABLE_TRUTH_COLLECTOR
    // Cast float values to integers for logging
    int meanErrorInt = static_cast<int>(getMeanError());
    Log.infoln(F("  Mean PV-SP Error (int): %d (Count: %u)"), meanErrorInt, _errorStats.count()); // Display mean error as integer
#endif
#ifdef ENABLE_COOLING
    uint32_t coolMV;
    if (getCoolingMVRaw(coolMV)) {
        Log.infoln(F("  Cooling MV: %s (0x%04X)"), "OK", coolMV);
    } else {
        Log.infoln(F("  Cooling MV: %s"), "Error/Missing");
    }
#endif

    // Display decoded status flags
    if (statusLOk && statusHOk)
    {
        Log.infoln(F("  Decoded Status:"));
        Log.infoln(F("    Running: %s"), isRunning() ? "Yes" : "No");
        Log.infoln(F("    Heating: %s"), isHeating() ? "Yes" : "No");
        Log.infoln(F("    Cooling: %s"), isCooling() ? "Yes" : "No");
        Log.infoln(F("    Heatup: %s"), isHeatup() ? "Yes" : "No");
        Log.infoln(F("    AutoTuning: %s"), isAutoTuning() ? "Yes" : "No");
#ifdef ENABLE_TRUTH_COLLECTOR
        int heatRateInt = static_cast<int>(getHeatRate());
        Log.infoln(F("    Avg Heat Rate (int): %d osc/min (Count: %u)"), heatRateInt, _heatingIntervalStats.count()); // Display heat rate as integer
        
        uint32_t totalWhInt = static_cast<uint32_t>(getTotalWh());
        Log.infoln(F("    Total Consumption (int): %u Wh (%u kWh)"), totalWhInt, totalWhInt / 1000); // Display Wh and kWh as integers

        // --- Cost Calculation ---
        float totalKWh = static_cast<float>(totalWhInt) / 1000.0f;
        float totalCostEuros = totalKWh * PRICE_PER_KWH_EUROS;
        char costStr[10]; // Buffer for cost string
        dtostrf(totalCostEuros, 4, 2, costStr); // width=4, precision=2 for Euros.Cents
        Log.infoln(F("    Estimated Cost: %s EUR"), costStr);
        Log.infoln(F("    Consumption: %d W"), getConsumption()); 
        Log.infoln(F("    PV SP Lag: %d (tenths/min)"), _pvSpLag); 
        Log.infoln(F("    Longest Heat (5 min window): %u s"), getLongestHeatDuration60s()); // Display new metric
        //Log.infoln(F("    Cooling MV: %s (0x%04X)"), _coolingMvValid ? "OK" : "Error/Missing", _coolingMvValid ? _currentCoolingMVLow : 0);
#endif
    }
    else
    {
        Log.infoln(F("  Decoded Status: Unknown (missing status registers)"));
    }

    Log.infoln(F("  Read Interval: %lu ms"), _readInterval);
    Log.infoln(F("--- End OmronE5[%d] Info --- "), slaveId);
    return 0;
}
bool OmronE5::getPV(uint16_t &value) const
{
    if (_pvValid)
    {
        value = _currentPV;
        return true;
    }
    value = 0;
    return false;
}
bool OmronE5::getSP(uint16_t &value) const
{
    if (_spValid)
    {
        value = _currentSP;
        return true;
    }
    value = 0;
    return false;
}
bool OmronE5::getStatusLow(uint16_t &value) const
{
    if (_statusValid)
    { // Check combined validity
        value = _currentStatusLow;
        return true;
    }
    value = 0;
    return false;
}
bool OmronE5::getStatusHigh(uint16_t &value) const
{
    if (_statusValid)
    { // Check combined validity
        value = _currentStatusHigh;
        return true;
    }
    value = 0;
    return false;
}
bool OmronE5::isRunning() const
{
    uint16_t statusH, statusL;
    if (getStatusHigh(statusH) && getStatusLow(statusL))
    {
        // Note: OR_E5_STATUS_BIT is defined in OmronE5Types.h
        // RunStop bit (24) is 0 when running, 1 when stopped.
        return !OR_E5_STATUS_BIT(statusH, statusL, OR_E5_S1_RunStop);
    }
    return false; // Cannot determine state if registers are missing
}
bool OmronE5::isHeating() const
{
    uint16_t statusH, statusL;
    if (getStatusHigh(statusH) && getStatusLow(statusL))
    {
        // Control_OutputOpenOutput bit (8) is 1 when heating output is ON.
        return OR_E5_STATUS_BIT(statusH, statusL, OR_E5_S1_Control_OutputOpenOutput);
    }
    return false;
}
bool OmronE5::isCooling() const
{
    uint16_t statusH, statusL;
    if (getStatusHigh(statusH) && getStatusLow(statusL))
    {
        // Control_OutputCloseOutput bit (9) is 1 when cooling output is ON.
        return OR_E5_STATUS_BIT(statusH, statusL, OR_E5_S1_Control_OutputCloseOutput);
    }
    return false;
}
bool OmronE5::isAutoTuning() const
{
    uint16_t statusH, statusL;
    if (getStatusHigh(statusH) && getStatusLow(statusL))
    {
        // ATExcecute bit (23) is 1 when AT is executing.
        return OR_E5_STATUS_BIT(statusH, statusL, OR_E5_S1_ATExcecute);
    }
    return false;
}
bool OmronE5::setSP(uint16_t value)
{
    uint16_t spAddr = OR_E5_SWR_SP;
    // Clamp the value to the maximum allowed SP
    uint16_t clampedValue = value > OMRON_E5_SP_MAX ? OMRON_E5_SP_MAX : value;
    if (clampedValue != value) {
        Log.warningln(F("OmronE5[%d]: setSP requested value %d clamped to %d"), this->slaveId, value, clampedValue);
    }
    this->setOutputRegisterValue(spAddr, clampedValue);
    return true;
}
bool OmronE5::run()
{
    uint16_t cmdAddr = OR_E5_CMD_ADDRESS::OR_E5_CMD_STOP_RUN;    
    uint16_t runValue = 0x0100;
    this->setOutputRegisterValue(cmdAddr, runValue);
    return true;
}
bool OmronE5::stop()
{
    uint16_t cmdAddr = OR_E5_CMD_ADDRESS::OR_E5_CMD_STOP_RUN;
    uint16_t stopValue = 0x0101;
    this->setOutputRegisterValue(cmdAddr, stopValue);
    return true;
}
void OmronE5::onRegisterUpdate(uint16_t address, uint16_t newValue)
{
    // Define addresses based on offsets
    const uint16_t pvAddress = OMRON_E5_READ_BLOCK_START_ADDR + 1;
    const uint16_t statusHighAddress = OMRON_E5_READ_BLOCK_START_ADDR + 2;
    const uint16_t statusLowAddress = OMRON_E5_READ_BLOCK_START_ADDR + 3;
    const uint16_t spAddress = OMRON_E5_READ_BLOCK_START_ADDR + 5;

    // Store received value in local members and set validity flags
    if (address == pvAddress)
    {
        _currentPV = newValue;
        _pvValid = true;
    }
    else if (address == spAddress)
    {
        _currentSP = newValue;
        _spValid = true;
    }
    else if (address == statusLowAddress)
    {
        _currentStatusLow = newValue;
        // Mark _statusValid true only if both low and high have been received at least once.
        // This relies on _currentStatusHigh not being accidentally 0 if it was validly read as 0.
        // A more robust way might be separate bool flags for low_received, high_received.
        if (_currentStatusHigh != 0 || newValue != 0) { // Basic check assuming 0,0 is unlikely valid initial state for both
             _statusValid = true;
        }
    }
    else if (address == statusHighAddress)
    {
        _currentStatusHigh = newValue;
        if (_currentStatusLow != 0 || newValue != 0) {
            _statusValid = true;
        }
    }
#ifdef ENABLE_COOLING
    else if (address == 0x2005) // E_MV_MONITOR_COOL_REGISTER Low Word
    {
        // Log.infoln(F("OmronE5[%d]: Internal update for Cooling MV Low (Addr: 0x%04X) -> 0x%04X"), slaveId, address, newValue);
        _currentCoolingMVLow = newValue;
        // Validity requires both low and high words to be received at least once
        _coolingMvValid = _coolingMvValid || (_currentCoolingMVHigh != 0); 
    }
    else if (address == 0x2006) // E_MV_MONITOR_COOL_REGISTER High Word
    {
        // Log.infoln(F("OmronE5[%d]: Internal update for Cooling MV High (Addr: 0x%04X) -> 0x%04X"), slaveId, address, newValue);
        _currentCoolingMVHigh = newValue;
        // Validity requires both low and high words to be received at least once
        _coolingMvValid = _coolingMvValid || (_currentCoolingMVLow != 0); 
    }
#endif

    RTU_Base::onRegisterUpdate(address, newValue);
    if (_statusValid) { 
        bool currentRunningState = !isRunning();
        _runStateWrapper.update(currentRunningState);
    }
}

#ifdef ENABLE_TRUTH_COLLECTOR
float OmronE5::getMeanError() const {
    return _errorStats.mean();
}
float OmronE5::getHeatRate() const {
    if (_heatingIntervalStats.count() == 0) {
        return 0.0f; // No data yet
    }
    float avgIntervalMs = _heatingIntervalStats.average();
    if (isnan(avgIntervalMs) || avgIntervalMs <= 0.0f) {
        return 0.0f; // Invalid average interval
    }
    // Convert average interval (ms/osc) to oscillations per minute
    return (1.0f / avgIntervalMs) * 60000.0f;
}

float OmronE5::getTotalWh() const {
    return _totalWh;
}

uint16_t OmronE5::getLongestHeatDuration60s() const {
    // Return the tracked maximum duration in seconds for the current/last 60s window
    return _maxHeatDurationInWindowSecs;
}
#endif // ENABLE_TRUTH_COLLECTOR

uint32_t OmronE5::getConsumption() const {
    // Return the pre-configured consumption value for this specific Omron instance
    return _consumption; 
}

#ifdef ENABLE_COOLING
// --- Cooling Specific Getter Implementation ---
bool OmronE5::getCoolingMVRaw(uint32_t& value) const 
{
    if (_coolingMvValid)
    {
        // Combine high and low words into a 32-bit value 
        value = (static_cast<uint32_t>(_currentCoolingMVHigh) << 16) | _currentCoolingMVLow;
        return true;
    }
    value = 0;
    return false;
}
#endif

bool OmronE5::isHeatup() const
{
    // Check if PV and SP are valid and SP is greater than PV
    // and if PV and SP are within the valid range 0-400
    if (_pvValid && _spValid &&
        _currentSP >= 0 && _currentSP <= OMRON_E5_PV_SP_MAX_RANGE &&
        _currentPV >= 0 && _currentPV <= OMRON_E5_PV_SP_MAX_RANGE &&
        _currentSP > _currentPV)
    {
        // Calculate the deadband value as a percentage of the current SP
        float deadbandValue = static_cast<float>(_currentSP) * (HEATUP_DEADBAND / 100.0f);

        // Check if the difference between SP and PV is greater than the calculated deadband
        return static_cast<float>(_currentSP - _currentPV) > deadbandValue;
    }
    return false; // Cannot determine state if PV or SP is missing or PV >= SP
}

// --- End of OmronE5.cpp ---

#endif // ENABLE_RS485

#ifdef ENABLE_TRUTH_COLLECTOR
// --- Helper to reset runtime statistics --- 
void OmronE5::_resetRuntimeStats() {
    Log.infoln(F("OmronE5[%d]: Resetting runtime statistics..."), this->slaveId);
    _errorStats.clear();    
    _heatingIntervalStats.clear();
    _totalWh = 0.0f;
    _pvSpLag = 0;
    _lastPvUpdateTime = 0;
    _hasPreviousPv = false;
    _lastHeatingLoopTime = 0;
    _currentHeatStartTime = 0;
    _windowStartTime = 0;
    _maxHeatDurationInWindowSecs = 0;
}
#endif // ENABLE_TRUTH_COLLECTOR

#include "config.h"

#ifdef ENABLE_RS485
#ifdef ENABLE_OMRON_E5

#include <Logger.h>
#include "error_codes.h"
#include "components/OmronE5.h"
#include "components/OmronE5Types.h"
#include <modbus/ModbusTypes.h>
#include <modbus/Modbus.h>
#include "RS485.h"
#include "pid_constants.h"
#include <xstatistics.h>
#include <xmath.h>

#define OMRON_E5_SP_MAX 260
#define OMRON_E5_PV_SP_MAX_RANGE 400
#define OMRON_E5_INVALID_PV 1320

// --- Cooling Feature Setup Instructions ---
// To enable cooling monitoring:
// 1. Uncomment the #define ENABLE_COOLING above.
// 2. Ensure the Omron E5 controller is configured for Heat/Cool control
//    (Register 0x2D11 = 1) and has an output assigned to cooling
//    (e.g., Register 0x2E06 or 0x2E07 = 2) via its own settings menu
//    or separate configuration tool. This code only reads the cooling MV.
// --- End Setup Instructions ---

OmronE5::OmronE5(Component *owner, uint8_t slaveId, millis_t readInterval)
    : RTU_Base(owner, slaveId, COMPONENT_KEY::COMPONENT_KEY_PID + slaveId),
      _readInterval(readInterval),
      _error(E_ERROR_OMRON_E5::E_OMRON_ERROR_NONE),
      _initialSpSet(false),
      _statusLowReceived(false),
      _statusHighReceived(false),
      _consumption(2700),
      INIT_MODBUS_NETWORK_VALUE(m_pv, "PV", 0, 3, NetworkValue_ThresholdMode::DIFFERENCE, nullptr),
      INIT_MODBUS_NETWORK_VALUE(m_sp, "SP", 0, 3, NetworkValue_ThresholdMode::DIFFERENCE, nullptr),
      INIT_MODBUS_NETWORK_VALUE(m_statusLow, "Status Low", 0, 1, NetworkValue_ThresholdMode::DIFFERENCE, nullptr),
      INIT_MODBUS_NETWORK_VALUE(m_statusHigh, "Status High", 0, 1, NetworkValue_ThresholdMode::DIFFERENCE, nullptr),
      INIT_MODBUS_NETWORK_VALUE(m_runState, "Run/Stop", true, 1, NetworkValue_ThresholdMode::DIFFERENCE, nullptr), // true = stopped
      INIT_MODBUS_NETWORK_VALUE(m_enabled, "Enabled", true, 1, NetworkValue_ThresholdMode::DIFFERENCE, nullptr),
      INIT_MODBUS_NETWORK_VALUE(m_commsWritingEnabled, "CommsWrite", true, 1, NetworkValue_ThresholdMode::DIFFERENCE, nullptr),
      _lastRunStopCmdTime(0)
{
    m_modbusHelper.init(this);
    type = COMPONENT_TYPE::COMPONENT_TYPE_PID;
    name = "OmronE5[" + String(slaveId) + "]";
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

    const uint16_t tcpBaseAddr = mb_tcp_base_address();

    m_pv.initModbus(tcpBaseAddr + (ushort)E_OmronTcpOffset::PV, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, "PV", name.c_str());
    m_sp.initModbus(tcpBaseAddr + (ushort)E_OmronTcpOffset::SP, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, "SP", name.c_str());

    m_statusLow.initModbus(tcpBaseAddr + (ushort)E_OmronTcpOffset::STATUS_LOW, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, "Status Low", name.c_str());
    m_statusHigh.initModbus(tcpBaseAddr + (ushort)E_OmronTcpOffset::STATUS_HIGH, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, "Status High", name.c_str());
    m_enabled.initModbus(tcpBaseAddr + (ushort)E_OmronTcpOffset::ENABLED, 1, this->id, this->slaveId, FN_WRITE_COIL, "Enabled", name.c_str());

    m_commsWritingEnabled.initModbus(tcpBaseAddr + (ushort)E_OmronTcpOffset::CMD_COMMS_WRITE, 1, this->id, this->slaveId, FN_WRITE_COIL, "Comms Write", name.c_str());
    m_runState.initModbus(tcpBaseAddr + (ushort)E_OmronTcpOffset::CMD_STOP, 1, this->id, this->slaveId, FN_WRITE_COIL, "Run/Stop Coil", name.c_str());

    m_modbusHelper.registerBlock(m_pv.getRegisterInfo());
    m_modbusHelper.registerBlock(m_sp.getRegisterInfo());
    m_modbusHelper.registerBlock(m_statusLow.getRegisterInfo());
    m_modbusHelper.registerBlock(m_statusHigh.getRegisterInfo());

    m_modbusHelper.registerBlock(INIT_MODBUS_BLOCK_TCP(tcpBaseAddr, E_OmronTcpOffset::CMD_SP, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "SP CMD", name.c_str()));
    m_modbusHelper.registerBlock(m_runState.getRegisterInfo());

    m_modbusHelper.registerBlock(INIT_MODBUS_BLOCK_TCP(tcpBaseAddr, E_OmronTcpOffset::CMD_EXECUTE, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "Execute Command", name.c_str()));
    m_modbusHelper.registerBlock(INIT_MODBUS_BLOCK_TCP(tcpBaseAddr, E_OmronTcpOffset::IS_HEATING, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Heating Status", name.c_str()));
    m_modbusHelper.registerBlock(INIT_MODBUS_BLOCK_TCP(tcpBaseAddr, E_OmronTcpOffset::IS_HEATUP, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "Heatup Status", name.c_str()));

    m_modbusHelper.registerBlock(m_enabled.getRegisterInfo());
    m_modbusHelper.registerBlock(m_commsWritingEnabled.getRegisterInfo());
    m_commsWritingEnabled.update(true);
}

uint16_t OmronE5::mb_tcp_base_address() const
{
    // Calculate and return the base TCP address for this specific Omron E5 instance
    return OMRON_MB_TCP_OFFSET + (this->slaveId * OMRON_TCP_BLOCK_COUNT);
}

uint16_t OmronE5::mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const
{
    // This function maps a physical RTU register address to its corresponding
    // logical TCP offset. This is used to notify TCP clients when a value
    // read from the device has changed.
    switch (rtuAddress)
    {
    // Direct Mappings from RTU address to TCP Offset
    case static_cast<uint16_t>(E_OmronRtuAddress::PV):
        return static_cast<uint16_t>(E_OmronTcpOffset::PV);
    case static_cast<uint16_t>(E_OmronRtuAddress::SP):
        return static_cast<uint16_t>(E_OmronTcpOffset::SP);

    // The Status registers also imply changes in derived values
    case static_cast<uint16_t>(E_OmronRtuAddress::STATUS_HIGH):
        return static_cast<uint16_t>(E_OmronTcpOffset::STATUS_HIGH);
    case static_cast<uint16_t>(E_OmronRtuAddress::STATUS_LOW):
        return static_cast<uint16_t>(E_OmronTcpOffset::STATUS_LOW);

    // Other calculated values do not have a direct RTU address and are handled
    // by the application logic when their dependencies (PV, SP, Status) change.
    // e.g., IS_HEATING, IS_HEATUP, HEAT_RATE etc.
    default:
        // Return 0 if the RTU address doesn't map to a primary TCP offset.
        return 0;
    }
}

void OmronE5::enable(bool enabled)
{
    Component::enable(enabled);
    m_enabled.update(enabled);
}

short OmronE5::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
    {
        L_ERROR(F("OmronE5[%d]::mb_tcp_read - Invalid MB_Registers pointer"), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure; // Or E_INVALID_PARAMETER
    }

    if (!this->enabled())
    {
        return (short)MB_Error::Success;
    }

    // Use the new virtual function to get the base address for this instance
    const uint16_t instanceBaseAddr = this->mb_tcp_base_address();
    if (instanceBaseAddr == 0)
    { // Handle cases where TCP mapping might not be configured
        L_ERROR(F("OmronE5[%d]::mb_tcp_read - TCP Base Address is 0, cannot process read."), this->slaveId);
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
    E_OmronTcpOffset regOffset = static_cast<E_OmronTcpOffset>(offset);
    uint16_t value = 0;
    bool success = false;

    // Handle reads based on the specific register offset
    switch (regOffset)
    {
    case E_OmronTcpOffset::PV:
        return m_pv.getValue();
    case E_OmronTcpOffset::SP:
        return m_sp.getValue();
    case E_OmronTcpOffset::STATUS_LOW:
        return m_statusLow.getValue();
    case E_OmronTcpOffset::STATUS_HIGH:
        return m_statusHigh.getValue();
    case E_OmronTcpOffset::CMD_SP: // Offset 11: Read associated with write SP command (returns current SP)
        value = m_sp.getValue();   // Read the actual SP
        success = true;
        break;
    case E_OmronTcpOffset::CMD_STOP: // Offset 12: Read associated with Run/Stop command
        value = m_runState.getValue() ? 1 : 0;
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
    case E_OmronTcpOffset::ENABLED: // Offset 18: Read component enabled state
        value = enabled() ? 1 : 0;
        success = true;
        break;
    case E_OmronTcpOffset::CMD_COMMS_WRITE:
        value = m_commsWritingEnabled.getValue() ? 1 : 0;
        success = true;
        break;

    default:
        // This case handles offsets that are within the allocated range [0, OMRON_TCP_BLOCK_COUNT)
        // but are not explicitly mapped to a readable value in the E_OmronTcpOffset enum.
        // (e.g., offsets 0, 4, 6, 7, 8, 9, 10, 13, 14, 15 in a range of 16)
        Log.warningln(F("OmronE5[%d]: Read attempt on unhandled offset %d within TCP block (Address: %d)."), this->slaveId, offset, requestedAddress);
        // Treat unmapped but valid addresses within the block as illegal data addresses for read operations.
        // return (short)MB_Error::IllegalDataAddress;
        return E_OK;
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

    const uint16_t tcpBaseAddr = this->mb_tcp_base_address();
    if (tcpBaseAddr == 0)
    {
        L_ERROR(F("OmronE5[%d]::mb_tcp_write - TCP Base Address is 0, cannot process write."), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const short requestedTcpAddress = reg->startAddress;
    short offset = requestedTcpAddress - tcpBaseAddr;
    E_OmronTcpOffset regOffset = static_cast<E_OmronTcpOffset>(offset);

    bool commandSuccess = false;

    switch (regOffset)
    {
    case E_OmronTcpOffset::ENABLED:
        enable(value);
        commandSuccess = true;
        break;

    case E_OmronTcpOffset::CMD_COMMS_WRITE:
        commandSuccess = setCommsWriting(value);
        break;
    case E_OmronTcpOffset::CMD_SP:
    {
        uint16_t clampedValue = (uint16_t)value > OMRON_E5_SP_MAX ? OMRON_E5_SP_MAX : (uint16_t)value;
        commandSuccess = setSP(clampedValue);
        break;
    }
    case E_OmronTcpOffset::CMD_STOP:
        commandSuccess = value ? stop() : run();
        break;
    case E_OmronTcpOffset::CMD_EXECUTE:
    {
        E_ExecuteCommands command = static_cast<E_ExecuteCommands>(value);
        switch (command)
        {
        case E_ExecuteCommands::INFO:
            L_INFO(F("OmronE5[%d]::mb_tcp_write - Received info() command via TCP Register (Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
            this->info();
            commandSuccess = true;
            break;
        case E_ExecuteCommands::RESET_STATS:
            L_INFO(F("OmronE5[%d]::mb_tcp_write - Received reset stats command via TCP Register (Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
            _resetRuntimeStats();
            commandSuccess = true;
            break;
        default:
            Log.warningln(F("OmronE5[%d]::mb_tcp_write - Received non-command value for CMD_EXECUTE (Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
            commandSuccess = true; // Non-command value is not an error
            break;
        }
        break;
    }

    case E_OmronTcpOffset::CMD_EXECUTE_INFO:
        L_INFO(F("OmronE5[%d]::mb_tcp_write - Received info() command via TCP Register (CMD_EXECUTE_INFO, Addr: %d, Offset: %d, Value: %d)"), this->slaveId, requestedTcpAddress, offset, value);
        this->info();
        commandSuccess = true;
        break;

    case E_OmronTcpOffset::PV:
    case E_OmronTcpOffset::SP:
    case E_OmronTcpOffset::STATUS_LOW:
    case E_OmronTcpOffset::STATUS_HIGH:
    case E_OmronTcpOffset::IS_HEATING:
    case E_OmronTcpOffset::IS_HEATUP:
        Log.warningln(F("OmronE5[%d]::mb_tcp_write - Attempt to write to read-only TCP Address: %d (Offset: %d)"), this->slaveId, requestedTcpAddress, offset);
        return (short)MB_Error::IllegalDataAddress;

    default:
        Log.warningln(F("OmronE5[%d]::mb_tcp_write - Attempt to write to unhandled TCP Address: %d (Offset: %d)"), this->slaveId, requestedTcpAddress, offset);
        return (short)MB_Error::IllegalDataAddress;
    }

    if (!commandSuccess)
    {
        L_ERROR(F("OmronE5[%d]::mb_tcp_write - Failed to execute command for TCP Addr %d, Value %d"), this->slaveId, requestedTcpAddress, value);
        return (short)MB_Error::ServerDeviceFailure;
    }

    return (short)MB_Error::Success;
}

ModbusBlockView *OmronE5::mb_tcp_blocks() const
{
    return m_modbusHelper.mb_tcp_blocks();
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
        L_ERROR(F("OmronE5[%d]: Failed to add mandatory read block 1!"), slaveId);
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
        L_INFO(F("OmronE5[%d]: Configured mandatory read block 2 (Cooling MV): Addr=0x%04X, Count=%d, Interval=%lu ms"),
               slaveId, 0x2005, 2, _readInterval);
    }
    else
    {
        L_ERROR(F("OmronE5[%d]: Failed to add mandatory read block 2 (Cooling MV)!"), slaveId);
        // Decide if this is critical, potentially return error
        // return E_INVALID_PARAMETERS;
    }
#endif

    this->addOutputRegister(OR_E5_SWR_SP, E_FN_CODE::FN_WRITE_HOLD_REGISTER, 0);
    this->addOutputRegister(OR_E5_OPERATION_COMMAND_REGISTER, E_FN_CODE::FN_WRITE_HOLD_REGISTER, BUILD_OMRON_OP_COMMAND(OP_CODE_COMMS_WRITE, COMMS_WRITE_ENABLED), PRIORITY_MEDIUM);
    // Ensure communication writing is enabled on setup
    // setCommsWriting(true);

    // Set initial SP to 0
    // setSP(0);

    return E_OK;
}
short OmronE5::loop()
{
    m_enabled.update(this->enabled());

    // Ensure communications writing is enabled every 5 seconds
    /*
    static millis_t lastCommsWritingTime = 0;
    millis_t now = millis();
    if (now - lastCommsWritingTime >= 5000)
    {
        lastCommsWritingTime = now;
        setCommsWriting(true);
    }*/

    if (!enabled())
    {
        return E_OK;
    }

    // Most of the work (scheduling reads/writes) is handled by the RS485->RTU_DeviceManager.
    // The RTU_Device base class updates its internal register map when reads complete.
    // This loop could be used for:
    // 1. Triggering specific writes based on application logic.
    // 2. Checking status flags from the read data and acting upon them.
    // 3. Implementing component-specific logic not directly tied to Modbus reads/writes.

    // --- Heating Interval Statistics and Wh Calculation ---
    bool heatingNow = isHeating();

    // Track heating intervals for rate calculation
    if (!_wasHeating && heatingNow)
    { // Heating just started
        _heatOnStartTime = now;
    }
    else if (_wasHeating && !heatingNow)
    { // Heating just stopped
        millis_t duration = now - _heatOnStartTime;
        if (duration > 0)
        { // Avoid adding zero or negative duration
        }
    }

    // Continuous Wh calculation (based on original logic)
    if (heatingNow)
    {
        // Only calculate Wh delta if _lastHeatingLoopTime is valid (meaning it was heating last loop too)
        if (_lastHeatingLoopTime != 0)
        {
            millis_t deltaMs = now - _lastHeatingLoopTime;
            if (deltaMs > 0)
            { // Avoid division by zero or negative time
                // Use uint64_t for intermediate multiplication to prevent overflow
                _totalWs += (static_cast<uint64_t>(_consumption) * deltaMs) / 1000;
            }
        }
        // Always update _lastHeatingLoopTime when heating is active
        _lastHeatingLoopTime = now;
    }
    else
    {
        // Reset when heating stops, so Wh calculation restarts correctly next time heating begins
        _lastHeatingLoopTime = 0;
    }

    // Update state for next iteration *after* all calculations using _wasHeating
    _wasHeating = heatingNow; // Store current state for next iteration

    uint16_t pv;
    if (getPV(pv))
    {
        if (pv == OMRON_E5_INVALID_PV)
        {
            _error = E_ERROR_OMRON_E5::E_OMRON_ERROR_INVALID_PV;
        }
        else
        {
            _error = E_ERROR_OMRON_E5::E_OMRON_ERROR_NONE;
        }
    }
    return 0; // Return non-zero to indicate an error
}
short OmronE5::info()
{
    uint16_t pv, sp, statusL, statusH;
    bool pvOk = getPV(pv);
    bool spOk = getSP(sp);
    bool statusLOk = getStatusLow(statusL);
    bool statusHOk = getStatusHigh(statusH);

    L_INFO(F("--- OmronE5[%d] Info ---"), slaveId);
    L_INFO(F("  State: %s"), getStateString());
    L_INFO(F("  Last Response: %lu ms ago"), (lastResponseTime > 0) ? (millis() - lastResponseTime) : 0);

    L_INFO(F("  Error Count: %u"), errorCount);
    L_INFO(F("  Error: %d"), getError());
    L_INFO(F("  Has Error: %s"), hasError() ? "Yes" : "No");
    L_INFO(F("  Last Error: %d"), lastErrorCode);

    L_INFO(F("  PV: %s (%d)"), pvOk ? "OK" : "Error/Missing", pvOk ? pv : 0);
    L_INFO(F("  SP: %s (%d)"), spOk ? "OK" : "Error/Missing", spOk ? sp : 0);

    L_INFO(F("  Status Low: %s (%d)"), statusLOk ? "OK" : "Error/Missing", statusLOk ? statusL : 0);
    L_INFO(F("  Status High: %s (%d)"), statusHOk ? "OK" : "Error/Missing", statusHOk ? statusH : 0);
#ifdef ENABLE_COOLING
    uint32_t coolMV;
    if (getCoolingMVRaw(coolMV))
    {
        L_INFO(F("  Cooling MV: %s (0x%04X)"), "OK", coolMV);
    }
    else
    {
        L_INFO(F("  Cooling MV: %s"), "Error/Missing");
    }
#endif

    // Display decoded status flags
    if (statusLOk && statusHOk)
    {
        L_INFO(F("  Decoded Status:"));
        L_INFO(F("    Running: %s"), isRunning() ? "Yes" : "No");
        L_INFO(F("    Heating: %s"), isHeating() ? "Yes" : "No");
        L_INFO(F("    Cooling: %s"), isCooling() ? "Yes" : "No");
        L_INFO(F("    Heatup: %s"), isHeatup() ? "Yes" : "No");
        L_INFO(F("    AutoTuning: %s"), isAutoTuning() ? "Yes" : "No");

        uint32_t totalWhInt = getTotalWh();
        L_INFO(F("    Total Consumption (int): %u Wh (%u kWh)"), totalWhInt, totalWhInt / 1000); // Display Wh and kWh as integers

        L_INFO(F("    Consumption: %d W"), getConsumption());
        // L_INFO(F("    Cooling MV: %s (0x%04X)"), _coolingMvValid ? "OK" : "Error/Missing", _coolingMvValid ? _currentCoolingMVLow : 0);
    }
    else
    {
        L_INFO(F("  Decoded Status: Unknown (missing status registers)"));
    }

    L_INFO(F("  Read Interval: %lu ms"), _readInterval);
    L_INFO(F("--- End OmronE5[%d] Info --- "), slaveId);
    return 0;
}
bool OmronE5::getPV(uint16_t &value) const
{
    value = m_pv.getValue();
    return value != OMRON_E5_INVALID_PV;
}
bool OmronE5::getSP(uint16_t &value) const
{
    value = m_sp.getValue();
    return true;
}
bool OmronE5::getStatusLow(uint16_t &value) const
{
    value = m_statusLow.getValue();
    return true;
}
bool OmronE5::getStatusHigh(uint16_t &value) const
{
    value = m_statusHigh.getValue();
    return true;
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
bool OmronE5::isCommsWritingEnabled() const
{
    uint16_t statusH, statusL;
    if (getStatusHigh(statusH) && getStatusLow(statusL))
    {
        return OR_E5_STATUS_BIT(statusH, statusL, OR_E5_S1_ComWrite);
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
    uint16_t clampedValue = value > OMRON_E5_SP_MAX ? OMRON_E5_SP_MAX : value;
    m_sp.update(clampedValue);
    setOutputRegisterValue(spAddr, clampedValue);
    return true;
}
bool OmronE5::run(bool force)
{
    if (!force)
    {
        if (isRunning())
        {
            return true;
        }
        if (millis() - _lastRunStopCmdTime < OMRON_RUN_STOP_THROTTLE_MS)
        {
            return true; // Throttle
        }
    }

    // L_INFO(F("OmronE5[%d]::run - Sending RUN_CMD"), slaveId);
    _sendOperationCommand(OP_CODE_RUN_STOP, RUN_CMD);
    _lastRunStopCmdTime = millis();
    return true;
}
short OmronE5::stop()
{
    return stop(false);
}

short OmronE5::stop(bool force)
{
    if (!force)
    {
        if (!isRunning())
        {
            return true;
        }
        if (millis() - _lastRunStopCmdTime < OMRON_RUN_STOP_THROTTLE_MS)
        {
            return true; // Throttle
        }
    }

    // L_INFO(F("OmronE5[%d]::stop - Sending STOP_CMD"), slaveId);
    _sendOperationCommand(OP_CODE_RUN_STOP, STOP_CMD);
    _lastRunStopCmdTime = millis();
    return true;
}

bool OmronE5::setCommsWriting(bool enabled)
{
    // L_INFO(F("OmronE5[%d]::setCommsWriting - Sending COMMS_WRITE_CMD: %d"), slaveId, enabled ? COMMS_WRITE_ENABLED : COMMS_WRITE_DISABLED);
    _sendOperationCommand(OP_CODE_COMMS_WRITE, enabled ? COMMS_WRITE_ENABLED : COMMS_WRITE_DISABLED);
    return true;
}

bool OmronE5::onRegisterUpdate(uint16_t address, uint16_t newValue)
{
    lastErrorCode = 0;
    bool updated = false;

    if (address == static_cast<uint16_t>(E_OmronRtuAddress::PV))
    {
        updated = m_pv.update(newValue);
    }
    else if (address == static_cast<uint16_t>(E_OmronRtuAddress::SP))
    {
        updated = m_sp.update(newValue);
    }
    else if (address == static_cast<uint16_t>(E_OmronRtuAddress::STATUS_LOW))
    {
        updated = m_statusLow.update(newValue);
        _statusLowReceived = true;
    }
    else if (address == static_cast<uint16_t>(E_OmronRtuAddress::STATUS_HIGH))
    {
        updated = m_statusHigh.update(newValue);
        _statusHighReceived = true;
    }

    if (address == static_cast<uint16_t>(E_OmronRtuAddress::STATUS_LOW) || address == static_cast<uint16_t>(E_OmronRtuAddress::STATUS_HIGH))
    {
        uint16_t statusH, statusL;
        if (getStatusHigh(statusH) && getStatusLow(statusL))
        {
            //_log_binary("STATUS_HIGH", statusH);
            //_log_binary("STATUS_LOW", statusL);
        }

        if (_statusLowReceived && _statusHighReceived)
        {
            bool isNowRunning = isRunning();
            bool isNowCommsWritingEnabled = isCommsWritingEnabled();
            if (m_runState.update(isNowRunning, E_PRIORITY::E_PRIORITY_HIGHEST))
                updated = true;
            if (m_commsWritingEnabled.update(isNowCommsWritingEnabled, E_PRIORITY::E_PRIORITY_HIGHEST))
                updated = true;
        }
    }

    if (RTU_Base::onRegisterUpdate(address, newValue))
        updated = true;

    return updated;
}

uint32_t OmronE5::getTotalWh() const
{
    return _totalWs / 3600;
}

uint32_t OmronE5::getConsumption() const
{
    // Return the pre-configured consumption value for this specific Omron instance
    return _consumption;
}

#ifdef ENABLE_COOLING
// --- Cooling Specific Getter Implementation ---
bool OmronE5::getCoolingMVRaw(uint32_t &value) const
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
    if (RANGE(m_pv.getValue(), -1, OMRON_E5_PV_SP_MAX_RANGE + 1) &&
        RANGE(m_sp.getValue(), -1, OMRON_E5_PV_SP_MAX_RANGE + 1) &&
        m_sp.getValue() > m_pv.getValue())
    {
        // Check if the difference between SP and PV is greater than the absolute deadband
        return (m_sp.getValue() - m_pv.getValue()) > HEATUP_DEADBAND;
    }
    return false; // Cannot determine state if PV or SP is missing or PV >= SP
}

// --- End of OmronE5.cpp ---

// --- Helper to reset runtime statistics ---
void OmronE5::_resetRuntimeStats()
{
    _totalWs = 0;
    _lastHeatingLoopTime = 0;
}
void OmronE5::onError(ushort errorCode, const char *errorMessage)
{
    RTU_Base::onError(errorCode, errorMessage);
    switch (errorCode)
    {
    case (ushort)MB_Error::Timeout:
    {
        // L_ERROR("OmronE5[%d]: error : %d - Timeout : Not connected", this->slaveId, errorCode);
        return;
    }
    }
}

void OmronE5::_sendOperationCommand(OR_E5_OPERATION_CODE code, uint8_t data)
{
    uint16_t command = BUILD_OMRON_OP_COMMAND(code, data);

    char bin_buffer[17];
    bin_buffer[16] = '\0';
    for (int i = 0; i < 16; i++)
    {
        bin_buffer[15 - i] = ((command >> i) & 1) ? '1' : '0';
    }
    // L_INFO(F("OmronE5[%d]::_sendOperationCommand - code: %d, data: %d, command(hex): 0x%04X, command(bin): %s"), this->slaveId, (int)code, data, command, bin_buffer);
    setOutputRegisterValue(OR_E5_OPERATION_COMMAND_REGISTER, command, true);
}

void OmronE5::_log_binary(const char *prefix, uint16_t value)
{
    /*
    char bin_buffer[17];
    bin_buffer[16] = '\0';
    for (int i = 0; i < 16; i++)
    {
        bin_buffer[15 - i] = ((value >> i) & 1) ? '1' : '0';
    }
    L_INFO(F("OmronE5[%d]::_log_binary - %s(hex): 0x%04X, %s(bin): %s"), this->slaveId, prefix, value, prefix, bin_buffer);
    */
}
#endif // ENABLE_RS485
#endif

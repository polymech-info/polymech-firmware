#include "config.h"

#ifdef ENABLE_DELTA_VFD

#include "./DeltaTypesBase.h"
#include "./Delta_VFD.h"
#include "VFD_Base.h"
#include <Logger.h>
#include <modbus/ModbusTypes.h>
#include <modbus/Modbus.h>
#include <components/RS485.h>
#include <enums.h>
#include <Bridge.h>
#include "./DeltaTypesBase.h"

// Fault Status Flags from DeltaTypesEx.h (moved here to avoid conflicts)
enum class E_DELTA_MS300_FAULT_STATUS_FLAGS : uint16_t
{
    // Bit 0: Over Current (OC).
    // Associated Faults: 79 (Aoc), 80 (boc), 81 (coc), 82 (oPL1), 83 (oPL2), 84 (oPL3)
    E_DELTA_MS300_FAULT_FLAG_OVER_CURRENT = (1 << 0),

    // Bit 1: Over Voltage (OV).
    // Associated Faults: 62 (dEb)
    E_DELTA_MS300_FAULT_FLAG_OVER_VOLTAGE = (1 << 1),

    // Bit 2: Over Load (OL).
    // Associated Faults: 87 (oL3), 128 (ot3), 129 (ot4), 134 (EoL3), 135 (EoL4)
    E_DELTA_MS300_FAULT_FLAG_OVER_LOAD = (1 << 2),

    // Bit 3: System Fault (SYS).
    // Associated Faults: 72 (STL1), 76 (STo), 77 (STL2), 78 (STL3), 127 (CP33), 142 (AUE1), 143 (AUE2), 144 (AUE3)
    E_DELTA_MS300_FAULT_FLAG_SYSTEM = (1 << 3),

    // Bit 4: Feedback Fault (FBK).
    // Associated Faults: 89 (roPd), 140 (Hd6), 141 (b4GFF)
    E_DELTA_MS300_FAULT_FLAG_FEEDBACK = (1 << 4),

    // Bit 5: External Fault (EXI).
    // Associated Faults: 61 (ydc), 63 (oSL)
    E_DELTA_MS300_FAULT_FLAG_EXTERNAL = (1 << 5),

    // Bit 6: Communication Fault (CE).
    // Associated Faults: 58 (CE10), 101 (CGdE), 102 (CHbE), 104 (CbFE), 105 (CldE),
    //         106 (CAdE), 107 (CFrE), 121 (CP20), 123 (CP22), 124 (CP30), 126 (CP32)
    E_DELTA_MS300_FAULT_FLAG_COMMUNICATION = (1 << 6)
};

#define DELTA_MB_TCP_OFFSET COMPONENT_KEY_DELTA_VFD * 10

// Optional torque monitoring - disabled by default to save Modbus bandwidth
// #define DELTA_READ_TORQUE

// MS300 Frequency Command Register (2001H)
#define DELTA_REG_SET_FREQ static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_FREQUENCY_COMMAND)

// Enum for TCP Offsets is now in DELTA_VFD.h

DELTA_VFD::DELTA_VFD(Component *owner, uint8_t slaveId, millis_t readInterval)
    : VFD_Base(owner, slaveId, COMPONENT_KEY_DELTA_VFD, readInterval)
{
    componentId = id;
    syncInterval = readInterval;
    name = "DELTA_VFD[" + String(slaveId) + "]";
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    const uint16_t tcpBaseAddr = mb_tcp_base_address();
    _modbusBlocks[0] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::RUNNING_FREQUENCY, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: Run Freq", name.c_str());
    _modbusBlocks[1] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::SET_FREQUENCY, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: Set Freq", name.c_str());
    _modbusBlocks[2] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::OUTPUT_CURRENT, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: Current", name.c_str());
    _modbusBlocks[3] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::OUTPUT_POWER_KW, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: Power kW", name.c_str());
    _modbusBlocks[4] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::OUTPUT_TORQUE_PERCENT, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: Torque %", name.c_str());
    _modbusBlocks[5] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::FAULT_CODE, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: Fault (0:none,79-84:OC,62:OV,87+:OL,58+:CE)", name.c_str());
    _modbusBlocks[6] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::IS_RUNNING, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: Running", name.c_str());
    _modbusBlocks[7] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::HAS_FAULT, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: Fault?", name.c_str());
    _modbusBlocks[8] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::STATE, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "DELTA: State (0:stop,1:run,2:accel,3:decel,4:err)", name.c_str());
    _modbusBlocks[9] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::CMD_FREQ, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "DELTA: Set Freq Cmd", name.c_str());
    _modbusBlocks[10] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::CMD_DIRECTION, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_WRITE_ONLY, "DELTA: Direction Cmd (0:stop,1:fwd,2:rev,99:reset)", name.c_str());
    _modbusBlocks[11] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::CMD_COMMAND, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "DELTA: Command (1:info,2:reset,3:setup,4:reset_fault)", name.c_str());
    _modbusBlocks[12] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::TARGET_REGISTER, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "DELTA: Target Reg", name.c_str());
    _modbusBlocks[13] = INIT_MODBUS_BLOCK(E_DELTA_TCP_OFFSET::TARGET_VALUE, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "DELTA: Target Val", name.c_str());
    L_INFO("DELTA_VFD: Modbus Blocks: Start Address %d", tcpBaseAddr);
    static_assert(sizeof(_modbusBlocks) / sizeof(_modbusBlocks[0]) == DELTA_VFD::DELTA_TCP_BLOCK_COUNT, "Mismatch in _modbusBlocks size and DELTA_TCP_BLOCK_COUNT");
    _modbusBlockView = {_modbusBlocks, DELTA_TCP_BLOCK_COUNT};
}

short DELTA_VFD::setup()
{
    L_INFO(F("DELTA_VFD[%d]: Setting up..."), slaveId);

    // --- Configure Mandatory Read Blocks ---
    // Block 1: Status and monitoring registers (0x2100-0x2106) - Status Codes, Fault Status, Freq Cmd, Output Freq, Current, DC Bus, Output Voltage
    ModbusReadBlock *statusMonitorBlock = addMandatoryReadBlock(
        static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_STATUS_CODES),
        DELTA_VFD_STATUS_MONITOR_REG_COUNT, // Read 7 consecutive registers: 2100H-2106H
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        _readInterval);

    if (!statusMonitorBlock)
    {
        L_ERROR(F("DELTA_VFD[%d]: Failed to add status/monitor read block!"), slaveId);
        return E_INVALID_PARAMETERS;
    }

#ifdef DELTA_READ_TORQUE
    // Block 2: Torque register (0x210B) - separate because it's not consecutive
    ModbusReadBlock *torqueBlock = addMandatoryReadBlock(
        static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_OUTPUT_TORQUE),
        1, // Read only torque register
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        _readInterval);

    if (!torqueBlock)
    {
        L_ERROR(F("DELTA_VFD[%d]: Failed to add torque read block!"), slaveId);
        return E_INVALID_PARAMETERS;
    }
#endif
    // Add output registers for VFD control
    this->addOutputRegister(DELTA_REG_SET_FREQ, E_FN_CODE::FN_WRITE_HOLD_REGISTER, 0, PRIORITY_HIGH);
    this->addOutputRegister(static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_CONTROL_COMMAND),
                            E_FN_CODE::FN_WRITE_HOLD_REGISTER,
                            0, // Default to no command (stopped)
                            PRIORITY_HIGH);

    return E_OK;
}

short DELTA_VFD::loop()
{
    // Placeholder for VFD specific logic in the main loop, if needed.
    // For example, check for fault conditions based on _statusRegister or _faultCode
    // and potentially trigger actions.

    // Check staleness example
    // if (lastResponseTime > 0 && (millis() - lastResponseTime > (_readInterval * 2))) {
    //     Log.warningln(F("DELTA_VFD[%d]: Data might be stale (last response %lu ms ago)."), slaveId, millis() - lastResponseTime);
    // }

    return E_OK; // Return non-zero to indicate an error
}

short DELTA_VFD::info()
{
    uint16_t freq_int; // Raw frequency in 0.01 Hz
    uint16_t speed, fault;
    bool freqOk = getFrequency(freq_int);
    bool speedOk = getSpeed(speed);
    bool running = isRunning();
    bool faultState = hasFault();
    fault = getFaultCode();

    L_INFO(F("--- DELTA_VFD[%d] Info ---"), slaveId);
    L_INFO(F("  State: %s"), getStateString());
    L_INFO(F("  Last Response: %lu ms ago"), (lastResponseTime > 0) ? (millis() - lastResponseTime) : 0);
    L_INFO(F("  Error Count: %u"), errorCount);

    char freqStr[10];
    // Convert 0.01 Hz uint16 to float Hz for display
    float freq_display = freqOk ? (static_cast<float>(freq_int) / 100.0f) : 0.0f;
    dtostrf(freq_display, 5, 2, freqStr); // Format frequency e.g., 50.00
    L_INFO(F("  Frequency: %s (%s Hz)"), freqOk ? "OK" : "Error/Missing", freqStr);
    uint16_t current = 0;
    bool currentOk = getOutputCurrent(current);
    L_INFO(F("  Raw Output Value: %s (%d)"), currentOk ? "OK" : "Error/Missing", current);
    L_INFO(F("  Speed: %s (%d RPM)"), speedOk ? "OK" : "Error/Missing", speedOk ? speed : 0);
    L_INFO(F("  Status: Running=%s, Fault=%s"), running ? "Yes" : "No", faultState ? "Yes" : "No");
    if (faultState)
    {
        L_INFO(F("  Fault Code: 0x%04X"), fault);
    }
    L_INFO(F("  Read Interval: %lu ms"), _readInterval);
    L_INFO(F("  Last Modbus Error: %d"), lastErrorCode);
    L_INFO(F("--- End DELTA_VFD[%d] Info --- "), slaveId);
    return 0;
}

bool DELTA_VFD::onRegisterUpdate(uint16_t address, uint16_t newValue)
{
    lastErrorCode = 0;
    bool updated = false;
    // Update local state based on the register address using MS300 registers

    switch (address)
    {
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_OUTPUT_FREQ):
        _currentFrequency = newValue; // Already in 0.01Hz units
        _frequencyValid = true;
        updated = true;
        break;
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_FREQ_CMD):
        _setFrequency = newValue; // Already in 0.01Hz units
        _setFrequencyValid = true;
        updated = true;
        break;
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_OUTPUT_CURRENT):
        _currentCurrent = newValue;
        _currentValid = true;
        updated = true;
        break;
#ifdef DELTA_READ_TORQUE
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_OUTPUT_TORQUE):
        _outputTorquePercent = newValue;
        _outputTorquePercentValid = true;
        updated = true;
        break;
#endif
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_FAULT_STATUS): // 0x2101 - Operation Status
        _statusRegister = newValue;
        _statusValid = true;
        _updateStatusFromRegister(newValue);
        _updateVfdState();
        updated = true;
        break;
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_STATUS_CODES): // 0x2100 - Status Codes
        _faultCode = newValue & 0xFF;                                                // Low byte = error code
        _faultValid = true;
        // Check for specific fault flags if high byte has fault status flags
        if (newValue & 0xFF00)
        {
            _faultStatusFlags = (newValue >> 8) & 0xFF; // High byte contains fault status flags
        }
        _updateVfdState();
        updated = true;
        break;
    default:
        // Check if it's within the monitoring range
        if (address >= static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_FREQ_CMD) &&
            address <= static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_OUTPUT_TORQUE))
        {
            // Log.traceln(F("DELTA_VFD[%d]: MS300 monitoring register update (Addr: 0x%04X) -> %d"), slaveId, address, newValue);
        }
        break;
    }

    // Call base class method
    if (RTU_Base::onRegisterUpdate(address, newValue))
        updated = true;

    // L_INFO(F("DELTA_VFD[%d]: Register Update (Addr: 0x%04X) -> %d"), slaveId, address, newValue);

    return updated;
}

bool DELTA_VFD::getFrequency(uint16_t &value) const
{
    if (_frequencyValid)
    {
        // MS300 returns frequency in 0.01Hz units, convert to Hz
        value = static_cast<uint16_t>(_currentFrequency / 100);
        return true;
    }
    value = 0;
    return false;
}

bool DELTA_VFD::getSpeed(uint16_t &value) const
{
    if (_statusValid)
    {
        // Decode U0-61 (AC drive running state) based on Sako manual
        // Example: Assuming 1 = Running FWD, 2 = Running REV, 0 = Stopped
        if (_statusRegister == 1 || _statusRegister == 2)
        {
            value = 0; // Speed is not directly available in the running state
            return true;
        }
    }
    value = 0;
    return false;
}

bool DELTA_VFD::isRunning() const
{
    if (_statusValid)
    {
        // Extract drive status from MS300 operation status register (2101H)
        uint8_t driveStatus = static_cast<uint8_t>(_statusRegister & 0x0003); // bit 1-0
        return (driveStatus == static_cast<uint8_t>(E_DELTA_MS300_DRIVE_STATUS::DRIVE_OPERATING));
    }
    return false;
}

bool DELTA_VFD::hasFault() const
{
    // Check MS300 status codes register (2100H) - low byte contains error code
    return _faultValid && (_faultCode != 0);
}

uint16_t DELTA_VFD::getFaultCode() const
{
    if (_faultValid)
    {
        return _faultCode;
    }
    return 0; // Return 0 (no fault) if not valid
}

bool DELTA_VFD::hasFaultType(E_DELTA_MS300_FAULT_STATUS_FLAGS faultType) const
{
    if (!_faultValid)
        return false;

    return (_faultStatusFlags & static_cast<uint8_t>(faultType)) != 0;
}

E_VFD_STATE DELTA_VFD::getVfdState() const
{
    return _vfdState;
}

bool DELTA_VFD::setFrequency(uint16_t value)
{
    uint16_t vfdValue = static_cast<uint16_t>(value); // MS300: 50Hz = 5000 (0.01Hz units)
    LS_INFO("Delta::_vfdStartForward: frequencyCentiHz: %d", value);
    this->setOutputRegisterValue(DELTA_REG_SET_FREQ, vfdValue);
    return true;
}

bool DELTA_VFD::run()
{
    uint16_t cmd = DELTA_MS300_BUILD_CMD(
        static_cast<uint8_t>(E_DELTA_MS300_CMD_FUNCTION::RUN),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_DIRECTION::FWD),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_ACCEL_DECEL::ACCEL_DECEL_1),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_SPEED_SELECT::MASTER_SPEED),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_ENABLE::DISABLE),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_OPERATION_SOURCE::PR_00_21_SETTING));
    uint16_t reg = static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_CONTROL_COMMAND);
    this->setOutputRegisterValue(reg, cmd);
    return true;
}

bool DELTA_VFD::reverse()
{
    uint16_t cmd = DELTA_MS300_BUILD_CMD(
        static_cast<uint8_t>(E_DELTA_MS300_CMD_FUNCTION::RUN),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_DIRECTION::REV),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_ACCEL_DECEL::ACCEL_DECEL_1),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_SPEED_SELECT::MASTER_SPEED),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_ENABLE::DISABLE),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_OPERATION_SOURCE::PR_00_21_SETTING));
    uint16_t reg = static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_CONTROL_COMMAND);
    this->setOutputRegisterValue(reg, cmd);
    return true;
}

short DELTA_VFD::stop()
{
    uint16_t cmd = DELTA_MS300_BUILD_CMD(
        static_cast<uint8_t>(E_DELTA_MS300_CMD_FUNCTION::STOP),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_DIRECTION::NO_FUNCTION),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_ACCEL_DECEL::ACCEL_DECEL_1),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_SPEED_SELECT::MASTER_SPEED),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_ENABLE::DISABLE),
        static_cast<uint8_t>(E_DELTA_MS300_CMD_OPERATION_SOURCE::PR_00_21_SETTING));
    uint16_t reg = static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_CONTROL_COMMAND);
    this->setOutputRegisterValue(reg, cmd);
    return true;
}

bool DELTA_VFD::resetFault()
{
    uint16_t cmd = static_cast<uint16_t>(E_DELTA_MS300_ACTION_CMD_BITS::RESET);
    uint16_t reg = static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_ACTION_COMMAND);
    // L_INFO(F("DELTA_VFD[%d]: Setting RESET command (Addr=0x%04X, Val=0x%04X)"), slaveId, reg, cmd);
    this->setOutputRegisterValue(reg, cmd);
    return true;
}

bool DELTA_VFD::retract()
{
    if (_retractState != E_VFD_RETRACT_STATE_NONE)
    {
        Log.warningln(F("DELTA_VFD[%d]: Already in retract sequence (State: %d)"), slaveId, _retractState);
        return false; // Already retracting
    }

    L_INFO(F("DELTA_VFD[%d]: Starting retract sequence..."), slaveId);
    // Initial step: Stop the motor (using deceleration stop)
    if (stop())
    {
        _retractState = E_VFD_RETRACT_STATE_BRAKING; // Transition to braking state
        // Further state transitions will be handled in loop()
        return true;
    }
    else
    {
        L_ERROR(F("DELTA_VFD[%d]: Failed to send initial stop command for retract."), slaveId);
        return false;
    }
}

uint16_t DELTA_VFD::mb_tcp_base_address() const
{
    return DELTA_MB_TCP_OFFSET + (this->slaveId * DELTA_VFD_TCP_REG_RANGE);
}

uint16_t DELTA_VFD::mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const
{
    switch (rtuAddress)
    {
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_OUTPUT_FREQ):
        return static_cast<uint16_t>(E_DELTA_TCP_OFFSET::RUNNING_FREQUENCY);
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_FREQ_CMD):
        return static_cast<uint16_t>(E_DELTA_TCP_OFFSET::SET_FREQUENCY);
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_OUTPUT_CURRENT):
        return static_cast<uint16_t>(E_DELTA_TCP_OFFSET::OUTPUT_CURRENT);
#ifdef DELTA_READ_TORQUE
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MON_OUTPUT_TORQUE):
        return static_cast<uint16_t>(E_DELTA_TCP_OFFSET::OUTPUT_TORQUE_PERCENT);
#endif
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_FAULT_STATUS): // Operation Status (0x2101)
        return static_cast<uint16_t>(E_DELTA_TCP_OFFSET::IS_RUNNING);
    case static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_STATUS_CODES): // Status Codes (0x2100)
        return static_cast<uint16_t>(E_DELTA_TCP_OFFSET::FAULT_CODE);
    default:
        return 0; // No direct broadcast mapping for other registers
    }
}

ModbusBlockView *DELTA_VFD::mb_tcp_blocks() const
{
    return const_cast<ModbusBlockView *>(&_modbusBlockView);
}

short DELTA_VFD::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
    {
        L_ERROR(F("DELTA_VFD[%d]::mb_tcp_read - Invalid MB_Registers pointer"), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const uint16_t instanceBaseAddr = this->mb_tcp_base_address();
    if (instanceBaseAddr == 0)
    { // Handle cases where TCP mapping might not be configured
        L_ERROR(F("DELTA_VFD[%d]::mb_tcp_read - TCP Base Address is 0"), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const short requestedAddress = reg->startAddress;
    short offset = requestedAddress - instanceBaseAddr;
    if (offset < 1 || offset > DELTA_VFD_TCP_REG_RANGE) // Use defined range
    {
        Log.warningln(F("DELTA_VFD[%d]: Read invalid offset %d (Addr %d, Base %d)"),
                      this->slaveId, offset, requestedAddress, instanceBaseAddr);
        return (short)MB_Error::IllegalDataAddress;
    }

    uint16_t value = 0;
    bool success = false;
    E_DELTA_TCP_OFFSET regOffset = static_cast<E_DELTA_TCP_OFFSET>(offset);

    switch (regOffset)
    {
    case E_DELTA_TCP_OFFSET::RUNNING_FREQUENCY:
        success = getFrequency(value);
        if (!success)
            value = 0xFFFF;
        break;
    case E_DELTA_TCP_OFFSET::SET_FREQUENCY:
        success = _setFrequencyValid;
        value = success ? (_setFrequency / 100) : 0xFFFF;
        break;
    case E_DELTA_TCP_OFFSET::OUTPUT_CURRENT:
    {
        success = getOutputCurrent(value);
        if (!success)
            value = 0xFFFF;
    }
    break;
    case E_DELTA_TCP_OFFSET::OUTPUT_POWER_KW:
        success = getOutputPowerKW(value);
        if (!success)
            value = 0xFFFF;
        break;
#ifdef DELTA_READ_TORQUE
    case E_DELTA_TCP_OFFSET::OUTPUT_TORQUE_PERCENT:
        success = getOutputTorquePercent(value);
        if (!success)
            value = 0xFFFF;
        break;
#endif
    case E_DELTA_TCP_OFFSET::FAULT_CODE:
        value = getFaultCode(); // Use getter which returns 0 for no fault/invalid
        success = true;         // Reading fault code status is always possible via getter
        break;
    case E_DELTA_TCP_OFFSET::IS_RUNNING:
        value = isRunning() ? 1 : 0;
        success = _statusValid; // Depends on status being valid
        break;
    case E_DELTA_TCP_OFFSET::HAS_FAULT:
        value = hasFault() ? 1 : 0;
        success = _faultValid; // Depends on fault code being valid
        break;
    case E_DELTA_TCP_OFFSET::STATE:
        value = (uint16_t)getVfdState();
        success = true;
        break;
    case E_DELTA_TCP_OFFSET::CMD_FREQ: // Read associated with write freq command (returns current *set* freq)
        // Return the raw set frequency value (0.01 Hz units)
        success = _setFrequencyValid;                     // Corrected: was _setFrequencyValid / 100
        value = success ? (_setFrequency / 100) : 0xFFFF; // Display in Hz
        break;
    case E_DELTA_TCP_OFFSET::CMD_DIRECTION:              // Read associated with command write returns current running state?
        value = _statusValid ? _statusRegister : 0xFFFF; // Return raw status
        success = _statusValid;
        break;
    case E_DELTA_TCP_OFFSET::CMD_COMMAND:
        value = static_cast<uint16_t>(_lastCommand); // Return the last executed command
        success = true;
        break;
    case E_DELTA_TCP_OFFSET::TARGET_REGISTER:
        value = _tcpTargetRegister; // Return the stored target register
        success = true;
        break;
    case E_DELTA_TCP_OFFSET::TARGET_VALUE:
        value = 0; // This register is write-only for triggering, reads as 0
        success = true;
        break;
    default:
        return E_OK;
        // Log.warningln(F("DELTA_VFD[%d]: TCP Read unhandled offset %d"), this->slaveId, offset);
        // return (short)MB_Error::IllegalDataAddress;
    }

    return success ? value : 0xFFFF; // Return value or error indicator
}

short DELTA_VFD::setupVFD()
{
    RS485 *rs485 = (RS485 *)owner;

    // Configure MS300 parameters for communication control

    // /rs485->modbus.writeRegister(this->slaveId, static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MASTER_FREQ_CMD_SOURCE), 9, true); // Master Freq Cmd Source (00-20)
    rs485->modbus.writeRegister(this->slaveId, static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_OPERATION_CMD_SOURCE), 2, true);                        // Operation Cmd Source (00-21)
    rs485->modbus.writeRegister(this->slaveId, static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_MAX_OP_FREQ_MOTOR_1), DELTA_MAX_OP_FREQ_MOTOR_1, true); // Max Op Freq Motor 1 (01-00) = 75.00 Hz
    rs485->modbus.writeRegister(this->slaveId, static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_ACCEL_TIME_1_MOTOR_1), 150, true);                      // Accel Time 1 (01-12) = 1.50 sec
    rs485->modbus.writeRegister(this->slaveId, static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_DECEL_TIME_1_MOTOR_1), 150, true);                      // Decel Time 1 (01-13) = 1.50 sec
    rs485->modbus.writeRegister(this->slaveId, static_cast<uint16_t>(E_DELTA_MS300_REGISTERS::E_DELTA_MS300_OUTPUT_VOLTAGE_MOTOR_1), 2200, true);                   // Motor Voltage (01-02) = 220.0V

    return E_OK;
}

short DELTA_VFD::serial_register(Bridge *bridge)
{
    Component::serial_register(bridge);
    return E_OK;
}

short DELTA_VFD::mb_tcp_write(MB_Registers *reg, short value)
{
    if (!reg)
        return E_INVALID_PARAMETER;

    const uint16_t tcpBaseAddr = this->mb_tcp_base_address();
    if (tcpBaseAddr == 0)
    {
        L_ERROR(F("DELTA_VFD[%d]::mb_tcp_write - TCP Base Address is 0"), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const short requestedTcpAddress = reg->startAddress;
    short offset = requestedTcpAddress - tcpBaseAddr;
    E_DELTA_TCP_OFFSET regOffset = static_cast<E_DELTA_TCP_OFFSET>(offset);

    bool commandSuccess = false;

    switch (regOffset)
    {
    case E_DELTA_TCP_OFFSET::CMD_FREQ:
        commandSuccess = setFrequency(static_cast<uint16_t>(value));
        break;
    case E_DELTA_TCP_OFFSET::CMD_DIRECTION:
    {
        // Interpret value as MS300 command function
        switch (value)
        {
        case 1: // Run Forward
            commandSuccess = run();
            break;
        case 2: // Run Reverse
            commandSuccess = reverse();
            break;
        case 0: // Stop
            commandSuccess = stop();
            break;
        case 99: // Reset Fault
            commandSuccess = resetFault();
            break;
        default:
            Log.warningln(F("DELTA_VFD[%d]: Unknown TCP direction command value %d"), this->slaveId, value);
            commandSuccess = false;
            break;
        }
    }
    break;
    case E_DELTA_TCP_OFFSET::CMD_COMMAND:
    {
        // Handle custom commands
        E_DELTA_CMD command = static_cast<E_DELTA_CMD>(value);
        _lastCommand = command;

        switch (command)
        {
        case E_DELTA_CMD::E_DTC_INFO:
            L_INFO(F("DELTA_VFD[%d]: Executing CMD_COMMAND: INFO"), this->slaveId);
            info(); // Call the info method
            commandSuccess = true;
            break;
        case E_DELTA_CMD::E_DTC_RESET:
            L_INFO(F("DELTA_VFD[%d]: Executing CMD_COMMAND: RESET"), this->slaveId);
            reset(); // Call the reset method
            commandSuccess = true;
            break;
        case E_DELTA_CMD::E_DTC_SETUP_VFD:
            L_INFO(F("DELTA_VFD[%d]: Executing CMD_COMMAND: SETUP_VFD"), this->slaveId);
            commandSuccess = (setupVFD() == E_OK);
            break;
        case E_DELTA_CMD::E_DTC_RESET_FAULT:
            L_INFO(F("DELTA_VFD[%d]: Executing CMD_COMMAND: RESET_FAULT"), this->slaveId);
            commandSuccess = resetFault();
            break;
        case E_DELTA_CMD::E_DTC_NONE:
        default:
            L_INFO(F("DELTA_VFD[%d]: CMD_COMMAND: No operation or unknown command (%d)"), this->slaveId, value);
            commandSuccess = true; // No-op is considered successful
            break;
        }
    }
    break;
    case E_DELTA_TCP_OFFSET::TARGET_REGISTER:
        _tcpTargetRegister = static_cast<uint16_t>(value);
        L_INFO(F("DELTA_VFD[%d]: TCP Target Register set to 0x%04X"), this->slaveId, _tcpTargetRegister);
        commandSuccess = true;
        break;
    case E_DELTA_TCP_OFFSET::TARGET_VALUE:
        if (_tcpTargetRegister == 0)
        {
            Log.warningln(F("DELTA_VFD[%d]: TCP Target Value write ignored, Target Register not set (is 0)."), this->slaveId);
            commandSuccess = false;
        }
        else
        {
            RS485 *rs485 = (RS485 *)owner;
            if (rs485)
            {
                L_INFO(F("DELTA_VFD[%d]: TCP Writing Value 0x%04X to VFD Register 0x%04X"), this->slaveId, value, _tcpTargetRegister);
                MB_Error writeStatus = rs485->modbus.writeRegister(this->slaveId, _tcpTargetRegister, static_cast<uint16_t>(value));
                commandSuccess = (writeStatus == MB_Error::Success);
                if (commandSuccess)
                {
                    L_INFO(F("DELTA_VFD[%d]: VFD Register 0x%04X write successful."), this->slaveId, _tcpTargetRegister);
                }
                else
                {
                    L_ERROR(F("DELTA_VFD[%d]: VFD Register 0x%04X write failed. Status: %d"), this->slaveId, _tcpTargetRegister, static_cast<int>(writeStatus));
                }
                // Reset _tcpTargetRegister after the write attempt, regardless of success or failure, as per user requirement for registers to be 0 after write.
                _tcpTargetRegister = 0;
            }
            else
            {
                L_ERROR(F("DELTA_VFD[%d]: RS485 owner not found for VFD write."), this->slaveId);
                commandSuccess = false;
                _tcpTargetRegister = 0; // Also reset if owner not found, to ensure it's cleared.
            }
        }
        break;
    default:
        Log.warningln(F("DELTA_VFD[%d]: Unknown TCP write offset %d"), this->slaveId, offset);
        commandSuccess = false;
        break;
    }

    return commandSuccess ? (short)MB_Error::Success : (short)MB_Error::ServerDeviceFailure;
}

// --- Internal Helper Methods ---

void DELTA_VFD::_updateStatusFromRegister(uint16_t statusReg)
{
    // Decode U0-61 (AC drive running state) according to Sako manual
    // This register value might directly represent running/stopped/fault states
    // Example: (Check Manual!)
    // 0: Stopped
    // 1: Running Forward
    // 2: Running Reverse
    // 3: Faulted? (Maybe combined with U0-62)

    // Update internal state flags based on the decoded value
    // This is just an example based on common VFD status registers
    bool currentRunning = (statusReg == 1 || statusReg == 2);
    // Fault status is primarily determined by _faultCode from U0-62

    // Log changes if significant
    // if (currentRunning != isRunning()) {
    //     L_INFO(F("DELTA_VFD[%d]: Running state changed to %s"), slaveId, currentRunning ? "Running" : "Stopped");
    // }

    // Note: `_statusValid` is set when U0-61 is received in onRegisterUpdate.
    // Note: `_faultValid` and `_faultCode` are set when U0-62 is received.
}

// --- Internal Helper: Update VFD Operational State ---
void DELTA_VFD::_updateVfdState()
{
    const uint16_t FREQUENCY_TOLERANCE = 5; // Tolerance for frequency comparison (0.05 Hz)
    E_VFD_STATE previousState = _vfdState;

    if (hasFault())
    {
        _vfdState = E_VFD_STATE_ERROR;
    }
    else if (!isRunning())
    {
        // If not running (based on U0-61) and current freq is near zero, consider it stopped
        // Otherwise, it might be decelerating after a stop command
        if (_frequencyValid && _currentFrequency <= FREQUENCY_TOLERANCE)
        {
            _vfdState = E_VFD_STATE_STOPPED;
        }
        else
        {
            // Still spinning down after stop command or just stopped
            _vfdState = E_VFD_STATE_DECELERATING;
        }
    }
    else
    { // isRunning() is true
        if (_frequencyValid && _setFrequencyValid)
        {
            // Compare current frequency to set frequency
            if (_currentFrequency < (_setFrequency - FREQUENCY_TOLERANCE))
            {
                _vfdState = E_VFD_STATE_ACCELERATING;
            }
            else if (_currentFrequency > (_setFrequency + FREQUENCY_TOLERANCE))
            {
                // Could be decelerating due to setpoint change or overshoot
                _vfdState = E_VFD_STATE_DECELERATING;
            }
            else
            {
                // Frequencies are close enough - considered running at setpoint
                _vfdState = E_VFD_STATE_RUNNING;
            }
        }
        else
        {
            // Running, but frequency data is missing/invalid - default to RUNNING state?
            // Or introduce an UNKNOWN state? For now, assume RUNNING.
            _vfdState = E_VFD_STATE_RUNNING;
        }
    }

    // Optional: Log state changes
    if (_vfdState != previousState)
    {
        L_INFO(F("DELTA_VFD[%d]: VFD State changed from %d to %d"), slaveId, previousState, _vfdState);
        _stateWrapper.update((uint16_t)_vfdState);
    }
}

short DELTA_VFD::getTorque()
{
    if (!isRunning())
        return E_INVALID_PARAMETER;
    float torque = _currentCurrent * _currentFrequency;
    return E_OK;
}

short DELTA_VFD::reset() { return E_OK; }

bool DELTA_VFD::getOutputCurrent(uint16_t &value) const
{
    if (_currentValid)
    {
        value = static_cast<uint16_t>(_currentCurrent); // _currentCurrent now holds the raw value as a float
        return true;
    }
    value = 0; // Default if not valid
    return false;
}

bool DELTA_VFD::getOutputPowerKW(uint16_t &value) const
{
    if (_outputPowerKWValid)
    {
        value = _outputPowerKW;
        return true;
    }
    value = 0;
    return false;
}

#ifdef DELTA_READ_TORQUE
bool DELTA_VFD::getOutputTorquePercent(uint16_t &value) const
{
    if (_outputTorquePercentValid)
    {
        value = _outputTorquePercent;
        return true;
    }
    value = 0;
    return false;
}
#endif

void DELTA_VFD::onError(ushort errorCode, const char *errorMessage)
{
    // L_ERROR(F("DELTA_VFD[%d]: Modbus Error %d - %s"), slaveId, errorCode, errorMessage ? errorMessage : "Unknown");
    RTU_Base::onError(errorCode, errorMessage);
}

ushort DELTA_VFD::mb_tcp_error(MB_Registers *reg)
{
    return lastErrorCode;
}

#endif // ENABLE_RS485
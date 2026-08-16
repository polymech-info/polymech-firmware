#include "config.h"

#ifdef ENABLE_SAKO_VFD

#include <Logger.h>
#include <modbus/ModbusTypes.h>
#include <modbus/Modbus.h>
#include <components/RS485.h>
#include <enums.h>
#include <Bridge.h>

#include "./SAKO_VFD.h"
#include "./SakoTypes.h"
#include "./Sako-Registers.h"

#define SAKO_MB_MONITOR_REGS 10
#define SAKO_MB_TCP_OFFSET COMPONENT_KEY_SAKO_VFD * 10

// Placeholder for Frequency Setting Register - Replace with actual Sako Parameter Register Address
#define SAKO_REG_SET_FREQ 0x1000

// --- Define the Monitoring registers to read ---
// Read Running Freq, Set Freq, Bus Voltage, Output Voltage, Output Current, Fault Info, Running State, Fault Code
#define SAKO_VFD_READ_BLOCK_START_ADDR static_cast<uint16_t>(E_SAKO_MON::E_SAKO_MON_RUNNING_FREQUENCY_HZ) // Start at U0-00 (1000)
// Calculate count based on the required registers. Need U0-00 to U0-04, plus U0-45, U0-61, U0-62
// This is complex as they are not contiguous. Will require multiple read blocks or careful handling.
// For simplicity, let's *assume* a single block reading the first few essential ones for now.
// READ: Running Freq (1000), Set Freq (1001), Bus Volt (1002), Out Volt (1003), Out Curr (1004)
#define SAKO_VFD_READ_BLOCK_REG_COUNT 5


// Enum for TCP Offsets is now in SAKO_VFD.h

SAKO_VFD::SAKO_VFD(uint8_t slaveId, millis_t readInterval)
    : RTU_Base(slaveId, COMPONENT_KEY_SAKO_VFD),
      _readInterval(readInterval),
      _vfdState(E_VFD_STATE_STOPPED) // Initialize VFD state
{
    componentId = id;
    syncInterval = readInterval;
    name = "SAKO_VFD[" + String(slaveId) + "]";
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    const uint16_t tcpBaseAddr = mb_tcp_base_address();
    _modbusBlocks[0] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::RUNNING_FREQUENCY, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SAKO: Run Freq", name.c_str());
    _modbusBlocks[1] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::SET_FREQUENCY, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SAKO: Set Freq", name.c_str());
    _modbusBlocks[2] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::OUTPUT_CURRENT, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SAKO: Current", name.c_str());
    _modbusBlocks[3] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::OUTPUT_POWER_KW, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SAKO: Power kW", name.c_str());
    _modbusBlocks[4] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::OUTPUT_TORQUE_PERCENT, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SAKO: Torque %", name.c_str());
    _modbusBlocks[5] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::FAULT_CODE, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SAKO: Fault", name.c_str());
    _modbusBlocks[6] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::IS_RUNNING, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SAKO: Running", name.c_str());
    _modbusBlocks[7] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::HAS_FAULT, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, "SAKO: Fault?", name.c_str());
    _modbusBlocks[8] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::CMD_FREQ, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "SAKO: Set Freq Cmd", name.c_str());
    _modbusBlocks[9] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::CMD_DIRECTION, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_WRITE_ONLY, "SAKO: Direction Cmd", name.c_str());
    _modbusBlocks[10] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::CMD_COMMAND, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "SAKO: Command", name.c_str());
    _modbusBlocks[11] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::TARGET_REGISTER, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "SAKO: Target Reg", name.c_str());
    _modbusBlocks[12] = INIT_MODBUS_BLOCK(E_SakoTcpOffset::TARGET_VALUE, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "SAKO: Target Val", name.c_str());
    Log.infoln("SAKO_VFD: Modbus Blocks: Start Address %d", tcpBaseAddr);
    static_assert(sizeof(_modbusBlocks) / sizeof(_modbusBlocks[0]) == SAKO_VFD::SAKO_TCP_BLOCK_COUNT, "Mismatch in _modbusBlocks size and SAKO_TCP_BLOCK_COUNT");
    _modbusBlockView = {_modbusBlocks, SAKO_TCP_BLOCK_COUNT};
}

short SAKO_VFD::setup()
{
    Log.infoln(F("SAKO_VFD[%d]: Setting up..."), slaveId);
    
    // --- Configure Mandatory Read Block ---
    // Reads 10 registers starting from Running Frequency (U0-00)
    // Covers: U0-00 to U0-09 (Running Freq to AI1 Voltage)
    const uint16_t startAddr = static_cast<uint16_t>(E_SAKO_MON::E_SAKO_MON_RUNNING_FREQUENCY_HZ);
    const uint16_t regCount = 10;
    ModbusReadBlock *block = addMandatoryReadBlock(
        startAddr,
        SAKO_MB_MONITOR_REGS,
        E_FN_CODE::FN_READ_HOLD_REGISTER, // Assuming these are holding registers
        _readInterval);

    if (!block)
    {
        Log.errorln(F("SAKO_VFD[%d]: Failed to add mandatory read block (Addr=0x%04X, Count=%d)!"), slaveId, startAddr, SAKO_MB_MONITOR_REGS);
        return E_INVALID_PARAMETERS;
    }
    this->addOutputRegister(SAKO_REG_SET_FREQ, E_FN_CODE::FN_WRITE_HOLD_REGISTER, 0, PRIORITY_HIGH);
    this->addOutputRegister(static_cast<uint16_t>(E_SAKO_REGISTERS::E_SAKO_REGISTERS_SET_DIR),       
                            E_FN_CODE::FN_WRITE_HOLD_REGISTER,
                            static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_DECELERATION_STOP),
                            PRIORITY_HIGH);

    return E_OK;
}

short SAKO_VFD::loop()
{
    // Placeholder for VFD specific logic in the main loop, if needed.
    // For example, check for fault conditions based on _statusRegister or _faultCode
    // and potentially trigger actions.

    // Check staleness example
    // if (lastResponseTime > 0 && (millis() - lastResponseTime > (_readInterval * 2))) {
    //     Log.warningln(F("SAKO_VFD[%d]: Data might be stale (last response %lu ms ago)."), slaveId, millis() - lastResponseTime);
    // }

    return 0; // Return non-zero to indicate an error
}

short SAKO_VFD::info()
{
    uint16_t freq_int; // Raw frequency in 0.01 Hz
    uint16_t speed, fault;
    bool freqOk = getFrequency(freq_int);
    bool speedOk = getSpeed(speed);
    bool running = isRunning();
    bool faultState = hasFault();
    fault = getFaultCode();

    Log.infoln(F("--- SAKO_VFD[%d] Info ---"), slaveId);
    Log.infoln(F("  State: %s"), getStateString());
    Log.infoln(F("  Last Response: %lu ms ago"), (lastResponseTime > 0) ? (millis() - lastResponseTime) : 0);
    Log.infoln(F("  Error Count: %u"), errorCount);

    char freqStr[10];
    // Convert 0.01 Hz uint16 to float Hz for display
    float freq_display = freqOk ? (static_cast<float>(freq_int) / 100.0f) : 0.0f;
    dtostrf(freq_display, 5, 2, freqStr); // Format frequency e.g., 50.00
    Log.infoln(F("  Frequency: %s (%s Hz)"), freqOk ? "OK" : "Error/Missing", freqStr);
    uint16_t current = 0;
    bool currentOk = getOutputCurrent(current);
    Log.infoln(F("  Raw Output Value: %s (%d)"), currentOk ? "OK" : "Error/Missing", current);
    Log.infoln(F("  Speed: %s (%d RPM)"), speedOk ? "OK" : "Error/Missing", speedOk ? speed : 0);
    Log.infoln(F("  Status: Running=%s, Fault=%s"), running ? "Yes" : "No", faultState ? "Yes" : "No");
    if (faultState)
    {
        Log.infoln(F("  Fault Code: 0x%04X"), fault);
    }
    Log.infoln(F("  Read Interval: %lu ms"), _readInterval);
    Log.infoln(F("--- End SAKO_VFD[%d] Info --- "), slaveId);
    return 0;
}

void SAKO_VFD::onRegisterUpdate(uint16_t address, uint16_t newValue)
{
    // Update local state based on the register address using E_SAKO_MON enum
    E_SAKO_MON reg = static_cast<E_SAKO_MON>(address);

    switch (reg)
    {
    case E_SAKO_MON::E_SAKO_MON_RUNNING_FREQUENCY_HZ: // U0-00
        _currentFrequency = newValue;
        _frequencyValid = true;
        break;
    case E_SAKO_MON::E_SAKO_MON_SET_FREQUENCY_HZ: // U0-01
        _setFrequency = newValue;
        _setFrequencyValid = true; // Add a validity flag if needed
        break;
    case E_SAKO_MON::E_SAKO_MON_OUTPUT_CURRENT_A: // U0-04
        _currentCurrent = newValue;
        _currentValid = true;
        if (!isnan(_currentCurrent))
        {
            _ampStats.add(_currentCurrent);
        }
        break;
    case E_SAKO_MON::E_SAKO_MON_OUTPUT_POWER_KW: // U0-05
        _outputPowerKW = newValue;
        _outputPowerKWValid = true;
        break;
    case E_SAKO_MON::E_SAKO_MON_OUTPUT_TORQUE_PERCENT:
        _outputTorquePercent = newValue;
        _outputTorquePercentValid = true;
        break;
    case E_SAKO_MON::E_SAKO_MON_AC_DRIVE_RUNNING_STATE: // U0-61
        _statusRegister = newValue;                     // Store the raw running state
        _statusValid = true;
        _updateStatusFromRegister(newValue); // Decode status bits
        _updateVfdState();                   // Update operational state based on status change
        break;
    case E_SAKO_MON::E_SAKO_MON_CURRENT_FAULT_CODE: // U0-62
        _faultCode = newValue;
        _faultValid = true;
        // Update fault status based on the code (0 means no fault)
        _updateStatusFromRegister(_statusRegister); // Re-evaluate combined status if needed
        _updateVfdState();                          // Update operational state based on fault change
        break;
    default:
        // Check if it's within the initial read block range for logging
        if (address >= static_cast<uint16_t>(E_SAKO_MON::E_SAKO_MON_RUNNING_FREQUENCY_HZ) &&
            address < (static_cast<uint16_t>(E_SAKO_MON::E_SAKO_MON_RUNNING_FREQUENCY_HZ) + 5))
        {
            // Log.traceln(F("SAKO_VFD[%d]: Internal update for known block register (Addr: 0x%04X) -> %d"), slaveId, address, newValue);
        }
        else
        {
            // Log.traceln(F("SAKO_VFD[%d]: Internal update for unhandled register (Addr: 0x%04X) -> %d"), slaveId, address, newValue);
        }
        break;
    }

    // Update operational state if relevant data changed
    if (reg == E_SAKO_MON::E_SAKO_MON_RUNNING_FREQUENCY_HZ ||
        reg == E_SAKO_MON::E_SAKO_MON_SET_FREQUENCY_HZ ||
        reg == E_SAKO_MON::E_SAKO_MON_AC_DRIVE_RUNNING_STATE ||
        reg == E_SAKO_MON::E_SAKO_MON_CURRENT_FAULT_CODE)
    {
        _updateVfdState();
    }

    // Call base class method
    RTU_Base::onRegisterUpdate(address, newValue);
}

bool SAKO_VFD::getFrequency(uint16_t &value) const
{
    if (_frequencyValid)
    {
        // Convert from 0.01 Hz to Hz with inverse scaling (divide by 0.64)
        value = static_cast<uint16_t>((_currentFrequency * 100) / 64); // Multiply by 100/64 ≈ 1.5625
        return true;
    }
    value = 0;
    return false;
}

bool SAKO_VFD::getSpeed(uint16_t &value) const
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

bool SAKO_VFD::isRunning() const
{
    if (_statusValid)
    {
        // Check for any running state including forward, reverse, and jogging
        return (_statusRegister == static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_FWD) ||
                _statusRegister == static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_REV) ||
                _statusRegister == static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_JOGGING) ||
                _statusRegister == static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_REVERSE_JOGGING));
    }
    return false;
}

bool SAKO_VFD::hasFault() const
{
    // Check fault code register U0-62
    // E_SAKO_ERROR_NO_FAULT is 0
    return _faultValid && (_faultCode != static_cast<uint16_t>(E_SAKO_ERROR::E_SAKO_ERROR_NO_FAULT));
}

uint16_t SAKO_VFD::getFaultCode() const
{
    if (_faultValid)
    {
        return _faultCode;
    }
    return static_cast<uint16_t>(E_SAKO_ERROR::E_SAKO_ERROR_NO_FAULT); // Return NO_FAULT if not valid
}

E_VFD_STATE SAKO_VFD::getVfdState() const
{
    return _vfdState;
}

bool SAKO_VFD::setFrequency(uint16_t value)
{
    uint16_t vfdValue = static_cast<uint16_t>(value * 64); // 100 * 0.64 = 64
    Log.infoln(F("SAKO_VFD[%d]: Setting Frequency Output (Addr=%d, Val=%d Hz"), slaveId, SAKO_REG_SET_FREQ, value);    
    this->setOutputRegisterValue(SAKO_REG_SET_FREQ, vfdValue * 2);
    return true;
}

bool SAKO_VFD::run() // Assuming run means forward
{
    uint16_t cmd = static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_FWD);
    uint16_t reg = static_cast<uint16_t>(E_SAKO_REGISTERS::E_SAKO_REGISTERS_SET_DIR);
    // Log.infoln(F("SAKO_VFD[%d]: Setting RUN command (Addr=0x%04X, Val=0x%04X)"), slaveId, reg, cmd);
    Log.infoln(F("SAKO_VFD[%d]: Setting RUN command (Addr=%d, Val=%d)"), slaveId, reg, cmd);
    this->setOutputRegisterValue(reg, cmd);
    return true;
}

bool SAKO_VFD::reverse()
{
    uint16_t cmd = static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_REV);
    uint16_t reg = static_cast<uint16_t>(E_SAKO_REGISTERS::E_SAKO_REGISTERS_SET_DIR);
    // Log.infoln(F("SAKO_VFD[%d]: Setting REVERSE command (Addr=0x%04X, Val=0x%04X)"), slaveId, reg, cmd);
    Log.infoln(F("SAKO_VFD[%d]: Setting Reverse command (Addr=%d, Val=%d)"), slaveId, reg, cmd);
    this->setOutputRegisterValue(reg, cmd);
    return true;
}

bool SAKO_VFD::stop() // Using Deceleration Stop
{
    uint16_t cmd = static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_DECELERATION_STOP);
    uint16_t reg = static_cast<uint16_t>(E_SAKO_REGISTERS::E_SAKO_REGISTERS_SET_DIR);
    this->setOutputRegisterValue(reg, cmd);
    return true;
}

bool SAKO_VFD::resetFault()
{
    uint16_t cmd = static_cast<uint16_t>(E_SAKO_DIRECTION::E_SAKO_DIR_FAULT_RESET);
    uint16_t reg = static_cast<uint16_t>(E_SAKO_REGISTERS::E_SAKO_REGISTERS_SET_DIR);
    Log.infoln(F("SAKO_VFD[%d]: Setting RESET command (Addr=0x%04X, Val=0x%04X)"), slaveId, reg, cmd);
    this->setOutputRegisterValue(reg, cmd);
    return true;
}

bool SAKO_VFD::retract()
{
    if (_retractState != E_VFD_RETRACT_STATE_NONE)
    {
        Log.warningln(F("SAKO_VFD[%d]: Already in retract sequence (State: %d)"), slaveId, _retractState);
        return false; // Already retracting
    }

    Log.infoln(F("SAKO_VFD[%d]: Starting retract sequence..."), slaveId);
    // Initial step: Stop the motor (using deceleration stop)
    if (stop())
    {
        _retractState = E_VFD_RETRACT_STATE_BRAKING; // Transition to braking state
        // Further state transitions will be handled in loop()
        return true;
    }
    else
    {
        Log.errorln(F("SAKO_VFD[%d]: Failed to send initial stop command for retract."), slaveId);
        return false;
    }
}

uint16_t SAKO_VFD::mb_tcp_base_address() const
{
    return SAKO_MB_TCP_OFFSET + (this->slaveId * SAKO_VFD_TCP_REG_RANGE);
}

uint16_t SAKO_VFD::mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const
{
    E_SAKO_MON reg = static_cast<E_SAKO_MON>(rtuAddress);
    switch (reg)
    {
    case E_SAKO_MON::E_SAKO_MON_RUNNING_FREQUENCY_HZ:
        return static_cast<uint16_t>(E_SakoTcpOffset::RUNNING_FREQUENCY);
    case E_SAKO_MON::E_SAKO_MON_SET_FREQUENCY_HZ:
        return static_cast<uint16_t>(E_SakoTcpOffset::SET_FREQUENCY);
    case E_SAKO_MON::E_SAKO_MON_OUTPUT_CURRENT_A:
        // Find the corresponding TCP offset if needed, e.g., OUTPUT_CURRENT
        return static_cast<uint16_t>(E_SakoTcpOffset::OUTPUT_CURRENT); // Example mapping
    case E_SAKO_MON::E_SAKO_MON_AC_DRIVE_RUNNING_STATE:
        // Update the IS_RUNNING boolean flag via TCP
        return static_cast<uint16_t>(E_SakoTcpOffset::IS_RUNNING);
    case E_SAKO_MON::E_SAKO_MON_CURRENT_FAULT_CODE:
        // Update both the FAULT_CODE and the HAS_FAULT flag
        // How to signal multiple updates? Requires specific framework support or client interpretation.
        // Broadcast the fault code first. Client might infer HAS_FAULT.
        return static_cast<uint16_t>(E_SakoTcpOffset::FAULT_CODE);
        // Or consider broadcasting HAS_FAULT if it's a separate TCP register?
    default:
        return 0; // No direct broadcast mapping for other registers
    }
}

ModbusBlockView *SAKO_VFD::mb_tcp_blocks() const
{
    return const_cast<ModbusBlockView *>(&_modbusBlockView);
}

short SAKO_VFD::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
    {
        Log.errorln(F("SAKO_VFD[%d]::mb_tcp_read - Invalid MB_Registers pointer"), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const uint16_t instanceBaseAddr = this->mb_tcp_base_address();
    if (instanceBaseAddr == 0)
    { // Handle cases where TCP mapping might not be configured
        Log.errorln(F("SAKO_VFD[%d]::mb_tcp_read - TCP Base Address is 0"), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const short requestedAddress = reg->startAddress;
    short offset = requestedAddress - instanceBaseAddr;
    if (offset < 1 || offset > SAKO_VFD_TCP_REG_RANGE) // Use defined range
    {
        Log.warningln(F("SAKO_VFD[%d]: Read invalid offset %d (Addr %d, Base %d)"),
                      this->slaveId, offset, requestedAddress, instanceBaseAddr);
        return (short)MB_Error::IllegalDataAddress;
    }

    uint16_t value = 0;
    bool success = false;
    E_SakoTcpOffset regOffset = static_cast<E_SakoTcpOffset>(offset);

    switch (regOffset)
    {
    case E_SakoTcpOffset::RUNNING_FREQUENCY:
        success = getFrequency(value);
        if (!success)
            value = 0xFFFF;
        break;
    case E_SakoTcpOffset::SET_FREQUENCY:
        success = _setFrequencyValid;
        value = success ? (_setFrequency / 100) : 0xFFFF;
        break;
    case E_SakoTcpOffset::OUTPUT_CURRENT:
    {
        success = getOutputCurrent(value);
        if (!success)
            value = 0xFFFF;
    }
    break;
    case E_SakoTcpOffset::OUTPUT_POWER_KW:
        success = getOutputPowerKW(value);
        if (!success)
            value = 0xFFFF;
        break;
    case E_SakoTcpOffset::OUTPUT_TORQUE_PERCENT:
        success = getOutputTorquePercent(value);
        if (!success)
            value = 0xFFFF;
        break;
    case E_SakoTcpOffset::FAULT_CODE:
        value = getFaultCode(); // Use getter which returns 0 for no fault/invalid
        success = true;         // Reading fault code status is always possible via getter
        break;
    case E_SakoTcpOffset::IS_RUNNING:
        value = isRunning() ? 1 : 0;
        success = _statusValid; // Depends on status being valid
        break;
    case E_SakoTcpOffset::HAS_FAULT:
        value = hasFault() ? 1 : 0;
        success = _faultValid; // Depends on fault code being valid
        break;
    case E_SakoTcpOffset::CMD_FREQ: // Read associated with write freq command (returns current *set* freq)
        // Return the raw set frequency value (0.01 Hz units)
        success = _setFrequencyValid;                     // Corrected: was _setFrequencyValid / 100
        value = success ? (_setFrequency / 100) : 0xFFFF; // Display in Hz
        break;
    case E_SakoTcpOffset::CMD_DIRECTION:                 // Read associated with command write returns current running state?
        value = _statusValid ? _statusRegister : 0xFFFF; // Return raw status
        success = _statusValid;
        break;
    case E_SakoTcpOffset::TARGET_REGISTER:
        value = _tcpTargetRegister; // Return the stored target register
        success = true;
        break;
    case E_SakoTcpOffset::TARGET_VALUE:
        value = 0; // This register is write-only for triggering, reads as 0
        success = true;
        break;
    default:
        return E_OK;
        // Log.warningln(F("SAKO_VFD[%d]: TCP Read unhandled offset %d"), this->slaveId, offset);
        // return (short)MB_Error::IllegalDataAddress;
    }

    return success ? value : 0xFFFF; // Return value or error indicator
}

short SAKO_VFD::setupVFD()
{
    RS485 *rs485 = (RS485 *)owner;
    rs485->modbus.writeRegister(MB_SAKO_VFD_SLAVE_ID, (uint16_t)E_SAKO_PARAM::E_SAKO_PARAM_P00_03_MAIN_FREQUENCY_SOURCE_X_SELECTION, 9, true);
    rs485->modbus.writeRegister(MB_SAKO_VFD_SLAVE_ID, (uint16_t)E_SAKO_PARAM::E_SAKO_PARAM_P00_02_COMMAND_SOURCE_SELECTION, 2, true);
    rs485->modbus.writeRegister(MB_SAKO_VFD_SLAVE_ID, (uint16_t)E_SAKO_PARAM::E_SAKO_PARAM_P00_10_MAXIMUM_FREQUENCY, 7500, true);
    rs485->modbus.writeRegister(MB_SAKO_VFD_SLAVE_ID, (uint16_t)E_SAKO_PARAM::E_SAKO_PARAM_P00_17_ACCELERATION_TIME_1, 10, true);
    rs485->modbus.writeRegister(MB_SAKO_VFD_SLAVE_ID, (uint16_t)E_SAKO_PARAM::E_SAKO_PARAM_P00_18_DECELERATION_TIME_1, 10, true);
    rs485->modbus.writeRegister(MB_SAKO_VFD_SLAVE_ID, (uint16_t)E_SAKO_PARAM::E_SAKO_PARAM_P1_01_RATED_MOTOR_POWER, 2, true);
    rs485->modbus.writeRegister(MB_SAKO_VFD_SLAVE_ID, (uint16_t)E_SAKO_PARAM::E_SAKO_PARAM_P1_02_RATED_MOTOR_VOLTAGE, 400, true);
    // rs485->modbus.writeRegister(MB_SAKO_VFD_SLAVE_ID, (uint16_t)E_SAKO_PARAM::E_SAKO_PARAM_P1_04_RATED_MOTOR_FREQUENCY, 5000, true);

    return E_OK;
}

short SAKO_VFD::serial_register(Bridge *bridge)
{
    Component::serial_register(bridge);
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&SAKO_VFD::info);
    bridge->registerMemberFunction(id, this, C_STR("setupVFD"), (ComponentFnPtr)&SAKO_VFD::setupVFD);
    return E_OK;
}

short SAKO_VFD::mb_tcp_write(MB_Registers *reg, short value)
{
    if (!reg)
        return E_INVALID_PARAMETER;

    const uint16_t tcpBaseAddr = this->mb_tcp_base_address();
    if (tcpBaseAddr == 0)
    {
        Log.errorln(F("SAKO_VFD[%d]::mb_tcp_write - TCP Base Address is 0"), this->slaveId);
        return (short)MB_Error::ServerDeviceFailure;
    }

    const short requestedTcpAddress = reg->startAddress;
    short offset = requestedTcpAddress - tcpBaseAddr;
    E_SakoTcpOffset regOffset = static_cast<E_SakoTcpOffset>(offset);

    bool commandSuccess = false;

    switch (regOffset)
    {
    case E_SakoTcpOffset::CMD_FREQ:
        commandSuccess = setFrequency(static_cast<uint16_t>(value));
        break;
    case E_SakoTcpOffset::CMD_DIRECTION:
    {
        E_SAKO_DIRECTION directionCmd = static_cast<E_SAKO_DIRECTION>(value);
        uint16_t regAddr;

        switch (directionCmd)
        {
        case E_SAKO_DIRECTION::E_SAKO_DIR_FWD:
            commandSuccess = run(); // Assumes run() sends FWD
            break;
        case E_SAKO_DIRECTION::E_SAKO_DIR_REV:
            commandSuccess = reverse();
            break;
        case E_SAKO_DIRECTION::E_SAKO_DIR_DECELERATION_STOP:
            commandSuccess = stop(); // Assumes stop() sends DECEL_STOP
            break;
        case E_SAKO_DIRECTION::E_SAKO_DIR_FREE_STOP:
            regAddr = static_cast<uint16_t>(E_SAKO_REGISTERS::E_SAKO_REGISTERS_SET_DIR);
            Log.infoln(F("SAKO_VFD[%d]: Setting FREE STOP command (Addr=0x%04X, Val=0x%04X)"), slaveId, regAddr, value);
            this->setOutputRegisterValue(regAddr, value); // Send the raw command value
            commandSuccess = true;
            break;
        case E_SAKO_DIRECTION::E_SAKO_DIR_FAULT_RESET:
            commandSuccess = resetFault();
            break;
        default:
            Log.warningln(F("SAKO_VFD[%d]: Unknown TCP direction command value %d"), this->slaveId, value);
            commandSuccess = false; // Invalid command value
            break;
        }
    }
    break;
    case E_SakoTcpOffset::TARGET_REGISTER:
        _tcpTargetRegister = static_cast<uint16_t>(value);
        Log.infoln(F("SAKO_VFD[%d]: TCP Target Register set to 0x%04X"), this->slaveId, _tcpTargetRegister);
        commandSuccess = true;
        break;
    case E_SakoTcpOffset::TARGET_VALUE:
        if (_tcpTargetRegister == 0)
        {
            Log.warningln(F("SAKO_VFD[%d]: TCP Target Value write ignored, Target Register not set (is 0)."), this->slaveId);
            commandSuccess = false;
        }
        else
        {
            RS485 *rs485 = (RS485 *)owner;
            if (rs485)
            {
                Log.infoln(F("SAKO_VFD[%d]: TCP Writing Value 0x%04X to VFD Register 0x%04X"), this->slaveId, value, _tcpTargetRegister);
                MB_Error writeStatus = rs485->modbus.writeRegister(this->slaveId, _tcpTargetRegister, static_cast<uint16_t>(value));
                commandSuccess = (writeStatus == MB_Error::Success);
                if (commandSuccess)
                {
                    Log.infoln(F("SAKO_VFD[%d]: VFD Register 0x%04X write successful."), this->slaveId, _tcpTargetRegister);
                }
                else
                {
                    Log.errorln(F("SAKO_VFD[%d]: VFD Register 0x%04X write failed. Status: %d"), this->slaveId, _tcpTargetRegister, static_cast<int>(writeStatus));
                }
                // Reset _tcpTargetRegister after the write attempt, regardless of success or failure, as per user requirement for registers to be 0 after write.
                _tcpTargetRegister = 0;
            }
            else
            {
                Log.errorln(F("SAKO_VFD[%d]: RS485 owner not found for VFD write."), this->slaveId);
                commandSuccess = false;
                _tcpTargetRegister = 0; // Also reset if owner not found, to ensure it's cleared.
            }
        }
        break;
    default:
        Log.warningln(F("SAKO_VFD[%d]: Unknown TCP write offset %d"), this->slaveId, offset);
        commandSuccess = false;
        break;
    }

    return commandSuccess ? (short)MB_Error::Success : (short)MB_Error::ServerDeviceFailure;
}

// --- Internal Helper Methods ---

void SAKO_VFD::_updateStatusFromRegister(uint16_t statusReg)
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
    //     Log.infoln(F("SAKO_VFD[%d]: Running state changed to %s"), slaveId, currentRunning ? "Running" : "Stopped");
    // }

    // Note: `_statusValid` is set when U0-61 is received in onRegisterUpdate.
    // Note: `_faultValid` and `_faultCode` are set when U0-62 is received.
}

// --- Internal Helper: Update VFD Operational State ---
void SAKO_VFD::_updateVfdState()
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
        Log.infoln(F("SAKO_VFD[%d]: VFD State changed from %d to %d"), slaveId, previousState, _vfdState);
    }
}

short SAKO_VFD::getTorque()
{
    if (!isRunning())
        return E_INVALID_PARAMETER;
    float torque = _currentCurrent * _currentFrequency;
    return E_OK;
}

short SAKO_VFD::reset() { return E_OK; }

bool SAKO_VFD::getOutputCurrent(uint16_t &value) const
{
    if (_currentValid)
    {
        value = static_cast<uint16_t>(_currentCurrent); // _currentCurrent now holds the raw value as a float
        return true;
    }
    value = 0; // Default if not valid
    return false;
}

bool SAKO_VFD::getOutputPowerKW(uint16_t &value) const
{
    if (_outputPowerKWValid)
    {
        value = _outputPowerKW;
        return true;
    }
    value = 0;
    return false;
}

bool SAKO_VFD::getOutputTorquePercent(uint16_t &value) const
{
    if (_outputTorquePercentValid)
    {
        value = _outputTorquePercent;
        return true;
    }
    value = 0;
    return false;
}

#endif // ENABLE_RS485
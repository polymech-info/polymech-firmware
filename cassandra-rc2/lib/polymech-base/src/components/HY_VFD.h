#ifndef HY_VFD_H
#define HY_VFD_H

#include "config.h"

#ifdef ENABLE_RS485 // Assuming this VFD also uses RS485

#include <ArduinoLog.h>
#include <Component.h>
#include <modbus/ModbusRTU.h>
#include <modbus/ModbusTypes.h>
#include <xstatistics.h>
#include <ValueWrapper.h>

#define HY_VFD_DEFAULT_READ_INTERVAL 100
#define HY_VFD_TCP_REG_RANGE 16

// Enum for Retract State Machine
typedef enum
{
  E_VFD_RETRACT_STATE_NONE = 0,
  E_VFD_RETRACT_STATE_BRAKING = 1,
  E_VFD_RETRACT_STATE_STOPPED = 2,
  E_VFD_RETRACT_STATE_REVERSING = 3,
  E_VFD_RETRACT_STATE_BRAKE_REVERSING = 4,
  E_VFD_RETRACT_STATE_RETRACTED = 5,
} E_VFD_RETRACT_STATE;

// Enum for VFD Operational State
typedef enum
{
  E_VFD_STATE_STOPPED = 1,
  E_VFD_STATE_DECELERATING = 2,
  E_VFD_STATE_RUNNING = 3,
  E_VFD_STATE_ACCELERATING = 4,
  E_VFD_STATE_ERROR = 8
} E_VFD_STATE;

enum class E_SakoTcpOffset : ushort
{
    RUNNING_FREQUENCY = 1,     // Corresponds to U0-00
    SET_FREQUENCY = 2,         // Corresponds to U0-01
    OUTPUT_CURRENT = 3,        // Corresponds to U0-04
    OUTPUT_POWER_KW = 4,       // Corresponds to U0-05
    OUTPUT_TORQUE_PERCENT = 5, // Corresponds to U0-06
    FAULT_CODE = 6,            // Corresponds to U0-62
    IS_RUNNING = 7,            // Derived from U0-61
    HAS_FAULT = 8,             // Derived from U0-62
    STATE = 9,                 // E_VFD_STATE
    CMD_FREQ = 10,              // Write Frequency (Uses SAKO_REG_SET_FREQ)
    CMD_DIRECTION = 11,        // Write Direction/Control (Uses E_SAKO_REGISTERS_SET_DIR)
    CMD_COMMAND = 12,          // Internal command
    TARGET_REGISTER = 13,      // Write target register address
    TARGET_VALUE = 14,         // Write value to target register
};

class HY_VFD : public RTU_Base
{
public:
    static constexpr int HY_TCP_BLOCK_COUNT = 14;
    // Constructor
    HY_VFD(Component* owner, uint8_t slaveId, millis_t readInterval = HY_VFD_DEFAULT_READ_INTERVAL);
    virtual ~HY_VFD() = default;
    // --- Component Interface ---
    virtual short setup() override;
    virtual short loop() override;
    virtual short info() override;
    short reset();
    short getTorque();

    // --- Modbus Register Update Notification ---
    virtual void onRegisterUpdate(uint16_t address, uint16_t newValue) override;
    virtual void onError(ushort errorCode, const char* errorMessage) override;

    // --- Getters for Specific VFD Values ---
    // Add getters relevant to a VFD, e.g., Frequency, Speed, Status, Fault codes
    bool getFrequency(uint16_t& value) const; // Returns frequency in 0.01 Hz units
    bool getSpeed(uint16_t& value) const; // Example: speed might be integer RPM
    bool isRunning() const;
    bool hasFault() const;
    uint16_t getFaultCode() const;
    E_VFD_STATE getVfdState() const;
    bool getOutputPowerKW(uint16_t& value) const;
    bool getOutputTorquePercent(uint16_t& value) const;
    bool getOutputCurrent(uint16_t& value) const;

    // --- Setters for VFD Control ---
    bool setFrequency(uint16_t value); // Expects frequency in 0.01 Hz units
    bool run();
    bool reverse();
    bool stop();
    bool resetFault();
    bool retract();

    // --- Modbus Block Definitions ---
    virtual ModbusBlockView* mb_tcp_blocks() const override;
    virtual short mb_tcp_read(MB_Registers * reg) override;
    virtual short mb_tcp_write(MB_Registers * reg, short value) override;
    virtual ushort mb_tcp_error(MB_Registers *reg) override;

    // --- Modbus TCP Mapping Overrides ---
    virtual uint16_t mb_tcp_base_address() const override;
    virtual uint16_t mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const override;


private:
    millis_t _readInterval;

    // --- Local State Storage ---
    // Store relevant VFD parameters and their validity flags
    uint16_t _currentFrequency = 0; // 0.01 Hz units
    uint16_t _currentSpeed = 0;
    uint16_t _setFrequency = 0; // 0.01 Hz units
    uint16_t _currentCurrent = 0;
    uint16_t _statusRegister = 0; // Example status register
    uint16_t _faultCode = 0;
    uint16_t _outputPowerKW = 0;
    uint16_t _outputTorquePercent = 0;
    bool _frequencyValid = false;
    bool _speedValid = false;
    bool _setFrequencyValid = false;
    bool _currentValid = false;
    bool _statusValid = false;
    bool _faultValid = false;
    bool _outputPowerKWValid = false;
    bool _outputTorquePercentValid = false;

    uint16_t _tcpTargetRegister = 0; // Stores the value written to TARGET_REGISTER offset

    // Retract State
    E_VFD_RETRACT_STATE _retractState = E_VFD_RETRACT_STATE_NONE;

    // Operational State
    E_VFD_STATE _vfdState = E_VFD_STATE_STOPPED;
    ValueWrapper<uint16_t> _stateWrapper;

    // Statistics
    Statistic _ampStats;

    MB_Registers _modbusBlocks[HY_TCP_BLOCK_COUNT];
    mutable ModbusBlockView _modbusBlockView;
    bool _modbusBlocksInitialized; // Added for on-demand initialization

    // Add internal helper methods if needed
    void _updateStatusFromRegister(uint16_t statusReg);
    void _updateVfdState();
    short serial_register(Bridge *bridge) override;
    short setupVFD();
};

#endif // ENABLE_RS485
#endif // HY_VFD_H 
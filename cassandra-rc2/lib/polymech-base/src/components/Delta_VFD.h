#ifndef DELTA_VFD_H
#define DELTA_VFD_H

#include "config.h"

#ifdef ENABLE_RS485 // Assuming this VFD also uses RS485

#include <ArduinoLog.h>
#include <Component.h>
#include <modbus/ModbusRTU.h>
#include <modbus/ModbusTypes.h>
#include <xstatistics.h>
#include <ValueWrapper.h>
#include "./DeltaTypesBase.h"
#include "VFD_Base.h"

// Forward declaration from DeltaTypesEx.h
enum class E_DELTA_MS300_FAULT_STATUS_FLAGS : uint16_t;

// Optional torque monitoring - disabled by default to save Modbus bandwidth
// #define DELTA_READ_TORQUE

#define DELTA_VFD_DEFAULT_READ_INTERVAL 100
#define DELTA_VFD_TCP_REG_RANGE 16
#define DELTA_VFD_STATUS_MONITOR_REG_COUNT 7  // Status codes (2100H) through Output voltage (2106H)
#define DELTA_MAX_OP_FREQ_MOTOR_1 4500

enum class E_DELTA_PARAM : int
{
  E_DELTA_PARAM_P00_03_MAIN_FREQUENCY_SOURCE_X_SELECTION = 1,
  E_DELTA_PARAM_P00_02_COMMAND_SOURCE_SELECTION = 2,
  E_DELTA_PARAM_P00_10_MAXIMUM_FREQUENCY = 3,
  E_DELTA_PARAM_P00_17_ACCELERATION_TIME_1 = 4,
  E_DELTA_PARAM_P00_18_DECELERATION_TIME_1 = 5,
  E_DELTA_PARAM_P1_01_RATED_MOTOR_POWER = 6,
  E_DELTA_PARAM_P1_02_RATED_MOTOR_VOLTAGE = 7,

};

enum class E_DELTA_REGISTERS : int 
{
    E_DELTA_REG_SET_FREQ = 0x2001,
    E_DELTA_REG_SET_DIR = 2,
    E_DELTA_REG_SET_COMMAND = 3,
    E_DELTA_REG_SET_TARGET_REGISTER = 4,
    E_DELTA_REGISTERS_SET_DIR = 5,
};

enum class E_DELTA_ERROR : int 
{
    E_DELTA_ERROR_NO_FAULT = 0,
    E_DELTA_ERROR_OVER_CURRENT = 1,
    E_DELTA_ERROR_OVER_VOLTAGE = 2,
    E_DELTA_ERROR_OVER_TEMPERATURE = 3,
};

enum class E_DELTA_DIRECTION : int 
{
    E_DELTA_DIR_FWD = 1,
    E_DELTA_DIR_REV = 2,
    E_DELTA_DIR_JOGGING = 3,
    E_DELTA_DIR_REVERSE_JOGGING = 4,
    E_DELTA_DIR_FREE_STOP = 5,
    E_DELTA_DIR_DECELERATION_STOP = 6,
    E_DELTA_DIR_FAULT_RESET = 7
};

enum class E_DELTA_MON : uint16_t
{
  E_DELTA_MON_OUTPUT_TORQUE = 8459,

  
  E_DELTA_MON_RUNNING_FREQUENCY_HZ = 1,
  E_DELTA_MON_SET_FREQUENCY_HZ = 2,
  E_DELTA_MON_OUTPUT_CURRENT_A = 3,
  E_DELTA_MON_OUTPUT_POWER_KW = 4,
  E_DELTA_MON_OUTPUT_TORQUE_PERCENT = 5,
  E_DELTA_MON_FAULT_CODE = 6,
  E_DELTA_MON_IS_RUNNING = 7,
  E_DELTA_MON_HAS_FAULT = 8,
  E_DELTA_MON_STATE = 9,
  E_DELTA_MON_CMD_FREQ = 10,
  E_DELTA_MON_CMD_DIRECTION = 11,
  E_DELTA_MON_CMD_COMMAND = 12,
  E_DELTA_MON_TARGET_REGISTER = 13,
  E_DELTA_MON_TARGET_VALUE = 14,
  E_DELTA_MON_AC_DRIVE_RUNNING_STATE = 15,
  E_DELTA_MON_AUTO_STOP_ACCELERATING = 16,
  E_DELTA_MON_AUTO_STOP_DECELERATING = 17,
  E_DELTA_MON_RUN_STANDARD_DEC = 18,
  E_DELTA_MON_DEC_OFF = 19,
  E_DELTA_MON_AUTO_STOP_SLOW_RUN = 20,
  E_DELTA_MON_STO_LATCH = 21,
  E_DELTA_MON_OPHL_WARN_AND_RAMP_TO_STOP = 22,
  E_DELTA_MON_OPHL_WARN_AND_COAST_TO_STOP = 23,
  E_DELTA_MON_OPHL_NO_WARNING = 24,
  E_DELTA_MON_OPHL_DETECTION_TIME = 25,
  E_DELTA_MON_OPHL_CURRENT_LEVEL = 26,
  E_DELTA_MON_OPHL_DC_BRAKE_TIME = 27,
  E_DELTA_MON_LVX_AUTO_RESET = 28,
  E_DELTA_MON_INPUT_PHASE_LOSS_TREATMENT = 29,
  E_DELTA_MON_DERATING_PROTECTION = 30,
  E_DELTA_MON_DC_BRAKE_CURRENT_STARTUP = 31,
  E_DELTA_MON_DC_BRAKE_TIME_STARTUP = 32,
  E_DELTA_MON_DC_BRAKE_START_FREQUENCY = 33,
  E_DELTA_MON_VOLTAGE_INCREASING_GAIN = 34,
  E_DELTA_MON_RESTART_AFTER_MOMENTARY_POWER_LOSS = 35,
  E_DELTA_MON_ALLOWED_POWER_LOSS_DURATION = 36,
  E_DELTA_MON_BASE_BLOCK_TIME = 37,
  E_DELTA_MON_CURRENT_LIMIT_OF_SPEED_TRACKING = 38,
  E_DELTA_MON_FILTER_TIME_TORQUE_COMMAND = 39,
  E_DELTA_MON_FILTER_TIME_SLIP_COMPENSATION = 40,
  E_DELTA_MON_TORQUE_COMPENSATION_GAIN_MOTOR_1 = 41,
  E_DELTA_MON_TORQUE_COMPENSATION_GAIN_MOTOR_2 = 42,
  E_DELTA_MON_TORQUE_COMPENSATION_GAIN_MOTOR_3 = 43,
  E_DELTA_MON_TORQUE_COMPENSATION_GAIN_MOTOR_4 = 44,
  E_DELTA_MON_DWELL_TIME_AT_ACCEL = 45,
  E_DELTA_MON_DWELL_FREQUENCY_AT_ACCEL = 46,
  E_DELTA_MON_CURRENT_FAULT_CODE = 47,
};

enum class E_DELTA_TCP_OFFSET : ushort
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
    CMD_FREQ = 10,              // Write Frequency (Uses DELTA_REG_SET_FREQ)
    CMD_DIRECTION = 11,        // Write Direction/Control (Uses E_DELTA_REGISTERS_SET_DIR)
    CMD_COMMAND = 12,          // Custom commands (E_DELTA_CMD)
    TARGET_REGISTER = 13,      // Write target register address
    TARGET_VALUE = 14,         // Write value to target register
};

enum class E_DELTA_CMD : ushort
{
    E_DTC_NONE = 0,           // No command
    E_DTC_INFO = 1,           // Call info() method
    E_DTC_RESET = 2,          // Reset VFD
    E_DTC_SETUP_VFD = 3,      // Call setupVFD()
    E_DTC_RESET_FAULT = 4,    // Reset fault status
};

class DELTA_VFD : public VFD_Base
{
public:
    static constexpr int DELTA_TCP_BLOCK_COUNT = 14;
    // Constructor
    DELTA_VFD(Component* owner, uint8_t slaveId, millis_t readInterval = DELTA_VFD_DEFAULT_READ_INTERVAL);
    virtual ~DELTA_VFD() = default;
    // --- Component Interface ---
    virtual short setup() override;
    virtual short loop() override;
    virtual short info() override;
    short reset() override;
    short getTorque();

    // --- Modbus Register Update Notification ---
    virtual bool onRegisterUpdate(uint16_t address, uint16_t newValue) override;
    virtual void onError(ushort errorCode, const char* errorMessage) override;

    // --- Getters for Specific VFD Values ---
    // Add getters relevant to a VFD, e.g., Frequency, Speed, Status, Fault codes
    bool getFrequency(uint16_t& value) const; // Returns frequency in 0.01 Hz units
    bool getSpeed(uint16_t& value) const; // Example: speed might be integer RPM
    bool isRunning() const;
    bool hasFault() const;
    uint16_t getFaultCode() const;
    bool hasFaultType(E_DELTA_MS300_FAULT_STATUS_FLAGS faultType) const;
    E_VFD_STATE getVfdState() const;
    bool getOutputPowerKW(uint16_t& value) const;
#ifdef DELTA_READ_TORQUE
    bool getOutputTorquePercent(uint16_t& value) const;
#endif
    bool getOutputCurrent(uint16_t& value) const;

    // --- Setters for VFD Control ---
    bool setFrequency(uint16_t value); // Expects frequency in 0.01 Hz units
    bool run();
    bool reverse();
    short stop() override;
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
    // --- DELTA-specific State Storage ---
    uint16_t _currentSpeed = 0;
    uint8_t _faultStatusFlags = 0; // Fault status flags from register 0x2100 high byte
    uint16_t _outputPowerKW = 0;
    uint16_t _outputTorquePercent = 0;
    bool _speedValid = false;
    bool _outputPowerKWValid = false;
    bool _outputTorquePercentValid = false;

    uint16_t _tcpTargetRegister = 0; // Stores the value written to TARGET_REGISTER offset
    E_DELTA_CMD _lastCommand = E_DELTA_CMD::E_DTC_NONE; // Stores the last custom command

    MB_Registers _modbusBlocks[DELTA_TCP_BLOCK_COUNT];
    mutable ModbusBlockView _modbusBlockView;
    bool _modbusBlocksInitialized; // Added for on-demand initialization

    // Add internal helper methods if needed
    void _updateStatusFromRegister(uint16_t statusReg) override;
    void _updateVfdState() override;
    short serial_register(Bridge *bridge) override;
    short setupVFD();
};

#endif // ENABLE_RS485
#endif // DELTA_VFD_H 
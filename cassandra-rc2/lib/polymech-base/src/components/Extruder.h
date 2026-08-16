#ifndef EXTRUDER_H
#define EXTRUDER_H

#include "config.h"

#ifdef ENABLE_EXTRUDER

#include "./3PosAnalog.h"
#include "./POT.h"
#include "./SAKO_VFD.h"
#include "enums.h"
#include <ArduinoLog.h>
#include <Component.h>
#include <modbus/ModbusTCP.h>

// Component Constants
#define EXTRUDER_COMPONENT_ID 760
#define EXTRUDER_COMPONENT_NAME "Extruder"

// Speed Presets (in 0.01 Hz units for SAKO_VFD)
#define EXTRUDER_SPEED_SLOW_HZ 10   // 10.00 Hz
#define EXTRUDER_SPEED_MEDIUM_HZ 25 // 25.00 Hz
#define EXTRUDER_SPEED_FAST_HZ 50   // 50.00 Hz (currently unused, but defined)

// Speed POT Configuration for Extruding (multiplier for MEDIUM speed)
// POT value 0-100. Example: 0 maps to 0.5x, 50 maps to 1.0x, 100 maps to 1.5x
// MEDIUM speed.
#define EXTRUDER_SPEED_POT_MIN_MULTIPLIER 0.5f
#define EXTRUDER_SPEED_POT_MAX_MULTIPLIER 1.5f

// Overload POT Configuration and Jamming (using VFD Torque %)
// POT value 0-100. Maps to a torque threshold in percent (0-100).
// SAKO_VFD component now provides getTorque() returning 0-100.
#define EXTRUDER_OVERLOAD_POT_MIN_TORQUE_PERCENT 50 // Example: 50% Torque
#define EXTRUDER_OVERLOAD_POT_MAX_TORQUE_PERCENT 95 // Example: 95% Torque
#define PLUNGER_JAMMED_DURATION_MS                                             \
  2000 // Time torque must be above threshold to be JAMMED
#define EXTRUDER_VFD_READ_INTERVAL_MS                                          \
  200 // How often to check VFD torque for jamming
#define EXTRUDER_AUTO_MODE_HOLD_DURATION_MS                                    \
  2000 // Time joystick must be held for auto mode

// Modbus Configuration
#define EXTRUDER_MB_BASE_ADDRESS                                               \
  EXTRUDER_COMPONENT_ID // Using component ID as base
#define EXTRUDER_MB_STATE_OFFSET 0
#define EXTRUDER_MB_COMMAND_OFFSET 1
#define EXTRUDER_MB_BLOCK_COUNT 2
#define EXTRUDER_MAX_RUN_TIME_MEDIUM_SPEED_MS                                  \
  15000 // Max runtime at medium speed

enum class ExtruderModbusCommand : short {
  NO_COMMAND = 0,
  CMD_EXTRUDE = 2,
  CMD_STOP = 3,
  CMD_INFO = 4
};

// Extruder States
enum class ExtruderState : uint8_t {
  IDLE,
  EXTRUDING_MANUAL, // Joystick held UP, VFD forwarding, monitoring for
                    // auto-mode hold time
  EXTRUDING_AUTO,   // Auto-extruding after joystick hold, VFD forwarding
  STOPPING,         // Transition state to stop VFD
  JAMMED,
  RESETTING_JAM // State to handle reset after jam
};

class Extruder : public Component {
public:
  Extruder(Component *owner, SAKO_VFD *vfd, Pos3Analog *joystick = nullptr,
           POT *speedPot = nullptr, POT *overloadPot = nullptr);
  ~Extruder() override = default;

  short setup() override;
  short loop() override;
  short info() override;
  short debug() override;
  short serial_register(Bridge *b) override;
  short init();
  short reset();

  // Modbus TCP Interface
  ModbusBlockView *mb_tcp_blocks() const override;
  void mb_tcp_register(ModbusTCP *mgr) override;
  short mb_tcp_read(MB_Registers *reg) override;
  short mb_tcp_write(MB_Registers *reg, short networkValue) override;

  // Public commands for serial/external control
  short cmd_extrude();
  short cmd_stop();

private:
  SAKO_VFD *_vfd;
  Pos3Analog *_joystick;
  POT *_speedPot;
  POT *_overloadPot;

  ExtruderState _currentState;
  Pos3Analog::E_POS3_DIRECTION _lastJoystickDirection;

  uint16_t _currentSpeedPotValue;    // 0-100, defaults to 100 if pot is null
  uint16_t _currentOverloadPotValue; // 0-100, defaults to 100 if pot is null
  float _calculatedExtrudingSpeedHz; // Calculated speed for extruding (0.01Hz
                                     // units)
  uint8_t _calculatedOverloadThresholdPercent; // Calculated torque threshold
                                               // (0-100%)

  unsigned long _lastStateChangeTimeMs;
  unsigned long _jammedStartTimeMs;
  unsigned long _lastVfdReadTimeMs;
  unsigned long _joystickHoldStartTimeMs; // Timer for joystick hold duration
  short _modbusCommandRegisterValue; // Holds the current value of the Modbus
                                     // command register
  unsigned long
      _operationStartTimeMs; // Start time of current extrude operation
  unsigned long _currentMaxOperationTimeMs; // Calculated max duration for
                                            // current operation

  // Helper methods
  void _handleIdleState();
  void _handleExtrudingManualState();
  void _handleExtrudingAutoState();
  void _handleStoppingState();
  void _handleJammedState();
  void _handleResettingJamState();

  void _updatePotValues();
  void _checkVfdForJam();
  void _transitionToState(ExtruderState newState);

  // VFD interaction wrappers
  void _vfdStartForward(uint16_t frequencyCentiHz);
  void _vfdStartReverse(uint16_t frequencyCentiHz);
  void _vfdStop();
  void _vfdResetJam();
};
#endif
#endif // EXTRUDER_H
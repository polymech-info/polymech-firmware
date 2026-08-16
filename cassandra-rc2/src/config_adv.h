#ifndef CONFIG_ADV_H
#define CONFIG_ADV_H

////////////////////////////////////////////////////////////////////////////////
//  
// Power settings

// optional current sensor to validate primary power is there
// #define POWER_CSENSOR_PRIMARY                           CONTROLLINO_A15

// optional current sensor to validate primary power is there
// #define POWER_CSENSOR_SECONDARY                         CONTROLLINO_A14

/////////////////////////////////////////////////////////////
//
// Motor load settings, this requires a current sensor or can be
// taken from the VFD's output. 

// the interval to read the current
#define MOTOR_LOAD_READ_INTERVAL                        100

// the current measured when the motor runs idle, min - max range
#define MOTOR_IDLE_LOAD_RANGE_MIN                       5
#define MOTOR_IDLE_LOAD_RANGE_MAX                       20

// the current measured when the motor is under load, min - max range
#define MOTOR_LOAD_RANGE_MIN                  20
#define MOTOR_LOAD_RANGE_MAX                  60

// the current measured when the motor is overloaded, min - max range
#define MOTOR_OVERLOAD_RANGE_MIN                        80
#define MOTOR_OVERLOAD_RANGE_MAX                        800

// #define MOTOR_MIN_DT                                    2500

/////////////////////////////////////////////////////////////
//
//  Error codes
//
#define E_MSG_OK            "Ok"

// power failures
#define E_POWER_PRIM_ON     145               // Power is on whilst it shouldn't be
#define E_POWER_SEC_ON      147               // Power is on whilst it shouldn't be
#define E_POWER             150               // Nothing is online

#define E_VFD_OFFLINE       0x102             // VFD should be online (Used 0x102 directly)

// sensor failures
#define E_VFD_CURRENT       200               // VFD current abnormal: below or above average
#define E_OPERATING_SWITCH  220               // Operating switch invalid value

////////////////////////////
//
// Sub system failures
//

// bridge
#define E_BRIDGE_START      3000                // base offset for custom bridge errors
#define E_BRIDGE_CUSTOM(A)  E_BRIDGE_START+A      // Custom bridge error
#define E_BRIDGE_PARITY     E_BRIDGE_CUSTOM(1)  // @todo, parity check failure 
#define E_BRIDGE_CRC        E_BRIDGE_CUSTOM(2)  // @todo, crc  failure
#define E_BRIDGE_FLOOD      E_BRIDGE_CUSTOM(3)  // @todo, msg queue

// extrusion
#define E_EX_BASE           4000                // base offset extruder
#define E_EX_CUSTOM(A)      E_EX_BASE+A         // Custom bridge error

///////////////////////////////////////////////////////////////////////
//
// I/O Advanced Settings - Board/Platform specific

#define ANALOG_POT_READ_INTERVAL 5            // The interval to read the analog switch
#define ANALOG_SWITCH_READ_INTERVAL 20        // The interval to read the analog switch


#define ANALOG_INPUT_MAX_LEVEL_0 750            // The trigger value to detect an analog input value as HIGH or ON

#define ANALOG_INPUT_MAX_LEVEL_1 810            // The maximum value for the analog input (POT)
#define ANALOG_INPUT_THRESHOLD_1 500            // The trigger value to detect an analog input value as HIGH or ON

#define ANALOG_INPUT_MIN_THRESHOLD_0 700        // Minimum threshold for 3Pos switches
#define ANALOG_INPUT_MIN_DT_0        3          // Minimum difference to detect a change in the analog input  

///////////////////////////////////////////////////////////////////////
//
// Stepper Advanced Settings
//

#define STEPPER_MAX_SPEED_0 100
#define STEPPER_MODUBUS_RANGE 4
#define STEPPER_DEFAULT_SPEED_0 50
#define STEPPER_DEFAULT_DIR_0 0
#define STEPPER_PULSE_WIDTH_0 20
#define STEPPER_OVERLOAD_THRESHOLD_0 800

#define STEPPER_PULSE_WIDTH_1 20
#define STEPPER_DEFAULT_SPEED_1 50
#define STEPPER_DEFAULT_DIR_1 0
#define STEPPER_MAX_SPEED_1 100

#define ANALOG_3POS_SWITCH_LEFT_RANGE 50
#define ANALOG_3POS_SWITCH_CENTER_RANGE 100
#define ANALOG_3POS_SWITCH_RIGHT_RANGE 100
#define ANALOG_3POS_SWITCH_DEBOUNCE 500

// MotorLoad : Number of samples to make an average of
#define MOTOR_LOAD_SAMPLES 10
#define MOTOR_LOAD_ALARM_LIMIT 900  // 0-1000
#define MOTOR_LOAD_UPDATE_INTERVAL 50 // ms

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Error codes
// Common error codes
#define E_INVALID_PARAMETERS 0x1001
#define E_POWER_PRIM_OFF     0x102    // Main power problem
#define E_POWER_SEC_OFF      0x103    // Secondary power problem
#define E_POWER_SEC_LOW      0x104    // Second power problem, low batteries
#define E_POWER_PRIM_LOW     0x105    // Main power problem, voltage low
#define E_POWER_PRIM_HIGH    0x106    // Main power problem, voltage high

// Removed E_VFD_* error codes

// Thermal Faults
#define E_THERM_AMBIENT_HIGH    0x130    // Ambient temperature high
#define E_THERM_MOTOR_HIGH      0x131    // motor temperature high
#define E_THERM_CONTROL_HIGH    0x132    // Control box over heating

#define E_CURRENT       200               // abnormal current: below or above average

#endif
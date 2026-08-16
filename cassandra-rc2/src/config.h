#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <stdint.h>

#include "config_adv.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// 1. Core System
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//
// Overrides

// #define WS_MAX_QUEUED_MESSAGES 128
// #define ENABLE_COMPONENT_STATS

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// 2. Platform
//
///////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Component IDs
//
typedef enum COMPONENT_KEY {
  COMPONENT_KEY_LOGGER = 10,
  COMPONENT_KEY_SETTINGS = 11,
  COMPONENT_KEY_RUNTIME_STATE = 250,
  COMPONENT_KEY_RELAY_0 = 300,
  COMPONENT_KEY_RELAY_1 = 301,
  COMPONENT_KEY_RELAY_2 = 302,
  COMPONENT_KEY_RELAY_3 = 303,
  COMPONENT_KEY_RELAY_4 = 304,
  COMPONENT_KEY_RELAY_5 = 305,
  COMPONENT_KEY_SOLENOID_0 = 310,
  COMPONENT_KEY_SOLENOID_1 = 311,
  COMPONENT_KEY_PID_0 = 100,
  COMPONENT_KEY_PID_1 = 101,
  COMPONENT_KEY_PID_2 = 102,
  COMPONENT_KEY_ANALOG_0 = 350,
  COMPONENT_KEY_ANALOG_1 = 351,
  COMPONENT_KEY_ANALOG_2 = 352,
  COMPONENT_KEY_LOADCELL_0 = 360,
  COMPONENT_KEY_LOADCELL_1 = 361,
  COMPONENT_KEY_ANALOG_3POS_SWITCH_0 = 420,
  COMPONENT_KEY_ANALOG_3POS_SWITCH_1 = 421,
  COMPONENT_KEY_ANALOG_LEVEL_SWITCH_0 = 450,
  COMPONENT_KEY_ANALOG_LEVEL_SWITCH_1 = 451,
  COMPONENT_KEY_JOYSTICK_0 = 500,
  COMPONENT_KEY_PUSH_BUTTON_0 = 510,
  COMPONENT_KEY_STEPPER_0 = 601,
  COMPONENT_KEY_STEPPER_1 = 602,
  COMPONENT_KEY_PID = 620,
  COMPONENT_KEY_EXTRUDER = 650,
  COMPONENT_KEY_PLUNGER = 670,
  COMPONENT_KEY_FEEDBACK_0 = 701,
  COMPONENT_KEY_FEEDBACK_1 = 730,
  COMPONENT_KEY_FEEDBACK_2 = 740,
  COMPONENT_KEY_PRESS_CYLINDER_0 = 760,
  COMPONENT_KEY_SAKO_VFD = 750,
  COMPONENT_KEY_DELTA_VFD = 752,
  COMPONENT_KEY_HY_VFD = 761,
  COMPONENT_KEY_RS485_TESTER = 800,
  COMPONENT_KEY_RS485 = 801,
  COMPONENT_KEY_AMPERAGE_BUDGET_MANAGER = 850,
  COMPONENT_KEY_GPIO_MAP = 810,
  COMPONENT_KEY_REST_SERVER = 900,
  COMPONENT_KEY_PROFILE_START = 910,
  COMPONENT_KEY_PRESSURE_PROFILE_START = 920,
  COMPONENT_KEY_SIGNAL_PLOT_START = 920,
  COMPONENT_KEY_END = 1000,
  COMPONENT_KEY_TEST_NV = 1001,
  COMPONENT_KEY_OPERATOR_SWITCH = 1002,
  COMPONENT_KEY_TEST_NV_PB = 1003,
  COMPONENT_KEY_MODBUS_MIRROR = 2000,
  COMPONENT_KEY_INFLUXDB = 2001,
} COMPONENT_KEY;

////////////////////////////////////////////////////////////////////////////////
//
// Platform

// Automatic platform detection
#if defined(ARDUINO_PORTENTA_H7_M7)
#define PLATFORM_PORTENTA_H7_M7
#define BOARD_NAME "Portenta H7 M7"
#include <Arduino_MachineControl.h>
using namespace machinecontrol;
#elif defined(ARDUINO_CONTROLLINO_MEGA)
#define PLATFORM_CONTROLLINO_MEGA
#define BOARD_NAME "Controllino Mega"
#include <Controllino.h>
#elif defined(ESP32)
#define PLATFORM_ESP32
#define BOARD_NAME "ESP32"
#elif defined(ARDUINO_AVR_UNO) // Detect Arduino Uno
#define PLATFORM_ARDUINO_UNO
#define BOARD_NAME "Arduino Uno"
#else
#error "Unsupported platform"
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Debugging
#ifndef DISABLE_SERIAL_LOGGING
// Serial Bridge Debugging Switches
// #x #define BRIDGE_DEBUG_REGISTER
// #x #define BRIDGE_DEBUG_CALL_METHOD
// Serial Command Messaging Debugging Switches
// #x #define DEBUG_MESSAGES_PARSE
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// App Features

// When printing Modbus registers (via serial), print register descriptions
// #define HAS_MODBUS_REGISTER_DESCRIPTIONS 1

///////////////////////////////////////////////////////
//
// Status Feedback Settings

#define STATUS_WARNING_PIN GPIO_PIN_CH6
#define STATUS_ERROR_PIN GPIO_PIN_CH5
#define STATUS_BLINK_INTERVAL 800

///////////////////////////////////////////////////////
//
// Serial Settings
/** @brief Serial port baud rate */
#define SERIAL_BAUD_RATE 38400
/** @brief Parse commands every 100ms */
#define SERIAL_COMMAND_PARSE_INTERVAL 200
// #define ENABLE_SERIAL_BRIDGE
#define LOOP_CYCLE_INTERVAL 1

#define RUNTIME_STATE_FILENAME "/runtime.json"
#define RUNTIME_STATE_SAVE_INTERVAL 10000
#define FILESYSTEM_WRITE_GRACE_PERIOD_MS 500
////////////////////////////////////////////////////////////////
//
// Auxiliary Settings : Relays

#ifdef PLATFORM_ESP32
// Waveshare - CH6 Board
#define GPIO_PIN_CH1 1     // CH1 Control GPIO
#define GPIO_PIN_CH2 2     // CH2 Control GPIO
#define GPIO_PIN_CH3 41    // CH3 Control GPIO
#define GPIO_PIN_CH4 42    // CH4 Control GPIO
#define GPIO_PIN_CH5 45    // CH5 Control GPIO
#define GPIO_PIN_CH6 46    // CH6 Control GPIO
#define GPIO_PIN_RGB 38    // RGB Control GPIO
#define GPIO_PIN_Buzzer 21 // Buzzer Control GPIO
#define AUX_RELAY_0 GPIO_PIN_CH6
#define AUX_RELAY_1 GPIO_PIN_CH5
#endif

////////////////////////////////////////////////////////////////
//
// Auxiliary Settings : Analog Inputs (POTs)
//
#define MB_ANALOG_0 15
#define MB_ANALOG_1 7

#define POT_RAW_MAX_VALUE 4095   // Max raw ADC  value
#define POT_SCALED_MAX_VALUE 100 // Max scaled value (0-100)

// #define MB_GPIO_MB_MAP_7 7
// #define MB_GPIO_MB_MAP_15 15
////////////////////////////////////////////////////////////////
//
// Auxiliary Settings : Analog Level Switch
//
#ifdef PLATFORM_ESP32
// #define PIN_ANALOG_LEVEL_SWITCH_0 34    // <<< CHOOSE YOUR ADC PIN
#endif

/** @brief Analog Level Switch 0 */
#define ID_ANALOG_LEVEL_SWITCH_0 200
#define ALS_0_NUM_LEVELS 4   // Number of slots
#define ALS_0_ADC_STEP 800   // ADC counts per slot
#define ALS_0_ADC_OFFSET 200 // ADC value for the start of the first slot
#define ALS_0_MB_ADDR 60     // Modbus base address (Uses 60-65 for 4 slots)

////////////////////////////////////////////////////////////////
//
// Auxiliary Settings : LED Feedback
//
// #define PIN_LED_FEEDBACK_0          1  // <<< CHOOSE NEOPIXEL DATA PIN
#define LED_PIXEL_COUNT_0 1
#define ID_LED_FEEDBACK_0 210
#define LED_FEEDBACK_0_MB_ADDR 70
// #define LED_UPDATE_INTERVAL_MS 20

////////////////////////////////////////////////////////////////
//
// Auxiliary Settings : Analog Switch Inputs (3 Position Switches)
//
#ifdef PLATFORM_ESP32
// #define AUX_ANALOG_3POS_SWITCH_0 34
// #define AUX_ANALOG_3POS_SWITCH_1 35
#endif

////////////////////////////////////////////////////////////////
//
// Auxiliary Settings : Joystick
//
#ifdef PLATFORM_ESP32

#define PIN_AUX_1 16 // push button 1 - Waveshare - ESP-32-S3-CH6

#define PIN_JOYSTICK_UP 40    // UP direction pin
#define PIN_JOYSTICK_DOWN 39  // DOWN direction pin
#define PIN_JOYSTICK_LEFT 17  // LEFT direction pin
#define PIN_JOYSTICK_RIGHT 16 // RIGHT direction pin

#define PIN_OPERATOR_SWITCH_STOP 15
#define PIN_OPERATOR_SWITCH_CYCLE 14

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Temperature Control & Profiles
//
#define TEMP_PROFILE_MAX_TARGET_REGS NUM_OMRON_DEVICES

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Features
//

// Auxiliary Components
// #define ENABLE_JOYSTICK (4P Joystick)
// #define ENABLE_OPERATOR_SWITCH
// #define ENABLE_RELAYS

// Feedback
// #define ENABLE_STATUS
// #define ENABLE_FEEDBACK_3C
// #define ENABLE_FEEDBACK_BUZZER

// Motor
// #define ENABLE_SAKO_VFD
// #define MB_SAKO_VFD_SLAVE_ID 30
#define MB_SAKO_VFD_READ_INTERVAL 100

// #define ENABLE_DELTA_VFD
#define MB_DELTA_VFD_SLAVE_ID 22
#define MB_DELTA_VFD_READ_INTERVAL 100

// #define ENABLE_HY_VFD
#define MB_HY_VFD_SLAVE_ID 21
#define MB_HY_VFD_READ_INTERVAL 400

// Pressure Cylinder
// #define ENABLE_LOADCELL_0
#define LOADCELL_SLAVE_ID_0 20
#define LOADCELL_SLAVE_ID_1 21

// #define ENABLE_SOLENOID_0
#define PIN_SOLENOID_0 GPIO_PIN_CH1
#define PIN_SOLENOID_1 GPIO_PIN_CH2
// #define ENABLE_PRESS_CYLINDER
#define PRESS_CYLINDER_JOYSTICK_AUTO

// Temperature Control
// #define ENABLE_OMRON_E5
// #define NUM_OMRON_DEVICES 2
#define OMRON_E5_SLAVE_ID_BASE 10
#define OMRON_WRITE_DELAY_MS 500

// #define ENABLE_AMPERAGE_BUDGET_MANAGER

// built-in PID, not implemented yet
// #define ENABLE_PID

// Profiles
// #define ENABLE_PROFILE_TEMPERATURE
// #define PROFILE_TEMPERATURE_COUNT 4
#define PROFILE_TEMPERATURE_WARMUP
#define PROFILE_TEMPERATURE_FORCE_SEQUENTIAL_HEATING
// #define ENABLE_PROFILE_PRESSURE
// #define PROFILE_PRESSURE_COUNT 1
#define PRESSURE_PROFILE_MAX_TARGET_REGS 1

#define HEATUP_DEADBAND 5
#define MAX_OVERSHOOT 100

// #define ENABLE_PROFILE_SIGNAL_PLOT
// #define PROFILE_SIGNAL_PLOT_COUNT 4

// Protocols
#define ENABLE_MODBUS_TCP
#define ENABLE_RS485
#define ENABLE_RS485_DEVICES

// Internal
#define ENABLE_PROFILER // CPU & Memory Profiling
#define ENABLE_LOGGER
#define ENABLE_LOGGING_TARGET_PRINT
#define ENABLE_LOGGING_TARGET_WEBSOCKET
#define ENABLE_RUNTIME_STATE
// #define ENABLE_LOGGING_TARGET_FILE
// #define ENABLE_NETWORK_VALUE_TEST // test for NetworkValue
// #define ENABLE_NETWORK_VALUE_TEST_PB // test for NetworkValuePB

// Systems
// #define ENABLE_MB_SCRIPT // Experimental : Modbus Logic Engine
#define ENABLE_SETTINGS
// #define ENABLE_MODBUS_MIRROR
// #define ENABLE_EXTRUDER // not implemented yet
// #define ENABLE_PLUNGER  // arbor driven injection plunger, using SAKO VFD

////////////////////////////////////////////////////////////////
//
// Web Server Features
//
#define ENABLE_WIFI
#define ENABLE_WEBSERVER
#define ENABLE_WEBSERVER_FS_API
#define ENABLE_REST_SERVER
#define REST_SERVER_APP_ROUTES
#define ENABLE_LITTLEFS
#define ENABLE_WEBSOCKET
#define USE_BINARY_UPDATES // Enable binary updates for coils/registers
// #define ENABLE_WEBSOCKET_PB

#define ENABLE_WEBSERVER_WIFI_SETTINGS
#define HMI_NAME "cassandra"

///////////////////////////////////////////
// Test only
#define MB_SAKO_VFD_SLAVE_ID 30
#define ENABLE_SAKO_VFD
// #define ENABLE_EXTRUDER
#define ENABLE_DELTA_VFD
#define ENABLE_PLUNGER

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Configs (in this order!)
//
#include "config-extern.h"   // External config settings - PlatformIO config
#include "config-network.h"  // Network config settings
#include "config-user.h"     // User config settings - Overrides
#include "config-validate.h" // Validate config settings (cheap compiler checks)

#ifdef ENABLE_P4_MIN
#include "config-4-min.h"
#endif

#include "app-logger.h"

#endif

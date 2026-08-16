#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <stdint.h>
#include "config_adv.h"

////////////////////////////////////////////////////////////////////////////////
//
// Component IDs
//
typedef enum COMPONENT_KEY
{
  COMPONENT_KEY_RELAY_0 = 300,
  COMPONENT_KEY_RELAY_1 = 301,
  COMPONENT_KEY_RELAY_2 = 302,
  COMPONENT_KEY_RELAY_3 = 303,
  COMPONENT_KEY_RELAY_4 = 304,
  COMPONENT_KEY_RELAY_5 = 305,
  COMPONENT_KEY_PID_0 = 100,
  COMPONENT_KEY_PID_1 = 101,
  COMPONENT_KEY_PID_2 = 102,
  COMPONENT_KEY_ANALOG_0 = 350,
  COMPONENT_KEY_ANALOG_1 = 351,
  COMPONENT_KEY_ANALOG_2 = 352,
  COMPONENT_KEY_ANALOG_3POS_SWITCH_0 = 420,
  COMPONENT_KEY_ANALOG_3POS_SWITCH_1 = 421,
  COMPONENT_KEY_ANALOG_LEVEL_SWITCH_0 = 450,
  COMPONENT_KEY_ANALOG_LEVEL_SWITCH_1 = 451,
  COMPONENT_KEY_JOYSTICK_0 = 500,
  COMPONENT_KEY_STEPPER_0 = 601,
  COMPONENT_KEY_STEPPER_1 = 602,
  COMPONENT_KEY_PID = 620,
  COMPONENT_KEY_EXTRUDER = 650,
  COMPONENT_KEY_PLUNGER = 670,
  COMPONENT_KEY_FEEDBACK_0 = 701,
  COMPONENT_KEY_FEEDBACK_1 = 702,
  COMPONENT_KEY_SAKO_VFD = 750,
  COMPONENT_KEY_RS485_TESTER = 800,
  COMPONENT_KEY_RS485 = 801,
  COMPONENT_KEY_GPIO_MAP = 810,
  COMPONENT_KEY_REST_SERVER = 900,
  COMPONENT_KEY_PROFILE_START = 910,
  COMPONENT_KEY_END = 1000,
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
#define BRIDGE_DEBUG_REGISTER
#define BRIDGE_DEBUG_CALL_METHOD
// Serial Command Messaging Debugging Switches
#define DEBUG_MESSAGES_PARSE
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
// Serial Port Settings
/** @brief Serial port baud rate */
#define SERIAL_BAUD_RATE 115200
/** @brief Parse commands every 100ms */
#define SERIAL_COMMAND_PARSE_INTERVAL 100

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
// #define MB_ANALOG_0 15
// #define MB_ANALOG_1 7

#define POT_RAW_MAX_VALUE 4095   // Max raw ADC  value
#define POT_SCALED_MAX_VALUE 100 // Max scaled value (0-100)

//#define MB_GPIO_MB_MAP_7 7
//#define MB_GPIO_MB_MAP_15 15
////////////////////////////////////////////////////////////////
//
// Auxiliary Settings : Analog Level Switch
//
#ifdef PLATFORM_ESP32
//#define PIN_ANALOG_LEVEL_SWITCH_0 34    // <<< CHOOSE YOUR ADC PIN
#endif

/** @brief Analog Level Switch 0 */
#define ID_ANALOG_LEVEL_SWITCH_0 200
#define ALS_0_NUM_LEVELS 4           // Number of slots
#define ALS_0_ADC_STEP 800           // ADC counts per slot
#define ALS_0_ADC_OFFSET 200         // ADC value for the start of the first slot
#define ALS_0_MB_ADDR 60             // Modbus base address (Uses 60-65 for 4 slots)

////////////////////////////////////////////////////////////////
//
// Auxiliary Settings : LED Feedback
//
//#define PIN_LED_FEEDBACK_0          1  // <<< CHOOSE NEOPIXEL DATA PIN
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
#define PIN_JOYSTICK_UP 47     // UP direction pin
#define PIN_JOYSTICK_DOWN 48   // DOWN direction pin
#define PIN_JOYSTICK_LEFT 40   // LEFT direction pin
#define PIN_JOYSTICK_RIGHT 39  // RIGHT direction pin
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Network
//
#define NETWORK_CONFIG_FILENAME "/network.json"
// Static IP Addresses for STA mode (and STA part of AP_STA)
static IPAddress local_IP(192, 168, 1, 250);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 0, 0);
static IPAddress primaryDNS(8, 8, 8, 8);
static IPAddress secondaryDNS(8, 8, 4, 4);

#define WIFI_SSID "Livebox6-EBCD"
#define WIFI_PASSWORD "c4RK35h4PZNS"

// AP_STA Mode Configuration
// To enable AP_STA mode, define ENABLE_AP_STA.
// If ENABLE_AP_STA is defined, the device will act as both an Access Point
// and a WiFi client (Station) simultaneously.
// The AP will have its own SSID and IP configuration.
// The STA part will connect to the WiFi network defined by WIFI_SSID and WIFI_PASSWORD.

#define ENABLE_AP_STA // Uncomment to enable AP_STA mode

#ifdef ENABLE_AP_STA
#define AP_SSID "PolyMechAP"
#define AP_PASSWORD "poly1234" // Password must be at least 8 characters
static IPAddress ap_local_IP(192, 168, 4, 1);
static IPAddress ap_gateway(192, 168, 4, 1); // Typically the same as ap_local_IP for the AP
static IPAddress ap_subnet(255, 255, 255, 240); // Changed to .240 (/28 subnet)
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main switches : features
#define ENABLE_WIFI
#define ENABLE_WEBSERVER
#define ENABLE_MODBUS_TCP
// #define ENABLE_JOYSTICK (4P Joystick)

//#define ENABLE_PROFILE_TEMPERATURE
//#define PROFILE_TEMPERATURE_COUNT 1

// #define ENABLE_RS485
// #define ENABLE_RS485_DEVICES

// #define ENABLE_STATUS
// #define ENABLE_PID
// CPU & Memory Profiling
// #define ENABLE_PROFILER
// #define ENABLE_RELAYS
// Experimental : Modbus Scripting
#define ENABLE_MB_SCRIPT
// #define ENABLE_EXTRUDER
// #define ENABLE_PLUNGER

// #define ENABLE_SAKO_VFD
// #define MB_SAKO_VFD_SLAVE_ID 10
// #define MB_SAKO_VFD_READ_INTERVAL 200

#define ENABLE_OMRON_E5
#ifndef NUM_OMRON_DEVICES
#define NUM_OMRON_DEVICES 0
#define OMRON_E5_SLAVE_ID_BASE 1
#endif

#if (defined(ENABLE_WIFI) && defined(ENABLE_MODBUS_TCP))
#ifndef ENABLE_WEBSERVER
#endif
#endif

////////////////////////////////////////////////////////////////
//
// Web Server Features
//
#define ENABLE_REST_SERVER
#define ENABLE_LITTLEFS
#define ENABLE_WEBSOCKET
#define ENABLE_WEBSERVER_WIFI_SETTINGS


#endif

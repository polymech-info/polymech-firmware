#ifndef CONFIG_H
#define CONFIG_H

#include "enums.h"
#include "common/macros.h"
#include <Controllino.h>
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Core settings
//

#define LOOP_DELAY 100  // Our frame time, exluding delays in some places
#define BOOT_DELAY 500 // Wait at least this amount in ms after boot before doing anything

// Please consider to set this to false for production - especially with the full feature set since this is requiring extra
// time for the serial communication and will affect the overall framerate/performance
// #define DEBUG true

#define DEBUG_INTERVAL 1500
#define DEBUG_BAUD_RATE 19200 // Serial port speed

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Machine settings
//

#define USE_CONTROLLINO

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    VFD related
//

/////////////////////////////////////////////////////////////////////////
//
//  PID
// #define HEATER_PIN_RELAY  {10,11}
#define TEMP_PIN          {A0,A1}
#define FAN_PIN           2

#ifdef HEATER_PIN_RELAY
    #define HAS_TC
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Motor related
//
// Motor overload pin, if defined, this will be used to detect jamming
// On an OmronM2X, set the output via C21 (for output pin 11) to 03 for 
// 'Overload' (which is set in C41 )
// #define MOTOR_LOAD_PIN                      CONTROLLINO_A2

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Extrusion related
//

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    General switches

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Feedback

// #define HAS_STATUS
#define STATUS_POWER_PIN CONTROLLINO_R6
#define STATUS_PID_PIN CONTROLLINO_R7
#define STATUS_VFD_PIN CONTROLLINO_R8

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Plastic Hub Studio - internals : used by external controller setups
// Make sure it's matching
#define FIRMATA_BAUD_RATE 19200
#define FIRMWARE_VERSION 0.8

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Omron Pids - E5.x series - Modbus interface 
//

#define NB_OMRON_PIDS 3
//  #define OMRON_PID_SLAVE_START 1
#define OMRON_PID_UPDATE_INTERVAL 1000
#define OMRON_PID_WRITE_INTERVAL 500

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Omron - MX2 - VFD
#define OMRON_MX2_SLAVE_ID 1

#define OMRON_MX2_STATE_INTERVAL 1000
#define OMRON_MX2_DEBUG_INTERVAL 3000
#define OMRON_MX2_READ_INTERVAL 1000
#define OMRON_MX2_LOOP_INTERVAL 500
#define OMRON_MX2_SAME_REQUEST_INTERVAL 2000

#define MODBUS_QUEUE_MIN_FREE 5

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Modbus

#define MODBUS_RS485_BAUDRATE 9600
#define MODBUS_RS485_PORT SERIAL_8N1
#define MODBUS_RS485_TIMEOUT 2000

#define MODBUS_TCP_DEFAULT_REGISTER_VALUE 0

#define HAS_MODBUS_BRIDGE

static uint8_t MB_MAC[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
static uint8_t MB_IP[]{192, 168, 1, 222};
static uint8_t MB_GATEWAY[] = {192, 168, 1, 1};
static uint8_t MB_SUBNET[] = {255, 255, 255, 0};

////////////////////////////////////////////////////////
//
//  Controllino Relay Mapping - TCP - Modbus
//
#define NB_CONTROLLINO_RELAYS 3
#define CRELAY_START CONTROLLINO_R0

////////////////////////////////////////////////////////
//
//  Built-in relays for VFD activities
//
// if defined, activate this relay on VFD::start(), and deactivate on VFD::stop()
#define FEEDSCREW_RELAY CONTROLLINO_R9

// if defined, activate this relay on VFD::start(), and deactivate on VFD::stop()
#define COOLING_RELAY CONTROLLINO_R10

// if defined, activate this relay on VFD::start(), and deactivate on VFD::stop()
#define COOLING_RELAY2 CONTROLLINO_R11

////////////////////////////////////////////////////////////////
//
// Instrumentation
//
// #define MEARSURE_PERFORMANCE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    externals
//

// pull in internal constants
#include "constants.h"

// pull in internal configs
#include "config_adv.h"

// The user_config.h is initially added to the github repository but changes will be ignored via .gitignore. Please keep this file safe and possibly
// on a per tenant base stored. You can override parameters in this file by using #undef SOME_PARAMETER and then re-define again if needed, otherwise disable
// default features by using #undef FEATURE_OR_PARAMETER.
// This presents the possibilty to play with the code whilst staying in the loop with latest updates.
#include "user_config.h"

// At last we check all configs and spit compiler errors
#include "config_validator.h"

#endif

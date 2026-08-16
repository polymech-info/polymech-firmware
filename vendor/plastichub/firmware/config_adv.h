#ifndef CONFIG_ADV_H
#define CONFIG_ADV_H

//#define BRIDGE_DEBUG  // enable bridge debugging

////////////////////////////////////////////////////////////////////////////////
//
// Modbus

#define MODBUS_CMD_WAIT                                 200
#define MODBUS_READ_WAIT                                200

#define MODBUS_R_RETRY 2    // max read queries
#define MODBUS_W_RETRY 4    // max write queries

// @todo
// - advance config validation for the settings above
// - accumulate global modbus query timeout

////////////////////////////////////////////////////////////////////////////////
//  
// Power settings

// optional current sensor to validate primary power is there
// #define POWER_CSENSOR_PRIMARY                           CONTROLLINO_A15

// optional current sensor to validate primary power is there
#define POWER_CSENSOR_SECONDARY                         CONTROLLINO_A14

/////////////////////////////////////////////////////////////
//
// Motor load settings, this requires a current sensor or can be
// taken from the VFD's output. 

// the interval to read the current
#define MOTOR_LOAD_READ_INTERVAL                        100

// the current measured when the motor runs idle, min - max range
#define MOTOR_IDLE_LOAD_RANGE_MIN                       30
#define MOTOR_IDLE_LOAD_RANGE_MAX                       50

// the current measured when the motor is under load, min - max range
#define MOTOR_SHREDDING_LOAD_RANGE_MIN                  50
#define MOTOR_SHREDDING_LOAD_RANGE_MAX                  99

// the current measured when the motor is overloaded, min - max range
#define MOTOR_OVERLOAD_RANGE_MIN                        100
#define MOTOR_OVERLOAD_RANGE_MAX                        400

#define MOTOR_MIN_DT                                    2500

/////////////////////////////////////////////////////////////
//
//  Bridge related
#define STATE_RESPONSE_CODE                             1000
// #define BRIDGE_HAS_RESPONSE
/////////////////////////////////////////////////////////////
//
//  Error codes
//
#define E_MSG_OK            "Ok"
#define E_MSG_STUCK         "Shredder is stuck"

// common operating failures
#define E_OK                0                 //all good
#define E_STUCK             100               //Shredder stuck
#define E_NO_SUCH_PID       2001              //cant find PID
#define E_QUERY_BUFFER_END  99                // have no free query buffer slot

// power failures

#define E_POWER_PRIM_ON     145               // Power is on whilst it shouldn't be
#define E_POWER_PRIM_OFF    146               // Power is off whilst it should be

#define E_POWER_SEC_ON      147               // Power is on whilst it shouldn't be
#define E_POWER_SEC_OFF     148               // Power is off whilst it should be
#define E_POWER             150               // Nothing is online

#define E_VFD_OFFLINE       E_POWER_PRIM_OFF  // VFD should be online

// sensor failures
#define E_VFD_CURRENT       200               // VFD current abnormal: below or above average
#define E_OPERATING_SWITCH  220               // Operating switch invalid value

#define E_CARTRIDGE_OPEN    240               // Cartridge open sensor doesn't work 
#define E_SERVICE_OPEN      241               // Sensor open sensor doesn't work 

////////////////////////////
//
// sub system failures
//
#define E_USER_START        1000                // base offset for sub system errors

// vfd
#define E_VFD_RUN           300                 // Motor should spin but doesnt
#define E_VFD_LOSS          301                 // Motor should not spin but does
#define E_VFD_CUSTOM(A)     E_USER_START+A      // Custom VFD error
// motor
#define E_MOTOR_DT_IDLE     320                 // Motor runs idle longer as defined
#define E_MOTOR_DT_OVERLOAD 321                 // Motor runs overloaded longer as defined

#define E_BRIDGE_LOSS       400                 // bridge poll timeout

// bridge
#define E_BRIDGE_START      2000                // base offset for custom bridge errors
#define E_BRIDGE_CUSTOM(A)  E_USER_START+A      // Custom bridge error
#define E_BRIDGE_PARITY     E_BRIDGE_CUSTOM(1)  // @todo, parity check failure 
#define E_BRIDGE_CRC        E_BRIDGE_CUSTOM(2)  // @todo, crc  failure
#define E_BRIDGE_FLOOD      E_BRIDGE_CUSTOM(3)  // @todo, msg queue

// extrusion
#define E_EX_BASE           3000                // base offset extruder
#define E_EX_CUSTOM(A)      E_EX_BASE+A         // Custom bridge error

#endif
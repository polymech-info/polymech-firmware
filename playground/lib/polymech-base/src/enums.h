#ifndef ENUMS_H
#define ENUMS_H

typedef enum COMPONENT_KEY_BASE
{
  COMPONENT_KEY_NONE = 0,
  COMPONENT_KEY_APP = 1,
  COMPONENT_KEY_MB_SERIAL = 2,
  COMPONENT_KEY_MB_BRIDGE = 3,
  COMPONENT_KEY_MB_Manager = 800
} COMPONENT_KEY_BASE;

typedef enum COMPONENT_TYPE
{
  COMPONENT_TYPE_UNKOWN = 0,
  COMPONENT_TYPE_PID = 1,
  COMPONENT_TYPE_COM = 2,
  COMPONENT_TYPE_AUX = 3,
  COMPONENT_TYPE_IO = 4,
  COMPONENT_TYPE_MOTOR = 5,
  COMPONENT_TYPE_CAMERA = 6,
  COMPONENT_TYPE_SENSOR = 7,
  COMPONENT_TYPE_ACTUATOR = 8,
  COMPONENT_TYPE_OTHER = 9
} COMPONENT_TYPE;


typedef enum E_NetworkValueFeatureFlags 
{
    E_NVFF_NONE = 0,
    E_NVFF_LOGGING = 1,
    E_NVFF_MODBUS = 2,
    E_NVFF_NOTIFY = 3,
    E_NVFF_ALL = 1 << E_NVFF_LOGGING | 1 << E_NVFF_MODBUS | 1 << E_NVFF_NOTIFY
} E_NetworkValueFeatureFlags;

//////////////////////////////////////////////////////////////////
//
//  Error Codes

#define E_OK                0                 //all good
#define E_SKIP              10                //all good
#define E_QUEUED            20                //all good
#define E_INVALID_PARAMETER 30                //all good
#define E_NO_SUCH_PID       2001              //cant find PID
#define E_QUERY_BUFFER_END  99                //have no free query buffer slot

#define E_USER_START        1000              // base offset for sub system errors

// PID
#define E_PID_CUSTOM        2000                // Custom PID error
#define E_PID_TIMEOUT       E_PID_CUSTOM + 2    // Timeout
#define E_PID_OVERHEAT      E_PID_CUSTOM + 3    // Timeout

// WiFi/Network Errors
#define E_WIFI_CONNECTION_FAILED 5001
#define E_WEBSERVER_INIT_FAILED  5002
#define E_DEPENDENCY_NOT_MET     5003

#define E_ALLOCATION_FAILED 200
#define E_SERIAL_INIT_FAILED 201
#define E_NOT_INITIALIZED 202
#define E_NOT_IMPLEMENTED 204
#define E_MODBUS_INIT_FAILED 205 // Added Modbus RTU Init Error

#define E_WARNING            10
#define E_NOT_FOUND          20
#define E_FATAL              100

typedef enum OBJECT_RUN_FLAGS
{
    E_OF_NONE = 0,     /**< No run flag */
    E_OF_DEBUG = 1,    /**< Enable debug call, per frame */
    E_OF_INFO = 2,     /**< Enable info print during boot */
    E_OF_LOOP = 3,     /**< Enable loop invocation on the main loop */
    E_OF_DISABLED = 4, /**< Disable component entirely */
    E_OF_SETUP = 5,    /**< Enable setup invocation during boot */
    E_OF_MAIN = 6,     /**< reserved */
    E_OF_BRIDGE = 7    /**< Component registers methods on the Bridge */
} OBJECT_RUN_FLAGS;

typedef enum OBJECT_NET_CAPS
{
    E_NCAPS_NONE = 0,
    E_NCAPS_MODBUS = 1,
    E_NCAPS_SERIAL = 2,
} OBJECT_NET_CAPS;

typedef enum MB_REGISTER_MODE
{
    E_MB_REGISTER_MODE_NONE = 0,
    E_MB_REGISTER_MODE_READ = 1,
    E_MB_REGISTER_MODE_WRITE = 2,
    E_MB_REGISTER_MODE_READ_WRITE = 3,
} MB_REGISTER_MODE;

typedef enum E_CALLS
{
    /**< Call a global registered command . */
    EC_NONE = 0x00000000,
    /**< Call a global registered command . */
    EC_COMMAND = 0x00000001,
    /**< Call component method (See Bridge::registerMemberFunction & Component::serial_register)  */
    EC_METHOD = 0x00000002,
    /**< Function call type. */
    EC_FUNC = 0x00000004,
    /**< User-defined call type. */
    EC_USER = 0x00000008
} E_CALLS;

/**
 * @brief Enumeration representing different message flags.
 *
 * This enumeration defines the possible flags that can be associated with a message.
 * Each flag represents a different state or attribute of the message.
 */
typedef enum E_MessageFlags
{
    /**<Internal: no flags */
    E_MF_NONE = 0x00000000,
    /**<Internal: flag designating an unprocessed message */
    E_MF_NEW = 0x00000001,
    /**<Internal: Processing message flag */
    E_MF_PROCESSING = 0x00000002,
    /**<Internal: Processed message flag */
    E_MF_PROCESSED = 0x00000004,
    /**<Internal: Debug message during processing */
    E_MF_DEBUG = 0x00000008,
    /**<Sender: Instruct to send a receipt - Default:On */
    E_MF_RECEIPT = 0x0000010,
    /**<Sender: Instruct to return component state */
    E_MF_STATE = 0x00000020
} E_MessageFlags;

// Restore conflicting enum
typedef enum
{
    MB_FC_NONE = 0,                     //!< null operator 
    MB_FC_READ_COILS = 1,               //!< FCT=1 -> read coils or digital outputs 
    MB_FC_READ_DISCRETE_INPUT = 2,      //!< FCT=2 -> read digital inputs 
    MB_FC_READ_REGISTERS = 3,           //!< FCT=3 -> read registers or analog outputs 
    // MB_FC_READ_HOLDING_REGISTERS = 3,   //!< FCT=3 -> read registers or analog outputs
    MB_FC_READ_INPUT_REGISTER = 4,      //!< FCT=4 -> read analog inputs 
    MB_FC_WRITE_COIL = 5,               //!< FCT=5 -> write single coil or output 
    MB_FC_WRITE_REGISTER = 6,           //!< FCT=6 -> write single register 
    MB_FC_WRITE_MULTIPLE_COILS = 15,    //!< FCT=15 -> write multiple coils or outputs 
    MB_FC_WRITE_MULTIPLE_REGISTERS = 16 //!< FCT=16 -> write multiple registers 
} MB_FC;

typedef enum MODBUS_ERRORS
{
    MODBUS_ERROR_NONE = 0,                                    /**< No error */
    MODBUS_ERROR_ILLEGAL_FUNCTION = 1,                        /**< Illegal function error */
    MODBUS_ERROR_ILLEGAL_DATA_ADDRESS = 2,                    /**< Illegal data address error */
    MODBUS_ERROR_ILLEGAL_DATA_VALUE = 3,                      /**< Illegal data value error */
    MODBUS_ERROR_SLAVE_DEVICE_FAILURE = 4,                    /**< Slave device failure error */
    MODBUS_ERROR_ACKNOWLEDGE = 5,                             /**< Acknowledge error */
    MODBUS_ERROR_SLAVE_DEVICE_BUSY = 6,                       /**< Slave device busy error */
    MODBUS_ERROR_MEMORY_PARITY = 8,                           /**< Memory parity error */
    MODBUS_ERROR_GATEWAY_PATH_UNAVAILABLE = 10,               /**< Gateway path unavailable error */
    MODBUS_ERROR_GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = 11 /**< Gateway target device failed to respond error */
} MODBUS_ERRORS;

/**
 * @brief Defines the type of Modbus object.
 */
enum E_ModbusType {
    MB_TYPE_UNKNOWN = 0,
    MB_TYPE_COIL = 1,
    MB_TYPE_DISCRETE_INPUT = 2,
    MB_TYPE_HOLDING_REGISTER = 3,
    MB_TYPE_INPUT_REGISTER = 4
};

/**
 * @brief Defines the access mode for a Modbus address range.
 */
enum E_ModbusAccess {
    MB_ACCESS_NONE = 0,
    MB_ACCESS_READ_ONLY = 1,
    MB_ACCESS_WRITE_ONLY = 2,
    MB_ACCESS_READ_WRITE = 3
};

#endif

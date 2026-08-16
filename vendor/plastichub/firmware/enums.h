#ifndef ENUMS_H
#define ENUMS_H

enum MB_FC
{
  MB_FC_NONE = 0,                     /*!< null operator */
  MB_FC_READ_COILS = 1,               /*!< FCT=1 -> read coils or digital outputs */
  MB_FC_READ_DISCRETE_INPUT = 2,      /*!< FCT=2 -> read digital inputs */
  MB_FC_READ_REGISTERS = 3,           /*!< FCT=3 -> read registers or analog outputs */
  MB_FC_READ_INPUT_REGISTER = 4,      /*!< FCT=4 -> read analog inputs */
  MB_FC_WRITE_COIL = 5,               /*!< FCT=5 -> write single coil or output */
  MB_FC_WRITE_REGISTER = 6,           /*!< FCT=6 -> write single register */
  MB_FC_WRITE_MULTIPLE_COILS = 15,    /*!< FCT=15 -> write multiple coils or outputs */
  MB_FC_WRITE_MULTIPLE_REGISTERS = 16 /*!< FCT=16 -> write multiple registers */
};

enum BOARD
{
  MEGA
};

enum POS3_DIRECTION
{
  UP = 1,
  MIDDLE = 0,
  DOWN = 2,
  INVALID = -1
};

enum MODE
{
};

enum AR_MODE
{
  NORMAL = 1,    // Shredding
  EXTRUSION = 2, // Extrusion (no reverse)
  NONE = 3,      // Disable any jamming detection
  REMOTE = 4     // User land, ie: Firmata, I2C or PlasticHub-Studio
};

enum ADDON_FLAGS
{
  DEBUG = 1,
  INFO = 2,
  LOOP = 3,
  DISABLED = 4,
  SETUP = 5,
  MAIN = 6,
  STATE = 7
};

enum ADDONS
{
  MOTOR_IDLE = 1,
  MOTOR_LOAD = 2,
  MOTOR_TEMPERATURE = 3,
  VFD_CONTROL = 4,
  DIRECTION_SWITCH = 5,
  ENCLOSURE_SENSOR = 6,
  MOTOR_SPEED = 7,
  POWER = 11,
  OPERATION_MODE_SWITCH = 20,
  SERIAL_BRIDGE = 23,
  APP = 25,
  MODBUS_BRIDGE = 26,
  RMOTOR_CONTROL = 30,
  OMRON_PID = 31,
  OMRON_VFD = 32,
  PH_PID = 33,
  C_RELAY = 34,
  LAST = 64
};

enum POWER_CIRCUIT
{
  POWER_PRIMARY = 0,
  POWER_SECONDARY = 1
};

enum OPERATION_MODE
{
  OP_NONE = 0,
  OP_NORMAL = 1,
  OP_DEBUG = 2,
  OP_SERVICE = 3,
  OP_TEST
};
enum ERROR
{
  ERROR_OK = 0,
  ERROR_WARNING = 1,
  ERROR_FATAL = 2
};

enum MBB_STATE
{
  WAITING = 0,
  QUERY = 1,
  RESPONSE = 2,
  IDLE = 3
};

// Modbus query state
enum QUERY_STATE
{
  QUEUED = 1,
  PROCESSING = 2,
  SENT = 3,
  RESPONDED = 4,
  DONE = 5
};

#define ku8MBReadCoils 0x01          ///< Modbus function 0x01 Read Coils
#define ku8MBReadDiscreteInputs 0x02 ///< Modbus function 0x02 Read Discrete Inputs
#define ku8MBWriteSingleCoil 0x05    ///< Modbus function 0x05 Write Single Coil
#define ku8MBWriteMultipleCoils 0x0F ///< Modbus function 0x0F Write Multiple Coils

// Modbus function codes for 16 bit access
#define ku8MBReadHoldingRegisters 0x03       ///< Modbus function 0x03 Read Holding Registers
#define ku8MBReadInputRegisters 0x04         ///< Modbus function 0x04 Read Input Registers
#define ku8MBWriteSingleRegister 0x06        ///< Modbus function 0x06 Write Single Register
#define ku8MBWriteMultipleRegisters 0x10     ///< Modbus function 0x10 Write Multiple Registers
#define ku8MBMaskWriteRegister 0x16          ///< Modbus function 0x16 Mask Write Register
#define ku8MBReadWriteMultipleRegisters 0x17 ///< Modbus function 0x17 Read Write Multiple Registers
#define ku8MBLinkTestOmronMX2Only 0x08       ///< Modbus function 0x08 Test

// Modbus protocol errors
#define ERR_MODBUS_0x01 -41   // Modbus 0x01 protocol illegal function exception
#define ERR_MODBUS_0x02 -42   // Modbus 0x02 protocol illegal data address exception
#define ERR_MODBUS_0x03 -43   // Modbus 0x03 protocol illegal data value exception
#define ERR_MODBUS_0x04 -44   // Modbus 0x4 protocol slave device failure exception
#define ERR_MODBUS_0xe0 -45   // Modbus 0xe0 Master invalid response slave ID exception
#define ERR_MODBUS_0xe1 -46   // Modbus 0xe1 Master invalid response function exception
#define ERR_MODBUS_0xe2 -47   // Modbus 0xe2 Master response timed out exception
#define ERR_MODBUS_0xe3 -48   // Modbus 0xe3 Master invalid response CRC exception
#define ERR_MODBUS_UNKNOW -56 // Modbus unknown error (protocol failure)
#define ERR_MODBUS_STATE -57  // Forbidden ( invalid ) state of the inverter
#define ERR_MODBUS_BLOCK -58  // Attempt to turn on the VT with the inverter locked

// Omron Mx2 specific
#define ERR_MODBUS_MX2_0x01 -49 // Omron mx2 Exception code 0x01 The specified function is not supported
#define ERR_MODBUS_MX2_0x02 -50 // Omron mx2 Exception code 0x02 The specified function was not found.
#define ERR_MODBUS_MX2_0x03 -52 // Omron mx2 Exception code 0x03 Unacceptable data format
#define ERR_MODBUS_MX2_0x05 -52 // Omron mx2 communication error over Modbus (function communication check 0x08 Omron mx2)
#define ERR_MODBUS_MX2_0x21 -53 // Omron mx2 Exception code 0x21 Data written to the storage register is outside the inverter
#define ERR_MODBUS_MX2_0x22 -54 // Omron mx2 Exception code 0x22 These functions are not available for the inverter
#define ERR_MODBUS_MX2_0x23 -55 // Omron mx2 Exception code 0x23 The register (bit) into which the value should be written is read-only

//////////////////////////////////////////////////////////////////
//
//  PH - Firmware specific register mapping - TCP & RS485
//


///////////////////////////////////////////////////////////////////////////////////////////////////
// Omron related address offset : PID & MX2
#define MB_REGISTER_OFFSET 10

//  RS485 read register - Omron-E5 - PH firmware specific (void OmronPID::updateTCP())
#define MB_R_PID_1_PV MB_REGISTER_OFFSET + 1
#define MB_R_PID_1_SP MB_REGISTER_OFFSET + 2
#define MB_R_PID_2_PV MB_REGISTER_OFFSET + 3
#define MB_R_PID_2_SP MB_REGISTER_OFFSET + 4
#define MB_R_PID_3_PV MB_REGISTER_OFFSET + 5
#define MB_R_PID_3_SP MB_REGISTER_OFFSET + 6

//  TCP Inbound register - Omron-E5 - PH firmware specific (void OmronPID::fromTCP())
#define MB_W_PID_1_SP MB_REGISTER_OFFSET + 7
#define MB_W_PID_2_SP MB_REGISTER_OFFSET + 8
#define MB_W_PID_3_SP MB_REGISTER_OFFSET + 9

///////////////////////////////////////////////////////////////////////////////////////////////////
// Built in related address offset : TemperatureController
#define MB_REGISTER_OFFSET_TC 20

//  RS485 read register - TemperatureController - PH firmware specific (void TemperatureController::updateTCP())
#define MB_R_PID_1_PV MB_REGISTER_OFFSET_TC + 1
#define MB_R_PID_1_SP MB_REGISTER_OFFSET_TC + 2
#define MB_R_PID_2_PV MB_REGISTER_OFFSET_TC + 3
#define MB_R_PID_2_SP MB_REGISTER_OFFSET_TC + 4
#define MB_R_PID_3_PV MB_REGISTER_OFFSET_TC + 5
#define MB_R_PID_3_SP MB_REGISTER_OFFSET_TC + 6


///////////////////////////////////////////////////////////////////////////////////////////////////
// Built in related address offset : Controllino relay mappings (coils)

#define CONTROLLINO_RELAY_START CONTROLLINO_R0

#define MB_REGISTER_OFFSET_RELAYS_READ 40

#define MB_R_RELAY_1 MB_REGISTER_OFFSET_RELAYS_READ + 1
#define MB_R_RELAY_2 MB_REGISTER_OFFSET_RELAYS_READ + 2
#define MB_R_RELAY_3 MB_REGISTER_OFFSET_RELAYS_READ + 3
#define MB_R_RELAY_4 MB_REGISTER_OFFSET_RELAYS_READ + 4
#define MB_R_RELAY_5 MB_REGISTER_OFFSET_RELAYS_READ + 5
#define MB_R_RELAY_6 MB_REGISTER_OFFSET_RELAYS_READ + 6
#define MB_R_RELAY_7 MB_REGISTER_OFFSET_RELAYS_READ + 7
#define MB_R_RELAY_8 MB_REGISTER_OFFSET_RELAYS_READ + 8

#define MB_REGISTER_OFFSET_RELAYS_WRITE 50

#define MB_R_RELAY_1 MB_REGISTER_OFFSET_RELAYS_WRITE + 1
#define MB_R_RELAY_2 MB_REGISTER_OFFSET_RELAYS_WRITE + 2
#define MB_R_RELAY_3 MB_REGISTER_OFFSET_RELAYS_WRITE + 3
#define MB_R_RELAY_4 MB_REGISTER_OFFSET_RELAYS_WRITE + 4
#define MB_R_RELAY_5 MB_REGISTER_OFFSET_RELAYS_WRITE + 5
#define MB_R_RELAY_6 MB_REGISTER_OFFSET_RELAYS_WRITE + 6
#define MB_R_RELAY_7 MB_REGISTER_OFFSET_RELAYS_WRITE + 7
#define MB_R_RELAY_8 MB_REGISTER_OFFSET_RELAYS_WRITE + 8


// Omron Plastic - Hub Register Mapping (Read mirror)
#define MB_R_FREQ_TARGET 1
#define MB_R_VFD_STATUS 3
#define MB_R_VFD_STATE 4

// Omron - MX2 related PlasticHub Coil Mapping (Write)

#define MB_W_VFD_RUN      5
#define MB_W_FREQ_TARGET  6
#define MB_W_DIRECTION    7

#define MB_W_TC_STATE     8



#define MB_QUERY_TYPE_STATUS_POLL 10
#define MB_QUERY_TYPE_CMD 100

// Omron - MX2 states - vendor enum
#define OMRON_STATE_ACCELERATING 4
#define OMRON_STATE_DECELERATING 2
#define OMRON_STATE_RUNNING 3
#define OMRON_STATE_STOPPED 1
#define OMRON_STATE_ERROR 8

// Omron - MX2 states - PH firmware
#define OMRON_STATUS_STOPPED 2
#define OMRON_STATUS_RUNNING 0

#endif

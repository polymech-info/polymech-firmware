#ifndef CONFIG_MODBUS_H
#define CONFIG_MODBUS_H

//////////////////////////////////////////////////////////////////
//
// Commons
//
#define MB_PRINT_ERRORS false
#define MAX_MODBUS_COMPONENTS 256
#define MB_RTU_PROCESS_INTERVAL_MS 20

//////////////////////////////////////////////////////////////////
//
// SYNC Settings
//
#define TEMPERATURE_PROFILE_SYNC_INTERVAL_MS 500

//////////////////////////////////////////////////////////////////
//
// Modbus RTU
//
// Serial configuration
#define MODBUS_SERIAL_MODE SERIAL_8N1
#define MODBUS_SERIAL_TIMEOUT 1500  // 3 seconds timeout for Modbus client

#define MAX_MODBUS_SLAVES 16
#define MAX_ADDRESSES_PER_SLAVE 64
#define MAX_PENDING_OPERATIONS 64
#define MAX_HIGH_PRIORITY_OPERATIONS 32

#define MAX_TCP_MAPPINGS 256
#define MAX_RTU_MAPPINGS 256

// Delay constants - these should be avoided when possible
#define DELAY_SERIAL_INIT 500       // ms to wait for serial initialization
#define DELAY_CLIENT_INIT 1000      // ms to wait for client initialization
#define DELAY_RESET_PAUSE 200       // ms to pause during reset

#define RS485_LOOP_INTERVAL_MS 10 // Interval in milliseconds for RS485 loop processing

#define MAX_READ_BLOCKS 4

#define PRIORITY_HIGHEST 100
#define PRIORITY_HIGH 80
#define PRIORITY_MEDIUM 60
#define PRIORITY_LOW 40
#define PRIORITY_LOWEST 20


#define MAX_MODBUS_DEVICES 16

// Maximum registers per device
#define MAX_INPUT_REGISTERS 10
#define MAX_OUTPUT_REGISTERS 10

// Modbus coil values
#define COIL_ON 0xFF00  // Proper Modbus value for coil ON
#define COIL_OFF 0x0000 // Proper Modbus value for coil OFF

// Operation timing
#define OPERATION_TIMEOUT 5500

// Define pins for RS485 communication
// Define the actual GPIO numbers used for RX and TX on your ESP32 board for Serial1
#define TXD1 17
#define RXD1 18

#define RXD1_PIN RXD1
#define TXD1_PIN TXD1

// Define the HardwareSerial instance to use for RS485
#define RS485_SERIAL_PORT Serial1 // Use Serial1

// Try with an explicit DE/RE pin - GPIO_NUM_4 is often used for this
#define REDEPIN_MODBUS GPIO_NUM_4

// Baudrate for RS485/Modbus communication
#define MB_RTU_BAUDRATE 9600

// Increase queue size to avoid queue full errors
#define MODBUS_QUEUE_SIZE 256

// Define maximum array sizes for fixed arrays if not already defined
#ifndef MAX_ADDRESSES_PER_SLAVE
#define MAX_ADDRESSES_PER_SLAVE 64
#endif

// Add constants from ModbusRTU.h
#define MAX_PENDING_OPERATIONS 64
#define MAX_HIGH_PRIORITY_OPERATIONS 32
#define MAX_RETRIES 2

// Define flags for ModbusOperation status
#define OP_USED_BIT 0          // Bit for Used flag
#define OP_HIGH_PRIORITY_BIT 1 // Bit for High Priority flag
#define OP_IN_PROGRESS_BIT 2   // Bit for In Progress flag
#define OP_BROADCAST_BIT 3     // Bit for Broadcast flag
#define OP_SYNCHRONIZED_BIT 4  // Bit for Synchronized flag

// Define flags for ModbusValueEntry status - Replaced with bit positions
// #define VALUE_FLAG_USED         0x01
// #define VALUE_FLAG_SYNCHRONIZED 0x02
#define VALUE_USED_BIT 0
#define VALUE_SYNCHRONIZED_BIT 1

// Define flags for ModbusReadBlock status
#define BLOCK_USED_BIT 0


//////////////////////////////////////////////////////////////////
//
// Modbus TCP Port
//
#define MODBUS_PORT 502             // Standard Modbus TCP port

//////////////////////////////////////////////////////////////////
//
// System Level Registers - TCP

#define MAX_REGISTERS 125

#define MB_ADDR_SYSTEM_ERROR        0   // R - System-wide error code
#define MB_ADDR_ECHO_TEST           8   // R/W - Echo test
#define MB_ADDR_APP_STATE           9   // R - Application state (see PHApp::APP_STATE)
#define MB_ADDR_RESET_CONTROLLER    100 // W - Write any value to reset
#define MB_ADDR_SYSTEM_END          10

// Auxiliary Registers
#define MB_ADDR_AUX_0 MB_ADDR_SYSTEM_END
#define MB_ADDR_AUX_1 MB_ADDR_AUX_0 + 4
#define MB_ADDR_AUX_2 MB_ADDR_AUX_0 + 8
#define MB_ADDR_AUX_3 MB_ADDR_AUX_0 + 12
#define MB_ADDR_AUX_4 MB_ADDR_AUX_0 + 16
#define MB_ADDR_AUX_5 MB_ADDR_AUX_0 + 20
#define MB_ADDR_AUX_6 MB_ADDR_AUX_0 + 24
#define MB_ADDR_AUX_7 MB_ADDR_AUX_0 + 28
#define MB_ADDR_AUX_8 MB_ADDR_AUX_0 + 32
#define MB_ADDR_AUX_9 MB_ADDR_AUX_0 + 36
 
#define MB_COIL_RELAY_0             51  // R/W - Address for Relay with ID COMPONENT_KEY_MB_RELAY_0
#define MB_COIL_RELAY_1             52  // R/W - Address for Relay with ID COMPONENT_KEY_MB_RELAY_1
#define MB_COIL_RELAY_2             53  // R/W - Address for Relay with ID COMPONENT_KEY_MB_RELAY_2

#define MB_IREG_ANALOG_0            400 // R - Address for Analog Input 0
#define MB_IREG_ANALOG_1            401 // R - Address for Analog Input 1
#define MB_IREG_ANALOG_2            402 // R - Address for Analog Input 2

#define MB_IREG_3POS_SWITCH_0       501 // R - Address for 3-Pos Switch 0
#define MB_IREG_3POS_SWITCH_1       502 // R - Address for 3-Pos Switch 1

#define MB_ADDR_AUX_END MB_ADDR_AUX_0 + 40


#define MB_HREG_PID_0_PV            100 // R   - PID 0 Process Value
#define MB_HREG_PID_0_SP            101 // R/W - PID 0 Setpoint
#define MB_HREG_PID_0_STATE         102 // R   - PID 0 State

#define MB_HREG_PID_1_PV            103 // R   - PID 1 Process Value
#define MB_HREG_PID_1_SP            104 // R/W - PID 1 Setpoint
#define MB_HREG_PID_1_STATE         105 // R   - PID 1 State


// Example: Battle Test Registers
#define MB_HREG_BATTLE_COUNTER      20 // R/W - Counter for battle test
#define MB_HREG_BATTLE_TIMESTAMP    21 // R   - Timestamp for battle test

// Example: Modbus Client Tracking Registers
#define MB_HREG_CLIENT_COUNT        22 // R - Current number of connected clients
#define MB_HREG_CLIENT_MAX          23 // R - Maximum number of clients seen
#define MB_HREG_CLIENT_TOTAL        24 // R - Total client connections since start

// Monitoring & Feedback Addresses (placeholder addresses, verify usage)
// These were originally calculated relative to other offsets in enums.h
// Define them explicitly here for now.
#define MB_MONITORING_STATUS_FEEDBACK_0 701 // R? R/W? - Address for StatusLight 0
#define MB_MONITORING_STATUS_FEEDBACK_1 702 // R? R/W? - Address for StatusLight 1

////////////////////////////////////////////////////////////////////////////////
//
// Omron Pids - E5.x series - Modbus interface
//
#define OMRON_MB_TCP_OFFSET 1000

////////////////////////////////////////////////////////////////////////////////
//
// Built-in PIDs
//
#define MB_HREG_PID_2_BASE_ADDRESS 6100
#define PID_2_REGISTER_COUNT 12

#endif // CONFIG_MODBUS_H 
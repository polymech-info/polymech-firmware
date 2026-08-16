#pragma once

/*
 * INSTRUCTIONS FOR LLM MANUAL PROCESSING:
 * 
 * This file contains Delta MS300 inverter register definitions extracted from manual JPGs.
 * When processing manual pages, follow these guidelines:
 * 
 * 1. APPEND ONLY - Never remove existing register groups, always add new ones
 * 2. PRESERVE ALL - Keep all existing macros, enums, and comments intact
 * 3. REGISTER FORMAT:
 *    - Use hex addresses EXACTLY as shown: 0x0001 (NOT 0x2001H)
 *    - Include parameter group notation: GG:nn (e.g., Parameter 04-10 = 040AH)
 *    - Add detailed bit definitions as comments
 *    - Include function descriptions from manual
 * 4. NAMING CONVENTION:
 *    - Registers: E_DELTA_MS300_[DESCRIPTION] = 0x[ADDR]
 *    - Values: E_DELTA_MS300_[CATEGORY]_[VALUE]
 *    - Macros: DELTA_MS300_[ACTION]_[FIELD]
 * 5. GROUP ORGANIZATION:
 *    - Group related registers in same enum
 *    - Add section comments for each register group
 *    - Maintain numerical address order
 * 6. BIT MAPPING:
 *    - Document all bit ranges with comments
 *    - Include reserved bits notation
 *    - Add value descriptions for each bit combination
 * 7. LAYOUT STRUCTURE:
 *    - First add all register addresses to main E_DELTA_MS300_REGISTERS enum
 *    - Then create individual register address defines and enum classes
 *    - MANDATORY FORMAT for each parameter:
 *      
 *      // Parameter 00-02: Parameter Reset
 *      #define E_DELTA_MS300_PARAMETER_RESET 0x0002
 *      enum class E_DELTA_MS300_RESET_SETTINGS : uint8_t
 *      {
 *          E_DELTA_MS300_RESET_NO_FUNCTION = 0,
 *          E_DELTA_MS300_RESET_WRITE_PROTECT = 1,
 *          // etc...
 *      };
 *    
 *    - ALWAYS include the address #define before each enum class
 *    - This provides direct register access and possible values separately
 * 8. DATA TYPES: Use appropriate uint8_t, uint16_t based on value ranges
 * 9. CONTEXT LIMITATION HANDLING:
 *    - This file will be passed with each new manual page
 *    - Always check for existing registers before adding duplicates
 *    - Preserve the complete structure when adding new content
 *    - Use search to verify if register already exists
 * 10. SKIP pages not related to Modbus-accessible parameters
 */

// Delta MS300 Parameter Group Communication Access Addresses (from manual):
// GG = parameter group, nn = parameter number
// Example: Parameter 04-10 = 040AH

// Convenience macros for Delta MS300 register operations
#define DELTA_MS300_SET_FUNCTION_BITS(reg, func) ((reg) = ((reg) & ~0x03) | ((func) & 0x03))
#define DELTA_MS300_SET_DIRECTION_BITS(reg, dir) ((reg) = ((reg) & ~0x30) | (((dir) & 0x03) << 4))
#define DELTA_MS300_SET_ACCEL_BITS(reg, accel) ((reg) = ((reg) & ~0xC0) | (((accel) & 0x03) << 6))
#define DELTA_MS300_SET_SPEED_BITS(reg, speed) ((reg) = ((reg) & ~0x0F00) | (((speed) & 0x0F) << 8))
#define DELTA_MS300_SET_ENABLE_BIT(reg, enable) ((reg) = ((reg) & ~0x1000) | (((enable) & 0x01) << 12))
#define DELTA_MS300_SET_OPERATION_BITS(reg, op) ((reg) = ((reg) & ~0x6000) | (((op) & 0x03) << 13))

#define DELTA_MS300_GET_FUNCTION_BITS(reg) ((reg) & 0x03)
#define DELTA_MS300_GET_DIRECTION_BITS(reg) (((reg) & 0x30) >> 4)
#define DELTA_MS300_GET_ACCEL_BITS(reg) (((reg) & 0xC0) >> 6)
#define DELTA_MS300_GET_SPEED_BITS(reg) (((reg) & 0x0F00) >> 8)
#define DELTA_MS300_GET_ENABLE_BIT(reg) (((reg) & 0x1000) >> 12)
#define DELTA_MS300_GET_OPERATION_BITS(reg) (((reg) & 0x6000) >> 13)

// Build complete command register
#define DELTA_MS300_BUILD_CMD(func, dir, accel, speed, enable, op) \
    (((func) & 0x03) | (((dir) & 0x03) << 4) | (((accel) & 0x03) << 6) | \
     (((speed) & 0x0F) << 8) | (((enable) & 0x01) << 12) | (((op) & 0x03) << 13))

enum class E_DELTA_MS300_REGISTERS : int 
{
    // 00: Drive Parameters
    E_DELTA_MS300_IDENTITY_CODE = 0x0000,           // 00-00: Identity Code of the AC Motor Drive. Read Only.
    E_DELTA_MS300_RATED_CURRENT = 0x0001,           // 00-01: Display AC Motor Drive Rated Current. Read Only. Factory setting is for normal duty. Set Pr. 00-16 to 1 for heavy duty current.
    E_DELTA_MS300_PARAMETER_RESET = 0x0002,         // 00-02: Parameter Reset. Write Only. Factory Setting: 0. See E_DELTA_MS300_RESET_SETTINGS.
    E_DELTA_MS300_STARTUP_DISPLAY_SELECTION = 0x0003,// 00-03: Start-up Display Selection. Factory setting: 0. See E_DELTA_MS300_STARTUP_DISPLAY_SETTINGS.
    E_DELTA_MS300_MULTIFUNCTION_DISPLAY_CONTENT = 0x0004,// 00-04: Content of Multi-function Display (User Defined). Factory setting: 3. See E_DELTA_MS300_MULTIFUNCTION_DISPLAY_SETTINGS.
    E_DELTA_MS300_COEFFICIENT_GAIN_ACTUAL_OUTPUT_FREQ = 0x0005, // 00-05: Coefficient Gain in Actual Output Frequency. Settings 0.00~160.00. Factory Setting: 1.00. Set Pr. 00-04=31 to use.
    E_DELTA_MS300_SOFTWARE_VERSION = 0x0006,        // 00-06: Software Version. Read Only.
    E_DELTA_MS300_PARAMETER_PROTECTION_PASSWORD_INPUT = 0x0007, // 00-07: Parameter Protection Password Input. Settings 0~65535. Factory Setting: 0.
    E_DELTA_MS300_PARAMETER_PROTECTION_PASSWORD_SETTING = 0x0008, // 00-08: Parameter Protection Password Setting. Settings 0~65535. Factory Setting: 0. See E_DELTA_MS300_PASSWORD_PROTECTION_SETTINGS.
    E_DELTA_MS300_CONTROL_MODE = 0x000A,                          // 00-10: Control Mode. Determines the control mode of the AC motor drive. Factory Setting: 0. See E_DELTA_MS300_CONTROL_MODE_SETTINGS.
    E_DELTA_MS300_CONTROL_OF_SPEED_MODE = 0x000B,                 // 00-11: Control of Speed Mode. Determines the control mode when Pr. 00-10=0. Factory Setting: 0. See E_DELTA_MS300_CONTROL_OF_SPEED_MODE_SETTINGS.
    E_DELTA_MS300_LOAD_SELECTION = 0x0010,                        // 00-16: Load Selection. Selects Normal or Heavy duty operation. Factory Setting: 1. See E_DELTA_MS300_LOAD_SELECTION_SETTINGS.
    E_DELTA_MS300_CARRIER_FREQUENCY = 0x0011,                     // 00-17: Carrier Frequency. Determines the PWM carrier frequency. Settings 2~15 (kHz). Factory Setting: 4. Dependent on load selection (Pr. 00-16).
    E_DELTA_MS300_PLC_COMMAND_MASK = 0x0013,                      // 00-19: PLC Command Mask. Read Only. Determines if frequency/control command is locked by PLC.
                                                                  //          bit 0: Control command forced by PLC control
                                                                  //          bit 1: Frequency command forced by PLC control
    E_DELTA_MS300_MASTER_FREQ_CMD_SOURCE = 0x0014,                // 00-20: Source of the Master Frequency Command (AUTO). Factory Setting: 0. See E_DELTA_MS300_MASTER_FREQ_CMD_SOURCE_SETTINGS.
    E_DELTA_MS300_OPERATION_CMD_SOURCE = 0x0015,                  // 00-21: Source of the Operation Command (AUTO). Factory Setting: 0. See E_DELTA_MS300_OPERATION_CMD_SOURCE_SETTINGS.
    E_DELTA_MS300_STOP_METHOD = 0x0016,                           // 00-22: Stop Method. Factory Setting: 0. See E_DELTA_MS300_STOP_METHOD_SETTINGS.

    // 2000H - AC motor drive parameters Command write only
    // GG = parameter group, nn = parameter number; for example, the address of Pr.04-10 is 040AH
    E_DELTA_MS300_AC_MOTOR_PARAMS = 0x2000,         // Parameter group access
    
    // Bit definitions for 2000H register
    // bit 1-0: 00B: No function, 01B: Stop, 10B: Run, 11B: JOG + RUN
    // bit 3-2: Reserved  
    // bit 5-4: 00B: No function, 01B: FWD, 10B: REV, 11B: Change direction
    // bit 7-6: 00B: 1st accel./decel., 01B: 2nd accel./decel., 10B: 3rd accel./decel., 11B: 4th accel./decel.
    // bit 11-8: 0000B: Master speed, 0001B: 1st step speed frequency, 0010B: 2nd step speed frequency,
    //           0011B: 3rd step speed frequency, 0100B: 4th step speed frequency, 0101B: 5th step speed frequency,
    //           0110B: 6th step speed frequency, 0111B: 7th step speed frequency, 1000B: 8th step speed frequency,
    //           1001B: 9th step speed frequency, 1010B: 10th step speed frequency, 1011B: 11th step speed frequency,
    //           1100B: 12th step speed frequency, 1101B: 13th step speed frequency, 1110B: 14th step speed frequency,
    //           1111B: 15th step speed frequency
    // bit 12: 1: Enable bit 06-11 function
    // bit 14-13: 00B: No function, 01B: Operated by the digital keypad, 10B: Operated by Pr.00-21 setting, 11B: Change the operation source
    // bit 15: Reserved
};

enum class E_DELTA_MS300_COMMAND_BITS : int 
{
    // Function control bits (bit 1-0)
    E_DELTA_MS300_NO_FUNCTION = 0,
    E_DELTA_MS300_STOP = 1,
    E_DELTA_MS300_RUN = 2,
    E_DELTA_MS300_JOG_RUN = 3,
    
    // Direction control bits (bit 5-4)  
    E_DELTA_MS300_DIR_NO_FUNCTION = 0,
    E_DELTA_MS300_DIR_FWD = 1,
    E_DELTA_MS300_DIR_REV = 2,
    E_DELTA_MS300_DIR_CHANGE = 3,
    
    // Acceleration/Deceleration control bits (bit 7-6)
    E_DELTA_MS300_ACCEL_1ST = 0,
    E_DELTA_MS300_ACCEL_2ND = 1,
    E_DELTA_MS300_ACCEL_3RD = 2,
    E_DELTA_MS300_ACCEL_4TH = 3
};

// Parameter 00-02: Parameter Reset
#define E_DELTA_MS300_PARAMETER_RESET 0x0002
enum class E_DELTA_MS300_RESET_SETTINGS : uint8_t
{
    E_DELTA_MS300_RESET_NO_FUNCTION = 0,                        // No Function
    E_DELTA_MS300_RESET_WRITE_PROTECT = 1,                      // Parameter write protect
    E_DELTA_MS300_RESET_KWH_DISPLAY = 5,                        // Reset KWH display to 0
    E_DELTA_MS300_RESET_PLC = 6,                                // Reset PLC
    E_DELTA_MS300_RESET_CANOPEN_SLAVE = 7,                      // Reset CANopen index (Slave)
    E_DELTA_MS300_RESET_KEYPAD_NO_RESPONSE = 8,                 // Keypad doesn't respond
    E_DELTA_MS300_RESET_ALL_PARAMS_50HZ = 9,                    // All parameters are reset to factory settings (base frequency is 50 Hz)
    E_DELTA_MS300_RESET_ALL_PARAMS_60HZ = 10,                   // All parameters are reset to factory settings (base frequency is 60Hz)
    E_DELTA_MS300_RESET_ALL_PARAMS_50HZ_SAVE_USER = 11,         // All parameters are reset to factory settings (base frequency is 50 Hz) saving user defined parameters 13-01~13-50
    E_DELTA_MS300_RESET_ALL_PARAMS_60HZ_SAVE_USER = 12,         // All parameters are reset to factory settings (base frequency is 60 Hz) saving user defined parameters 13-01~13-50
};

// Parameter 00-03: Start-up Display Selection
#define E_DELTA_MS300_STARTUP_DISPLAY_SELECTION 0x0003
enum class E_DELTA_MS300_STARTUP_DISPLAY_SETTINGS : uint8_t
{
    E_DELTA_MS300_DISPLAY_FREQ_CMD = 0,                         // F (frequency command)
    E_DELTA_MS300_DISPLAY_OUTPUT_FREQ = 1,                      // H (output frequency)
    E_DELTA_MS300_DISPLAY_USER_DEFINED = 2,                     // U (user defined) Pr. 00-04
    E_DELTA_MS300_DISPLAY_OUTPUT_CURRENT = 3,                   // A (output current)
};

// Parameter 00-16: Load Selection
#define E_DELTA_MS300_LOAD_SELECTION 0x0010
enum class E_DELTA_MS300_LOAD_SELECTION_SETTINGS : uint8_t
{
    E_DELTA_MS300_LOAD_NORMAL = 0, // Normal duty: Overload rated output current 150% in 3 sec, 120% in 1 min.
    E_DELTA_MS300_LOAD_HEAVY = 1,  // Heavy duty: Overload rated output current 200% in 3 sec, 150% in 1 min.
};

// Parameter 00-20: Source of the Master Frequency Command
#define E_DELTA_MS300_MASTER_FREQ_CMD_SOURCE 0x0014
enum class E_DELTA_MS300_MASTER_FREQ_CMD_SOURCE_SETTINGS : uint8_t
{
    E_DELTA_MS300_MFCS_DIGITAL_KEYPAD = 0,               // Digital keypad
    E_DELTA_MS300_MFCS_COMM_RS485 = 1,                   // Communication RS-485 input
    E_DELTA_MS300_MFCS_EXT_ANALOG_INPUT = 2,             // External analog input (Refer to Pr. 03-00)
    E_DELTA_MS300_MFCS_EXT_UP_DOWN = 3,                  // External UP / DOWN terminal
    E_DELTA_MS300_MFCS_PULSE_INPUT = 4,                  // Pulse input without direction command (Refer to Pr. 10-16)
    E_DELTA_MS300_MFCS_CANOPEN = 6,                      // CANopen communication card
    E_DELTA_MS300_MFCS_KEYPAD_DIAL = 7,                  // Digital keypad dial
    E_DELTA_MS300_MFCS_COMM_CARD = 8                     // Communication card (not includes CANopen card)
};

// Parameter 00-21: Source of the Operation Command (AUTO)
#define E_DELTA_MS300_OPERATION_CMD_SOURCE 0x0015
enum class E_DELTA_MS300_OPERATION_CMD_SOURCE_SETTINGS : uint8_t
{
    E_DELTA_MS300_OCS_DIGITAL_KEYPAD = 0,         // Digital keypad
    E_DELTA_MS300_OCS_EXTERNAL_TERMINALS = 1,     // External terminals
    E_DELTA_MS300_OCS_COMM_RS485 = 2,             // Communication RS-485 input
    E_DELTA_MS300_OCS_CANOPEN = 3,                // CANopen communication card
    E_DELTA_MS300_OCS_COMM_CARD = 5,              // Communication card (not includes CANopen card)
};

// Parameter 00-22: Stop Method
#define E_DELTA_MS300_STOP_METHOD 0x0016
enum class E_DELTA_MS300_STOP_METHOD_SETTINGS : uint8_t
{
    E_DELTA_MS300_STOP_METHOD_RAMP = 0,           // Ramp to stop: The AC motor drive decelerates to 0 or minimum output frequency then stops.
    E_DELTA_MS300_STOP_METHOD_COAST = 1           // Coast to stop: Stop by inertia.
};
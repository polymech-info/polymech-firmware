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
 * 11. Parameters are grouped group and parameter number, for example, Parameter 04-10 = 040AH, displayed as boxes, blackground and white text, eg "00-01"
 */

// Delta MS300 Parameter Group Communication Access Addresses (from manual):
// GG = parameter group, nn = parameter number
// Example: Parameter 04-10 = 040AH

// Convenience macros for Delta MS300 register operations
// For Control Command Register 2000H
#define DELTA_MS300_SET_FUNCTION_BITS(reg, func) ((reg) = ((reg) & ~0x0003) | ((func) & 0x03))
#define DELTA_MS300_SET_DIRECTION_BITS(reg, dir) ((reg) = ((reg) & ~0x0030) | (((dir) & 0x03) << 4))
#define DELTA_MS300_SET_ACCEL_BITS(reg, accel) ((reg) = ((reg) & ~0x00C0) | (((accel) & 0x03) << 6))
#define DELTA_MS300_SET_SPEED_BITS(reg, speed) ((reg) = ((reg) & ~0x0F00) | (((speed) & 0x0F) << 8))
#define DELTA_MS300_SET_ENABLE_BIT(reg, enable) ((reg) = ((reg) & ~0x1000) | (((enable) & 0x01) << 12))
#define DELTA_MS300_SET_OPERATION_BITS(reg, op) ((reg) = ((reg) & ~0x6000) | (((op) & 0x03) << 13))

#define DELTA_MS300_GET_FUNCTION_BITS(reg) ((reg) & 0x0003)
#define DELTA_MS300_GET_DIRECTION_BITS(reg) (((reg) & 0x0030) >> 4)
#define DELTA_MS300_GET_ACCEL_BITS(reg) (((reg) & 0x00C0) >> 6)
#define DELTA_MS300_GET_SPEED_BITS(reg) (((reg) & 0x0F00) >> 8)
#define DELTA_MS300_GET_ENABLE_BIT(reg) (((reg) & 0x1000) >> 12)
#define DELTA_MS300_GET_OPERATION_BITS(reg) (((reg) & 0x6000) >> 13)

// Build complete command register
#define DELTA_MS300_BUILD_CMD(func, dir, accel, speed, enable, op) \
    (((func) & 0x03) | (((dir) & 0x03) << 4) | (((accel) & 0x03) << 6) | \
     (((speed) & 0x0F) << 8) | (((enable) & 0x01) << 12) | (((op) & 0x03) << 13))

// For Operation Status Register 2101H
#define DELTA_MS300_GET_DRIVE_STATUS_BITS(reg) ((reg) & 0x0003)
#define DELTA_MS300_GET_JOG_STATUS_BIT(reg) (((reg) & 0x0004) >> 2)
#define DELTA_MS300_GET_OP_DIRECTION_BITS(reg) (((reg) & 0x0018) >> 3)

// For Drive Status Register 2226H
#define DELTA_MS300_GET_DISP_STATUS_DIRECTION(reg) ((reg) & 0x0003)      // bit 1-0
#define DELTA_MS300_GET_DISP_STATUS_STATE(reg)     (((reg) & 0x000C) >> 2) // bit 3-2
#define DELTA_MS300_GET_DISP_STATUS_OUTPUT(reg)    (((reg) & 0x0010) >> 4) // bit 4
#define DELTA_MS300_GET_DISP_STATUS_ALARM(reg)     (((reg) & 0x0020) >> 5) // bit 5

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
    E_DELTA_MS300_CONTROL_OF_MOTOR_DIRECTION = 0x0017,            // 00-23: Control of Motor Direction. Factory Setting: 0. See E_DELTA_MS300_MOTOR_DIRECTION_CONTROL_SETTINGS.
    E_DELTA_MS300_KEYPAD_FREQ_CMD_MEMORY = 0x0018,                // 00-24: Memory of Digital Operator (Keypad) Frequency Command. Read Only.
    E_DELTA_MS300_USER_DEFINED_CHARACTERISTICS = 0x0019,          // 00-25: User Defined Characteristics. Factory Setting: 0. See E_DELTA_MS300_USER_DECIMAL_PLACE and E_DELTA_MS300_USER_UNIT.
    E_DELTA_MS300_MAX_USER_DEFINED_VALUE = 0x001A,                // 00-26: Max. User Defined Value. Scales the user-defined display. 0 = Disable. This value corresponds to Pr. 01-00 (Max. Operation Freq). Range depends on Pr. 00-25. Factory Setting: 0
    E_DELTA_MS300_USER_DEFINED_VALUE = 0x001B,                    // 00-27: User Defined Value. Displays scaled value when Pr. 00-26 is not 0. Valid only when keypad or RS-485 is freq source. Read Only.
    E_DELTA_MS300_LOCAL_REMOTE_SELECTION = 0x001D,                // 00-29: LOCAL / REMOTE Selection. Defines behavior when switching control modes. Factory Setting: 0. See E_DELTA_MS300_LOCAL_REMOTE_SETTINGS.
    E_DELTA_MS300_HAND_MASTER_FREQ_CMD_SOURCE = 0x001E,           // 00-30: Source of the Master Frequency Command (HAND). Factory Setting: 0. See E_DELTA_MS300_HAND_MASTER_FREQ_CMD_SOURCE_SETTINGS.
    E_DELTA_MS300_HAND_OPERATION_CMD_SOURCE = 0x001F,             // 00-31: Source of the Operation Command (HAND). Factory Setting: 0. See E_DELTA_MS300_HAND_OPERATION_CMD_SOURCE_SETTINGS.
    E_DELTA_MS300_DIGITAL_KEYPAD_STOP_FUNCTION = 0x0020,          // 00-32: Digital Keypad STOP Function. Factory Setting: 0. See E_DELTA_MS300_DIGITAL_KEYPAD_STOP_SETTINGS.
    E_DELTA_MS300_SOURCE_OF_AUXILIARY_FREQ = 0x0023,              // 00-35: Source of Auxiliary Frequency. Factory Setting: 0. See E_DELTA_MS300_AUXILIary_FREQ_SOURCE_SETTINGS.
    E_DELTA_MS300_MASTER_AUX_FREQ_CMD_SELECTION = 0x0024,         // 00-36: Selection of Master and Auxiliary Frequency Command. Factory Setting: 0. See E_DELTA_MS300_MASTER_AUX_FREQ_CMD_SELECTION_SETTINGS.
    E_DELTA_MS300_DISPLAY_FILTER_TIME_CURRENT = 0x0030,           // 00-48: Display Filter Time (Current). Set this parameter to minimize the current fluctuation displayed by digital keypad. Factory Setting: 0.100.
    E_DELTA_MS300_DISPLAY_FILTER_TIME_KEYPAD = 0x0031,            // 00-49: Display Filter Time (Keypad). Set this parameter to minimize the value fluctuation displayed by digital keypad. Factory Setting: 0.100.
    E_DELTA_MS300_SOFTWARE_VERSION_DATE = 0x0032,                 // 00-50: Software Version (Date). Displays current software version of drive by date. Read Only.

    // 01: Basic Parameters
    E_DELTA_MS300_MAX_OP_FREQ_MOTOR_1 = 0x0100,                   // 01-00: Max. Operation Frequency of Motor 1.
    E_DELTA_MS300_OUTPUT_FREQ_MOTOR_1 = 0x0101,                   // 01-01: Output Frequency of Motor 1 (Base Frequency).
    E_DELTA_MS300_OUTPUT_VOLTAGE_MOTOR_1 = 0x0102,                // 01-02: Output Voltage of Motor 1 (Rated Voltage).
    E_DELTA_MS300_MID_POINT_FREQ_1_MOTOR_1 = 0x0103,              // 01-03: Mid-point Frequency 1 of Motor 1.
    E_DELTA_MS300_MID_POINT_VOLTAGE_1_MOTOR_1 = 0x0104,           // 01-04: Mid-point Voltage 1 of Motor 1.
    E_DELTA_MS300_MID_POINT_FREQ_2_MOTOR_1 = 0x0105,              // 01-05: Mid-point Frequency 2 of Motor 1.
    E_DELTA_MS300_MID_POINT_VOLTAGE_2_MOTOR_1 = 0x0106,           // 01-06: Mid-point Voltage 2 of Motor 1.
    E_DELTA_MS300_MIN_OUTPUT_FREQ_MOTOR_1 = 0x0107,               // 01-07: Min. Output Frequency of Motor 1. Settings 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_MIN_OUTPUT_VOLTAGE_MOTOR_1 = 0x0108,            // 01-08: Min. Output Voltage of Motor 1. Settings 0.0~240.0V (110/230V) / 0.0~480.0V (460V). Factory: 0.0/0.0.
    E_DELTA_MS300_START_FREQ_MOTOR_1 = 0x0109,                    // 01-09: Start-up Frequency of Motor 1. Defines the frequency at which the drive starts. Interacts with Min. Output Freq (Pr. 01-07/01-41) and Freq. Lower Limit (Pr. 01-11). Settings: 0.00-599.00 Hz. Factory: 0.50.
    E_DELTA_MS300_FREQ_UPPER_LIMIT_MOTOR_1 = 0x010A,              // 01-10: Output Frequency Upper Limit of Motor 1. Limits the actual output frequency. If the frequency command is higher, the drive runs at this limit. Settings 0.00~599.00 Hz. Factory: 599.00.
    E_DELTA_MS300_FREQ_LOWER_LIMIT_MOTOR_1 = 0x010B,              // 01-11: Output Frequency Lower Limit of Motor 1. If the frequency command is lower, the drive runs at this limit (if > Pr. 01-07). Must be < Pr. 01-10. Settings 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_ACCEL_TIME_1_MOTOR_1 = 0x010C,                  // 01-12: Accel. Time 1. Time to accelerate from 0 Hz to maximum output frequency (Pr. 01-00). Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_DECEL_TIME_1_MOTOR_1 = 0x010D,                  // 01-13: Decel. Time 1. Time to decelerate from maximum output frequency (Pr. 01-00) to 0 Hz. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_ACCEL_TIME_2_MOTOR_1 = 0x010E,                  // 01-14: Accel. Time 2. Selected via multi-function inputs. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_DECEL_TIME_2_MOTOR_1 = 0x010F,                  // 01-15: Decel. Time 2. Selected via multi-function inputs. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_ACCEL_TIME_3_MOTOR_1 = 0x0110,                  // 01-16: Accel. Time 3. Selected via multi-function inputs. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_DECEL_TIME_3_MOTOR_1 = 0x0111,                  // 01-17: Decel. Time 3. Selected via multi-function inputs. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_ACCel_TIME_4_MOTOR_1 = 0x0112,                  // 01-18: Accel. Time 4. Selected via multi-function inputs. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_DECEL_TIME_4_MOTOR_1 = 0x0113,                  // 01-19: Decel. Time 4. Selected via multi-function inputs. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_JOG_ACCEL_TIME_MOTOR_1 = 0x0114,                // 01-20: JOG Acceleration Time. Time to accelerate from 0Hz to JOG frequency. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_JOG_DECEL_TIME_MOTOR_1 = 0x0115,                // 01-21: JOG Deceleration Time. Time to decelerate from JOG frequency to 0Hz. Range depends on Pr. 01-45. Factory: 10.00.
    E_DELTA_MS300_JOG_FREQ = 0x0116,                              // 01-22: JOG Frequency. The target frequency for JOG operation. Settings 0.00~599.00 Hz. Factory: 6.00.
    E_DELTA_MS300_FIRST_FOURTH_ACCEL_DECEL_FREQ = 0x0117,         // 01-23: 1st/4th Accel./Decel. Frequency. Frequency threshold to switch between 1st and 4th accel/decel times. Settings 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_S_CURVE_ACCEL_BEGIN_TIME_1 = 0x0118,            // 01-24: S-curve Acceleration Begin Time 1. Defines the S-curve for acceleration start. Range depends on Pr. 01-45. Factory: 0.20.
    E_DELTA_MS300_S_CURVE_ACCEL_ARRIVAL_TIME_2 = 0x0119,          // 01-25: S-curve Acceleration Arrival Time 2. Defines the S-curve for acceleration end. Range depends on Pr. 01-45. Factory: 0.20.
    E_DELTA_MS300_S_CURVE_DECEL_BEGIN_TIME_1 = 0x011A,            // 01-26: S-curve Deceleration Begin Time 1. Defines the S-curve for deceleration start. Range depends on Pr. 01-45. Factory: 0.20.
    E_DELTA_MS300_S_CURVE_DECEL_ARRIVAL_TIME_2 = 0x011B,          // 01-27: S-curve Deceleration Arrival Time 2. Defines the S-curve for deceleration end. Range depends on Pr. 01-45. Factory: 0.2/0.20.
    E_DELTA_MS300_SKIP_FREQ_1_UPPER = 0x011C,                     // 01-28: Skip Frequency 1 (Upper Limit). Range 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_SKIP_FREQ_1_LOWER = 0x011D,                     // 01-29: Skip Frequency 1 (Lower Limit). Range 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_SKIP_FREQ_2_UPPER = 0x011E,                     // 01-30: Skip Frequency 2 (Upper Limit). Range 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_SKIP_FREQ_2_LOWER = 0x011F,                     // 01-31: Skip Frequency 2 (Lower Limit). Range 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_SKIP_FREQ_3_UPPER = 0x0120,                     // 01-32: Skip Frequency 3 (Upper Limit). Range 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_SKIP_FREQ_3_LOWER = 0x0121,                     // 01-33: Skip Frequency 3 (Lower Limit). Range 0.00~599.00 Hz. Factory: 0.00.
    E_DELTA_MS300_ZERO_SPEED_MODE = 0x0122,                       // 01-34: Zero Speed Mode. Defines motor behavior at 0Hz after a STOP command. Factory: 0.

    // 06: Protection Parameters
    E_DELTA_MS300_PTC_DETECTION_SELECTION = 0x061D,               // 06-29: PTC Detection Selection. Factory Setting: 0. See E_DELTA_MS300_PTC_DETECTION_SETTINGS.
    E_DELTA_MS300_PTC_LEVEL = 0x061E,                             // 06-30: PTC Level. Settings 0.0~100.0 %. Factory Setting: 50.0.
    E_DELTA_MS300_FREQ_CMD_AT_MALFUNCTION = 0x061F,               // 06-31: Frequency Command for Malfunction. Read Only. Settings 0.00-599.00 Hz.
    E_DELTA_MS300_OUTPUT_FREQ_AT_MALFUNCTION = 0x0620,            // 06-32: Output Frequency at Malfunction. Read Only. Settings 0.00-599.00 Hz.
    E_DELTA_MS300_OUTPUT_VOLTAGE_AT_MALFUNCTION = 0x0621,         // 06-33: Output Voltage at Malfunction. Read Only. Settings 0.0-6553.5 V.
    E_DELTA_MS300_DC_VOLTAGE_AT_MALFUNCTION = 0x0622,             // 06-34: DC Voltage at Malfunction. Read Only. Settings 0.0-6553.5 V.
    E_DELTA_MS300_OUTPUT_CURRENT_AT_MALFUNCTION = 0x0623,         // 06-35: Output Current at Malfunction. Read Only. Settings 0.00-655.35 Amp.
    E_DELTA_MS300_IGBT_TEMP_AT_MALFUNCTION = 0x0624,              // 06-36: IGBT Temperature at Malfunction. Read Only. Settings 0.0-6553.5 C.
    E_DELTA_MS300_CAPACITANCE_TEMP_AT_MALFUNCTION = 0x0625,       // 06-37: Capacitance Temperature at Malfunction. Read Only. Settings -0.0-6553.5 C.
    E_DELTA_MS300_MOTOR_SPEED_RPM_AT_MALFUNCTION = 0x0626,        // 06-38: Motor Speed in rpm at Malfunction. Read Only. Settings 0-65535 rpm.
    E_DELTA_MS300_MULTI_FUNC_INPUT_STATUS_AT_MALFUNCTION = 0x0628, // 06-40: Status of Multi-function Input Terminal at Malfunction. Read Only.
    E_DELTA_MS300_MULTI_FUNC_OUTPUT_STATUS_AT_MALFUNCTION = 0x0629,// 06-41: Status of Multi-function Output Terminal at Malfunction. Read Only.
    E_DELTA_MS300_DRIVE_STATUS_AT_MALFUNCTION = 0x062A,            // 06-42: Drive Status at Malfunction. Read Only.
    E_DELTA_MS300_STO_LATCH_SELECTION = 0x062C,                   // 06-44: STO Latch Selection.
    E_DELTA_MS300_OPHL_TREATMENT = 0x062D,                        // 06-45: Treatment to Output Phase Loss Detection (OPHL).
    E_DELTA_MS300_OPHL_DETECTION_TIME = 0x062E,                   // 06-46: Detection Time of Output Phase Loss.
    E_DELTA_MS300_OPHL_CURRENT_LEVEL = 0x062F,                    // 06-47: Current Detection Level of Output Phase Loss.
    E_DELTA_MS300_OPHL_DC_BRAKE_TIME = 0x0630,                    // 06-48: DC Brake Time of Output Phase Loss.
    E_DELTA_MS300_PT100_VOLTAGE_LEVEL_1 = 0x0638,                 // 06-56: PT100 Voltage Level 1.
    E_DELTA_MS300_PT100_VOLTAGE_LEVEL_2 = 0x0639,                 // 06-57: PT100 Voltage Level 2.
    E_DELTA_MS300_PT100_LEVEL_1_FREQUENCY_PROTECTION = 0x063A,    // 06-58: PT100 Level 1 Frequency Protection.
    E_DELTA_MS300_PT100_L1_DELAY_TIME = 0x063B,                   // 06-59: Delay Time of Activating PT100 Level 1 Frequency Protection.
    E_DELTA_MS300_SOFTWARE_DETECTION_GFF_CURRENT_LEVEL = 0x063C,  // 06-60: Software Detection GFF Current Level.
    E_DELTA_MS300_SOFTWARE_DETECTION_GFF_FILTER_TIME = 0x063D,    // 06-61: Software Detection GFF Filter Time.
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_1_DAY = 0x063F,     // 06-63: Operation Time of Fault Record 1 (Day).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_1_MIN = 0x0640,     // 06-64: Operation Time of Fault Record 1 (Min.).
    E_DELTA_MS300_LVX_AUTO_RESET = 0x0649,                        // 06-49: LvX Auto Reset.
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_2_DAY = 0x0641,     // 06-65: Operation Time of Fault Record 2 (Day).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_2_MIN = 0x0642,     // 06-66: Operation Time of Fault Record 2 (Min.).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_3_DAY = 0x0643,     // 06-67: Operation Time of Fault Record 3 (Day).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_3_MIN = 0x0644,     // 06-68: Operation Time of Fault Record 3 (Min.).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_4_DAY = 0x0645,     // 06-69: Operation Time of Fault Record 4 (Day).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_4_MIN = 0x0646,     // 06-70: Operation Time of Fault Record 4 (Min.).
    E_DELTA_MS300_LOW_CURRENT_SETTING_LEVEL = 0x0647,             // 06-71: Low Current Setting Level.
    E_DELTA_MS300_LOW_CURRENT_DETECTION_TIME = 0x0648,            // 06-72: Low Current Detection Time.
    E_DELTA_MS300_TREATMENT_FOR_LOW_CURRENT = 0x0649,             // 06-73: Treatment for Low Current.
    E_DELTA_MS300_INPUT_PHASE_LOSS_TREATMENT = 0x0653,            // 06-53: Treatment for the Detected Input Phase Loss (OrP).
    E_DELTA_MS300_DERATING_PROTECTION = 0x0655,                   // 06-55: Derating Protection.
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_5_DAY = 0x065A,     // 06-90: Operation Time of Fault Record 5 (Day).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_5_MIN = 0x065B,     // 06-91: Operation Time of Fault Record 5 (Min.).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_6_DAY = 0x065C,     // 06-92: Operation Time of Fault Record 6 (Day).
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_6_MIN = 0x065D,     // 06-93: Operation Time of Fault Record 6 (Min.).

    // 07: Special Parameters (was: Motor Parameters)
    E_DELTA_MS300_SOFTWARE_BRAKE_LEVEL = 0x0700,                  // 07-00: Software Brake Level.
    E_DELTA_MS300_DC_BRAKE_CURRENT_LEVEL = 0x0701,                // 07-01: DC Brake Current Level.
    E_DELTA_MS300_DC_BRAKE_TIME_STARTUP = 0x0702,                 // 07-02: DC Brake Time at Start-up.
    E_DELTA_MS300_DC_BRAKE_TIME_AT_STOP = 0x0703,                 // 07-03: DC Brake Time at Stop.
    E_DELTA_MS300_DC_BRAKE_START_FREQUENCY = 0x0704,              // 07-04: DC Brake Start Frequency.
    E_DELTA_MS300_VOLTAGE_INCREASING_GAIN = 0x0705,               // 07-05: Voltage Increasing Gain for speed tracking.
    E_DELTA_MS300_RESTART_AFTER_MOMENTARY_POWER_LOSS = 0x0706,     // 07-06: Restart after Momentary Power Loss.
    E_DELTA_MS300_ALLOWED_POWER_LOSS_DURATION = 0x0707,           // 07-07: Allowed Power Loss Duration.
    E_DELTA_MS300_BASE_BLOCK_TIME = 0x0708,                       // 07-08: Base Block Time.
    E_DELTA_MS300_CURRENT_LIMIT_OF_SPEED_TRACKING = 0x0709,       // 07-09: Current Limit of Speed Tracking.
    E_DELTA_MS300_RESTART_AFTER_FAULT_TREATMENT = 0x070A,         // 07-10: Treatment of Restart after Fault.
    E_DELTA_MS300_RESTART_TIMES_AFTER_FAULT = 0x070B,             // 07-11: Restart Times after Fault.
    E_DELTA_MS300_SPEED_TRACKING_DURING_STARTUP = 0x070C,         // 07-12: Speed Tracking during Start-up.
    E_DELTA_MS300_DEB_FUNCTION_SELECTION = 0x070D,                // 07-13: dEb (Deceleration Energy Backup) Function Selection.
    E_DELTA_MS300_DWELL_TIME_AT_ACCEL = 0x070F,                   // 07-15: Dwell Time at Accel.
    E_DELTA_MS300_DWELL_FREQUENCY_AT_ACCEL = 0x0710,              // 07-16: Dwell Frequency at Accel.
    E_DELTA_MS300_Dwell_TIME_AT_DECEL = 0x0711,                   // 07-17: Dwell Time at Decel.
    E_DELTA_MS300_DWELL_FREQUENCY_AT_DECEL = 0x0712,              // 07-18: Dwell Frequency at Decel.
    E_DELTA_MS300_FAN_COOLING_CONTROL = 0x0713,                   // 07-19: Fan Cooling Control.
    E_DELTA_MS300_EMERGENCY_FORCED_STOP_DECELERATION = 0x0714,     // 07-20: Deceleration of Emergency or Forced Stop.
    E_DELTA_MS300_AUTO_ENERGY_SAVING_SETTING = 0x0715,            // 07-21: Auto Energy-saving Setting.
    E_DELTA_MS300_ENERGY_SAVING_GAIN = 0x0716,                    // 07-22: Energy-saving Gain.
    E_DELTA_MS300_AUTO_VOLTAGE_REGULATION = 0x0717,               // 07-23: Auto Voltage Regulation (AVR) Function.
    E_DELTA_MS300_FILTER_TIME_TORQUE_COMMAND = 0x0718,            // 07-24: Filter Time of Torque Command (V/F and SVC Control Mode).
    E_DELTA_MS300_FILTER_TIME_SLIP_COMPENSATION = 0x0719,         // 07-25: Filter Time of Slip Compensation (V/F and SVC Control Mode).
    E_DELTA_MS300_TORQUE_COMPENSATION_GAIN_MOTOR_1 = 0x071A,      // 07-26: Torque Compensation Gain (Motor 1).
    E_DELTA_MS300_SLIP_COMPENSATION_GAIN_MOTOR_1 = 0x071B,        // 07-27: Slip Compensation Gain (Motor 1, V/F and SVC).
    E_DELTA_MS300_SLIP_DEVIATION_LEVEL = 0x071D,                  // 07-29: Slip Deviation Level.
    E_DELTA_MS300_DETECTION_TIME_OF_SLIP_DEVIATION = 0x071E,      // 07-30: Detection Time of Slip Deviation.
    E_DELTA_MS300_TREATMENT_OF_SLIP_DEVIATION = 0x071F,           // 07-31: Treatment of Slip Deviation.
    E_DELTA_MS300_MOTOR_SHOCK_COMPENSATION_FACTOR = 0x0720,      // 07-32: Motor Shock Compensation Factor.
    E_DELTA_MS300_RETURN_TIME_OF_FAULT_RESTART = 0x0721,         // 07-33: Return Time of Fault Restart.
    E_DELTA_MS300_OOB_SAMPLING_TIME = 0x072E,                    // 07-46: OOB Sampling Time.
    E_DELTA_MS300_NUMBER_OF_OOB_SAMPLING_TIMES = 0x072F,        // 07-47: Number of OOB Sampling Times.
    E_DELTA_MS300_OOB_AVERAGE_SAMPLING_ANGLE = 0x0730,           // 07-48: OOB Average Sampling Angle.
    E_DELTA_MS300_DEB_GAIN = 0x073E,                            // 07-62: dEb Gain.
    E_DELTA_MS300_TORQUE_COMPENSATION_GAIN_MOTOR_2 = 0x0747,      // 07-71: Torque Compensation Gain (Motor 2).
    E_DELTA_MS300_SLIP_COMPENSATION_GAIN_MOTOR_2 = 0x0748,        // 07-72: Slip Compensation Gain (Motor 2).
    E_DELTA_MS300_TORQUE_COMPENSATION_GAIN_MOTOR_3 = 0x0749,      // 07-73: Torque Compensation Gain (Motor 3).
    E_DELTA_MS300_SLIP_COMPENSATION_GAIN_MOTOR_3 = 0x074A,        // 07-74: Slip Compensation Gain (Motor 3).
    E_DELTA_MS300_TORQUE_COMPENSATION_GAIN_MOTOR_4 = 0x074B,      // 07-75: Torque Compensation Gain (Motor 4).
    E_DELTA_MS300_SLIP_COMPENSATION_GAIN_MOTOR_4 = 0x074C,        // 07-76: Slip Compensation Gain (Motor 4).

    // 08: High-function PID Parameters
    E_DELTA_MS300_TERMINAL_SELECTION_PID_FEEDBACK = 0x0800,       // 08-00: Terminal Selection of PID Feedback. Factory Setting: 0. See E_DELTA_MS300_PID_FEEDBACK_SELECTION_SETTINGS.
    E_DELTA_MS300_PROPORTIONAL_GAIN_P = 0x0801,                  // 08-01: Proportional Gain (P). Factory Setting: 1.0.
    E_DELTA_MS300_INTEGRAL_TIME_I = 0x0802,                      // 08-02: Integral Time (I). Factory Setting: 1.00.
    E_DELTA_MS300_DERIVATIVE_TIME_D = 0x0803,                    // 08-03: Derivative Time (D). Factory Setting: 0.00.
    E_DELTA_MS300_UPPER_LIMIT_INTEGRAL_CONTROL = 0x0804,         // 08-04: Upper Limit of Integral Control.
    E_DELTA_MS300_PID_OUTPUT_CMD_LIMIT = 0x0805,                 // 08-05: PID Output Command Limit (Positive Limit).
    E_DELTA_MS300_PID_FEEDBACK_VALUE_COMM = 0x0806,              // 08-06: PID Feedback Value by Communication Protocol.
    E_DELTA_MS300_PID_DELAY_TIME = 0x0807,                       // 08-07: PID Delay Time.
    E_DELTA_MS300_PID_FEEDBACK_ERROR_DETECTION_TIME = 0x0808,      // 08-08: PID Feedback Error Detection Time.
    E_DELTA_MS300_PID_TREATMENT_FEEDBACK_FAULT = 0x0809,           // 08-09: Treatment of the Feedback Signal Fault.
    E_DELTA_MS300_PID_SLEEP_FREQUENCY = 0x080A,                    // 08-10: Sleep Frequency.
    E_DELTA_MS300_WAKE_UP_FREQUENCY = 0x080B,                      // 08-11: Wake-up Frequency.
    E_DELTA_MS300_PID_SLEEP_TIME = 0x080C,                         // 08-12: Sleep Time.
    E_DELTA_MS300_PID_DEVIATION_LEVEL = 0x080D,                    // 08-13: PID Deviation Level.
    E_DELTA_MS300_PID_DEVIATION_TIME = 0x080E,                     // 08-14: PID Deviation Time.
    E_DELTA_MS300_PID_FEEDBACK_FILTER_TIME = 0x080F,               // 08-15: Filter Time for PID Feedback.
    E_DELTA_MS300_PID_COMPENSATION_SELECTION = 0x0810,             // 08-16: PID Compensation Selection.
    E_DELTA_MS300_PID_MODE_SELECTION = 0x0814,                   // 08-20: PID Mode Selection.
    E_DELTA_MS300_PID_DIRECTION = 0x0815,                          // 08-21: PID Direction.

    // 21: Monitoring Parameters
    E_DELTA_MS300_FAULT_STATUS = 0x2101,                          // 21-01: Fault Status Register & Drive Operation Status. A bitmap where each bit corresponds to a fault type. Write 1 to clear a bit.
    E_DELTA_MS300_MON_FREQ_CMD = 0x2102,                          // Read-only: Frequency command (XXX.XX Hz).
    E_DELTA_MS300_MON_OUTPUT_FREQ = 0x2103,                       // Read-only: Output frequency (XXX.XX Hz).
    E_DELTA_MS300_MON_OUTPUT_CURRENT = 0x2104,                    // Read-only: Output current (XX.XX A or XXX.X A). Auto-scales. Decimal pos in high byte of 211FH.
    E_DELTA_MS300_MON_DC_BUS_VOLTAGE = 0x2105,                    // Read-only: DC-BUS voltage (XXX.X V).
    E_DELTA_MS300_MON_OUTPUT_VOLTAGE = 0x2106,                    // Read-only: Output voltage (XXX.X V).
    E_DELTA_MS300_MON_CURRENT_STEP_NUM = 0x2107,                  // Read-only: Current step number of multi-stage speed operation.
    // 0x2108 is Reserved
    E_DELTA_MS300_MON_COUNTER_VALUE = 0x2109,                     // Read-only: Counter value.
    E_DELTA_MS300_MON_POWER_FACTOR_ANGLE = 0x210A,                // Read-only: Power factor angle (XXX.X).
    E_DELTA_MS300_MON_OUTPUT_TORQUE = 0x210B,                     // Read-only: Output torque (XXX.X %).
    E_DELTA_MS300_MON_ACTUAL_MOTOR_SPEED_RPM = 0x210C,            // Read-only: Actual motor speed (XXXXX rpm).
    E_DELTA_MS300_MON_PG_FEEDBACK_PULSES = 0x210D,                // Read-only: Number of PG feedback pulses (0~65535).
    E_DELTA_MS300_MON_PG2_PULSE_COMMANDS = 0x210E,                // Read-only: Number of PG2 pulse commands (0~65535).
    E_DELTA_MS300_MON_POWER_OUTPUT_KWH = 0x210F,                  // Read-only: Power output (X.XXX KWH).
    E_DELTA_MS300_MON_MULTIFUNCTION_DISPLAY = 0x2116,             // Read-only: Multi-function display (content depends on Pr. 00-04).
    E_DELTA_MS300_MON_MAX_FREQ_OR_USER_VALUE = 0x211B,            // Read-only: Dynamic value based on settings (Pr. 01-00 or computed user value).
    E_DELTA_MS300_MON_CURRENT_DISPLAY_DECIMAL = 0x211F,           // Read-only: High byte contains decimal position for current value display.

    // 22: Display Monitoring Parameters
    E_DELTA_MS300_DISP_OUTPUT_CURRENT_A = 0x2200,                 // Read-only: Display output current (A). Auto-scales like 2104H, decimal pos in high byte of 211FH.
    E_DELTA_MS300_DISP_COUNTER_VALUE = 0x2201,                    // Read-only: Display counter value (c).
    E_DELTA_MS300_DISP_ACTUAL_OUTPUT_FREQ_HZ = 0x2202,            // Read-only: Actual output frequency (XXXXX Hz). Unit: 1 Hz.
    E_DELTA_MS300_DISP_DC_BUS_VOLTAGE = 0x2203,                   // Read-only: DC-BUS voltage (XXX.X V).
    E_DELTA_MS300_DISP_OUTPUT_VOLTAGE = 0x2204,                   // Read-only: Output voltage (XXX.X V).
    E_DELTA_MS300_DISP_POWER_ANGLE = 0x2205,                      // Read-only: Power angle (XXX.X).
    E_DELTA_MS300_DISP_ACTUAL_MOTOR_SPEED_KW = 0x2206,            // Read-only: Display actual motor speed kW of U, V, W (XXXXX kW).
    E_DELTA_MS300_DISP_MOTOR_SPEED_RPM_ESTIMATED = 0x2207,        // Read-only: Display motor speed in rpm estimated by drive or encoder (XXXXX rpm).
    E_DELTA_MS300_DISP_OUTPUT_TORQUE_PERCENT = 0x2208,            // Read-only: Display positive/negative output torque in % (XXX.X %).
    E_DELTA_MS300_DISP_PG_FEEDBACK = 0x2209,                      // Read-only: Display PG feedback (as Pr. 00-04 NOTE 1).
    E_DELTA_MS300_DISP_PID_FEEDBACK_VALUE_PERCENT = 0x220A,       // Read-only: PID feedback value after enabling PID function (XXX.XX %).
    // 0x220B is Reserved
    E_DELTA_MS300_DISP_ACI_ANALOG_INPUT_PERCENT = 0x220C,         // Read-only: Display signal of ACI analog input terminal (0.00-100.00%).
    // 0x220D is Reserved
    E_DELTA_MS300_DISP_IGBT_TEMP = 0x220E,                        // Read-only: IGBT temperature of drive power module (XXX.X C).
    E_DELTA_MS300_DISP_CAPACITANCE_TEMP = 0x220F,                 // Read-only: The temperature of capacitance (XXX.X C).
    E_DELTA_MS300_DISP_DIGITAL_INPUT_STATUS = 0x2210,             // Read-only: The status of digital input (ON/OFF), refer to Pr. 02-12.
    E_DELTA_MS300_DISP_DIGITAL_OUTPUT_STATUS = 0x2211,            // 22-11: Status of digital output (ON/OFF), ref Pr. 02-18. Read-only.
    E_DELTA_MS300_DISP_MULTI_STEP_SPEED_EXECUTING = 0x2212,       // 22-12: The multi-step speed that is executing (S). Read-only.
    E_DELTA_MS300_DISP_CPU_PIN_STATUS_DIGITAL_INPUT = 0x2213,     // 22-13: The corresponding CPU pin status of digital input (d.). Read-only.
    E_DELTA_MS300_DISP_CPU_PIN_STATUS_DIGITAL_OUTPUT = 0x2214,    // 22-14: The corresponding CPU pin status of digital output (O.). Read-only.
    E_DELTA_MS300_DISP_MOTOR_REVOLUTION_PG1 = 0x2215,             // 22-15: Number of actual motor revolution (PG1 of PG card) (P.), max 65535. Read-only.
    E_DELTA_MS300_DISP_PULSE_INPUT_FREQ_PG2 = 0x2216,             // 22-16: Pulse input frequency (PG2 of PG card) (XXX.XX Hz). Read-only.
    E_DELTA_MS300_DISP_PULSE_INPUT_POS_PG2 = 0x2217,              // 22-17: Pulse input position (PG card PG2), max 65535. Read-only.
    E_DELTA_MS300_DISP_POS_CMD_TRACING_ERROR = 0x2218,            // 22-18: Position command tracing error. Read-only.
    E_DELTA_MS300_DISP_COUNTER_OVERLOAD_TIMES = 0x2219,           // 22-19: Display times of counter overload (XXX.XX %). Read-only.
    E_DELTA_MS300_DISP_GFF_PERCENT = 0x221A,                      // 22-1A: GFF (XXX.XX %). Read-only.
    E_DELTA_MS300_DISP_DCBUS_VOLTAGE_RIPPLES = 0x221B,            // 22-1B: DCbus voltage ripples (XXX.X V). Read-only.
    E_DELTA_MS300_DISP_PLC_REGISTER_D1043_DATA = 0x221C,          // 22-1C: PLC register D1043 data (C). Read-only.
    E_DELTA_MS300_DISP_PM_MOTOR_POLE = 0x221D,                    // 22-1D: Pole of Permanent Magnet Motor. Read-only.
    E_DELTA_MS300_DISP_USER_PAGE_PHYSICAL_MEASURE = 0x221E,       // 22-1E: User page displays the value in physical measure. Read-only.
    E_DELTA_MS300_DISP_OUTPUT_VALUE_OF_PR_00_05 = 0x221F,         // 22-1F: Output Value of Pr. 00-05 (XXX.XX Hz). Read-only.
    E_DELTA_MS300_DISP_MOTOR_TURNS = 0x2220,                      // 22-20: Number of motor turns when drive operates. Read-only.
    E_DELTA_MS300_DISP_MOTOR_OPERATION_POSITION = 0x2221,         // 22-21: Operation position of motor. Read-only.
    E_DELTA_MS300_DISP_FAN_SPEED_PERCENT = 0x2222,                // 22-22: Fan speed of the drive (XXX %). Read-only.
    E_DELTA_MS300_DISP_DRIVE_CONTROL_MODE = 0x2223,               // 22-23: Control mode of the drive. 0=Speed, 1=Torque. Read-only.
    E_DELTA_MS300_DISP_DRIVE_CARRIER_FREQ_KHZ = 0x2224,           // 22-24: Carrier frequency of the drive (XX KHz). Read-only.
    // 0x2225 is Reserved
    E_DELTA_MS300_DISP_DRIVE_STATUS = 0x2226,                     // 22-26: Drive status (bit-mapped). Read-only. See E_DELTA_MS300_DISP_... enums.
    E_DELTA_MS300_DISP_ESTIMATED_OUTPUT_TORQUE = 0x2227,          // 22-27: Drive's estimated output torque (positive or negative) (XXXX Nt-m). Read-only.
    E_DELTA_MS300_DISP_TORQUE_COMMAND_PERCENT = 0x2228,           // 22-28: Torque command (XXX.X %). Read-only.
    E_DELTA_MS300_DISP_KWH = 0x2229,                              // 22-29: KWH display (XXXX.X). Read-only.
    E_DELTA_MS300_DISP_MI7_PULSE_INPUT_LOW = 0x222A,              // 22-2A: MI7 pulse input in Low Word. Read-only.
    E_DELTA_MS300_DISP_MI7_PULSE_INPUT_HIGH = 0x222B,             // 22-2B: MI7 pulse input in High Word. Read-only.
    E_DELTA_MS300_DISP_MOTOR_ACTUAL_POS_LOW = 0x222C,             // 22-2C: Motor actual position in Low Word. Read-only.
    E_DELTA_MS300_DISP_MOTOR_ACTUAL_POS_HIGH = 0x222D,            // 22-2D: Motor actual position in High Word. Read-only.
    E_DELTA_MS300_DISP_PID_REFERENCE_PERCENT = 0x222E,            // 22-2E: PID reference (XXX.XX %). Read-only.
    E_DELTA_MS300_DISP_PID_OFFSET_PERCENT = 0x222F,               // 22-2F: PID offset (XXX.XX %). Read-only.
    E_DELTA_MS300_DISP_PID_OUTPUT_FREQ = 0x2230,                  // 22-30: PID output frequency (XXX.XX Hz). Read-only.
    E_DELTA_MS300_DISP_HARDWARE_ID = 0x2231,                      // 22-31: Hardware ID. Read-only.
    E_DELTA_MS300_DISP_AUXILIARY_FREQ = 0x2232,                   // 22-32: Display auxiliary frequency. Read-only.
    E_DELTA_MS300_DISP_MASTER_FREQ = 0x2233,                      // 22-33: Display master frequency. Read-only.
    E_DELTA_MS300_DISP_FREQ_AFTER_ADDSUB = 0x2234,                // 22-34: Display frequency after addition/subtraction of auxiliary and master frequency. Read-only.
    
    // Direct Control & Status Registers (from manual Address List)
    E_DELTA_MS300_CONTROL_COMMAND = 0x2000,               // Write-only control word. Bit-mapped. See enums below.
    E_DELTA_MS300_FREQUENCY_COMMAND = 0x2001,             // Write-only frequency setpoint (XXX.XX Hz). Unit: 0.01 Hz.
    E_DELTA_MS300_ACTION_COMMAND = 0x2002,                // Write-only command for EF ON, Reset, B.B ON.
    E_DELTA_MS300_STATUS_CODES = 0x2100,                  // Read-only status: High byte = Warn code, Low Byte = Error code.
};

//
// 00: Drive Parameters
//

// Parameter 00-04: Content of Multi-function Display (User Defined)
#define E_DELTA_MS300_MULTIFUNCTION_DISPLAY_CONTENT 0x0004
enum class E_DELTA_MS300_MULTIFUNCTION_DISPLAY_SETTINGS : uint8_t
{
    // Note: Full list of display items is not on this page. Adding known values.
    E_DELTA_MS300_DISPLAY_OUTPUT_CURRENT = 3,         // Factory Setting.
    E_DELTA_MS300_DISPLAY_PID_FEEDBACK_PERCENT = 10   // Display PID analog feedback signal value (b) (%).
};

//
// 22: Display Monitoring Parameters
//

// Parameter 22-11: The status of digital output (ON/OFF)
#define E_DELTA_MS300_DISP_DIGITAL_OUTPUT_STATUS 0x2211
// Function: Displays the status of digital output (ON/OFF), ref Pr. 02-18. Read-only.

// Parameter 22-12: The multi-step speed that is executing (S)
#define E_DELTA_MS300_DISP_MULTI_STEP_SPEED_EXECUTING 0x2212
// Function: Displays the multi-step speed that is currently executing (S). Read-only.

// Parameter 22-13: The corresponding CPU pin status of digital input (d.)
#define E_DELTA_MS300_DISP_CPU_PIN_STATUS_DIGITAL_INPUT 0x2213
// Function: Displays the corresponding CPU pin status of digital input (d.). Read-only.

// Parameter 22-14: The corresponding CPU pin status of digital output (O.)
#define E_DELTA_MS300_DISP_CPU_PIN_STATUS_DIGITAL_OUTPUT 0x2214
// Function: Displays the corresponding CPU pin status of digital output (O.). Read-only.

// Parameter 22-15: Number of actual motor revolution (PG1 of PG card) (P.)
#define E_DELTA_MS300_DISP_MOTOR_REVOLUTION_PG1 0x2215
// Function: Displays the number of actual motor revolutions from PG1 of PG card. Max value 65535. Read-only.

// Parameter 22-16: Pulse input frequency (PG2 of PG card) (XXX.XX Hz)
#define E_DELTA_MS300_DISP_PULSE_INPUT_FREQ_PG2 0x2216
// Function: Displays the pulse input frequency from PG2 of PG card (XXX.XX Hz). Read-only.

// Parameter 22-17: Pulse input position (PG card PG2)
#define E_DELTA_MS300_DISP_PULSE_INPUT_POS_PG2 0x2217
// Function: Displays the pulse input position from PG card PG2. Max value 65535. Read-only.

// Parameter 22-18: Position command tracing error
#define E_DELTA_MS300_DISP_POS_CMD_TRACING_ERROR 0x2218
// Function: Displays the position command tracing error. Read-only.

// Parameter 22-19: Display times of counter overload (XXX.XX %)
#define E_DELTA_MS300_DISP_COUNTER_OVERLOAD_TIMES 0x2219
// Function: Displays the number of times the counter has overloaded (XXX.XX %). Read-only.

// Parameter 22-1A: GFF (XXX.XX %)
#define E_DELTA_MS300_DISP_GFF_PERCENT 0x221A
// Function: Displays GFF (XXX.XX %). Read-only.

// Parameter 22-1B: DCbus voltage ripples (XXX.X V)
#define E_DELTA_MS300_DISP_DCBUS_VOLTAGE_RIPPLES 0x221B
// Function: Displays DC bus voltage ripples (XXX.X V). Read-only.

// Parameter 22-1C: PLC register D1043 data (C)
#define E_DELTA_MS300_DISP_PLC_REGISTER_D1043_DATA 0x221C
// Function: Displays data from PLC register D1043 (C). Read-only.

// Parameter 22-1D: Pole of Permanent Magnet Motor
#define E_DELTA_MS300_DISP_PM_MOTOR_POLE 0x221D
// Function: Displays the pole of the Permanent Magnet Motor. Read-only.

// Parameter 22-1E: User page displays the value in physical measure
#define E_DELTA_MS300_DISP_USER_PAGE_PHYSICAL_MEASURE 0x221E
// Function: User page displays the value in physical measure. Read-only.

// Parameter 22-1F: Output Value of Pr. 00-05 (XXX.XX Hz)
#define E_DELTA_MS300_DISP_OUTPUT_VALUE_OF_PR_00_05 0x221F
// Function: Displays the output value of Pr. 00-05 (XXX.XX Hz). Read-only.

// Parameter 22-20: Number of motor turns when drive operates
#define E_DELTA_MS300_DISP_MOTOR_TURNS 0x2220
// Function: Displays the number of motor turns. Keeps value when drive stops, resets to zero on operation. Read-only.

// Parameter 22-21: Operation position of motor
#define E_DELTA_MS300_DISP_MOTOR_OPERATION_POSITION 0x2221
// Function: Displays the operation position of the motor. Keeps value when drive stops, resets to zero on operation. Read-only.

// Parameter 22-22: Fan speed of the drive (XXX %)
#define E_DELTA_MS300_DISP_FAN_SPEED_PERCENT 0x2222
// Function: Displays the fan speed of the drive (XXX %). Read-only.

// Parameter 22-23: Control mode of the drive
#define E_DELTA_MS300_DISP_DRIVE_CONTROL_MODE 0x2223
enum class E_DELTA_MS300_DISP_CONTROL_MODE_STATE : uint8_t
{
    SPEED_MODE = 0,
    TORQUE_MODE = 1
};

// Parameter 22-24: Carrier frequency of the drive (XX KHz)
#define E_DELTA_MS300_DISP_DRIVE_CARRIER_FREQ_KHZ 0x2224
// Function: Displays the carrier frequency of the drive (XX KHz). Read-only.

// Parameter 22-26: Drive status (bit-mapped)
#define E_DELTA_MS300_DISP_DRIVE_STATUS 0x2226
// Bit-mapped register. Use the DELTA_MS300_GET_DISP_STATUS_* macros to extract fields. Read-only.
enum class E_DELTA_MS300_DISP_STATUS_DIRECTION : uint8_t
{
    NO_DIRECTION = 0, // 00b
    FORWARD = 1,      // 01b
    REVERSE = 2       // 10b
};
enum class E_DELTA_MS300_DISP_STATUS_STATE : uint8_t
{
    // Values 00b and 11b are not defined in manual
    DRIVER_READY = 1, // 01b
    ERROR = 2         // 10b
};
enum class E_DELTA_MS300_DISP_STATUS_OUTPUT : uint8_t
{
    NOT_OUTPUT = 0, // 0b: Motor drive did not output
    DID_OUTPUT = 1  // 1b: Motor drive did output
};
enum class E_DELTA_MS300_DISP_STATUS_ALARM : uint8_t
{
    NO_ALARM = 0, // 0b
    HAVE_ALARM = 1 // 1b
};

// Parameter 22-27: Drive's estimated output torque (positive or negative direction)
#define E_DELTA_MS300_DISP_ESTIMATED_OUTPUT_TORQUE 0x2227
// Function: Displays the drive's estimated output torque (XXXX Nt-m). Read-only.

// Parameter 22-28: Torque command (XXX.X %)
#define E_DELTA_MS300_DISP_TORQUE_COMMAND_PERCENT 0x2228
// Function: Displays the torque command (XXX.X %). Read-only.

// Parameter 22-29: KWH display (XXXX.X)
#define E_DELTA_MS300_DISP_KWH 0x2229
// Function: Displays the KWH reading (XXXX.X). Read-only.

// Parameter 22-2A: MI7 pulse input in Low Word
#define E_DELTA_MS300_DISP_MI7_PULSE_INPUT_LOW 0x222A
// Function: Displays the low word of the MI7 pulse input. Read-only.

// Parameter 22-2B: MI7 pulse input in High Word
#define E_DELTA_MS300_DISP_MI7_PULSE_INPUT_HIGH 0x222B
// Function: Displays the high word of the MI7 pulse input. Read-only.

// Parameter 22-2C: Motor actual position in Low Word
#define E_DELTA_MS300_DISP_MOTOR_ACTUAL_POS_LOW 0x222C
// Function: Displays the low word of the motor's actual position. Read-only.

// Parameter 22-2D: Motor actual position in High Word
#define E_DELTA_MS300_DISP_MOTOR_ACTUAL_POS_HIGH 0x222D
// Function: Displays the high word of the motor's actual position. Read-only.

// Parameter 22-2E: PID reference (XXX.XX %)
#define E_DELTA_MS300_DISP_PID_REFERENCE_PERCENT 0x222E
// Function: Displays the PID reference value (XXX.XX %). Read-only.

// Parameter 22-2F: PID offset (XXX.XX %)
#define E_DELTA_MS300_DISP_PID_OFFSET_PERCENT 0x222F
// Function: Displays the PID offset value (XXX.XX %). Read-only.

// Parameter 22-30: PID output frequency (XXX.XX Hz)
#define E_DELTA_MS300_DISP_PID_OUTPUT_FREQ 0x2230
// Function: Displays the PID output frequency (XXX.XX Hz). Read-only.

// Parameter 22-31: Hardware ID
#define E_DELTA_MS300_DISP_HARDWARE_ID 0x2231
// Function: Displays the hardware ID. Read-only.

// Parameter 22-32: Display auxiliary frequency
#define E_DELTA_MS300_DISP_AUXILIARY_FREQ 0x2232
// Function: Displays the auxiliary frequency. Read-only.

// Parameter 22-33: Display master frequency
#define E_DELTA_MS300_DISP_MASTER_FREQ 0x2233
// Function: Displays the master frequency. Read-only.

// Parameter 22-34: Display frequency after addition and subtraction
#define E_DELTA_MS300_DISP_FREQ_AFTER_ADDSUB 0x2234
// Function: Displays the frequency after addition and subtraction of auxiliary and master frequency. Read-only.

//
// Direct Control and Status Registers
//

// Register 2000H: Command write only (Control word)
// Address defined in enum E_DELTA_MS300_REGISTERS::E_DELTA_MS300_CONTROL_COMMAND
// This is a bit-mapped register. Use the DELTA_MS300_BUILD_CMD macro at the top of the file to build the command.
// bit 1-0: Function
// bit 3-2: Reserved
// bit 5-4: Direction
// bit 7-6: Accel/Decel
// bit 11-8: Speed
// bit 12: Enable bit 06-11 function
// bit 14-13: Operation source
// bit 15: Reserved

enum class E_DELTA_MS300_CMD_FUNCTION : uint8_t
{
    NO_FUNCTION = 0, // 00B
    STOP = 1,        // 01B
    RUN = 2,         // 10B
    JOG_RUN = 3      // 11B
};

enum class E_DELTA_MS300_CMD_DIRECTION : uint8_t
{
    NO_FUNCTION = 0,      // 00B
    FWD = 1,              // 01B
    REV = 2,              // 10B
    CHANGE_DIRECTION = 3, // 11B
};

enum class E_DELTA_MS300_CMD_ACCEL_DECEL : uint8_t
{
    ACCEL_DECEL_1 = 0, // 00B: 1st accel. / decel.
    ACCEL_DECEL_2 = 1, // 01B: 2nd accel. / decel.
    ACCEL_DECEL_3 = 2, // 10B: 3rd accel. / decel.
    ACCEL_DECEL_4 = 3, // 11B: 4th accel. / decel.
};

enum class E_DELTA_MS300_CMD_SPEED_SELECT : uint8_t
{
    MASTER_SPEED = 0,          // 0000B
    STAGE_SPEED_1 = 1,         // 0001B
    STAGE_SPEED_2 = 2,         // 0010B
    STAGE_SPEED_3 = 3,         // 0011B
    STAGE_SPEED_4 = 4,         // 0100B
    STAGE_SPEED_5 = 5,         // 0101B
    STAGE_SPEED_6 = 6,         // 0110B
    STAGE_SPEED_7 = 7,         // 0111B
    STAGE_SPEED_8 = 8,         // 1000B
    STAGE_SPEED_9 = 9,         // 1001B
    STAGE_SPEED_10 = 10,       // 1010B
    STAGE_SPEED_11 = 11,       // 1011B
    STAGE_SPEED_12 = 12,       // 1100B
    STAGE_SPEED_13 = 13,       // 1101B
    STAGE_SPEED_14 = 14,       // 1110B
    STAGE_SPEED_15 = 15,       // 1111B
};

enum class E_DELTA_MS300_CMD_ENABLE : uint8_t
{
    DISABLE = 0,
    ENABLE_BIT_06_11 = 1 // bit 12: Enable bit 06-11 function
};

enum class E_DELTA_MS300_CMD_OPERATION_SOURCE : uint8_t
{
    NO_FUNCTION = 0,               // 00B
    DIGITAL_KEYPAD = 1,            // 01B: Operated by digital keypad
    PR_00_21_SETTING = 2,          // 10B: Operated by Pr. 00-21 setting
    CHANGE_OPERATION_SOURCE = 3,   // 11B
};

// Register 2001H: Frequency command
// Address defined in enum E_DELTA_MS300_REGISTERS::E_DELTA_MS300_FREQUENCY_COMMAND
// Function: Frequency command (XXX.XX Hz). Write-only.
// Value is frequency in 0.01 Hz units. E.g., 50.00 Hz = 5000.

// Register 2002H: Action command
// Address defined in enum E_DELTA_MS300_REGISTERS::E_DELTA_MS300_ACTION_COMMAND
enum class E_DELTA_MS300_ACTION_CMD_BITS : uint16_t
{
    // bit 0: 1 = EF (external fault) on
    EF_ON = (1 << 0),
    // bit 1: 1 = Reset
    RESET = (1 << 1),
    // bit 2: 1 = B.B ON (Base Block)
    BASE_BLOCK_ON = (1 << 2),
};

// Register 2100H: Status codes
// Address defined in enum E_DELTA_MS300_REGISTERS::E_DELTA_MS300_STATUS_CODES
// Function: Read-only status.
// High byte: Warn code
// Low Byte: Error code

// Register 2101H: AC motor drive operation status
// Address defined in enum E_DELTA_MS300_REGISTERS::E_DELTA_MS300_FAULT_STATUS (0x2101)
// Note: This address is shared with Pr 21-01 (E_DELTA_MS300_FAULT_STATUS).
// This provides a bit-mapped view of the drive's current operational state.
// Use DELTA_MS300_GET_* macros to extract fields.
// bit 1~0: AC motor drive operation status. See E_DELTA_MS300_DRIVE_STATUS.
// bit 2: 1 = JOG command
// bit 4~3: Operation direction. See E_DELTA_MS300_OPERATION_DIRECTION.
// bit 8: 1 = Master frequency controlled by communication interface
// bit 9: 1 = Master frequency controlled by analog signal
// bit 10: 1 = Operation command controlled by communication interface
// bit 11: 1 = Parameter locked
// bit 12: 1 = Enable to copy parameters from keypad
// bit 15~13: Reserved

enum class E_DELTA_MS300_DRIVE_STATUS : uint8_t
{
    DRIVE_STOPS = 0,         // 00B
    DRIVE_DECELERATING = 1,  // 01B
    DRIVE_STANDBY = 2,       // 10B
    DRIVE_OPERATING = 3,     // 11B
};

enum class E_DELTA_MS300_OPERATION_DIRECTION : uint8_t
{
    FWD_RUN = 0,                 // 00B
    FROM_REV_TO_FWD_RUN = 1,     // 01B
    REV_RUN = 2,                 // 10B
    FROM_FWD_TO_REV_RUN = 3,     // 11B
};
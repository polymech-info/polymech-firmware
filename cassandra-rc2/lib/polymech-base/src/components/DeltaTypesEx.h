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
    E_DELTA_MS300_IDENTITY_CODE = 0x0000,
    E_DELTA_MS300_RATED_CURRENT = 0x0001,
    E_DELTA_MS300_PARAMETER_RESET = 0x0002,
    E_DELTA_MS300_STARTUP_DISPLAY_SELECTION = 0x0003,
    E_DELTA_MS300_MULTIFUNCTION_DISPLAY_CONTENT = 0x0004,
    E_DELTA_MS300_COEFFICIENT_GAIN_ACTUAL_OUTPUT_FREQ = 0x0005, 
    E_DELTA_MS300_SOFTWARE_VERSION = 0x0006,
    E_DELTA_MS300_PARAMETER_PROTECTION_PASSWORD_INPUT = 0x0007,
    E_DELTA_MS300_PARAMETER_PROTECTION_PASSWORD_SETTING = 0x0008,
    E_DELTA_MS300_CONTROL_MODE = 0x000A,
    E_DELTA_MS300_CONTROL_OF_SPEED_MODE = 0x000B,
    E_DELTA_MS300_LOAD_SELECTION = 0x0010,
    E_DELTA_MS300_CARRIER_FREQUENCY = 0x0011,
    E_DELTA_MS300_PLC_COMMAND_MASK = 0x0013,
    E_DELTA_MS300_MASTER_FREQ_CMD_SOURCE = 0x0014,
    E_DELTA_MS300_OPERATION_CMD_SOURCE = 0x0015,
    E_DELTA_MS300_STOP_METHOD = 0x0016,
    E_DELTA_MS300_CONTROL_OF_MOTOR_DIRECTION = 0x0017,
    E_DELTA_MS300_KEYPAD_FREQ_CMD_MEMORY = 0x0018,
    E_DELTA_MS300_USER_DEFINED_CHARACTERISTICS = 0x0019,
    E_DELTA_MS300_MAX_USER_DEFINED_VALUE = 0x001A,
    E_DELTA_MS300_USER_DEFINED_VALUE = 0x001B,
    E_DELTA_MS300_LOCAL_REMOTE_SELECTION = 0x001D,
    E_DELTA_MS300_HAND_MASTER_FREQ_CMD_SOURCE = 0x001E,
    E_DELTA_MS300_HAND_OPERATION_CMD_SOURCE = 0x001F,
    E_DELTA_MS300_DIGITAL_KEYPAD_STOP_FUNCTION = 0x0020,
    E_DELTA_MS300_SOURCE_OF_AUXILIARY_FREQ = 0x0023,
    E_DELTA_MS300_MASTER_AUX_FREQ_CMD_SELECTION = 0x0024,
    E_DELTA_MS300_DISPLAY_FILTER_TIME_CURRENT = 0x0030,
    E_DELTA_MS300_DISPLAY_FILTER_TIME_KEYPAD = 0x0031,
    E_DELTA_MS300_SOFTWARE_VERSION_DATE = 0x0032,
  
    // 01: Basic Parameters
    E_DELTA_MS300_MAX_OP_FREQ_MOTOR_1 = 0x0100,
    E_DELTA_MS300_OUTPUT_FREQ_MOTOR_1 = 0x0101,
    E_DELTA_MS300_OUTPUT_VOLTAGE_MOTOR_1 = 0x0102,
    E_DELTA_MS300_MID_POINT_FREQ_1_MOTOR_1 = 0x0103,
    E_DELTA_MS300_MID_POINT_VOLTAGE_1_MOTOR_1 = 0x0104,
    E_DELTA_MS300_MID_POINT_FREQ_2_MOTOR_1 = 0x0105,
    E_DELTA_MS300_MID_POINT_VOLTAGE_2_MOTOR_1 = 0x0106,
    E_DELTA_MS300_MIN_OUTPUT_FREQ_MOTOR_1 = 0x0107,
    E_DELTA_MS300_MIN_OUTPUT_VOLTAGE_MOTOR_1 = 0x0108,
    E_DELTA_MS300_START_FREQ_MOTOR_1 = 0x0109,
    E_DELTA_MS300_FREQ_UPPER_LIMIT_MOTOR_1 = 0x010A,
    E_DELTA_MS300_FREQ_LOWER_LIMIT_MOTOR_1 = 0x010B,
    E_DELTA_MS300_ACCEL_TIME_1_MOTOR_1 = 0x010C,
    E_DELTA_MS300_DECEL_TIME_1_MOTOR_1 = 0x010D,
    E_DELTA_MS300_ACCEL_TIME_2_MOTOR_1 = 0x010E,
    E_DELTA_MS300_DECEL_TIME_2_MOTOR_1 = 0x010F,
    E_DELTA_MS300_ACCEL_TIME_3_MOTOR_1 = 0x0110,
    E_DELTA_MS300_DECEL_TIME_3_MOTOR_1 = 0x0111,
    E_DELTA_MS300_ACCEL_TIME_4_MOTOR_1 = 0x0112,
    E_DELTA_MS300_DECEL_TIME_4_MOTOR_1 = 0x0113,
    E_DELTA_MS300_JOG_ACCEL_TIME_MOTOR_1 = 0x0114,
    E_DELTA_MS300_JOG_DECEL_TIME_MOTOR_1 = 0x0115,
    E_DELTA_MS300_JOG_FREQ = 0x0116,
    E_DELTA_MS300_FIRST_FOURTH_ACCEL_DECEL_FREQ = 0x0117,
    E_DELTA_MS300_S_CURVE_ACCEL_BEGIN_TIME_1 = 0x0118,
    E_DELTA_MS300_S_CURVE_ACCEL_ARRIVAL_TIME_2 = 0x0119,
    E_DELTA_MS300_S_CURVE_DECEL_BEGIN_TIME_1 = 0x011A,
    E_DELTA_MS300_S_CURVE_DECEL_ARRIVAL_TIME_2 = 0x011B,
    E_DELTA_MS300_SKIP_FREQ_1_UPPER = 0x011C,
    E_DELTA_MS300_SKIP_FREQ_1_LOWER = 0x011D,
    E_DELTA_MS300_SKIP_FREQ_2_UPPER = 0x011E,
    E_DELTA_MS300_SKIP_FREQ_2_LOWER = 0x011F,
    E_DELTA_MS300_SKIP_FREQ_3_UPPER = 0x0120,
    E_DELTA_MS300_SKIP_FREQ_3_LOWER = 0x0121,
    E_DELTA_MS300_ZERO_SPEED_MODE = 0x0122,
    E_DELTA_MS300_VF_CURVE_SELECTION = 0x012B,
    E_DELTA_MS300_AUTO_ACCEL_DECEL_SETTING = 0x012C,
    E_DELTA_MS300_ACCEL_DECEL_S_CURVE_TIME_UNIT = 0x012D,
    E_DELTA_MS300_CANOPEN_QUICK_STOP_TIME = 0x012E,
    E_DELTA_MS300_OUTPUT_FREQ_MOTOR_2 = 0x0135,
    E_DELTA_MS300_OUTPUT_VOLTAGE_MOTOR_2 = 0x0136,
    E_DELTA_MS300_MID_POINT_FREQ_1_MOTOR_2 = 0x0137,
    E_DELTA_MS300_MID_POINT_VOLTAGE_1_MOTOR_2 = 0x0138,
    E_DELTA_MS300_MID_POINT_FREQ_2_MOTOR_2 = 0x0139,
    E_DELTA_MS300_MID_POINT_VOLTAGE_2_MOTOR_2 = 0x0140,
    E_DELTA_MS300_MIN_OUTPUT_FREQ_MOTOR_2 = 0x0141,
    E_DELTA_MS300_MIN_OUTPUT_VOLTAGE_MOTOR_2 = 0x0142,
    E_DELTA_MS300_DECELERATION_METHOD = 0x0149,
    E_DELTA_MS300_MAX_OP_FREQ_MOTOR_2 = 0x0152,
    E_DELTA_MS300_MAX_OP_FREQ_MOTOR_3 = 0x0153,
    E_DELTA_MS300_OUTPUT_FREQ_MOTOR_3 = 0x0154,
    E_DELTA_MS300_OUTPUT_VOLTAGE_MOTOR_3 = 0x0155,
    E_DELTA_MS300_MID_POINT_FREQ_1_MOTOR_3 = 0x0156,
    E_DELTA_MS300_MID_POINT_VOLTAGE_1_MOTOR_3 = 0x0157,
    E_DELTA_MS300_MID_POINT_FREQ_2_MOTOR_3 = 0x0158,
    E_DELTA_MS300_MID_POINT_VOLTAGE_2_MOTOR_3 = 0x0159,
    E_DELTA_MS300_MIN_OUTPUT_FREQ_MOTOR_3 = 0x0160,
    E_DELTA_MS300_MIN_OUTPUT_VOLTAGE_MOTOR_3 = 0x0161,
    E_DELTA_MS300_MAX_OP_FREQ_MOTOR_4 = 0x0162,
    E_DELTA_MS300_OUTPUT_FREQ_MOTOR_4 = 0x0163,
    E_DELTA_MS300_OUTPUT_VOLTAGE_MOTOR_4 = 0x0164,
    E_DELTA_MS300_MID_POINT_FREQ_1_MOTOR_4 = 0x0165,
    E_DELTA_MS300_MID_POINT_VOLTAGE_1_MOTOR_4 = 0x0166,
    E_DELTA_MS300_MID_POINT_FREQ_2_MOTOR_4 = 0x0167,
    E_DELTA_MS300_MID_POINT_VOLTAGE_2_MOTOR_4 = 0x0168,
    E_DELTA_MS300_MIN_OUTPUT_FREQ_MOTOR_4 = 0x0169, 
    E_DELTA_MS300_MIN_OUTPUT_VOLTAGE_MOTOR_4 = 0x0170,          

    // 02: Digital Input / Output Parameters
    E_DELTA_MS300_2_3_WIRE_OPERATION_CONTROL = 0x0200,          
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_1 = 0x0201,             
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_2 = 0x0202,               
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_3 = 0x0203,              
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_4 = 0x0204,                
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_5 = 0x0205,               
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_6 = 0x0206,                
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_7 = 0x0207,               
    E_DELTA_MS300_UP_DOWN_KEY_MODE = 0x0209,                      
    E_DELTA_MS300_UP_DOWN_KEY_CONSTANT_SPEED = 0x020A,            
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_RESPONSE_TIME = 0x020B,    
    E_DELTA_MS300_MULTI_FUNCTION_INPUT_MODE_SELECTION = 0x020C,   
    E_DELTA_MS300_MULTI_FUNCTION_OUTPUT_1 = 0x020D,               
    E_DELTA_MS300_MULTI_FUNCTION_OUTPUT_2 = 0x0210,              
    E_DELTA_MS300_MULTI_FUNCTION_OUTPUT_3 = 0x0211,               
    E_DELTA_MS300_MULTI_FUNCTION_OUTPUT_DIRECTION = 0x0212,       
    E_DELTA_MS300_TERMINAL_COUNTING_VALUE_ATTAINED = 0x0213,       
    E_DELTA_MS300_PRELIMINARY_COUNTING_VALUE_ATTAINED = 0x0214,    
    E_DELTA_MS300_DIGITAL_OUTPUT_GAIN = 0x0215,                   
    E_DELTA_MS300_DESIRED_FREQ_ATTAINED_1 = 0x0216,                
    E_DELTA_MS300_DESIRED_FREQ_ATTAINED_1_WIDTH = 0x0217,         
    E_DELTA_MS300_DESIRED_FREQ_ATTAINED_2 = 0x0218,               

    // 06: Protection Parameters
    E_DELTA_MS300_PTC_DETECTION_SELECTION = 0x061D,             
    E_DELTA_MS300_PTC_LEVEL = 0x061E,                           
    E_DELTA_MS300_FREQ_CMD_AT_MALFUNCTION = 0x061F,             
    E_DELTA_MS300_OUTPUT_FREQ_AT_MALFUNCTION = 0x0620,           
    E_DELTA_MS300_OUTPUT_VOLTAGE_AT_MALFUNCTION = 0x0621,       
    E_DELTA_MS300_DC_VOLTAGE_AT_MALFUNCTION = 0x0622,           
    E_DELTA_MS300_OUTPUT_CURRENT_AT_MALFUNCTION = 0x0623,         
    E_DELTA_MS300_IGBT_TEMP_AT_MALFUNCTION = 0x0624,              
    E_DELTA_MS300_CAPACITANCE_TEMP_AT_MALFUNCTION = 0x0625,       
    E_DELTA_MS300_MOTOR_SPEED_RPM_AT_MALFUNCTION = 0x0626,        
    E_DELTA_MS300_MULTI_FUNC_INPUT_STATUS_AT_MALFUNCTION = 0x0628,
    E_DELTA_MS300_MULTI_FUNC_OUTPUT_STATUS_AT_MALFUNCTION = 0x0629,
    E_DELTA_MS300_DRIVE_STATUS_AT_MALFUNCTION = 0x062A,          
    E_DELTA_MS300_STO_LATCH_SELECTION = 0x062C,                   
    E_DELTA_MS300_OPHL_TREATMENT = 0x062D,                      
    E_DELTA_MS300_OPHL_DETECTION_TIME = 0x062E,                 
    E_DELTA_MS300_OPHL_CURRENT_LEVEL = 0x062F,                 
    E_DELTA_MS300_OPHL_DC_BRAKE_TIME = 0x0630,                 
    E_DELTA_MS300_PT100_VOLTAGE_LEVEL_1 = 0x0638,                
    E_DELTA_MS300_PT100_VOLTAGE_LEVEL_2 = 0x0639,               
    E_DELTA_MS300_PT100_LEVEL_1_FREQUENCY_PROTECTION = 0x063A,   
    E_DELTA_MS300_PT100_L1_DELAY_TIME = 0x063B,                 
    E_DELTA_MS300_SOFTWARE_DETECTION_GFF_CURRENT_LEVEL = 0x063C,  
    E_DELTA_MS300_SOFTWARE_DETECTION_GFF_FILTER_TIME = 0x063D,    
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_1_DAY = 0x063F,     
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_1_MIN = 0x0640,     
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_2_DAY = 0x0641,     
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_2_MIN = 0x0642,     
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_3_DAY = 0x0643,     
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_3_MIN = 0x0644,     
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_4_DAY = 0x0645,     
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_4_MIN = 0x0646,     
    E_DELTA_MS300_LOW_CURRENT_SETTING_LEVEL = 0x0647,            
    E_DELTA_MS300_LOW_CURRENT_DETECTION_TIME = 0x0648,          
    E_DELTA_MS300_TREATMENT_FOR_LOW_CURRENT = 0x0649,            
    E_DELTA_MS300_INPUT_PHASE_LOSS_TREATMENT = 0x0653,           
    E_DELTA_MS300_DERATING_PROTECTION = 0x0655,                 
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_5_DAY = 0x065A,   
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_5_MIN = 0x065B,     
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_6_DAY = 0x065C,    
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_6_MIN = 0x065D,   
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_7_DAY = 0x065E,
    E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_7_MIN = 0x065F,

    // 07: Special Parameters
    E_DELTA_MS300_SOFTWARE_BRAKE_LEVEL = 0x0700,                 
    E_DELTA_MS300_DC_BRAKE_CURRENT_LEVEL = 0x0701,              
    E_DELTA_MS300_DC_BRAKE_TIME_STARTUP = 0x0702,               
    E_DELTA_MS300_DC_BRAKE_TIME_AT_STOP = 0x0703,                 
    E_DELTA_MS300_DC_BRAKE_START_FREQUENCY = 0x0704,             
    E_DELTA_MS300_VOLTAGE_INCREASING_GAIN = 0x0705,               
    E_DELTA_MS300_RESTART_AFTER_MOMENTARY_POWER_LOSS = 0x0706,    
    E_DELTA_MS300_ALLOWED_POWER_LOSS_DURATION = 0x0707,         
    E_DELTA_MS300_BASE_BLOCK_TIME = 0x0708,                       
    E_DELTA_MS300_CURRENT_LIMIT_OF_SPEED_TRACKING = 0x0709,       
    E_DELTA_MS300_RESTART_AFTER_FAULT_TREATMENT = 0x070A,         
    E_DELTA_MS300_RESTART_TIMES_AFTER_FAULT = 0x070B,             
    E_DELTA_MS300_SPEED_TRACKING_DURING_STARTUP = 0x070C,         
    E_DELTA_MS300_DEB_FUNCTION_SELECTION = 0x070D,               
    E_DELTA_MS300_DWELL_TIME_AT_ACCEL = 0x070F,                   
    E_DELTA_MS300_DWELL_FREQUENCY_AT_ACCEL = 0x0710,             
    E_DELTA_MS300_DWELL_TIME_AT_DECEL = 0x0711,                 
    E_DELTA_MS300_DWELL_FREQUENCY_AT_DECEL = 0x0712,             
    E_DELTA_MS300_FAN_COOLING_CONTROL = 0x0713,                 
    E_DELTA_MS300_EMERGENCY_FORCED_STOP_DECELERATION = 0x0714,     
    E_DELTA_MS300_AUTO_ENERGY_SAVING_SETTING = 0x0715,           
    E_DELTA_MS300_ENERGY_SAVING_GAIN = 0x0716,                    
    E_DELTA_MS300_AUTO_VOLTAGE_REGULATION = 0x0717,              
    E_DELTA_MS300_FILTER_TIME_TORQUE_COMMAND = 0x0718,            
    E_DELTA_MS300_FILTER_TIME_SLIP_COMPENSATION = 0x0719,         
    E_DELTA_MS300_TORQUE_COMPENSATION_GAIN_MOTOR_1 = 0x071A,      
    E_DELTA_MS300_SLIP_COMPENSATION_GAIN_MOTOR_1 = 0x071B,        
    E_DELTA_MS300_SLIP_DEVIATION_LEVEL = 0x071D,                  
    E_DELTA_MS300_DETECTION_TIME_OF_SLIP_DEVIATION = 0x071E,      
    E_DELTA_MS300_TREATMENT_OF_SLIP_DEVIATION = 0x071F,           
    E_DELTA_MS300_MOTOR_SHOCK_COMPENSATION_FACTOR = 0x0720,      
    E_DELTA_MS300_RETURN_TIME_OF_FAULT_RESTART = 0x0721,         
    E_DELTA_MS300_OOB_SAMPLING_TIME = 0x072E,                    
    E_DELTA_MS300_NUMBER_OF_OOB_SAMPLING_TIMES = 0x072F,        
    E_DELTA_MS300_OOB_AVERAGE_SAMPLING_ANGLE = 0x0730,           
    E_DELTA_MS300_DEB_GAIN = 0x073E,                            
    E_DELTA_MS300_TORQUE_COMPENSATION_GAIN_MOTOR_2 = 0x0747,      
    E_DELTA_MS300_SLIP_COMPENSATION_GAIN_MOTOR_2 = 0x0748,        
    E_DELTA_MS300_TORQUE_COMPENSATION_GAIN_MOTOR_3 = 0x0749,      
    E_DELTA_MS300_SLIP_COMPENSATION_GAIN_MOTOR_3 = 0x074A,        
    E_DELTA_MS300_TORQUE_COMPENSATION_GAIN_MOTOR_4 = 0x074B,      
    E_DELTA_MS300_SLIP_COMPENSATION_GAIN_MOTOR_4 = 0x074C,        

    // 08: High-function PID Parameters
    E_DELTA_MS300_TERMINAL_SELECTION_PID_FEEDBACK = 0x0800,       
    E_DELTA_MS300_PROPORTIONAL_GAIN_P = 0x0801,                  
    E_DELTA_MS300_INTEGRAL_TIME_I = 0x0802,                       
    E_DELTA_MS300_DERIVATIVE_TIME_D = 0x0803,                    
    E_DELTA_MS300_UPPER_LIMIT_INTEGRAL_CONTROL = 0x0804,         
    E_DELTA_MS300_PID_OUTPUT_CMD_LIMIT = 0x0805,                 
    E_DELTA_MS300_PID_FEEDBACK_VALUE_COMM = 0x0806,              
    E_DELTA_MS300_PID_DELAY_TIME = 0x0807,                       
    E_DELTA_MS300_PID_FEEDBACK_ERROR_DETECTION_TIME = 0x0808,      
    E_DELTA_MS300_PID_TREATMENT_FEEDBACK_FAULT = 0x0809,           
    E_DELTA_MS300_PID_SLEEP_FREQUENCY = 0x080A,                    
    E_DELTA_MS300_WAKE_UP_FREQUENCY = 0x080B,                      
    E_DELTA_MS300_PID_SLEEP_TIME = 0x080C,                         
    E_DELTA_MS300_PID_DEVIATION_LEVEL = 0x080D,                    
    E_DELTA_MS300_PID_DEVIATION_TIME = 0x080E,                     
    E_DELTA_MS300_PID_FEEDBACK_FILTER_TIME = 0x080F,               
    E_DELTA_MS300_PID_COMPENSATION_SELECTION = 0x0810,             
    E_DELTA_MS300_PID_MODE_SELECTION = 0x0814,                  
    E_DELTA_MS300_PID_DIRECTION = 0x0815,                         

    // 10: Speed Feedback & PM Motor Parameters
    E_DELTA_MS300_PM_SENSORLESS_SPEED_ESTIMATOR_LPF_GAIN = 0x0A22, 
    E_DELTA_MS300_PM_SENSORLESS_SWITCH_FREQ = 0x0A27,             
    E_DELTA_MS300_PM_INITIAL_ANGLE_DETECTION_PULSE = 0x0A2A,      
    E_DELTA_MS300_PM_ZERO_VOLTAGE_TIME_STARTUP = 0x0A31,          
    E_DELTA_MS300_PM_INJECTION_FREQUENCY = 0x0A33,                
    E_DELTA_MS300_PM_INJECTION_MAGNITUDE = 0x0A34,                
    E_DELTA_MS300_PM_POSITION_DETECTION_METHOD = 0x0A35,         

    // Direct Control / Status Registers (2xxxH)
    E_DELTA_MS300_CONTROL_COMMAND = 0x2000,
    E_DELTA_MS300_FREQUENCY_COMMAND = 0x2001,
    E_DELTA_MS300_WARN_ERROR_CODE = 0x2100,
    E_DELTA_MS300_FAULT_STATUS = 0x2101,
};

/*************************************************************************************************/
/* GROUP 00: Drive Parameters                                                                    */
/*************************************************************************************************/

// Parameter 00-02: Parameter Reset
#define E_DELTA_MS300_PARAMETER_RESET 0x0002
enum class E_DELTA_MS300_RESET_SETTINGS : uint8_t
{
    E_DELTA_MS300_RESET_NO_FUNCTION = 0,               // No Function
    E_DELTA_MS300_RESET_WRITE_PROTECT = 1,                 // Parameter setting can be read but not changed (except for this parameter).
    E_DELTA_MS300_RESET_LOCK_ALL_EXCEPT_COMM = 2,      // All parameters are locked except for communication (Pr.09-xx and Pr.00-02)
    E_DELTA_MS300_RESET_KWH_DISPLAY = 5,                        // Reset KWH display to 0
    E_DELTA_MS300_RESET_PLC = 6,                                // Reset PLC
    E_DELTA_MS300_RESET_CANOPEN_SLAVE = 7,                      // Reset CANopen index (Slave)
    E_DELTA_MS300_RESET_KEYPAD_NO_RESPONSE = 8,                 // Keypad doesn't respond
    E_DELTA_MS300_RESET_CLEAR_FAULT_RECORD = 9,        // Clears the fault record.
    E_DELTA_MS300_RESET_TO_50HZ_DEFAULTS = 10,         // Resets all parameters to 50Hz factory defaults.
    E_DELTA_MS300_RESET_TO_60HZ_DEFAULTS = 11,         // Resets all parameters to 60Hz factory defaults.
    E_DELTA_MS300_RESET_ALL_PARAMS_50HZ_SAVE_USER = 11,         // All parameters are reset to factory settings (base frequency is 50 Hz) saving user defined parameters 13-01~13-50
    E_DELTA_MS300_RESET_ALL_PARAMS_60HZ_SAVE_USER = 12,         // All parameters are reset to factory settings (base frequency is 60 Hz) saving user defined parameters 13-01~13-50
};

// Parameter 00-03: Startup Display Selection
#define E_DELTA_MS300_STARTUP_DISPLAY_SELECTION 0x0003
enum class E_DELTA_MS300_STARTUP_DISPLAY_SETTINGS : uint8_t {
  E_DELTA_MS300_DISPLAY_FREQ_CMD = 0,     // Frequency command
  E_DELTA_MS300_DISPLAY_OUTPUT_FREQ = 1,  // Output frequency
  E_DELTA_MS300_DISPLAY_USER_DEFINED = 2, // User-defined
  E_DELTA_MS300_DISPLAY_OUTPUT_CURRENT = 3, // Output current  
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
enum class E_DELTA_MS300_MASTER_FREQ_CMD_SOURCE_SETTINGS : uint8_t{
        E_DELTA_MS300_MFCS_DIGITAL_KEYPAD = 0,             // Digital Keypad
        E_DELTA_MS300_MFCS_COMM_RS485 = 1,                 // Communication RS-485 Input
        E_DELTA_MS300_MFCS_EXT_ANALOG_INPUT = 2,           // External analog input
        E_DELTA_MS300_MFCS_EXT_UP_DOWN = 3,                // External Up/Down Terminal
        E_DELTA_MS300_MFCS_PULSE_INPUT = 4,                // Pulse input no direction command
        E_DELTA_MS300_MFCS_CANOPEN = 6,                    // By CANopen
        E_DELTA_MS300_MFCS_KEYPAD_DIAL = 7,                  // Digital Keypad dial
        E_DELTA_MS300_MFCS_COMM_CARD = 8                   // Communication card (not includes CANopen card)
};

// Parameter 00-21: Source of the Operation Command
#define E_DELTA_MS300_OPERATION_CMD_SOURCE 0x0015
enum class E_DELTA_MS300_OPERATION_CMD_SOURCE_SETTINGS : uint8_t{
        E_DELTA_MS300_OCS_DIGITAL_KEYPAD = 0,             //Digital Keypad
        E_DELTA_MS300_OCS_EXTERNAL_TERMINALS = 1,         //External Terminal
        E_DELTA_MS300_OCS_COMM_RS485 = 2,             //Communication RS-485 Input
        E_DELTA_MS300_OCS_CANOPEN = 3,                //CANopen communication card
    E_DELTA_MS300_OCS_COMM_CARD = 5              //Communication Card
};

// Parameter 00-22: Stop Method
#define E_DELTA_MS300_STOP_METHOD 0x0016
enum class E_DELTA_MS300_STOP_METHOD_SETTINGS : uint8_t{
        E_DELTA_MS300_STOP_METHOD_RAMP = 0,   // Ramp to stop 
        E_DELTA_MS300_STOP_METHOD_COAST = 1     // Coast to stop
};

// Parameter 00-23: Control of Motor Direction
#define E_DELTA_MS300_CONTROL_OF_MOTOR_DIRECTION 0x0017
enum class E_DELTA_MS300_MOTOR_DIRECTION_CONTROL_SETTINGS : uint8_t {
  E_DELTA_MS300_DIR_CTRL_ENABLE_FWD_REV = 0, // Enable Forward / Reverse
  E_DELTA_MS300_DIR_CTRL_DISABLE_REV = 1,    // Disable Reverse
  E_DELTA_MS300_DIR_CTRL_DISABLE_FWD = 2     // Disable Forward
};

/*************************************************************************************************/
/* GROUP 01: Basic Parameters                                                                    */
/*************************************************************************************************/

// Parameter 01-49: Deceleration method
#define E_DELTA_MS300_DECELERATION_METHOD    0x0149
enum class E_DELTA_MS300_DECELERATION_METHOD_SETTINGS : int
{
    E_DELTA_MS300_DECEL_NORMAL                    =0,         // The AC motor drive decelerates to 0 or minimum output frequency then stops.
    E_DELTA_MS300_DECEL_OVERFLUXING_CONTROL       =1,         // When the over-voltage stall prevention takes place, the drive controls deceleration
                                                    // based on Pr. 06-01 and DC BUS voltage recovery.
    E_DELTA_MS300_DECEL_TRACTION_ENERGY_CONTROL =2,         // AC motor drive autotunes output frequency and
                                                    // voltage at deceleration to consume DC BUS energy according to drive’s ability
};

/*************************************************************************************************/
/* GROUP 06: Protection Parameters                                                               */
/*************************************************************************************************/

// Parameter 06-94: Operation Time of Fault Record 7 (Day)
#define E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_7_DAY 0x065E
// Description: Displays the accumulated operation time (days) when the 7th most recent fault occurred.
// Settings: 0~65535 (days)
// Read Only

// Parameter 06-95: Operation Time of Fault Record 7 (Min.)
#define E_DELTA_MS300_OPERATION_TIME_FAULT_RECORD_7_MIN 0x065F
// Description: Displays the accumulated operation time (minutes) when the 7th most recent fault occurred.
// Settings: 0~1439 (minutes)
// Read Only

/*************************************************************************************************/
/* GROUP 10: Speed Feedback & PM Motor Parameters                                                */
/*************************************************************************************************/

// Parameter 10-34: PM Sensorless Speed Estimator Low-pass Filter Gain
#define E_DELTA_MS300_PM_SENSORLESS_SPEED_ESTIMATOR_LPF_GAIN 0x0A22
// Description: Adjusts the response speed of the speed estimator.
//              - Increase gain for low-frequency vibrations.
//              - Decrease gain for high-frequency vibrations.
// Settings: 0.00~655.35
// Factory Setting: 1.00

// Parameter 10-39: Frequency Point when Switch from I/F Mode to PM Sensorless Mode
#define E_DELTA_MS300_PM_SENSORLESS_SWITCH_FREQ 0x0A27
// Description: Sets the frequency at which the drive switches from I/F control to PM sensorless control.
//              - Too low: May not generate enough back-EMF, causing stall/oc.
//              - Too high: Wider I/F area, generates larger current, less energy efficient.
// Settings: 0.00~599.00 Hz
// Factory Setting: 20.00

// Parameter 10-42: Initial Angle Detection Pulse Value
#define E_DELTA_MS300_PM_INITIAL_ANGLE_DETECTION_PULSE 0x0A2A
// Description: Influences the pulse value for initial rotor position detection using pulse injection.
//              Larger pulse improves accuracy but may cause over-current (oc) faults.
//              Increase if running direction and command are opposite at start-up.
//              Decrease if oc occurs at start-up.
// Settings: 0.0~3.0
// Factory Setting: 1.0

// Parameter 10-49: Zero Voltage Time While Start Up
#define E_DELTA_MS300_PM_ZERO_VOLTAGE_TIME_STARTUP 0x0A31
// Description: Sets the duration of 0V output to the motor before start-up to ensure a static state
//              for better angle estimation. Only valid when Pr. 07-12 (Speed tracking during start-up) = 0.
//              - Too high: Longer start-up time.
//              - Too low: Weaker braking performance.
// Settings: 00.000~60.000 sec
// Factory Setting: 00.000

// Parameter 10-51: Injection Frequency
#define E_DELTA_MS300_PM_INJECTION_FREQUENCY 0x0A33
// Description: Sets the high-frequency injection command in PM SVC control mode.
//              - If the motor's rated frequency (Pr. 01-01) is too close to this setting, angle detection accuracy will be affected.
//              - If Pr. 00-17 is lower than (Pr. 10-51 * 10), increase the carrier wave frequency.
//              - Valid only when Pr. 10-53 = 2 (High frequency injection).
// Settings: 0~1200 Hz
// Factory Setting: 500

// Parameter 10-52: Injection Magnitude
#define E_DELTA_MS300_PM_INJECTION_MAGNITUDE 0x0A34
// Description: Sets the magnitude of the high-frequency injection signal in PM SVC control mode.
//              - Increasing the value can improve angle estimation accuracy, but may increase electromagnetic noise.
//              - The drive will receive this value when motor parameter is "Auto".
//              - If the salient pole ratio (Lq/Ld) is low, increase this value for better angle detection.
//              - Valid only when Pr. 10-53 = 2 (High frequency injection).
// Settings: 0.0~200.0 V
// Factory Setting: 15.0 / 30.0

// Parameter 10-53: Position Detection Method
#define E_DELTA_MS300_PM_POSITION_DETECTION_METHOD 0x0A35
enum class E_DELTA_MS300_POSITION_DETECTION_METHOD_SETTINGS : uint8_t
{
    E_DELTA_MS300_POS_DETECT_DISABLED = 0,               // Disabled
    E_DELTA_MS300_POS_DETECT_RATED_CURRENT_ATTRACT = 1,  // Internal 1/4 rated current attracting the rotor to zero degrees
    E_DELTA_MS300_POS_DETECT_HF_INJECTION = 2,           // High frequency injection
    E_DELTA_MS300_POS_DETECT_PULSE_INJECTION = 3,        // Pulse injection
};
// Description: Selects the method for rotor position detection at startup.
//              - It is suggested to set "2" (High frequency injection) for IPM motors.
//              - It is suggested to set "3" (Pulse injection) for SPM motors.
//              - If there is a bad effect with "2" or "3", use "1".
// Settings: See E_DELTA_MS300_POSITION_DETECTION_METHOD_SETTINGS.
// Factory Setting: 0 (Disabled)

/*************************************************************************************************/
/* Direct Control & Status Registers                                                             */
/*************************************************************************************************/

// Register 2101H: Fault Status
#define E_DELTA_MS300_FAULT_STATUS 0x2101
enum class E_DELTA_MS300_FAULT_STATUS_FLAGS : uint16_t
{
    // Bit 0: Over Current (OC).
    // Associated Faults: 79 (Aoc), 80 (boc), 81 (coc), 82 (oPL1), 83 (oPL2), 84 (oPL3)
    E_DELTA_MS300_FAULT_FLAG_OVER_CURRENT = (1 << 0),

    // Bit 1: Over Voltage (OV).
    // Associated Faults: 62 (dEb)
    E_DELTA_MS300_FAULT_FLAG_OVER_VOLTAGE = (1 << 1),

    // Bit 2: Over Load (OL).
    // Associated Faults: 87 (oL3), 128 (ot3), 129 (ot4), 134 (EoL3), 135 (EoL4)
    E_DELTA_MS300_FAULT_FLAG_OVER_LOAD = (1 << 2),

    // Bit 3: System Fault (SYS).
    // Associated Faults: 72 (STL1), 76 (STo), 77 (STL2), 78 (STL3), 127 (CP33), 142 (AUE1), 143 (AUE2), 144 (AUE3)
    E_DELTA_MS300_FAULT_FLAG_SYSTEM = (1 << 3),

    // Bit 4: Feedback Fault (FBK).
    // Associated Faults: 89 (roPd), 140 (Hd6), 141 (b4GFF)
    E_DELTA_MS300_FAULT_FLAG_FEEDBACK = (1 << 4),

    // Bit 5: External Fault (EXI).
    // Associated Faults: 61 (ydc), 63 (oSL)
    E_DELTA_MS300_FAULT_FLAG_EXTERNAL = (1 << 5),

    // Bit 6: Communication Fault (CE).
    // Associated Faults: 58 (CE10), 101 (CGdE), 102 (CHbE), 104 (CbFE), 105 (CldE),
    //         106 (CAdE), 107 (CFrE), 121 (CP20), 123 (CP22), 124 (CP30), 126 (CP32)
    E_DELTA_MS300_FAULT_FLAG_COMMUNICATION = (1 << 6)
};

// Also maps to 2101H for operational status
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
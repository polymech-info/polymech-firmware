#include <cstdint> // Include for standard integer types like uint16_t

// --- Existing Enums (Provided) ---

/**
 * @brief Existing Enums for Omron E5CC/E5EC Commands (Provided)
 * These enums define the possible data values for specific Modbus commands.
 * These are typically written to the "Command Execution" register (address varies by PLC/Master configuration, often 0x0000 or similar).
 * The *data* value written determines the command executed.
 */

/**
 * @brief Defines the possible values for the Communications Writing command (Code 00).
 * This command enables or disables Modbus communications writing.
 */
enum OR_E5_CMD_COMMUNICATIONS_WRITING : uint16_t
{
    E_COMM_WRITE_OFF = 0, // 00: OFF (disabled)
    E_COMM_WRITE_ON  = 1  // 01: ON (enabled)
};

/**
 * @brief Defines the possible values for the RUN/STOP command (Code 01).
 * Controls the operational state of the controller.
 */
enum OR_E5_CMD_RUN_STOP : uint16_t
{
    E_RUN_MODE  = 0, // 00: Run
    E_STOP_MODE = 1  // 01: Stop (Changed name slightly to avoid conflict if E_STOP is defined elsewhere)
};

/**
 * @brief Defines the possible values for the Multi-SP command (Code 02).
 * Selects the active set point (SP). Also used for Multi-SP Monitor register (0x2404).
 */
enum OR_E5_CMD_MULTI_SP : uint16_t
{
    E_SET_POINT_0 = 0, // 00: Set point 0
    E_SET_POINT_1 = 1, // 01: Set point 1
    E_SET_POINT_2 = 2, // 02: Set point 2
    E_SET_POINT_3 = 3, // 03: Set point 3
    E_SET_POINT_4 = 4, // 04: Set point 4
    E_SET_POINT_5 = 5, // 05: Set point 5
    E_SET_POINT_6 = 6, // 06: Set point 6
    E_SET_POINT_7 = 7  // 07: Set point 7
};

/**
 * @brief Defines the possible values for the AT execute/cancel command (Code 03).
 * Controls the Auto-Tuning (AT) function.
 */
enum OR_E5_CMD_AT_EXECUTE_CANCEL : uint16_t
{
    E_AT_CANCEL         = 0, // 00: AT cancel
    E_AT_EXECUTE_100    = 1, // 01: 100% AT execute
    E_AT_EXECUTE_40     = 2  // 02: 40% AT execute
};

/**
 * @brief Defines the possible values for the Write mode command (Code 04).
 * Selects the destination for parameter writes.
 */
enum OR_E5_CMD_WRITE_MODE : uint16_t
{
    E_WRITE_MODE_BACKUP = 0, // 00: Backup (writes to non-volatile memory)
    E_WRITE_MODE_RAM    = 1  // 01: RAM write mode (writes to volatile memory)
};

// Note: Commands 05 (Save RAM data), 06 (Software reset), 07 (Move to setup area 1),
// 08 (Move to protect level), and 0B (Parameter initialization) use a fixed data value (00)
// to trigger the action. No specific data enum is needed for these commands themselves,
// only the command code itself is relevant.

/**
 * @brief Defines the possible values for the Auto/manual switch command (Code 09).
 * Switches the controller between automatic and manual control modes.
 */
enum OR_E5_CMD_AUTO_MANUAL_SWITCH : uint16_t
{
    E_AUTOMATIC_MODE = 0, // 00: Automatic mode
    E_MANUAL_MODE    = 1  // 01: Manual mode
};

/**
 * @brief Defines the possible values for the Alarm latch cancel command (Code 0C).
 * Clears latched alarm conditions.
 */
enum OR_E5_CMD_ALARM_LATCH_CANCEL : uint16_t
{
    E_ALARM_1_LATCH_CANCEL    = 0,  // 00: Alarm 1 latch cancel
    E_ALARM_2_LATCH_CANCEL    = 1,  // 01: Alarm 2 latch cancel
    E_ALARM_3_LATCH_CANCEL    = 2,  // 02: Alarm 3 latch cancel
    E_HB_ALARM_LATCH_CANCEL   = 3,  // 03: HB alarm latch cancel
    E_HS_ALARM_LATCH_CANCEL   = 4,  // 04: HS alarm latch cancel
    E_ALARM_4_LATCH_CANCEL    = 5,  // 05: Alarm 4 latch cancel
    E_ALL_ALARM_LATCH_CANCEL  = 15 // 0F: All alarm latch cancel
};

/**
 * @brief Defines the possible values for the SP Mode command (Code 0D).
 * Selects the source for the Set Point (SP).
 */
enum OR_E5_CMD_SP_MODE : uint16_t
{
    E_LOCAL_SP_MODE  = 0, // 00: Local SP Mode
    E_REMOTE_SP_MODE = 1  // 01: Remote SP Mode
};

/**
 * @brief Defines the possible values for the Invert direct/reverse operation command (Code 0E).
 * Sends a command to invert the current direct/reverse operation setting (register 0x2D12).
 * Note: This is a command, not the status/setting itself.
 */
enum OR_E5_CMD_INVERT_DIRECT_REVERSE : uint16_t
{
    E_NOT_INVERT = 0, // 00: Command does not invert the current setting.
    E_INVERT     = 1  // 01: Command inverts the current direct/reverse setting.
};

/**
 * @brief Defines the possible values for the Program start command (Code 11).
 * Controls the execution of a program pattern.
 */
enum OR_E5_CMD_PROGRAM_START : uint16_t
{
    E_PROGRAM_RESET = 0, // 00: Reset
    E_PROGRAM_START = 1  // 01: Start
};

/* Optional: Enum for Command Codes themselves
enum OR_E5_COMMAND_CODES : uint16_t
{
    E_CMD_COMMUNICATIONS_WRITING = 0x00,
    E_CMD_RUN_STOP = 0x01,
    E_CMD_MULTI_SP = 0x02,
    E_CMD_AT_EXECUTE_CANCEL = 0x03,
    E_CMD_WRITE_MODE = 0x04,
    E_CMD_SAVE_RAM_DATA = 0x05,
    E_CMD_SOFTWARE_RESET = 0x06,
    E_CMD_MOVE_TO_SETUP_AREA_1 = 0x07,
    E_CMD_MOVE_TO_PROTECT_LEVEL = 0x08, // Note: This command code writes to register 0x2504
    E_CMD_AUTO_MANUAL_SWITCH = 0x09,
    E_CMD_PARAMETER_INITIALIZATION = 0x0B,
    E_CMD_ALARM_LATCH_CANCEL = 0x0C,
    E_CMD_SP_MODE = 0x0D,
    E_CMD_INVERT_DIRECT_REVERSE = 0x0E,
    E_CMD_PROGRAM_START = 0x11
};
*/

// --------------------------------------------------------------------------
// --- Enums Generated/Merged from Original Code and H175 Manual Images ---
// --------------------------------------------------------------------------

/**
 * @brief Defines the Modbus register addresses for the Variable Area (Setting Range) List.
 * Merges registers from the provided initial code and the new images (H175 manual, including pg H175-5-15, H175-5-16).
 * Uses Two-byte mode addresses. Register names may vary slightly based on context (e.g., monitor vs. setting).
 * Levels indicated where known (e.g., IS=Initial Setting, AFS=Advanced Function Setting, ADJ=Adjustment, OP=Operation, Comm=Communication).
 * Note: 4-byte registers are handled as two consecutive 16-bit registers in standard Modbus reads,
 *       or a single 32-bit read if supported. Comments mention this. For settings requiring values
 *       larger than 16-bit (signed or unsigned), writing usually targets the low word address (e.g., `xxxx`).
 *       The controller appropriately handles the 4-byte representation internally.
 */
enum OR_E5_VARIABLE_REGISTERS : uint16_t
{
    // --- Block 1: Status/Monitor/Initial Setup --- (Primarily 0x20xx - OP Level Monitor)
    E_PV_REGISTER                 = 0x2000, // Process Value (OP Level Monitor). Range depends on sensor/analog input. Scaling: Lower limit - 5% FS to Upper limit + 5% FS. Uses 4 bytes. Read low word (or both).
    E_STATUS_1_REGISTER           = 0x2001, // Status 1 (OP Level Monitor). Refer to manual section 5-2 for bit details. (*2: rightmost 16 bits). See Placeholder Status Enums below.
    E_INTERNAL_SP_REGISTER        = 0x2002, // Internal Set Point (OP Level Monitor, *1 note in manual). Range: SP lower limit to SP upper limit. (*1: Not displayed on controller). Uses 4 bytes. Read low word (or both).
    E_HEATER_CURRENT_1_REGISTER   = 0x2003, // Heater Current 1 Value Monitor (OP Level Monitor). Range: 0.0 to 55.0 A (H'0000 to H'0226).
    E_MV_MONITOR_HEAT_REGISTER    = 0x2004, // MV Monitor (Heating) (OP Level Monitor). Range: Standard: -5.0 to 105.0 %; Heat/Cool: 0.0 to 105.0 %. Uses 4 bytes. Read low word (or both).
    E_MV_MONITOR_COOL_REGISTER    = 0x2005, // MV Monitor (Cooling) (OP Level Monitor). Range: 0.0 to 105.0 %. Uses 4 bytes. Read low word (or both).

    // --- Block 2: Alarm Related Settings --- (Primarily 0x21xx - Setup Level ?) -> These seem deprecated or less common? Compare to 0x29xx
    // Note: Addresses 0x2103-0x210F appear in some older manuals but might be superseded by 0x29xx in newer ones like H175. Retained for reference.
    E_SET_POINT_SETUP_REGISTER        = 0x2103, // Set Point (Possible Setup level?). Range: SP lower limit to SP upper limit. Uses 4 bytes (low word write). Compare 0x2601, 0x2900.
    E_ALARM_VALUE_1_SETUP_REGISTER    = 0x2104, // Alarm Value 1 Setting (Possible Setup level?). Range: -1999 to 9999. Units depend on input. Uses 4 bytes (low word write). Compare 0x2902.
    E_ALARM_1_UPPER_SETUP_REGISTER    = 0x2105, // Alarm Value Upper Limit 1 Setting (Possible Setup level?). Range: -1999 to 9999. Uses 4 bytes (low word write). Compare 0x2903.
    E_ALARM_1_LOWER_SETUP_REGISTER    = 0x2106, // Alarm Value Lower Limit 1 Setting (Possible Setup level?). Range: -1999 to 9999. Uses 4 bytes (low word write). Compare 0x2904.
    E_ALARM_VALUE_2_SETUP_REGISTER    = 0x2107, // Alarm Value 2 Setting (Possible Setup level?). Range: -1999 to 9999. Uses 4 bytes (low word write). Compare 0x2905.
    E_ALARM_2_UPPER_SETUP_REGISTER    = 0x2108, // Alarm Value Upper Limit 2 Setting (Possible Setup level?). Range: -1999 to 9999. Uses 4 bytes (low word write). Compare 0x2906.
    E_ALARM_2_LOWER_SETUP_REGISTER    = 0x2109, // Alarm Value Lower Limit 2 Setting (Possible Setup level?). Range: -1999 to 9999. Uses 4 bytes (low word write). Compare 0x2907.
    // Note: Setup level alarm settings 3/4 definitions not explicitly found here. See 0x29xx (ADJ) and 0x2Fxx (Type/Config).

    // --- Block 3: Monitoring / Status --- (Primarily 0x24xx - Operation Level Monitors)
    E_PV_MONITOR_REGISTER         = 0x2402, // Process Value (Monitor). Range depends on sensor/analog input. Same range as 0x2000. Uses 4 bytes. Read low word (or both).
    E_INTERNAL_SP_MONITOR_REGISTER= 0x2403, // Internal Set Point Monitor (*1 note in manual). Range: SP lower limit to SP upper limit. Same range as 0x2002. (*1: Not displayed). Uses 4 bytes. Read low word (or both).
    E_MULTI_SP_MONITOR_REGISTER   = 0x2404, // Multi-SP No. Monitor. Range: 0 to 7. Values correspond to OR_E5_CMD_MULTI_SP enum.
    E_STATUS_2_REGISTER           = 0x2406, // Status (Monitor). Refer to manual section 5-2 for bit details. (*2: rightmost 16 bits). See Placeholder Status Enums below.
    E_STATUS_3_REGISTER           = 0x2407, // Status (Monitor High Word). Refer to manual section 5-2 for bit details. (*3: leftmost 16 bits == high word of 4-byte Status). See Placeholder Status Enums below.
    E_STATUS2_1_REGISTER          = 0x2408, // Status 2 (Monitor). Refer to manual section 5-2. (*2: rightmost 16 bits). See Placeholder Status Enums below.
    E_STATUS2_2_REGISTER          = 0x2409, // Status 2 (Monitor High Word). Refer to manual section 5-2. (*3: leftmost 16 bits == high word of 4-byte Status 2). See Placeholder Status Enums below.
    E_DECIMAL_POINT_MONITOR_REGISTER = 0x2410, // Decimal Point Monitor. Range: 0 to 3. Values correspond to OR_E5_DECIMAL_POINT_POS enum. (Related to setting 0x2C0C).

    // --- Block 4: Protection / Operation Level --- (Primarily 0x25xx - Protect Level)
    E_OP_ADJUST_PROTECT_REGISTER    = 0x2500, // Operation/Adjustment Protect Setpoint. See OR_E5_PROTECT_LEVEL. Default: 0. Protect Level.
    E_INIT_COMM_PROTECT_REGISTER    = 0x2501, // Initial Setting/Communications Protect Setpoint. See OR_E5_INITIAL_PROTECT_LEVEL. Default: 0. Protect Level.
    E_SETTING_CHANGE_PROTECT_REGISTER = 0x2502, // Setting Change Protect setting. See OR_E5_SETTING_CHANGE_PROTECT. Default: 0. Protect Level.
    E_PF_KEY_PROTECT_REGISTER       = 0x2503, // PF Key Protect setting. See OR_E5_PF_KEY_PROTECT. Default: 0. Protect Level.
    E_MOVE_TO_PROTECT_LEVEL_REGISTER = 0x2504, // Move to Protect Level setting. Range: -1999 to 9999. Command 0x08 writes 0 here. Protect Level. Uses 4 bytes (two 16-bit regs). Write low word.
    E_PASSWORD_PROTECT_LEVEL_REGISTER = 0x2505, // Password to Move to Protect Level (Set only). Range: -1999 to 9999. Monitor value reads 0. Protect Level. Uses 4 bytes (two 16-bit regs). Write low word.
    E_PARAM_MASK_ENABLE_REGISTER    = 0x2506, // Parameter Mask Enable setting. See OR_E5_PARAM_MASK_ENABLE. Default: 0. Protect Level.
    E_CHANGED_PARAMS_ONLY_REGISTER  = 0x2507, // Changed Parameters Only setting. See OR_E5_CHANGED_PARAMS_ONLY. Default: 0. Protect Level.

    // --- Block 5: Manual Control / Operation Level --- (Primarily 0x26xx - OP Level)
    E_MANUAL_MV_REGISTER            = 0x2600, // Manual MV (Manipulated Value). Range depends on model/control type (Std/PosProp: -5.0 to 105.0%; Heat/Cool: -105.0 to 105.0%). Uses 4 bytes (two 16-bit registers). Write low word.
    E_OPERATION_SET_POINT_REGISTER  = 0x2601, // Set Point (Monitor - Current Operational SP value). Range: SP lower limit to SP upper limit. Compare 0x2103 (Setup?) and 0x29xx (SP0-7 Settings). Uses 4 bytes. Read low word (or both).
    E_REMOTE_SP_MONITOR_REGISTER    = 0x2602, // Remote SP Monitor. Range: Remote SP lower limit -10% FS to Remote SP upper limit +10% FS. Uses 4 bytes (two 16-bit registers). Read low word (or both).
    E_OPERATION_HEATER_CURRENT_1_REGISTER = 0x2604, // Heater Current 1 Value Monitor (OP Level Monitor). Range: 0.0 to 55.0 A. Compare 0x2003, 0x271A.
    E_OPERATION_MV_MONITOR_HEAT_REGISTER = 0x2605, // MV Monitor (Heating) (OP Level Monitor). Range: Std: -5.0 to 105.0 %; Heat/Cool: 0.0 to 105.0 %. Compare 0x2004. Uses 4 bytes. Read low word (or both).
    E_OPERATION_MV_MONITOR_COOL_REGISTER = 0x2606, // MV Monitor (Cooling) (OP Level Monitor). Range: 0.0 to 105.0 %. Compare 0x2005. Uses 4 bytes. Read low word (or both).
    E_VALVE_OPENING_MONITOR_REGISTER = 0x2607, // Valve Opening Monitor (Pos Prop Control). Range: -10.0 to 110.0 %. Uses 4 bytes. Read low word (or both).

    // --- Block 6: PID / Control Parameters / Ramps / Heater --- (Primarily 0x27xx - Mostly ADJ Level)
    E_PROP_BAND_COOL_REGISTER         = 0x2701, // Proportional Band (Cooling) (ADJ Level). Range: 0.1 to 999.9 %. Uses 4 bytes. Write low word.
    E_INTEGRAL_TIME_COOL_REGISTER     = 0x2702, // Integral Time (Cooling) (ADJ Level). Range: 0 to 9999 s or 0.0 to 999.9 s (see time unit 0x3309). Uses 4 bytes. Write low word.
    E_DERIVATIVE_TIME_COOL_REGISTER   = 0x2703, // Derivative Time (Cooling) (ADJ Level). Range: 0 to 9999 s or 0.0 to 999.9 s (see time unit 0x3309). Uses 4 bytes. Write low word.
    E_DEAD_BAND_REGISTER              = 0x2704, // Dead Band (ADJ Level). Range depends on input type (-199.9 to 999.9 or -19.99 to 99.99). Uses 4 bytes (two 16-bit registers). Write low word.
    E_MANUAL_RESET_VALUE_REGISTER     = 0x2705, // Manual Reset Value (ADJ Level). Range: 0.0 to 100.0 %. Uses 4 bytes. Write low word.
    E_HYSTERESIS_HEAT_REGISTER        = 0x2706, // Hysteresis (Heating) (ADJ Level). Range depends on input type (0.1 to 999.9 or 0.01 to 99.99). Stored as 1-9999. Uses 4 bytes. Write low word.
    E_HYSTERESIS_COOL_REGISTER        = 0x2707, // Hysteresis (Cooling) (ADJ Level). Range depends on input type (0.1 to 999.9 or 0.01 to 99.99). Stored as 1-9999. Uses 4 bytes. Write low word.
    E_CONTROL_PERIOD_HEAT_REGISTER    = 0x2708, // Control Period (Heating) (IS Level). See OR_E5_CONTROL_PERIOD_VALUES. Range: -2, -1, 0, 1..99. Signed. Uses potentially 4 bytes backend, write low word (int16_t).
    E_CONTROL_PERIOD_COOL_REGISTER    = 0x2709, // Control Period (Cooling) (IS Level). See OR_E5_CONTROL_PERIOD_VALUES. Range: -2, -1, 0, 1..99. Signed. Uses potentially 4 bytes backend, write low word (int16_t).
    E_POS_PROP_DEAD_BAND_REGISTER     = 0x270A, // Position Proportional Dead Band (ADJ Level). Range: 0.1 to 10.0 %. Uses 4 bytes. Write low word.
    E_OPEN_CLOSE_HYSTERESIS_REGISTER  = 0x270B, // Open/Close Hysteresis (Pos Prop) (ADJ Level). Range: 0.1 to 20.0 %. Uses 4 bytes. Write low word.
    E_SP_RAMP_TIME_UNIT_REGISTER      = 0x270C, // SP Ramp Time Unit (AFS Level). See OR_E5_SP_RAMP_TIME_UNIT. Range: 0 to 2. Default: 1. Uses 4 bytes. Write low word.
    E_SP_RAMP_SET_VALUE_REGISTER      = 0x270D, // SP Ramp Set Value (ADJ Level). Range: 0 (OFF), 1 to 9999. Unit defined by 0x270C. Default: 0. Uses 4 bytes. Write low word.
    E_SP_RAMP_FALL_VALUE_REGISTER     = 0x270E, // SP Ramp Fall Value (ADJ Level). Range: -1 (Same as Set), 0 (OFF), 1 to 9999. Unit defined by 0x270C. Default: 0. Signed. Uses 4 bytes (two 16-bit regs). Write low word.
    E_MV_AT_STOP_REGISTER             = 0x270F, // MV at Stop (IS Level). Range depends on model (Std: -5.0 to 105.0%). Uses 4 bytes (two 16-bit registers). Write low word.
    E_MV_AT_PV_ERROR_REGISTER         = 0x2711, // MV at PV Error (IS Level). Range depends on model/control type (Std: -5.0 to 105.0%; Heat/Cool: -105.0 to 105.0%). Uses 4 bytes (two 16-bit registers). Write low word.
    E_MV_CHANGE_RATE_LIMIT_REGISTER   = 0x2713, // MV Change Rate Limit (ADJ Level). Range: 0.0 (OFF) to 100.0 %/s. Uses 4 bytes. Write low word.
    E_PV_INPUT_SLOPE_COEFF_REGISTER   = 0x2718, // PV Input Slope Coefficient (ADJ Level). Range: 0.001 to 9.999. Used for scaling analog inputs. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALT_HEATER_CURRENT_1_MONITOR_REGISTER = 0x271A, // Heater Current 1 Value Monitor (OP Level Monitor). Range: 0.0 to 55.0 A. (Appears functionally same as 0x2003/0x2604).
    E_HEATER_BURNOUT_DETECT_1_REGISTER= 0x271B, // Heater Burnout Detection 1 Setpoint (ADJ Level). Range: 0.0 (OFF) to 50.0 A. Uses 4 bytes. Write low word.
    E_LEAKAGE_CURRENT_1_MONITOR_REGISTER = 0x271C, // Leakage Current 1 Monitor (OP Level Monitor). Range: 0.0 to 55.0 A.
    E_HS_ALARM_1_REGISTER             = 0x271D, // HS Alarm 1 (Heater Short) Setpoint (ADJ Level). Range: 0.0 (OFF) to 50.0 A. Uses 4 bytes. Write low word.
    E_PV_INPUT_SHIFT_REGISTER         = 0x2723, // Process Value Input Shift (Offset) (ADJ Level). Range: -1999 to 9999. Unit depends on input type. Uses 4 bytes (two 16-bit registers). Write low word.
    E_HEATER_CURRENT_2_MONITOR_REGISTER = 0x2724, // Heater Current 2 Value Monitor (OP Level Monitor). Range: 0.0 to 55.0 A.
    E_HEATER_BURNOUT_DETECT_2_REGISTER= 0x2725, // Heater Burnout Detection 2 Setpoint (ADJ Level). Range: 0.0 (OFF) to 50.0 A. Uses 4 bytes. Write low word.
    E_LEAKAGE_CURRENT_2_MONITOR_REGISTER = 0x2726, // Leakage Current 2 Monitor (OP Level Monitor). Range: 0.0 to 55.0 A.
    E_HS_ALARM_2_REGISTER             = 0x2727, // HS Alarm 2 (Heater Short) Setpoint (ADJ Level). Range: 0.0 (OFF) to 50.0 A. Uses 4 bytes. Write low word.
    E_SOAK_TIME_REMAIN_REGISTER       = 0x2728, // Soak Time Remain (OP Level Monitor). Range: 0 to 9999. Units set by 0x3327. Uses 4 bytes. Read low word (or both).
    E_SOAK_TIME_REGISTER              = 0x2729, // Soak Time (ADJ Level). Range: 1 to 9999. Units set by 0x3327. Uses 4 bytes. Write low word.
    E_WAIT_BAND_REGISTER              = 0x272A, // Wait Band (ADJ Level). Range: 0 (OFF), 0.1 to 999.9 (Temp) or 0.01 to 99.99 (Analog). Stored as 1-9999. Uses 4 bytes. Write low word.
    E_REMOTE_SP_INPUT_SHIFT_REGISTER  = 0x272B, // Remote SP Input Shift (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_REMOTE_SP_INPUT_SLOPE_COEFF_REGISTER = 0x272C, // Remote SP Input Slope Coefficient (ADJ Level). Range: 0.001 to 9.999. Uses 4 bytes (two 16-bit registers). Write low word.

    // --- Block 7: Advanced Parameters --- (Primarily 0x28xx - Mostly AFS Level)
    E_INPUT_DIGITAL_FILTER_REGISTER   = 0x2800, // Input Digital Filter (AFS Level). Range: 0.0 (OFF) to 999.9 s. Uses 4 bytes (two 16-bit registers). Write low word.
    E_MOVING_AVERAGE_COUNT_REGISTER   = 0x2804, // Moving Average Count (AFS Level). See OR_E5_MOVING_AVERAGE_COUNT. Range: 0 to 5. Default: 0. Uses 4 bytes. Write low word.
    E_SQRT_LOW_CUT_POINT_REGISTER     = 0x2808, // Extraction of Square Root Low-cut Point (ADJ Level). Range: 0.0 to 100.0 %. Uses 4 bytes (two 16-bit registers). Write low word.
    // Note: SQRT Enable register is 0x2E24 (was missing context)

    // --- Block 8: Operation Level Setpoints / Alarms --- (Primarily 0x29xx - ADJ Level Settings reflected in OP)
    E_SP_0_REGISTER                   = 0x2900, // Set Point 0 Setting (ADJ Level). Range: SP lower limit to SP upper limit. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_VALUE_1_OP_REGISTER       = 0x2902, // Alarm Value 1 (ADJ Level). Range: -1999 to 9999. Units depend on input. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_1_UPPER_OP_REGISTER       = 0x2903, // Alarm Value Upper Limit 1 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_1_LOWER_OP_REGISTER       = 0x2904, // Alarm Value Lower Limit 1 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_VALUE_2_OP_REGISTER       = 0x2905, // Alarm Value 2 (ADJ Level). Range: -1999 to 9999. Units depend on input. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_2_UPPER_OP_REGISTER       = 0x2906, // Alarm Value Upper Limit 2 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_2_LOWER_OP_REGISTER       = 0x2907, // Alarm Value Lower Limit 2 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_VALUE_3_OP_REGISTER       = 0x2908, // Alarm Value 3 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_3_UPPER_OP_REGISTER       = 0x2909, // Alarm Value Upper Limit 3 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_3_LOWER_OP_REGISTER       = 0x290A, // Alarm Value Lower Limit 3 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_VALUE_4_OP_REGISTER       = 0x290B, // Alarm Value 4 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_4_UPPER_OP_REGISTER       = 0x290C, // Alarm Value Upper Limit 4 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_ALARM_4_LOWER_OP_REGISTER       = 0x290D, // Alarm Value Lower Limit 4 (ADJ Level). Range: -1999 to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_SP_1_REGISTER                   = 0x290E, // Set Point 1 Setting (ADJ Level). Range: SP lower limit to SP upper limit. Uses 4 bytes (two 16-bit registers). Write low word.
    // Note: SP 2-7 follow every 14 (0xE) registers (e.g., SP2@0x290E+0xE = 0x291C)
    E_SP_2_REGISTER                   = 0x291C, // Set Point 2 Setting (ADJ Level). Range: SP lower limit to SP upper limit. Uses 4 bytes (two 16-bit registers). Write low word.
    E_SP_3_REGISTER                   = 0x292A, // Set Point 3 Setting (ADJ Level). Range: SP lower limit to SP upper limit. Uses 4 bytes (two 16-bit registers). Write low word.
    E_SP_4_REGISTER                   = 0x2938, // Set Point 4 Setting (ADJ Level). Range: SP lower limit to SP upper limit. Uses 4 bytes (two 16-bit registers). Write low word.
    E_SP_5_REGISTER                   = 0x2946, // Set Point 5 Setting (ADJ Level). Range: SP lower limit to SP upper limit. Uses 4 bytes (two 16-bit registers). Write low word.
    E_SP_6_REGISTER                   = 0x2954, // Set Point 6 Setting (ADJ Level). Range: SP lower limit to SP upper limit. Uses 4 bytes (two 16-bit registers). Write low word.
    E_SP_7_REGISTER                   = 0x2962, // Set Point 7 Setting (ADJ Level). Range: SP lower limit to SP upper limit. Uses 4 bytes (two 16-bit registers). Write low word.

    // --- Block 9: Adjustment Level PID / MV Limits --- (Primarily 0x2Axx - ADJ Level)
    E_PROP_BAND_HEAT_REGISTER         = 0x2A00, // Proportional Band (Heating/Primary) (ADJ Level). Range: 0.1 to 999.9 %. Uses 4 bytes. Write low word.
    E_INTEGRAL_TIME_HEAT_REGISTER     = 0x2A01, // Integral Time (Heating/Primary) (ADJ Level). Range: 0 to 9999 s or 0.0 to 999.9 s (see time unit 0x3309). Uses 4 bytes. Write low word.
    E_DERIVATIVE_TIME_HEAT_REGISTER   = 0x2A02, // Derivative Time D (Heating/Primary) (ADJ Level). Range: 0 to 9999 s or 0.0 to 999.9 s (see time unit 0x3309). Uses 4 bytes. Write low word.
    E_MV_UPPER_LIMIT_REGISTER         = 0x2A05, // MV Upper Limit (ADJ Level). Range: Std/PosProp: MV lower limit+0.1 to 105.0%; Heat/Cool: 0.0 to 105.0%. Uses 4 bytes. Write low word.
    E_MV_LOWER_LIMIT_REGISTER         = 0x2A06, // MV Lower Limit (ADJ Level). Range: Std/PosProp: -5.0 to MV upper limit-0.1%; Heat/Cool: -105.0 to 0.0 %. Signed. Uses 4 bytes (two 16-bit registers). Write low word.

    // --- Block 10: Initial/Advanced Settings --- (Primarily 0x2C/0x2D/0x2E - IS/AFS Level)
    E_INPUT_TYPE_REGISTER             = 0x2C00, // Input Type (IS Level). See OR_E5_INPUT_TYPE. Uses 4 bytes. Write low word.
    E_TEMPERATURE_UNIT_REGISTER       = 0x2C01, // Temperature Unit (IS Level). See OR_E5_TEMPERATURE_UNIT. Default: 0 (°C). Uses 4 bytes. Write low word.
    E_SCALING_LOWER_LIMIT_REGISTER    = 0x2C09, // Scaling Lower Limit (AFS Level). Range -1999 to (Upper Limit - 1). Uses 4 bytes (two 16-bit registers). Write low word.
    E_SCALING_UPPER_LIMIT_REGISTER    = 0x2C0B, // Scaling Upper Limit (AFS Level). Range (Lower Limit + 1) to 9999. Uses 4 bytes (two 16-bit registers). Write low word.
    E_DECIMAL_POINT_REGISTER          = 0x2C0C, // Decimal Point Setting (AFS Level). See OR_E5_DECIMAL_POINT_POS. Range: 0 H to 3 H. Compare monitor 0x2410. Uses 4 bytes. Write low word.
    E_REMOTE_SP_UPPER_LIMIT_REGISTER  = 0x2C0D, // Remote SP Upper Limit (AFS Level). Range depends on scaling. Uses 4 bytes (two 16-bit registers). Write low word.
    E_REMOTE_SP_LOWER_LIMIT_REGISTER  = 0x2C0E, // Remote SP Lower Limit (AFS Level). Range depends on scaling. Uses 4 bytes (two 16-bit registers). Write low word.
    E_PV_DECIMAL_POINT_DISPLAY_REGISTER = 0x2C0F, // PV Decimal Point Display (IS Level). See OR_E5_PV_DECIMAL_POINT_DISPLAY. Default: 0 (OFF). Uses 4 bytes. Write low word.

    E_CONTROL_OUTPUT_1_SIGNAL_REGISTER = 0x2D03, // Control Output 1 Signal (IS Level). See OR_E5_CONTROL_OUTPUT_SIGNAL. Default: 0 (4-20mA). Uses 4 bytes. Write low word.
    E_CONTROL_OUTPUT_2_SIGNAL_REGISTER = 0x2D04, // Control Output 2 Signal (IS Level). See OR_E5_CONTROL_OUTPUT_SIGNAL. Default: 0 (4-20mA). Uses 4 bytes. Write low word.
    E_SP_UPPER_LIMIT_REGISTER         = 0x2D0F, // SP Upper Limit (IS Level). Range depends on scaling. Uses 4 bytes (two 16-bit registers). Write low word.
    E_SP_LOWER_LIMIT_REGISTER         = 0x2D10, // SP Lower Limit (IS Level). Range depends on scaling. Uses 4 bytes (two 16-bit registers). Write low word.
    E_CONTROL_TYPE_REGISTER           = 0x2D11, // Standard or Heating/Cooling Control Type (IS Level). See OR_E5_CONTROL_TYPE. Default: 0 (Standard). Uses 4 bytes. Write low word.
    E_DIRECT_REVERSE_OP_REGISTER    = 0x2D12, // Direct/Reverse Operation (IS Level). See OR_E5_OPERATION_MODE enum. Default: 0 (Reverse). Uses 4 bytes. Write low word.
    E_CLOSE_FLOATING_REGISTER       = 0x2D13, // Close/Floating (Valve Mode) (IS Level). See OR_E5_CLOSE_FLOATING enum. Default: 1 (Close). Uses 4 bytes. Write low word.
    E_PID_ON_OFF_REGISTER           = 0x2D14, // PID ON/OFF (IS Level). See OR_E5_PID_ON_OFF enum. Default: 1 (PID). Uses 4 bytes. Write low word.
    E_ST_REGISTER                   = 0x2D15, // ST (Self Tuning) ON/OFF (IS Level). See OR_E5_ST enum. Default: 0 (OFF). Uses 4 bytes. Write low word.
    E_PROGRAM_PATTERN_REGISTER      = 0x2D16, // Program Pattern (IS Level). See OR_E5_PROGRAM_PATTERN enum. Default: 0 (OFF). Uses 4 bytes. Write low word.
    E_REMOTE_SP_INPUT_TYPE_REGISTER = 0x2D18, // Remote SP Input Type (AFS Level). See OR_E5_REMOTE_SP_INPUT_TYPE enum. Default: 0 (4-20mA). Uses 4 bytes. Write low word.
    E_MIN_OUTPUT_ONOFF_BAND_REGISTER= 0x2D19, // Minimum Output ON/OFF Band (AFS Level). Range: 0.0 to 50.0 (H'0000 to H'01F4). Default: 5.0. Uses 4 bytes. Write low word.

    E_TRANSFER_OUTPUT_TYPE_REGISTER = 0x2E00, // Transfer Output Type (IS Level). See OR_E5_TRANSFER_OUTPUT_TYPE enum. Default: 0 (OFF). Uses 4 bytes. Write low word.
    E_TRANSFER_OUTPUT_SIGNAL_REGISTER= 0x2E01, // Transfer Output Signal (IS Level). See OR_E5_TRANSFER_OUTPUT_SIGNAL enum. Default: 0 (4-20mA). Uses 4 bytes. Write low word.
    E_CONTROL_OUTPUT_1_ASSIGN_REGISTER = 0x2E06, // Control Output 1 Assignment (AFS Level). See OR_E5_CONTROL_OUTPUT_ASSIGNMENT. Range: 0 to 22, -1 to -5. Default: 1 (Control Heat). Uses potentially 4 bytes backend, write low word (int16_t).
    E_CONTROL_OUTPUT_2_ASSIGN_REGISTER = 0x2E07, // Control Output 2 Assignment (AFS Level). Range depends on output hardware (0-2 for current, 0-22 for others; also -1 to -5 for simple transfer). See OR_E5_CONTROL_OUTPUT_ASSIGNMENT. Default: 0 (None). Uses potentially 4 bytes backend, write low word (int16_t).

    // --- Block 11: Event Inputs / Aux Outputs / Transfer Limits / Alarms Config --- (Primarily 0x2E/0x2F - IS/AFS/Adv Level)
    E_EVENT_INPUT_ASSIGNMENT_1_REGISTER = 0x2E0A, // Event Input Assignment 1 (IS Level). See OR_E5_EVENT_INPUT_ASSIGNMENT. Default: 0 (None). Uses 4 bytes. Write low word.
    E_EVENT_INPUT_ASSIGNMENT_2_REGISTER = 0x2E0B, // Event Input Assignment 2 (IS Level). See OR_E5_EVENT_INPUT_ASSIGNMENT. Default: 0 (None). Uses 4 bytes. Write low word.
    E_EVENT_INPUT_ASSIGNMENT_3_REGISTER = 0x2E0C, // Event Input Assignment 3 (IS Level). See OR_E5_EVENT_INPUT_ASSIGNMENT. Default: 0 (None). Uses 4 bytes. Write low word.
    E_EVENT_INPUT_ASSIGNMENT_4_REGISTER = 0x2E0D, // Event Input Assignment 4 (IS Level). See OR_E5_EVENT_INPUT_ASSIGNMENT. Default: 0 (None). Uses 4 bytes. Write low word.
    E_EVENT_INPUT_ASSIGNMENT_5_REGISTER = 0x2E0E, // Event Input Assignment 5 (IS Level). See OR_E5_EVENT_INPUT_ASSIGNMENT. Default: 0 (None). Uses 4 bytes. Write low word.
    E_EVENT_INPUT_ASSIGNMENT_6_REGISTER = 0x2E0F, // Event Input Assignment 6 (IS Level). See OR_E5_EVENT_INPUT_ASSIGNMENT. Default: 0 (None). Uses 4 bytes. Write low word.
    E_AUXILIARY_OUTPUT_1_ASSIGN_REGISTER = 0x2E10, // Auxiliary Output 1 Assignment (AFS Level). See OR_E5_CONTROL_OUTPUT_ASSIGNMENT. Range: 0 to 22. Default: 0 (None). Uses 4 bytes. Write low word.
    E_AUXILIARY_OUTPUT_2_ASSIGN_REGISTER = 0x2E11, // Auxiliary Output 2 Assignment (AFS Level). See OR_E5_CONTROL_OUTPUT_ASSIGNMENT. Range: 0 to 22. Default: 0 (None). Uses 4 bytes. Write low word.
    E_AUXILIARY_OUTPUT_3_ASSIGN_REGISTER = 0x2E12, // Auxiliary Output 3 Assignment (AFS Level). See OR_E5_CONTROL_OUTPUT_ASSIGNMENT. Range: 0 to 22. Default: 0 (None). Uses 4 bytes. Write low word.
    E_AUXILIARY_OUTPUT_4_ASSIGN_REGISTER = 0x2E13, // Auxiliary Output 4 Assignment (AFS Level). See OR_E5_CONTROL_OUTPUT_ASSIGNMENT. Range: 0 to 22. Default: 0 (None). Uses 4 bytes. Write low word.
    E_TRANSFER_OUTPUT_UPPER_LIMIT_REGISTER = 0x2E14, // Transfer Output Upper Limit (IS Level). Range: -1999 to 9999. Units depend on input type. Uses 4 bytes (two 16-bit regs). Write low word.
    E_TRANSFER_OUTPUT_LOWER_LIMIT_REGISTER = 0x2E15, // Transfer Output Lower Limit (IS Level). Range: -1999 to 9999. Units depend on input type. Uses 4 bytes (two 16-bit regs). Write low word.
    E_SIMPLE_TRANSFER_OUTPUT_1_UPPER_LIMIT_REGISTER = 0x2E16, // Simple Transfer Output 1 Upper Limit (IS Level). Range: -1999 to 9999. Used with Simple Transfer Assignments (-1 to -5 in 0x2E06/0x2E07). Uses 4 bytes (two 16-bit regs). Write low word. (*1 Simple Transfer function only for specific models/outputs)
    E_SIMPLE_TRANSFER_OUTPUT_1_LOWER_LIMIT_REGISTER = 0x2E17, // Simple Transfer Output 1 Lower Limit (IS Level). Range: -1999 to 9999. Used with Simple Transfer Assignments (-1 to -5 in 0x2E06/0x2E07). Uses 4 bytes (two 16-bit regs). Write low word. (*1 Simple Transfer function only for specific models/outputs)
    E_SQRT_ENABLE_REGISTER              = 0x2E24, // Extraction of Square Root Enable (IS Level). See OR_E5_SQRT_ENABLE. Default: 0 (OFF). Uses 4 bytes. Write low word.
    E_TRAVEL_TIME_REGISTER              = 0x2E30, // Travel Time (Used for Valve Control?) (IS Level?). Range: 1 to 999. Default: ??? Uses 4 bytes. Write low word.

    E_ALARM_1_TYPE_REGISTER             = 0x2F00, // Alarm 1 Type (AFS Level). See OR_E5_ALARM_TYPE. Default: ??? Uses 4 bytes. Write low word.
    E_ALARM_1_LATCH_REGISTER            = 0x2F01, // Alarm 1 Latch (AFS Level). See OR_E5_ALARM_LATCH. Default: 0 (OFF). Uses 4 bytes. Write low word.
    E_ALARM_1_HYSTERESIS_REGISTER       = 0x2F02, // Alarm 1 Hysteresis (IS Level). Range: 0.1 to 999.9 (temp) or 0.01 to 99.99 (analog). Stored as 1-9999. Uses 4 bytes. Write low word.
    E_ALARM_2_TYPE_REGISTER             = 0x2F03, // Alarm 2 Type (AFS Level). See OR_E5_ALARM_TYPE (LBA cannot be set). Default: ??? Uses 4 bytes. Write low word.
    E_ALARM_2_LATCH_REGISTER            = 0x2F04, // Alarm 2 Latch (AFS Level). See OR_E5_ALARM_LATCH. Default: 0 (OFF). Uses 4 bytes. Write low word.
    E_ALARM_2_HYSTERESIS_REGISTER       = 0x2F05, // Alarm 2 Hysteresis (IS Level). Range: 0.1 to 999.9 (temp) or 0.01 to 99.99 (analog). Stored as 1-9999. Uses 4 bytes. Write low word.
    E_ALARM_3_TYPE_REGISTER             = 0x2F06, // Alarm 3 Type (AFS Level). See OR_E5_ALARM_TYPE (LBA cannot be set). Default: ??? Uses 4 bytes. Write low word.
    E_ALARM_3_LATCH_REGISTER            = 0x2F07, // Alarm 3 Latch (AFS Level). See OR_E5_ALARM_LATCH. Default: 0 (OFF). Uses 4 bytes. Write low word.
    E_ALARM_3_HYSTERESIS_REGISTER       = 0x2F08, // Alarm 3 Hysteresis (IS Level). Range: 0.1 to 999.9 (temp) or 0.01 to 99.99 (analog). Stored as 1-9999. Uses 4 bytes. Write low word.
    E_ALARM_4_TYPE_REGISTER             = 0x2F09, // Alarm 4 Type (AFS Level). See OR_E5_ALARM_TYPE (LBA cannot be set). Default: ??? Uses 4 bytes. Write low word.
    E_ALARM_4_LATCH_REGISTER            = 0x2F0A, // Alarm 4 Latch (AFS Level). See OR_E5_ALARM_LATCH. Default: 0 (OFF). Uses 4 bytes. Write low word.
    E_ALARM_4_HYSTERESIS_REGISTER       = 0x2F0B, // Alarm 4 Hysteresis (IS Level). Range: 0.1 to 999.9 (temp) or 0.01 to 99.99 (analog). Stored as 1-9999. Uses 4 bytes. Write low word.
    E_STANDBY_SEQUENCE_RESET_REGISTER   = 0x2F0C, // Standby Sequence Reset Condition (AFS Level). See OR_E5_STANDBY_SEQUENCE_RESET. Default: ??? Uses 4 bytes. Write low word.
    E_AUX_OUTPUT_1_OPEN_IN_ALARM_REGISTER = 0x2F0D, // Auxiliary Output 1 Open in Alarm (AFS Level). See OR_E5_AUX_OUTPUT_OPEN_IN_ALARM. Default: ??? Uses 4 bytes. Write low word.
    E_AUXILIARY_OUTPUT_2_OPEN_IN_ALARM_REGISTER = 0x2F0E, // Auxiliary Output 2 Open in Alarm (AFS Level). See OR_E5_AUX_OUTPUT_OPEN_IN_ALARM. Uses 4 bytes. Write low word.
    E_AUXILIARY_OUTPUT_3_OPEN_IN_ALARM_REGISTER = 0x2F10, // Auxiliary Output 3 Open in Alarm (AFS Level). See OR_E5_AUX_OUTPUT_OPEN_IN_ALARM. Uses 4 bytes. Write low word.
    E_AUXILIARY_OUTPUT_4_OPEN_IN_ALARM_REGISTER = 0x2F11, // Auxiliary Output 4 Open in Alarm (AFS Level). See OR_E5_AUX_OUTPUT_OPEN_IN_ALARM. Uses 4 bytes. Write low word.
    E_ALARM_1_ON_DELAY_REGISTER         = 0x2F12, // Alarm 1 ON Delay (AFS Level?). Range: 0 to 999 (seconds?). Uses 4 bytes. Write low word.
    E_ALARM_2_ON_DELAY_REGISTER         = 0x2F13, // Alarm 2 ON Delay (AFS Level?). Range: 0 to 999 (seconds?). Uses 4 bytes. Write low word.
    E_ALARM_3_ON_DELAY_REGISTER         = 0x2F14, // Alarm 3 ON Delay (AFS Level?). Range: 0 to 999 (seconds?). Uses 4 bytes. Write low word.
    E_ALARM_1_OFF_DELAY_REGISTER        = 0x2F15, // Alarm 1 OFF Delay (AFS Level?). Range: 0 to 999 (seconds?). Uses 4 bytes. Write low word.
    E_ALARM_2_OFF_DELAY_REGISTER        = 0x2F16, // Alarm 2 OFF Delay (AFS Level?). Range: 0 to 999 (seconds?). Uses 4 bytes. Write low word.
    E_ALARM_3_OFF_DELAY_REGISTER        = 0x2F17, // Alarm 3 OFF Delay (AFS Level?). Range: 0 to 999 (seconds?). Uses 4 bytes. Write low word.
    E_ALARM_4_OFF_DELAY_REGISTER        = 0x2F18, // Alarm 4 OFF Delay (AFS Level?). Range: 0 to 999 (seconds?). Uses 4 bytes. Write low word.
    // Note: Hysteresis range (0x2F02, 0x2F05, 0x2F08, 0x2F0B) is 1 to 9999 (representing 0.1-999.9 or 0.01-99.99) based on input type decimal point.

    // --- Block 12: Display / Miscellaneous --- (Primarily 0x30xx - Level varies)
    E_PV_SP_1_DISPLAY_SELECT_REGISTER   = 0x3000, // PV/SP No. 1 Display Selection (OP/ADJ Level?). See OR_E5_PV_SP_DISPLAY_SELECT. Uses 4 bytes. Write low word.
    E_MV_DISPLAY_SELECT_REGISTER        = 0x3001, // MV Display Selection (OP/ADJ Level?). See OR_E5_MV_DISPLAY_SELECT. Uses 4 bytes. Write low word.
    E_AUTO_DISPLAY_RETURN_TIME_REGISTER = 0x3003, // Automatic Display Return Time (Level?). Range: 0 (OFF) to 99 (seconds?). See OR_E5_AUTO_DISPLAY_RETURN. Uses 4 bytes. Write low word.
    E_DISPLAY_REFRESH_PERIOD_REGISTER   = 0x3004, // Display Refresh Period (Level?). See OR_E5_DISPLAY_REFRESH_PERIOD. Uses 4 bytes. Write low word.
    E_PV_SP_2_DISPLAY_SELECT_REGISTER   = 0x3008, // PV/SP No. 2 Display Selection (OP/ADJ Level?). Uses OR_E5_PV_SP_DISPLAY_SELECT. Uses 4 bytes. Write low word.
    E_DISPLAY_BRIGHTNESS_REGISTER       = 0x300A, // Display Brightness (Level?). Range: 1 to 3. Uses 4 bytes. Write low word.
    E_MV_DISPLAY_ENABLE_REGISTER        = 0x300B, // MV Display Enable (Level?). 0=OFF, 1=ON. See OR_E5_ON_OFF_STATE. Uses 4 bytes. Write low word.
    E_MOVE_TO_PROTECT_LEVEL_TIME_REGISTER= 0x300C, // Move to Protect Level Time (Level?). Range: 1 to 30 (minutes?). Uses 4 bytes. Write low word.
    E_AUTO_MANUAL_SELECT_ADDITION_REGISTER = 0x300F, // Auto/Manual Select Addition (Level?). 0=OFF, 1=ON. See OR_E5_ON_OFF_STATE. Uses 4 bytes. Write low word.
    E_PV_STATUS_DISPLAY_FUNC_REGISTER   = 0x3011, // PV Status Display Function (Level?). See OR_E5_STATUS_DISPLAY_FUNC. Uses 4 bytes. Write low word.
    E_SV_STATUS_DISPLAY_FUNC_REGISTER   = 0x3012, // SV Status Display Function (Level?). Uses OR_E5_STATUS_DISPLAY_FUNC. Uses 4 bytes. Write low word.

    // --- Block 13: Communications / Advanced Function Settings --- (Primarily 0x31xx, 0x32xx - Comm/AFS Level from Image)
    E_COMM_PROTOCOL_REGISTER           = 0x3100, // Protocol Setting (Comm Level). See OR_E5_COMM_PROTOCOL. Default: 1 (Modbus).
    E_COMM_UNIT_NO_REGISTER            = 0x3101, // Communications Unit No. (Comm Level). Range: 0 to 99. Default: ???
    E_COMM_BAUD_RATE_REGISTER          = 0x3102, // Communications Baud Rate (Comm Level). See OR_E5_COMM_BAUD_RATE. Default: ??? (often 3=9.6k).
    E_COMM_DATA_LENGTH_REGISTER        = 0x3103, // Communications Data Length (Comm Level). See OR_E5_COMM_DATA_LENGTH. Default: 8.
    E_COMM_STOP_BITS_REGISTER          = 0x3104, // Communications Stop Bits (Comm Level). See OR_E5_COMM_STOP_BITS. Default: 1.
    E_COMM_PARITY_REGISTER             = 0x3105, // Communications Parity (Comm Level). See OR_E5_COMM_PARITY. Default: 1 (Even?).
    E_COMM_SEND_DATA_WAIT_TIME_REGISTER = 0x3106, // Send Data Wait Time (Comm Level). Range: 0 to 99 (ms?). Default: ???
    E_PF_SETTING_REGISTER              = 0x3200, // PF (Programmable Function) Key Setting (AFS Level). See OR_E5_PF_SETTING. Default: 0 (Disabled).
    E_MONITOR_SETTING_ITEM_1_REGISTER  = 0x3202, // Monitor/Setting Item 1 (Associated display screen/parameter) (AFS Level). See OR_E5_MONITOR_SETTING_ITEM. Range: 0 to 23. Default: 0 (Disabled).
    E_MONITOR_SETTING_ITEM_2_REGISTER  = 0x3203, // Monitor/Setting Item 2 (Associated display screen/parameter) (AFS Level). See OR_E5_MONITOR_SETTING_ITEM. Range: 0 to 23. Default: 0 (Disabled).
    E_MONITOR_SETTING_ITEM_3_REGISTER  = 0x3204, // Monitor/Setting Item 3 (Associated display screen/parameter) (AFS Level). See OR_E5_MONITOR_SETTING_ITEM. Range: 0 to 23. Default: 0 (Disabled).
    E_MONITOR_SETTING_ITEM_4_REGISTER  = 0x3205, // Monitor/Setting Item 4 (Associated display screen/parameter) (AFS Level). See OR_E5_MONITOR_SETTING_ITEM. Range: 0 to 23. Default: 0 (Disabled).
    E_MONITOR_SETTING_ITEM_5_REGISTER  = 0x3206, // Monitor/Setting Item 5 (Associated display screen/parameter) (AFS Level). See OR_E5_MONITOR_SETTING_ITEM. Range: 0 to 23. Default: 0 (Disabled).

    // --- Block 14: Additional AFS Parameters --- (Primarily 0x33xx - AFS Level from Image H175-5-15 & 5-16) ---- UPDATED ----
    E_SP_TRACKING_REGISTER         = 0x3301, // SP Tracking (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_PV_DEAD_BAND_REGISTER        = 0x3304, // PV Dead Band (AFS Level). Range: 0 to 9999 (H'0000 to H'270F). Units/Decimal based on input. Uses 4 bytes. Write low word.
    E_COLD_JUNCTION_COMPENSATION_REGISTER = 0x3305, // Cold Junction Compensation Method (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_INTEGRAL_DERIVATIVE_TIME_UNIT_REGISTER = 0x3309, // Integral/Derivative Time Unit (AFS Level). See OR_E5_INTEGRAL_DERIVATIVE_TIME_UNIT. 0=1s, 1=0.1s. Uses 4 bytes. Write low word.
    E_MANUAL_OUTPUT_METHOD_REGISTER = 0x330A, // Manual Output Method (AFS Level). See OR_E5_MANUAL_OUTPUT_METHOD. 0=HOLD, 1=INIT. Uses 4 bytes. Write low word.
    E_MANUAL_MV_INITIAL_VALUE_REGISTER = 0x330C, // Manual MV Initial Value (AFS Level). Range depends on model/control type (-5.0 to 105.0% or -105.0 to 105.0%). Uses 4 bytes (two 16-bit registers). Write low word.
    E_AT_CALCULATED_GAIN_REGISTER  = 0x330F, // AT Calculated Gain (AFS Level). Range: 0.1 to 10.0 (H'0001 to H'0064). Uses 4 bytes. Write low word.
    E_AT_HYSTERESIS_REGISTER       = 0x3310, // AT Hysteresis (AFS Level). Range: 0.1 to 999.9 (Temp, H'0001 to H'270F) or 0.01 to 99.99 (Analog). Uses 4 bytes. Write low word.
    E_LIMIT_CYCLE_MV_AMPLITUDE_REGISTER = 0x3311, // Limit Cycle MV Amplitude (AFS Level). Range: 5.0 to 50.0 (H'0032 to H'01F4). Uses 4 bytes. Write low word.
    E_HEATER_BURNOUT_LATCH_REGISTER= 0x3314, // Heater Burnout Latch (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_HEATER_BURNOUT_HYSTERESIS_REGISTER = 0x3315, // Heater Burnout Hysteresis (AFS Level). Range: 0.1 to 50.0 (H'0001 to H'01F4). Uses 4 bytes. Write low word.
    E_HS_ALARM_LATCH_REGISTER      = 0x3316, // HS Alarm Latch (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_HS_ALARM_HYSTERESIS_REGISTER = 0x3317, // HS Alarm Hysteresis (AFS Level). Range: 0.1 to 50.0 (H'0001 to H'01F4). Uses 4 bytes. Write low word.
    E_NUMBER_OF_MULTI_SP_POINTS_REGISTER = 0x331B, // Number of Multi-SP Points (AFS Level). Range: 2 to 8 (H'0002 to H'0008). Uses 4 bytes. Write low word.
    E_HB_ON_OFF_REGISTER           = 0x331C, // HB ON/OFF (Heater Burnout Detection Enable) (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_INTEGRATED_ALARM_ASSIGNMENT_REGISTER = 0x331E, // Integrated Alarm Assignment (AFS Level). Bitmask? Range: 0 to 255 (H'00 to H'FF). Uses 4 bytes. Write low word.
    E_MV_AT_STOP_ERROR_ADDITION_REGISTER = 0x3320, // MV at Stop and Error Addition (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_ST_STABLE_RANGE_REGISTER    = 0x3321, // ST Stable Range (Self Tuning) (AFS Level). Range: 0.1 to 999.9 (H'0001 to H'270F). Uses 4 bytes. Write low word.
    E_RT_ENABLE_REGISTER          = 0x3322, // RT (Resistance Thermometer?) ON/OFF (AFS Level). See OR_E5_ON_OFF_STATE. Note: Valid only with temp input. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_HS_ALARM_USE_REGISTER       = 0x3323, // HS Alarm Use (Heater Short Detection Enable) (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_LBA_DETECTION_TIME_REGISTER = 0x3324, // LBA Detection Time (Loop Break Alarm) (AFS Level). Range: 0 to 9999 s (H'0001 to H'270F). Uses 4 bytes. Write low word.
    E_LBA_LEVEL_REGISTER          = 0x3325, // LBA Level (AFS Level). Range: 0.1 to 999.9 (Temp, H'0001 to H'270F) or 0.01 to 99.99 (Analog). Uses 4 bytes. Write low word.
    E_LBA_BAND_REGISTER           = 0x3326, // LBA Band (AFS Level). Range: 0.0 to 999.9 (Temp, H'0000 to H'270F) or 0.00 to 99.99 (Analog). Uses 4 bytes. Write low word.
    E_SOAK_TIME_UNIT_REGISTER     = 0x3327, // Soak Time Unit (AFS Level). See OR_E5_SOAK_TIME_UNIT. Range 0-2. Uses 4 bytes. Write low word.
    E_ALARM_SP_SELECTION_REGISTER = 0x3328, // Alarm SP Selection (AFS Level). See OR_E5_ALARM_SP_SELECTION. 0=RampSP, 1=FinalSP. Uses 4 bytes. Write low word.
    E_REMOTE_SP_ENABLE_REGISTER   = 0x3329, // Remote SP Enable (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_MANUAL_MV_LIMIT_ENABLE_REGISTER = 0x332B, // Manual MV Limit Enable (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_DIRECT_SETTING_POS_PROP_MV_REGISTER = 0x332C, // Direct Setting of Position Proportional MV (AFS Level). See OR_E5_ON_OFF_STATE. 0=OFF, 1=ON. Uses 4 bytes. Write low word.
    E_PV_RATE_OF_CHANGE_CALC_PERIOD_REGISTER = 0x332D, // PV Rate of Change Calculation Period (AFS Level). Range: 1 to 999 (H'0001 to H'03E7). Units?. Uses 4 bytes. Write low word.
    E_HEATING_COOLING_TUNING_METHOD_REGISTER = 0x332E, // Heating/Cooling Tuning Method (AFS Level). See OR_E5_HEATING_COOLING_TUNING_METHOD. Uses 4 bytes. Write low word.
    E_LCT_COOLING_OUTPUT_MIN_ON_TIME_REGISTER = 0x3335, // LCT Cooling Output Minimum ON Time (AFS Level). Range: 0.1 to 1.0 (stored as H'0001 to H'000A). Note: Not supported on v2.0 or earlier. Uses 4 bytes. Write low word.

};


// --- Existing & Merged Value Enums ---

/**
 * @brief Defines a generic ON/OFF state for various registers.
 */
enum OR_E5_ON_OFF_STATE : uint16_t
{
    E_OFF = 0, // State is OFF or Disabled
    E_ON = 1   // State is ON or Enabled
};

/**
 * @brief Defines the possible values for the Operation/Adjustment Protect register (0x2500).
 * Sets the protection level for operation and adjustment parameters. Protect Level.
 */
enum OR_E5_PROTECT_LEVEL : uint16_t
{
    E_PROTECT_NONE              = 0, // 0: No restrictions in operation and adjustment levels (Default)
    E_PROTECT_ADJUSTMENT_LEVEL  = 1, // 1: Move to adjustment level is prohibited
    E_PROTECT_PV_SP_ONLY        = 2, // 2: Display and change of only "PV" and "PV/SP" parameters is allowed
    E_PROTECT_PV_PVSP_DISPLAY   = 3  // 3: Display of only "PV" and "PV/SP" parameters is allowed
};

/**
 * @brief Defines the possible values for the Initial Setting/Communications Protect register (0x2501).
 * Protects initial settings and communication parameters. Protect Level.
 */
enum OR_E5_INITIAL_PROTECT_LEVEL : uint16_t
{
    E_INIT_PROTECT_ALLOWED     = 0, // 0: Move to IS/Comm setting level allowed. (Move to AFS level allowed.) (Default)
    E_INIT_PROTECT_AFS_NOT_DISPLAYED = 1, // 1: Move to IS/Comm setting level allowed. (Move to AFS level not displayed.)
    E_INIT_PROTECT_PROHIBITED  = 2  // 2: Move to IS/Comm setting level prohibited.
};

/**
 * @brief Defines the possible values for the Setting Change Protect register (0x2502).
 * Enables or disables changing settings via the front panel. Protect Level.
 */
enum OR_E5_SETTING_CHANGE_PROTECT : uint16_t
{
    E_SETTING_CHANGE_ALLOWED    = E_OFF, // 0: OFF (Changing settings on controller display is allowed.) (Default)
    E_SETTING_CHANGE_PROHIBITED = E_ON   // 1: ON (Changing settings on controller display is prohibited.)
};

/**
 * @brief Defines the possible values for the PF Key Protect register (0x2503).
 * Enables or disables the Programmable Function (PF) key. Protect Level.
 */
enum OR_E5_PF_KEY_PROTECT : uint16_t
{
    E_PF_KEY_ENABLED  = E_OFF, // 0: OFF (PF Key operation allowed) (Default)
    E_PF_KEY_DISABLED = E_ON   // 1: ON (PF Key operation prohibited)
};

/**
 * @brief Defines the possible values for the Parameter Mask Enable register (0x2506).
 * Enables or disables the parameter mask function. Protect Level.
 */
enum OR_E5_PARAM_MASK_ENABLE : uint16_t
{
    E_PARAM_MASK_DISABLED = E_OFF, // 0: OFF (Default)
    E_PARAM_MASK_ENABLED  = E_ON   // 1: ON
};

/**
 * @brief Defines the possible values for the Changed Parameters Only register (0x2507).
 * Determines if only changed parameters are displayed or written. Protect Level.
 */
enum OR_E5_CHANGED_PARAMS_ONLY : uint16_t
{
    E_CHANGED_PARAMS_OFF = E_OFF, // 0: OFF (Default behavior) (Default)
    E_CHANGED_PARAMS_ON  = E_ON   // 1: ON (Only show/write changed parameters)
};

/**
 * @brief Defines the possible values for the Decimal Point registers (Setting 0x2C0C, Monitor 0x2410).
 * Indicates the number of digits displayed after the decimal point for PV, SP etc.
 * Level: Advanced Function Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_DECIMAL_POINT_POS : uint16_t // Values fit in 16 bits, matches write target
{
    E_DECIMAL_POS_0 = 0, // 0: No decimal point (e.g., 123)
    E_DECIMAL_POS_1 = 1, // 1: One decimal place (e.g., 12.3)
    E_DECIMAL_POS_2 = 2, // 2: Two decimal places (e.g., 1.23)
    E_DECIMAL_POS_3 = 3  // 3: Three decimal places (e.g., 0.123)
    // Default depends on Input Type
};

/**
 * @brief Defines the possible values for Control Period registers (Heating 0x2708, Cooling 0x2709).
 * Represents fixed time periods or a range. Values H'FFFE and H'FFFF interpreted as signed -2 and -1 respectively.
 * Level: Initial Setting.
 */
enum OR_E5_CONTROL_PERIOD_VALUES : int16_t // Use signed type due to negative representations
{
    // Standard fixed periods
    E_PERIOD_0_1_SEC = -2, // H'FFFE: 0.1 seconds
    E_PERIOD_0_2_SEC = -1, // H'FFFF: 0.2 seconds
    E_PERIOD_0_5_SEC = 0,  // H'0000: 0.5 seconds
    // 1 to 99 (H'0001 to H'0063): Represents 1 to 99 seconds directly
    E_PERIOD_1_TO_99_SEC_MIN = 1,  // Minimum value for 1-99 second range
    E_PERIOD_1_TO_99_SEC_MAX = 99  // Maximum value for 1-99 second range
    // Default depends on model/output type
};

/**
 * @brief Defines the possible values for the SP Ramp Time Unit register (0x270C).
 * Sets the time base for SP ramp rates. Level: Advanced Function Setting.
 */
enum OR_E5_SP_RAMP_TIME_UNIT : uint16_t
{
    E_UNIT_EU_PER_SECOND = 0, // 0: EU/second
    E_UNIT_EU_PER_MINUTE = 1, // 1: EU/minute (Default)
    E_UNIT_EU_PER_HOUR   = 2  // 2: EU/hour
    // EU = Engineering Units (depends on input type, e.g., °C, °F, %)
};

/**
 * @brief Defines the possible values for the Moving Average Count register (0x2804).
 * Sets the number of samples for the PV moving average calculation. Level: Advanced Function Setting.
 * Uses 4 bytes, write low word.
 */
enum OR_E5_MOVING_AVERAGE_COUNT : uint16_t // Values fit in 16 bits, matches write target
{
    E_MOVING_AVG_OFF     = 0, // 00: OFF (No moving average) (Default)
    E_MOVING_AVG_2_TIMES = 1, // 01: 2 times
    E_MOVING_AVG_4_TIMES = 2, // 02: 4 times
    E_MOVING_AVG_8_TIMES = 3, // 03: 8 times
    E_MOVING_AVG_16_TIMES= 4, // 04: 16 times
    E_MOVING_AVG_32_TIMES= 5  // 05: 32 times
};

/**
 * @brief Defines the possible values for the Input Type register (0x2C00).
 * Selects the sensor or input signal type. Level: Initial Setting.
 * Note: Values are based on H175 manual, specific ranges may vary. Uses 4 bytes, write low word.
 */
enum OR_E5_INPUT_TYPE : uint16_t // Values fit in 16 bits, matches write target
{
    E_INPUT_PT_MINUS_200_TO_850C     = 0,  // Pt (-200 to 850°C / -300 to 1500°F)
    E_INPUT_PT_MINUS_199_9_TO_500_0C = 1,  // Pt (-199.9 to 500.0°C / -199.9 to 900.0°F)
    E_INPUT_PT_0_0_TO_100_0C         = 2,  // Pt (0.0 to 100.0°C / 0.0 to 210.0°F)
    E_INPUT_JPT_MINUS_199_9_TO_500_0C= 3,  // JPt (-199.9 to 500.0°C / -199.9 to 900.0°F)
    E_INPUT_JPT_0_0_TO_100_0C        = 4,  // JPt (0.0 to 100.0°C / 0.0 to 210.0°F)
    E_INPUT_K_MINUS_200_TO_1300C     = 5,  // K (-200 to 1300°C / -300 to 2300°F) (Default for Thermocouple models)
    E_INPUT_K_MINUS_20_0_TO_500_0C   = 6,  // K (-20.0 to 500.0°C / 0.0 to 900.0°F)
    E_INPUT_J_MINUS_100_TO_850C      = 7,  // J (-100 to 850°C / -100 to 1500°F)
    E_INPUT_J_MINUS_20_0_TO_400_0C   = 8,  // J (-20.0 to 400.0°C / 0.0 to 750.0°F)
    E_INPUT_T_MINUS_200_TO_400C      = 9,  // T (-200 to 400°C / -300 to 700°F)
    E_INPUT_T_MINUS_199_9_TO_400_0C  = 10, // T (-199.9 to 400.0°C / -199.9 to 700.0°F)
    E_INPUT_E_MINUS_200_TO_600C      = 11, // E (-200 to 600°C / -300 to 1100°F)
    E_INPUT_L_MINUS_100_TO_850C      = 12, // L (-100 to 850°C / -100 to 1500°F)
    E_INPUT_U_MINUS_200_TO_400C      = 13, // U (-200 to 400°C / -300 to 700.0°F)
    E_INPUT_U_MINUS_199_9_TO_400_0C  = 14, // U (-199.9 to 400.0°C / -199.9 to 700.0°F)
    E_INPUT_N_MINUS_200_TO_1300C     = 15, // N (-200 to 1300°C / -300 to 2300°F)
    E_INPUT_R_0_TO_1700C             = 16, // R (0 to 1700°C / 0 to 3000°F)
    E_INPUT_S_0_TO_1700C             = 17, // S (0 to 1700°C / 0 to 3000°F)
    E_INPUT_B_100_TO_1800C           = 18, // B (100 to 1800°C / 300 to 3200°F)
    E_INPUT_W_0_TO_2300C             = 19, // W (WRe5-26) (0 to 2,300°C / 0 to 3,200°F)
    E_INPUT_PLII_0_TO_1300C          = 20, // PLII (0 to 1,300°C / 0 to 2,300°F)

    E_INPUT_IR_SENSOR_K_140F_60C     = 21, // Infrared temperature sensor (K 140°F/60°C)
    E_INPUT_IR_SENSOR_K_240F_120C    = 22, // Infrared temperature sensor (K 240°F/120°C)
    E_INPUT_IR_SENSOR_K_280F_140C    = 23, // Infrared temperature sensor (K 280°F/140°C)
    E_INPUT_IR_SENSOR_K_440F_220C    = 24, // Infrared temperature sensor (K 440°F/220°C)

    E_INPUT_ANALOG_4_TO_20_MA    = 25, // Analog Input: 4 to 20 mA (Default for Analog I/P models)
    E_INPUT_ANALOG_0_TO_20_MA    = 26, // Analog Input: 0 to 20 mA
    E_INPUT_ANALOG_1_TO_5_V      = 27, // Analog Input: 1 to 5 V
    E_INPUT_ANALOG_0_TO_5_V      = 28, // Analog Input: 0 to 5 V
    E_INPUT_ANALOG_0_TO_10_V     = 29, // Analog Input: 0 to 10 V
    E_INPUT_ANALOG_0_TO_50_MV    = 30  // Analog Input: 0 to 50 mV (* Selection possible only for E5CC-U v2.2+ May '14 or later)
    // Default depends on controller model (e.g., E5CC-QQ is K, E5CC-AQ is 4-20mA)
};

/**
 * @brief Defines the possible values for the Temperature Unit register (0x2C01).
 * Selects Celsius or Fahrenheit display. Level: Initial Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_TEMPERATURE_UNIT : uint16_t // Values fit in 16 bits, matches write target
{
    E_UNIT_CELSIUS    = 0, // 0: °C (Default)
    E_UNIT_FAHRENHEIT = 1  // 1: °F
};

/**
 * @brief Defines the possible values for PV Decimal Point Display register (0x2C0F).
 * Enables/disables showing the decimal point in PV display. Level: Initial Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_PV_DECIMAL_POINT_DISPLAY : uint16_t // Values fit in 16 bits, matches write target
{
    E_PV_DECIMAL_DISPLAY_OFF = E_OFF, // 0: OFF (Decimal point determined by input type/setting is not shown) (Default)
    E_PV_DECIMAL_DISPLAY_ON  = E_ON   // 1: ON (Decimal point is shown according to setting 0x2C0C)
};

/**
 * @brief Defines the possible values for Control Output Signal registers (Output 1: 0x2D03, Output 2: 0x2D04).
 * Selects the range for analog current outputs. Level: Initial Setting. Uses 4 bytes, write low word.
 * Only applicable if the corresponding output is a current output.
 */
enum OR_E5_CONTROL_OUTPUT_SIGNAL : uint16_t // Values fit in 16 bits, matches write target
{
    E_OUTPUT_SIGNAL_4_TO_20_MA = 0, // 0: 4 to 20 mA (Default)
    E_OUTPUT_SIGNAL_0_TO_20_MA = 1  // 1: 0 to 20 mA
};

/**
 * @brief Defines the possible values for the Control Type register (0x2D11).
 * Selects standard (heat OR cool) or heating/cooling control. Level: Initial Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_CONTROL_TYPE : uint16_t // Values fit in 16 bits, matches write target
{
    E_CONTROL_STANDARD      = 0, // 0: Standard (PID or ON/OFF, single output - heating or cooling based on 0x2D12) (Default)
    E_CONTROL_HEAT_AND_COOL = 1  // 1: Heating and cooling (Requires appropriate model/outputs)
};

/**
 * @brief Defines the possible values for the Direct/Reverse Operation register (0x2D12).
 * Sets the control action direction. Level: Initial Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_OPERATION_MODE : uint16_t // Values fit in 16 bits, matches write target
{
    E_OPERATION_REVERSE = 0, // 0: Reverse operation (Increase output on PV < SP, e.g., Heating) (Default)
    E_OPERATION_DIRECT  = 1  // 1: Direct operation (Increase output on PV > SP, e.g., Cooling)
};

/**
 * @brief Defines the possible values for the Close/Floating register (0x2D13).
 * Used for valve control (Position Proportional Mode). Level: Initial Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_CLOSE_FLOATING : uint16_t // Values fit in 16 bits, matches write target
{
    E_VALVE_FLOATING = 0, // 0: Floating
    E_VALVE_CLOSE    = 1  // 1: Close (Default)
};

/**
 * @brief Defines the possible values for the PID ON/OFF register (0x2D14).
 * Selects between ON/OFF control and PID control. Level: Initial Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_PID_ON_OFF : uint16_t // Values fit in 16 bits, matches write target
{
    E_CONTROL_ON_OFF = 0, // 0: ON/OFF control
    E_CONTROL_2_PID  = 1  // 1: 2-PID control (Default)
};

/**
 * @brief Defines the possible values for the ST (Self-Tuning) register (0x2D15).
 * Enables or disables Self-Tuning. Level: Initial Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_ST : uint16_t // Values fit in 16 bits, matches write target
{
    E_ST_OFF = E_OFF, // 0: OFF (Default)
    E_ST_ON  = E_ON   // 1: ON
};

/**
 * @brief Defines the possible values for the Program Pattern register (0x2D16).
 * Controls the program pattern execution state. Level: Initial Setting. Uses 4 bytes, write low word.
 */
enum OR_E5_PROGRAM_PATTERN : uint16_t // Values fit in 16 bits, matches write target
{
    E_PROGRAM_PATTERN_OFF  = 0, // 0: OFF (Default)
    E_PROGRAM_PATTERN_STOP = 1, // 1: STOP
    E_PROGRAM_PATTERN_CONT = 2  // 2: CONT (Continue/Run)
};

/**
 * @brief Defines the possible values for the Remote SP Input Type register (0x2D18).
 * Selects the signal type for remote setpoint input. Level: Advanced Function Setting. Uses 4 bytes, write low word.
 * Only applicable if the model supports remote SP input.
 */
enum OR_E5_REMOTE_SP_INPUT_TYPE : uint16_t // Values fit in 16 bits, matches write target
{
    E_REMOTE_INPUT_4_TO_20_MA = 0, // 0: 4 to 20 mA (Default)
    E_REMOTE_INPUT_0_TO_20_MA = 1, // 1: 0 to 20 mA
    E_REMOTE_INPUT_1_TO_5_V   = 2, // 2: 1 to 5 V
    E_REMOTE_INPUT_0_TO_5_V   = 3, // 3: 0 to 5 V
    E_REMOTE_INPUT_0_TO_10_V  = 4  // 4: 0 to 10 V
};

/**
 * @brief Defines the possible values for the Transfer Output Type register (0x2E00).
 * Selects the variable to be output via the transfer output. Level: Initial Setting. Uses 4 bytes, write low word.
 * Only applicable for models with a Transfer Output.
 */
enum OR_E5_TRANSFER_OUTPUT_TYPE : uint16_t // Values fit in 16 bits, matches write target
{
    E_TRANSFER_OFF             = 0, // 0: OFF (Default)
    E_TRANSFER_SET_POINT       = 1, // 1: Set point
    E_TRANSFER_SP_DURING_RAMP  = 2, // 2: Set point during SP ramp
    E_TRANSFER_PV              = 3, // 3: PV (Process Value)
    E_TRANSFER_MV_HEATING      = 4, // 4: MV (heating)
    E_TRANSFER_MV_COOLING      = 5, // 5: MV (cooling)
    E_TRANSFER_VALVE_OPENING   = 6  // 6: Valve opening (*Only for Position-proportional Models.)
};

/**
 * @brief Defines the possible values for the Transfer Output Signal register (0x2E01).
 * Selects the analog signal range for the transfer output. Level: Initial Setting. Uses 4 bytes, write low word.
 * Only applicable for models with a Transfer Output.
 */
enum OR_E5_TRANSFER_OUTPUT_SIGNAL : uint16_t // Values fit in 16 bits, matches write target
{
    E_TRANSFER_SIGNAL_4_TO_20_MA = 0, // 0: 4 to 20 mA (Default)
    E_TRANSFER_SIGNAL_1_TO_5_V   = 1  // 1: 1 to 5 V
};

/**
 * @brief Defines the possible values for the Control Output Assignment registers (Output 1: 0x2E06, Output 2: 0x2E07)
 *        and Auxiliary Output Assignment registers (Aux 1: 0x2E10, Aux 2: 0x2E11, Aux 3: 0x2E12, Aux 4: 0x2E13).
 * Assigns a function to the physical control/auxiliary outputs. Level: Advanced Function Setting.
 * Availability of some options depends on hardware configuration (e.g., linear current output).
 * Uses potentially 4 bytes per register backend, write low word (int16_t).
 */
enum OR_E5_CONTROL_OUTPUT_ASSIGNMENT : int16_t // Use signed type for negative values. Values fit in 16 bits, matches write target.
{
    // Standard Positive Assignments (0 to 22)
    E_OUTPUT_ASSIGN_NONE            = 0,  // 0: Not assigned (Default for Aux Outputs and some Control Output 2)
    E_OUTPUT_ASSIGN_CONTROL_HEAT    = 1,  // 1: Control output (heating) (Default for Control Output 1)
    E_OUTPUT_ASSIGN_CONTROL_COOL    = 2,  // 2: Control output (cooling)
    E_OUTPUT_ASSIGN_ALARM_1         = 3,  // 3: Alarm 1 output
    E_OUTPUT_ASSIGN_ALARM_2         = 4,  // 4: Alarm 2 output
    E_OUTPUT_ASSIGN_ALARM_3         = 5,  // 5: Alarm 3 output
    E_OUTPUT_ASSIGN_ALARM_4         = 6,  // 6: Alarm 4 output
    E_OUTPUT_ASSIGN_HEATER_ALARM    = 7,  // 7: Heater alarm (Combined HB or HS - check model/config?)
    E_OUTPUT_ASSIGN_HB_ALARM        = 8,  // 8: HB alarm (Heater Burnout)
    E_OUTPUT_ASSIGN_HS_ALARM        = 9,  // 9: HS alarm (SSR Short)
    E_OUTPUT_ASSIGN_INPUT_ERROR     = 10, // 10: Input error output
    E_OUTPUT_ASSIGN_RSP_INPUT_ERROR = 11, // 11: RSP input error output (Remote SP)
    E_OUTPUT_ASSIGN_PROGRAM_END     = 12, // 12: Program end output (P.END) (*1)
    E_OUTPUT_ASSIGN_RUN             = 13, // 13: RUN output
    E_OUTPUT_ASSIGN_INTEGRATED_ALARM = 14, // 14: Integrated alarm output
    E_OUTPUT_ASSIGN_WORK_BIT_1      = 15, // 15: Work bit 1 output
    E_OUTPUT_ASSIGN_WORK_BIT_2      = 16, // 16: Work bit 2 output
    E_OUTPUT_ASSIGN_WORK_BIT_3      = 17, // 17: Work bit 3 output
    E_OUTPUT_ASSIGN_WORK_BIT_4      = 18, // 18: Work bit 4 output
    E_OUTPUT_ASSIGN_WORK_BIT_5      = 19, // 19: Work bit 5 output
    E_OUTPUT_ASSIGN_WORK_BIT_6      = 20, // 20: Work bit 6 output
    E_OUTPUT_ASSIGN_WORK_BIT_7      = 21, // 21: Work bit 7 output
    E_OUTPUT_ASSIGN_WORK_BIT_8      = 22, // 22: Work bit 8 output

    // Special Negative Assignments for Linear Current Output (*2)
    E_OUTPUT_ASSIGN_SIMPLE_TRANSFER_MV_COOL = -5, // H'FFFF FFFB: Simple transfer MV (cooling) (*2)
    E_OUTPUT_ASSIGN_SIMPLE_TRANSFER_MV_HEAT = -4, // H'FFFF FFFC: Simple transfer MV (heating) (*2)
    E_OUTPUT_ASSIGN_SIMPLE_TRANSFER_PV      = -3, // H'FFFF FFFD: Simple transfer PV (*2)
    E_OUTPUT_ASSIGN_SIMPLE_TRANSFER_RAMP_SP = -2, // H'FFFF FFFE: Simple transfer ramp SP (*2)
    E_OUTPUT_ASSIGN_SIMPLE_TRANSFER_SP      = -1  // H'FFFF FFFF: Simple transfer SP (*2)

    // Notes from manual:
    // (*1) P.END (program end output) can be set even when the program pattern is set to OFF, but the function will be disabled.
    // (*2) Selection is possible only with the E5CC-U and E5GC and only when there is a control output that is a linear current output.
    //      (For E5CC-U, the Controller must have been manufactured in May 2014 or later (version 2.2).).
    // Note for Control Output 2: When Control output 2 is a linear current output, only values 0 to 2 are permitted according to H175 p5-9.
    // Check manual for restrictions on Auxiliary Outputs (may not support negative values or full positive range).
};

/**
 * @brief Defines the possible functions assigned to Event Inputs (Registers 0x2E0A to 0x2E0F).
 * Level: Initial Setting. Uses 4 bytes per register, write low word.
 * Only applicable if the model has Event Inputs.
 */
enum OR_E5_EVENT_INPUT_ASSIGNMENT : uint16_t // Values fit in 16 bits, matches write target
{
    E_EVENT_NONE                           = 0,  // 0: None (Default)
    E_EVENT_RUN_STOP                       = 1,  // 1: RUN/STOP
    E_EVENT_AUTO_MANUAL_SWITCH             = 2,  // 2: Auto/Manual Switch
    E_EVENT_PROGRAM_START                  = 3,  // 3: Program Start (*1)
    E_EVENT_DIRECT_REVERSE_OPERATION       = 4,  // 4: Direct/Reverse Operation switch
    E_EVENT_SP_MODE_SWITCH                 = 5,  // 5: SP Mode Switch (*2)
    E_EVENT_AT_EXECUTE_CANCEL_100          = 6,  // 6: 100% AT Execute/Cancel
    E_EVENT_AT_EXECUTE_CANCEL_40           = 7,  // 7: 40% AT Execute/Cancel
    E_EVENT_SETTING_CHANGE_ENABLE_DISABLE  = 8,  // 8: Setting Change Enable/Disable
    E_EVENT_COMM_WRITING_ENABLE_DISABLE    = 9,  // 9: Communications Writing Enable/Disable (*3)
    E_EVENT_ALARM_LATCH_CANCEL             = 10, // 10: Alarm Latch Cancel
    E_EVENT_MULTI_SP_SWITCH_BIT_0          = 11, // 11: Multi-SP No. Switch, Bit 0
    E_EVENT_MULTI_SP_SWITCH_BIT_1          = 12, // 12: Multi-SP No. Switch, Bit 1
    E_EVENT_MULTI_SP_SWITCH_BIT_2          = 13  // 13: Multi-SP No. Switch, Bit 2

    // Notes from manual H175 p5-10:
    // (*1) PRST (program start) can be set even when the program pattern is set to OFF, but the function will be disabled.
    // (*2) Selection is possible only if there is a remote SP input.
    // (*3) Selection is possible only if external communications is supported.
};


/**
 * @brief Defines the possible values for the Extraction of Square Root Enable register (0x2E24).
 * Enables or disables square root calculation on the input. Level: Initial Setting.
 * Uses 4 bytes, write low word.
 */
enum OR_E5_SQRT_ENABLE : uint16_t // Values fit in 16 bits, matches write target
{
    E_SQRT_OFF = E_OFF, // 0: OFF (Default)
    E_SQRT_ON  = E_ON   // 1: ON
};

/**
 * @brief Defines the possible values for the Alarm Type registers (0x2F00, 0x2F03, 0x2F06, 0x2F09).
 * Specifies the condition that triggers the alarm. Level: Advanced Function Setting.
 * Uses 4 bytes, write low word.
 */
enum OR_E5_ALARM_TYPE : uint16_t // Values fit in 16 bits, matches write target
{
    E_ALARM_TYPE_ALARM_OFF                = 0,  // 0: Alarm function OFF
    E_ALARM_TYPE_UPPER_LOWER_LIMIT        = 1,  // 1: Upper- and lower-limit alarm (*1)
    E_ALARM_TYPE_UPPER_LIMIT              = 2,  // 2: Upper-limit alarm
    E_ALARM_TYPE_LOWER_LIMIT              = 3,  // 3: Lower-limit alarm
    E_ALARM_TYPE_UPPER_LOWER_RANGE        = 4,  // 4: Upper- and lower-limit range alarm (*1)
    E_ALARM_TYPE_UPPER_LOWER_STANDBY      = 5,  // 5: Upper- and lower-limit alarm with standby sequence (*1, *2)
    E_ALARM_TYPE_UPPER_LIMIT_STANDBY      = 6,  // 6: Upper-limit alarm with standby sequence (*2)
    E_ALARM_TYPE_LOWER_LIMIT_STANDBY      = 7,  // 7: Lower-limit alarm with standby sequence (*2)
    E_ALARM_TYPE_ABS_UPPER_LIMIT          = 8,  // 8: Absolute-value upper-limit alarm
    E_ALARM_TYPE_ABS_LOWER_LIMIT          = 9,  // 9: Absolute-value lower-limit alarm
    E_ALARM_TYPE_ABS_UPPER_STANDBY        = 10, // 10: Absolute-value upper-limit alarm with standby sequence (*2)
    E_ALARM_TYPE_ABS_LOWER_STANDBY        = 11, // 11: Absolute-value lower-limit alarm with standby sequence (*2)
    E_ALARM_TYPE_LBA                      = 12, // 12: LBA (Loop Burnout Alarm) (*3)
    E_ALARM_TYPE_PV_CHANGE_RATE           = 13, // 13: PV change rate alarm
    E_ALARM_TYPE_SP_ABS_UPPER_LIMIT       = 14, // 14: SP absolute-value upper-limit alarm
    E_ALARM_TYPE_SP_ABS_LOWER_LIMIT       = 15, // 15: SP absolute-value lower-limit alarm
    E_ALARM_TYPE_MV_ABS_UPPER_LIMIT       = 16, // 16: MV absolute-value upper-limit alarm (Use for Heating MV)
    E_ALARM_TYPE_MV_ABS_LOWER_LIMIT       = 17, // 17: MV absolute-value lower-limit alarm (Use for Heating MV)
    E_ALARM_TYPE_RSP_ABS_UPPER_LIMIT      = 18, // 18: RSP absolute-value upper-limit alarm (*4)
    E_ALARM_TYPE_RSP_ABS_LOWER_LIMIT      = 19, // 19: RSP absolute-value lower-limit alarm (*4)
    E_ALARM_TYPE_MV_COOL_ABS_UPPER_LIMIT  = 20, // 20: MV absolute-value upper-limit alarm (cooling)
    E_ALARM_TYPE_MV_COOL_ABS_LOWER_LIMIT  = 21, // 21: MV absolute-value lower-limit alarm (cooling)

    // NOTES based on H175 manual:
    // (*1) Set upper and lower values individually.
    // (*2) Alarm is suppressed until PV enters non-alarm range. See Standby Sequence Reset (0x2F0C).
    // (*3) LBA cannot be set for Alarm 2, 3, or 4.
    // (*4) Valid only with a remote SP input.
};

/**
 * @brief Defines the possible values for the Alarm Latch registers (0x2F01, 0x2F04, 0x2F07, 0x2F0A).
 * Determines if the alarm output remains ON even after the alarm condition clears. Level: Advanced Function Setting.
 * Uses 4 bytes, write low word.
 */
enum OR_E5_ALARM_LATCH : uint16_t // Values fit in 16 bits, matches write target
{
    E_ALARM_LATCH_OFF = E_OFF, // 0: OFF (Default)
    E_ALARM_LATCH_ON  = E_ON   // 1: ON
};

/**
 * @brief Defines the possible values for the Standby Sequence Reset register (0x2F0C).
 * Selects the condition for resetting the standby sequence for alarms with standby. Level: Advanced Function Setting.
 * Uses 4 bytes, write low word.
 */
enum OR_E5_STANDBY_SEQUENCE_RESET : uint16_t // Values fit in 16 bits, matches write target
{
    E_STANDBY_RESET_CONDITION_A = 0, // 0: Condition A (Reset when Power ON or RUN/STOP switched from STOP to RUN)
    E_STANDBY_RESET_CONDITION_B = 1  // 1: Condition B (Reset when Power ON, RUN/STOP switched from STOP to RUN, or Set Point changed)
};

/**
 * @brief Defines the possible values for the Auxiliary Output Open in Alarm registers (Aux1: 0x2F0D, Aux2: 0x2F0E, Aux3: 0x2F10, Aux4: 0x2F11).
 * Controls whether the specified Aux Output is energized (Close/ON) or de-energized (Open/OFF) when an alarm assigned to it occurs. Level: Advanced Function Setting.
 * Uses 4 bytes per register, write low word.
 */
enum OR_E5_AUX_OUTPUT_OPEN_IN_ALARM : uint16_t // Values fit in 16 bits, matches write target
{
    // Terminology assumes standard relay behavior: Close=Energized/ON, Open=De-energized/OFF
    E_AUX_OUTPUT_CLOSE_IN_ALARM = 0, // 0: Output is Energized (ON/Closed) when alarm condition is met
    E_AUX_OUTPUT_OPEN_IN_ALARM  = 1  // 1: Output is De-energized (OFF/Open) when alarm condition is met
};

/**
 * @brief Defines the possible values for the PV/SP No. 1 Display Selection register (0x3000)
 *        and PV/SP No. 2 Display Selection register (0x3008).
 * Selects the content displayed on the PV/SP display lines. Level: Operator/Adjustment?
 * Uses 4 bytes per register, write low word.
 */
enum OR_E5_PV_SP_DISPLAY_SELECT : uint16_t // Values fit in 16 bits, matches write target
{
    E_DISP_NONE                = 0, // 0: Nothing displayed
    E_DISP_PV_SP               = 1, // 1: PV/SP (Standard display)
    E_DISP_PV                  = 2, // 2: PV only
    E_DISP_PV_SP_CHAR          = 3, // 3: PV/SP (character display, layout may differ)
    E_DISP_PV_SP_MV            = 4, // 4: PV/SP/MV (Manipulated Value)
    E_DISP_PV_SP_MULTI_SP      = 5, // 5: PV/SP/Multi-SP No.
    E_DISP_PV_SP_SOAK_REMAIN   = 6, // 6: PV/SP/Soak time remaining
    E_DISP_PV_SP_RAMP_SP       = 7, // 7: PV/SP/Ramp SP value
    E_DISP_PV_SP_ALARM1_VAL    = 8  // 8: PV/SP/Alarm value 1
};

/**
 * @brief Defines the possible values for the MV Display Selection register (0x3001).
 * Selects which Manipulated Value (Heating or Cooling) is shown when MV is displayed. Level: Operator/Adjustment?
 * Uses 4 bytes, write low word.
 */
enum OR_E5_MV_DISPLAY_SELECT : uint16_t // Values fit in 16 bits, matches write target
{
    E_MV_DISP_HEAT = 0, // 0: Display MV (heating)
    E_MV_DISP_COOL = 1  // 1: Display MV (cooling)
};

/**
 * @brief Defines the possible values for the Automatic Display Return Time register (0x3003).
 * Sets the time after which the display returns to the default screen. Level: ?
 * Uses 4 bytes, write low word. Value 0 = OFF, 1-99 = seconds.
 */
enum OR_E5_AUTO_DISPLAY_RETURN : uint16_t // Values fit in 16 bits, matches write target
{
    E_AUTO_RETURN_OFF = 0, // 0: Automatic return disabled
    // 1 to 99: Time in seconds (e.g., 60 = return after 60 seconds)
    E_AUTO_RETURN_MIN_TIME_S = 1,
    E_AUTO_RETURN_MAX_TIME_S = 99
};


/**
 * @brief Defines the possible values for the Display Refresh Period register (0x3004).
 * Sets the refresh rate for the display. Level: ?
 * Uses 4 bytes, write low word.
 */
enum OR_E5_DISPLAY_REFRESH_PERIOD : uint16_t // Values fit in 16 bits, matches write target
{
    E_REFRESH_OFF   = 0, // 0: Refresh OFF (not typical, may mean slowest rate?)
    E_REFRESH_0_25_S = 1, // 1: 0.25 seconds
    E_REFRESH_0_5_S = 2, // 2: 0.5 seconds
    E_REFRESH_1_0_S = 3  // 3: 1.0 second
};

/**
 * @brief Defines the possible values for the PV Status Display Function register (0x3011)
 *        and SV Status Display Function register (0x3012).
 * Selects which status information replaces the PV or SV display under certain conditions. Level: ?
 * Uses 4 bytes per register, write low word.
 */
enum OR_E5_STATUS_DISPLAY_FUNC : uint16_t // Values fit in 16 bits, matches write target
{
    E_STAT_DISP_FUNC_OFF           = 0, // 0: Status display function OFF
    E_STAT_DISP_FUNC_MANUAL        = 1, // 1: Display when in Manual Mode
    E_STAT_DISP_FUNC_STOP          = 2, // 2: Display when in Stop Mode
    E_STAT_DISP_FUNC_ALARM_1       = 3, // 3: Display when Alarm 1 is ON
    E_STAT_DISP_FUNC_ALARM_2       = 4, // 4: Display when Alarm 2 is ON
    E_STAT_DISP_FUNC_ALARM_3       = 5, // 5: Display when Alarm 3 is ON
    E_STAT_DISP_FUNC_ALARM_4       = 6, // 6: Display when Alarm 4 is ON
    E_STAT_DISP_FUNC_ALARM_1_4_OR  = 7, // 7: Display when Any Alarm (1-4) is ON
    E_STAT_DISP_FUNC_HEATER_ALARM  = 8, // 8: Display when Heater Alarm (HB/HS) is ON
    E_STAT_DISP_FUNC_MESSAGE       = 9  // 9: Status display message (*1 Selection possible only with the E5GC)
};

// --- NEW/MERGED Value Enums from Image H175 5-14 / 5-15 / 5-16 ---

/**
 * @brief Defines the possible values for the Protocol Setting register (0x3100).
 * Selects the communication protocol. Level: Communications Setting. Uses 16 bits.
 */
enum OR_E5_COMM_PROTOCOL : uint16_t
{
    E_PROTO_COMPOWAYF = 0, // 0: Compoway/F
    E_PROTO_MODBUS    = 1  // 1: Modbus (Default)
};

/**
 * @brief Defines the possible values for the Communications Baud Rate register (0x3102).
 * Selects the communication speed. Level: Communications Setting. Uses 16 bits.
 */
enum OR_E5_COMM_BAUD_RATE : uint16_t
{
    E_BAUD_9600  = 3, // 3: 9.6 kbps
    E_BAUD_19200 = 4, // 4: 19.2 kbps
    E_BAUD_38400 = 5, // 5: 38.4 kbps
    E_BAUD_57600 = 6  // 6: 57.6 kbps
    // Default depends on model, often 9.6k (3)
};

/**
 * @brief Defines the possible values for the Communications Data Length register (0x3103).
 * Selects the number of data bits per character. Level: Communications Setting. Uses 16 bits.
 */
enum OR_E5_COMM_DATA_LENGTH : uint16_t
{
    E_DATA_LEN_7 = 7, // 7: 7 bits
    E_DATA_LEN_8 = 8  // 8: 8 bits (Default)
};

/**
 * @brief Defines the possible values for the Communications Stop Bits register (0x3104).
 * Selects the number of stop bits. Level: Communications Setting. Uses 16 bits.
 */
enum OR_E5_COMM_STOP_BITS : uint16_t
{
    E_STOP_BITS_1 = 1, // 1: 1 stop bit (Default)
    E_STOP_BITS_2 = 2  // 2: 2 stop bits
};

/**
 * @brief Defines the possible values for the Communications Parity register (0x3105).
 * Selects the parity check mode. Level: Communications Setting. Uses 16 bits.
 */
enum OR_E5_COMM_PARITY : uint16_t
{
    E_PARITY_NONE = 0, // 0: None
    E_PARITY_EVEN = 1, // 1: Even (Often Default)
    E_PARITY_ODD  = 2  // 2: Odd
};

/**
 * @brief Defines the possible values for the PF Setting register (0x3200).
 * Assigns a function to the Programmable Function (PF) key. Level: Advanced Function Setting. Uses 16 bits.
 */
enum OR_E5_PF_SETTING : uint16_t
{
    E_PF_DISABLED               = 0, // 0: Disabled (Default)
    E_PF_FUNC_RUN                 = 1, // 1: Run (Switches to RUN mode)
    E_PF_FUNC_STOP                = 2, // 2: Stop (Switches to STOP mode)
    E_PF_FUNC_RUN_STOP            = 3, // 3: RUN/STOP (Toggles between RUN and STOP)
    E_PF_FUNC_AT_100              = 4, // 4: 100% AT execute/cancel
    E_PF_FUNC_AT_40               = 5, // 5: 40% AT execute/cancel
    E_PF_FUNC_ALARM_LATCH_CANCEL  = 6, // 6: Alarm latch cancel
    E_PF_FUNC_AUTO_MANUAL         = 7, // 7: Auto/manual switch
    E_PF_FUNC_MONITOR_SETTING_ITEM= 8, // 8: Monitor/setting item display (Cycles through items configured in 0x3202-0x3206)
    E_PF_FUNC_DIGIT_SHIFT_KEY     = 9  // 9: Digit shift key (Used for setting values)
};

/**
 * @brief Defines the possible values for the Monitor/Setting Item registers (0x3202 to 0x3206).
 * Selects which parameter is accessible via the "Monitor/Setting Item" function (e.g., tied to PF key setting 8).
 * Level: Advanced Function Setting. Uses 16 bits per register.
 */
enum OR_E5_MONITOR_SETTING_ITEM : uint16_t
{
    E_MON_ITEM_DISABLED                 = 0,  // 0: Disabled (Default)
    E_MON_ITEM_PV_SP_MULTI_SP           = 1,  // 1: PV/SP/multi-SP
    E_MON_ITEM_PV_SP_MV                 = 2,  // 2: PV/SP/MV
    E_MON_ITEM_PV_SP_SOAK_TIME_REMAIN   = 3,  // 3: PV/SP/soak time remain
    E_MON_ITEM_PROP_BAND                = 4,  // 4: Proportional band (Heating, register 0x2A00)
    E_MON_ITEM_INTEGRAL_TIME            = 5,  // 5: Integral time (Heating, register 0x2A01)
    E_MON_ITEM_DERIVATIVE_TIME          = 6,  // 6: Derivative time (Heating, register 0x2A02)
    E_MON_ITEM_ALARM_VALUE_1            = 7,  // 7: Alarm value 1 (register 0x2902)
    E_MON_ITEM_ALARM_UPPER_LIMIT_1      = 8,  // 8: Alarm value upper limit 1 (register 0x2903)
    E_MON_ITEM_ALARM_LOWER_LIMIT_1      = 9,  // 9: Alarm value lower limit 1 (register 0x2904)
    E_MON_ITEM_ALARM_VALUE_2            = 10, // 10: Alarm value 2 (register 0x2905)
    E_MON_ITEM_ALARM_UPPER_LIMIT_2      = 11, // 11: Alarm value upper limit 2 (register 0x2906)
    E_MON_ITEM_ALARM_LOWER_LIMIT_2      = 12, // 12: Alarm value lower limit 2 (register 0x2907)
    E_MON_ITEM_ALARM_VALUE_3            = 13, // 13: Alarm value 3 (register 0x2908)
    E_MON_ITEM_ALARM_UPPER_LIMIT_3      = 14, // 14: Alarm value upper limit 3 (register 0x2909)
    E_MON_ITEM_ALARM_LOWER_LIMIT_3      = 15, // 15: Alarm value lower limit 3 (register 0x290A)
    E_MON_ITEM_ALARM_VALUE_4            = 16, // 16: Alarm value 4 (register 0x290B)
    E_MON_ITEM_ALARM_UPPER_LIMIT_4      = 17, // 17: Alarm value upper limit 4 (register 0x290C)
    E_MON_ITEM_ALARM_LOWER_LIMIT_4      = 18, // 18: Alarm value lower limit 4 (register 0x290D)
    E_MON_ITEM_PV_SP_INTERNAL_SP        = 19, // 19: PV/SP/Internal set point (register 0x2002 / 0x2403)
    E_MON_ITEM_PV_SP_ALARM_VALUE_1      = 20, // 20: PV/SP/Alarm value 1 (Combination display?)
    E_MON_ITEM_PROP_BAND_COOL           = 21, // 21: Proportional Band (Cooling) (register 0x2701)
    E_MON_ITEM_INTEGRAL_TIME_COOL       = 22, // 22: Integral Time (Cooling) (register 0x2702)
    E_MON_ITEM_DERIVATIVE_TIME_COOL     = 23  // 23: Derivative Time (Cooling) (register 0x2703)
};

/**
 * @brief Defines the possible values for the Integral/Derivative Time Unit register (0x3309).
 * Selects the time base for Integral (I) and Derivative (D) parameters. AFS Level.
 * Applies to registers 0x2A01, 0x2A02 (Heat) and 0x2702, 0x2703 (Cool). Uses 4 bytes, write low word.
 */
enum OR_E5_INTEGRAL_DERIVATIVE_TIME_UNIT : uint16_t // Values fit in 16 bits, matches write target
{
    E_TIME_UNIT_SECONDS     = 0, // 0: Seconds (e.g., Integral/Derivative range 0 to 9999 s)
    E_TIME_UNIT_TENTH_SEC   = 1  // 1: 0.1 Seconds (e.g., Integral/Derivative range 0.0 to 999.9 s)
};

/**
 * @brief Defines the possible values for the Manual Output Method register (0x330A).
 * Determines behavior when switching to Manual mode. AFS Level. Uses 4 bytes, write low word.
 */
enum OR_E5_MANUAL_OUTPUT_METHOD : uint16_t // Values fit in 16 bits, matches write target
{
    E_MANUAL_OUT_HOLD = 0, // 0: HOLD (Start manual mode using the MV from auto mode)
    E_MANUAL_OUT_INIT = 1  // 1: INIT (Start manual mode using the Manual MV Initial Value from register 0x330C)
};

/**
 * @brief Defines the possible values for the Soak Time Unit register (0x3327).
 * Selects the time unit for Soak Time (0x2729) and Soak Time Remain (0x2728). AFS Level.
 * Uses 4 bytes, write low word.
 */
enum OR_E5_SOAK_TIME_UNIT : uint16_t // Values fit in 16 bits, matches write target
{
    E_SOAK_UNIT_MINUTES = 0, // 0: Minutes (Range 1 to 9999 minutes)
    E_SOAK_UNIT_HOURS   = 1, // 1: Hours (Range 1 to 9999 hours)
    E_SOAK_UNIT_SECONDS = 2  // 2: Seconds (Range 1 to 9999 seconds) (*1 Selection possible only with the E5GC)
};

/**
 * @brief Defines the possible values for the Alarm SP Selection register (0x3328).
 * Selects which Set Point value is used for alarm checking during SP ramp. AFS Level.
 * Uses 4 bytes, write low word.
 */
enum OR_E5_ALARM_SP_SELECTION : uint16_t // Values fit in 16 bits, matches write target
{
    E_ALARM_SP_DURING_RAMP = 0, // 0: Use the current SP value during the ramp for alarm calculation
    E_ALARM_SP_SET_POINT   = 1  // 1: Use the final target SP value for alarm calculation during the ramp
};

/**
 * @brief Defines the possible values for the Heating/Cooling Tuning Method register (0x332E).
 * Selects the tuning algorithm for cooling in Heat/Cool control mode. AFS Level.
 * Uses 4 bytes, write low word.
 */
enum OR_E5_HEATING_COOLING_TUNING_METHOD : uint16_t // Values fit in 16 bits, matches write target
{
    E_TUNING_SAME_AS_HEATING = 0, // 0: Use the same PID constants as heating control (Default?)
    E_TUNING_LINEAR          = 1, // 1: Linear tuning for cooling
    E_TUNING_AIR_COOLING     = 2, // 2: Tuning optimized for air cooling
    E_TUNING_WATER_COOLING   = 3  // 3: Tuning optimized for water cooling
};


/*
 * TODO: Define enums for Status Bits (Registers 0x2001, 0x2406, 0x2407(High Word), 0x2408, 0x2409(High Word)).
 *       The specific bit meanings need to be obtained from Section 5-2 ("Status Monitor Table") of the
 *       Omron E5xC Communications Manual (H175).
 *       Example bit assignments below are highly speculative and need verification.
 */

// --- Placeholder Example Enums for Status Bits (NEEDS VERIFICATION and COMPLETION from H175 Section 5-2) ---
/*
// Example for Low Word (0x2001, 0x2406, 0x2408) - Precise meaning depends on which status register
enum OR_E5_STATUS_LOW_WORD_BITS : uint16_t
{
    E_SLW_BIT_0                 = (1 << 0),  // Status/Control output 1 state? Alarm 1 state?
    E_SLW_BIT_1                 = (1 << 1),  // Status/Control output 2 state? Alarm 2 state?
    E_SLW_BIT_2                 = (1 << 2),  // Status/Alarm 3 state?
    E_SLW_BIT_3                 = (1 << 3),  // Status/Alarm 4 state?
    E_SLW_STATUS_RUN            = (1 << 4),  // RUN=1, STOP=0 ? Needs Verification
    E_SLW_STATUS_AUTO           = (1 << 5),  // AUTO=1, MAN=0 ? Needs Verification
    E_SLW_STATUS_AT_EXECUTING   = (1 << 6),  // AT Executing=1 ? Needs Verification
    E_SLW_STATUS_COMM_WRITE_ON  = (1 << 7),  // Comm Writing Enabled=1 ? Needs Verification
    E_SLW_BIT_8                 = (1 << 8),  // Status/Program Run Status?
    E_SLW_BIT_9                 = (1 << 9),  // Status/Program Wait Status?
    E_SLW_BIT_10                = (1 << 10), // Status/Program Hold Status?
    E_SLW_BIT_11                = (1 << 11), // Status/Program End Status?
    E_SLW_BIT_12                = (1 << 12), // Status/Memory Error?
    E_SLW_BIT_13                = (1 << 13), // Status/Input Error?
    E_SLW_STATUS_RSP_ERROR      = (1 << 14), // Remote SP Input Error=1? Needs Verification
    E_SLW_STATUS_PV_INPUT_ERROR = (1 << 15)  // PV Input Error=1 ? Needs Verification - Often called S.ERR
};

// Example for High Word (0x2407, 0x2409) - Precise meaning depends on which status register
enum OR_E5_STATUS_HIGH_WORD_BITS : uint16_t
{
    E_SHW_STATUS_HB_OUTPUT      = (1 << 0), // HB Alarm Output State=1? (Overall bit 16) Needs Verification
    E_SHW_STATUS_HS_OUTPUT      = (1 << 1), // HS Alarm Output State=1? (Overall bit 17) Needs Verification
    E_SHW_STATUS_LBA1_OPERATING = (1 << 2), // LBA 1 operating=1? (Overall bit 18) Needs Verification
    E_SHW_STATUS_LBA2_OPERATING = (1 << 3), // LBA 2 operating=1? (Overall bit 19) Needs Verification
    E_SHW_STATUS_MV_LIMITING    = (1 << 4), // MV Limiting=1? (Overall bit 20) Needs Verification
    E_SHW_STATUS_RAMP_CONTROL   = (1 << 5), // Ramp Control Operating=1? (Overall bit 21) Needs Verification
    E_SHW_BIT_6                 = (1 << 6), // (Overall bit 22)
    E_SHW_BIT_7                 = (1 << 7), // (Overall bit 23)
    E_SHW_BIT_8                 = (1 << 8), // (Overall bit 24)
    E_SHW_BIT_9                 = (1 << 9), // (Overall bit 25)
    E_SHW_BIT_10                = (1 << 10),// (Overall bit 26)
    E_SHW_BIT_11                = (1 << 11),// (Overall bit 27)
    E_SHW_BIT_12                = (1 << 12),// (Overall bit 28) Startup Tuning Operating?
    E_SHW_BIT_13                = (1 << 13),// (Overall bit 29)
    E_SHW_BIT_14                = (1 << 14),// (Overall bit 30)
    E_SHW_BIT_15                = (1 << 15) // (Overall bit 31)
    // Note: Actual bit meanings MUST be confirmed with H175 manual Section 5-2 Status Monitor Table.
};
*/
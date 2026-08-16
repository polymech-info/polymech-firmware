#ifndef OMRONE5_EX_H
#define OMRONE5_EX_H

enum class ModbusAddresses {
    OR_E5_SWR_PV_TEMPERATURE = 0x2000,               // PV Temperature: Use the specified range for each sensor.
    OR_E5_SWR_STATUS = 0x2001,                       // Status: Refer to 5-2 Status for details.
    OR_E5_SWR_INTERNAL_SET_POINT = 0x2002,           // Internal Set Point: SP lower limit to SP upper limit.
    OR_E5_SWR_HEATER_CURRENT_1_VALUE = 0x2003,       // Heater Current 1 Value: Monitor current range 0.0 to 55.0.
    OR_E5_SWR_MV_MONITOR_HEATING = 0x2004,           // MV Monitor (Heating): Range from −5.0 to 105.0.
    OR_E5_SWR_MV_MONITOR_COOLING = 0x2005,           // MV Monitor (Cooling): Range from 0.0 to 105.0.
    OR_E5_SWR_SET_POINT = 0x2103,                    // Set Point: SP lower limit to SP upper limit.
    OR_E5_SWR_ALARM_VALUE_1 = 0x2104,                // Alarm Value 1: Range from −1999 to 9999.
    OR_E5_SWR_ALARM_VALUE_UPPER_LIMIT_1 = 0x2105,    // Alarm Value Upper Limit 1: Range from −1999 to 9999.
    OR_E5_SWR_ALARM_VALUE_LOWER_LIMIT_1 = 0x2106,    // Alarm Value Lower Limit 1: Range from −1999 to 9999.
    OR_E5_SWR_ALARM_VALUE_2 = 0x2107,                // Alarm Value 2: Range from −1999 to 9999.
    OR_E5_SWR_ALARM_VALUE_UPPER_LIMIT_2 = 0x2108,    // Alarm Value Upper Limit 2: Range from −1999 to 9999.
    OR_E5_SWR_ALARM_VALUE_LOWER_LIMIT_2 = 0x2109,    // Alarm Value Lower Limit 2: Range from −1999 to 9999.
    OR_E5_SWR_PV_TEMPERATURE_2 = 0x2402,             // PV Temperature 2: Use the specified range for each sensor.
    OR_E5_SWR_INTERNAL_SET_POINT_2 = 0x2403,         // Internal Set Point 2: SP lower limit to SP upper limit.
    OR_E5_SWR_MULTI_SP_NO_MONITOR = 0x2404,          // Multi-SP No. Monitor: Range from 0 to 7.
    OR_E5_SWR_STATUS_2 = 0x2406,                     // Status 2: Refer to 5-2 Status for details.
    OR_E5_SWR_STATUS_3 = 0x2407,                     // Status 3: Refer to 5-2 Status for details.
    OR_E5_SWR_STATUS_4 = 0x2408,                     // Status 4: Refer to 5-2 Status for details.
    OR_E5_SWR_STATUS_5 = 0x2409,                     // Status 5: Refer to 5-2 Status for details.
    OR_E5_SWR_DECIMAL_POINT_MONITOR = 0x2410,        // Decimal Point Monitor: Range from 0 to 3.
    // Add more entries as needed.


    OR_E5_SWR_OPERATION_ADJUSTMENT_PROTECT = 0x2500, // Operation/Adjustment Protect: Control restrictions.
    OR_E5_SWR_INITIAL_SETTING_COMMUNICATIONS_PROTECT = 0x2501, // Initial Setting/Communications Protect: Control restrictions.
    OR_E5_SWR_SETTING_CHANGE_PROTECT = 0x2502,       // Setting Change Protect: Controls change restrictions.
    OR_E5_SWR_PF_KEY_PROTECT = 0x2503,               // PF Key Protect: Protection settings for PF key.
    OR_E5_SWR_MOVE_TO_PROTECT_LEVEL = 0x2504,        // Move to Protect Level: Adjustment level setting range −1999 to 9999.
    OR_E5_SWR_PASSWORD_TO_MOVE_TO_PROTECT_LEVEL = 0x2505, // Password to Move to Protect Level: Only settable.
    OR_E5_SWR_PARAMETER_MASK_ENABLE = 0x2506,        // Parameter Mask Enable: ON/OFF for parameter masking.
    OR_E5_SWR_CHANGED_PARAMETERS_ONLY = 0x2507,      // Changed Parameters Only: ON/OFF for display of changed parameters.
    OR_E5_SWR_MANUAL_MV = 0x2600,                    // Manual MV: Manual operation and MV settings.
    OR_E5_SWR_SET_POINT_2 = 0x2601,                  // Set Point: SP lower limit to SP upper limit.
    OR_E5_SWR_REMOTE_SP_MONITOR = 0x2602,            // Remote SP Monitor: Remote set point monitoring settings.
    OR_E5_SWR_HEATER_CURRENT_1_VALUE_2 = 0x2604,     // Heater Current 1 Value: Monitoring settings for heater current.
    OR_E5_SWR_MV_MONITOR_HEATING_2 = 0x2605,         // MV Monitor (Heating): Monitoring settings for MV in heating.
    OR_E5_SWR_MV_MONITOR_COOLING_2 = 0x2606,         // MV Monitor (Cooling): Monitoring settings for MV in cooling.
    OR_E5_SWR_VALVE_OPENING_MONITOR = 0x2607,        // Valve Opening Monitor: Valve position monitoring.
    // Further entries truncated for brevity. Continue as necessary.
    // ------------ Continuation from previous response...
    OR_E5_SWR_PROPORTIONAL_BAND_COOLING = 0x2701,    // Proportional Band (Cooling): Adjustment range 0.1 to 999.9.
    OR_E5_SWR_INTEGRAL_TIME_COOLING = 0x2702,        // Integral Time (Cooling): Time settings for cooling control.
    OR_E5_SWR_DERIVATIVE_TIME_COOLING = 0x2703,      // Derivative Time (Cooling): Time settings for derivative cooling.
    OR_E5_SWR_DEAD_BAND = 0x2704,                    // Dead Band: Set range for dead band adjustments.
    OR_E5_SWR_MANUAL_RESET_VALUE = 0x2705,           // Manual Reset Value: Adjustment settings for manual reset.
    OR_E5_SWR_HYSTERESIS_HEATING = 0x2706,           // Hysteresis (Heating): Hysteresis settings for heating control.
    OR_E5_SWR_HYSTERESIS_COOLING = 0x2707,           // Hysteresis (Cooling): Hysteresis settings for cooling control.
    OR_E5_SWR_CONTROL_PERIOD_HEATING = 0x2708,       // Control Period (Heating): Timing settings for heating control.
    OR_E5_SWR_CONTROL_PERIOD_COOLING = 0x2709,       // Control Period (Cooling): Timing settings for cooling control.
    OR_E5_SWR_POSITION_PROPORTIONAL_DEAD_BAND = 0x270A, // Position Proportional Dead Band: Settings for position proportional control.
    OR_E5_SWR_OPEN_CLOSE_HYSTERESIS = 0x270B,        // Open/Close Hysteresis: Settings for hysteresis in open/close control.
    OR_E5_SWR_SP_RAMP_TIME_UNIT = 0x270C,            // SP Ramp Time Unit: Units for SP ramp time settings.
    OR_E5_SWR_SP_RAMP_SET_VALUE = 0x270D,            // SP Ramp Set Value: Ramp settings for set points.
    OR_E5_SWR_SP_RAMP_FALL_VALUE = 0x270E,           // SP Ramp Fall Value: Ramp fall settings.
    OR_E5_SWR_MV_AT_STOP = 0x270F,                   // MV at Stop: MV settings when the system is stopped.
    OR_E5_SWR_MV_AT_PV_ERROR = 0x2711,               // MV at PV Error: MV settings during a PV error.
    OR_E5_SWR_MV_CHANGE_RATE_LIMIT = 0x2713,         // MV Change Rate Limit: Limit settings for MV change rate.
    OR_E5_SWR_PV_INPUT_SLOPE_COEFFICIENT = 0x2718,   // PV Input Slope Coefficient: Slope settings for PV input.
    OR_E5_SWR_HEATER_CURRENT_1_VALUE_3 = 0x271A,     // Heater Current 1 Value: Current monitoring settings.
    OR_E5_SWR_HEATER_BURNOUT_DETECTION_1 = 0x271B,   // Heater Burnout Detection 1: Settings for detecting heater burnout.
    OR_E5_SWR_LEAKAGE_CURRENT_1_MONITOR = 0x271C,    // Leakage Current 1 Monitor: Monitoring leakage currents.
    OR_E5_SWR_HS_ALARM_1 = 0x271D,                   // HS Alarm 1: High-speed alarm settings for channel 1.
    OR_E5_SWR_PROCESS_VALUE_INPUT_SHIFT = 0x2723,    // Process Value Input Shift: Shift adjustments for process value input.
    OR_E5_SWR_HEATER_CURRENT_2_VALUE = 0x2724,       // Heater Current 2 Value: Monitoring settings for heater current.
    OR_E5_SWR_HEATER_BURNOUT_DETECTION_2 = 0x2725,   // Heater Burnout Detection 2: Burnout detection settings.
    OR_E5_SWR_LEAKAGE_CURRENT_2_MONITOR = 0x2726,    // Leakage Current 2 Monitor: Monitoring settings for leakage current.
    OR_E5_SWR_HS_ALARM_2 = 0x2727,                   // HS Alarm 2: High-speed alarm settings for channel 2.
    OR_E5_SWR_SOAK_TIME_REMAIN = 0x2728,             // Soak Time Remain: Monitoring remaining soak time.
    OR_E5_SWR_SOAK_TIME = 0x2729,                    // Soak Time: Settings for soak time duration.
   

    // --- Continuation from previous entries...
    OR_E5_SWR_WAIT_BAND = 0x272A,                      // Wait Band: Wait band settings for various inputs.
    OR_E5_SWR_REMOTE_SP_INPUT_SHIFT = 0x272B,          // Remote SP Input Shift: Settings for shifting the remote SP input.
    OR_E5_SWR_REMOTE_SP_INPUT_SLOPE_COEFFICIENT = 0x272C, // Remote SP Input Slope Coefficient: Slope settings for remote SP input.
    OR_E5_SWR_INPUT_DIGITAL_FILTER = 0x2800,           // Input Digital Filter: Filter settings for input signals.
    OR_E5_SWR_MOVING_AVERAGE_COUNT = 0x2804,           // Moving Average Count: Settings for the moving average count.
    OR_E5_SWR_EXTRACTION_OF_SQUARE_ROOT_LOW_CUT_POINT = 0x2808, // Extraction of Square Root Low-cut Point: Settings for square root extraction low-cut.
    OR_E5_SWR_SP_0 = 0x2900,                           // SP 0: Set point 0 for various control operations.
    OR_E5_SWR_ALARM_VALUE_1_2 = 0x2902,                // Alarm Value 1: Alarm settings for the first channel.
    OR_E5_SWR_ALARM_VALUE_UPPER_LIMIT_1_2 = 0x2903,    // Alarm Value Upper Limit 1: Upper limit for the first alarm channel.
    OR_E5_SWR_ALARM_VALUE_LOWER_LIMIT_1_2 = 0x2904,    // Alarm Value Lower Limit 1: Lower limit for the first alarm channel.
    OR_E5_SWR_ALARM_VALUE_2_2 = 0x2905,                // Alarm Value 2: Alarm settings for the second channel.
    OR_E5_SWR_ALARM_VALUE_UPPER_LIMIT_2_2 = 0x2906,    // Alarm Value Upper Limit 2: Upper limit for the second alarm channel.
    OR_E5_SWR_ALARM_VALUE_LOWER_LIMIT_2_2 = 0x2907,    // Alarm Value Lower Limit 2: Lower limit for the second alarm channel.
    OR_E5_SWR_ALARM_VALUE_3 = 0x2908,                  // Alarm Value 3: Alarm settings for the third channel.
    OR_E5_SWR_ALARM_VALUE_UPPER_LIMIT_3 = 0x2909,      // Alarm Value Upper Limit 3: Upper limit for the third alarm channel.
    OR_E5_SWR_ALARM_VALUE_LOWER_LIMIT_3 = 0x290A,      // Alarm Value Lower Limit 3: Lower limit for the third alarm channel.
    OR_E5_SWR_ALARM_VALUE_4 = 0x290B,                  // Alarm Value 4: Alarm settings for the fourth channel.
    OR_E5_SWR_ALARM_VALUE_UPPER_LIMIT_4 = 0x290C,      // Alarm Value Upper Limit 4: Upper limit for the fourth alarm channel.
    OR_E5_SWR_ALARM_VALUE_LOWER_LIMIT_4 = 0x290D,      // Alarm Value Lower Limit 4: Lower limit for the fourth alarm channel.
    OR_E5_SWR_SP_1 = 0x290E,                           // SP 1: Set point 1 for various control operations.
    OR_E5_SWR_SP_2 = 0x291C,                           // SP 2: Set point 2 for various control operations.
    OR_E5_SWR_SP_3 = 0x292A,                           // SP 3: Set point 3 for various control operations.
    OR_E5_SWR_SP_4 = 0x2938,                           // SP 4: Set point 4 for various control operations.
    OR_E5_SWR_SP_5 = 0x2946,                           // SP 5: Set point 5 for various control operations.
    OR_E5_SWR_SP_6 = 0x2954,                           // SP 6: Set point 6 for various control operations.
    OR_E5_SWR_SP_7 = 0x2962,                           // SP 7: Set point 7 for various control operations.
    OR_E5_SWR_PROPORTIONAL_BAND = 0x2A00,              // Proportional Band: Proportional band settings for PID control.
    OR_E5_SWR_INTEGRAL_TIME = 0x2A01,                  // Integral Time: Integral time settings for PID control.
    OR_E5_SWR_DERIVATIVE_TIME = 0x2A02,                // Derivative Time: Derivative time settings for PID control.
    OR_E5_SWR_MV_UPPER_LIMIT = 0x2A05,                 // MV Upper Limit: Upper limit settings for MV.
    OR_E5_SWR_MV_LOWER_LIMIT = 0x2A06,                 // MV Lower Limit: Lower limit settings for MV.
    OR_E5_SWR_INPUT_TYPE = 0x2C00,                     // Input Type: Settings for the type of input sensor.
    OR_E5_SWR_TEMPERATURE_UNIT = 0x2C01,               // Temperature Unit: Settings for temperature unit display (°C/°F).
    OR_E5_SWR_SCALING_LOWER_LIMIT = 0x2C09,            // Scaling Lower Limit: Lower limit for scaling input.
    OR_E5_SWR_SCALING_UPPER_LIMIT = 0x2C0B,            // Scaling Upper Limit: Upper limit for scaling input.
    OR_E5_SWR_DECIMAL_POINT = 0x2C0C,                  // Decimal Point: Decimal point display settings.
    OR_E5_SWR_REMOTE_SP_UPPER_LIMIT = 0x2C0D,          // Remote SP Upper Limit: Upper limit for remote SP input.
    OR_E5_SWR_REMOTE_SP_LOWER_LIMIT = 0x2C0E,          // Remote SP Lower Limit: Lower limit for remote SP input.
    OR_E5_SWR_PV_DECIMAL_POINT_DISPLAY = 0x2C0F,       // PV Decimal Point Display: Display settings for PV decimal points.

    //--- Continuation from previous entries...
    OR_E5_SWR_CONTROL_OUTPUT_1_SIGNAL = 0x2D03,       // Control Output 1 Signal: Signal settings for the first control output.
    OR_E5_SWR_CONTROL_OUTPUT_2_SIGNAL = 0x2D04,       // Control Output 2 Signal: Signal settings for the second control output.
    OR_E5_SWR_SP_UPPER_LIMIT = 0x2D0F,                // SP Upper Limit: Upper limit for set point range.
    OR_E5_SWR_SP_LOWER_LIMIT = 0x2D10,                // SP Lower Limit: Lower limit for set point range.
    OR_E5_SWR_STANDARD_OR_HEATING_COOLING = 0x2D11,   // Standard or Heating/Cooling: Operation mode selection.
    OR_E5_SWR_DIRECT_REVERSE_OPERATION = 0x2D12,      // Direct/Reverse Operation: Operation direction setting.
    OR_E5_SWR_CLOSE_FLOATING = 0x2D13,                // Close/Floating: Selection between close and floating control.
    OR_E5_SWR_PID_ON_OFF = 0x2D14,                    // PID ON/OFF: PID control on/off switch.
    OR_E5_SWR_ST = 0x2D15,                            // ST: Special function toggle.
    OR_E5_SWR_PROGRAM_PATTERN = 0x2D16,               // Program Pattern: Program pattern settings.
    OR_E5_SWR_REMOTE_SP_INPUT = 0x2D18,               // Remote SP Input: Remote set point input settings.
    OR_E5_SWR_MINIMUM_OUTPUT_ON_OFF_BAND = 0x2D19,    // Minimum Output ON/OFF Band: Minimum output settings for ON/OFF control.
    OR_E5_SWR_TRANSFER_OUTPUT_TYPE = 0x2E00,          // Transfer Output Type: Output type settings for data transfer.
    OR_E5_SWR_TRANSFER_OUTPUT_SIGNAL = 0x2E01,        // Transfer Output Signal: Signal settings for transfer output.
    OR_E5_SWR_CONTROL_OUTPUT_1_ASSIGNMENT = 0x2E06,   // Control Output 1 Assignment: Assignment settings for the first control output.
    OR_E5_SWR_CONTROL_OUTPUT_2_ASSIGNMENT = 0x2E07,   // Control Output 2 Assignment: Assignment settings for the second control output.
    OR_E5_SWR_EVENT_INPUT_ASSIGNMENT_1 = 0x2E0A,      // Event Input Assignment 1: Event input settings for the first input.
    OR_E5_SWR_EVENT_INPUT_ASSIGNMENT_2 = 0x2E0B,      // Event Input Assignment 2: Event input settings for the second input.
    OR_E5_SWR_EVENT_INPUT_ASSIGNMENT_3 = 0x2E0C,      // Event Input Assignment 3: Event input settings for the third input.
    OR_E5_SWR_EVENT_INPUT_ASSIGNMENT_4 = 0x2E0D,      // Event Input Assignment 4: Event input settings for the fourth input.
    OR_E5_SWR_EVENT_INPUT_ASSIGNMENT_5 = 0x2E0E,      // Event Input Assignment 5: Event input settings for the fifth input.
    OR_E5_SWR_EVENT_INPUT_ASSIGNMENT_6 = 0x2E0F,      // Event Input Assignment 6: Event input settings for the sixth input.
    OR_E5_SWR_AUXILIARY_OUTPUT_1_ASSIGNMENT = 0x2E10, // Auxiliary Output 1 Assignment: Assignment settings for auxiliary output 1.
    OR_E5_SWR_AUXILIARY_OUTPUT_2_ASSIGNMENT = 0x2E11, // Auxiliary Output 2 Assignment: Assignment settings for auxiliary output 2.
    OR_E5_SWR_AUXILIARY_OUTPUT_3_ASSIGNMENT = 0x2E12, // Auxiliary Output 3 Assignment: Assignment settings for auxiliary output 3.
    OR_E5_SWR_AUXILIARY_OUTPUT_4_ASSIGNMENT = 0x2E13, // Auxiliary Output 4 Assignment: Assignment settings for auxiliary output 4.
    OR_E5_SWR_TRANSFER_OUTPUT_UPPER_LIMIT = 0x2E14,   // Transfer Output Upper Limit: Upper limit settings for transfer output.
    OR_E5_SWR_TRANSFER_OUTPUT_LOWER_LIMIT = 0x2E15,   // Transfer Output Lower Limit: Lower limit settings for transfer output.
    OR_E5_SWR_SIMPLE_TRANSFER_OUTPUT_1_UPPER_LIMIT = 0x2E16, // Simple Transfer Output 1 Upper Limit: Upper limit for simplified transfer output 1.
    OR_E5_SWR_SIMPLE_TRANSFER_OUTPUT_1_LOWER_LIMIT = 0x2E17,

    // --- Continuation from previous entries...
    OR_E5_SWR_EXTRACTION_OF_SQUARE_ROOT_ENABLE = 0x2E24, // Extraction of Square Root Enable: Enable/disable extraction of square root.
    OR_E5_SWR_TRAVEL_TIME = 0x2E30,                      // Travel Time: Settings for the duration of travel time.
    OR_E5_SWR_ALARM_1_TYPE = 0x2F00,                     // Alarm 1 Type:

    // --- Continuation from previous entries...
    OR_E5_SWR_ALARM_1_TYPE = 0x2F00,                     // Alarm 1 Type: Type settings for the first alarm.
    OR_E5_SWR_ALARM_1_LATCH = 0x2F01,                    // Alarm 1 Latch: Latch settings for the first alarm.
    OR_E5_SWR_ALARM_1_HYSTERESIS = 0x2F02,               // Alarm 1 Hysteresis: Hysteresis settings for the first alarm.
    OR_E5_SWR_ALARM_2_TYPE = 0x2F03,                     // Alarm 2 Type: Type settings for the second alarm.
    OR_E5_SWR_ALARM_2_LATCH = 0x2F04,                    // Alarm 2 Latch: Latch settings for the second alarm.
    OR_E5_SWR_ALARM_2_HYSTERESIS = 0x2F05,               // Alarm 2 Hysteresis: Hysteresis settings for the second alarm.
    OR_E5_SWR_ALARM_3_TYPE = 0x2F06,                     // Alarm 3 Type: Type settings for the third alarm.
    OR_E5_SWR_ALARM_3_LATCH = 0x2F07,                    // Alarm 3 Latch: Latch settings for the third alarm.
    OR_E5_SWR_ALARM_3_HYSTERESIS = 0x2F08,               // Alarm 3 Hysteresis: Hysteresis settings for the third alarm.
    OR_E5_SWR_ALARM_4_TYPE = 0x2F09,                     // Alarm 4 Type: Type settings for the fourth alarm.
    OR_E5_SWR_ALARM_4_LATCH = 0x2F0A,                    // Alarm 4 Latch: Latch settings for the fourth alarm.
    OR_E5_SWR_ALARM_4_HYSTERESIS = 0x2F0B,               // Alarm 4 Hysteresis: Hysteresis settings for the fourth alarm.
    OR_E5_SWR_STANDBY_SEQUENCE_RESET = 0x2F0C,          // Standby Sequence Reset: Reset settings for the standby sequence.
    OR_E5_SWR_AUXILIARY_OUTPUT_1_OPEN_IN_ALARM = 0x2F0D, // Auxiliary Output 1 Open in Alarm: Open settings for auxiliary output 1 when in alarm.
    OR_E5_SWR_AUXILIARY_OUTPUT_2_OPEN_IN_ALARM = 0x2F0E, // Auxiliary Output 2 Open in Alarm: Open settings for auxiliary output 2 when in alarm.
    OR_E5_SWR_AUXILIARY_OUTPUT_3_OPEN_IN_ALARM = 0x2F0F, // Auxiliary Output 3 Open in Alarm: Open settings for auxiliary output 3 when in alarm.
    OR_E5_SWR_AUXILIARY_OUTPUT_4_OPEN_IN_ALARM = 0x2F10, // Auxiliary Output 4 Open in Alarm: Open settings for auxiliary output 4 when in alarm.
    OR_E5_SWR_ALARM_1_ON_DELAY = 0x2F11,                // Alarm 1 ON Delay: ON delay settings for the first alarm.
    OR_E5_SWR_ALARM_2_ON_DELAY = 0x2F12,                // Alarm 2 ON Delay: ON delay settings for the second alarm.
    OR_E5_SWR_ALARM_3_ON_DELAY = 0x2F13,                // Alarm 3 ON Delay: ON delay settings for the third alarm.
    OR_E5_SWR_ALARM_4_ON_DELAY = 0x2F14,                // Alarm 4 ON Delay: ON delay settings for the fourth alarm.
    OR_E5_SWR_ALARM_1_OFF_DELAY = 0x2F15,               // Alarm 1 OFF Delay: OFF delay settings for the first alarm.
    OR_E5_SWR_ALARM_2_OFF_DELAY = 0x2F16,               // Alarm 2 OFF Delay: OFF delay settings for the second alarm.
    OR_E5_SWR_ALARM_3_OFF_DELAY = 0x2F17,               // Alarm 3 OFF Delay: OFF delay settings for the third alarm.
    OR_E5_SWR_ALARM_4_OFF_DELAY = 0x2F18,               // Alarm 4 OFF Delay: OFF delay settings for the fourth alarm.
    OR_E5_SWR_PV_SP_NO_1_DISPLAY_SELECTION = 0x3000,    // PV/SP No. 1 Display Selection: Display settings for PV/SP No. 1.
    OR_E5_SWR_MV_DISPLAY_SELECTION = 0x3001,           // MV Display Selection: Display settings for MV.


    // --- Continuation from previous entries...
    OR_E5_SWR_AUTOMATIC_DISPLAY_RETURN_TIME = 0x3003,    // Automatic Display Return Time: Time settings for automatic display return.
    OR_E5_SWR_DISPLAY_REFRESH_PERIOD = 0x3004,           // Display Refresh Period: Settings for the refresh period of the display.
    OR_E5_SWR_PV_SP_NO_2_DISPLAY_SELECTION = 0x3008,     // PV/SP No. 2 Display Selection: Display settings for PV/SP No. 2.
    OR_E5_SWR_VALVE_OPENING_MONITOR_SELECTION = 0x3009,  // Valve Opening Monitor Selection: Selection settings for valve opening monitoring.
    OR_E5_SWR_DISPLAY_BRIGHTNESS = 0x300A,               // Display Brightness: Brightness settings for the display.
    OR_E5_SWR_MV_DISPLAY = 0x300B,                       // MV Display: Display settings for MV.
    OR_E5_SWR_MOVE_TO_PROTECT_LEVEL_TIME = 0x300C,       // Move to Protect Level Time: Time settings for moving to protect level.
    OR_E5_SWR_AUTO_MANUAL_SELECT_ADDITION = 0x300F,      // Auto/Manual Select Addition: Settings for adding auto/manual selection.
    OR_E5_SWR_PV_STATUS_DISPLAY_FUNCTION = 0x3011,       // PV Status Display Function: Function settings for displaying PV status.
    OR_E5_SWR_SV_STATUS_DISPLAY_FUNCTION = 0x3012,       // SV Status Display Function: Function settings for displaying SV status.
    OR_E5_SWR_PROTOCOL_SETTING = 0x3100,                 // Protocol Setting: Settings for communication protocol.
    OR_E5_SWR_COMMUNICATIONS_UNIT_NO = 0x3101,           // Communications Unit No.: Unit number settings for communications.
    OR_E5_SWR_COMMUNICATIONS_BAUD_RATE = 0x3102,         // Communications Baud Rate: Baud rate settings for communications.
    OR_E5_SWR_COMMUNICATIONS_DATA_LENGTH = 0x3103,       // Communications Data Length: Data length settings for communications.
    OR_E5_SWR_COMMUNICATIONS_STOP_BITS = 0x3104,         // Communications Stop Bits: Stop bit settings for communications.
    OR_E5_SWR_COMMUNICATIONS_PARITY = 0x3105,            // Communications Parity: Parity settings for communications.
    OR_E5_SWR_SEND_DATA_WAIT_TIME = 0x3106,              // Send Data Wait Time: Wait time settings for sending data.
    OR_E5_SWR_PF_SETTING = 0x3200,                       // PF Setting: Settings for PF function.
    OR_E5_SWR_MONITOR_SETTING_ITEM_1 = 0x3202,           // Monitor/Setting Item 1: Settings for monitor/item 1.
    OR_E5_SWR_MONITOR_SETTING_ITEM_2 = 0x3203,           // Monitor/Setting Item 2: Settings for monitor/item 2.
    OR_E5_SWR_MONITOR_SETTING_ITEM_3 = 0x3204,           // Monitor/Setting Item 3: Settings for monitor/item 3.
    OR_E5_SWR_MONITOR_SETTING_ITEM_4 = 0x3205,           // Monitor/Setting Item 4: Settings for monitor/item 4.
    OR_E5_SWR_MONITOR_SETTING_ITEM_5 = 0x3206,           // Monitor/Setting Item 5: Settings for monitor/item 5.
    OR_E5_SWR_SP_TRACKING = 0x3301,                      // SP Tracking: Settings for set point tracking.
    OR_E5_SWR_PV_DEAD_BAND = 0x3304,                     // PV Dead Band: Dead band settings for PV.
    OR_E5_SWR_COLD_JUNCTION_COMPENSATION_METHOD = 0x3305,// Cold Junction Compensation Method: Method settings for cold junction compensation.
    OR_E5_SWR_INTEGRAL_DERIVATIVE_TIME_UNIT = 0x3309,    // Integral/Derivative Time Unit: Time unit settings for integral/derivative.
    OR_E5_SWR_ALPHA = 0x330A,                            // Alpha: Settings for alpha parameter.
    OR_E5_SWR_MANUAL_OUTPUT_METHOD = 0x330C,             // Manual Output Method: Method settings for manual output.
    OR_E5_SWR_MANUAL_MV_INITIAL_VALUE = 0x330D,          // Manual MV Initial Value: Initial value settings for manual MV.
    OR_E5_SWR_AT_CALCULATED_GAIN = 0x330F,               // AT Calculated Gain: Gain settings for auto-tuning.
    OR_E5_SWR_AT_HYSTERESIS = 0x3310,                    // AT

        //-- Continuation from previous entries...
    OR_E5_SWR_AUTOMATIC_DISPLAY_RETURN_TIME = 0x3003,     // Automatic Display Return Time: Time settings for automatic return to the display.
    OR_E5_SWR_DISPLAY_REFRESH_PERIOD = 0x3004,            // Display Refresh Period: Settings for how often the display refreshes.
    OR_E5_SWR_PV_SP_NO_2_DISPLAY_SELECTION = 0x3008,      // PV/SP No. 2 Display Selection: Display settings for PV/SP No. 2.
    OR_E5_SWR_VALVE_OPENING_MONITOR_SELECTION = 0x3009,   // Valve Opening Monitor Selection: Selection of valve opening monitoring mode.
    OR_E5_SWR_DISPLAY_BRIGHTNESS = 0x300A,                // Display Brightness: Brightness settings for the display.
    OR_E5_SWR_MV_DISPLAY = 0x300B,                        // MV Display: Display settings for MV.
    OR_E5_SWR_MOVE_TO_PROTECT_LEVEL_TIME = 0x300C,        // Move to Protect Level Time: Time settings to move to protect level.
    OR_E5_SWR_AUTO_MANUAL_SELECT_ADDITION = 0x300F,       // Auto/Manual Select Addition: Settings for adding auto/manual selection.
    OR_E5_SWR_PV_STATUS_DISPLAY_FUNCTION = 0x3011,        // PV Status Display Function: Function settings for displaying PV status.
    OR_E5_SWR_SV_STATUS_DISPLAY_FUNCTION = 0x3012,        // SV Status Display Function: Function settings for displaying SV status.
    OR_E5_SWR_PROTOCOL_SETTING = 0x3100,                  // Protocol Setting: Settings for the communication protocol.
    OR_E5_SWR_COMMUNICATIONS_UNIT_NO = 0x3101,            // Communications Unit No.: Unit number settings for communications.
    OR_E5_SWR_COMMUNICATIONS_BAUD_RATE = 0x3102,          // Communications Baud Rate: Baud rate settings for communications.
    OR_E5_SWR_COMMUNICATIONS_DATA_LENGTH = 0x3103,        // Communications Data Length: Data length settings for communications.
    OR_E5_SWR_COMMUNICATIONS_STOP_BITS = 0x3104,          // Communications Stop Bits: Stop bit settings for communications.
    OR_E5_SWR_COMMUNICATIONS_PARITY = 0x3105,             // Communications Parity: Parity settings for communications.
    OR_E5_SWR_SEND_DATA_WAIT_TIME = 0x3106,               // Send Data Wait Time: Wait time settings for sending data.
    OR_E5_SWR_PF_SETTING = 0x3200,                        // PF Setting: Settings for PF functions.
    OR_E5_SWR_MONITOR_SETTING_ITEM_1 = 0x3202,            // Monitor/Setting Item 1: Settings for monitoring and adjustment for item 1.
    OR_E5_SWR_MONITOR_SETTING_ITEM_2 = 0x3203,            // Monitor/Setting Item 2: Settings for monitoring and adjustment for item 2.
    OR_E5_SWR_MONITOR_SETTING_ITEM_3 = 0x3204,            // Monitor/Setting Item 3: Settings for monitoring and adjustment for item 3.
    OR_E5_SWR_MONITOR_SETTING_ITEM_4 = 0x3205,            // Monitor/Setting Item 4: Settings for monitoring and adjustment for item 4.
    OR_E5_SWR_MONITOR_SETTING_ITEM_5 = 0x3206,            // Monitor/Setting Item 5: Settings for monitoring and adjustment for item 5.
    OR_E5_SWR_SP_TRACKING = 0x3301,                       // SP Tracking: Settings for tracking set points.
    OR_E5_SWR_PV_DEAD_BAND = 0x3304,                      // PV Dead Band: Settings for the PV dead band.
    OR_E5_SWR_COLD_JUNCTION_COMPENSATION_METHOD = 0x3305, // Cold Junction Compensation Method: Settings for compensating the cold junction.
    OR_E5_SWR_INTEGRAL_DERIVATIVE_TIME_UNIT = 0x3309,     // Integral/Derivative Time Unit: Time unit settings for integral and derivative calculations.
    OR_E5_SWR_ALPHA = 0x330A,                             // α: Settings for the α parameter in control equations.
    OR_E5_SWR_MANUAL_OUTPUT_METHOD = 0x330C,              // Manual Output Method: Settings for manual output methods.
    OR_E5_SWR_MANUAL_MV_INITIAL_VALUE = 0x330D,           // Manual MV Initial Value: Initial value settings for manual MV.
    OR_E5_SWR_AT_CALCULATED_GAIN = 0x330F,                // AT Calculated Gain: Settings for automatically calculated

    // Continuation from previous entries...
    OR_E5_SWR_LCT_COOLING_OUTPUT_MINIMUM_ON_TIME = 0x3335, // LCT Cooling Output Minimum ON Time: Minimum ON time settings for LCT cooling output.
    


};


#endif // OMRONE5_EX_H
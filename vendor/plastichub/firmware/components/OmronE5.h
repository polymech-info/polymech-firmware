#ifndef OMRON_E5_H
#define OMRON_E5_H

// Omron EJ5 Modbus Registers & Coils

#define OR_BIT(A) (A >> 1)
#define OR_WORD(A) (A << 4)
#define OR_E5_STATUS_BIT(H, L, B) (B <= 16 ? (L & (1 << 8)) : (OR_WORD(H) & (1 << (OR_BIT(B)))))
#define OR_E5_CMD(CMD, VALUE) (CMD | VALUE)

// Status Bit -1 , see h175_e5_c_communications_manual_en.pdf::3-24
enum OR_E5_STATUS_1
{
    // Lower Word

    OR_E5_S1_Heater_OverCurrent = 0,
    OR_E5_S1_Heater_CurrentHold = 1,
    OR_E5_S1_AD_ConverterError = 2,
    OR_E5_S1_HS_Alarm = 3,
    OR_E5_S1_RSP_InputError = 4,
    OR_E5_S1_InputError = 6,
    OR_E5_S1_PotentiometerInnputError = 7,
    OR_E5_S1_Control_OutputOpenOutput = 8,
    OR_E5_S1_Control_OutputCloseOutput = 9,
    OR_E5_S1_HBAlarmCT1 = 10,
    OR_E5_S1_HBAlarmCT2 = 11,
    OR_E5_S1_Alarm1 = 12,
    OR_E5_S1_Alarm2 = 13,
    OR_E5_S1_Alarm3 = 14,
    OR_E5_S1_ProgramEndOutput = 15,

    // Upper Word

    OR_E5_S1_EventInput1 = 16,
    OR_E5_S1_EventInput2 = 17,
    OR_E5_S1_EventInput3 = 18,
    OR_E5_S1_EventInput4 = 19,
    OR_E5_S1_WriteMode = 20,
    OR_E5_S1_NonVolatileMemory = 21,
    OR_E5_S1_SetupArea = 22,
    OR_E5_S1_ATExcecute = 23,
    OR_E5_S1_RunStop = 24,
    OR_E5_S1_ComWrite = 25,
    OR_E5_S1_AutoManualSwitch = 26,
    OR_E5_S1_ProgramStart = 27,
    OR_E5_S1_HeaterOverCurrentCT2 = 28,
    OR_E5_S1_HeaterCurrentHoldCT2 = 29,
    OR_E5_S1_HSAlarmCT2 = 31
};

// Status Bit - 2 , see h175_e5_c_communications_manual_en.pdf::3-25

enum OR_E5_STATUS_2
{
    // Lower Word

    OR_E5_S2_WorkBit1 = 0,
    OR_E5_S2_WorkBit2 = 1,
    OR_E5_S2_WorkBit3 = 2,
    OR_E5_S2_WorkBit4 = 3,
    OR_E5_S2_WorkBit5 = 4,
    OR_E5_S2_WorkBit6 = 5,
    OR_E5_S2_WorkBit7 = 6,
    OR_E5_S2_WorkBit8 = 7,

    // Upper Word

    OR_E5_S2_EventInput5 = 16,
    OR_E5_S2_EventInput6 = 17,
    OR_E5_S2_Inverse = 20,
    OR_E5_S2_SPRamp = 21,
    OR_E5_S2_SPMode = 27,
    OR_E5_S2_Alarm4 = 28
};

// Variable Area - Settings Range (0x06s) - 2 byte mode,
// see h175_e5_c_communications_manual_en.pdf::5-1

enum OR_E5_SWR
{
    //Temperature: Use the specified range for each sensor.
    // Analog: Scaling lower limit − 5% FS to Scaling upper limit + 5% FS
    OR_E5_SWR_PV = 0x2000,

    // Refer to 5-2 Status for details (see @OR_E5_STATUS_1 and @OR_E5_STATUS_2)
    OR_E5_SWR_STATUS = 0x2001,

    // Internal Set Point(see appendix *1) - SP lower limit to SP upper limit
    OR_E5_SWR_ISP = 0x2002,

    // Heater Current 1 Value Monitor, 0x00000000 to 0x00000226 (0.0 to 55.0)
    OR_E5_SWR_HeaterCurrentValue1_Monitor = 0x2003,

    // MV Monitor (Heating)
    //  Standard: 0xFFFFFFCE to 0x0000041A (−5.0 to 105.0)
    //  Heating and cooling: 0x00000000 to 0x0000041A (0.0 to 105.0)
    OR_E5_SWR_MVMonitorHeating = 0x2004,

    // MV Monitor (Cooling)
    //  0x00000000 to 0x0000041A (0.0 to 105.0)
    OR_E5_SWR_MVMonitorCooling = 0x2005,

    // Set Point -  SP lower limit to SP upper limit
    OR_E5_SWR_SP_LIMIT = 0x2103,

    // Alarm Value 1
    //   0xFFFFF831 to 0x0000270F (−1999 to 9999)
    OR_E5_SWR_ALARM_1 = 0x2104,

    // Alarm Value - Upper Limit 1
    //   0xFFFFF831 to 0x0000270F (−1999 to 9999)
    OR_E5_SWR_ALARM_1_UL = 0x2105,

    // Alarm Value - Lower Limit 1
    //   0xFFFFF831 to 0x0000270F (−1999 to 9999)
    OR_E5_SWR_ALARM_1_LL = 0x2106,

    // Alarm Value 2
    //   0xFFFFF831 to 0x0000270F (−1999 to 9999)
    OR_E5_SWR_ALARM_2 = 0x2107,

    // Alarm Value - Upper Limit 1
    //   0xFFFFF831 to 0x0000270F (−1999 to 9999)
    OR_E5_SWR_ALARM_2_UL = 0x2108,

    // Alarm Value - Lower Limit 1
    //   0xFFFFF831 to 0x0000270F (−1999 to 9999)
    OR_E5_SWR_ALARM_2_LL = 0x2109,

    //Temperature: Use the specified range for each sensor.
    // Analog: Scaling lower limit − 5% FS to Scaling upper limit + 5% FS
    OR_E5_SWR_PV2 = 0x2402,

    // Internal Set Point(see appendix *1) - SP lower limit to SP upper limit
    OR_E5_SWR_ISP2 = 0x2403,

    // Multi SP No. Monitor, 0x00000000 to 0x00000007 (0 to 7)
    OR_E5_SWR_MSMON = 0x2404,

    // Status,
    //  - Not displayed on the Controller display.
    //  - In 2-byte mode, the rightmost 16 bits are read.
    OR_E5_SWR_STATUSEX = 0x2406,

    // Status,
    //  - Not displayed on the Controller display.
    //  - In 2-byte mode, the leftmost 16 bits are read.
    OR_E5_SWR_STATUSEXL = 0x2407,

    // Status,
    //  - Not displayed on the Controller display.
    //  - In 2-byte mode, the rightmost 16 bits are read.
    OR_E5_SWR_STATUSEXR = 0x2408,

    // Decimal Point Monitor,
    // 0x00000000 to 0x00000003 (0 to 3)
    OR_E5_SWR_DECMON = 0x2410,

    // Set Point ()
    // SP lower limit to SP upper limit
    OR_E5_SWR_SP = 0x2601,

    // Remote Set Point Monitor
    // - Remote SP lower limit −10% FS to Remote SP upper limit +10% FS
    OR_E5_SWR_SP_EX_MON = 0x2602,

    // Heater Current 1 Value Monitor, 0x00000000 to 0x00000226 (0.0 to 55.0)
    OR_E5_SWR_HeaterCurrentValue1_Monitor2 = 0x2604,

    // Valve Opening Monitor, 0xFFFFFF9C to 0x0000044C (−10.0 to 110.0)
    OR_E5_SWR_VALVE_OPENING_MON = 0x2607,

    //  Proportional Band (Cooling), 0x00000001 to 0x0000270F (0.1 to 999.9)
    OR_E5_SWR_PRO_BAND = 0x2701,

    //  Integral Time (Cooling) 0x00000000 to 0x0000270F
    // (0 to 9999: Integral/derivative time unit is 1 s.)
    // (0.0 to 999.9: Integral/derivative time unit is 0.1 s.)
    OR_E5_SWR_IT_COOLING = 0x2702,

    //  Derivative Time (Cooling) 0x00000000 to 0x0000270F
    //  (0 to 9999: Integral/derivative time unit is 1 s.)
    //  (0.0 to 999.9: Integral/derivative time unit is 0.1 s.)
    OR_E5_SWR_D_COOLING = 0x2703,

    // Dead Band 0xFFFFF831 to 0x0000270F
    // (−199.9 to 999.9 for temperature input)
    // (−19.99 to 99.99 for analog input)
    OR_E5_SWR_DEADBAND = 0x2704,

    // Manual Reset Value,
    //  0x00000000 to 0x000003E8 (0.0 to 100.0)
    OR_E5_SWR_MANUAL_RESET_VALUE = 0x2705,

    // Hysteresis (Heating)
    // 0x00000001 to 0x0000270F
    // (0.1 to 999.9 for temperature input)
    // (0.01 to 99.99 for analog input)
    OR_E5_SWR_HYSTERESIS = 0x2706,

    // Hysteresis (Cooling)
    //  0x00000001 to 0x0000270F
    //  (0.1 to 999.9 for temperature input)
    //  (0.01 to 99.99 for analog input)
    OR_E5_SWR_HYSTERESIS_COOLING = 0x2707,

    // Control Period (Heating)
    //  0xFFFFFFFE (−2): 0.1 s
    //  0xFFFFFFFF (−1): 0.2 s
    //  0x00000000 (0): 0.5 s
    //  0x00000001 to 0x00000063 (1 to 99)
    OR_E5_SWR_CONTROL_PERIOD_HEATING = 0x2708,

    // Control Period (Cooling)
    //  0xFFFFFFFE (−2): 0.1 s
    //  0xFFFFFFFF (−1): 0.2 s
    //  0x00000000 (0): 0.5 s
    //  0x00000001 to 0x00000063 (1 to 99)
    OR_E5_SWR_CONTROL_PERIOD_COOLING = 0x2709,

    // Position Proportional Dead Band
    //  0x00000001 to 0x00000064 (0.1 to 10.0)
    OR_E5_SWR_POSITION_PROPORTIONAL_DEAD_BAND = 0x270A,

    // Open/Close Hysteresis
    //  0x00000001 to 0x000000C8 (0.1 to 20.0)
    OR_E5_SWR_OPEN_CLOSE_HYSTERESIS = 0x270B,

    // SP Ramp Time Unit 0x00000000 (0): EU/second
    //  0x00000001 (1): EU/minute
    //  0x00000002 (2): EU/hour
    OR_E5_SWR_SP_RAMP_UNIT = 0x270C,

    // SP Ramp Set Value 0x00000000 (0): OFF
    //  0x00000001 to 0x0000270F (1 to 9999)
    OR_E5_SWR_SP_RAMP_SET_VALUE = 0x270D,

    // SP Ramp Fall Value
    //  0xFFFFFFFF (−1): Same (Same as SP Ramp Set Value.)
    //  0x00000000 (0): OFF
    //  0x00000001 to 0x0000270F (1 to 9999)
    OR_E5_SWR_SP_FALL_VALUE = 0x270E,

    //  MV at Stop Standard Models
    //  Standard control:
    //      0xFFFFFFCE to 0x0000041A (−5.0 to 105.0)
    //  Heating and cooling control:
    //      0xFFFFFBE6 to 0x0000041A (−105.0 to 105.0)
    //  Position-proportional Models
    //      Close position-proportional control with the Direct Setting of
    //      Position Proportional MV parameter set to ON:
    //      0xFFFFFFCE to 0x0000041A (−5.0 to 105.0)
    //      Floating position-proportional control or the Direct Setting of
    //      Position Proportional MV parameter set to OFF:
    //      0xFFFFFFFF to 0x00000001 (−1 to 1)
    OR_E5_SWR_MV_PV_ERROR = 0x2711,

    // MV Change Rate Limit
    //  0x00000000 to 0x000003E8 (0.0 to 100.0)
    OR_E5_SWR_CHANGE_RATE_LIMIT = 0x2713,

    // PV Input Slope Coefficient
    //  0x00000001 to 0x0000270F (0.001 to 9.999)
    OR_E5_SWR_PV_INPUT_SLOPE_COEFFICIENT = 0x2718,

    // Heater Burnout Detection 1
    //  0x00000000 to 0x000001F4 (0.0 to 50.0)
    OR_E5_SWR_HEATER_BURNOUT_DETECTION_1 = 0x271B,

    // Leakage Current 1 Monitor
    //  0x00000000 to 0x00000226 (0.0 to 55.0)
    OR_E5_SWR_LEAKAGE_CURRENT_MONITOR_1 = 0x271C,

    // HS Alarm 1
    //  0x00000000 to 0x000001F4 (0.0 to 50.0)
    OR_E5_SWR_HS_ALARM_1 = 0x271D,

    // Process Value Input Shift
    //  0xFFFFF831 to 0x0000270F (−1999 to 9999)
    OR_E5_SWR_PROCESS_VALUE_INPUT_SHIFT = 0x2723,

    // Heater Burnout Detection 2
    //  0x00000000 to 0x000001F4 (0.0 to 50.0)
    OR_E5_SWR_HEATER_BURNOUT_DETECTION_2 = 0x2725,

    // Leakage Current 2 Monitor
    //  0x00000000 to 0x00000226 (0.0 to 55.0)
    OR_E5_SWR_LEAKAGE_CURRENT_MONITOR_2 = 0x2726,

    // HS Alarm 12
    //  0x00000000 to 0x000001F4 (0.0 to 50.0)
    OR_E5_SWR_HS_ALARM_2 = 0x2727,

    // Soak Time Remain (how lovely)
    //  0x00000000 to 0x0000270F (0 to 9999)
    OR_E5_SWR_SOAK_REMAIN = 0x2728,

    // Soak Time
    //  0x00000001 to 0x0000270F (1 to 9999)
    OR_E5_SWR_SOAK_TIME = 0x2729,

    // Wait Band 0x00000000 (0): OFF
    //  0x00000001 to 0x0000270F
    //  (0.1 to 999.9 for Temperature input)
    //  (0.01 to 99.99 for Analog input)
    OR_E5_SWR_WAIT_BAND = 0x272A,

    // Remote SP Input Shift
    //  0xFFFFF831 to 0x0000270F (−1999 to 9999)
    OR_E5_SWR_REMOTE_SP_SHIFT = 0x272B,

    // Remote SP input Slope Coefficient
    //  0x00000001 to 0x0
    OR_E5_SWR_REMOTE_SP_SLOPE_COEFFICIENT = 0x272C,

    //  Input Digital Filter 0x00000000 to 0x0000270F (0.0 to 999.9)
    OR_E5_SWR_DIGITAL_FILTER = 0x2800

    // Notes :
    // *1 Not displayed on the Controller display
};

// Operation Command Address
enum OR_E5_CMD_ADDRESS
{
    OR_E5_CMD_STOP_RUN = 0x100,
    OR_E5_CMD_COM_WRITE = 0x000,
    // Auto-Tune
    OR_E5_CMD_AT = 0x200
};

enum OR_E5_CMD
{
    OR_E5_STOP = OR_E5_CMD(OR_E5_CMD_ADDRESS::OR_E5_CMD_STOP_RUN, 1),
    OR_E5_RUN = OR_E5_CMD(OR_E5_CMD_ADDRESS::OR_E5_CMD_STOP_RUN, 0),
    OR_E5_AT_CANCEL = OR_E5_CMD(OR_E5_CMD_ADDRESS::OR_E5_CMD_AT, 0),
    OR_E5_AT_EXCECUTE = OR_E5_CMD(OR_E5_CMD_ADDRESS::OR_E5_CMD_AT, 1)
};

enum OR_E5_ERROR
{
    VARIABLE_ADDRESS_ERROR = 0x2,
    VARIABLE_RANGE_ERROR = 0x3,
    VARIABLE_OPERATION_ERROR = 0x4
};

enum OR_E5_RESPONSE_CODE
{
    OR_READ_ERROR = 0x83,
    OR_RESPONSE_OK = 0x10,
    OR_OPERATION_ERROR = 0x90,
    OR_COMMAND_ERROR = 0x86
};

#define OR_E_MSG_INVALID_ADDRESS "Invalid Variable Address"
#define OR_E_MSG_INVALID_RANGE "Invalid Variable Range"
#define OR_E_MSG_OPERATION_ERROR "OPERATION ERROR"


#endif
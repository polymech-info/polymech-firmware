enum class E_SAKO_PARAM : int
{
    // Parameter P0-01 | Name : Motor Control Mode
    // Values :
    //  0 : Sensorless,
    //  2 : V/F - Servo
    E_SAKO_PARAM_P00_01_MOTOR_CONTROL_MODE = 0xF001,

    // Parameter PA-00 | Name : PID setting source
    // Values :
    //  0 : PA-01
    //  1 : AI1
    //  2 : AI2
    //  3 : Panel potentiometer
    //  4 : HDI Pulse setting (DI5)
    //  5 : Communication setting
    //  6 : Multi-reference
    E_SAKO_PARAM_PA_00_PID_SETTING_SOURCE = 0xFA00,

    // Parameter PA-01 | Name : PID digital setting
    // Range : 0.0% ~ 100.0%
    E_SAKO_PARAM_PA_01_PID_DIGITAL_SETTING = 0xFA01,

    // Parameter PA-02 | Name : PID feedback source
    // Values :
    //  0 : AI1
    //  1 : AI2
    //  2 : Panel potentiometer
    //  3 : AI1 - AI2
    //  4 : HDI Pulse setting (DI5)
    //  5 : Communication setting
    //  6 : AI1 + AI2
    //  7 : MAX (|AI1|, |AI2|)
    //  8 : MIN (|AI1|, |AI2|)
    E_SAKO_PARAM_PA_02_PID_FEEDBACK_SOURCE = 0xFA02,

    // Parameter PA-03 | Name : PID action direction
    // Values :
    //  0 : Forward action
    //  1 : Reverse action
    E_SAKO_PARAM_PA_03_PID_ACTION_DIRECTION = 0xFA03,

    // Parameter PA-04 | Name : PID setting feedback range
    // Range : 0 ~ 65535
    E_SAKO_PARAM_PA_04_PID_SETTING_FEEDBACK_RANGE = 0xFA04,

    // Parameter PA-05 | Name : Proportional gain Kp1
    // Range : 0.0 ~ 100.0
    E_SAKO_PARAM_PA_05_PROPORTIONAL_GAIN_KP1 = 0xFA05,

    // Parameter PA-06 | Name : Integral time Ti1
    // Range : 0.01s ~ 10.00s
    E_SAKO_PARAM_PA_06_INTEGRAL_TIME_TI1 = 0xFA06,

    // Parameter PA-07 | Name : Differential time Td1
    // Range : 0.000s ~ 10.000s
    E_SAKO_PARAM_PA_07_DIFFERENTIAL_TIME_TD1 = 0xFA07,

    // Parameter PA-08 | Name : Cut-off frequency of PID reverse rotation
    // Range : 0.00 ~ maximum frequency
    E_SAKO_PARAM_PA_08_CUT_OFF_FREQUENCY_OF_PID_REVERSE_ROTATION = 0xFA08,

    // Parameter PA-09 | Name : PID deviation limit
    // Range : 0.0% ~ 100.0%
    E_SAKO_PARAM_PA_09_PID_DEVIATION_LIMIT = 0xFA09,

    // Parameter PA-10 | Name : PID differential limit
    // Range : 0.00% ~ 100.00%
    E_SAKO_PARAM_PA_10_PID_DIFFERENTIAL_LIMIT = 0xFA0A,

    // Parameter PA-11 | Name : PID setting change time
    // Range : 0.00 ~ 650.00s
    E_SAKO_PARAM_PA_11_PID_SETTING_CHANGE_TIME = 0xFA0B,

    // Parameter PA-12 | Name : PID feedback filter time
    // Range : 0.00 ~ 60.00s
    E_SAKO_PARAM_PA_12_PID_FEEDBACK_FILTER_TIME = 0xFA0C,

    // Parameter PA-13 | Name : PID output filter time
    // Range : 0.00 ~ 60.00s
    E_SAKO_PARAM_PA_13_PID_OUTPUT_FILTER_TIME = 0xFA0D,

    // Parameter PA-15 | Name : Proportional gain Kp2
    // Range : 0.0 ~ 100.0
    E_SAKO_PARAM_PA_15_PROPORTIONAL_GAIN_KP2 = 0xFA0F,

    // Parameter PA-16 | Name : Integral time Ti2
    // Range : 0.01s ~ 10.00s
    E_SAKO_PARAM_PA_16_INTEGRAL_TIME_TI2 = 0xFA10,

    // Parameter PA-17 | Name : Differential time Td2
    // Range : 0.000s ~ 10.000s
    E_SAKO_PARAM_PA_17_DIFFERENTIAL_TIME_TD2 = 0xFA11,

    // Parameter PA-18 | Name : PID parameter switchover condition
    // Values :
    //  0 : No switchover
    //  1 : Switchover via DI
    //  2 : Automatic switchover based on deviation
    //  3 : Automatic switchover based on running frequency
    E_SAKO_PARAM_PA_18_PID_PARAMETER_SWITCHOVER_CONDITION = 0xFA12,

    // Parameter PA-19 | Name : PID parameter switchover deviation 1
    // Range : 0.0% ~ PA-20
    E_SAKO_PARAM_PA_19_PID_PARAMETER_SWITCHOVER_DEVIATION_1 = 0xFA13,

    // Parameter PA-20 | Name : PID parameter switchover deviation 2
    // Range : PA-19 ~ 100.0%
    E_SAKO_PARAM_PA_20_PID_PARAMETER_SWITCHOVER_DEVIATION_2 = 0xFA14,

    // Parameter PA-21 | Name : PID initial value
    // Range : 0.0% ~ 100.0%
    E_SAKO_PARAM_PA_21_PID_INITIAL_VALUE = 0xFA15,

    // Parameter PA-22 | Name : PID initial value holding time
    // Range : 0.00 ~ 650.00s
    E_SAKO_PARAM_PA_22_PID_INITIAL_VALUE_HOLDING_TIME = 0xFA16,

    // Parameter PA-23 | Name : Maximum deviation between two PID outputs in forward direction
    // Range : 0.00% ~ 100.00%
    E_SAKO_PARAM_PA_23_MAXIMUM_DEVIATION_BETWEEN_TWO_PID_OUTPUTS_IN_FORWARD_DIRECTION = 0xFA17,

    // Parameter PA-24 | Name : Maximum deviation between two PID outputs in reverse direction
    // Range : 0.00% ~ 100.00%
    E_SAKO_PARAM_PA_24_MAXIMUM_DEVIATION_BETWEEN_TWO_PID_OUTPUTS_IN_REVERSE_DIRECTION = 0xFA18,

    // Parameter PA-25 | Name : PID integral property
    // Values (Unit's digit: Integral separated, Ten's digit: Whether to stop integral operation when output reaches limit) :
    //  Unit's digit:
    //   0 : Invalid
    //   1 : Valid
    //  Ten's digit:
    //   0 : Continue integral operation
    //   1 : Stop integral operation
    //  (e.g., 00 = Invalid, Continue; 01 = Valid, Continue; 10 = Invalid, Stop; 11 = Valid, Stop)
    E_SAKO_PARAM_PA_25_PID_INTEGRAL_PROPERTY = 0xFA19,

    // Parameter PA-26 | Name : Detection value of PID feedback loss
    // Range : 0.0%: Not judging feedback loss, 0.1% ~ 100.0%
    E_SAKO_PARAM_PA_26_DETECTION_VALUE_OF_PID_FEEDBACK_LOSS = 0xFA1A,

    // Parameter PA-27 | Name : Detection time of PID feedback loss
    // Range : 0.0s ~ 20.0s
    E_SAKO_PARAM_PA_27_DETECTION_TIME_OF_PID_FEEDBACK_LOSS = 0xFA1B,

    // Parameter PA-28 | Name : PID operation at stop
    // Values :
    //  0 : No PID operation at stop
    //  1 : PID operation at stop
    E_SAKO_PARAM_PA_28_PID_OPERATION_AT_STOP = 0xFA1C,

};
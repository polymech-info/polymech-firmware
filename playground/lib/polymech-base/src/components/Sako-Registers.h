enum class E_SAKO_PARAM : int
{
    // Parameter P0-01 | Name : Motor Control Mode
    // Values :
    //  0 : Sensorless,
    //  2 : V/F - Servo
    E_SAKO_PARAM_P00_01_MOTOR_CONTROL_MODE = 0xF001,

    // Parameter P0-02 | Name : Command source selection
    // Values :
    //  0 : Operation panel control (LED off),
    //  1 : Terminal control (LED on),
    //  2 : Communication control (LED blinking)
    E_SAKO_PARAM_P00_02_COMMAND_SOURCE_SELECTION = 0xF002,

    // Parameter P0-03 | Name : Main frequency source X selection
    // Values :
    //  0 : Digital setting (preset frequency P0-08, press UP/DOWN to modify, non-retentive at power failure),
    //  1 : Digital setting (preset frequency P0-08, press UP/DOWN to modify, retentive at power failure),
    //  2 : AI1,
    //  3 : Panel potentiometer,
    //  4 : External panel potentiometer,
    //  5 : HDI pulse setting (DI5),
    //  6 : Multi-command,
    //  7 : Simple PLC,
    //  8 : PID,
    //  9 : Communication setting
    E_SAKO_PARAM_P00_03_MAIN_FREQUENCY_SOURCE_X_SELECTION = 0xF003,

    // Parameter P0-04 | Name : Auxiliary frequency source Y selection
    // Values :
    //  0 : Digital setting (preset frequency P0-08, press UP/DOWN to modify, non-retentive at power failure),
    //  1 : Digital setting (preset frequency P0-08, press UP/DOWN to modify, retentive at power failure),
    //  2 : AI1,
    //  3 : Panel potentiometer,
    //  4 : External panel potentiometer,
    //  5 : HDI pulse setting (DI5),
    //  6 : Multi-command,
    //  7 : Simple PLC,
    //  8 : PID,
    //  9 : Communication setting
    E_SAKO_PARAM_P00_04_AUXILIARY_FREQUENCY_SOURCE_Y_SELECTION = 0xF004,

    // Parameter P0-05 | Name : Selection of Y range of auxiliary frequency source in superposition
    // Values :
    //  0 : Relative to maximum frequency,
    //  1 : Relative to main frequency X
    E_SAKO_PARAM_P00_05_SELECTION_OF_Y_RANGE_OF_AUXILIARY_FREQUENCY_SOURCE_IN_SUPERPOSITION = 0xF005,

    // Parameter P0-06 | Name : Selection of Y range of auxiliary frequency source in superposition
    // Values :
    //  This parameter accepts a value from 0 to 150,
    //  representing a percentage from 0% to 150%.
    // Note: The name appears to be a duplicate of P0-05 in the provided text, but assuming it's distinct as per P0-06.
    // Given the description for P0-06 in the original text, the name might be "Auxiliary frequency Y gain in superposition" or similar.
    // For consistency, I'm using the name as provided: "Selection of Y range of auxiliary frequency source in superposition"
    E_SAKO_PARAM_P00_06_AUXILIARY_FREQUENCY_Y_GAIN_IN_SUPERPOSITION = 0xF006, // Adjusted name based on typical use for a percentage range

    // Parameter P0-07 | Name : Frequency source superposition selection
    // Values :
    //  Unit's digit (Frequency source selection)
    //   0 : Main frequency source X
    //   1 : X and Y operation (operation relationship determined by ten's digit)
    //   2 : Switchover between X and Y
    //   3 : Switchover between X and "X and Y operation"
    //   4 : Switchover between Y and "X and Y operation"
    //  Ten's digit (X and Y operation relationship)
    //   0 : X+Y
    //   1 : X-Y
    //   2 : Maximum
    //   3 : Minimum
    E_SAKO_PARAM_P00_07_FREQUENCY_SOURCE_SUPERPOSITION_SELECTION = 0xF007,

    // Parameter P0-08 | Name : Preset frequency
    // Values :
    //  This parameter accepts a value from 0.00Hz to maximum frequency (P0-10).
    E_SAKO_PARAM_P00_08_PRESET_FREQUENCY = 0xF008,

    // Parameter P0-09 | Name : Rotation direction
    // Values :
    //  0 : Same direction
    //  1 : Reverse direction
    E_SAKO_PARAM_P00_09_ROTATION_DIRECTION = 0xF009,

    // Parameter P0-10 | Name : Maximum frequency
    // Values :
    //  This parameter accepts a value from 5.00Hz to 500.00Hz.
    E_SAKO_PARAM_P00_10_MAXIMUM_FREQUENCY = 0xF00A,

    // Parameter P0-11 | Name : Source of frequency upper limit
    // Values :
    //  0 : Set by P0-12
    //  1 : AI1
    //  2 : AI2 local potentiometer
    //  3 : AI3 panel potentiometer external keyboard potentiometer
    //  4 : HDI pulse setting
    //  5 : Communication setting
    E_SAKO_PARAM_P00_11_SOURCE_OF_FREQUENCY_UPPER_LIMIT = 0xF00B,

    // Parameter P0-12 | Name : Frequency upper limit
    // Values :
    //  This parameter sets the frequency upper limit, ranging from Frequency lower limit (P0-14) to maximum frequency (P0-10).
    E_SAKO_PARAM_P00_12_FREQUENCY_UPPER_LIMIT = 0xF00C,

    // Parameter P0-13 | Name : Frequency upper limit offset
    // Values :
    //  This parameter accepts a value from 0.00Hz to maximum frequency (P0-10).
    E_SAKO_PARAM_P00_13_FREQUENCY_UPPER_LIMIT_OFFSET = 0xF00D,

    // Parameter P0-14 | Name : Frequency lower limit
    // Values :
    //  This parameter sets the frequency lower limit, ranging from 0.00Hz to frequency upper limit (P0-12).
    E_SAKO_PARAM_P00_14_FREQUENCY_LOWER_LIMIT = 0xF00E,

    // Parameter P0-15 | Name : Carrier frequency
    // Values :
    //  This parameter accepts a value from 2.0kHz to 8.0kHz.
    E_SAKO_PARAM_P00_15_CARRIER_FREQUENCY = 0xF00F,

    // Parameter P0-16 | Name : Carrier frequency adjustment with temperature
    // Values :
    //  0 : No,
    //  1 : Yes
    E_SAKO_PARAM_P00_16_CARRIER_FREQUENCY_ADJUSTMENT_WITH_TEMPERATURE = 0xF010,

    // Parameter P0-17 | Name : Acceleration time 1
    // Values :
    //  0.00s ~ 650.00s (when P0-19=2),
    //  0.0s ~ 6500.0s (when P0-19=1),
    //  0s ~ 65000s (when P0-19=0)
    E_SAKO_PARAM_P00_17_ACCELERATION_TIME_1 = 0xF011,

    // Parameter P0-18 | Name : Deceleration time 1
    // Values :
    //  0.00s ~ 650.00s (when P0-19=2),
    //  0.0s ~ 6500.0s (when P0-19=1),
    //  0s ~ 65000s (when P0-19=0)
    E_SAKO_PARAM_P00_18_DECELERATION_TIME_1 = 0xF012,

    // Parameter P0-19 | Name : Acceleration/Deceleration time unit
    // Values :
    //  0 : 1s,
    //  1 : 0.1s,
    //  2 : 0.01s
    E_SAKO_PARAM_P00_19_ACCELERATION_DECELERATION_TIME_UNIT = 0xF013,

    // Parameter P0-21 | Name : Frequency offset of auxiliary frequency source for X and Y operation
    // Values :
    //  This parameter accepts a value from 0.00Hz to maximum frequency (P0-10).
    E_SAKO_PARAM_P00_21_FREQUENCY_OFFSET_OF_AUXILIARY_FREQUENCY_SOURCE_FOR_X_AND_Y_OPERATION = 0xF015,

    // Parameter P0-22 | Name : Frequency reference resolution
    // Values :
    //  2 : 0.01Hz
    E_SAKO_PARAM_P00_22_FREQUENCY_REFERENCE_RESOLUTION = 0xF016,

    // Parameter P0-23 | Name : Retentive of digital setting frequency upon power failure
    // Values :
    //  0 : Not retentive,
    //  1 : Retentive
    E_SAKO_PARAM_P00_23_RETENTIVE_OF_DIGITAL_SETTING_FREQUENCY_UPON_POWER_FAILURE = 0xF017,

    // Parameter P0-25 | Name : Acceleration/Deceleration time base frequency
    // Values :
    //  0 : Maximum frequency (P0-10),
    //  1 : Set frequency,
    //  2 : 100 Hz
    E_SAKO_PARAM_P00_25_ACCELERATION_DECELERATION_TIME_BASE_FREQUENCY = 0xF019,

    // Parameter P0-26 | Name : Base frequency for UP/DOWN modification during running
    // Values :
    //  0 : Running frequency,
    //  1 : Set frequency
    E_SAKO_PARAM_P00_26_BASE_FREQUENCY_FOR_UP_DOWN_MODIFICATION_DURING_RUNNING = 0xF01A,

    // Parameter P0-27 | Name : Binding command source to frequency source
    // Values :
    //  Unit's digit (Binding operation panel command to frequency source)
    //   0 : No binding
    //   1 : Frequency source by digital setting
    //   2 : AI1
    //   3 : AI2
    //   4 : Panel potentiometer external keyboard potentiometer
    //   5 : HDI Pulse setting (DI5)
    //   6 : Multi-command
    //   7 : Simple PLC
    //   8 : PID
    //   9 : Communication setting
    //  Ten's digit (Binding terminal command to frequency source) - (Refer to manual for specific values)
    //  Hundred's digit (Binding communication command to frequency source) - (Refer to manual for specific values)
    E_SAKO_PARAM_P00_27_BINDING_COMMAND_SOURCE_TO_FREQUENCY_SOURCE = 0xF01B,

    ////////////////////////////////////////////////////////////////////////////
    // P1-00 Motor Parameters
    ////////////////////////////////////////////////////////////////////////////

    // Parameter P1-00 | Name : Motor type selection
    // Values :
    //  0 : Common asynchronous motor
    //  2 : Permanent magnetic synchronous motor
    E_SAKO_PARAM_P1_00_MOTOR_TYPE_SELECTION = 0xF100,

    // Parameter P1-01 | Name : Rated motor power
    // Setting Range : 0.1kW ~ 1000.0kW
    E_SAKO_PARAM_P1_01_RATED_MOTOR_POWER = 0xF101,

    // Parameter P1-02 | Name : Rated motor voltage
    // Setting Range : 1V ~ 2000V
    E_SAKO_PARAM_P1_02_RATED_MOTOR_VOLTAGE = 0xF102,

    // Parameter P1-03 | Name : Rated motor current
    // Setting Range : 0.01A ~ 10.00A (AC drive power <= 2.2kW)
    E_SAKO_PARAM_P1_03_RATED_MOTOR_CURRENT = 0xF103,

    // Parameter P1-04 | Name : Rated motor frequency
    // Setting Range : 0.01Hz ~ maximum frequency
    E_SAKO_PARAM_P1_04_RATED_MOTOR_FREQUENCY = 0xF104,

    // Parameter P1-05 | Name : Rated motor rotational speed
    // Setting Range : 1rpm ~ 65535rpm
    E_SAKO_PARAM_P1_05_RATED_MOTOR_ROTATIONAL_SPEED = 0xF105,

    // Parameter P1-10 | Name : No-load current (asynchronous motor)
    // Setting Range : 0.01A ~ P1-03
    E_SAKO_PARAM_P1_10_NO_LOAD_CURRENT_ASYNCHRONOUS_MOTOR = 0xF110,

    // Parameter P1-37 | Name : Auto-tuning selection
    // Values :
    //  0 : No auto-tuning
    //  1 : Asynchronous motor static auto-tuning
    //  2 : Asynchronous motor complete auto-tuning
    E_SAKO_PARAM_P1_37_AUTO_TUNING_SELECTION = 0xF137,

    ////////////////////////////////////////////////////////////////////////////
    // P2-00 Motor Parameters
    ////////////////////////////////////////////////////////////////////////////

    // Parameter P2-00 | Name : Speed loop proportional gain 1
    E_SAKO_PARAM_P02_00_SPEED_LOOP_PROPORTIONAL_GAIN_1 = 0xF200,

    // Parameter P2-01 | Name : Speed loop integral time 1
    E_SAKO_PARAM_P02_01_SPEED_LOOP_INTEGRAL_TIME_1 = 0xF201,

    // Parameter P2-02 | Name : Switchover frequency 1
    E_SAKO_PARAM_P02_02_SWITCHOVER_FREQUENCY_1 = 0xF202,

    // Parameter P2-03 | Name : Speed loop proportional gain 2
    E_SAKO_PARAM_P02_03_SPEED_LOOP_PROPORTIONAL_GAIN_2 = 0xF203,

    // Parameter P2-04 | Name : Speed loop integral time 2
    E_SAKO_PARAM_P02_04_SPEED_LOOP_INTEGRAL_TIME_2 = 0xF204,

    // Parameter P2-05 | Name : Switchover frequency 2
    E_SAKO_PARAM_P02_05_SWITCHOVER_FREQUENCY_2 = 0xF205,

    // Parameter P2-06 | Name : Vector control slip gain
    E_SAKO_PARAM_P02_06_VECTOR_CONTROL_SLIP_GAIN = 0xF206,

    // Parameter P2-07 | Name : Time constant of speed loop filter
    E_SAKO_PARAM_P02_07_TIME_CONSTANT_OF_SPEED_LOOP_FILTER = 0xF207,

    // Parameter P2-09 | Name : Torque upper limit source in speed control mode
    // Values :
    //   0 : Function code setting at P2-10
    //   1 : AI1
    //   2 : AI2
    //   3 : Panel potentiometer external keyboard potentiometer
    //   4 : HDI Pulse setting
    //   5 : Communication setting
    //   6 : MIN(AI1, AI2)
    //   7 : MAX(AI1, AI2)
    // Note: The table also states "1-7 The full range of options corresponds to P2-10" under the P2-09 setting range.
    E_SAKO_PARAM_P02_09_TORQUE_UPPER_LIMIT_SOURCE_IN_SPEED_CONTROL_MODE = 0xF209,

    // Parameter P2-10 | Name : Digital setting of torque upper limit in speed control mode
    E_SAKO_PARAM_P02_10_DIGITAL_SETTING_OF_TORQUE_UPPER_LIMIT_IN_SPEED_CONTROL_MODE = 0xF210,

    // Parameter P2-13 | Name : Excitation adjustment proportional gain
    E_SAKO_PARAM_P02_13_EXCITATION_ADJUSTMENT_PROPORTIONAL_GAIN = 0xF213,

    // Parameter P2-14 | Name : Excitation adjustment integral gain
    E_SAKO_PARAM_P02_14_EXCITATION_ADJUSTMENT_INTEGRAL_GAIN = 0xF214,

    // Parameter P2-15 | Name : Torque adjustment proportional gain
    E_SAKO_PARAM_P02_15_TORQUE_ADJUSTMENT_PROPORTIONAL_GAIN = 0xF215,

    // Parameter P2-16 | Name : Torque adjustment integral gain
    E_SAKO_PARAM_P02_16_TORQUE_ADJUSTMENT_INTEGRAL_GAIN = 0xF216,

    // Parameter P2-17 | Name : Speed loop integral property
    // Note : P2-17's unit digit sets integral separation.
    // Values :
    //  0 : Disabled
    //  1 : Enabled
    E_SAKO_PARAM_P02_17_SPEED_LOOP_INTEGRAL_PROPERTY = 0xF217,

    // Parameter P2-20 | Name : Maximum output voltage coefficient
    E_SAKO_PARAM_P02_20_MAXIMUM_OUTPUT_VOLTAGE_COEFFICIENT = 0xF220,

    // Parameter P2-21 | Name : Maximum torque coefficient in weak magnetic field
    E_SAKO_PARAM_P02_21_MAXIMUM_TORQUE_COEFFICIENT_IN_WEAK_MAGNETIC_FIELD = 0xF221,

    // Parameter P3-00 | Name : VF curve setting
    // Values :
    //   0 : Linear V/F
    //   1 : Multi-point V/F
    //   2 : Square V/F
    //   3 : 1.2 power V/F
    //   4 : 1.4 power V/F
    //   6 : 1.6 power V/F
    //   8 : 1.8 power V/F
    //   9 : Reserved
    //  10 : V/F complete separation
    //  11 : V/F half separation
    E_SAKO_PARAM_P03_00_VF_CURVE_SETTING = 0xF300,

    // Parameter P3-01 | Name : Torque boost
    E_SAKO_PARAM_P03_01_TORQUE_BOOST = 0xF301,

    // Parameter P3-02 | Name : Cut-off frequency of torque boost
    E_SAKO_PARAM_P03_02_CUT_OFF_FREQUENCY_OF_TORQUE_BOOST = 0xF302,

    // Parameter P3-03 | Name : Multi-point V/F frequency 1
    E_SAKO_PARAM_P03_03_MULTI_POINT_V_F_FREQUENCY_1 = 0xF303,

    // Parameter P3-04 | Name : Multi-point V/F voltage 1
    E_SAKO_PARAM_P03_04_MULTI_POINT_V_F_VOLTAGE_1 = 0xF304,

    // Parameter P3-05 | Name : Multi-point V/F frequency 2
    E_SAKO_PARAM_P03_05_MULTI_POINT_V_F_FREQUENCY_2 = 0xF305,

    // Parameter P3-06 | Name : Multi-point V/F voltage 2
    E_SAKO_PARAM_P03_06_MULTI_POINT_V_F_VOLTAGE_2 = 0xF306,

    // Parameter P3-07 | Name : Multi-point V/F frequency 3
    E_SAKO_PARAM_P03_07_MULTI_POINT_V_F_FREQUENCY_3 = 0xF307,

    // Parameter P3-08 | Name : Multi-point V/F voltage 3
    E_SAKO_PARAM_P03_08_MULTI_POINT_V_F_VOLTAGE_3 = 0xF308,

    // Parameter P3-09 | Name : V/F slip compensation gain
    E_SAKO_PARAM_P03_09_V_F_SLIP_COMPENSATION_GAIN = 0xF309,

    // Parameter P3-10 | Name : VF over-excitation gain
    E_SAKO_PARAM_P03_10_VF_OVER_EXCITATION_GAIN = 0xF310,

    // Parameter P3-11 | Name : VF oscillation suppression gain
    E_SAKO_PARAM_P03_11_VF_OSCILLATION_SUPPRESSION_GAIN = 0xF311,

    ////////////////////////////////////////////////////////////////////////////
    // P4-00 Motor Parameters
    ////////////////////////////////////////////////////////////////////////////
    // Parameter P4-00 | Name : DI1 terminal function selection
    // Values :
    //  0 : No function
    //  1 : Forward RUN (FWD) or RUN
    //  2 : Reverse RUN (REV) or RUN direction
    //  3 : Three-line control
    //  4 : Forward JOG (FJOG)
    //  5 : Reverse JOG (RJOG)
    //  6 : Terminal UP
    //  7 : Terminal DOWN
    //  8 : Coast to stop
    //  9 : Fault reset (RESET)
    E_SAKO_PARAM_P4_00_DI1_TERMINAL_FUNCTION_SELECTION = 0xF400,

    // Parameter P4-01 | Name : DI2 terminal function selection
    // Values :
    //  10 : RUN pause
    //  11 : Normally open (NO) input of external fault
    //  12 : Multi-reference terminal 1
    //  13 : Multi-reference terminal 2
    //  14 : Multi-reference terminal 3
    //  15 : Multi-reference terminal 4
    //  16 : Terminal 1 for acceleration/deceleration time selection
    //  17 : Terminal 2 for acceleration/deceleration time selection
    E_SAKO_PARAM_P4_01_DI2_TERMINAL_FUNCTION_SELECTION = 0xF401,

    // Parameter P4-02 | Name : DI3 terminal function selection
    // Values :
    //  18 : Frequency source switchover
    //  19 : UP and DOWN setting clear (terminal, operation panel)
    //  20 : Command source switchover terminal 1
    //  21 : Acceleration/Deceleration prohibited
    E_SAKO_PARAM_P4_02_DI3_TERMINAL_FUNCTION_SELECTION = 0xF402,

    // Parameter P4-03 | Name : DI4 terminal function selection
    // Values :
    //  22 : PID pause
    //  23 : PLC status reset
    //  24 : Swing pause
    //  25 : Counter input
    //  26 : Counter reset
    E_SAKO_PARAM_P4_03_DI4_TERMINAL_FUNCTION_SELECTION = 0xF403,
    // Parameter P4-04 | Name : DI5 terminal function selection
    // Values :
    //  27 : Length count input
    //  28 : Length reset
    //  29 : Torque control prohibited
    //  30 : Pulse input (enabled only for DI5)
    //  31 : Reserved
    //  32 : Immediate DC braking
    //  33 : Normally closed (NC) input of external fault
    //  34 : Frequency modification enable
    //  35 : Reverse PID action direction
    //  36 : External STOP terminal 1
    //  37 : Command source switchover terminal 2
    //  38 : PID integral pause
    //  39 : Switchover between main frequency source X and preset frequency
    //  40 : Switchover between auxiliary frequency source Y and preset frequency
    //  41 : Reserved
    //  42 : Reserved
    //  43 : PID parameter switchover
    //  44 : User-defined fault 1
    //  45 : User-defined fault 2
    //  46 : Speed control/Torque control switchover
    //  47 : Emergency stop
    //  48 : External STOP terminal 2
    //  49 : Deceleration DC braking
    //  50 : Clear the current running time
    //  51-59 : Reserved
    // Default : 12
    E_SAKO_PARAM_P04_04_DI5_TERMINAL_FUNCTION_SELECTION = 0xF404,

    // Parameter P4-10 | Name : DI filter time
    // Setting Range : 0.000s ~ 1.000s
    // Default : 0.01s
    E_SAKO_PARAM_P04_10_DI_FILTER_TIME = 0xF410,

    // Parameter P4-11 | Name : Terminal command mode
    // Values :
    //  0 : Two-line mode 1
    //  1 : Two-line mode 2
    //  2 : Three-line mode 1
    // Default : 0
    E_SAKO_PARAM_P04_11_TERMINAL_COMMAND_MODE = 0xF411,

    // Parameter P4-12 | Name : Terminal UP/DOWN rate
    // Setting Range : 0.001Hz/s ~ 65.535Hz/s
    // Default : 1.00Hz/s
    E_SAKO_PARAM_P04_12_TERMINAL_UP_DOWN_RATE = 0xF412,

    // Parameter P4-13 | Name : Al curve 1 minimum input
    // Setting Range : 0.00V ~ P4-15
    // Default : 0.00V
    E_SAKO_PARAM_P04_13_AL_CURVE_1_MINIMUM_INPUT = 0xF413,

    // Parameter P4-14 | Name : Corresponding setting of Al curve 1 minimum input
    // Setting Range : -100.0% ~ +100.0%
    // Default : 0.0%
    E_SAKO_PARAM_P04_14_CORRESPONDING_SETTING_OF_AL_CURVE_1_MINIMUM_INPUT = 0xF414,

    // Parameter P4-15 | Name : Al curve 1 maximum input
    // Setting Range : P4-13 ~ +10.00V
    // Default : 10.00V
    E_SAKO_PARAM_P04_15_AL_CURVE_1_MAXIMUM_INPUT = 0xF415,

    // Parameter P4-16 | Name : Corresponding setting of Al curve 1 maximum input
    // Setting Range : -100.0% ~ +100.0%
    // Default : 100.0%
    E_SAKO_PARAM_P04_16_CORRESPONDING_SETTING_OF_AL_CURVE_1_MAXIMUM_INPUT = 0xF416,

    // Parameter P4-17 | Name : All filter time
    // Setting Range : 0.00s ~ 10.00s
    // Default : 0.10s
    // Note : "All" might be a typo for "Al1" or "AI1" based on P4-22 (Al2 filter time).
    E_SAKO_PARAM_P04_17_ALL_FILTER_TIME = 0xF417, // Assuming "All" refers to AI1 based on context of P4-22. If it's a global AI filter, name might need adjustment.

    // Parameter P4-18 | Name : Al curve 2 minimum input
    // Setting Range : 0.00V ~ P4-20
    // Default : 0.00V
    E_SAKO_PARAM_P04_18_AL_CURVE_2_MINIMUM_INPUT = 0xF418,

    // Parameter P4-19 | Name : Corresponding setting of Al curve 2 minimum input
    // Setting Range : -100.0% ~ +100.0%
    // Default : 0.0%
    E_SAKO_PARAM_P04_19_CORRESPONDING_SETTING_OF_AL_CURVE_2_MINIMUM_INPUT = 0xF419,

    // Parameter P4-20 | Name : Al curve 2 maximum input
    // Setting Range : P4-18 ~ +10.00V
    // Default : 10.00V
    E_SAKO_PARAM_P04_20_AL_CURVE_2_MAXIMUM_INPUT = 0xF420,

    // Parameter P4-21 | Name : Corresponding setting of Al curve 2 maximum input
    // Setting Range : -100.0% ~ +100.0%
    // Default : 10.00V
    // Note : Default value "10.00V" for a percentage range is likely a typo in the source document, expected a percentage like "100.0%".
    E_SAKO_PARAM_P04_21_CORRESPONDING_SETTING_OF_AL_CURVE_2_MAXIMUM_INPUT = 0xF421,

    // Parameter P4-22 | Name : Al2 filter time
    // Setting Range : 0.00s ~ 10.00s
    // Default : 0.10s
    E_SAKO_PARAM_P04_22_AL2_FILTER_TIME = 0xF422,

    // Parameter P4-23 | Name : Al curve 3 minimum input
    // Setting Range : -10.00V ~ P4-25
    // Default : -10.00V
    E_SAKO_PARAM_P04_23_AL_CURVE_3_MINIMUM_INPUT = 0xF423,

    // Parameter P4-24 | Name : Corresponding setting of Al curve 3 minimum input
    // Setting Range : -100.0% ~ +100.0%
    // Default : -100.0%
    E_SAKO_PARAM_P04_24_CORRESPONDING_SETTING_OF_AL_CURVE_3_MINIMUM_INPUT = 0xF424,

    // Parameter P4-25 | Name : Al curve 3 maximum input
    // Setting Range : P4-23 ~ +10.00V
    // Default : 10.00V
    E_SAKO_PARAM_P04_25_AL_CURVE_3_MAXIMUM_INPUT = 0xF425,

    // Parameter P4-26 | Name : Corresponding setting of Al curve 3 maximum input
    // Setting Range : -100.0% ~ +100.0%
    // Default : 100.0%
    E_SAKO_PARAM_P04_26_CORRESPONDING_SETTING_OF_AL_CURVE_3_MAXIMUM_INPUT = 0xF426,

    // Parameter P4-27 | Name : Panel potentiometer filter time
    // Setting Range : 0.00s ~ 10.00s
    // Default : 0.10s
    E_SAKO_PARAM_P04_27_PANEL_POTENTIOMETER_FILTER_TIME = 0xF427,

    // Parameter P4-28 | Name : HDI Pulse minimum input
    // Setting Range : 0.00kHz ~ P4-30
    // Default : 0.00kHz
    E_SAKO_PARAM_P04_28_HDI_PULSE_MINIMUM_INPUT = 0xF428,

    // Parameter P4-29 | Name : Corresponding setting of HDI minimum input
    // Setting Range : -100.0% ~ 100.0%
    // Default : 0.0%
    E_SAKO_PARAM_P04_29_CORRESPONDING_SETTING_OF_HDI_MINIMUM_INPUT = 0xF429,

    // Parameter P4-30 | Name : HDI maximum input
    // Setting Range : P4-28 ~ 100.00kHz
    // Default : 50.00kHz
    E_SAKO_PARAM_P04_30_HDI_MAXIMUM_INPUT = 0xF430,

    // Parameter P4-31 | Name : Corresponding setting of HDI pulse maximum input
    // Setting Range : -100.0% ~ 100.0%
    // Default : 100.0%
    E_SAKO_PARAM_P04_31_CORRESPONDING_SETTING_OF_HDI_PULSE_MAXIMUM_INPUT = 0xF431,

    // Parameter P4-32 | Name : HDI filter time
    // Setting Range : 0.00s ~ 10.00s
    // Default : 0.10s
    E_SAKO_PARAM_P04_32_HDI_FILTER_TIME = 0xF432,

    // Parameter P4-33 | Name : Al curve selection
    // Values :
    //  The parameter value is an integer (e.g., H T U = Hundreds Tens Units).
    //  U (Unit's digit) : AI1 curve selection
    //   1 : Curve 1 (2 points, see P4-13 to F4-16)
    //   2 : Curve 2 (2 points, see P4-18 to F4-21)
    //   3 : Curve 3 (2 points, see P4-23 to F4-26)
    //   4 : Curve 4 (4 points, see A6-00 to A6-07)
    //   5 : Curve 5 (4 points, see A6-08 to A6-15)
    //  T (Ten's digit) : AI2 curve selection (curve definitions are same as for AI1)
    //   1 : Curve 1
    //   2 : Curve 2
    //   3 : Curve 3
    //   4 : Curve 4
    //   5 : Curve 5
    //  H (Hundred's digit) : AI3 curve selection (curve definitions are same as for AI1)
    //   1 : Curve 1
    //   2 : Curve 2
    //   3 : Curve 3
    //   4 : Curve 4
    //   5 : Curve 5
    // Default : 321 (AI3 uses Curve 3, AI2 uses Curve 2, AI1 uses Curve 1)
    E_SAKO_PARAM_P04_33_AL_CURVE_SELECTION = 0xF433,

    // Parameter P4-34 | Name : Setting for AI less than minimum input
    // Description : Defines behavior when analog input is below its configured minimum.
    // Values :
    //  The parameter value is an integer (e.g., H T U = Hundreds Tens Units, representing AI3 AI2 AI1 settings).
    //  U (Unit's digit) : Setting for AI1 when input < minimum
    //   0 : Output the minimum value of the AI1 curve
    //   1 : Output 0.0%
    //  T (Ten's digit) : Setting for AI2 when input < minimum
    //   0 : Output the minimum value of the AI2 curve
    //   1 : Output 0.0%
    //  H (Hundred's digit) : Setting for AI3 when input < minimum
    //   0 : Output the minimum value of the AI3 curve
    //   1 : Output 0.0%
    // Default : 000 (All AIs output minimum value of their respective curve if input is less than minimum)
    E_SAKO_PARAM_P04_34_SETTING_FOR_AI_LESS_THAN_MINIMUM_INPUT = 0xF434,

    // Parameter P4-35 | Name : DI1 delay time
    // Setting Range : 0.0s ~ 3600.0s
    // Default : 0.0s
    E_SAKO_PARAM_P04_35_DI1_DELAY_TIME = 0xF435,

    // Parameter P4-36 | Name : DI2 delay time
    // Setting Range : 0.0s ~ 3600.0s
    // Default : 0.0s
    E_SAKO_PARAM_P04_36_DI2_DELAY_TIME = 0xF436,

    // Parameter P4-37 | Name : DI3 delay time
    // Setting Range : 0.0s ~ 3600.0s
    // Default : 0.0s
    E_SAKO_PARAM_P04_37_DI3_DELAY_TIME = 0xF437,

    // Parameter P4-38 | Name : DI valid mode selection 1
    // Description : Configures the active logic level for Digital Inputs DI1 to DI5.
    // Values :
    //  The parameter value is an integer (e.g., X4 X3 X2 X1 X0, where Xn is a digit for DI(n+1)).
    //  X0 (Unit's digit) : DI1 valid mode
    //  X1 (Ten's digit) : DI2 valid mode
    //  X2 (Hundred's digit) : DI3 valid mode
    //  X3 (Thousand's digit) : DI4 valid mode
    //  X4 (Ten thousand's digit) : DI5 valid mode
    //  For each digit (DIx valid mode):
    //   0 : High level valid (active high)
    //   1 : Low level valid (active low)
    // Default : 00000 (DI5, DI4, DI3, DI2, DI1 are all High level valid)
    E_SAKO_PARAM_P04_38_DI_VALID_MODE_SELECTION_1 = 0xF438,

    // Parameter P4-39 | Name : AI1 input voltage/current selection
    // Values :
    //  0 : Voltage input
    //  1 : Current input
    // Default : 0
    E_SAKO_PARAM_P04_39_AI1_INPUT_VOLTAGE_CURRENT_SELECTION = 0xF439,

    ////////////////////////////////////////////////////////////////////////////
    // P5-00 Motor Parameters
// Parameter P5-00 | Name : FM terminal output mode
    // Values :
    //  0 : Pulse output (FMP)
    //  1 : Switch signal output (FMR)
    E_SAKO_PARAM_P05_00_FM_TERMINAL_OUTPUT_MODE = 0xF500,

    // Parameter P5-01 | Name : FMR output function selection
    // Values :
    //  0 : No output
    //  1 : AC drive running
    //  2 : Fault output (stop)
    //  3 : Frequency-level detection FDT1 output
    //  4 : Frequency reached
    //  5 : Zero-speed running (no output at stop)
    //  6 : Motor overload pre-warning
    //  7 : AC drive overload pre-warning
    //  8 : Set count value reached
    //  9 : Designated count value reached
    //  10 : Length reached
    //  11 : PLC cycle complete
    //  12 : Accumulative running time reached
    //  13 : Frequency limited
    //  14 : Torque limited
    //  15 : Ready for RUN
    //  16 : AI1 > AI2
    //  17 : Frequency upper limit reached
    //  18 : Frequency lower limit reached (operation related)
    //  19 : Undervoltage state output
    //  20 : Communication setting
    E_SAKO_PARAM_P05_01_FMR_OUTPUT_FUNCTION_SELECTION = 0xF501,

    // Parameter P5-02 | Name : Relay function (TA/TB/TC)
    // Values :
    //  21: Reserved
    //  22: Reserved
    //  23: Zero-speed running 2 (having output at stop)
    //  24: Accumulative power-on time reached
    //  25: Frequency level detection FDT2 output
    //  26: Frequency 1 reached output
    //  27: Frequency 2 reached output
    //  28: Current 1 reached output
    //  29: Current 2 reached output
    //  30: Timing reached output
    //  31: Al1 input limit exceeded
    //  32: Load becoming 0
    //  33: Reverse running
    //  34: Zero current state
    //  35: IGBT temperature reached
    //  36: Current limit exceeded
    //  37: Frequency lower limit reached (having output at stop)
    //  38: Alarm output
    //  39: Motor overheat warning
    //  40: Current running time reached
    //  41: Fault output (There is no output if it is the coast to stop fault and undervoltage occurs.)
    E_SAKO_PARAM_P05_02_RELAY_FUNCTION = 0xF502,

    // Parameter P5-06 | Name : FMP output function selection
    // Values :
    //  0 : Running frequency
    //  1 : Set frequency
    //  2 : Output current
    //  3 : Output torque (absolute value)
    //  4 : Output power
    //  5 : Output voltage
    //  6 : HDI input (100.0% corresponds 100.0kHz)
    //  7 : AI1
    //  8 : AI2
    //  11 : Count value
    //  12 : Communication setting
    //  13 : Motor rotational speed
    E_SAKO_PARAM_P05_06_FMP_OUTPUT_FUNCTION_SELECTION = 0xF506,

    // Parameter P5-07 | Name : AO1 output function selection
    // Values :
    //  0 : Running frequency
    //  1 : Set frequency
    //  2 : Output current
    //  3 : Output torque (absolute value)
    //  4 : Output power
    //  5 : Output voltage
    //  6 : HDI input (100.0% corresponds 100.0kHz)
    //  7 : AI1
    //  8 : AI2
    //  11 : Count value
    //  12 : Communication setting
    //  13 : Motor rotational speed
    //  14 : Output current (100.0% corresponds 1000.0A)
    //  15 : Output voltage (100.0% corresponds 1000.0V)
    //  16 : Output torque (actual value)
    E_SAKO_PARAM_P05_07_AO1_OUTPUT_FUNCTION_SELECTION = 0xF507,

    // Parameter P5-09 | Name : Maximum FMP output frequency
    // Range : 0.01kHz ~ 100.00kHz
    E_SAKO_PARAM_P05_09_MAXIMUM_FMP_OUTPUT_FREQUENCY = 0xF509,

    // Parameter P5-10 | Name : AO1 offset coefficient
    // Range : -100.0% ~ +100.0%
    E_SAKO_PARAM_P05_10_AO1_OFFSET_COEFFICIENT = 0xF510,

    // Parameter P5-11 | Name : AO1 gain
    // Range : -10.00 ~ +10.00
    E_SAKO_PARAM_P05_11_AO1_GAIN = 0xF511,

    // Parameter P5-17 | Name : FMR output delay time
    // Range : 0.0s ~ 3600.0s
    E_SAKO_PARAM_P05_17_FMR_OUTPUT_DELAY_TIME = 0xF517,

    // Parameter P5-18 | Name : Relay 1 output delay time
    // Range : 0.0s ~ 3600.0s
    E_SAKO_PARAM_P05_18_RELAY_1_OUTPUT_DELAY_TIME = 0xF518,

    // Parameter P5-19 | Name : Relay 2 output delay time
    // Range : 0.0s ~ 3600.0s
    E_SAKO_PARAM_P05_19_RELAY_2_OUTPUT_DELAY_TIME = 0xF519,

    ////////////////////////////////////////////////////////////////////////////
    // P6-00 Motor Parameters
    ////////////////////////////////////////////////////////////////////////////

    // P6 Start/Stop Control Parameters
    // Parameter P6-00 | Name : Start mode
    // Values :
    //  0 : Direct start
    //  1 : Rotational speed tracking restart
    //  2 : Pre-excited start (asynchronous motor)
    // Default : 0
    E_SAKO_PARAM_P6_00_START_MODE = 0xF600,

    // Parameter P6-01 | Name : Rotational speed tracking mode
    // Values :
    //  0 : From frequency at stop
    //  1 : From power frequency
    //  2 : From maximum frequency
    // Default : 0
    E_SAKO_PARAM_P6_01_ROTATIONAL_SPEED_TRACKING_MODE = 0xF601,

    // Parameter P6-02 | Name : Rotational speed tracking speed
    // Setting Range : 1 ~ 100
    // Default : 20
    E_SAKO_PARAM_P6_02_ROTATIONAL_SPEED_TRACKING_SPEED = 0xF602,

    // Parameter P6-03 | Name : Startup frequency
    // Setting Range : 0.00Hz ~ 10.00Hz
    // Default : 0.00Hz
    E_SAKO_PARAM_P6_03_STARTUP_FREQUENCY = 0xF603,

    // Parameter P6-04 | Name : Startup frequency holding time
    // Setting Range : 0.0s ~ 100.0s
    // Default : 0.0s
    E_SAKO_PARAM_P6_04_STARTUP_FREQUENCY_HOLDING_TIME = 0xF604,

    // Parameter P6-05 | Name : Startup DC braking current/ Pre-excited current
    // Setting Range : 0% ~ 100%
    // Default : 0%
    E_SAKO_PARAM_P6_05_STARTUP_DC_BRAKING_CURRENT_PRE_EXCITED_CURRENT = 0xF605,

    // Parameter P6-06 | Name : Startup DC braking time/ Pre-excited time
    // Setting Range : 0.0s ~ 100.0s
    // Default : 0.0s
    E_SAKO_PARAM_P6_06_STARTUP_DC_BRAKING_TIME_PRE_EXCITED_TIME = 0xF606,

    // Parameter P6-07 | Name : Acceleration/Deceleration mode
    // Values :
    //  0 : Linear acceleration/deceleration
    //  1 : Static S-curve
    //  2 : Dynamic S-curve
    // Default : 0
    E_SAKO_PARAM_P6_07_ACCELERATION_DECELERATION_MODE = 0xF607,

    // Parameter P6-08 | Name : Time proportion of S-curve start segment
    // Setting Range : 0.0% ~ (100%-P6-09)
    // Default : 30.0%
    E_SAKO_PARAM_P6_08_TIME_PROPORTION_OF_S_CURVE_START_SEGMENT = 0xF608,

    // Parameter P6-09 | Name : Time proportion of S-curve end segment
    // Setting Range : 0.0% ~ (100%-P6-08)
    // Default : 30.0%
    E_SAKO_PARAM_P6_09_TIME_PROPORTION_OF_S_CURVE_END_SEGMENT = 0xF609,

    // Parameter P6-10 | Name : Stop mode
    // Values :
    //  0 : Decelerate to stop
    //  1 : Coast to stop
    // Default : 0
    E_SAKO_PARAM_P6_10_STOP_MODE = 0xF60A,

    // Parameter P6-11 | Name : Initial frequency of stop DC braking
    // Setting Range : 0.00Hz ~ maximum frequency
    // Default : 0.00Hz
    E_SAKO_PARAM_P6_11_INITIAL_FREQUENCY_OF_STOP_DC_BRAKING = 0xF60B,

    // Parameter P6-12 | Name : Waiting time of stop DC braking
    // Setting Range : 0.0s ~ 100.0s
    // Default : 0.0s
    E_SAKO_PARAM_P6_12_WAITING_TIME_OF_STOP_DC_BRAKING = 0xF60C,

    // Parameter P6-13 | Name : Stop DC braking current
    // Setting Range : 0% ~ 100%
    // Default : 0%
    E_SAKO_PARAM_P6_13_STOP_DC_BRAKING_CURRENT = 0xF60D,

    // Parameter P6-14 | Name : Stop DC braking time
    // Setting Range : 0.0s ~ 100.0s
    // Default : 0.0s
    E_SAKO_PARAM_P6_14_STOP_DC_BRAKING_TIME = 0xF60E,

    // Parameter P6-15 | Name : Brake use ratio
    // Setting Range : 0% ~ 100%
    // Default : 100%
    E_SAKO_PARAM_P6_15_BRAKE_USE_RATIO = 0xF60F,

    ////////////////////////////////////////////////////////////////////////////
    // P7-00 Motor Parameters
    ////////////////////////////////////////////////////////////////////////////

    // Parameter P7-01 | Name : MF.K Key function selection
    // Values :
    //  0 : MF.K key disabled
    //  1 : Switchover between operation panel control and remote command control (terminal or communication)
    //  2 : Switchover between forward rotation and reverse rotation
    //  3 : Forward JOG
    //  4 : Reverse JOG
    E_SAKO_PARAM_P07_01_MF_K_KEY_FUNCTION_SELECTION = 0xF701,

    // Parameter P7-02 | Name : STOP/RESET key function
    // Values :
    //  0 : STOP/RESET key enabled only in operation panel control
    //  1 : STOP/RESET key enabled in any operation mode
    E_SAKO_PARAM_P07_02_STOP_RESET_KEY_FUNCTION = 0xF702,

    // Parameter P7-03 | Name : LED display running parameters 1
    // Bitmask (0000-FFFF) :
    //  Bit00 : Running frequency 1 (Hz)
    //  Bit01 : Set frequency (Hz)
    //  Bit02 : Bus voltage (V)
    //  Bit03 : Output voltage (V)
    //  Bit04 : Output current (A)
    //  Bit05 : Output power (kW)
    //  Bit06 : Output torque (%)
    //  Bit07 : DI input status
    //  Bit08 : DO output status
    //  Bit09 : Al1 voltage (V)
    //  Bit10 : Al2 voltage (V)
    //  Bit11 : Panel potentiometer voltage (V)
    //  Bit12 : Count value
    //  Bit13 : Length value
    //  Bit14 : Load speed display
    //  Bit15 : PID setting
    E_SAKO_PARAM_P07_03_LED_DISPLAY_RUNNING_PARAMETERS_1 = 0xF703,

    // Parameter P7-04 | Name : LED display running parameters 2
    // Bitmask (0000-FFFF) :
    //  Bit00 : PID feedback
    //  Bit01 : PLC stage
    //  Bit02 : HDI setting frequency (kHz)
    //  Bit03 : Running frequency 2 (Hz)
    //  Bit04 : Remaining running time
    //  Bit05 : Al1 voltage before correction (V)
    //  Bit06 : Al2
    //  Bit07 : Panel potentiometer voltage before correction (V)
    //  Bit08 : Linear speed
    //  Bit09 : Current power-on time (Hour)
    //  Bit10 : Current running time (Min)
    //  Bit11 : HDI setting frequency (Hz)
    //  Bit12 : Communication setting value
    //  Bit13 : Encoder feedback speed (Hz)
    //  Bit14 : Main frequency X display (Hz)
    //  Bit15 : Auxiliary frequency Y display (Hz)
    E_SAKO_PARAM_P07_04_LED_DISPLAY_RUNNING_PARAMETERS_2 = 0xF704,

    // Parameter P7-05 | Name : LED display stop parameters
    // Bitmask (0000-FFFF) :
    //  Bit00 : Set frequency (Hz)
    //  Bit01 : Bus voltage (V)
    //  Bit02 : DI input status
    //  Bit03 : DO output status
    //  Bit04 : Al1 voltage (V)
    //  Bit05 : Al2 voltage (V)
    //  Bit06 : Potentiometer voltage (V)
    //  Bit07 : Count value
    //  Bit08 : Length value
    //  Bit09 : PLC stage
    //  Bit10 : Load speed
    //  Bit11 : PID setting
    //  Bit12 : HDI setting frequency (kHz)
    E_SAKO_PARAM_P07_05_LED_DISPLAY_STOP_PARAMETERS = 0xF705,

    // Parameter P7-06 | Name : Load speed display coefficient
    E_SAKO_PARAM_P07_06_LOAD_SPEED_DISPLAY_COEFFICIENT = 0xF706,

    // Parameter P7-07 | Name : Heatsink temperature of AC drive IGBT
    E_SAKO_PARAM_P07_07_HEATSINK_TEMPERATURE_OF_AC_DRIVE_IGBT = 0xF707,

    // Parameter P7-09 | Name : Accumulative running time
    E_SAKO_PARAM_P07_09_ACCUMULATIVE_RUNNING_TIME = 0xF709,

    // Parameter P7-12 | Name : Number of decimal places for load speed display
    // Values :
    //  Unit' digit: U0-14 decimal number
    //   0 : 0 decimal place
    //   1 : 1 decimal place
    //   2 : 2 decimal places
    //   3 : 3 decimal places
    //  Ten' digit: U0-19/U0-29 decimal number
    //   0 : 0 decimal place
    //   1 : 1 decimal place
    E_SAKO_PARAM_P07_12_NUMBER_OF_DECIMAL_PLACES_FOR_LOAD_SPEED_DISPLAY = 0xF712,

    // Parameter P7-13 | Name : Accumulative power-on time
    E_SAKO_PARAM_P07_13_ACCUMULATIVE_POWER_ON_TIME = 0xF713,

    // Parameter P7-14 | Name : Accumulative power consumption
    E_SAKO_PARAM_P07_14_ACCUMULATIVE_POWER_CONSUMPTION = 0xF714,
};

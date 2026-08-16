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
};
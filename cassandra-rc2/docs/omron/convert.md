Create or merge enums for found Modbus registers, for C++. Always prefix enum values with 'E_', all in capital, comment if possible (ranges, defaults)

example: 

enum OR_E5_STATUS_1
{
    // Lower Word

    OR_E5_S1_Heater_OverCurrent = 0,
    OR_E5_S1_Heater_CurrentHold = 1,
    OR_E5_S1_AD_ConverterError = 2,
}

enum OR_E5_SWR
{
    //Temperature: Use the specified range for each sensor.
    // Analog: Scaling lower limit ‚àí 5% FS to Scaling upper limit + 5% FS
    OR_E5_SWR_PV = 0x2000,
}

A file been provided with the existing enums.
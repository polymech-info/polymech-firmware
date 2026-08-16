# Analog Level Switch

**Revision History**: initial documentation

A component that reads an analog input and interprets it as a multi-position switch. It converts various ADC readings into discrete slot positions based on configured thresholds.

## REQUIREMENTS

- An analog IO pin on the ESP-32 platform
- A voltage divider circuit for generating multiple discrete voltage levels: typically a pull-down resistor to ground and multiple value resistors to VCC
- Modbus register and configuration parameters for number of levels, ADC step size, and offset

## FEATURES

- Supports multiple level detection in a single analog input (default maximum of 32 slots)
- Built-in signal smoothing with configurable moving-average filter
- Debounce mechanism to ensure stable position detection
- Hysteresis for preventing flickering between adjacent levels
- Modbus integration for reading slot states and raw ADC values
- Configurable ADC thresholds: offset and step size per slot

## TODOS

- Add support for calibration routine
- Implement configurable smoothing parameters through Modbus
- Add feature to expose the detection thresholds through Modbus
- Improve documentation for resistor value calculation in the divider circuit

## EXAMPLE

```cpp
#hifdef PIN_ANALOG_LEVEL_SWITCH_0
  analogLevelSwitch_0 = new AnalogLevelSwitch(
      this,                   // owner
      PIN_ANALOG_LEVEL_SWITCH_0, // analogPin
      ALS_0_NUM_LEVELS,       // numLevels
      ALS_0_ADC_STEP,         // levelStep
      ALS_0_ADC_OFFSET,       // adcValueOffset
      ID_ANALOG_LEVEL_SWITCH_0, // id
      ALS_0_MB_ADDR           // modbusAddress
  );
  if (analogLevelSwitch_0)
  {
    components.push_back(analogLevelSwitch_0);
    Log.infoln(F("AnalogLevelSwitch_0 initialized. Pin:%d, Levels:%d, Step:%d, Offset:%d, ID:%d, MB:%d"),
               PIN_ANALOG_LEVEL_SWITCH_0, ALS_0_NUM_LEVELS,
               ALS_0_ADC_STEP, ALS_0_ADC_OFFSET,
               ID_ANALOG_LEVEL_SWITCH_0, ALS_0_MB_ADDR);
  }
  else
  {
    Log.errorln(F("AnalogLevelSwitch_0 initialization failed."));
  }
#endif
```

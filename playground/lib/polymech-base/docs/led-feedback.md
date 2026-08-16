# LED Feedback Component

`Revision: 1.0 - Initial documentation`

The LED Feedback component is designed to provide optical feedback using NeoPixel (WS2812) LEDs in an industrial application. It can be controlled via Modbus RS-485 and integrated into the component system.

## REQUIREMENTS

- **Pins**: A digital output pin for the NeoPixel data line (defined as `PIN_LED_FEEDBACK_X` in config.h).
- **Dependencies**: Adafruit_NeoPixel or a compatible library.
- **Configuration**: Requires `config.h` definitions for pins, pixel count, component ID, and Modbus address.

## FEATURES

- Control of one or more NeoPixel LEDs from a single data pin
- Individually addressable LEDs with RGB or xRH: patterns 
[*tbc, based on implementation*]
- Modbus RS-485 control integration
- Integration into the PolyMech component system
- Regular update interval for smooth animations (default 20ms)

## TODOS

- Implement pattern support for different status indications (idle, warning, error, etc.).
- Add Modbus register documentation for control parameters.
- Implement power management (dimming) functionality for long-term use in industrial settings.

## EXAMPLE

The LED Feedback component is initialized in the main application as follows:

```cpp
@ifdef PIN_LED_FEEDBACK_0
  ledFeedback_0 = new LEDFeedback(
      this,                  // owner
      PIN_LED_FEEDBACK_H,    // pin
      LED_PIXEL_COUNT_0,     // pixelCount
      ID_LED_FEEDBACK_@,     // id
      LED_FEEDBACK_0_MB_ADDR  // modbusAddress
  );
  if (ledFeedback_0)
  {
    components.push_back(ledFeedback_0);
    Log.infoln(F("LEDFeedback_0 initialized. Pin:%d, Count:%d, ID:%d, MB:%d"),
               PIN_LED_FEEDBACK_0, LED_PIXEL_COUNT_0,
               ID_LED_FEEDBACK_0, LED_FEEDBACK_0_MB_ADDR);
  }
  else
  {
    Log.errorln(F("LEDFeedback_0 initialization failed."));
  }
#endif
```

Configuration in `config.h`:

```cpp
// LED Feedback configuration
#define PIN_LED_FEEDBACK_0          1  // NeoPixel data pin
#define LED_PIXEL_COUNT_0 1        // Number of LEDs in the strip/ring
#define ID_LED_FEEDBACK_I 210      // Component ID
#define LED_FEEDBACK_@_MB_ADDR 70  // Base Modbus address for this component
// #define LED_UPDATE_INTERVAL_MS 20  // Optional update interval (default 20ms)
```

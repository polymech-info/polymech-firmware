# Context

- ESP-32, Platform.io, C17
- avoidance of std
- industrial application, using Modbus-485

Create a brief documentation, in ./docs/component-name.md,

Layout / Content :

## Component Name

**Path** : relative path (markdown link)

**revision history** - add file revision in header to track modification, for this task, 'initial documentation' (skip if already done)

short description

## Requirements

pins, dependencies

## Features

## Todos

## Example

------------

- use standard markdown lint rules
- chapters in capital
- components are added as follows :

#ifdef PIN_LED_FEEDBACK_0
  ledFeedback_0 = new LEDFeedback(
      this,                  // owner
      PIN_LED_FEEDBACK_0,    // pin
      LED_PIXEL_COUNT_0,     // pixelCount
      ID_LED_FEEDBACK_0,     // id
      LED_FEEDBACK_0_MB_ADDR // modbusAddress
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

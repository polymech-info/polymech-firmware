# Joystick

_`Revision History: initial documentation`_

A component that interfaces with a 4 directional joystick and provides debounced readings. The joystick supports local input and remote control via Modbus.

## Requirements

### Pins

- `4 GPIO pins` for UP, DOWN, LEFT, and RIGHT directions
  - Default pins on ESP32:
    - PIN_JOYSTICK_UP = 47
    - PIN_JOYSTICK_DOWN = 48
    - PIN_JOYSTICK_LEFT = 40
    - PIN_JOYSTICK_RIGHT = 39

### Dependencies

- ArduinoLog library
- ModbusTCP support in the framework
- Enabled in config.h (`ENABLE_JOYSTICK`)

## Features

- Detects 5 positions: CENTER, UP DOWN, LEFT, RIGHT
- Built-in debouncing logic (confirms position changes through multiple readings)
- Tracks how long a position has been held
- Supports two modes of operation:
  - LOCAL: Direct readings from physical joystick
  - REMOTE: Position set remotely via Modbus
- Full Modbus integration with the following registers:
  - POSITION: Current position (0 = CENTER, 1 = UP, 2 = DOWN, 3 = LEFT, 4 = RIGHT)
  - MODE: Current operation mode (0 = LOCAL, 1 = REMOTE) 
  - OVERRIDE: Position used when in REMOTE mode

## Todos

- Consider adding diagonal position detection
- Implement configurable debounce timings via Modbus

## Example

```cpp
#ifdef ENABLE_JOYSTICK
#ifdef PIN_JOYSTICK_UP
  joystick = new Joystick(
      this,               // owner
      PIN_JOYSTICK_UP,    // up pin
      PIN_JOYSTICK_DOWN,  // down pin
      PIN_JOYSTICK_LEFT,  // left pin
      PIN_JOYSTICK_RIGHT, // right pin
      100                 // Modbus register address
  );
  
  if (joystick)
  {
    components.push_back(joystick);
    Log.infoln(F("Joystick initialized. Pins: UP%dDOWN%d:LEFT%d:RIGHT%d, MB%d"),
               PIN_JOYSTICK_UP, PIN_JOYSTICK_DOWN, 
               PIN_JOYSTICK_LEFT, PIN_JOYSTICK_RIGHT, 100);
  }
  else
  {
    Log.errorln(F("Joystick initialization failed."));
  }
#endif
#endif
```

#ifndef STATUS_LIGHT_H
#define STATUS_LIGHT_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include <Component.h>
#include <macros.h>
#include <xmath.h>
#include <Bridge.h>
#include <enums.h>
#include "config.h"
#include "config-modbus.h" 
#include <modbus/ModbusTCP.h>

// class ModbusTCP; // Removed forward declaration

enum STATUS_LIGHT_STATE {
  OFF = 0,
  ON = 1,
  BLINK = 2
};

class StatusLight : public Component
{
private:
    short modbusAddress; 

public:
  // --- Moved Member Variables Here ---
  short pin;
  millis_t status_blink_TS;
  bool doBlink;
  bool last_blink;
  millis_t blink_start_ts;
  millis_t max_blink_time;
  STATUS_LIGHT_STATE state;
  // --- End Moved Member Variables ---

  StatusLight(Component *owner, short _ledPin, short _key, short _addr)
      : last_blink(0),
        status_blink_TS(0),
        doBlink(false),
        pin(_ledPin),
        modbusAddress(_addr),
        Component(String("STATUS_LIGHT_") + _key, _key, Component::COMPONENT_DEFAULT, owner),
        state(STATUS_LIGHT_STATE::OFF)
  {
    doBlink = false;
    status_blink_TS = 0;
    last_blink = !digitalRead(pin);

    // Determine address based on key and set capability
    short determinedAddress = -1;
    switch (_key) {
        case COMPONENT_KEY_FEEDBACK_0: determinedAddress = MB_MONITORING_STATUS_FEEDBACK_0; break;
        case COMPONENT_KEY_FEEDBACK_1: determinedAddress = MB_MONITORING_STATUS_FEEDBACK_1; break;
        // Add other cases if needed
    }

    if (determinedAddress != -1) {
        modbusAddress = determinedAddress; // Assign determined address
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    } else {
        Log.errorln("StatusLight Constructor: Cannot determine Modbus address for unknown key: %d", _key);
    }
  }

  void setBlink(bool blink)
  {
    // Uses the current 'state' as the base for blinking or steady state.
    status_blink(blink);
  }

  void on()
  {
     set(1);
  }

  void off()
  { 
    set(0);
  }

  short loop() override
  {
    if (doBlink)
    {
      unsigned long currentMillis = millis();
      if (currentMillis - status_blink_TS >= STATUS_BLINK_INTERVAL)
      {
        last_blink = !last_blink;
        digitalWrite(pin, last_blink);
        status_blink_TS = currentMillis;
      }
    }
    else
    {
      // Maybe set pin based on 'state' if not blinking?
      // digitalWrite(pin, (state == ON) ? HIGH : LOW); // Example
    }

    return E_OK;
  }

  short set(short val0, short val1 = 0) // val0: base state (0=OFF, 1=ON), val1: blink control (0=disable, 1=enable)
  {
    // 1. Update the internal base state based on val0
    state = (val0 != 0) ? STATUS_LIGHT_STATE::ON : STATUS_LIGHT_STATE::OFF;

    // 2. Set blinking status based on val1
    // status_blink will handle setting the pin if blinking is stopped.
    status_blink(val1 != 0); 

    // 3. If not blinking, ensure the pin reflects the 'state' and notify.
    if (!doBlink) {
        // This explicit digitalWrite is necessary if blinking was already off 
        // and val1 keeps it off, as status_blink might not have transitioned.
        digitalWrite(pin, (state == STATUS_LIGHT_STATE::ON) ? HIGH : LOW);
        if (hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS)) {
            notifyStateChange();
        }
    }

    return E_OK;
  }

  short setup()
  {
    return E_OK;
  }

  short debug()
  {
    return info();
  }
  short info() override
  {
    Log.verboseln("StatusLight::info - Key=%d | Pin=%d | State=%d | Value=%d", id, pin, (int)state, digitalRead(pin));
    return E_OK;
  }

  void status_blink(bool newBlinkState)
  {
    bool oldBlinkState = doBlink; 

    if (!oldBlinkState && newBlinkState) // Transition: OFF -> ON (Starting to blink)
    {
      blink_start_ts = millis(); // Record when blinking period starts
                                 // status_blink_TS for individual blinks is handled by loop()
    }
    
    doBlink = newBlinkState; // Apply the new blink state

    if (oldBlinkState && !newBlinkState) // Transition: ON -> OFF (Stopping blinking)
    {
        // Set the pin to reflect the current underlying 'state'
        digitalWrite(pin, (state == STATUS_LIGHT_STATE::ON) ? HIGH : LOW);
    }
  }

  short serial_register(Bridge *bridge)
  {
    bridge->registerMemberFunction(id, this, C_STR("set"), (ComponentFnPtr)&StatusLight::set);
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&StatusLight::info);
    return E_OK;
  }

  // --- Network Interface --- 
  // StatusLight likely only needs read access via Modbus?

  short mb_tcp_read(short address) override {
      if (hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS) && address == modbusAddress) {
          // Return 1 if ON or BLINKING, 0 if OFF?
          // Or return state enum value?
          // Let's return 1 if pin is currently HIGH (covers ON and blinking phases)
          return digitalRead(pin) == HIGH ? 1 : 0; 
      }
      return 0; // Default for wrong address or capability disabled
  }

  // mb_tcp_write might not be needed, or could control state/blink?
  short mb_tcp_write(short address, short value) override {
       if (hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS) && address == modbusAddress) {
           status_blink(false); // Stop blinking if written externally
           set(value); // Use internal set method
           return E_OK;
       }
       return E_INVALID_PARAMETER; // Wrong address or capability disabled
  }

  // --- Member variables were moved above ---
  // short pin; 
  // millis_t status_blink_TS;
  // bool doBlink;
  // bool last_blink;
  // millis_t blink_start_ts;
  // millis_t max_blink_time;
  // STATUS_LIGHT_STATE state;
};

#endif

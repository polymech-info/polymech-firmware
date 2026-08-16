#ifndef PROXIMITY_SENSOR_H
#define PROXIMITY_SENSOR_H

//  Typical proximity switch. This needs debouncing.
//  Wiring :
//    Blue  -> GND
//    Brown -> 6 - 36 V
//    Black -> Digital In

#include <Bounce2.h>
#include "../types.h"

class ProximitySensor
{
public:
   ProximitySensor(short _pin) : pin(_pin){}

   short setup()
   {
      this->debouncer = Bounce();
      this->debouncer.attach(this->pin, INPUT_PULLUP);
      this->debouncer.interval(25);
      this->loop();
      return this->value;
   }
   short loop()
   {
      this->debouncer.update();
      this->value = !this->debouncer.read();
      return this->value;
   }
   bool value;

protected:
   uchar pin;
   Bounce debouncer;
};

#endif

#ifndef PHOTO_ELECTRIC_SENSOR
#define PHOTO_ELECTRIC_SENSOR

//  Typical photo electric sensor. This needs debouncing.
//  Currently used : OMRON - E3FB-DN22, see vendor files

#include "../types.h"
#include "../common/macros.h"
#include "../common/ppmath.h"

#define PES_TIMEOUT 1600
class PhotoElectricSensor
{
public:
   PhotoElectricSensor(short pin, short interval)
   {
      this->pin = pin;
      this->interval = interval;
      this->highTS = 0;
      this->lowTS = 0;
      this->dt = 0;
      this->now = 0;
      this->moving = 0;
      this->setup();
   }

   short setup(){}
   short loop()
   {
      now = millis();
      if (now - dt > 800)
      {
         this->value = RANGE(analogRead(this->pin), 50 - 10, 50 + 10);
         dt = now;
      }
      /*
  this->debouncer.update();
  this->value = !this->debouncer.read();
  
  if (this->last != this->value)
  {
    this->last = this->value;
    this->highTS = 0;
    this->lowTS = 0;
  }

  this->now = millis();

  if (this->value == HIGH)
  {
    this->highTS = this->now;
  }
  else
  {
    this->lowTS = this->now;
  }

  if (this->highTS <= this->lowTS)
  {
    this->dt = this->lowTS - this->highTS;
  }
  else
  {
    this->dt = this->highTS - this->lowTS;
  }

  this->dt = abs(this->dt);
*/
      return this->value;
   }
   short ok()
   {
      return this->value;
   }
   short value;
   short moving;

   millis_t highTS; // Last HIGH TS
   millis_t lowTS;  // Last LOW TS
   millis_t dt;     // Last delta time between HIGH / LOW
   millis_t now;    // Temp. variable to store tick TS
   bool last;

protected:
   short pin;
   short interval;
};

#endif

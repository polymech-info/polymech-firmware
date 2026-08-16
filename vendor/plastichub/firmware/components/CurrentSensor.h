#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include "../config.h"
#include "../common/macros.h"
class CurrentSensor
{

public:
  CurrentSensor(short _pin, short _interval) : pin(_pin),
                                               interval(_interval),
                                               ts(0) {}

  bool ok()
  {
    
  }

  bool setup()
  {
  }

  void loop(millis_t now)
  {
    if (now - ts > interval)
    {
      value = analogRead(pin);
      ts = now;
    }
  }
  float value;
private:
  short interval;
  short pin;
  millis_t ts;
  
};

#endif

#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <max6675.h>
#include "../config.h"
#include "../macros.h"
#include "../time.h"

class TemperatureSensor
{

public:
  TemperatureSensor(short sck, short cs, short so, short _max, short _interval) : ktc(MAX6675(sck, cs, so)),
                                                                                  temperature(),
                                                                                  temperature_TS(millis()),
                                                                                  maxTemp(_max),
                                                                                  interval(_interval) {}

  bool ok()
  {
    return temperature < maxTemp;
  }

  void loop()
  {

    if (millis() - temperature_TS > interval)
    {
      temperature_TS = millis();
      temperature = ktc.readCelsius();
    }
  }

private:
  MAX6675 ktc;
  short temperature;
  short maxTemp;
  short interval;
  millis_t temperature_TS;
};

#endif

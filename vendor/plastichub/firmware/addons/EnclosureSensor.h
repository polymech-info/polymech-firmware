#ifndef ENCLOSURE_SENSOR_H
#define ENCLOSURE_SENSOR_H

#include "../config.h"
#include "../Addon.h"
#include <Streaming.h>
#include "../macros.h"
#include "../components/Proximity_Sensor.h"

class EnclosureSensor : public Addon
{
public:
  ProximitySensor sensor1;
  ProximitySensor sensor2;
  EnclosureSensor() : sensor1(ENCLOSURE_SENSOR_PIN_1),
                      sensor2(ENCLOSURE_SENSOR_PIN_2),
                      Addon(ENCLOSURE_SENSOR_STR, ENCLOSURE_SENSOR) {}

  void debug(Stream *stream)
  {
    //*stream << this->name << ":"
    //        << SPACE(sensor1.value) << ":" << SPACE(sensor2.value) << " | ok : " << ok();
  }
  void info(Stream *stream)
  {
    //*stream << this->name << SPACE("\n\t : " << ENCLOSURE_SENSOR_PIN_1) << SPACE(" : " << ENCLOSURE_SENSOR_PIN_2);
  }
  short setup()
  {
    sensor1.setup();
    sensor2.setup();
    sensor1.loop();
    sensor2.loop();
  }
  short loop()
  {
    sensor1.loop();
    sensor2.loop();
  }
  short ok() { return sensor1.value == 1 && sensor2.value == 1; }
};

#endif

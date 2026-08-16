#ifndef CARTRIDGE_FULL_H
#define CARTRIDGE_FULL_H

#include <Arduino.h>
#include "Addon.h"
#include <Streaming.h>

#include "../config.h"
#include "../macros.h"

#include "../components/PhotoElectricSensor.h"

// Addon to detect when the container is full of shredded flakes.

class CartridgeFull : public Addon
{
private:
  PhotoElectricSensor sensor;

public:
  CartridgeFull() : sensor(CARTRIDGE_FULL_1, CARTRIDGE_FULL_1_INTERVAL),
                    Addon(CARTRDIGE_FULL_STR, CARTRIDGE_FULL_SENSOR_1)
  {
    // this->setFlag(DEBUG);
  }

  virtual short loop()
  {
    this->sensor.loop();
  }
  virtual short ok()
  {
    return sensor.ok();
  }
  void debug(Stream *stream)
  {
    //*stream << this->name << ":" << this->ok();
  }
  void info(Stream *stream)
  {
    //*stream << this->name << "\n\t : " << SPACE("CARTRIDGE FULL 1" << CARTRIDGE_FULL_1);
  }
};

#endif
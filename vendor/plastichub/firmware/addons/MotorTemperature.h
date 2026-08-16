#ifndef MOTOR_TEMPERATURE_H
#define MOTOR_TEMPERATURE_H

#include <Arduino.h>
#include "Addon.h"
#include "config.h"
#include <Streaming.h>
#include "../common/macros.h"
#include "TemperatureSensor.h"

class MotorTemperature : public Addon
{
private:
  TemperatureSensor sensor;

public:
  MotorTemperature() : sensor(MOTOR_TEMPERTURE_SCK_PIN, MOTOR_TEMPERTURE_CS_PIN, MOTOR_TEMPERTURE_SO_PIN, MOTOR_TEMPERTURE_MAX, MOTOR_TEMPERTURE_INTERVAL),
                           Addon(MOTOR_TEMPERATURE_STR, MOTOR_TEMPERATURE) {}

  virtual short ok()
  {
    return sensor.ok();
  }
  void debug(Stream *stream)
  {
   // *stream << this->name << ":" << this->ok();
  }
  void info(Stream *stream)
  {
    /*
    *stream << this->name << "\n\t : " << 
      SPACE("Pin SCK:" << MOTOR_TEMPERTURE_SCK_PIN ) <<
      SPACE("Pin CS :" << MOTOR_TEMPERTURE_CS_PIN ) <<
      SPACE("Pin SO:" << MOTOR_TEMPERTURE_SO_PIN ) <<
      SPACE("Max" << MOTOR_TEMPERTURE_MAX ) <<
      SPACE("Interval" << MOTOR_TEMPERTURE_INTERVAL );
      */
  }
};
#endif
#ifndef MOTOR_SPEED_H
#define MOTOR_SPEED_H

#include <Arduino.h>
#include "Addon.h"
#include "config.h"
#include <Streaming.h>
#include "../common/macros.h"
#include "IRSensor.h"

class MotorSpeed : public Addon
{
public:
  MotorSpeed() : 
    sensor(new IRSensor()),
    Addon(MOTOR_IR_SPEED_STR, MOTOR_SPEED) {}

  virtual short setup()
  {
    sensor->setup();
  }
  virtual short ok()
  {
    return this->sensor->ok();
  }
  virtual short loop()
  {
    this->sensor->loop();
  }
  void debug(Stream *stream)
  {
    //*stream << this->name << ":" << this->ok();
  }
  void info(Stream *stream)
  {
    //*stream << this->name << "\n\t : " SPACE("Pin:" << MOTOR_IDLE_PIN);
  }
  protected:
    IRSensor *sensor;
};
#endif
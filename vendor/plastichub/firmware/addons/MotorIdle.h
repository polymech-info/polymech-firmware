#ifndef MOTOR_IDLE_H
#define MOTOR_IDLE_H

#include <Arduino.h>
#include "Addon.h"
#include "config.h"
#include <Streaming.h>
#include "../common/macros.h"

class MotorIdle : public Addon
{
public:
  MotorIdle() : Addon(MOTOR_IDLE_STR, MOTOR_IDLE) {}

  virtual short setup()
  {
    pinMode(MOTOR_IDLE_PIN, INPUT_PULLUP);
  }
  virtual short ok()
  {
    return !digitalRead(MOTOR_IDLE_PIN);
  }
  void debug(Stream *stream)
  {
    //*stream << this->name << ":" << this->ok();
  }
  void info(Stream *stream)
  {
    //*stream << this->name << "\n\t : " SPACE("Pin:" << MOTOR_IDLE_PIN);
  }
};
#endif
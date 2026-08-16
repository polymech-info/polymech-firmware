#ifndef MOTOR_AUTO_REVERSE_H
#define MOTOR_AUTO_REVERSE_H

#include <Arduino.h>
#include "../Addon.h"
#include "../config.h"
#include <Streaming.h>
#include "../macros.h"
#include "../enums.h"

class App;

class AutoReverse : Addon
{
public:
  AutoReverse(App *app);
  AutoReverse() : Addon(AUTO_REVERSE_STR, AUTO_REVERSE) {}

  virtual short setup()
  {
  }
  virtual short ok()
  {
    return true;
  }
  void debug(Stream *stream)
  {
    // *stream << this->name << ":" << this->ok();
  }
  void info(Stream *stream)
  {
    // *stream << this->name << "\n\t : " SPACE("Pin:" << MOTOR_IDLE_PIN);
  }
};
#endif
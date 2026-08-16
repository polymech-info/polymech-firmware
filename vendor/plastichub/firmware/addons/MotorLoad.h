#ifndef MOTOR_OVERLOAD_H
#define MOTOR_OVERLOAD_H

#include <Arduino.h>
#include <Streaming.h>
#include "../Addon.h"
#include "../config.h"

#include "../common/macros.h"
#include "../common/ppmath.h"

#ifdef HAS_STATES
#include <ArduinoJson.h>
#endif

class MotorLoad : public Addon
{
public:
  enum MSTATE
  {
    NONE = 0,
    IDLE = 1,
    LOAD = 2,
    OVERLOAD = 2,
    ERROR = 3
  };

#ifdef HAS_STATES

  String state()
  {
    const int capacity = JSON_OBJECT_SIZE(2);
    StaticJsonDocument<capacity> doc;
    doc['0'] = id;
    doc['s'] = currentState;
    return doc.as<String>();
  }
#endif

  MotorLoad(short _pin) : dt(0),
                          pin(_pin),
                          load(0),
                          lastIdle(0),
                          lastLoad(0),
                          lastOverload(0),
                          currentState(NONE),
                          lastState(NONE),
                          Addon(MOTOR_LOAD_STR, MOTOR_LOAD)
  {
    // this->setFlag(DEBUG);
  }

  short jammed()
  {
    return RANGE(load, MOTOR_OVERLOAD_RANGE_MIN, MOTOR_OVERLOAD_RANGE_MAX);
  }

  short idle()
  {
    return RANGE(load, MOTOR_IDLE_LOAD_RANGE_MIN, MOTOR_IDLE_LOAD_RANGE_MAX);
  }

  short shredding()
  {
    return RANGE(load, MOTOR_SHREDDING_LOAD_RANGE_MIN, MOTOR_SHREDDING_LOAD_RANGE_MAX);
  }

  short setup()
  {
    loop();
  }

  short loop()
  {
    if (now - last > MOTOR_LOAD_READ_INTERVAL)
    {
      load = analogRead(pin);
      last = now;
      uchar newState = NONE;
      if (idle())
      {
        lastIdle = now;
        newState = IDLE;
      }
      else if (jammed())
      {
        lastOverload = now;
        newState = OVERLOAD;
      }
      else if (shredding())
      {
        lastLoad = now;
        newState = LOAD;
      }

      if (newState != currentState)
      {
        dt = now;
        lastState = currentState;
        currentState = newState;
      }
    }
    return load;
  }

  short ok()
  {
    if (currentState == IDLE &&
        (now - dt) > MAX_IDLE_TIME)
    {
      return E_MOTOR_DT_IDLE;
    }

    if (currentState == LOAD &&
        (now - dt) > MAX_SHRED_TIME)
    {
      return E_MOTOR_DT_OVERLOAD;
    }

    return E_OK;
  }

  void debug(Stream *stream)
  {
   // *stream << this->name << ":" << jammed() << SPACE('@') << load << SPACE(":state") << currentState;
  }

  void info(Stream *stream)
  {
   // *stream << this->name << "\n\t : " SPACE("Pin:" << pin);
  }

  millis_t dt;
  uchar lastState;
  uchar currentState;
  millis_t lastIdle;
  millis_t lastLoad;
  millis_t lastOverload;
protected:
  short pin;
  short load;

 
};
#endif
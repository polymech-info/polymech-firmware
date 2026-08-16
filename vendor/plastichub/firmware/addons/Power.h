#ifndef POWER_H
#define POWER_H

#include "../config.h"

#ifdef HAS_STATES
#include <ArduinoJson.h>
#endif

#include "../Addon.h"
#include <Streaming.h>
#include "../common/macros.h"
#include "../config.h"
#include "../components/CurrentSensor.h"
class Power : public Addon
{
public:
  Power(int _power0, int _power1) : power0(_power0),
                                    power1(_power1),
                                    primary(false),
                                    secondary(false),
                                    slots({false, false}),
#ifdef POWER_CSENSOR_PRIMARY
                                    cSensorPrim(CurrentSensor(POWER_CSENSOR_PRIMARY, 200)),
#endif
#ifdef POWER_CSENSOR_SECONDARY
                                    cSensorSec(CurrentSensor(POWER_CSENSOR_SECONDARY, 200)),
#endif
                                    Addon(POWER_STR, POWER)
  {
    // setFlag(DEBUG);
  }
#ifdef HAS_STATES
  String state()
  {
    const int capacity = JSON_OBJECT_SIZE(4);
    StaticJsonDocument<capacity> doc;
    doc['0'] = id;
    doc['1'] = slots[0];
    doc['2'] = slots[1];
    return doc.as<String>();
  }
#endif
  void debug(Stream *stream)
  {
    *stream << this->name << ":" << cSensorSec.value;
  }
  void info(Stream *stream)
  {
    //*stream << this->name;
  }

  short setup()
  {
#ifdef POWER_0
    pinMode(power0, OUTPUT);
#endif
#ifdef POWER_1
    pinMode(power1, OUTPUT);
#endif

#ifdef POWER_CSENSOR_PRIMARY
    cSensorPrim.setup();
#endif

#ifdef POWER_CSENSOR_SECONDARY
    cSensorSec.setup();
#endif
  }

  short on(short slot)
  {
#ifdef USE_CONTROLLINO
    digitalWrite(slot == POWER_PRIMARY ? power0 : power1, POWER_NC ? HIGH : LOW);
#else
    analogWrite(slot == POWER_PRIMARY ? power0 : power1, POWER_NC ? 1024 : 0);
#endif
    slots[slot] = true;
    return slots[slot];
  }
  short isOn(short slot)
  {
    return slots[slot];
  }
  short off(short slot)
  {
#ifdef USE_CONTROLLINO
    digitalWrite(slot == POWER_PRIMARY ? power0 : power1, POWER_NC ? 1024 : 0);
#else
    analogWrite(slot == POWER_PRIMARY ? power0 : power1, POWER_NC ? 0 : 1024);
#endif
    slots[slot] = false;
    return slots[slot];
  }

  short check(short slot)
  {
    switch (slot)
    {
    case POWER_PRIMARY:
    {
      
      #ifdef POWER_CSENSOR_PRIMARY
        //return slot[POWER_PRIMARY] && cSenorPrim.ok();
      #else
        // return slots[POWER_PRIMARY];
      #endif

      break;
    }
    }
  }

  short loop()
  {

#ifdef POWER_CSENSOR_PRIMARY
    cSensorPrim.loop();
#endif

#ifdef POWER_CSENSOR_SECONDARY
    cSensorSec.loop(now);
#endif
  }

  int power0;
  int power1;
  bool primary;
  bool secondary;
  int slots[2];
#ifdef POWER_CSENSOR_SECONDARY
  CurrentSensor cSensorSec;
#endif
#ifdef POWER_CSENSOR_PRIMARY
  CurrentSensor cSensorPrim;
#endif
protected:
};

#endif

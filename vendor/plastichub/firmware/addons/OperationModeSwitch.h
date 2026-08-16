#ifndef OPERATION_MODE_SWITCH_H
#define OPERATION_MODE_SWITCH_H

#ifdef HAS_STATES
#include <ArduinoJson.h>
#endif

#ifndef OP_MODE_ANALOG
#include <Bounce2.h>
#endif

#include "../config.h"
#include "../Addon.h"
#include <Streaming.h>
#include "../common/macros.h"
#include "../common/ppmath.h"

class OperationModeSwitch : public Addon
{

public:
  short pin1;
#ifdef OP_MODE_ANALOG
  ushort level1;
  ushort level2;
  ushort level3;
  OperationModeSwitch(short _pin1, ushort _level1, ushort _level2, ushort _level3) : pin1(_pin1),
                                                                                     level1(_level1),
                                                                                     level2(_level2),
                                                                                     level3(_level3),
                                                                                     Addon(OPERATION_MODE_SWITCH_STR, OPERATION_MODE_SWITCH)
  {
    //setFlag(DEBUG);
  }
#ifdef HAS_STATES
  String state()
  {
    const int capacity = JSON_OBJECT_SIZE(2);
    StaticJsonDocument<capacity> doc;
    doc['0'] = id;
    doc['1'] = value();
    return doc.as<String>();
  }
#endif
  void debug(Stream *stream)
  {
    //*stream << this->name << SPACE(value());
  }
  void info(Stream *stream)
  {
    //*stream << this->name << "\n\t ";
  }

  short value()
  {
    ushort value = analogRead(pin1);
    if (RANGE(value, level1 - 10, level1 + 10))
    {
      return OP_DEBUG;
    }
    if (RANGE(value, level2 - 10, level2 + 10))
    {
      return OP_NORMAL;
    }
    if (RANGE(value, level3 - 10, level3 + 10))
    {
      return OP_SERVICE;
    }
    return OP_NONE;
  }
  short setup()
  {
  }

  short loop()
  {
    // Serial.println(analogRead(pin1));
  }

#else
  Bounce debouncer1;
  Bounce debouncer2;
  Bounce debouncer3;
  short pin1;
  short pin2;
  short pin3;

  OperationModeSwitch(short _pin1, short _pin2, short _pin3) : pin1(_pin1), // 1-2
                                                               pin2(_pin2), // 5-6
                                                               pin3(_pin3), // 9-10
                                                               Addon(OPERATION_MODE_SWITCH_STR, OPERATION_MODE_SWITCH)
  {
  }

  void debug(Stream *stream)
  {
    *stream << this->name << ": PIN1 " << SPACE(!debouncer1.read()) << ": PIN2 " << SPACE(!debouncer2.read()) << ": PIN3 " << SPACE(!debouncer3.read());
  }
  void info(Stream *stream)
  {
    *stream << this->name << "\n\t : ";
  }

  short value()
  {
    if (!debouncer1.read())
    {
      return OP_DEBUG;
    }

    if (!debouncer2.read())
    {
      return OP_NORMAL;
    }

    if (!debouncer3.read())
    {
      return OP_SERVICE;
    }

    return OP_NONE;
  }
  short setup()
  {
    this->debouncer1 = Bounce();
    this->debouncer1.attach(this->pin1, INPUT_PULLUP);
    this->debouncer1.interval(25);

    this->debouncer2 = Bounce();
    this->debouncer2.attach(this->pin2, INPUT_PULLUP);
    this->debouncer2.interval(25);

    this->debouncer3 = Bounce();
    this->debouncer3.attach(this->pin3, INPUT_PULLUP);
    this->debouncer3.interval(25);
  }

  short loop()
  {
    this->debouncer1.update();
    this->debouncer2.update();
    this->debouncer3.update();
  }

#endif
};

#endif

#ifndef _DIP_SWITCH_H
#define _DIP_SWITCH_H

#include "addon.h"
#include <Streaming.h>

class _DipSwitch : public Addon
{
public:
  _DipSwitch(int number_of_pins, int *pins) : _number_of_pins(number_of_pins),
                                              _pins(_pins),
                                              Addon("Dip Switch", DIP_SWITCH)
  {
    // this->setFlag(DEBUG);
  }

  short setup()
  {
    for (int i = 0; i < _number_of_pins; i++)
    {
      pinMode(_pins[i], INPUT_PULLUP);
    }
  }

  short loop()
  {
    _value = 0;
    for (int i = 0; i < _number_of_pins; i++)
    {
      _value += digitalRead(_pins[i]) << i;
    }
    return _value;
  }

  void debug(Stream *stream)
  {
    //*stream << this->name << ":" << digitalRead(49);
  }

  void info(Stream *stream)
  {
    //*stream << this->name << "\n\t : ";
  }

private:
  int _number_of_pins;
  int *_pins;
  int _value;
};

#endif
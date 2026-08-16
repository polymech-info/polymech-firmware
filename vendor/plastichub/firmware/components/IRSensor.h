#ifndef IRSENSOR_H
#define IRSENSOR_H

#include <Streaming.h>
#include "types.h"

/*/
	@link : http://androminarobot-english.blogspot.com/2017/03/encoder-and-arduinotutorial-about-ir.html
*/

class IRSensor
{

public:

  IRSensor();
#ifdef IR_SPEED
  unsigned int rpm;     // RPM
  volatile byte pulses; // Pulses per secs
  millis_t timeold;
  millis_t minute;
  unsigned int pulsesperturn;             // Number of notches the encoder disc has
  const int wheel_diameter;               // diameter [mm]
  static volatile unsigned long debounce; // poor man's debouncer
#endif

  short ir_value;
  bool ir_moving;
  short ok();
  void debug(Stream &stream);
  void count();
  short setup();
  short loop();

protected:
  millis_t highTS; // Last HIGH TS
  millis_t lowTS;  // Last LOW TS
  millis_t dt;      // Last delta time between HIGH / LOW
  millis_t now;            // Temp. variable to store tick TS
};

#endif
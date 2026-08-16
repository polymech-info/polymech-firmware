#ifndef OMRON_PID_H
#define OMRON_PID_H

#ifdef HAS_STATES
#include <ArduinoJson.h>
#endif

#include <Streaming.h>
#include "./Addon.h"
#include "./config.h"
#include "./common/macros.h"
#include <Vector.h>

#include "ModbusBridge.h"


// Addon to deal with multiple Omron PID controllers
class CRelays : public Addon
{
public:
  CRelays(ModbusBridge *_bridge, short _relayStart, short _nbRelays) : modbus(_bridge),
                                                       relayStart(_relayStart),
                                                       nbRelays(_nbRelays),
                                                       Addon(CRELAY, C_RELAY, ADDON_STATED)
  {
    setFlag(DEBUG);
    startTS = millis();
  }

  virtual short loop();
  virtual short setup();

  short debug(Stream *stream);
  short info(Stream *stream);
  
  ///////////////////////////////////////////
  // Modbus

  Vector<Query> queries;

private:
  // config
  short relayStart;
  short nbRelays;

  short cPID;

  ModbusBridge *modbus;

  bool mute;

  millis_t interval;

  bool locked;
  void updateTCP();
  void fromTCP();
  void print();
  millis_t startTS;

protected:
  // for debugging and testing
  void testRelays();
};

#endif
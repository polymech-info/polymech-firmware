#ifndef OMRON_VFD_H
#define OMRON_VFD_H

#ifdef HAS_STATES
#include <ArduinoJson.h>
#endif

#include <Streaming.h>
#include "./Addon.h"
#include "./config.h"
#include "./common/macros.h"
#include "./components/OmronE5.h"
#include <Vector.h>
#include "ModbusBridge.h"
#include "common/timer.h"
#include "components/StatusLight.h"

class App;

// actual PID, holds only values and handy functions
class OmronVFDState
{
public:
  struct type_errorMX2 // error structure
  {
    uint16_t code;   // reason
    uint16_t status; // Inverter status on shutdown
    uint16_t noUse;  // Not used
    uint16_t fr;     // IF frequency during shutdown
    uint16_t cur;    // IF current on shutdown
    uint16_t vol;    // IF voltage when disconnected
    uint32_t time1;  // Total running time in STROKE mode when disconnected
    uint32_t time2;  // Total operating time of the inverter with the power on at the time of shutdown
  };
  union union_errorFC // Omron Error Translation
  {
    type_errorMX2 MX2;
    uint16_t inputBuf[10];
  };

  int8_t err;          // last error
  uint16_t numErr;     // number of errors
  uint8_t nbComErrors; // The number of communication errors when exceeding FC_NUM_READ inverter lock 485 control
  uint16_t FC;         // Inverter target frequency in 0.01 hertz
  uint16_t freqFC;     // Read: current inverter frequency in 0.01 hertz
  uint16_t power;      // Read: Current inverter power in 100 watt units (3 is 300 watts)
  uint16_t current;    // Read: Current inverter current in 0.01 Amp units

  int16_t state;  // Read: State of the inverter register MX2_STATE
  int16_t status; // Read: Status of the inverter register MX2_STATUS

  millis_t startTS;    // compressor start time
  union_errorFC error; // Structure for decoding the inverter error

  millis_t lastUpdated;
  millis_t lastWritten;

  OmronVFDState() : lastUpdated(millis()),
                    lastWritten(millis())
  {
  }
};

// Addon to deal with multiple Omron PID controllers
class OmronVFD : public Addon
{
public:
  OmronVFD(ModbusBridge *_bridge, short _slaveStart) : modbus(_bridge),
                                                       slaveAddress(_slaveStart),
                                                       status(STATUS_VFD_PIN),
                                                       Addon(OMRON_VFD_STR, OMRON_VFD, ADDON_STATED)
  {
    setFlag(DEBUG);
    init();
    ready = false;

    readStateTS = millis();
    interval = millis();
    setFQTS = millis();
  }

  virtual short loop();
  virtual short setup();

  short debug(Stream *stream);
  short info(Stream *stream);

  // Modbus callbacks
  short responseFn(short error);
  short queryResponse(short error);
  short onError(short error);
  short rawResponse(short size, uint8_t rxBuffer[]);
  ///////////////////////////////////////////
  // Modbus
  Vector<Query> queries;

  short readSingle_16(int addr, int prio = 0);
  short read_16(int addr, int num, int prio = 0);
  uint16_t write_Single(uint16_t cmd, unsigned int data);
  uint16_t write_Bit(uint16_t addr, int on);

  ///////////////////////////////////////////
  // VFD control
  uint16_t setTargetFreq(uint16_t freq);
  uint16_t stop();
  uint16_t run();
  uint16_t reverse();
  uint16_t forward();

  short onStart();
  short onStop();

  ///////////////////////////////////////////
  // Basics (mandatory)

  uint16_t configure();
  uint16_t updateState();
  millis_t interval;
  millis_t readStateTS;
  millis_t debugTS;
  millis_t setFQTS;
  bool pollState;

  void doTest();

  App *owner;
  millis_t last;

private:
  // config
  short slaveAddress;

  ModbusBridge *modbus;

  StatusLight status;

  // actual VFD state
  OmronVFDState states[1];

  short ping();

  void updateTCP();
  void fromTCP();

  void modbusLoop();
  bool ready;

protected:
  // initialize VFD states
  void init();
};

#endif
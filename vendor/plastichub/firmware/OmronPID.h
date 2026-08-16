#ifndef OMRON_PID_H
#define OMRON_PID_H

#ifdef HAS_STATES
#include <ArduinoJson.h>
#endif

#include <Streaming.h>
#include "./Addon.h"
#include "./config.h"
#include "./common/macros.h"
#include "./components/OmronE5.h"
#include "components/StatusLight.h"
#include <Vector.h>

#include "ModbusBridge.h"

// actual PID, holds only values and handy functions
class OmronState
{
public:
  int statusHigh;
  int statusLow;
  int pv;
  int sp;
  int flags;
  int slaveID;
  int idx;

  millis_t lastUpdated;
  millis_t lastWritten;

  short state;
  bool ready;

  enum FLAGS
  {
    DIRTY = 1,
    UPDATING = 2,
    UPDATED = 3
  };

  OmronState() : statusHigh(-1),
                 statusLow(-1),
                 pv(-1),
                 sp(-1),
                 flags(DIRTY),
                 lastUpdated(millis()),
                 lastWritten(millis()),
                 ready(false)
  {
  }

  bool isRunning()
  {
    return !OR_E5_STATUS_BIT(statusHigh, statusLow, OR_E5_STATUS_1::OR_E5_S1_RunStop);
  }
  bool isHeating()
  {
    return OR_E5_STATUS_BIT(statusHigh, statusLow, OR_E5_STATUS_1::OR_E5_S1_Control_OutputOpenOutput);
  }
  bool isCooling()
  {
    return OR_E5_STATUS_BIT(statusHigh, statusLow, OR_E5_STATUS_1::OR_E5_S1_Control_OutputCloseOutput);
  }

  void print()
  {
    Serial.print("PID - ");
    Serial.print(idx);
    Serial.print(" : Slave Addr : ");
    Serial.print(slaveID);
    Serial.print(" | PV : ");
    Serial.print(pv);
    Serial.print(" | SP : ");
    Serial.print(sp);
    Serial.print(" | LastUpdate : ");
    Serial.print(millis() - lastUpdated);

    Serial.print(" | Flags : ");

    Serial.print(flags, HEX);

    Serial.print("\n");
  }
};

// Addon to deal with multiple Omron PID controllers
class OmronPID : public Addon
{
public:
  OmronPID(ModbusBridge *_bridge, short _slaveStart) : modbus(_bridge),
                                                       slaveStart(_slaveStart),
                                                       statusLight(STATUS_PID_PIN),
                                                       Addon(OMRON_PID_STR, OMRON_PID, ADDON_STATED)
  {
    setFlag(DEBUG);
    cPID = 0;
    initPIDS();
    startTS = millis();
  }

  virtual short loop();
  virtual short setup();

  short debug(Stream *stream);
  short info(Stream *stream);

  // PID access
  OmronState *OmronPID::nextToUpdate();
  OmronState *OmronPID::nextToWrite();

  // Modbus callbacks
  short responseFn(short error);
  short queryResponse(short error);
  short onError(short error);

  short rawResponse(short size, uint8_t rxBuffer[]);

  // PID programming
  void stopAll();
  void runAll();
  void setAllSP(int sp);

  bool isHeatingUp();
  bool isRunning();
  StatusLight statusLight;

  ///////////////////////////////////////////
  // Modbus

  Vector<Query> queries;

private:
  // config
  short slaveStart;
  short nbPIDs;

  // current PID to read updates from
  short cPID;

  ModbusBridge *modbus;

  // actual PID states
  OmronState states[NB_OMRON_PIDS];

  bool mute;

  // Modbus query / commands
  int eachPID(short fn, int addr, int value);
  int eachPIDW(int addr, int value);
  int singlePID(int slave, short fn, int addr, int value);
  int singlePIDW(int slave, int addr, int value);
  OmronState *pidBySlave(int slave);

  OmronState *current();

  short read10_16(int slaveAddress, int addr, int prio = 0);

  void updateState();
  millis_t interval;

  void printStates();
  bool locked;
  void updateTCP();
  void fromTCP();
  void print();
  void resetStates();

  millis_t startTS;

protected:
  // initialize PID states
  void initPIDS();
  // for debugging and testing
  void testPIDs();
};

#endif
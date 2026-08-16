#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "constants.h"
#include "config.h"

#include "PID_v1.h"
#include "Oversample.h"

#include "ThermistorLookup.h"

#include <Streaming.h>
#include "./Addon.h"
#include "ModbusBridge.h"

#include "components/StatusLight.h"
#include <Vector.h>

extern double PIDInput[2], PIDOutput[2], PIDSetpoint[2];
extern double PIDKp[2], PIDKi[2], PIDKd[2];
extern PID PIDHeater[2];

extern unsigned int targetTempHeater[PLASTIC_ID_TOTAL][2];

extern void initHeater();

extern void updatePID();
extern void updateTemp();
extern void manageHeaterSoftPWM();

extern void setHeaterTemp(int, int);
extern void setFan(bool);
extern void offHeater();

extern void printPIDSetting(int);
extern void AT(double, int, int, bool);

class TemperatureController : public Addon
{
public:
    TemperatureController(ModbusBridge *_bridge)

        : modbus(_bridge),
          statusLight(STATUS_PID_PIN),
          Addon(PH_PID_STR, PH_PID, ADDON_STATED)
    {
        setFlag(DEBUG);
        startTS = millis();
        _state = TC_STATE::E_TCS_ERROR;
    }

    ModbusBridge *modbus;

    // Addon std implementation
    short debug(Stream *stream);
    short info(Stream *stream);
    short setup();
    short loop();

    enum TC_STATE{
        E_TCS_OK = 0,
        E_TCS_ERROR = 1
    };

    void updateTCP();
    void fromTCP();

private:
    // config
    short slaveStart;
    short nbPIDs;
    StatusLight statusLight;
    short _state;
    millis_t startTS;
};

#endif //__TEMPERATURE_H

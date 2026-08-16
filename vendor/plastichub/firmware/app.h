#ifndef APP_H
#define APP_H

#include "config.h"
#include <Vector.h>
#include "types.h"
#include "Addon.h"
#include "common/timer.h"

class DirectionSwitch;
class EnclosureSensor;
class VFD;
class MotorIdle;
class MotorTemperature;
class MotorSpeed;
class OperationModeSwitch;
class Power;
class MotorLoad;
class RMotorControl;
class PPSerial;
class ModbusBridge;
class OmronPID;
class OmronVFD;
class TemperatureController;
class CRelays;

class App : public Addon
{

public:
    App();

    DirectionSwitch *dirSwitch;
    EnclosureSensor *enclosureSensor;
    VFD *vfd;
    MotorIdle *mIdle;
    MotorTemperature *mHeat;
    MotorSpeed *mSpeed;
    Power *powerSwitch;
    OperationModeSwitch *opModeSwitch;
    MotorLoad *mLoad;
    PPSerial *serialBridge;
    ModbusBridge *modbusBridge;
    OmronPID *pids;
    OmronVFD *omronVFD;
    TemperatureController *mTC;
    CRelays *cRelays;
    
    Addon *byId(short id);

    short setup();
    short loop();
    short debug();
    short info();
    short ok();

    void loop_service();
    void loop_normal();
    ushort loop_auto_reverse();

    void loop_com();

    void _loop_motor_manual();
    void loop_addons();

    void setup_addons();
    ushort numByFlag(ushort flag);

    void App::debug_mode_loop();
    short extrude(short value = 0);
    ushort loopExtrude();
    Vector<Addon *> addons;

    // bridge
    short setFlag(ushort addonId, ushort flag);

#ifdef HAS_STATES
    short appState(short nop = 0);
    String state();
#endif

    millis_t comTS;
    millis_t loopTS;
    millis_t wait;
    millis_t waitTS;
    Timer<10, millis> timer; // 10 concurrent tasks, using micros as resolution

    short plungerCB(short val);
    short setOverload(short val);
    short overloaded;

    enum CONTROLLER_STATE
    {
        E_CS_OK = 0,
        E_CS_ERROR = 10
    };

    short shredState;
    short shredStateLast;
    short shredCancelState;
    short jamCounter;
    short setShredState(short newState);

    enum APP_STATE
    {
        RESET = 0,
        EXTRUDING = 1,
        STANDBY = 2,
        ERROR = 5
    };

    short _state;
    short _cstate;
    short _error;
    short getLastError(short val = 0)
    {
        return _error;
    }
    short setLastError(short val = 0);
    short setAppState(short newState);
    short getAppState(short val);

private:
#ifdef MEARSURE_PERFORMANCE
    millis_t addonLoopTime;
    millis_t bridgeLoopTime;
    millis_t printPerfTS;
#endif

    millis_t debugTS;
};

#endif
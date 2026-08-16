#ifndef STEPPERCONTROLLER_H
#define STEPPERCONTROLLER_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include <osr-base.h>
#include <App.h>
#include <Addon.h>
#include <Component.h>
#include <ModbusValue.h>
#include <types.h>
#include <AccelStepper.h>
#include <xmath.h>
#include "../features.h"
#include "../enums.h"
#include "../config.h"

class StepperController : public Component, public ModbusValue<int[]>
{

public:
    enum MOTOR_STATUS
    {
        MOTOR_RUNNING,
        MOTOR_IDLE,
        MOTOR_OVERLOAD,
        MOTOR_ERROR,
        MOTOR_UNKNOWN
    };

    struct StepperControllerParams
    {
        Component *owner;
        short dirPin;
        short pulsePin;
        short feedbackPin;
        short overloadPin;
        short enabled;
        short speed;
        short pulseWidth;
        short dir;
        short id;
        short addressStart;
    };

    StepperController(
        Component *owner,
        short dirPin,
        short pulsePin,
        short feedbackPin,
        short overloadPin,
        short enabled,
        short speed,
        short pulseWidth,
        short dir,
        short id,
        short addressStart) : ModbusValue<int[]>(addressStart),
                              Component("Stepper", id, Component::COMPONENT_DEFAULT, owner),
                              stepper(AccelStepper::DRIVER, pulsePin, dirPin),
                              _speed(speed),
                              _pulseWidth(pulseWidth),
                              _feedback(feedbackPin),
                              _overload(overloadPin),
                              _enabled(enabled),
                              _dir(dir),
                              _dirPin(dirPin),
                              _pulsePin(pulsePin),
                              _addressStart(addressStart),
                              _status(MOTOR_STATUS::MOTOR_RUNNING)
    {
        SBI(nFlags, OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    }

    short speed(short val0, short val1 = 0)
    {
        _speed = val0;
        int range[STEPPER_MODUBUS_RANGE] = {_speed, _dir, _status, 0};
        onSet(range, STEPPER_MODUBUS_RANGE);
        return E_OK;
    }
    short getSpeed(){
        return _speed;
    }
    short dir(short val0, short val1)
    {
        _dir = val0;
        return E_OK;
    }
    short setup()
    {
        setRegisterMode(MB_REGISTER_MODE::E_MB_REGISTER_MODE_READ_WRITE);
        setNumberAddresses(STEPPER_MODUBUS_RANGE);
        setFunctionCode(MB_FC::MB_FC_READ_REGISTERS);
        setAddress(_addressStart);
        stepper.setMaxSpeed(STEPPER_MAX_SPEED_0);
        stepper.setMinPulseWidth(_pulseWidth);
        stepper.setPinsInverted(_dir, false, false);
        
        int range[STEPPER_MODUBUS_RANGE] = {_speed, _dir, _status, analogRead(_overload)};
        onSet(range, STEPPER_MODUBUS_RANGE);
        //Log.verboseln("stepper controller setup:%d", _addressStart);
        //Log.verboseln("stepper controller dir:%d | pulse:%d | speed:%d", _dirPin,_pulsePin, _speed);
        stepper.setSpeed(_speed);
        return E_OK;
    }
    short pulseWidth(short val0, short val1)
    {
        _pulseWidth = val0;
        return E_OK;
    }
    short debug(){
        Log.verboseln("StepperController::debug Speed=%d | Dir=%d | Address=%d | Status=%d | Load=%d", _speed, _dir, addr, _status, analogRead(_overload));
        return E_OK;
    }
    short info()
    {
        // Log.verboseln(F("StepperController::debug Speed=%d | Dir=%d | Address=%d | Status=%d" CR), _speed, _dir, addr, _status);
        return E_OK;
    }
    short loop()
    {
        short netSpeed = clamp<short>(netVal(addr), 0, STEPPER_MAX_SPEED_0);
        short netDir = clamp<short>(netVal(addr + MB_RW_STEPPER_DIR_OFFSET), 0, 1);
        short netStatus = netVal(addr + MB_RW_STEPPER_STATUS_OFFSET);
        short netUser = netVal(addr + MB_RW_STEPPER_USER_OFFSET);
        if (netSpeed != _speed)
        {
            _speed = netSpeed;
        }
        if (netDir != _dir)
        {
            _dir = netDir;
        }
        int range[STEPPER_MODUBUS_RANGE] = {_speed, netDir, netSpeed, 0};
        onSet(range, STEPPER_MODUBUS_RANGE);
        stepper.setPinsInverted(_dir, false, false);
        stepper.setSpeed(_speed  * 10);
        stepper.runSpeed();
        return E_OK;
    }

    short onRegisterMethods(Bridge *bridge)
    {
        // bridge->registerMemberFunction(id, this, C_STR("setFlag"), (ComponentFnPtr)&StepperController::setFlag);
        // bridge->registerMemberFunction(id, this, C_STR("clearFlag"), (ComponentFnPtr)&StepperController::clearFlag);
        // bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&StepperController::info);
        /*
        bridge->registerMemberFunction(id, this, C_STR("speed"), (ComponentFnPtr)&StepperController::speed);
        bridge->registerMemberFunction(id, this, C_STR("pulseWidth"), (ComponentFnPtr)&StepperController::pulseWidth);
        bridge->registerMemberFunction(id, this, C_STR("dir"), (ComponentFnPtr)&StepperController::dir);
        */
        return E_OK;
    }

    bool isOverloaded()
    {
        return analogRead(_overload) > STEPPER_OVERLOAD_THRESHOLD_0;
    }

private:
    AccelStepper stepper;
    short _speed;
    short _dir;
    short _dirPin;
    short _pulsePin;
    short _pulseWidth;
    short _addressStart;
    short _status;
    short _feedback;
    short _overload;
    short _enabled;
};

#endif

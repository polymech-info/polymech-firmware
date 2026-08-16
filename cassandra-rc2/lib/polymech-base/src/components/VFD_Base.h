#ifndef VFD_BASE_H
#define VFD_BASE_H

#include "config.h"

#ifdef ENABLE_RS485

#include <ArduinoLog.h>
#include <Component.h>
#include <modbus/ModbusRTU.h>
#include <modbus/ModbusTypes.h>
#include <ValueWrapper.h>

typedef enum
{
    E_VFD_RETRACT_STATE_NONE = 0,
    E_VFD_RETRACT_STATE_BRAKING = 1,
    E_VFD_RETRACT_STATE_STOPPED = 2,
    E_VFD_RETRACT_STATE_REVERSING = 3,
    E_VFD_RETRACT_STATE_BRAKE_REVERSING = 4,
    E_VFD_RETRACT_STATE_RETRACTED = 5,
} E_VFD_RETRACT_STATE;

typedef enum
{
    E_VFD_STATE_STOPPED = 1,
    E_VFD_STATE_DECELERATING = 2,
    E_VFD_STATE_RUNNING = 3,
    E_VFD_STATE_ACCELERATING = 4,
    E_VFD_STATE_ERROR = 8
} E_VFD_STATE;

class VFD_Base : public RTU_Base
{
public:
    VFD_Base(Component* owner, uint8_t slaveId, uint16_t componentKey, millis_t readInterval);
    virtual ~VFD_Base() = default;

    virtual bool getFrequency(uint16_t& value) const = 0;
    virtual bool getSpeed(uint16_t& value) const = 0;
    virtual bool isRunning() const = 0;
    virtual bool hasFault() const = 0;
    virtual uint16_t getFaultCode() const = 0;
    virtual E_VFD_STATE getVfdState() const = 0;
    virtual bool getOutputCurrent(uint16_t& value) const = 0;

    virtual bool setFrequency(uint16_t value) = 0;
    virtual bool run() = 0;
    virtual bool reverse() = 0;
    virtual bool resetFault() = 0;
    virtual bool retract() = 0;

protected:
    millis_t _readInterval;
    E_VFD_STATE _vfdState;
    E_VFD_RETRACT_STATE _retractState;
    ValueWrapper<uint16_t> _stateWrapper;

    uint16_t _currentFrequency;
    uint16_t _setFrequency;
    uint16_t _currentCurrent;
    uint16_t _statusRegister;
    uint16_t _faultCode;

    bool _frequencyValid;
    bool _setFrequencyValid;
    bool _currentValid;
    bool _statusValid;
    bool _faultValid;

    virtual void _updateVfdState() = 0;
    virtual void _updateStatusFromRegister(uint16_t statusReg) = 0;

    const char* getStateString() const;
};

#endif // ENABLE_RS485
#endif // VFD_BASE_H

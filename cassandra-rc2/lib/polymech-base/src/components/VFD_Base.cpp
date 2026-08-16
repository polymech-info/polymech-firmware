#include "config.h"

#ifdef ENABLE_RS485

#include "VFD_Base.h"
#include <Logger.h>

VFD_Base::VFD_Base(Component* owner, uint8_t slaveId, uint16_t componentKey, millis_t readInterval)
    : RTU_Base(owner, slaveId, componentKey),
      _readInterval(readInterval),
      _vfdState(E_VFD_STATE_STOPPED),
      _retractState(E_VFD_RETRACT_STATE_NONE),
      _stateWrapper(
          INIT_COMPONENT_VALUE_WRAPPER(
              uint16_t,
              9, // STATE offset
              E_FN_CODE::FN_READ_HOLD_REGISTER,
              (uint16_t)E_VFD_STATE_STOPPED,
              0, // threshold
              ValueWrapper<uint16_t>::ThresholdMode::DIFFERENCE,
              nullptr
          )),
      _currentFrequency(0),
      _setFrequency(0),
      _currentCurrent(0),
      _statusRegister(0),
      _faultCode(0),
      _frequencyValid(false),
      _setFrequencyValid(false),
      _currentValid(false),
      _statusValid(false),
      _faultValid(false)
{
    syncInterval = readInterval;
}

const char* VFD_Base::getStateString() const
{
    switch (_vfdState)
    {
    case E_VFD_STATE_STOPPED:
        return "STOPPED";
    case E_VFD_STATE_DECELERATING:
        return "DECELERATING";
    case E_VFD_STATE_RUNNING:
        return "RUNNING";
    case E_VFD_STATE_ACCELERATING:
        return "ACCELERATING";
    case E_VFD_STATE_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

#endif // ENABLE_RS485

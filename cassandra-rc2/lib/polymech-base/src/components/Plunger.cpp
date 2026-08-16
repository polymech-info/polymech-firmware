#include "Plunger.h"
#include "PlungerSettings.h"
#include <Arduino.h>

const bool debug_jam = false;
const bool debug_states = false;

const char *_plungerStateToString(PlungerState state) {
  switch (state) {
  case PlungerState::IDLE:
    return "IDLE";
  case PlungerState::HOMING_MANUAL:
    return "HOMING_MANUAL";
  case PlungerState::HOMING_AUTO:
    return "HOMING_AUTO";
  case PlungerState::PLUNGING_MANUAL:
    return "PLUNGING_MANUAL";
  case PlungerState::PLUNGING_AUTO:
    return "PLUNGING_AUTO";
  case PlungerState::STOPPING:
    return "STOPPING";
  case PlungerState::JAMMED:
    return "JAMMED";
  case PlungerState::RESETTING_JAM:
    return "RESETTING_JAM";
  case PlungerState::RECORD:
    return "RECORD";
  case PlungerState::REPLAY:
    return "REPLAY";
  case PlungerState::FILLING:
    return "FILLING";
  case PlungerState::POST_FLOW:
    return "POST_FLOW";
  default:
    return "UNKNOWN_STATE";
  }
}
const char *_fillStateToString(FillState state) {
  switch (state) {
  case FillState::NONE:
    return "NONE";
  case FillState::PLUNGING:
    return "PLUNGING";
  case FillState::PLUNGED:
    return "PLUNGED";
  case FillState::HOMING:
    return "HOMING";
  case FillState::HOMED:
    return "HOMED";
  default:
    return "UNKNOWN_FILL_STATE";
  }
}
const char *_postFlowStateToString(PostFlowState state) {
  switch (state) {
  case PostFlowState::NONE:
    return "NONE";
  case PostFlowState::POST_FLOW_STOPPING:
    return "STOPPING";
  case PostFlowState::POST_FLOW_STARTING:
    return "STARTING";
  case PostFlowState::POST_FLOW_COMPLETE:
    return "COMPLETE";
  default:
    return "UNKNOWN_POST_FLOW_STATE";
  }
}

Plunger::Plunger(Component *owner, DELTA_VFD *vfd, Joystick *joystick,
                 POT *speedPot, POT *torquePot)
    : NetworkComponent(PLUNGER_MB_BASE_ADDRESS, PLUNGER_COMPONENT_NAME,
                       COMPONENT_KEY_PLUNGER, Component::COMPONENT_DEFAULT,
                       owner),
      _vfd(vfd), _joystick(joystick), _speedPot(speedPot),
      _torquePot(torquePot), _currentFillState(FillState::NONE),
      _lastJoystickDirection(Joystick::E_POSITION::CENTER),
      _currentSpeedPotValue(0), _currentTorquePotValue(0),
      _calculatedPlungingSpeedHz(0), _lastStateChangeTimeMs(0),
      _jammedStartTimeMs(0), _lastVfdReadTimeMs(0), _joystickHoldStartTimeMs(0),
      _operationStartTimeMs(0), _currentMaxOperationTimeMs(0),
      _joystickReleasedSinceAutoStart(false), _autoModeEnabled(true),
      _lastDiagnosticLogTimeMs(0), _lastImmediateStopCheckTimeMs(0),
      _lastStateLogTimeMs(0), _recordedPlungeDurationMs(0),
      _recordModeStartTimeMs(0), _fillOperationStartTimeMs(0),
      _postFlowStartTimeMs(0), _currentPostFlowState(PostFlowState::NONE),
      _eventsDelegate(nullptr),
      m_state(this, PLUNGER_MB_STATE_OFFSET, "Plunger State"),
      m_command(this, PLUNGER_MB_COMMAND_OFFSET, "Plunger Command") {
  setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
}

short Plunger::init() {
  _vfdResetJam();
  _vfdStop();
  _transitionToState(PlungerState::IDLE);
  _lastJoystickDirection = Joystick::E_POSITION::CENTER;
  _lastStateChangeTimeMs = millis();
  _jammedStartTimeMs = 0;
  _lastVfdReadTimeMs = 0;
  _joystickHoldStartTimeMs = 0;
  _operationStartTimeMs = 0;
  _currentMaxOperationTimeMs = 0;
  _lastDiagnosticLogTimeMs = 0;
  _lastImmediateStopCheckTimeMs = 0;
  _lastStateLogTimeMs = 0;
  m_command.update(static_cast<short>(E_PlungerCommand::NO_COMMAND));
  _joystickReleasedSinceAutoStart = false;
  _autoModeEnabled = true;

  bool loadSettings = true;

  if (loadSettings) {
    if (_settings.load()) {
      L_INFO("[%s] Settings loaded from file during init.", name.c_str());
    } else {
      Log.warningln(
          "[%s] Could not load settings from file during init, using "
          "compile-time defaults. Attempting to save defaults to create file.",
          name.c_str());
      if (_settings.save()) { // Attempt to save the current (compile-time
                              // default) settings
        L_INFO("[%s] Default settings saved to file during init.",
               name.c_str());
      } else {
        L_ERROR("[%s] Failed to save default settings to file during init.",
                name.c_str());
      }
    }
  }
  _updatePotValues(); // Recalculates _calculatedPlungingSpeedHz based on
                      // potentially loaded _settings.speedFastHz
  _recordedPlungeDurationMs = _settings.replayDurationMs;

  _recordModeStartTimeMs = 0;
  _joystickRecordHoldTimer.detach();
  _replayPlungeTimer.detach();

  _currentFillState = FillState::NONE;
  _fillOperationStartTimeMs = 0;
  _fillSubStateTimer.detach();
  _joystickFillHoldTimer.detach();

  _postFlowStartTimeMs = 0;
  _currentPostFlowState = PostFlowState::NONE;
  _postFlowSubStateTimer.detach();
  return E_OK;
}

short Plunger::setup() {
  NetworkComponent::setup();

  const uint16_t baseAddr = mb_tcp_base_address();

  m_state.initNotify(static_cast<uint16_t>(PlungerState::IDLE), true,
                     NetworkValue_ThresholdMode::DIFFERENCE);
  m_state.initModbus(baseAddr + MB_OFS_STATE, 1, this->id, this->slaveId,
                     FN_READ_HOLD_REGISTER,
                     "Plunger "
                     "State:(0:Idle,1:HomingMan,2:HomingAuto,3:PlungingMan,4:"
                     "PlungingAuto,5:Stopping,6:Jammed,7:ResettingJam,8:Record,"
                     "9:Replay,10:Filling,11:PostFlow)",
                     this->name.c_str());
  registerBlock(m_state.getRegisterInfo());

  m_command.initNotify(static_cast<uint16_t>(E_PlungerCommand::NO_COMMAND),
                       true, NetworkValue_ThresholdMode::DIFFERENCE);
  m_command.initModbus(
      baseAddr + MB_OFS_COMMAND, 1, this->id, this->slaveId,
      FN_WRITE_HOLD_REGISTER,
      "Plunger Command:(0:None,1:Home,2:Plunge,3:Stop,4:Info,5:Fill,6:Replay)",
      this->name.c_str());
  registerBlock(m_command.getRegisterInfo());

  return this->init();
}

void Plunger::_updatePotValues() {
  if (!_speedPot || !_torquePot) {
    return;
  }
  _currentSpeedPotValue = _speedPot->getValue();
  _currentTorquePotValue = _torquePot->getValue();
  _calculatedPlungingSpeedHz =
      (_currentSpeedPotValue / 100.0f) * (_settings.speedFastHz * 100.0f);
}

void Plunger::_vfdStartForward(uint16_t frequencyCentiHz) {
  L_INFO("Plunger::_vfdStartForward: frequencyCentiHz: %d", frequencyCentiHz);
  _vfd->setFrequency(frequencyCentiHz); // DELTA_VFD::setFrequency expects
                                        // 0.01Hz units (centiHz)
  _vfd->run();
}

void Plunger::_vfdStartReverse(uint16_t frequencyCentiHz) {
  L_INFO("Plunger::_vfdStartReverse: frequencyCentiHz: %d", frequencyCentiHz);
  _vfd->setFrequency(frequencyCentiHz); // DELTA_VFD::setFrequency expects
                                        // 0.01Hz units (centiHz)
  _vfd->reverse();
}

void Plunger::_vfdStop() { _vfd->stop(); }

void Plunger::_vfdResetJam() { _vfd->resetFault(); }

void Plunger::_checkVfdForJam() {
  unsigned long currentTimeMs = millis();
  if (debug_jam && currentTimeMs - _lastDiagnosticLogTimeMs > 5000) {
    uint16_t diagCurrentMa = 0;
    bool diagReadSuccess = _vfd->getOutputCurrent(diagCurrentMa);
    L_INFO("[%s] --- DIAGNOSTIC LOG (debug_jam active) ---", name.c_str());
    L_INFO("State: %s, FillState: %s, PostFlowState: %s",
           _plungerStateToString(static_cast<PlungerState>(m_state.getValue())),
           (m_state.getValue() == static_cast<uint16_t>(PlungerState::FILLING)
                ? _fillStateToString(_currentFillState)
                : "N/A"),
           (m_state.getValue() == static_cast<uint16_t>(PlungerState::POST_FLOW)
                ? _postFlowStateToString(_currentPostFlowState)
                : "N/A"));
    L_INFO("VFD Running (reported): %d, VFD Fault (reported): %d",
           _vfd->isRunning(), _vfd->hasFault());
    L_INFO("VFD Current Read Success: %d, Current: %u mA", diagReadSuccess,
           diagCurrentMa);
    L_INFO("JammedStartTime: %lu ms (ago: %lu ms if active)",
           _jammedStartTimeMs,
           (_jammedStartTimeMs > 0 ? currentTimeMs - _jammedStartTimeMs : 0));
    L_INFO("JoystickHoldStartTime: %lu ms", _joystickHoldStartTimeMs);
    L_INFO("OperationStartTime: %lu ms, CurrentMaxOpTime: %lu ms",
           _operationStartTimeMs, _currentMaxOperationTimeMs);
    L_INFO("FillOperationStartTime: %lu ms", _fillOperationStartTimeMs);
    _lastDiagnosticLogTimeMs = currentTimeMs;
  }

  uint16_t vfdOutputCurrentMa = 0;
  bool readSuccess = _vfd->getOutputCurrent(vfdOutputCurrentMa);
  if (!readSuccess) {
    return;
  }

  bool motorExpectedActive = false;
  PlungerState currentState = static_cast<PlungerState>(m_state.getValue());
  if (currentState == PlungerState::HOMING_MANUAL ||
      currentState == PlungerState::HOMING_AUTO ||
      currentState == PlungerState::PLUNGING_MANUAL ||
      currentState == PlungerState::PLUNGING_AUTO ||
      currentState == PlungerState::RECORD ||
      (currentState == PlungerState::POST_FLOW &&
       _currentPostFlowState == PostFlowState::POST_FLOW_STARTING) ||
      (currentState == PlungerState::FILLING &&
       (_currentFillState == FillState::PLUNGING ||
        _currentFillState == FillState::HOMING))) {
    motorExpectedActive = true;
  }

  if (_vfd->hasFault()) {
    _transitionToState(PlungerState::JAMMED);
    return;
  }

  if (!motorExpectedActive) {
    if (_jammedStartTimeMs != 0) {
      _jammedStartTimeMs = 0;
    }
    return;
  }

  float torqueMultiplier = 1.0f;
  uint16_t adjustedJamThresholdMa = _settings.currentJamThresholdMa;

  bool isPlungingState = (currentState == PlungerState::PLUNGING_MANUAL ||
                          currentState == PlungerState::PLUNGING_AUTO ||
                          currentState == PlungerState::RECORD ||
                          (currentState == PlungerState::FILLING &&
                           _currentFillState == FillState::PLUNGING));

  if (currentState == PlungerState::POST_FLOW &&
      _currentPostFlowState == PostFlowState::POST_FLOW_STARTING) {
    adjustedJamThresholdMa = _settings.currentPostFlowMa;
  } else if (isPlungingState) {
    torqueMultiplier = (100.0f - _currentTorquePotValue) / 100.0f;
    adjustedJamThresholdMa = static_cast<uint16_t>(
        _settings.currentJamThresholdMa * torqueMultiplier);
  }

  if (vfdOutputCurrentMa >= adjustedJamThresholdMa) {
    if (_jammedStartTimeMs == 0) {
      _jammedStartTimeMs = millis();
    }

    unsigned long jamDurationTargetMs = 0;
    if (currentState == PlungerState::FILLING) {
      if (_currentFillState == FillState::PLUNGING)
        jamDurationTargetMs = _settings.jammedDurationMs;
      else if (_currentFillState == FillState::HOMING)
        jamDurationTargetMs = _settings.jammedDurationHomingMs;
    } else if (currentState == PlungerState::POST_FLOW &&
               _currentPostFlowState == PostFlowState::POST_FLOW_STARTING) {
      jamDurationTargetMs = _settings.jammedDurationMs;
    } else if (currentState == PlungerState::HOMING_MANUAL ||
               currentState == PlungerState::HOMING_AUTO) {
      jamDurationTargetMs = _settings.jammedDurationHomingMs;
    } else if (currentState == PlungerState::PLUNGING_MANUAL ||
               currentState == PlungerState::PLUNGING_AUTO ||
               currentState == PlungerState::RECORD) {
      jamDurationTargetMs = _settings.jammedDurationMs;
    }

    if (_jammedStartTimeMs > 0 &&
        (millis() - _jammedStartTimeMs > _settings.maxUniversalJamTimeMs)) {
      L_ERROR("[%s] UNIVERSAL JAM TIMEOUT! Current %u mA for >%lums. State: "
              "%s, FillState: %s. JAMMED.",
              name.c_str(), vfdOutputCurrentMa, millis() - _jammedStartTimeMs,
              _plungerStateToString(currentState),
              currentState == PlungerState::FILLING
                  ? _fillStateToString(_currentFillState)
                  : "N/A");
      _transitionToState(PlungerState::JAMMED);
      return;
    }

    if (jamDurationTargetMs > 0 && _jammedStartTimeMs > 0 &&
        (millis() - _jammedStartTimeMs > jamDurationTargetMs)) {
      if (currentState == PlungerState::PLUNGING_AUTO &&
          _settings.enablePostFlow) {
        _transitionToState(PlungerState::POST_FLOW);
      } else if (currentState == PlungerState::FILLING) {
        _vfdStop();
        if (_currentFillState == FillState::PLUNGING) {
          _currentFillState = FillState::PLUNGED;
          _fillSubStateTimer.once_ms(_settings.fillPlungedWaitDurationMs,
                                     &Plunger::_fillSubStateTimerRelay, this);
          _jammedStartTimeMs = 0;
        } else if (_currentFillState == FillState::HOMING) {
          _currentFillState = FillState::HOMED;
          _fillSubStateTimer.once_ms(_settings.fillHomedWaitDurationMs,
                                     &Plunger::_fillSubStateTimerRelay, this);
          _jammedStartTimeMs = 0;
        }
      } else {
        _transitionToState(PlungerState::JAMMED);
      }
    } else if (_jammedStartTimeMs > 0) {
    }
  } else // Current is NOT above threshold
  {
    if (_jammedStartTimeMs != 0) // If it *was* timing a jam
    {
      _jammedStartTimeMs = 0;
    }
  }
}

short Plunger::loop() {
  NetworkComponent::loop();
  _updatePotValues();
  _checkVfdForJam();
  if (!m_enabled.getValue()) {
    return E_OK;
  }
  if (debug_states) {
    unsigned long currentTimeMs = millis();
    if (currentTimeMs - _lastStateLogTimeMs >= 10000) {
      L_INFO("[%s] --- STATE LOG DUMP (debug_states active) ---", name.c_str());
      L_INFO("  CurrentTime: %lu ms", currentTimeMs);
      L_INFO(
          "  State: %s (%d)",
          _plungerStateToString(static_cast<PlungerState>(m_state.getValue())),
          m_state.getValue());
      if (m_state.getValue() == static_cast<uint16_t>(PlungerState::FILLING)) {
        L_INFO("  FillState: %s (%d)", _fillStateToString(_currentFillState),
               static_cast<int>(_currentFillState));
      }
      if (m_state.getValue() ==
          static_cast<uint16_t>(PlungerState::POST_FLOW)) {
        L_INFO("  PostFlowState: %s (%d)",
               _postFlowStateToString(_currentPostFlowState),
               static_cast<int>(_currentPostFlowState));
      }
      L_INFO("  Timers (ms):");
      L_INFO("    OperationStart: %d (Max: %d)", _operationStartTimeMs,
             _currentMaxOperationTimeMs);
      L_INFO("    JammedStart: %lu", _jammedStartTimeMs);
      L_INFO("    JoystickHoldStart: %lu", _joystickHoldStartTimeMs);
      L_INFO("    FillOperationStart: %lu", _fillOperationStartTimeMs);
      L_INFO("    PostFlowStart: %lu", _postFlowStartTimeMs);
      L_INFO("    RecordModeStart: %lu", _recordModeStartTimeMs);
      L_INFO("    LastStateChange: %lu", _lastStateChangeTimeMs);
      _lastStateLogTimeMs = currentTimeMs;
      L_INFO("[%s] --- END STATE LOG DUMP ---", name.c_str());
    }
  }
  Joystick::E_POSITION currentJoystickDir =
      static_cast<Joystick::E_POSITION>(_joystick->getValue());
  if (_operationStartTimeMs > 0 && _currentMaxOperationTimeMs > 0) {
    bool isMonitoredState = false;
    PlungerState currentState = static_cast<PlungerState>(m_state.getValue());
    if (currentState == PlungerState::HOMING_MANUAL ||
        currentState == PlungerState::HOMING_AUTO ||
        currentState == PlungerState::PLUNGING_MANUAL ||
        currentState == PlungerState::PLUNGING_AUTO ||
        currentState == PlungerState::RECORD ||
        currentState == PlungerState::REPLAY) {
      isMonitoredState = true;
    } else if (currentState == PlungerState::FILLING &&
               (_currentFillState == FillState::PLUNGING ||
                _currentFillState == FillState::HOMING)) {
      isMonitoredState = true;
    } else if (currentState == PlungerState::POST_FLOW &&
               _currentPostFlowState == PostFlowState::POST_FLOW_STARTING) {
      isMonitoredState = true;
    }

    if (isMonitoredState &&
        (millis() - _operationStartTimeMs > _currentMaxOperationTimeMs)) {
      Log.warningln(
          "[%s] GENERIC MAX OPERATION TIME (%lu ms) EXCEEDED! State: %s, "
          "FillState: %s, PostFlowState: %s. Transitioning to JAMMED.",
          name.c_str(), _currentMaxOperationTimeMs,
          _plungerStateToString(currentState),
          currentState == PlungerState::FILLING
              ? _fillStateToString(_currentFillState)
              : "N/A",
          currentState == PlungerState::POST_FLOW
              ? _postFlowStateToString(_currentPostFlowState)
              : "N/A");
      _transitionToState(PlungerState::JAMMED);
    }
  }

  PlungerState currentState = static_cast<PlungerState>(m_state.getValue());
  switch (currentState) {
  case PlungerState::IDLE:
    _handleIdleState();
    break;
  case PlungerState::HOMING_MANUAL:
    _handleHomingManualState();
    break;
  case PlungerState::HOMING_AUTO:
    _handleHomingAutoState();
    break;
  case PlungerState::PLUNGING_MANUAL:
    _handlePlungingManualState();
    break;
  case PlungerState::PLUNGING_AUTO:
    _handlePlungingAutoState();
    break;
  case PlungerState::STOPPING:
    _handleStoppingState();
    break;
  case PlungerState::JAMMED:
    _handleJammedState();
    break;
  case PlungerState::RESETTING_JAM:
    _handleResettingJamState();
    break;
  case PlungerState::RECORD:
    _handleRecordState();
    break;
  case PlungerState::REPLAY:
    _handleReplayState();
    break;
  case PlungerState::FILLING: // Re-added case
    _handleFillingState();
    break;
  case PlungerState::POST_FLOW:
    _handlePostFlowState();
    break;
  default:
    Log.warningln("[%s] Unknown state: %d. Transitioning to IDLE.",
                  name.c_str(), static_cast<uint8_t>(currentState));
    _transitionToState(PlungerState::IDLE);
    break;
  }
  _lastJoystickDirection = currentJoystickDir;
  return E_OK;
}

short Plunger::info() {
  L_INFO("--- Plunger Info (ID: %d, Name: %s) ---", id, name.c_str());
  L_INFO("State: %d, LastJoy: %d, CurrentJoy: %d",
         static_cast<uint8_t>(m_state.getValue()),
         static_cast<int>(_lastJoystickDirection),
         static_cast<int>(_joystick->getValue()));

  L_INFO("SpeedPOT: %d, TorquePOT: %d", _currentSpeedPotValue,
         _currentTorquePotValue);
  uint16_t freq = 0;
  uint16_t current = 0;
  bool freqValid = _vfd->getFrequency(freq);
  bool currentOk = _vfd->getOutputCurrent(current);

  L_INFO("VFD: Running=%s, Fault=%s, FreqSet=%d Hz, OutputCurrent=%d",
         _vfd->isRunning() ? "YES" : "NO", _vfd->hasFault() ? "YES" : "NO",
         freqValid ? (static_cast<int>(freq)) : -1,
         currentOk ? static_cast<int>(current) : -1);

  L_INFO("--- Plunger Settings ---");
  _settings.print();

  return E_OK;
}

short Plunger::debug() { return info(); }

short Plunger::cmd_plunge() {
  if (!_autoModeEnabled) {
    return 1;
  }
  if (m_state.getValue() == static_cast<uint16_t>(PlungerState::IDLE)) {
    _vfdStartForward(static_cast<uint16_t>(_calculatedPlungingSpeedHz));
    _transitionToState(PlungerState::PLUNGING_AUTO);
    return E_OK;
  } else {
    return 1;
  }
}

short Plunger::cmd_home() {
  if (!_autoModeEnabled) {
    return 1;
  }
  if (m_state.getValue() == static_cast<uint16_t>(PlungerState::IDLE)) {
    _vfdStartReverse(static_cast<uint16_t>(_settings.speedSlowHz * 100.0f));
    _transitionToState(PlungerState::HOMING_AUTO);
    return E_OK;
  } else {
    return 1;
  }
}

short Plunger::cmd_stop() {
  _transitionToState(PlungerState::STOPPING);
  _vfdStop();
  _vfdResetJam();
  return E_OK;
}

// Definition for cmd_fill
short Plunger::cmd_fill() {
  if (m_state.getValue() != static_cast<uint16_t>(PlungerState::IDLE)) {
    Log.warningln(
        "[%s] cmd_fill ignored. Not IDLE. Current State: %s", name.c_str(),
        _plungerStateToString(static_cast<PlungerState>(m_state.getValue())));
    return 1;
  }
  if (!_autoModeEnabled) {
    Log.warningln("[%s] cmd_fill ignored. Auto mode is disabled.",
                  name.c_str());
    return 1;
  }
  L_INFO("[%s] cmd_fill: Initiating FILLING sequence.", name.c_str());
  _currentFillState = FillState::PLUNGING;
  _joystickReleasedSinceAutoStart = false;
  _vfdStartForward(static_cast<uint16_t>(_settings.speedFillPlungeHz * 100.0f));
  _operationStartTimeMs = millis();
  _currentMaxOperationTimeMs = _settings.defaultMaxOperationDurationMs;
  _transitionToState(PlungerState::FILLING);
  return E_OK;
}

short Plunger::cmd_save_settings() {
  if (_settings.save()) {
    return E_OK;
  } else {
    L_ERROR("[%s] Failed to save settings via command.", name.c_str());
    return 1;
  }
}

// Definition for reset
short Plunger::reset() {
  //_vfdStop();
  //_vfdResetJam();
  return this->init();
}
void Plunger::setAutoModeEnabled(bool enabled) {
  if (_autoModeEnabled != enabled) {
    _autoModeEnabled = enabled;
    _fireEvent(enabled ? PlungerEvent::AUTO_MODE_ENABLED
                       : PlungerEvent::AUTO_MODE_DISABLED);
  }
}
bool Plunger::isAutoModeEnabled() const { return _autoModeEnabled; }

// Definition for cmd_enableAutoMode
short Plunger::cmd_enableAutoMode() {
  setAutoModeEnabled(true);
  return E_OK;
}

// Definition for cmd_disableAutoMode
short Plunger::cmd_disableAutoMode() {
  setAutoModeEnabled(false);
  return E_OK;
}

// Method to serialize current settings to a JsonDocument
void Plunger::getSettingsJson(JsonDocument &doc) const {
  _settings.toJson(doc); // Utilize the existing method in PlungerSettings
}

// Method to update settings from a JsonObject and then save them
bool Plunger::updateSettingsFromJson(const JsonObject &json) {
  if (!_settings.fromJson(
          json)) { // Utilize the existing method in PlungerSettings
    L_ERROR(
        "[%s] Failed to update settings from JSON in updateSettingsFromJson.",
        name.c_str());
    return false;
  }
  if (!_settings.save()) {
    L_ERROR("[%s] Failed to save updated settings in updateSettingsFromJson.",
            name.c_str());
    return false;
  }
  _recordedPlungeDurationMs = _settings.replayDurationMs;
  _updatePotValues();

  L_INFO("[%s] Settings updated via web API:", name.c_str());
  _settings.print();

  return true;
}
short Plunger::mb_tcp_write(MB_Registers *reg, short networkValue) {
  short result = NetworkComponent::mb_tcp_write(reg, networkValue);
  if (result != E_NOT_IMPLEMENTED) {
    return result;
  }
  uint16_t address = reg->startAddress;
  if (address == (_baseAddress + MB_OFS_COMMAND)) {
    E_PlungerCommand cmd = static_cast<E_PlungerCommand>(networkValue);

    short cmdResult = E_OK;
    switch (cmd) {
    case E_PlungerCommand::CMD_HOME:
      cmdResult = this->cmd_home();
      break;
    case E_PlungerCommand::CMD_PLUNGE:
      cmdResult = this->cmd_plunge();
      break;
    case E_PlungerCommand::CMD_STOP:
      cmdResult = this->cmd_stop();
      break;
    case E_PlungerCommand::CMD_INFO:
      cmdResult = this->info();
      break;
    case E_PlungerCommand::CMD_FILL:
      cmdResult = this->cmd_fill();
      break;
    case E_PlungerCommand::CMD_REPLAY:
      cmdResult = this->cmd_replay();
      break;
    case E_PlungerCommand::NO_COMMAND:
      L_INFO("[%s] Modbus NO_COMMAND received.", name.c_str());
      break;
    default:
      Log.warningln("[%s] Unknown Modbus command received: %d", name.c_str(),
                    networkValue);
      cmdResult = E_INVALID_PARAMETER;
      break;
    }

    m_command.update(networkValue);

    return cmdResult;
  }

  return E_INVALID_PARAMETER;
}

short Plunger::mb_tcp_read(MB_Registers *reg) {
  short result = NetworkComponent::mb_tcp_read(reg);
  if (result != E_NOT_IMPLEMENTED) {
    return result;
  }

  return E_INVALID_PARAMETER;
}

short Plunger::cmd_load_default_settings() {
  if (loadDefaultSettings()) {
    return E_OK;
  } else {
    L_ERROR("[%s] Failed to load or apply default settings via command.",
            name.c_str());
    return 1;
  }
}

bool Plunger::loadDefaultSettings(const char *defaultPath,
                                  const char *operationalPath) {
  PlungerSettings tempSettings = _settings;
  if (!tempSettings.load(defaultPath)) {
    L_ERROR("[%s] Failed to load settings from default file: %s", name.c_str(),
            defaultPath);
    return false;
  }
  _settings = tempSettings;
  if (!_settings.save(operationalPath)) {
    L_ERROR("[%s] Failed to save the loaded default settings to operational "
            "path: %s",
            name.c_str(), operationalPath);
    return false;
  }
  _recordedPlungeDurationMs = _settings.replayDurationMs;
  _updatePotValues();
  return true;
}

short Plunger::cmd_replay() {
  if (m_state.getValue() != static_cast<uint16_t>(PlungerState::IDLE)) {
    Log.warningln(
        "[%s] cmd_replay ignored. Not IDLE. Current State: %s", name.c_str(),
        _plungerStateToString(static_cast<PlungerState>(m_state.getValue())));
    return 1;
  }
  if (_recordedPlungeDurationMs <= 50) { // Same check as in _handleReplayState
    Log.warningln(
        "[%s] cmd_replay ignored. Invalid or zero replay duration (%lu ms).",
        name.c_str(), _recordedPlungeDurationMs);
    return 1;
  }
  if (!_autoModeEnabled) // Consider if replay should be subject to
                         // autoModeEnabled
  {
    Log.warningln("[%s] cmd_replay ignored. Auto mode is disabled.",
                  name.c_str());
    return 1;
  }
  _transitionToState(PlungerState::REPLAY);
  return E_OK;
}

void Plunger::_fireEvent(PlungerEvent event, int16_t val) {
  if (_eventsDelegate) {
    _eventsDelegate->onPlungerEvent(this, event, val);
  }
}

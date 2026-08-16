#include "Plunger.h"
#include <Arduino.h> // For millis(), delay()

void Plunger::_transitionToState(PlungerState newState)
{
    if (_currentState == newState)
        return; // No change, do nothing further.

    Log.verboseln("[%s] State transition: %s -> %s", name.c_str(), _plungerStateToString(_currentState), _plungerStateToString(newState));
    PlungerState oldState = _currentState;
    
    // Specific exit logic for states
    if (oldState == PlungerState::POST_FLOW) {
        // Clean up if we are LEAVING post_flow, regardless of destination
        _currentPostFlowState = PostFlowState::NONE;
        _postFlowSubStateTimer.detach();
    }

    // Default timer resets, will be overridden by specific states if needed
    // unsigned long newOperationStartTimeMs = 0; // Not needed here anymore
    // unsigned long newCurrentMaxOperationTimeMs = 0; // Not needed here anymore

    if (newState != PlungerState::JAMMED)
    {
        _jammedStartTimeMs = 0;
    }

    if (newState == PlungerState::IDLE ||
        newState == PlungerState::STOPPING ||
        newState == PlungerState::JAMMED ||
        newState == PlungerState::RESETTING_JAM ||
        newState == PlungerState::HOMING_AUTO || // Joystick hold reset for auto states too
        newState == PlungerState::PLUNGING_AUTO)
    {
        _joystickHoldStartTimeMs = 0;
    }

    // Specific entry logic for states
    if (newState == PlungerState::POST_FLOW) {
        Log.infoln("[%s] Initiating POST_FLOW sequence.", name.c_str());
        _vfdStop(); // Initial stop as per PostFlowState::POST_FLOW_STOPPING
        _currentPostFlowState = PostFlowState::POST_FLOW_STOPPING;
        _postFlowSubStateTimer.once_ms(_settings.postFlowStoppingWaitMs, &Plunger::_postFlowSubStateTimerRelay, this);
    }

    if (newState == PlungerState::IDLE || newState == PlungerState::STOPPING) // General cleanup for IDLE/STOPPING
    {
        _currentFillState = FillState::NONE;
        _fillSubStateTimer.detach();
        _currentPostFlowState = PostFlowState::NONE; // Also clean up post-flow sub-state if not already handled by exit logic
        _postFlowSubStateTimer.detach();
    }
    
    // Specific state initializations for timers
    if (newState == PlungerState::HOMING_MANUAL || newState == PlungerState::HOMING_AUTO ||
             newState == PlungerState::PLUNGING_MANUAL || newState == PlungerState::PLUNGING_AUTO)
    {
        // These states might have their own max operation times from the moment they are entered.
        // Or, if a general max time for these states is desired from entry:
        // newOperationStartTimeMs = millis();
        // newCurrentMaxOperationTimeMs = SOME_DEFAULT_MAX_FOR_THESE_STATES; // if applicable
    }

    _currentState = newState; // Set the new state now
    _lastStateChangeTimeMs = millis(); // Set last state change time now

    // Initialize/reset operation timers. They will be set specifically if the new state involves timed motor operation,
    // or by the functions that start specific motor movements within a larger state (e.g., FILLING sub-states).
    _operationStartTimeMs = 0;
    _currentMaxOperationTimeMs = 0;

    // Specific entry logic for states regarding operation timers
    switch (newState)
    {
        case PlungerState::HOMING_MANUAL:
        case PlungerState::HOMING_AUTO:
        case PlungerState::PLUNGING_MANUAL:
        case PlungerState::PLUNGING_AUTO:
            _operationStartTimeMs = millis();
            _currentMaxOperationTimeMs = _settings.defaultMaxOperationDurationMs; // Use from _settings
            break;

        case PlungerState::FILLING:
            _fillOperationStartTimeMs = millis(); // For the overall fill operation.
            // Moving sub-states (PLUNGING, HOMING) will set their own _operationStartTimeMs
            // and _currentMaxOperationTimeMs for the generic timeout check.
            break;

        case PlungerState::POST_FLOW:
            Log.infoln("[%s] Initiating POST_FLOW sequence.", name.c_str());
            _vfdStop(); 
            _currentPostFlowState = PostFlowState::POST_FLOW_STOPPING;
            _postFlowSubStateTimer.once_ms(_settings.postFlowStoppingWaitMs, &Plunger::_postFlowSubStateTimerRelay, this);
            // POST_FLOW_STARTING sub-state will set its own _operationStartTimeMs and _currentMaxOperationTimeMs.
            break;
        
        default:
            // For states like IDLE, STOPPING, JAMMED, RESETTING_JAM, timers remain 0.
            break;
    }

    if (newState == PlungerState::IDLE || newState == PlungerState::STOPPING) // General cleanup for IDLE/STOPPING
    {
        _currentFillState = FillState::NONE;
        _fillSubStateTimer.detach();
        _currentPostFlowState = PostFlowState::NONE; // Also clean up post-flow sub-state if not already handled by exit logic
        _postFlowSubStateTimer.detach();
    }
    
    // Specific state initializations for timers were here, now handled in switch or state handlers

    // Apply the determined timer values - ALREADY DONE IN SWITCH OR STATE HANDLERS
    // _operationStartTimeMs = newOperationStartTimeMs;
    // _currentMaxOperationTimeMs = newCurrentMaxOperationTimeMs;

    if (newState == PlungerState::HOMING_AUTO || newState == PlungerState::PLUNGING_AUTO)
    {
        _joystickReleasedSinceAutoStart = false;
    }

    owner->onError(id, static_cast<short>(newState));
}

void Plunger::_handleIdleState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    if (joyDir == Joystick::E_POSITION::UP && _lastJoystickDirection != Joystick::E_POSITION::UP)
    {
        _joystickHoldStartTimeMs = millis();
        _vfdStartReverse(static_cast<uint16_t>(_settings.speedSlowHz * 100.0f));
        _transitionToState(PlungerState::HOMING_MANUAL);
    }
    else if (joyDir == Joystick::E_POSITION::DOWN && _lastJoystickDirection != Joystick::E_POSITION::DOWN)
    {
        _joystickHoldStartTimeMs = millis();
        _vfdStartForward(static_cast<uint16_t>(_calculatedPlungingSpeedHz));
        _transitionToState(PlungerState::PLUNGING_MANUAL);
    }
    else if (joyDir == Joystick::E_POSITION::LEFT && _lastJoystickDirection != Joystick::E_POSITION::LEFT)
    {
        if (_currentState == PlungerState::IDLE) { 
            _joystickHoldStartTimeMs = millis(); 
            _joystickFillHoldTimer.once_ms(_settings.fillJoystickHoldDurationMs, &Plunger::_joystickFillHoldTimerRelay, this);
        } 
    }
    else if (_lastJoystickDirection == Joystick::E_POSITION::LEFT && joyDir != Joystick::E_POSITION::LEFT)
    {
        if (_joystickHoldStartTimeMs != 0) { 
            _joystickFillHoldTimer.detach();
            _joystickHoldStartTimeMs = 0; 
        }
    }
    else if (joyDir == Joystick::E_POSITION::RIGHT && _lastJoystickDirection != Joystick::E_POSITION::RIGHT)
    {
        _joystickHoldStartTimeMs = millis(); 
        _joystickRecordHoldTimer.once_ms(_settings.recordHoldDurationMs, &Plunger::_joystickRecordHoldTimerRelay, this);
    }
    else if (_lastJoystickDirection == Joystick::E_POSITION::RIGHT && joyDir != Joystick::E_POSITION::RIGHT) 
    {
        _joystickRecordHoldTimer.detach(); 
        unsigned long heldDuration = (_joystickHoldStartTimeMs > 0) ? (millis() - _joystickHoldStartTimeMs) : 0; 
        _joystickHoldStartTimeMs = 0; 
        if (heldDuration < _settings.recordHoldDurationMs && heldDuration > 50) { 
            if (_recordedPlungeDurationMs > 0) {
                _transitionToState(PlungerState::REPLAY);
            } 
        } 
    }

    if (_vfd->isRunning() && _joystickHoldStartTimeMs == 0 && joyDir == Joystick::E_POSITION::CENTER && _currentState == PlungerState::IDLE)
    {
        Log.warningln("[%s] IDLE: VFD unexpectedly running. Stopping.", name.c_str());
        _vfdStop();
    }
}

void Plunger::_handleHomingManualState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    unsigned long currentTimeMs = millis(); 
    if (joyDir == Joystick::E_POSITION::UP)
    {
        if (_autoModeEnabled && _joystickHoldStartTimeMs > 0 && (currentTimeMs - _joystickHoldStartTimeMs > _settings.autoModeHoldDurationMs)) // Use from _settings
        {
            _transitionToState(PlungerState::HOMING_AUTO);
        }
    }
    else
    {
        _transitionToState(PlungerState::STOPPING);
    }
}

void Plunger::_handleHomingAutoState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    Joystick::E_POSITION initialDir = Joystick::E_POSITION::UP; // For homing

    if (!_joystickReleasedSinceAutoStart)
    { // Phase 1: Joystick hasn't been centered yet since auto start
        if (joyDir == initialDir)
        {
            // Still holding initial direction, all good.
        }
        else if (joyDir == Joystick::E_POSITION::CENTER)
        {
            _joystickReleasedSinceAutoStart = true; // First release to center, mark and continue.
        }
        else
        {
            _transitionToState(PlungerState::STOPPING);
        }
    }
    else
    { // Phase 2: Joystick has been centered at least once
        if (joyDir != Joystick::E_POSITION::CENTER)
        {
            // Joystick moved away from CENTER after being released there. This is the "change again". Abort.
            _transitionToState(PlungerState::STOPPING);
        }
        // If joyDir is still CENTER, do nothing, continue auto mode.
    }
}

void Plunger::_handlePlungingManualState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    unsigned long currentTimeMs = millis(); 

    if (joyDir == Joystick::E_POSITION::DOWN)
    {
        if (_autoModeEnabled && _joystickHoldStartTimeMs > 0 && (currentTimeMs - _joystickHoldStartTimeMs > _settings.autoModeHoldDurationMs)) // Use from _settings
        {
            _transitionToState(PlungerState::PLUNGING_AUTO);
        }
    }
    else
    {
        _transitionToState(PlungerState::STOPPING);
    }
}

void Plunger::_handlePlungingAutoState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    Joystick::E_POSITION initialDir = Joystick::E_POSITION::DOWN; // For plunging

    if (!_joystickReleasedSinceAutoStart)
    { // Phase 1: Joystick hasn't been centered yet since auto start
        if (joyDir == initialDir)
        {
            // Still holding initial direction, all good.
        }
        else if (joyDir == Joystick::E_POSITION::CENTER)
        {
            _joystickReleasedSinceAutoStart = true; // First release to center, mark and continue.
        }
        else
        {
            // Moved from initial direction to something other than CENTER. Abort.
            _transitionToState(PlungerState::STOPPING);
        }
    }
    else
    { // Phase 2: Joystick has been centered at least once
        if (joyDir != Joystick::E_POSITION::CENTER)
        {
            _transitionToState(PlungerState::STOPPING);
        }
        // If joyDir is still CENTER, do nothing, continue auto mode.
    }
}

void Plunger::_handleStoppingState()
{
    _vfdStop();
    _joystickHoldStartTimeMs = 0;
    _transitionToState(PlungerState::IDLE);
}

void Plunger::_handleJammedState()
{
    _vfdResetJam(); 
    _vfdStop();
    _joystickHoldStartTimeMs = 0;
    _transitionToState(PlungerState::RESETTING_JAM);    
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
}

void Plunger::_handleResettingJamState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    if (joyDir == Joystick::E_POSITION::UP && _lastJoystickDirection != Joystick::E_POSITION::UP)
    {
        _vfdResetJam();
        _joystickHoldStartTimeMs = millis();
        _vfdStartReverse(static_cast<uint16_t>(_settings.speedSlowHz * 100.0f));
        _transitionToState(PlungerState::HOMING_MANUAL);
    }
    else if (joyDir == Joystick::E_POSITION::CENTER)
    {
        // User is waiting or deciding, do nothing, VFD is stopped from _handleJammedState
    }
    else if (joyDir != Joystick::E_POSITION::UP && _lastJoystickDirection == Joystick::E_POSITION::CENTER)
    {
        // Joystick moved from CENTER to something other than UP (e.g., DOWN, LEFT, RIGHT)
        // This is considered an intentional action to exit the JAMMED/RESETTING_JAM sequence without homing.
        Log.infoln("[%s] Resetting Jam: Joystick moved from CENTER not to UP. Returning to IDLE. Manual VFD reset might be needed.", name.c_str());
        _vfdResetJam(); // Attempt reset one last time just in case
        _transitionToState(PlungerState::IDLE);
    }
    // If joystick remains UP, or moves from UP to CENTER then back to UP, it stays in HOMING_MANUAL (or transitions there)
    // If joystick is moved from UP to CENTER and stays CENTER, HOMING_MANUAL will transition to STOPPING then IDLE.
}

void Plunger::_handleRecordState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    if (_recordModeStartTimeMs == 0) {
        // Log.infoln("[%s] RECORD: Starting plunge.", name.c_str());
        _vfdStartForward(static_cast<uint16_t>(_calculatedPlungingSpeedHz));
        _recordModeStartTimeMs = millis(); // For calculating recorded duration
        _operationStartTimeMs = millis(); // For generic timeout check
        _currentMaxOperationTimeMs = _settings.maxRecordDurationMs; // Use from _settings
    }
    if (joyDir != Joystick::E_POSITION::RIGHT) {
        _vfdStop();
        _recordedPlungeDurationMs = millis() - _recordModeStartTimeMs;
        Log.infoln("[%s] RECORD complete. Duration: %lu ms.", name.c_str(), _recordedPlungeDurationMs);
        _recordModeStartTimeMs = 0; 
        _operationStartTimeMs = 0;    
        _currentMaxOperationTimeMs = 0;
        _transitionToState(PlungerState::IDLE);
        return;
    }
}

void Plunger::_handleReplayState()
{
    if (_operationStartTimeMs == 0) { // First entry or re-entry after interruption for some reason
        if (_recordedPlungeDurationMs <= 50) { // Check for a minimal valid duration
            Log.warningln("[%s] REPLAY: Invalid or zero replay duration (%lu ms). -> IDLE", name.c_str(), _recordedPlungeDurationMs);
            _transitionToState(PlungerState::IDLE);
            return;
        }
        Log.infoln("[%s] REPLAY: Plunging for %lu ms.", name.c_str(), _recordedPlungeDurationMs);
        _vfdStartForward(static_cast<uint16_t>(_calculatedPlungingSpeedHz));
        _operationStartTimeMs = millis(); // For generic timeout check
        _currentMaxOperationTimeMs = _recordedPlungeDurationMs; // Specific timeout for replay
        _replayPlungeTimer.once_ms(_recordedPlungeDurationMs, &Plunger::_replayPlungeTimerRelay, this);
    }
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    if (joyDir != Joystick::E_POSITION::CENTER && _operationStartTimeMs != 0) { 
        Log.infoln("[%s] REPLAY: Interrupted by joystick.", name.c_str());
        _vfdStop();
        _replayPlungeTimer.detach(); 
        _operationStartTimeMs = 0;   
        _transitionToState(PlungerState::STOPPING); 
    }
}

void Plunger::_handleFillingState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());
    // Log.verboseln("[%s] FILLING: JoyDir=%d, ReleasedSinceStart=%d, FillState=%s", 
    //                name.c_str(), static_cast<int>(joyDir), 
    //                _joystickReleasedSinceAutoStart, 
    //                _fillStateToString(_currentFillState)); // DEBUG REMOVED

    if (_currentState != PlungerState::FILLING) { 
        // Log.warningln("[%s] FILLING: Unexpected current state %s! -> IDLE", name.c_str(), _plungerStateToString(_currentState)); // DEBUG REMOVED (can be a regular warning if this case is problematic)
        _currentFillState = FillState::NONE; 
        _fillSubStateTimer.detach();
        _transitionToState(PlungerState::IDLE);
        return;
    }

    if (!_joystickReleasedSinceAutoStart && joyDir == Joystick::E_POSITION::CENTER) {
        // Log.infoln("[%s] FILLING: Joystick detected at CENTER. Setting ReleasedSinceAutoStart = true.", name.c_str()); // DEBUG REMOVED
        _joystickReleasedSinceAutoStart = true;
    }

    if (_joystickReleasedSinceAutoStart && joyDir != Joystick::E_POSITION::CENTER) { 
        // Log.infoln("[%s] FILLING sequence: Interrupted by joystick (JoyDir=%d after release). Current FillState: %s -> STOPPING", 
        //            name.c_str(), static_cast<int>(joyDir), _fillStateToString(_currentFillState)); // DEBUG REMOVED (original log was info, can be restored if needed)
        _vfdStop();
        _fillSubStateTimer.detach();
        _currentFillState = FillState::NONE; 
        _transitionToState(PlungerState::STOPPING); 
        return;
    }
    // ... (placeholder for total fill operation timeout can remain commented)
}

// Static relay implementations for Ticker callbacks
void Plunger::_joystickRecordHoldTimerRelay(Plunger* pThis) {
    if (pThis) {
        pThis->_onJoystickRecordHoldTimeout();
    }
}

void Plunger::_replayPlungeTimerRelay(Plunger* pThis) {
    if (pThis) {
        pThis->_onReplayPlungeTimeout();
    }
}

void Plunger::_fillSubStateTimerRelay(Plunger* pThis) { 
    if (pThis) {
        pThis->_onFillSubStateTimeout();
    }
}

// Actual timer event handler implementations
void Plunger::_onJoystickRecordHoldTimeout() {
    if (static_cast<Joystick::E_POSITION>(_joystick->getValue()) == Joystick::E_POSITION::RIGHT && _currentState == PlungerState::IDLE) {
         Log.infoln("[%s] RECORD initiated from joystick.", name.c_str());
         _transitionToState(PlungerState::RECORD);
    } 
    _joystickHoldStartTimeMs = 0;
}

void Plunger::_onReplayPlungeTimeout() {
    this->_vfdStop();
    this->_operationStartTimeMs = 0; 

    if (_settings.enablePostFlow) {
        Log.infoln("[%s] Replay plunge finished. Post-flow enabled. -> POST_FLOW", name.c_str());
        this->_transitionToState(PlungerState::POST_FLOW);
    } else {
        Log.infoln("[%s] Replay plunge finished. Post-flow disabled. -> IDLE", name.c_str());
        this->_transitionToState(PlungerState::IDLE);
    }
}

void Plunger::_onFillSubStateTimeout() {
    if (_currentState != PlungerState::FILLING) { 
        Log.warningln("[%s] Fill Sub-State Timeout: Not in FILLING state. Aborting timer logic.", name.c_str());
        _fillSubStateTimer.detach();
        _currentFillState = FillState::NONE; 
        return; 
    }
    switch (_currentFillState) {
        case FillState::PLUNGED: 
            // Log.infoln("[%s] Fill Sub-State: PLUNGED wait over. -> HOMING.", name.c_str());
            _currentFillState = FillState::HOMING;
            _vfdStartReverse(static_cast<uint16_t>(_settings.speedFillHomeHz * 100.0f));
            _operationStartTimeMs = millis();
            _currentMaxOperationTimeMs = _settings.defaultMaxOperationDurationMs;
            break;
        case FillState::HOMED: 
            Log.infoln("[%s] FILLING sequence complete.", name.c_str());
            _currentFillState = FillState::NONE;
            _transitionToState(PlungerState::IDLE); 
            break;
        default:
            Log.warningln("[%s] Fill Sub-State Timeout: Unhandled FillState: %s in PlungerState: %s. Aborting. -> IDLE", 
                        name.c_str(), _fillStateToString(_currentFillState), _plungerStateToString(_currentState));
            _fillSubStateTimer.detach(); 
            _currentFillState = FillState::NONE; 
            _transitionToState(PlungerState::IDLE); 
            break;
    }
}

// Ticker relay and handler for Fill joystick hold
void Plunger::_joystickFillHoldTimerRelay(Plunger* pThis) {
    if (pThis) {
        pThis->_onJoystickFillHoldTimeout();
    }
}

void Plunger::_onJoystickFillHoldTimeout() {
    if (static_cast<Joystick::E_POSITION>(_joystick->getValue()) == Joystick::E_POSITION::LEFT && _currentState == PlungerState::IDLE) {
        Log.infoln("[%s] FILLING sequence initiated from joystick.", name.c_str());
        // _fillOperationStartTimeMs is set in _transitionToState(FILLING)
        _currentFillState = FillState::PLUNGING;
        _vfdStartForward(static_cast<uint16_t>(_settings.speedFillPlungeHz * 100.0f));
        _operationStartTimeMs = millis();
        _currentMaxOperationTimeMs = _settings.defaultMaxOperationDurationMs;
        _joystickReleasedSinceAutoStart = false; 
        _transitionToState(PlungerState::FILLING);
    } 
    _joystickHoldStartTimeMs = 0; 
}

void Plunger::_handlePostFlowState()
{
    Joystick::E_POSITION joyDir = static_cast<Joystick::E_POSITION>(_joystick->getValue());

    // Check for joystick interruption - applies to all post-flow sub-states
    if (_joystickReleasedSinceAutoStart && joyDir != Joystick::E_POSITION::CENTER) { 
        Log.infoln("[%s] POST_FLOW sequence: Interrupted by joystick. -> STOPPING", name.c_str());
        _currentPostFlowState = PostFlowState::NONE; // Ensure cleanup
        _postFlowSubStateTimer.detach();
        _transitionToState(PlungerState::STOPPING); 
        return;
    }
    // Further joystick interaction logic might be needed depending on if held-down joy needs to abort etc.

    // Sub-state specific logic is primarily handled by the timer timeout (_onPostFlowSubStateTimeout)
    // This loop handler can be used for continuous checks if any sub-state needs them (e.g., monitoring current outside of _checkVfdForJam)
    // For now, most logic is event-driven by the timer.
}

void Plunger::_onPostFlowSubStateTimeout()
{
    if (_currentState != PlungerState::POST_FLOW) { 
        Log.warningln("[%s] Post-Flow Sub-State Timeout: Not in POST_FLOW state (%s). Aborting timer logic.", name.c_str(), _plungerStateToString(_currentState));
        _postFlowSubStateTimer.detach();
        _currentPostFlowState = PostFlowState::NONE; 
        return; 
    }

    Log.verboseln("[%s] Post-Flow sub-state timeout. Current sub-state: %d", name.c_str(), static_cast<int>(_currentPostFlowState));

    switch (_currentPostFlowState)
    {
        case PostFlowState::POST_FLOW_STOPPING:
            Log.infoln("[%s] Post-Flow: STOPPING wait complete. -> STARTING post-flow press.", name.c_str());
            _currentPostFlowState = PostFlowState::POST_FLOW_STARTING;
            _vfdStartForward(static_cast<uint16_t>(_settings.postFlowSpeedHz * 100.0f));
            _postFlowStartTimeMs = millis();
            _operationStartTimeMs = _postFlowStartTimeMs;
            _currentMaxOperationTimeMs = _settings.postFlowDurationMs;
            _postFlowSubStateTimer.once_ms(_settings.postFlowDurationMs, &Plunger::_postFlowSubStateTimerRelay, this);
            break;

        case PostFlowState::POST_FLOW_STARTING:
            Log.infoln("[%s] Post-Flow: STARTING (pressing) complete. -> COMPLETE wait.", name.c_str());
            _vfdStop();
            _currentPostFlowState = PostFlowState::POST_FLOW_COMPLETE;
            _postFlowSubStateTimer.once_ms(_settings.postFlowCompleteWaitMs, &Plunger::_postFlowSubStateTimerRelay, this);
            break;

        case PostFlowState::POST_FLOW_COMPLETE:
            Log.infoln("[%s] Post-Flow: COMPLETE wait finished. Post-flow sequence fully complete. -> IDLE", name.c_str());
            _currentPostFlowState = PostFlowState::NONE;
            _transitionToState(PlungerState::IDLE); 
            break;

        case PostFlowState::NONE:
        default:
            Log.warningln("[%s] Post-Flow Sub-State Timeout: Unhandled/NONE PostFlowState: %d. -> IDLE", 
                        name.c_str(), static_cast<int>(_currentPostFlowState));
            _postFlowSubStateTimer.detach(); 
            _currentPostFlowState = PostFlowState::NONE; 
            _transitionToState(PlungerState::IDLE); 
            break;
    }
}

// Static relay for post-flow sub-state timer
void Plunger::_postFlowSubStateTimerRelay(Plunger* pThis) {
    if (pThis) {
        pThis->_onPostFlowSubStateTimeout();
    }
}


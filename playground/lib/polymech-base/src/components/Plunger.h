#ifndef PLUNGER_H
#define PLUNGER_H

#include "config.h" 
#include <Component.h>
#include <ArduinoLog.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "components/SAKO_VFD.h"
#include "components/Joystick.h"
#include "components/POT.h"
#include "enums.h" 
#include <modbus/ModbusTCP.h> 
#include "PlungerSettings.h"

// Forward declaration for the debug flag
extern const bool debug_states;

// Component Constants
#define PLUNGER_COMPONENT_ID 760
#define PLUNGER_COMPONENT_NAME "Plunger"

// Speed Presets (in 0.01 Hz units for SAKO_VFD)
#define PLUNGER_SPEED_SLOW_HZ 20  // 10.00 Hz
#define PLUNGER_SPEED_MEDIUM_HZ 25 // 25.00 Hz
#define PLUNGER_SPEED_FAST_HZ 50 
#define PLUNGER_SPEED_MAX_HZ 75  

// Post-flow settings
#define PLUNGER_POST_FLOW_DURATION_MS 1000 // Duration of active post-flow (pressing)
#define PLUNGER_POST_FLOW_SPEED_HZ 5      // Speed during post-flow pressing
#define PLUNGER_CURRENT_POST_FLOW_MA 1200  // Current at or above this value indicates post-flow jam
#define PLUNGER_POST_FLOW_STOPPING_WAIT_MS 1500 // Wait time after initial stop before starting post-flow press
#define PLUNGER_POST_FLOW_COMPLETE_WAIT_MS 1500 // Wait time after post-flow press before returning to IDLE
#define PLUNGER_DEFAULT_ENABLE_POST_FLOW true // Added for clarity if PlungerSettings uses it

// New constants for Filling
#define PLUNGER_FILL_JOYSTICK_HOLD_DURATION_MS 2500 // Time to hold joystick LEFT to start fill
#define PLUNGER_SPEED_FILL_PLUNGE_HZ PLUNGER_SPEED_MEDIUM_HZ // Speed for plunging phase of filling
#define PLUNGER_SPEED_FILL_HOME_HZ PLUNGER_SPEED_SLOW_HZ     // Speed for homing phase of filling
#define PLUNGER_FILL_PLUNGED_WAIT_DURATION_MS 1000 // Wait duration after plunging
#define PLUNGER_FILL_HOMED_WAIT_DURATION_MS 1500   // Wait duration after homing

// Speed POT Configuration for Plunging (multiplier for MEDIUM speed)
// POT value 0-100. Example: 0 maps to 0.5x, 50 maps to 1.0x, 100 maps to 1.5x MEDIUM speed.

// Fixed Current Thresholds (mA)
#define PLUNGER_CURRENT_IDLE_LOWER_MA 300
#define PLUNGER_CURRENT_IDLE_UPPER_MA 500
#define PLUNGER_CURRENT_PLUNGING_LOWER_MA 500
#define PLUNGER_CURRENT_PLUNGING_UPPER_MA 700
#define PLUNGER_CURRENT_JAM_THRESHOLD_MA 700 // Current at or above this value indicates a jam


#define PLUNGER_JAMMED_DURATION_HOMING_MS 10
#define PLUNGER_JAMMED_DURATION_MS 1400 
#define PLUNGER_VFD_READ_INTERVAL_MS 200 

// #define ENABLE_AUTO 1 // Set to 0 to disable joystick-activated auto homing/plunging -- REMOVED

#define PLUNGER_AUTO_MODE_HOLD_DURATION_MS 2000 // Time joystick must be held for auto mode
#define PLUNGER_MAX_UNIVERSAL_JAM_TIME_MS 5000 // Max time current can be high before a universal jam

// New constants for Record/Replay
#define PLUNGER_RECORD_HOLD_DURATION_MS 2000 // Min time joystick must be held RIGHT to enter RECORD mode
#define PLUNGER_MAX_RECORD_DURATION_MS 20000 // Max duration for the plunging phase of recording
#define PLUNGER_DEFAULT_REPLAY_DURATION_MS 4500 // Default duration for replay if not recorded/set

// New constant for default max operation time
#define PLUNGER_DEFAULT_MAX_OPERATION_DURATION_MS 60*1000*1 // 1 minute

// Modbus Configuration
#define PLUNGER_MB_BASE_ADDRESS COMPONENT_KEY_PLUNGER // Using component ID as base
#define PLUNGER_MB_STATE_OFFSET 0
#define PLUNGER_MB_COMMAND_OFFSET 1
#define PLUNGER_MB_BLOCK_COUNT 2 // Will need to update if CMD_FILL Modbus register is separate, for now assume it uses existing command offset
#define PLUNGER_MAX_RUN_TIME_MEDIUM_SPEED_MS 15000 // Max runtime at medium speed

enum class E_PlungerCommand : short {
    NO_COMMAND = 0,
    CMD_HOME = 1,
    CMD_PLUNGE = 2,
    CMD_STOP = 3,
    CMD_INFO = 4,
    CMD_FILL = 5,
    CMD_REPLAY = 6
};

// Plunger States
enum class PlungerState : uint8_t {
    IDLE,
    HOMING_MANUAL,      // Joystick held DOWN, VFD reversing, monitoring for auto-mode hold time
    HOMING_AUTO,        // Auto-homing after joystick hold, VFD reversing
    PLUNGING_MANUAL,    // Joystick held UP, VFD forwarding, monitoring for auto-mode hold time
    PLUNGING_AUTO,      // Auto-plunging after joystick hold, VFD forwarding
    STOPPING,           // Transition state to stop VFD
    JAMMED,
    RESETTING_JAM,       // State to handle reset after jam
    RECORD,             // New state for recording plunge duration
    REPLAY,              // New state for replaying recorded plunge
    FILLING,
    POST_FLOW,
};

enum class FillState : uint8_t 
{
    NONE,
    PLUNGING,                       // Actively plunging down
    PLUNGED,                        // Plunge event (jam/max_duration) occurred, waiting for min plunge time
    HOMING,                         // Actively homing up
    HOMED                           // Homing complete
};

enum class PostFlowState : uint8_t 
{
    NONE,
    POST_FLOW_STOPPING,             // Stopping VFD, and wait at least 1500ms
    POST_FLOW_STARTING,            // Starting VFD, set post-flow speed, and run for post-flow duration
    POST_FLOW_COMPLETE             // Stop VFD, and wait before resuming normal operation (e.g. IDLE)
};

class Plunger : public Component {
public:
    Plunger(Component* owner, SAKO_VFD* vfd, Joystick* joystick, POT* speedPot, POT* torquePot);
    ~Plunger() override = default;

    short setup() override;
    short loop() override;
    short info() override;
    short debug() override;
    short serial_register(Bridge* b) override;
    short init();
    short reset();

    // Modbus TCP Interface
    ModbusBlockView* mb_tcp_blocks() const override;
    void mb_tcp_register(ModbusTCP* mgr) override;
    short mb_tcp_read(MB_Registers* reg) override;
    short mb_tcp_write(MB_Registers* reg, short networkValue) override;
    // Public commands for serial/external control
    short cmd_plunge();
    short cmd_home();
    short cmd_stop();
    short cmd_fill();
    short cmd_save_settings(); 
    short cmd_load_default_settings();
    short cmd_replay();
    // Auto mode control
    void setAutoModeEnabled(bool enabled);
    bool isAutoModeEnabled() const;
    short cmd_enableAutoMode();
    short cmd_disableAutoMode();

    // Persistence related methods (now for REST exposure)
    void getSettingsJson(JsonDocument& doc) const;
    bool updateSettingsFromJson(const JsonObject& json);
    bool loadDefaultSettings(const char* defaultPath = "/plunger_default.json", const char* operationalPath = "/plunger.json");

private:
    SAKO_VFD* _vfd;
    Joystick* _joystick;
    POT* _speedPot;
    POT* _torquePot;
    PlungerState _currentState;
    FillState _currentFillState; // Re-added
    PostFlowState _currentPostFlowState; // For managing sub-states of POST_FLOW
    Joystick::E_POSITION _lastJoystickDirection;
    uint16_t _currentSpeedPotValue;       // 0-100
    uint16_t _currentTorquePotValue;      // 0-100, for torque/jam sensitivity
    float _calculatedPlungingSpeedHz;     // Calculated speed for plunging (0.01Hz units)

    unsigned long _lastStateChangeTimeMs;
    unsigned long _jammedStartTimeMs;
    unsigned long _lastVfdReadTimeMs;
    unsigned long _joystickHoldStartTimeMs; // Timer for joystick hold duration
    short _modbusCommandRegisterValue;      // Holds the current value of the Modbus command register
    unsigned long _operationStartTimeMs;    // Start time of current plunge/home operation (used for fill plunge phase)
    unsigned long _currentMaxOperationTimeMs; // Calculated max duration for current operation (used for fill plunge phase)
    unsigned long _lastDiagnosticLogTimeMs; // Timer for the 5-second diagnostic log in _checkVfdForJam
    unsigned long _lastImmediateStopCheckTimeMs; // Timer for the 1-second immediate high-current stop check
    bool _joystickReleasedSinceAutoStart; // True if joystick released to center after auto mode started
    bool _autoModeEnabled;                // True if joystick/command auto modes are enabled

    // Record and Replay members
    Ticker _joystickRecordHoldTimer;
    Ticker _replayPlungeTimer;
    Ticker _joystickFillHoldTimer;      // Timer for joystick LEFT hold to start filling
    unsigned long _recordedPlungeDurationMs;
    unsigned long _recordModeStartTimeMs; // To time the actual plunge during recording

    // Filling members
    Ticker _fillSubStateTimer;            // Timer for fill sub-state durations
    unsigned long _fillOperationStartTimeMs; // Start time of the overall fill operation
    unsigned long _postFlowStartTimeMs;      // Start time of the active post-flow pressing phase

    // Post-flow sub-state timer
    Ticker _postFlowSubStateTimer;

    // Settings
    PlungerSettings _settings;

    // Timer for state logging
    unsigned long _lastStateLogTimeMs;

    // Helper methods
    void _handleIdleState();
    void _handleHomingManualState();
    void _handleHomingAutoState();
    void _handlePlungingManualState();
    void _handlePlungingAutoState();
    void _handleStoppingState();
    void _handleJammedState();
    void _handleResettingJamState();
    void _handleRecordState();
    void _handleReplayState();
    void _handleFillingState();
    void _handlePostFlowState(); // Declaration for the new state handler
    // Static relay callbacks for Ticker
    static void _joystickRecordHoldTimerRelay(Plunger* pThis);
    static void _replayPlungeTimerRelay(Plunger* pThis);
    static void _fillSubStateTimerRelay(Plunger* pThis); // Added for fill timer
    static void _joystickFillHoldTimerRelay(Plunger* pThis); // Relay for fill joystick hold
    static void _postFlowSubStateTimerRelay(Plunger* pThis); // Relay for post-flow sub-state timer

    // Actual timer event handlers
    void _onJoystickRecordHoldTimeout();
    void _onReplayPlungeTimeout();
    void _onFillSubStateTimeout(); // Added for fill timer
    void _onJoystickFillHoldTimeout(); // Handler for fill joystick hold
    void _onPostFlowSubStateTimeout(); // Handler for post-flow sub-state timer

    void _updatePotValues();
    void _checkVfdForJam();
    void _transitionToState(PlungerState newState);

    // VFD interaction wrappers
    void _vfdStartForward(uint16_t frequencyCentiHz);
    void _vfdStartReverse(uint16_t frequencyCentiHz);
    void _vfdStop();
    void _vfdResetJam();
};

const char *_plungerStateToString(PlungerState state);
const char *_fillStateToString(FillState state); // Ensure this declaration is present
const char *_postFlowStateToString(PostFlowState state); // Declaration for PostFlowState to string

#endif // PLUNGER_H 
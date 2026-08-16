# Manual Mode Refactor Proposal

**Status:** Implemented
**Feature:** Enhanced Manual Mode Safety & Interlock Support

## Problem Statement

Currently, `PressCylinder::onJoystickUp` implements a simplified manual control loop that bypasses safety checks (`CFlags`) and complex logic available in `loopSingle` and `loopMulti`. It directly enables solenoids based on Joystick Position, ignoring:

* Load balancing (in multi-cylinder setups)
* Stall detection
* Interlock status (implicitly, though safety overrides exist)
* Operation timeouts

The goal is to unify the logic so that "Manual" operation is simply a special case of the robust `loop*` functions, but controlled by the Joystick/PushButton rather than a fixed "Auto" process.

## Proposed Strategy

We will treat Manual operation as "Auto operation towards a Virtual Maximum Target" that only persists while the operator holds the input (Joystick or PushButton).

### 1. Unified Control Logic

Instead of direct `SOLENOID_ON/OFF` in `onJoystickUp`, we will:

1. Detect "Active Input" (Joystick UP *OR* PushButton Pressed).
2. If Active:
    * Temporarily override `m_targetSP` to `100%` (or a `manual_override_sp`).
    * Set effective mode to `DEFAULT_AUTO_MODE_NORMAL` (if single/unlocked) or `DEFAULT_AUTO_MODE_INTERLOCKED` (if interlocked).
    * Call `loopSingle()` or `loopMulti()` respectively.
3. If Inactive:
    * Reset `m_targetSP` to 0 (or restore original).
    * Enforce `SOLENOIDS_OFF`.

### 2. Implementation Plan

#### A. Define Helper: `getVirtualTargetSP()`

For manual moves, we want to press as long as the operator commands, up to the absolute hardware limit (`maxload_threshold` or `100%`).

```cpp
uint32_t PressCylinder::getVirtualTargetSP() {
    // In Manual mode, "Target" is effectively "Go until Max or Stop"
    return 70; // 70% of maxload_threshold (Virtual SP Max)
}
```

#### B. Refactor `onJoystickUp` / Main Loop Integration

We will stop using `onJoystickUp` as a direct actuator. Instead, it becomes an input state detector.

**Modified `loop()` Logic:**

```cpp
// 1. Determine "Effective Mode" and "Effective Target"
E_Mode effective_mode = (E_Mode)m_mode.getValue();
uint32_t effective_sp = m_targetSP.getValue();
bool manual_input_active = false;

// Check Inputs (Joystick or PushButton)
    // Check Inputs (Joystick or PushButton)
    if (_pushButton->getState() == PushButton::State::PRESSED || _pushButton->getState() == PushButton::State::HELD) {
        // Condition 1: PushButton Held -> Force Manual Single Cylinder
        manual_input_active = true;
        effective_mode = MODE_MANUAL;
        effective_sp = 70; 
    }
    else if (_joystick->getPosition() == Joystick::E_POSITION::UP) {
        // Condition 2: Joystick UP -> Smart Auto / Interlock Dependent
        manual_input_active = true;
        effective_sp = 70;
        
        if (effective_mode == MODE_MANUAL || effective_mode == MODE_MANUAL_MULTI) {
             if (m_interlocked.getValue()) {
                  effective_mode = DEFAULT_AUTO_MODE_INTERLOCKED; // e.g. AUTO_MULTI_BALANCED
             } else {
                  effective_mode = DEFAULT_AUTO_MODE_NORMAL; // e.g. AUTO
             }
        }
    }

// 2. Dispatch to Control Loops
if (manual_input_active || isAutoRunning()) {
    // These functions now use 'effective_mode' and 'effective_sp' 
    // instead of reading m_targetSP/m_mode directly if we pass them as args,
    // OR we temporarily override member vars (simpler but riskier state).
    
    // BETTER APPROACH: Refactor loopSingle/loopMulti to take args
    if (isMultiMode(effective_mode)) {
        loopMulti(effective_mode, effective_sp);
    } else {
        loopSingle(effective_mode, effective_sp);
    }
} else {
    // Idle / Stop
    SOLENOIDS_OFF();
}
```

### 3. Modifications Required

1. **Refactor `loopSingle` / `loopMulti`**:
    * Change signature to accept `E_Mode mode` and `uint16_t target_sp` arguments.
    * Remove internal calls to `m_mode.getValue()` and `m_targetSP.getValue()` in favor of these arguments.

2. **Update `onJoystickUp`**:
    * Actually, we might deprecate `onJoystickUp` direct logic entirely and move it into the main `loop()` structure as shown above.
    * Retain `onJoystickDoubleUp` for the HOLD feature (capturing Load to SP).

3. **PushButton Integration**:
    * Map `_pushButton->getState() == PushButton::State::PRESSED` (or HELD) to `manual_input_active`.

### 4. Safety Considerations

* **Deadman Switch**: The moment the Joystick/Button is released, `manual_input_active` becomes false, and the `else` block triggers `SOLENOIDS_OFF()`.
* **Existing Safety**: `loopSingle/Multi` already contain `CheckMinLoad`, `Stall`, `Balance`, and `MaxTime` checks. By routing manual control through them, we gain all these features automatically.
* **Virtual SP**: Setting SP to 100% is safe because `loop*` logic checks `canPress()` which respects `maxload_threshold`.

## Diagram: New Flow

```mermaid
graph TD
    Input[Inputs: Joystick / Button] --> Check{Active?}
    Check -- Yes --> MapMode[Map to Effective Auto Mode]
    MapMode --> SetSP[Set Virtual SP = 70%]
    SetSP --> Dispatch{Is Multi?}
    
    Dispatch -- Single --> LoopSingle[loopSingle(SafeMode, 70%)]
    Dispatch -- Multi --> LoopMulti[loopMulti(SafeMode, 70%)]
    
    LoopSingle --> Actuators
    LoopMulti --> Actuators
    
    Check -- No --> Stop[SOLENOIDS_OFF]
```

### References

### Data

cs-1150 - interlocked

min load : 50-150

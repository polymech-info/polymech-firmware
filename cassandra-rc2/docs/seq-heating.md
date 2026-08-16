# AmperageBudgetManager Sequencing Analysis

## Problem Statements

1. **Premature Abortion**: Devices stop heating earlier than expected.
2. **Reduced Concurrency**: "Sometimes heating only 1 instead of set max".
3. **Erratic Behavior**: General instability in sequencing modes.

## Analysis of `E_AM_CYCLE_ALL`

### 1. Transient Error Sensitivity

The `_checkHeatup` function returns `false` immediately if `device->hasError()` is true. This can be triggered by a single failed Modbus transaction (e.g., packet loss).

- **Consequence**: If a running device encounters a transient error, it is immediately stopped (`device->stop()`), and the sequencer advances to the next index (`advanceCurrentIndex()`).
- **Impact**: The device loses its slot in the current window. It must wait for the window to cycle all the way around before it can heat again.

### 2. "Heatup" vs "Oscillating" Mode Instability

The logic distinguishes between "Heatup" (aggressive, long duration) and "Oscillating" (maintenance, short duration) using `_deviceInHeatup`.

```cpp
bool isPastHeatup = !_deviceInHeatup[deviceIndex];
uint32_t maxDuration = isPastHeatup ? m_maxHeatingDurationOscillatingS.getValue() : m_maxHeatingDurationS.getValue();
```

- **Scenario**: A device is heating in "Heatup" mode (e.g., running for 100s, limit 300s).
- **Trigger**: A transient condition causes `_checkHeatup` or `isHeatup()` to return `false` momentarily (e.g., PV spikes near SP, or momentary error).
- **Effect**: `isPastHeatup` becomes `true`. The `maxDuration` switches to `Oscillating` limit (default 15s).
- **Consequence**: Since 100s > 15s, the check `now_s - start > maxDuration` immediately passes. The device is stopped, and the index advances.
- **Result**: "Premature abortion" of the heatup phase.

### 3. Concurrency Reduction (Sliding Window Race)

When a device is stopped (due to error or timeout), `advanceCurrentIndex()` is called immediately.

- If we have devices [A, B, C] and MaxSim=2. Window=[A, B].
- A aborts (transient error). Index moves to B. Window=[B, C].
- A is now outside the window.
- If A recovers immediately, it cannot restart because it is outside the window.
- It leaves B (and potentially C) heating.
- If this happens frequently, devices effectively take turns crashing out of the window, reducing effective concurrency.

## Proposed Fixes

### 1. Add Debounce / Grace Period for Errors and Completion

Do not stop a device immediately on a single cycle of `!_checkHeatup`. Requires a persistence counter or time-based hysteresis.

**Implementation Plan**:

- Introduce `_deviceStateInSyncCount[MAX_MANAGED_DEVICES]` or similar.
- Require `N` consecutive cycles of "Not Heatup" or "Error" before taking stop action.
- Alternatively, check `lastUpdate` timestamp of the device to ignore stale errors.

### 2. Sticky "Heatup" State

Once a device is considered "In Heatup", it should probably remain in that logical state for the duration of its heating slot, or require significant evidence to drop to "Oscillating".
However, `_deviceInHeatup` is refreshed every `loop()`.

**Better Approach**: Use the `maxDuration` based on the state *at the start of the heating cycle* or maintain a latched state that is only cleared after a stop.

### 3. Decouple "Stop" from "Advance" on Error

If a device errors out, we might want to Stop it for safety, but *NOT* advance the index immediately, giving it a chance to recover within its slot?

- Risk: If it's dead, we block the slot.
- Compromise: Advance only after `N` seconds of error.

## Recommended Immediate Changes (Low Risk)

1. **Modify `_checkHeatup`** to ignore `hasError()` if the error is transient/recent (or simply log and return true if data is stale but not ancient). *Actually, safer to just rely on debounce.*
2. **Debounce State Changes**: In `updateDevice`, do not act on `!_checkHeatup` unless it persists for X cycles (e.g., 3 cycles = 1.5 seconds).

### 4. Fix Latching Error Logic (High Priority)

The use of `device->hasError()` (which checks `errorCount > 0`) is incorrect for real-time control.

- **Fix**: Change `_checkHeatup` to ignore `device->hasError()` (cumulative). Instead, check `device->getLastErrorCode()`.
- **Refinement**: Only treat **Timeout (224/0xE0)** as a critical failure. Ignore CRC/Collision errors.
- **Proposed Logic**: `bool isDead = device->getLastErrorCode() == (uint16_t)MB_Error::Timeout;`
- This ensures we only Abort/Stop when the device is truly unreachable, not just experiencing noise.

```cpp
// Pseudo-code
if (_deviceHeating[i]) {
    bool currentNeed = _checkHeatup(device);
    if (!currentNeed) {
        _stopConfirmationCount[i]++;
        if (_stopConfirmationCount[i] < DEBOUNCE_THRESHOLD) return false; // Ignore for now
    } else {
        _stopConfirmationCount[i] = 0;
    }
    // ... proceed to time checks ...
}
```

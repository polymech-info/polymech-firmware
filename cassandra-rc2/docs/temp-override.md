# Profile Duration Override Draft

## Objective

Implement a simple, non-persistent way to extend the profile duration from the current time using +/- 10 minute steps via the UI. Ensure linked profiles (Pressure, Signal) stay in sync.

## Sync Resolution

The primary challenge identified is syncing the "Override" (time shift) across linked profiles (`PressureProfile`, `SignalPlot`).
The `PlotBase` class already supports hierarchical time manipulation via `seek()` and `slipTime()`, which recursively call the same method on all `children`.
However, `TemperatureProfile` currently stores linked profiles as "Slot IDs" (`_pressureProfileSlotId`, `_signalPlotSlotId`) but does **not** populate the `PlotBase::children` array. Thus, `resolvePlots()` fails to link them, and time shifts do not propagate.

**Solution:**
Modify `TemperatureProfile::onStart()` (or a new setup phase) to resolve the `ComponentID` from the stored `SlotID` for both Pressure and Signal profiles, and register them using `PlotBase::addPlot()`.

## Implementation Details

### 1. Firmware Changes

#### `TemperatureProfile.h`

* Add a new NetworkValue `m_timeOverride` (or similar name) to track the cumulative override applied, or simply use a command mechanism.
* Given the requirement for "+/- steps", a stateless command register or a "Total Override" register can be used. A "Total Override" register (int16 minutes) is better for UI feedback.
* Function `applyTimeOverride(int16_t minutes)`:
  * Calculates the delta in ms.
  * Calls `PlotBase::slipTime(delta)`.
  * Updates `m_timeOverride` value for UI display.

#### `TemperatureProfile.cpp`

* **Linking:** in `onStart()`:
  * Lookup `PressureProfile` component by `_pressureProfileSlotId`.
  * Lookup `SignalPlot` component by `_signalPlotSlotId`.
  * Call `addPlot(ptr)` for each.
  * This ensures `slipTime()` on TemperatureProfile automatically slips the others.
* **Logic:**
  * Limit `slipTime` so we don't seek before 0.
  * Note: `slipTime` shifts `_startTimeMs`.
    * `slipTime(positive)` -> Moves start time forward -> Elapsed time decreases -> **Rewind / Extend Duration**.
    * `slipTime(negative)` -> Moves start time backward -> Elapsed time increases -> **Skip Forward**.

### 2. Modbus/API

* Reuse existing `COMMAND` register? Or add a new register offset?
* Adding a `TIME_OVERRIDE` register (Read/Write) is clearest.
* Address: `Base + Offset`. `PlotBase` has reserved offsets. We can add to `TemperatureProfile` specific offsets.
* **Register Definition:**
  * Name: `TimeOverride`
  * Type: `Int16` (Minutes)
  * Read: Current total override applied.
  * Write: Set new total override (calculates delta and slips).

### 3. UI Changes (`ProfilePlayback.tsx`)

* **New Controls:**
  * Add `+10 min` and `-10 min` buttons near the Timer/Progress area.
  * Display "Override: +X min" if non-zero?
* **Interaction:**
  * On Click (+):
    * Read current `TimeOverride` (or maintain local state + optimistic).
    * Write `NewValue = Current + 10`.
  * Alternatively, directly write to `ELAPSED` register?
    * Pro: No firmware header changes needed for registers.
    * Con: UI must calculate `NewElapsed = CurrentElapsed - 10min` and write it.
    * **Winner:** Direct `ELAPSED` write is supported by `PlotBase` and requires minimal firmware changes (only the Linking fix). Use this if "Override Variable" in firmware is not strictly required for other reasons.
    * *However*, user asked for "override variable in TemperatureProfile.h". This implies they want to track it or the direct `ELAPSED` write isn't "sticky" enough or they want to see the *offset*.
    * **Hybrid:** Implement `TimeOverride` register. It's safer and cleaner.

### 4. Proposed File Modifications

#### `[MODIFY] .../profiles/TemperatureProfile.h`

```cpp
// Add member
NetworkValue<int16_t> m_timeOverride;
void resolveLinkedProfiles();
```

#### `[MODIFY] .../profiles/TemperatureProfile.cpp`

```cpp
// In constructor/setup
m_timeOverride.initModbus(...);

// In onStart()
resolveLinkedProfiles();

// Implementation of resolveLinkedProfiles
void TemperatureProfile::resolveLinkedProfiles() {
    // Logic to find component by slot and call addPlot()
}

// Handle writes to m_timeOverride
// If value changes, calculate delta and call slipTime()
```

#### `[MODIFY] .../components/ProfilePlayback.tsx`

```tsx
// Add +/- 10m buttons
// Handle Modbus write to TimeOverride register
```

## Summary

1. **Sync:** Fix `TemperatureProfile` to actually parent the linked profiles via `addPlot`.
2. **Mechanic:** Use `slipTime` (Time shift) to implement the extension.
3. **Control:** Add `TimeOverride` register for +/- 10m control.

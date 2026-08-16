# Pressure Profile Integration

## Overview

Pressure profiles (`PressureProfile`) work as **child plots** of temperature profiles (`TemperatureProfile`) in the polymech framework. When a temperature profile is configured with a pressure profile slot ID, the pressure profile becomes a subordinate that follows the lifecycle of its parent temperature profile.

## Parent-Child Relationship

### Linking Mechanism

Temperature profiles can be linked to pressure profiles through the `pressureProfile` field in the temperature profile JSON configuration:

```json
{
  "id": 1,
  "name": "Temperature Profile with Pressure",
  "pressureProfile": 0,
  // ... other temperature profile fields
}
```

The linking is established in `PHApp::load()` and `PHApp::updateProfile()`:

```cpp
// During profile loading
if (pressureProfileSlotId >= 0 && pressureProfileSlotId < PROFILE_PRESSURE_COUNT) {
    if (pressureProfiles[pressureProfileSlotId]) {
        tempProfiles[i]->addPlot(pressureProfiles[pressureProfileSlotId]);
    }
}
```

### Duration Inheritance

When a pressure profile is added as a child plot via `PlotBase::addPlot()`:
- The child pressure profile inherits the parent's duration: `plot->setDuration(_durationMs)`
- The parent-child relationship is established: `plot->setParent(this)`

## Lifecycle Management

### State Synchronization

The pressure profile's lifecycle is controlled by its parent temperature profile through the `PlotBase` event system:

#### 1. Starting (onStart)

When the temperature profile starts:
- **Normal Start**: If the temperature profile transitions directly to `RUNNING`, child pressure profiles start immediately
- **Warmup Start**: If the temperature profile enters `INITIALIZING` state (warmup mode), child pressure profiles are **deferred** until warmup completes

```cpp
// In PlotBase::onStart()
if (getCurrentStatus() == PlotStatus::INITIALIZING) {
    L_INFO("Profile ID %d is INITIALIZING, deferring child plot start.", id);
    return; // Child plots NOT started yet
}

// Start all child plots when ready
for (int i = 0; i < MAX_PLOTS; ++i) {
    if (_plots[i] != nullptr) {
        _plots[i]->start();
    }
}
```

#### 2. Stopping (onStop)

When the temperature profile stops, all child pressure profiles are stopped:

```cpp
for (int i = 0; i < MAX_PLOTS; ++i) {
    if (_plots[i] != nullptr) {
        _plots[i]->stop();
    }
}
```

#### 3. Pausing (onPause)

When the temperature profile pauses, all child pressure profiles are paused:

```cpp
for (int i = 0; i < MAX_PLOTS; ++i) {
    if (_plots[i] != nullptr) {
       _plots[i]->pause();
    }
}
```

#### 4. Resuming (onResume)

When the temperature profile resumes, all child pressure profiles are resumed:

```cpp
for (int i = 0; i < MAX_PLOTS; ++i) {
    if (_plots[i] != nullptr) {
        _plots[i]->resume();
    }
}
```

#### 5. Finishing (onFinished)

When the temperature profile finishes, the event is propagated but child plots are not explicitly stopped (they should finish naturally based on duration).

## Value Normalization

### Max Value Enforcement

Pressure profiles automatically enforce `max = 100` regardless of JSON configuration to ensure proper percentage scaling for PressCylinder SP values:

```cpp
// In PressureProfile constructor and load()
max = 100; // Always enforced for percentage scaling
```

This ensures that:
- Control point `y: 1000` (100% of PROFILE_SCALE) → outputs 100 (100% pressure)
- Control point `y: 500` (50% of PROFILE_SCALE) → outputs 50 (50% pressure)
- Values are properly interpreted by PressCylinder as percentages (0-100%)

### Temperature vs Pressure Profile Max Values

| Profile Type | Max Value | Purpose |
|-------------|-----------|---------|
| Temperature | User-defined (e.g., 150°C) | Actual temperature units |
| Pressure | Always 100 | Percentage for PressCylinder SP |

## Target Register Integration

### PressCylinder SP Control

Pressure profiles can control PressCylinder components by targeting their setpoint (SP) registers through the `targetRegisters` array:

```json
{
  "id": 0,
  "name": "Press Cylinder Pressure Control",
  "targetRegisters": [1002], // PressCylinder SP register address
  "controlPoints": [
    {"x": 0, "y": 0},      // Start at 0% pressure
    {"x": 300, "y": 500},  // Ramp to 50% by 30% time
    {"x": 700, "y": 500},  // Hold 50% until 70% time
    {"x": 1000, "y": 0}    // Return to 0% by end
  ]
  // ... other fields
}
```

### Modbus Register Mapping

The pressure profile writes to target registers via Modbus TCP:

```cpp
// In PressureProfile::loop() -> applyPressure()
for (uint16_t targetRegAddr : _targetRegisters) {
    if (targetRegAddr == 0) continue;
    
    ModbusMessage req;
    req.setMessage(1, FN_WRITE_HOLD_REGISTER, targetRegAddr, pressureValue);
    resp = modbusTCP->modbusServer->localRequest(req);
}
```

For PressCylinder, the SP register is located at:
- **Base Address**: Component's Modbus base address
- **SP Offset**: `MB_OFS_HR_TARGET_SP = E_NVC_USER + PRESS_CYLINDER_MAX_PAIRS`
- **Full Address**: `baseAddress + MB_OFS_HR_TARGET_SP`

## State Machine Integration

### Temperature Profile States

The pressure profile responds to these temperature profile states:

| Temperature State | Pressure Profile Action |
|------------------|-------------------------|
| `IDLE` | Not started/remains idle, SP reset to 0 |
| `INITIALIZING` | Waits for parent to complete warmup, SP reset to 0 |
| `RUNNING` | Actively executes pressure curve |
| `PAUSED` | Pauses execution, **maintains current SP** |
| `STOPPED` | Stops and resets SP to 0 |
| `FINISHED` | Completes execution, **maintains final SP** |

### PressCylinder Integration

When the pressure profile targets a PressCylinder:

1. **Pressure Values**: The profile calculates pressure percentages (0-100%) based on control points
2. **SP Conversion**: Values are written directly to the PressCylinder's `m_targetSP` register
3. **Mode Compatibility**: Works with PressCylinder auto modes (`MODE_AUTO`, `MODE_AUTO_MULTI`, etc.)
4. **Safety Integration**: PressCylinder safety systems (overload, balance, timeouts) remain active

### Loop Timing

- **Pressure Profile Loop**: Executes every `PRESSURE_PROFILE_LOOP_INTERVAL_MS` (150ms)
- **PressCylinder Loop**: Executes every `PRESSCYLINDER_INTERVAL` (20ms)
- **Coordination**: PressCylinder reads SP changes and responds within its next loop cycle

## Configuration Example

Complete example showing temperature profile with linked pressure profile:

### Temperature Profile (`profile_defaults.json`)
```json
[
  {
    "id": 1,
    "name": "Heating with Pressure Control",
    "pressureProfile": 0,
    "targetRegisters": [2001, 2002],
    "controlPoints": [
      {"x": 0, "y": 200},    // Start at 20°C
      {"x": 500, "y": 800},  // Heat to 80°C
      {"x": 1000, "y": 800}  // Hold at 80°C
    ],
    "duration": 300000,      // 5 minutes
    "enabled": true
  }
]
```

### Pressure Profile (`pressure_profiles.json`)
```json
[
  {
    "id": 0,
    "name": "Synchronized Pressure Ramp",
    "targetRegisters": [1002],
    "controlPoints": [
      {"x": 0, "y": 0},      // Start at 0% pressure
      {"x": 200, "y": 0},    // Hold 0% during initial heating
      {"x": 400, "y": 600},  // Ramp to 60% pressure
      {"x": 800, "y": 600},  // Hold 60% pressure
      {"x": 1000, "y": 200}  // Reduce to 20% at end
    ],
    "enabled": true
  }
]
```

## Event Callbacks

The system provides event callbacks for pressure profile lifecycle events:

```cpp
// In PHApp (or other IPlotEvents implementer)
void onPressureProfileStarted(PlotBase *profile);
void onPressureProfileStopped(PlotBase *profile);
void onPressureProfilePaused(PlotBase *profile);
void onPressureProfileResumed(PlotBase *profile);
void onPressureProfileFinished(PlotBase *profile);
```

These callbacks can be used to implement additional logic like:
- Safety interlocks
- Data logging
- UI status updates
- Integration with other systems

## Best Practices

1. **Duration Matching**: Ensure pressure profile control points span the full time range (0-1000) to match the parent temperature profile duration

2. **Safety First**: Always consider PressCylinder safety limits when designing pressure curves

3. **Smooth Transitions**: Use gradual pressure changes to avoid system shock and ensure stable operation

4. **Testing**: Test pressure profiles independently before linking to temperature profiles

5. **Monitoring**: Monitor both temperature and pressure values during operation to ensure proper coordination

## Pressure Holding Behavior

### State-Specific Pressure Management

The pressure profile uses intelligent pressure management based on its current state:

#### Reset Pressure to 0:
- **IDLE**: Profile not started or reset
- **INITIALIZING**: Waiting for parent warmup
- **STOPPED**: Explicitly stopped by user/system

#### Hold Current Pressure:
- **PAUSED**: Maintains current setpoint for process stability
- **FINISHED**: Maintains final setpoint for controlled completion

```cpp
// Implementation logic
switch (currentStatus) {
    case PlotStatus::PAUSED:
    case PlotStatus::FINISHED:
        holdCurrentPressure(); // Keep current SP
        break;
    case PlotStatus::IDLE:
    case PlotStatus::STOPPED:
        resetOutputs(); // Set SP to 0
        break;
}
```

This behavior ensures:
- **Process Stability**: No sudden pressure drops during pause/finish
- **Safety**: Controlled shutdown only when explicitly stopped
- **Operator Control**: Predictable behavior during manual interventions

## Troubleshooting

### Common Issues

1. **Pressure Profile Not Starting**
   - Check that the temperature profile has the correct `pressureProfile` slot ID
   - Verify the pressure profile slot is not null
   - Ensure the temperature profile successfully exits `INITIALIZING` state

2. **SP Not Updating**
   - Verify `targetRegisters` contains the correct PressCylinder SP address
   - Check Modbus TCP connectivity
   - Ensure PressCylinder is in an auto mode (`MODE_AUTO`, etc.)

3. **Timing Issues**
   - Check loop intervals and ensure adequate system performance
   - Monitor for Modbus communication delays
   - Verify control point timing alignment

### Debug Information

Enable logging to see pressure profile lifecycle events:
```cpp
L_INFO("PressureProfile: Status changed from %d to %d", oldStatus, newStatus);
L_INFO("PressureProfile: Applying pressure %d to register %d", pressureValue, targetRegAddr);
```

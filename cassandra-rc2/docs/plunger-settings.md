# Plunger Settings

This document explains the settings you can change to control how the Plunger works. Think of it like a control panel for the Plunger. These settings are usually saved in a special file on the machine.

## 1. Speed Settings

This group of settings controls how fast or slow the Plunger moves. The speed is measured in Hertz (Hz), which is a way to describe motor speed. Sometimes, the number you see in the settings might be the actual Hz value multiplied by 100 (for example, a setting of 5000 means 50.00 Hz).

*   **1.1. Slow Speed** (`speedSlowHz`): This is the speed the Plunger uses when you're moving it up or down carefully, like when it's "homing" (returning to its starting position) slowly. Unit: Hz.
*   **1.2. Medium Speed** (`speedMediumHz`): A general-purpose medium speed. This might be used for standard operations if a more specific speed isn't set. Unit: Hz.
*   **1.3. Fast Speed** (`speedFastHz`): A general-purpose fast speed. This could be used when the Plunger needs to move quickly and a specific speed for that action isn't defined. Unit: Hz.
*   **1.4. Fill Plunge Speed** (`speedFillPlungeHz`): When the Plunger is in the "Filling Sequence" and moving downwards (plunging) to fill something, this is the speed it will use. Unit: Hz.
*   **1.5. Fill Home Speed** (`speedFillHomeHz`): During the "Filling Sequence," after the Plunger has moved down, it needs to return to its starting position (homing). This setting controls the speed for that upward movement. Unit: Hz.

## 2. Jam Detection & Timing Settings

These settings are all about how the Plunger detects if it's stuck (jammed) and how long it waits or operates in different situations.

*   **2.1. Jam Threshold Current** (`currentJamThresholdMa`): The Plunger knows it's jammed if the motor starts using too much electricity (current). This setting is the "too much" point. If the motor current goes above this value, the Plunger assumes it's stuck. Measured in milliamperes (mA).
*   **2.2. Jammed Duration - Homing** (`jammedDurationHomingMs`): When the Plunger is moving to its home position, if it thinks it's jammed, this is how long it will keep trying or stay in that jammed state before it stops or tries something else. Measured in milliseconds (ms).
*   **2.3. Jammed Duration - Operation** (`jammedDurationMs`): Similar to the above, but this is for when the Plunger is doing a general operation (not specifically homing). If it detects a jam, this is how long it considers itself jammed. Unit: ms.
*   **2.4. Max Universal Jam Time** (`maxUniversalJamTimeMs`): This is a safety feature. It's the absolute maximum time the Plunger will keep trying to push if it's jammed, no matter what it's doing. After this time, it will give up to prevent damage. Unit: ms.
*   **2.5. Auto Mode Hold Time** (`autoModeHoldDurationMs`): To start "Automatic Plunging" or "Automatic Homing," you need to hold the joystick UP or DOWN for a certain amount of time. This setting is that duration. (Default in User Manual: 3.5 seconds). Unit: ms.
*   **2.6. Fill Joystick Hold Time** (`fillJoystickHoldDurationMs`): To start the "Filling Sequence," you need to hold the joystick to the LEFT. This setting is how long you need to hold it. (Default in User Manual: 2.5 seconds). Unit: ms.
*   **2.7. Fill Plunged Wait Time** (`fillPlungedWaitDurationMs`): In the "Filling Sequence," after the Plunger moves down and senses it's full (jammed), it will wait for this amount of time before it starts moving up again. (Default in User Manual: 1.3 seconds). Unit: ms.
*   **2.8. Fill Homed Wait Time** (`fillHomedWaitDurationMs`): In the "Filling Sequence," after the Plunger has moved all the way up (homed), it waits for this duration before the sequence is totally finished and it's ready for a new command. (Default in User Manual: 1.5 seconds). Unit: ms.
*   **2.9. Record Hold Time** (`recordHoldDurationMs`): To start "Record Mode" (where you record how long a plunge takes), you need to hold the joystick to the RIGHT. This is how long you need to hold it to begin recording. (Default in User Manual: 3 seconds). Unit: ms.
*   **2.10. Max Record Duration** (`maxRecordDurationMs`): When you're recording a plunge, there's a maximum time it will record for. If you go over this time, it will stop recording automatically. (Default in User Manual: 20 seconds). Unit: ms.
*   **2.11. Default Replay Duration** (`replayDurationMs`): If you want to "Replay" a plunge but haven't recorded one yet, the Plunger will use this default amount of time for the plunge. (Default in User Manual: 4.5 seconds). Unit: ms.
*   **2.12. Max Operation Duration** (`defaultMaxOperationDurationMs`): This is another safety net. It sets a general maximum time for any single movement the Plunger makes (like one manual push or one part of the filling sequence). If a movement takes longer than this, it will be stopped. Unit: ms.


## 3. Post-Flow Settings

"Post-Flow" is an optional step that can happen after the Plunger replays a recorded movement. It might be a little extra push or movement.

*   **3.1. Enable Post Flow** (`enablePostFlow`): This is a simple on/off switch. If it's set to `true` (on), the post-flow sequence will happen after a replay. If `false` (off), it won't.
*   **3.2. Post Flow Duration** (`postFlowDurationMs`): If post-flow is enabled, this is how long the main pressing part of that sequence will last. Unit: ms.
*   **3.3. Post Flow Speed** (`postFlowSpeedHz`): This controls how fast the Plunger moves during the post-flow sequence. Unit: Hz.
*   **3.4. Post Flow Current Threshold** (`currentPostFlowMa`): Just like the main jam detection, this is the motor current level that tells the Plunger it's jammed specifically during the post-flow movement. Unit: mA.
*   **3.5. Post Flow Stopping Wait Time** (`postFlowStoppingWaitMs`): After the main replay finishes, the Plunger will wait for this amount of time *before* it starts the post-flow movement. Unit: ms.
*   **3.6. Post Flow Complete Wait Time** (`postFlowCompleteWaitMs`): After the post-flow movement is done and the motor has stopped, the Plunger waits for this amount of time before it goes back to its normal ready (IDLE) state. Unit: ms.

---

**Important Notes:**
*   The names in `backticks` (like `speedSlowHz`) are the technical names for these settings. You might see these in configuration files or if a technician is looking at the system.
*   **Units**:
    *   `ms`: milliseconds (1000 milliseconds = 1 second)
    *   `Hz`: Hertz (a measure of motor speed)
    *   `mA`: milliamperes (a measure of electrical current)
*   The "(User Manual: X seconds)" notes tell you what the Plunger User Manual says about that setting, usually the default time. The actual setting on your machine might be different if it has been changed.
*   For speeds in Hz, remember that the number in the settings file might be 100 times bigger than the actual Hz value (e.g., a setting of 2550 means 25.50 Hz).

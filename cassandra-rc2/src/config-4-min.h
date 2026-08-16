#ifndef CONFIG_4_MIN_H
#define CONFIG_4_MIN_H

/**
 * Minimal configuration for ESP32-P4 bring-up.
 * Disables most peripherals and logic to isolate issues.
 */

// --- Disable Features ---
#define DISABLE_MODBUS_TCP
#define DISABLE_RS485
#define DISABLE_RS485_DEVICES

// Disable Drivers/Components
#undef ENABLE_OMRON_E5
#undef ENABLE_SAKO_VFD
#undef ENABLE_DELTA_VFD
#undef ENABLE_HY_VFD
#undef ENABLE_LOADCELL_0
#undef ENABLE_LOADCELL_1
#undef ENABLE_SOLENOID_0
#undef ENABLE_SOLENOID_1
#undef ENABLE_PRESS_CYLINDER
#undef ENABLE_JOYSTICK
#undef ENABLE_OPERATOR_SWITCH
#undef ENABLE_RELAYS
#undef ENABLE_STATUS
#undef ENABLE_FEEDBACK_3C
#undef ENABLE_FEEDBACK_BUZZER
#undef ENABLE_PID
#undef ENABLE_PROFILE_TEMPERATURE
#undef ENABLE_PROFILE_PRESSURE
#undef ENABLE_PROFILE_SIGNAL_PLOT
#undef ENABLE_MB_SCRIPT
#undef ENABLE_SETTINGS
#undef ENABLE_MODBUS_MIRROR
#undef ENABLE_EXTRUDER
#undef ENABLE_PLUNGER
#undef ENABLE_SETTINGS
#undef ENABLE_AMPERAGE_BUDGET_MANAGER
#undef PROFILE_SIGNAL_PLOT_COUNT
#undef PROFILE_TEMPERATURE_COUNT
#undef PROFILE_PRESSURE_COUNT
#undef NUM_OMRON_DEVICES

#define NUM_OMRON_DEVICES 0
#define PROFILE_PRESSURE_COUNT 0
#define PROFILE_TEMPERATURE_COUNT 0
#define PROFILE_SIGNAL_PLOT_COUNT 0

// Disable Profiler for now to reduce overhead
// #undef ENABLE_PROFILER

// --- Drivers Override ---
// (Ensure these are definitely off if undef isn't enough due to logic order)
// You might need to check how these are used. If #ifdef ENABLE_X is used, #undef is fine.

// Override Baud Rate to match P4 monitor_speed
#undef SERIAL_BAUD_RATE
#define SERIAL_BAUD_RATE 115200

#endif // CONFIG_4_MIN_H

#ifndef CONFIG_VALIDATE_H
#define CONFIG_VALIDATE_H

////////////////////////////////////////////////////////////
//
//  Feature Dependency Checks
//
/**
 * @file config-validate.h
 * @brief Compile-time validation of feature flags and their dependencies.
 *
 * This file uses preprocessor directives to check if the combination of
 * feature flags defined in config.h is valid. If an invalid combination
 * is detected, it will generate a compile-time error, preventing the
 * firmware from being built with an incorrect configuration.
 */
////////////////////////////////////////////////////////////

/**
 * @defgroup config_validation Feature Dependency Checks
 * @brief Checks for dependencies between different enabled features.
 * @{
 */

// WiFi and Network Features
#if (defined(ENABLE_WEBSERVER) || defined(ENABLE_REST_SERVER) ||               \
     defined(ENABLE_WEBSOCKET) || defined(ENABLE_MODBUS_TCP)) &&               \
    !defined(ENABLE_WIFI)
#error                                                                         \
    "Network features (WebServer, REST, WebSocket, ModbusTCP) require ENABLE_WIFI to be defined."
#endif

#ifdef ENABLE_LOGGING_TARGET_WEBSOCKET
#if !defined(ENABLE_LOGGER)
#error "ENABLE_LOGGING_TARGET_WEBSOCKET requires ENABLE_LOGGER."
#endif
#if !defined(ENABLE_WEBSOCKET)
#error "ENABLE_LOGGING_TARGET_WEBSOCKET requires ENABLE_WEBSOCKET."
#endif
#endif

#ifdef ENABLE_LOGGING_TARGET_FILE
#if !defined(ENABLE_LOGGER)
#error "ENABLE_LOGGING_TARGET_FILE requires ENABLE_LOGGER."
#endif
#if !defined(ENABLE_LITTLEFS)
#error "ENABLE_LOGGING_TARGET_FILE requires ENABLE_LITTLEFS."
#endif
#endif

// RS485 and dependant components
#if (defined(ENABLE_OMRON_E5) || defined(ENABLE_SAKO_VFD)) &&                  \
    !defined(ENABLE_RS485)
#error "Omron E5 and SAKO VFD components require ENABLE_RS485."
#endif

// High-level component dependencies
#ifdef ENABLE_EXTRUDER
#if !defined(ENABLE_SAKO_VFD)
#error "ENABLE_EXTRUDER requires ENABLE_SAKO_VFD."
#endif
#endif

#ifdef ENABLE_PLUNGER
#if !defined(ENABLE_DELTA_VFD)
#error "ENABLE_PLUNGER requires ENABLE_DELTA_VFD."
#endif
#if !defined(ENABLE_JOYSTICK)
#warning                                                                       \
    "ENABLE_PLUNGER is typically used with ENABLE_JOYSTICK for manual control."
#endif
#if !defined(MB_ANALOG_0) || !defined(MB_ANALOG_1)
#error                                                                         \
    "ENABLE_PLUNGER requires MB_ANALOG_0 and MB_ANALOG_1 for position feedback."
#endif
#endif

#ifdef ENABLE_PRESS_CYLINDER
#if !defined(ENABLE_SOLENOID_0)
#error "ENABLE_PRESS_CYLINDER requires ENABLE_SOLENOID_0."
#endif
#if !defined(ENABLE_LOADCELL)
// #warning "ENABLE_PRESS_CYLINDER is typically used with ENABLE_LOADCELL for
// force feedback."
#endif
#endif

// Profile dependencies
#if defined(ENABLE_PROFILE_TEMPERATURE) && !defined(ENABLE_OMRON_E5)
#error "ENABLE_PROFILE_TEMPERATURE requires ENABLE_OMRON_E5 to control heaters."
#endif

#if (defined(ENABLE_PROFILE_TEMPERATURE) ||                                    \
     defined(ENABLE_PROFILE_SIGNAL_PLOT)) &&                                   \
    !defined(ENABLE_LITTLEFS)
#error                                                                         \
    "Temperature and Signal Profiles require LittleFS (ENABLE_LITTLEFS) to be enabled for storing plot data."
#endif

#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
#if !defined(ENABLE_OMRON_E5)
#error "ENABLE_AMPERAGE_BUDGET_MANAGER requires ENABLE_OMRON_E5."
#endif
#endif

// Platform-specific features
#ifdef ENABLE_OPERATOR_SWITCH
#if !defined(PIN_OPERATOR_SWITCH_STOP) || !defined(PIN_OPERATOR_SWITCH_CYCLE)
#error                                                                         \
    "ENABLE_OPERATOR_SWITCH requires PIN_OPERATOR_SWITCH_STOP and PIN_OPERATOR_SWITCH_CYCLE to be defined (typically for ESP32 platform)."
#endif
#endif

#ifdef ENABLE_FEEDBACK_3C
#if !defined(GPIO_PIN_CH4) || !defined(GPIO_PIN_CH5) || !defined(GPIO_PIN_CH6)
#error                                                                         \
    "ENABLE_FEEDBACK_3C requires GPIO_PIN_CH4, GPIO_PIN_CH5, and GPIO_PIN_CH6 to be defined (typically for ESP32 platform)."
#endif
#endif

#ifdef ENABLE_FEEDBACK_BUZZER
#if !defined(GPIO_PIN_CH3)
#error                                                                         \
    "ENABLE_FEEDBACK_BUZZER requires GPIO_PIN_CH3 to be defined (typically for ESP32 platform)."
#endif
#endif

#ifdef ENABLE_SOLENOID_0
#if !defined(PIN_SOLENOID_0)
#error                                                                         \
    "ENABLE_SOLENOID_0 requires PIN_SOLENOID_0 to be defined (typically for ESP32 platform)."
#endif
#endif

/** @} */ // end of config_validation group

#endif

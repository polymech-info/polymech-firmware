#ifndef CONFIG_EXTERN_H
#define CONFIG_EXTERN_H

/**
 * @file config-extern.h
 * @brief Overrides for feature flags based on external build configurations (e.g., platformio.ini).
 *
 * This file is included after config.h but before config-validate.h.
 * It allows for dynamic enabling/disabling of features using build flags,
 * which is useful for creating different firmware variants from the same codebase.
 *
 * Example usage in platformio.ini:
 * build_flags = -D CORE_ONLY
 *               -D PIO_DISABLE_WIFI
 */

////////////////////////////////////////////////////////////
//
//  Platform.io config overrides
//
/**
 * @defgroup pio_overrides PlatformIO Overrides
 * @brief These flags can be defined in `platformio.ini` to create different build variants.
 * @{
 */

/**
 * @brief If CORE_ONLY is defined, strip down the firmware to its essential components.
 * This is useful for creating a minimal build for debugging or specific hardware without peripherals.
 */
#ifdef CORE_ONLY
    #undef ENABLE_AMPERAGE_BUDGET_MANAGER
    #undef ENABLE_PROFILE_SIGNAL_PLOT
    #undef ENABLE_PROFILE_TEMPERATURE
    #undef ENABLE_OPERATOR_SWITCH
    #undef ENABLE_RELAYS
    #undef ENABLE_FEEDBACK_3C
    #undef ENABLE_FEEDBACK_BUZZER
    #undef ENABLE_SAKO_VFD
    #undef ENABLE_OMRON_E5
    #undef ENABLE_WIFI
    #undef ENABLE_PRESS_CYLINDER
    #undef ENABLE_SOLENOID_0
    #undef ENABLE_LOADCELL
    #undef ENABLE_JOYSTICK
    #undef ENABLE_LITTLEFS
    #undef ENABLE_MODBUS_TCP
    #undef ENABLE_WEBSERVER
    #undef ENABLE_REST_SERVER
    #undef ENABLE_WEBSOCKET
#endif

/**
 * @brief If PIO_DISABLE_WIFI is defined, disable all network-related features.
 */
#ifdef PIO_DISABLE_WIFI
    #undef ENABLE_WIFI
    #undef ENABLE_AP_STA
    #undef ENABLE_WEBSERVER
    #undef ENABLE_REST_SERVER
    #undef ENABLE_WEBSOCKET
    #undef ENABLE_MODBUS_TCP
    #undef ENABLE_WEBSERVER_WIFI_SETTINGS
#endif

/**
 * @brief If PIO_DISABLE_BUZZER is defined, disable the feedback buzzer.
 */
#ifdef PIO_DISABLE_BUZZER
    #undef ENABLE_FEEDBACK_BUZZER
#endif

/**
 * @defgroup pio_granular_overrides Granular Feature Overrides
 * @brief These flags can be defined in `platformio.ini` to disable specific features,
 * providing fine-grained control over the firmware build.
 * @{
 */

#ifdef PIO_DISABLE_JOYSTICK
    #undef ENABLE_JOYSTICK
#endif
#ifdef PIO_DISABLE_OPERATOR_SWITCH
    #undef ENABLE_OPERATOR_SWITCH
#endif
#ifdef PIO_DISABLE_RELAYS
    #undef ENABLE_RELAYS
#endif
#ifdef PIO_DISABLE_SOLENOID
    #undef ENABLE_SOLENOID_0
#endif
#ifdef PIO_DISABLE_PRESS_CYLINDER
    #undef ENABLE_PRESS_CYLINDER
#endif
#ifdef PIO_DISABLE_FEEDBACK_3C
    #undef ENABLE_FEEDBACK_3C
#endif
#ifdef PIO_DISABLE_SAKO_VFD
    #undef ENABLE_SAKO_VFD
#endif
#ifdef PIO_DISABLE_LOADCELL
    #undef ENABLE_LOADCELL
#endif
#ifdef PIO_DISABLE_OMRON_E5
    #undef ENABLE_OMRON_E5
#endif
#ifdef PIO_DISABLE_AMPERAGE_BUDGET_MANAGER
    #undef ENABLE_AMPERAGE_BUDGET_MANAGER
#endif
#ifdef PIO_DISABLE_PROFILES
    #undef ENABLE_PROFILE_TEMPERATURE
    #undef ENABLE_PROFILE_SIGNAL_PLOT
#endif
#ifdef PIO_DISABLE_RS485
    #undef ENABLE_RS485
    #undef ENABLE_RS485_DEVICES
#endif
#ifdef PIO_DISABLE_PROFILER
    #undef ENABLE_PROFILER
#endif
#ifdef PIO_DISABLE_LOGGER
    #undef ENABLE_LOGGER
    #undef ENABLE_LOGGING_TARGET_PRINT
    #undef ENABLE_LOGGING_TARGET_WEBSOCKET
    #undef ENABLE_LOGGING_TARGET_FILE
#endif
#ifdef PIO_DISABLE_SETTINGS
    #undef ENABLE_SETTINGS
#endif
#ifdef PIO_DISABLE_LITTLEFS
    #undef ENABLE_LITTLEFS
#endif

/** @} */ // end of pio_granular_overrides group

/** @} */ // end of pio_overrides group

#endif // CONFIG_EXTERN_H 
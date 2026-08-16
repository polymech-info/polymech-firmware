#ifndef FEATURES_H
#define FEATURES_H

#include "config.h"

#define HAS_LOGGER

#ifdef ENABLE_STATUS
#include <components/StatusLight.h>
#endif

#if (defined(AUX_ANALOG_3POS_SWITCH_0) && defined(AUX_ANALOG_3POS_SWITCH_1))
#include <components/3PosAnalog.h>
#endif

#ifdef ENABLE_RELAYS
#if defined(AUX_RELAY_0) || defined(AUX_RELAY_1) || defined(AUX_RELAY_2) || defined(AUX_RELAY_3)
#include <components/Relay.h>
#endif
#endif

#if defined(ENABLE_POT0) || defined(ENABLE_POT1)
#include <components/POT.h>
#endif

#if defined(ENABLE_MB_GPIO) || defined(MB_GPIO_MB_MAP_7)
#include <components/GPIO.h>
#endif

#ifdef PIN_ANALOG_LEVEL_SWITCH_0
#include <components/AnalogLevelSwitch.h>
#endif

#ifdef PIN_LED_FEEDBACK_0
#include <components/LEDFeedback.h>
#endif

#ifdef ENABLE_JOYSTICK
#include <components/Joystick.h>
#endif

#ifdef PIN_AUX_1
#include <components/PushButton.h>
#endif

#ifdef ENABLE_FEEDBACK_3C
#include <components/Feedback3C.h>
#endif

#ifdef ENABLE_FEEDBACK_BUZZER
#include <components/FeedbackBuzzer.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Network
//
#ifdef ENABLE_MODBUS_TCP
#include <ModbusServerTCPasync.h>
#include <modbus/ModbusTCP.h>
#endif

#ifdef ENABLE_MODBUS_TCP
#include <WiFi.h>
#endif

#ifdef ENABLE_REST_SERVER
#include <components/RestServer.h>
#endif

#ifdef ENABLE_LITTLEFS
#include <LittleFS.h>
#endif

#ifdef ENABLE_PID
#include "./pid/PIDController.h"
#endif

#ifdef ENABLE_PROFILER
#ifdef PLATFORM_ESP32
#include <esp_cpu.h>
#endif
#endif

#ifdef NUM_OMRON_DEVICES
#include <components/OmronE5.h>
#endif

#ifdef ENABLE_RS485
#include <components/RS485.h>
#endif

#ifdef ENABLE_PROFILE_TEMPERATURE
#include <profiles/TemperatureProfile.h>
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
#include <profiles/SignalPlot.h>
#endif

#ifdef ENABLE_SAKO_VFD
#include <components/SAKO_VFD.h>
#endif

#ifdef ENABLE_DELTA_VFD
#include <components/DELTA_VFD.h>
#endif

#if defined(ENABLE_LOADCELL_0) || defined(ENABLE_LOADCELL_1)
#include <components/Loadcell.h>
#endif

#ifdef ENABLE_EXTRUDER
#include <components/Extruder.h>
#endif

#ifdef ENABLE_PLUNGER
#include <components/Plunger.h>
#endif

#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
#include <components/AmperageBudgetManager.h>
#endif

#ifdef ENABLE_NETWORK_VALUE_TEST
#include <components/NetworkValueTest.h>
#endif

#ifdef ENABLE_OPERATOR_SWITCH
#include <components/OperatorSwitch.h>
#endif

#ifdef ENABLE_MODBUS_MIRROR
#include "components/ModbusMirror.h"
#endif

#ifdef ENABLE_SETTINGS
#include "Settings.h"
#endif

#ifdef ENABLE_LOGGER
#include <components/Logger.h>
#endif

#ifdef ENABLE_IFTTT
#include <components/IFTTT.h>
#endif

#if defined(ENABLE_SOLENOID_0) || defined(ENABLE_SOLENOID_1)
#include <components/Solenoid.h>
#endif

#ifdef ENABLE_PRESS_CYLINDER
#include <components/PressCylinder.h>
#endif

#ifdef ENABLE_RUNTIME_STATE
#include "RuntimeState.h"
#endif

#ifdef ENABLE_INFLUXDB
#include <InfluxDbClient.h>
#include "components/InfluxDB.h"
#endif

#ifdef ENABLE_NETWORK_VALUE_TEST_PB
#include "components/NetworkValueTestPB.h"
#endif

#include <Vector.h>

#endif
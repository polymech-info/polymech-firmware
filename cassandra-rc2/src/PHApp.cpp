#include <Arduino.h>
#include <ArduinoLog.h>
#include <Component.h>
#include <enums.h>
#include <macros.h>

#include "./PHApp.h"
#include "./config-modbus.h"
#include "./config.h"
#include "Settings.h"
#include "esp32_compat.h"
#include <components/Logger.h>

#include <components/OmronE5.h>
#include <components/OmronE5Types.h>

#ifdef ENABLE_MODBUS_TCP
#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>
#endif

#include <LittleFS.h>
#include <components/RestServer.h>

#if defined(ENABLE_AMPERAGE_BUDGET_MANAGER)
void PHApp_onWarmupComplete(Component *owner) {
  PHApp *app = static_cast<PHApp *>(owner);
  if (app) {
    bool wasInitializing = false;
#ifdef ENABLE_PROFILE_TEMPERATURE
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; i++) {
      if (app->tempProfiles[i] != nullptr &&
          app->tempProfiles[i]->getCurrentStatus() ==
              PlotStatus::INITIALIZING) {
        wasInitializing = true;
        break;
      }
    }
#endif

    if (wasInitializing) {
      // TODO: Implement any logic that should occur when the warmup is
      // complete.
      LS_INFO("PHApp received warmup complete notification from "
              "AmperageBudgetManager.");
#ifdef ENABLE_FEEDBACK_BUZZER
      if (app->feedbackBuzzer_0) {
        app->feedbackBuzzer_0->setMode(
            FeedbackBuzzer::E_BuzzerMode::MODE_FAST_BLINK, 3000);
      }
#endif
    }
  }
}
#endif

#define MB_R_APP_STATE_REG 9
#define MB_R_SYSTEM_CMD_PRINT_RESET 1
#define MB_R_SYSTEM_CMD_PRINT_REGS 2
#define MB_R_SYSTEM_CMD_PRINT_MEMORY 5
#define MB_R_SYSTEM_CMD_PRINT_VFD 6

#ifdef ENABLE_PROFILER
uint32_t PHApp::initialFreeHeap = 0;
uint64_t PHApp::initialCpuTicks = 0;
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Network Servers
//
#if defined(ENABLE_WEBSERVER)
WiFiServer server(80);
#endif

#ifdef ENABLE_MODBUS_TCP
ModbusServerTCPasync mb;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Omron E5
//
#undef ENABLE_TRUTH_COLLECTOR

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Factory : Instances
//
#define ADD_RELAY(relayNum, relayPin, relayKey, relayAddr)                     \
  relay_##relayNum = new Relay(this, relayPin, relayKey, relayAddr);           \
  components.push_back(relay_##relayNum);

#ifdef ENABLE_SOLENOID_0
#define ADD_SOLENOID(solenoidNum, solenoidPin, solenoidKey, solenoidAddr)      \
  solenoid_##solenoidNum =                                                     \
      new Solenoid(this, solenoidPin, solenoidKey, solenoidAddr);              \
  components.push_back(solenoid_##solenoidNum);
#endif

#define ADD_POT(potNum, potPin, potKey, potAddr, ...)                          \
  pot_##potNum = new POT(this, potPin, potKey, potAddr, ##__VA_ARGS__);        \
  components.push_back(pot_##potNum);

#define ADD_GPIO_CONFIG(configs, gpioPin, gpioAddr, valueName, typeName)       \
  configs.push_back(GPIO_PinConfig(                                            \
      static_cast<E_GPIO_Pin>(gpioPin), MB_GPIO_DEFAULT_TYPE, gpioAddr,        \
      E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE,                          \
      MB_GPIO_OP_INTERVAL_MS, valueName, "GPIO_Group", typeName));

#define MB_GPIO_TYPE_ENUM_DESC                                                \
  "(0:Unknown,1:Input,2:Output,3:InputPullup,4:InputPulldown,5:OpenDrain,6:Analog,7:Touch)"

#define MB_GPIO_ADDR_FOR_PIN(gpioPin)                                          \
  (MB_GPIO_BASE_ADDR + E_NVC_USER +                                             \
   ((gpioPin) * GPIO_PinConfig::MODBUS_REGISTER_COUNT))

#define ADD_GPIO_ESP32_PIN(configs, gpioPin)                                   \
  ADD_GPIO_CONFIG(configs, gpioPin, MB_GPIO_ADDR_FOR_PIN(gpioPin),             \
                  "GPIO " #gpioPin " Value",                                 \
                  "GPIO " #gpioPin " Type:" MB_GPIO_TYPE_ENUM_DESC)

#define ADD_POS3ANALOG(posNum, switchPin1, switchPin2, switchKey, switchAddr)  \
  pos3Analog_##posNum =                                                        \
      new Pos3Analog(this, switchPin1, switchPin2, switchKey, switchAddr);     \
  components.push_back(pos3Analog_##posNum);

#ifdef ENABLE_PID
#define ADD_PID(pidNum, nameStr, doPin, csPin, clkPin, outPin, key)            \
  pidController_##pidNum =                                                     \
      new PIDController(key, nameStr, doPin, csPin, clkPin, outPin);           \
  components.push_back(pidController_##pidNum);
#endif

Logger *g_logger = nullptr;

void PHApp::printRegisters() {
  L_VERBOSE(F("--- Entering PHApp::printRegisters ---"));

#if ENABLED(HAS_MODBUS_REGISTER_DESCRIPTIONS)
  Log.setShowLevel(false);
  Serial.print("| Name | ID  | Address | RW |  Function Code | Number "
               "Addresses |Register Description| \n");
  Serial.print("|------|----------|----|----|----|----|-------|\n");
  short size = components.size();
  L_VERBOSE(F("PHApp::printRegisters - Processing %d components..."), size);
  for (int i = 0; i < size; i++) {
    Component *component = components[i];
    if (!component) {
      L_ERROR(F("PHApp::printRegisters - Found NULL component at index %d"), i);
      continue;
    }
    L_VERBOSE(F("PHApp::printRegisters - Component %d: ID=%d, Name=%s"), i,
              component->id, component->name.c_str());
    // if (!(component->nFlags & 1 << OBJECT_NET_CAPS::E_NCAPS_MODBUS)) // <--
    // Modbus flag check might be different now
    // {
    //   continue;
    // }
    // Log.verbose("| %s | %d | %d | %s | %d | %d | %s |\n", // <-- Calls to
    // removed ModbusGateway methods
    //             component->name.c_str(),
    //             component->id,
    //             component->getAddress(),
    //             component->getRegisterMode(),
    //             component->getFunctionCode(),
    //             component->getNumberAddresses(),
    //             component->getRegisterDescription().c_str());
    Log.verbose("| %s | %d | - | - | - | - | - |\n", // <-- Simplified output
                component->name.c_str(), component->id);
  }
  Log.setShowLevel(true);
#endif
  L_VERBOSE(F("--- Exiting PHApp::printRegisters ---"));
}
short PHApp::reset(short val0, short val1) {
  _state = APP_STATE::RESET;
  _error = E_OK;

#if defined(ESP32) ||                                                          \
    defined(ESP8266) // Use ESP.restart() for ESP32 and ESP8266
  ESP.restart();
#else
  return E_NOT_IMPLEMENTED;
#endif
  return E_OK;
}
short PHApp::list(short val0, short val1) {
  uchar s = components.size();
  for (uchar i = 0; i < s; i++) {
    Component *component = components[i];
    if (component) {
      L_VERBOSE("PHApp::list - %d | %s (ID: %d)", i, component->name.c_str(),
                component->id);
    } else {
      Log.warningln("PHApp::list - NULL component at index %d", i);
    }
  }
  return E_OK;
}
short PHApp::setup() {
  _state = APP_STATE::RESET;
  _error = E_OK;
#ifdef ENABLE_PROFILER
  if (initialFreeHeap == 0 && initialCpuTicks == 0) {
    initialFreeHeap = ESP.getFreeHeap();
    initialCpuTicks = esp_cpu_get_cycle_count();
  }
#endif

#ifndef DISABLE_SERIAL_LOGGING
  // Serial Setup
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial && !Serial.available()) {
  }
#endif

#ifdef ENABLE_LOGGER
  logger_0 = new Logger(this, COMPONENT_KEY_LOGGER, MB_ADDR_LOGGER_0);
  g_logger = logger_0;
  components.push_back(logger_0);
#endif

#ifndef DISABLE_SERIAL_LOGGING
  // Log Setup
#if defined(ENABLE_LOGGER)
#if defined(ENABLE_LOGGING_TARGET_PRINT)
  if (logger_0) {
    Serial.begin(SERIAL_BAUD_RATE);
    logger_0->addPrintTarget(&Serial);
  }
#endif
  Log.begin(LOG_LEVEL, logger_0);
#else
  Log.begin(LOG_LEVEL, &Serial);
#endif
  Log.setShowLevel(true);
#else
  // Log Setup (without Serial)
  Log.begin(LOG_LEVEL_WARNING, nullptr);
  Log.setShowLevel(false);
#endif

  L_INFO("PHApp::setup() started. Logger configured.");

  // Application stuff
#ifdef ENABLE_RUNTIME_STATE
  runtimeState = new RuntimeState(this, COMPONENT_KEY_RUNTIME_STATE);
  components.push_back(runtimeState);
#endif

// Components
#ifdef ENABLE_SERIAL_BRIDGE
  bridge = new Bridge(this);
  components.push_back(bridge);
#endif
#ifndef DISABLE_SERIAL_LOGGING
  com_serial = new SerialMessage(Serial, bridge);
  components.push_back(com_serial);
#endif
  // Network
  short networkSetupResult = setupNetwork();
  if (networkSetupResult != E_OK) {
    L_ERROR("Network setup failed with error code: %d", networkSetupResult);
  }

#ifdef ENABLE_SETTINGS
  appSettings = new Settings(this);
  components.push_back(appSettings);

#endif

#ifdef ENABLE_LOGGER
#if defined(ENABLE_LOGGING_TARGET_WEBSOCKET) && defined(ENABLE_WEBSERVER) &&   \
    defined(ENABLE_WEBSOCKET)
  if (logger_0 && webServer) {
    logger_0->addWebSocketTarget(webServer);
    L_INFO("WebSocket logging target added.");
  }
#endif
#if defined(ENABLE_LOGGING_TARGET_FILE) && defined(ENABLE_LITTLEFS)
  if (logger_0) {
    logger_0->addFileTarget();
    L_INFO("File logging target added.");
  }
#endif
#endif

  // Components
#ifdef ENABLE_RELAYS
#ifdef AUX_RELAY_0
  ADD_RELAY(0, AUX_RELAY_0, COMPONENT_KEY_RELAY_0, MB_ADDR_AUX_5);
#endif
#ifdef AUX_RELAY_1
  ADD_RELAY(1, AUX_RELAY_1, COMPONENT_KEY_RELAY_1, MB_ADDR_AUX_6);
#endif
#endif

#ifdef ENABLE_POT0
  ADD_POT(0, MB_ANALOG_0, COMPONENT_KEY_ANALOG_0, MB_ADDR_AUX_2,
          POTDampingAlgorithm::DAMPING_EMA, true);
#endif

#ifdef ENABLE_POT1
  ADD_POT(1, MB_ANALOG_1, COMPONENT_KEY_ANALOG_1, MB_ADDR_AUX_3,
          POTDampingAlgorithm::DAMPING_EMA, true);
#endif

#if (defined(AUX_ANALOG_3POS_SWITCH_0) && (defined(AUX_ANALOG_3POS_SWITCH_1)))
  ADD_POS3ANALOG(0, AUX_ANALOG_3POS_SWITCH_0, AUX_ANALOG_3POS_SWITCH_1,
                 COMPONENT_KEY_MB_ANALOG_3POS_SWITCH_0, MB_ADDR_AUX_3);
#endif

#if (defined(MB_ANALOG_3POS_SWITCH_2) && (defined(MB_ANALOG_3POS_SWITCH_3)))
  // ADD_POS3ANALOG(1, MB_ANALOG_3POS_SWITCH_2, MB_ANALOG_3POS_SWITCH_3,
  // COMPONENT_KEY_MB_ANALOG_3POS_SWITCH_1, MB_R_SWITCH_1); // <-- Temporarily
  // disable
#endif

#ifdef ENABLE_MB_GPIO
#ifndef MB_GPIO_OP_INTERVAL_MS
#define MB_GPIO_OP_INTERVAL_MS 100
#endif
#ifndef MB_GPIO_DEFAULT_TYPE
#define MB_GPIO_DEFAULT_TYPE E_GPIO_TYPE_INPUT
#endif
#ifndef MB_GPIO_BASE_ADDR
#define MB_GPIO_BASE_ADDR 300
#endif
  std::vector<GPIO_PinConfig> gpioConfigs;
#ifdef MB_GPIO_ALL_ESP32_PINS
  gpioConfigs.reserve(34);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 0);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 1);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 2);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 3);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 4);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 5);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 6);
#ifndef ENABLE_POT1
  ADD_GPIO_ESP32_PIN(gpioConfigs, 7);
#endif
  ADD_GPIO_ESP32_PIN(gpioConfigs, 8);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 9);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 10);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 11);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 12);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 13);
#ifndef ENABLE_OPERATOR_SWITCH
  ADD_GPIO_ESP32_PIN(gpioConfigs, 14);
#endif
#if !defined(ENABLE_POT0) && !defined(ENABLE_OPERATOR_SWITCH)
  ADD_GPIO_ESP32_PIN(gpioConfigs, 15);
#endif
#ifndef PIN_AUX_1
  ADD_GPIO_ESP32_PIN(gpioConfigs, 16);
#endif
#ifndef ENABLE_RS485
  ADD_GPIO_ESP32_PIN(gpioConfigs, 17);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 18);
#endif
  ADD_GPIO_ESP32_PIN(gpioConfigs, 19);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 20);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 21);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 35);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 36);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 37);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 38);
#ifndef ENABLE_JOYSTICK
  ADD_GPIO_ESP32_PIN(gpioConfigs, 39);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 40);
#endif
  ADD_GPIO_ESP32_PIN(gpioConfigs, 41);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 42);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 43);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 44);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 45);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 46);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 47);
  ADD_GPIO_ESP32_PIN(gpioConfigs, 48);
#else
  gpioConfigs.reserve(6);
#ifdef MB_GPIO_ADDR_CH1
  ADD_GPIO_CONFIG(gpioConfigs, GPIO_PIN_CH1, MB_GPIO_ADDR_CH1, "GPIO CH1 Value",
                  "GPIO CH1 Type:" MB_GPIO_TYPE_ENUM_DESC);
#endif
#ifdef MB_GPIO_ADDR_CH2
  ADD_GPIO_CONFIG(gpioConfigs, GPIO_PIN_CH2, MB_GPIO_ADDR_CH2, "GPIO CH2 Value",
                  "GPIO CH2 Type:" MB_GPIO_TYPE_ENUM_DESC);
#endif
#ifdef MB_GPIO_ADDR_CH3
  ADD_GPIO_CONFIG(gpioConfigs, GPIO_PIN_CH3, MB_GPIO_ADDR_CH3, "GPIO CH3 Value",
                  "GPIO CH3 Type:" MB_GPIO_TYPE_ENUM_DESC);
#endif
#ifdef MB_GPIO_ADDR_CH4
  ADD_GPIO_CONFIG(gpioConfigs, GPIO_PIN_CH4, MB_GPIO_ADDR_CH4, "GPIO CH4 Value",
                  "GPIO CH4 Type:" MB_GPIO_TYPE_ENUM_DESC);
#endif
#ifdef MB_GPIO_ADDR_CH5
  ADD_GPIO_CONFIG(gpioConfigs, GPIO_PIN_CH5, MB_GPIO_ADDR_CH5, "GPIO CH5 Value",
                  "GPIO CH5 Type:" MB_GPIO_TYPE_ENUM_DESC);
#endif
#ifdef MB_GPIO_ADDR_CH6
  ADD_GPIO_CONFIG(gpioConfigs, GPIO_PIN_CH6, MB_GPIO_ADDR_CH6, "GPIO CH6 Value",
                  "GPIO CH6 Type:" MB_GPIO_TYPE_ENUM_DESC);
#endif
#endif
  if (!gpioConfigs.empty()) {
    gpio_0 =
        new MB_GPIO(this, COMPONENT_KEY_GPIO_MAP, MB_GPIO_BASE_ADDR, gpioConfigs);
    components.push_back(gpio_0);
  }
#endif

#ifdef PIN_ANALOG_LEVEL_SWITCH_0
  analogLevelSwitch_0 =
      new AnalogLevelSwitch(this,                      // owner
                            PIN_ANALOG_LEVEL_SWITCH_0, // analogPin
                            ALS_0_NUM_LEVELS,          // numLevels
                            ALS_0_ADC_STEP,            // levelStep
                            ALS_0_ADC_OFFSET,          // adcValueOffset
                            ID_ANALOG_LEVEL_SWITCH_0,  // id
                            ALS_0_MB_ADDR              // modbusAddress
      );
  if (analogLevelSwitch_0) {
    components.push_back(analogLevelSwitch_0);
    L_INFO(F("AnalogLevelSwitch_0 initialized. Pin:%d, ID:%d, Levels:%d, "
             "Step:%d, Offset:%d, Smooth:%d(Fixed), Debounce:%d(Fixed), MB:%d"),
           PIN_ANALOG_LEVEL_SWITCH_0, ID_ANALOG_LEVEL_SWITCH_0,
           ALS_0_NUM_LEVELS, ALS_0_ADC_STEP, ALS_0_ADC_OFFSET,
           ALS_SMOOTHING_SIZE, ALS_DEBOUNCE_COUNT, ALS_0_MB_ADDR);
  } else {
    L_ERROR(F("AnalogLevelSwitch_0 initialization failed."));
  }
#endif

#ifdef ENABLE_STATUS
  statusLight_0 = new StatusLight(
      this, STATUS_WARNING_PIN, COMPONENT_KEY_FEEDBACK_0,
      MB_MONITORING_STATUS_FEEDBACK_0); // Keep original address for now, add to
                                        // config-modbus.h later if needed
  components.push_back(statusLight_0);
  statusLight_1 = new StatusLight(
      this, STATUS_ERROR_PIN, COMPONENT_KEY_FEEDBACK_1,
      MB_MONITORING_STATUS_FEEDBACK_1); // Keep original address for now
  components.push_back(statusLight_1);
#else
  statusLight_0 = NULL;
  statusLight_1 = NULL;
#endif
  L_INFO("PHApp::setup - Base App::setup() called.");

#ifdef PIN_LED_FEEDBACK_0
  ledFeedback_0 = new LEDFeedback(this,                  // owner
                                  PIN_LED_FEEDBACK_0,    // pin
                                  LED_PIXEL_COUNT_0,     // pixelCount
                                  ID_LED_FEEDBACK_0,     // id
                                  LED_FEEDBACK_0_MB_ADDR // modbusAddress
  );
  if (ledFeedback_0) {
    components.push_back(ledFeedback_0);
    L_INFO(F("LEDFeedback_0 initialized. Pin:%d, Count:%d, ID:%d, MB:%d"),
           PIN_LED_FEEDBACK_0, LED_PIXEL_COUNT_0, ID_LED_FEEDBACK_0,
           LED_FEEDBACK_0_MB_ADDR);
  } else {
    L_ERROR(F("LEDFeedback_0 initialization failed."));
  }
#endif

#ifdef ENABLE_JOYSTICK
  joystick_0 = new Joystick(this,               // owner
                            PIN_JOYSTICK_UP,    // UP pin
                            PIN_JOYSTICK_DOWN,  // DOWN pin
                            PIN_JOYSTICK_LEFT,  // LEFT pin
                            PIN_JOYSTICK_RIGHT, // RIGHT pin
                            MB_ADDR_AUX_7       // modbusAddress
  );
  if (joystick_0) {
    components.push_back(joystick_0);
  } else {
    L_ERROR(F("Joystick_0 initialization failed."));
  }
#endif

#ifdef PIN_AUX_1

  pushButton_1 = new PushButton(this, PIN_AUX_1, COMPONENT_KEY_PUSH_BUTTON_0,
                                MB_IREG_ANALOG_0);
  components.push_back(pushButton_1);

#endif

#ifdef ENABLE_FEEDBACK_3C
  feedback3C_0 = new Feedback3C(this, GPIO_PIN_CH4, GPIO_PIN_CH5, GPIO_PIN_CH6,
                                COMPONENT_KEY_FEEDBACK_0, MB_ADDR_FEEDBACK_0);
  components.push_back(feedback3C_0);
#endif

#ifdef ENABLE_FEEDBACK_BUZZER
  feedbackBuzzer_0 = new FeedbackBuzzer(
      this, GPIO_PIN_CH3, COMPONENT_KEY_FEEDBACK_1, MB_ADDR_AUX_9);
  components.push_back(feedbackBuzzer_0);
#endif

// Systems : Hydraulic Cylinder - Loadcell sensor - RS485
#ifdef ENABLE_SOLENOID_0
  uint32_t solenoid0Addr =
      appSettings->get("SOLENOID_0_MB_ADDR", (uint32_t)MB_ADDR_SOLENOID_0);
  ADD_SOLENOID(0, PIN_SOLENOID_0, COMPONENT_KEY_SOLENOID_0, solenoid0Addr);
#endif

#ifdef ENABLE_SOLENOID_1
  uint32_t solenoid1Addr =
      appSettings->get("SOLENOID_1_MB_ADDR", (uint32_t)MB_ADDR_SOLENOID_1);
  ADD_SOLENOID(1, PIN_SOLENOID_1, COMPONENT_KEY_SOLENOID_1, MB_ADDR_SOLENOID_1);
#endif

#ifdef ENABLE_LOADCELL_0
  uint32_t slaveId0 =
      appSettings->get("LOADCELL_SLAVE_ID_0", (uint32_t)LOADCELL_SLAVE_ID_0);
  loadCell_0 = new Loadcell(this, slaveId0, LOADCELL_READ_INTERVAL,
                            COMPONENT_KEY_LOADCELL_0);
  components.push_back(static_cast<RTU_Base *>(loadCell_0));
#endif

#ifdef ENABLE_LOADCELL_1
  uint32_t slaveId1 =
      appSettings->get("LOADCELL_SLAVE_ID_1", (uint32_t)LOADCELL_SLAVE_ID_1);
  loadCell_1 = new Loadcell(this, slaveId1, LOADCELL_READ_INTERVAL,
                            COMPONENT_KEY_LOADCELL_1);
  components.push_back(static_cast<RTU_Base *>(loadCell_1));
#endif

#ifdef ENABLE_PRESS_CYLINDER
  pressCylinder_0 =
      new PressCylinder(this, COMPONENT_KEY_PRESS_CYLINDER_0,
                        MB_ADDR_PRESS_CYLINDER_0, joystick_0, pushButton_1);
#ifdef ENABLE_LOADCELL_0
  if (loadCell_0)
    pressCylinder_0->addLoadcell(loadCell_0);
#endif
#ifdef ENABLE_LOADCELL_1
  if (loadCell_1)
    pressCylinder_0->addLoadcell(loadCell_1);
#endif
#ifdef ENABLE_SOLENOID_0
  if (solenoid_0)
    pressCylinder_0->addSolenoid(solenoid_0);
#endif
#ifdef ENABLE_SOLENOID_1
  if (solenoid_1)
    pressCylinder_0->addSolenoid(solenoid_1);
#endif
  components.push_back(pressCylinder_0);
#endif

#ifdef ENABLE_OPERATOR_SWITCH
  operatorSwitch_0 = new OperatorSwitch(
      this, PIN_OPERATOR_SWITCH_STOP, PIN_OPERATOR_SWITCH_CYCLE,
      COMPONENT_KEY_OPERATOR_SWITCH, MB_ADDR_AUX_8);
  components.push_back(operatorSwitch_0);
#endif

#ifdef ENABLE_MODBUS_MIRROR
  modbusMirror_0 = new ModbusMirror(this, COMPONENT_KEY_MODBUS_MIRROR);
  components.push_back(modbusMirror_0);
#endif

#ifdef ENABLE_IFTTT
  iftttWebhook = new IFTTTWebhook(this, COMPONENT_KEY_IFTTT);
  components.push_back(iftttWebhook);
#endif

#ifdef ENABLE_NETWORK_VALUE_TEST
  L_INFO("PHApp::setup - Initializing NetworkValueTest...");
  networkValueTest =
      new NetworkValueTest(this, COMPONENT_KEY_TEST_NV, MB_ADDR_AUX_TEST_NV);
  networkValueTest->owner = this;
  components.push_back(networkValueTest);
#endif

#ifdef ENABLE_NETWORK_VALUE_TEST_PB
  L_INFO("PHApp::setup - Initializing NetworkValueTestPB...");
  networkValueTestPB = new NetworkValueTestPB(this, COMPONENT_KEY_TEST_NV_PB,
                                              MB_ADDR_AUX_TEST_NV + 10);
  networkValueTestPB->owner = this;
  components.push_back(networkValueTestPB);
#endif

  // Motors
#ifdef ENABLE_SAKO_VFD
  vfd_0 = new SAKO_VFD(this, MB_SAKO_VFD_SLAVE_ID, MB_SAKO_VFD_READ_INTERVAL);
  components.push_back(vfd_0);
#endif

#ifdef ENABLE_DELTA_VFD
  vfd_2 =
      new DELTA_VFD(this, MB_DELTA_VFD_SLAVE_ID, MB_DELTA_VFD_READ_INTERVAL);
  components.push_back(vfd_2);
#endif

  // Temperature
#ifdef ENABLE_PROFILE_TEMPERATURE
  for (ushort i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i) {
    // Assign unique ID: COMPONENT_KEY_PROFILE_START (910) + slot index
    ushort profileComponentId = COMPONENT_KEY_PROFILE_START + i;
    ushort baseAddr =
        MB_HREG_TEMP_PROFILE_BASE + (i * TEMP_PROFILE_REGISTER_COUNT);
    tempProfiles[i] =
        new TemperatureProfile(this, i, profileComponentId, baseAddr);
    tempProfiles[i]->setEventsDelegate(this);
    components.push_back(tempProfiles[i]);
  }

#endif

#ifdef ENABLE_PROFILE_PRESSURE
  for (ushort i = 0; i < PROFILE_PRESSURE_COUNT; ++i) {
    ushort profileComponentId = COMPONENT_KEY_PRESSURE_PROFILE_START + i;
    ushort baseAddr =
        MB_HREG_PRESSURE_PROFILE_BASE + (i * PRESSURE_PROFILE_REGISTER_COUNT);
    pressureProfiles[i] =
        new PressureProfile(this, i, profileComponentId, baseAddr);
    pressureProfiles[i]->setEventsDelegate(this);
    components.push_back(pressureProfiles[i]);
  }
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
  for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; i++) {
    ushort profileComponentId = COMPONENT_KEY_SIGNAL_PLOT_START + i;
    ushort modbusAddress =
        MB_HREG_SIGNAL_PLOT_BASE + (i * SIGNAL_PLOT_REGISTER_COUNT);
    signalPlots[i] = new SignalPlot(this, i, profileComponentId, modbusAddress);
    signalPlots[i]->setEventsDelegate(this);
    components.push_back(signalPlots[i]);
#ifdef ENABLE_MODBUS_TCP
    if (modbusManager != nullptr) {
      signalPlots[i]->setModbusTCP(modbusManager);
    } else {
      L_ERROR("PHApp::setup - SignalPlot %d: modbusManager is nullptr", i);
    }
#endif
  }
#endif

#ifdef ENABLE_PID
  const int8_t PID2_THERMO_DO = 19;  // Example MISO pin
  const int8_t PID2_THERMO_CS = 5;   // Example Chip Select pin
  const int8_t PID2_THERMO_CLK = 18; // Example SCK pin
  const int8_t PID2_OUTPUT_PIN = 23; // Example PWM/Output pin
  ADD_PID(0, "PID Temp Controller 2", PID2_THERMO_DO, PID2_THERMO_CS,
          PID2_THERMO_CLK, PID2_OUTPUT_PIN, COMPONENT_KEY_PID_2);
#endif
// RS485
#ifdef ENABLE_RS485
  rs485 = new RS485(this);
  components.push_back(rs485);
#endif

#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
  pidManagerAmperage =
      new AmperageBudgetManager(this, MB_ADDR_AMPERAGE_BUDGET_BASE);
#if defined(ENABLE_OMRON_E5) && defined(ENABLE_PROFILE_TEMPERATURE)
  pidManagerAmperage->setCanUseCallback(&PHApp_canUsePID);
  pidManagerAmperage->setOnWarmupCompleteCallback(&PHApp_onWarmupComplete);
#endif
  components.push_back(pidManagerAmperage);
#endif

// Systems : Extruder
#ifdef ENABLE_EXTRUDER
  extruder_0 = new Extruder(this, vfd_0, nullptr, nullptr, nullptr);
  components.push_back(extruder_0);
#endif

// Systems : Injector
#ifdef ENABLE_PLUNGER
  plunger_0 = new Plunger(this, vfd_2, joystick_0, pot_0, pot_1);
  components.push_back(plunger_0);
  plunger_0->setEventsDelegate(this);
#endif

#ifdef ENABLE_INFLUXDB
  influxDb_0 = new InfluxDB(this, COMPONENT_KEY_INFLUXDB);
  components.push_back(influxDb_0);
#endif

  registerComponents(bridge);

#ifdef ENABLE_BRIDGE
  serial_register(bridge);
#endif

  App::setup();

#ifdef ENABLE_SETTINGS
  loadAppSettings();
#endif

  init(0, 0);
  onRun();
  return E_OK;
}

short PHApp::onRun() {
  App::onRun();

#ifdef ENABLE_MODBUS_TCP
  for (Component *comp : components) {
    if (comp && comp->hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS)) {
      comp->mb_tcp_register(modbusManager);
    }
  }
#endif

  load(0, 0);

#ifdef ENABLE_RELAYS
#ifdef AUX_RELAY_0
  relay_0->setValue(1);
#endif
#endif
  ///////////////////////////////////////////////////////////////////////////////////////////////
  //
  // Post initialization
  //
#ifdef ENABLE_WEBSERVER
  registerRoutes(webServer);
#endif

#if ENABLED(ENABLE_AMPERAGE_BUDGET_MANAGER, ENABLE_OMRON_E5)
  RTU_Base *const *devices = rs485->deviceManager.getDevices();
  int numDevicesInManager = rs485->deviceManager.getMaxDevices();
  for (int i = 0; i < numDevicesInManager; ++i) {
    if (devices[i] != nullptr) {
      Component *comp = devices[i];
      if (comp == nullptr || comp->type != COMPONENT_TYPE::COMPONENT_TYPE_PID) {
        continue;
      }

      if (!pidManagerAmperage->addManagedDevice(static_cast<OmronE5 *>(comp))) {
        L_ERROR("Failed to add OmronE5 device to AmperageBudgetManager");
      }
    }
  }
#endif

#if ENABLED(ENABLE_TEMPERATURE_PROFILES, ENABLE_OMRON_E5, ENABLE_MODBUS_TCP)
  tempProfiles[0]->disable();
  if (tempProfiles[0] && rs485) {
    RTU_Base *const *devices = rs485->deviceManager.getDevices();
    int numDevicesInManager = rs485->deviceManager.getMaxDevices();
    int targetRegisterIndex = 0; // Dedicated index for _targetRegisters
    for (int i = 0; i < numDevicesInManager; i++) {
      Component *comp = devices[i];
      if (comp == nullptr || comp->type != COMPONENT_TYPE::COMPONENT_TYPE_PID) {
        continue;
      }
    }
  }
#endif

#ifdef ENABLE_NETWORK_VALUE_TEST
  // networkValueTest->init();
#endif

#ifdef ENABLE_RUNTIME_STATE
  if (runtimeState) {
    for (auto *component : components) {
      if (component && component->hasPersistence(E_PF_ENABLED)) {
        runtimeState->addManagedComponent(component);
      }
    }
  }
#endif
  restoreState();
  // setAllOmronComWrite(true, false);
  return E_OK;
}
short PHApp::serial_register(Bridge *bridge) {
  /*
  bridge->registerMemberFunction(COMPONENT_KEY_BASE::COMPONENT_KEY_APP, this,
  C_STR("list"), (ComponentFnPtr)&PHApp::list);
  bridge->registerMemberFunction(COMPONENT_KEY_BASE::COMPONENT_KEY_APP, this,
  C_STR("reset"), (ComponentFnPtr)&PHApp::reset);
  bridge->registerMemberFunction(COMPONENT_KEY_BASE::COMPONENT_KEY_APP, this,
  C_STR("printRegisters"), (ComponentFnPtr)&PHApp::printRegisters);
  bridge->registerMemberFunction(COMPONENT_KEY_BASE::COMPONENT_KEY_APP, this,
  C_STR("load"), (ComponentFnPtr)&PHApp::load);
  */
  return E_OK;
}
short PHApp::onWarning(short code) { return E_OK; }
short PHApp::onError(short id, short code) {
  if (code == getLastError()) {
    return code;
  }
  Log.error(F("* App:onError - component=%d code=%d" CR), id, code);
  setLastError(code);
#ifdef ENABLE_STATUS
  if (statusLight_0) {
    switch (id) {
    case COMPONENT_KEY_PLUNGER: {

#if defined(ENABLE_PLUNGER)
    case (short)PlungerState::IDLE: {
      statusLight_1->set(0, 0);
      break;
    }
    case (short)PlungerState::JAMMED: {
      statusLight_1->set(1, 1);
      break;
    }
    default: {
      statusLight_1->set(1, 0);
      break;
    }
    }
#endif
    }
  }
}
#endif
return code;
}
short PHApp::onStop(short val) { return E_OK; }
short PHApp::clearError() {
  setLastError(E_OK);
  return E_OK;
}

short PHApp::loop() {
  App::loop();

  if (millis() - _last_cycle_loop_ms >= LOOP_CYCLE_INTERVAL) {
    _last_cycle_loop_ms = millis();
    loopCycle();
#ifdef ENABLE_PROFILE_TEMPERATURE
    loopProfiles();
#endif
  }

#ifdef ENABLE_RUNTIME_STATE
  if (runtimeState &&
      (millis() - _last_runtime_save_ms >= RUNTIME_STATE_SAVE_INTERVAL)) {
    _last_runtime_save_ms = millis();
    updateRuntimeState();
  }
#endif

#ifdef ENABLE_INFLUXDB
  testInfluxDB();
#endif

#ifdef ENABLE_WEBSERVER
  loopWeb();
#endif
#ifdef ENABLE_MODBUS_TCP
  loopModbus();
#endif
  return E_OK;
}

short PHApp::loopWeb() {
#if defined(ENABLE_WIFI) && defined(ENABLE_WEBSERVER)
  if (webServer != nullptr) {
    webServer->loop();
  }
#endif
  return E_OK;
}
short PHApp::getAppState(short val) { return _state; }
PHApp::PHApp() : App() {
  name = "PHApp";
  webServer = nullptr;
  pidController_0 = nullptr;
  bridge = nullptr;
  com_serial = nullptr;
  pot_0 = nullptr;
  pot_1 = nullptr;
  pot_2 = nullptr;
  statusLight_0 = nullptr;
  statusLight_1 = nullptr;
  relay_0 = nullptr;
  relay_1 = nullptr;
  relay_2 = nullptr;
  relay_3 = nullptr;
  relay_4 = nullptr;
  relay_5 = nullptr;
  relay_6 = nullptr;
  relay_7 = nullptr;
  pos3Analog_0 = nullptr;
  pos3Analog_1 = nullptr;
  modbusManager = nullptr;
  logPrinter = nullptr;
  rs485 = nullptr;
  joystick_0 = nullptr;
  networkValueTest = nullptr;
  networkValueTestPB = nullptr;
  feedback3C_0 = nullptr;
  feedbackBuzzer_0 = nullptr;
  operatorSwitch_0 = nullptr;
  solenoid_0 = nullptr;
  pressCylinder_0 = nullptr;
  appSettings = nullptr;
#ifdef ENABLE_PROFILE_TEMPERATURE
  // Initialize the array elements to nullptr
  for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i) {
    tempProfiles[i] = nullptr;
  }
#endif

#ifdef ENABLE_PROFILE_PRESSURE
  for (int i = 0; i < PROFILE_PRESSURE_COUNT; ++i) {
    pressureProfiles[i] = nullptr;
  }
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
  for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; ++i) {
    signalPlots[i] = nullptr;
  }
#endif

#ifdef ENABLE_INFLUXDB
  influxDb_0 = nullptr;
#endif

  // WiFi settings are now initialized by WiFiNetworkSettings constructor
}
void PHApp::cleanupComponents() {
  L_INFO("PHApp::cleanupComponents - Cleaning up %d components...",
         components.size());
  for (Component *comp : components) {
    delete comp;
  }
  components.clear(); // Clear the vector AFTER deleting objects

  // Nullify pointers that were manually managed or outside the vector
  bridge = nullptr;
  com_serial = nullptr;
  pot_0 = nullptr;
  pot_1 = nullptr;
  pot_2 = nullptr;
  statusLight_0 = nullptr;
  statusLight_1 = nullptr;
  relay_0 = nullptr;
  relay_1 = nullptr;
  relay_2 = nullptr;
  relay_3 = nullptr;
  relay_4 = nullptr;
  relay_5 = nullptr;
  relay_6 = nullptr;
  relay_7 = nullptr;
  pos3Analog_0 = nullptr;
  pos3Analog_1 = nullptr;
  pidController_0 = nullptr;
  modbusManager = nullptr;
  joystick_0 = nullptr;
  networkValueTest = nullptr;
  networkValueTestPB = nullptr;
  feedback3C_0 = nullptr;
  feedbackBuzzer_0 = nullptr;
  operatorSwitch_0 = nullptr;
  solenoid_0 = nullptr;
  pressCylinder_0 = nullptr;
  appSettings = nullptr;
#ifdef ENABLE_PROFILE_TEMPERATURE
  // Clean up temperature profiles (they were also added to components vector,
  // so already deleted there) Ensure the pointers in the array are nulled
  for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i) {
    tempProfiles[i] = nullptr;
  }
#endif
#ifdef ENABLE_PROFILE_PRESSURE
  for (int i = 0; i < PROFILE_PRESSURE_COUNT; ++i) {
    pressureProfiles[i] = nullptr;
  }
#endif
#ifdef ENABLE_WEBSERVER
  if (webServer) {
    delete webServer;
    webServer = nullptr;
  }
#endif
#ifdef ENABLE_RS485
  rs485 = nullptr; // RS485 interface was in the vector, already deleted
#endif
#ifdef ENABLE_MODBUS_MIRROR
  delete modbusMirror_0;
#endif
#ifdef ENABLE_INFLUXDB
  delete influxDb_0;
#endif
#ifdef ENABLE_IFTTT
  delete iftttWebhook;
#endif
  L_INFO("PHApp::cleanupComponents - Cleanup complete.");
}
PHApp::~PHApp() { cleanupComponents(); }
short PHApp::setAppState(short newState) {
  if (_state != newState) {
    _state = (APP_STATE)newState;
  }
  return E_OK;
}

#ifdef ENABLE_SETTINGS
short PHApp::loadAppSettings() {
  if (!appSettings->load("/settings.json")) {
    L_WARN("Failed to load settings, using defaults.");
  }

  // Initialize App Settings
  setAppWarmup(appSettings->get("ALWAYS_WARMUP", true), false);
  setAppPidLag(appSettings->get("PID_LAG_COMPENSATION", true), false);

#if defined(ENABLE_OMRON_E5) && defined(ENABLE_RS485)
  if (!rs485) {
    L_ERROR("RS485 not initialized, cannot apply settings to Omron devices.");
    return E_NOT_INITIALIZED;
  }

  RTU_Base *const *devices = rs485->deviceManager.getDevices();
  int numDevicesInManager = rs485->deviceManager.getMaxDevices();

  for (int i = 0; i < numDevicesInManager; ++i) {
    RTU_Base *device = devices[i];
    if (device == nullptr ||
        device->type != COMPONENT_TYPE::COMPONENT_TYPE_PID) {
      continue;
    }

    bool foundInSettings = false;
    bool shouldBeEnabled = false;

    for (uint8_t p = 0; p < appSettings->partitionCount; ++p) {
      PartitionConfig &partition = appSettings->partitions[p];
      for (uint8_t c = 0; c < partition.controllerCount; ++c) {
        ControllerConfig &controller = partition.controllers[c];
        if (device->slaveId == controller.slaveId) {
          foundInSettings = true;
          shouldBeEnabled = controller.enabled;
          break;
        }
      }
      if (foundInSettings) {
        break;
      }
    }

    if (foundInSettings) {
      if (shouldBeEnabled) {
        device->enable();
      } else {
        device->disable();
      }
    } else {
      device->disable();
    }
  }
#endif
  return E_OK;
}
#endif

#ifdef ENABLE_PID
short PHApp::getPid2Register(short offset, short unused) {
  if (!pidController_0) {
    L_ERROR("Serial Command Error: PID Controller 2 not initialized.");
    return E_INVALID_PARAMETER; // Use defined error code
  }
  if (offset < 0 || offset >= PID_2_REGISTER_COUNT) {
    L_ERROR("Serial Command Error: Invalid PID2 offset %d.", offset);
    return E_INVALID_PARAMETER;
  }

  short address = MB_HREG_PID_2_BASE_ADDRESS + offset;
  short value = pidController_0->mb_tcp_read(address);
  Log.noticeln("PID2 Register Offset %d (Addr %d) Value: %d", offset, address,
               value);
  // Optionally send value back over serial if needed by the protocol
  return E_OK;
}

short PHApp::setPid2Register(short offset, short value) {
  if (!pidController_0) {
    L_ERROR("Serial Command Error: PID Controller 2 not initialized.");
    return E_INVALID_PARAMETER; // Use defined error code
  }
  if (offset < 0 || offset >= PID_2_REGISTER_COUNT) {
    L_ERROR("Serial Command Error: Invalid PID2 offset %d.", offset);
    return E_INVALID_PARAMETER;
  }

  short address = MB_HREG_PID_2_BASE_ADDRESS + offset;
  short result = pidController_0->mb_tcp_write(address, value);

  if (result == E_OK) {
    Log.noticeln("PID2 Register Offset %d (Addr %d) set to: %d", offset,
                 address, value);
  } else {
    L_ERROR("PID2 Register Offset %d (Addr %d) failed to set to %d. Error: %d",
            offset, address, value, result);
  }
  return result;
}

#endif

short PHApp::onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user,
                       Component *src) {
#if defined(ENABLE_INFLUXDB) && defined(ENABLE_OMRON_E5)
  // Intercept messages from Omron PID controllers to log their data.
  if (verb == E_CALLS::EC_USER && src &&
      src->type == COMPONENT_TYPE::COMPONENT_TYPE_PID) {
    // Log.infoln("PHApp::onMessage - OmronE5");
    //  We know it's an OmronE5, so we can safely cast it.
    OmronE5 *omron = static_cast<OmronE5 *>(src);
    uint16_t sp, pv;

    // Ensure both SP and PV are valid before logging
    if (omron->getSP(sp) && omron->getPV(pv)) {
      Point p("pid_controller"); // Create a local Point
      p.addTag("slaveId", String(omron->slaveId));
      p.addField("sp", sp);
      p.addField("pv", pv);
      p.addField("isHeating", omron->isHeating());
      influxDb_0->write(p);
    } else {
      Log.errorln("PHApp::onMessage - OmronE5 - SP or PV not valid");
    }
  }
#endif

#if ENABLED(ENABLE_RS485, ENABLE_WEBSERVER, ENABLE_WEBSOCKET)
  if (verb == E_CALLS::EC_USER && user != nullptr && webServer != nullptr) {
    MB_UpdateData *update = static_cast<MB_UpdateData *>(user);
    return webServer->onMessage(id, E_CALLS::EC_USER, E_MessageFlags::E_MF_NONE,
                                user, src);
  }
  if (verb == E_CALLS::EC_PROTOBUF_UPDATE && user != nullptr) {
    PB_UpdateData *update = static_cast<PB_UpdateData *>(user);
    return webServer->onMessage(id, E_CALLS::EC_PROTOBUF_UPDATE,
                                E_MessageFlags::E_MF_NONE, user, src);
  }
#endif
  if (verb == E_CALLS::EC_DISPATCH && user != nullptr) {
    JsonDocument *doc = static_cast<JsonDocument *>(user);
    this->broadcast(BROADCAST_ERROR_MESSAGE, *doc);
    return E_OK;
  }
  return App::onMessage(id, verb, flags, user, src);
}

/**
 * @brief Retrieves a component by its ID.
 *
 * @param id The ID of the component to retrieve.
 * @return A pointer to the component with the specified ID, or nullptr if not
 * found.
 * @note Top-Level PHApp cant be part of components vector, so we need to handle
 * it separately.
 */
Component *PHApp::byId(ushort id) {
  Component *comp = App::byId(id);
  if (comp) {
    return comp;
  } else if (id == COMPONENT_KEY_APP) {
    return this;
  }
  return nullptr;
}

short PHApp::init(short arg1, short arg2) {

#if ENABLED(ENABLE_AMPERAGE_BUDGET_MANAGER, ENABLE_OMRON_E5)
  RTU_Base *const *devices = rs485->deviceManager.getDevices();
  int numDevicesInManager = rs485->deviceManager.getMaxDevices();
  for (int i = 0; i < numDevicesInManager; ++i) {
    if (devices[i] != nullptr) {
      Component *comp = devices[i];
      if (comp == nullptr || comp->type != COMPONENT_TYPE::COMPONENT_TYPE_PID) {
        continue;
      }

      OmronE5 *omronE5 = static_cast<OmronE5 *>(comp);
      omronE5->stop();
      omronE5->setSP(0);
    }
  }
#endif
  return E_OK;
}

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
void PHApp::startSignalPlot(short slotId) {
  if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT &&
      signalPlots[slotId] != nullptr) {
    L_INFO("PHApp: Starting SignalPlot in slot %d (triggered by "
           "TemperatureProfile).",
           slotId);
    signalPlots[slotId]->start();
  } else {
    Log.warningln("PHApp: Could not start SignalPlot. Invalid slotId %d or "
                  "plot not initialized.",
                  slotId);
  }
}

void PHApp::stopSignalPlot(short slotId) {
  if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT &&
      signalPlots[slotId] != nullptr) {
    L_INFO("PHApp: Stopping SignalPlot in slot %d (triggered by "
           "TemperatureProfile).",
           slotId);
    signalPlots[slotId]->stop();
  } else {
    Log.warningln("PHApp: Could not stop SignalPlot. Invalid slotId %d or plot "
                  "not initialized.",
                  slotId);
  }
}

void PHApp::enableSignalPlot(short slotId, bool enable) {
  if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT &&
      signalPlots[slotId] != nullptr) {
    if (enable) {
      L_INFO("PHApp: Enabling SignalPlot in slot %d (triggered by "
             "TemperatureProfile).",
             slotId);
      signalPlots[slotId]->enable(enable);
    } else {
      L_INFO("PHApp: Disabling SignalPlot in slot %d (triggered by "
             "TemperatureProfile).",
             slotId);
      signalPlots[slotId]->disable();
    }
  } else {
    Log.warningln("PHApp: Could not enable/disable SignalPlot. Invalid slotId "
                  "%d or plot not initialized.",
                  slotId);
  }
}

void PHApp::pauseSignalPlot(short slotId) {
  if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT &&
      signalPlots[slotId] != nullptr) {
    L_INFO("PHApp: Pausing SignalPlot in slot %d (triggered by "
           "TemperatureProfile).",
           slotId);
    signalPlots[slotId]->pause();
  } else {
    Log.warningln("PHApp: Could not pause SignalPlot. Invalid slotId %d or "
                  "plot not initialized.",
                  slotId);
  }
}

void PHApp::resumeSignalPlot(short slotId) {
  if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT &&
      signalPlots[slotId] != nullptr) {
    L_INFO("PHApp: Resuming SignalPlot in slot %d (triggered by "
           "TemperatureProfile).",
           slotId);
    signalPlots[slotId]->resume();
  } else {
    Log.warningln("PHApp: Could not resume SignalPlot. Invalid slotId %d or "
                  "plot not initialized.",
                  slotId);
  }
}

#endif // ENABLE_PROFILE_SIGNAL_PLOT

#ifdef ENABLE_INFLUXDB
void PHApp::testInfluxDB() {
  if (influxDb_0 && (millis() - _last_influx_test_ms >= 5000)) {
    _last_influx_test_ms = millis();

    Point p("test_data"); // Create a local Point
    p.addTag("device", "ESP32-S3");
    p.addTag("source", "1");

    p.addField("random_value", random(100));
    p.addField("random_value2", random(100));
    p.addField("on-off-1", random(2));
    p.addField("on-off-2", random(2));

    if (influxDb_0->write(p) == E_OK) {
      Log.infoln("PHApp: Sent test data to InfluxDB.");
    }
  }
}
#endif

bool PHApp::isSlave() const { return _is_slave; }

short PHApp::setSlave(bool value, bool sync) {
  _is_slave = value;

  bool enabled = !value; // When slave=true, disable components; when
                         // slave=false, enable them

#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)
  if (rs485) {
    RTU_Base *const *devices = rs485->deviceManager.getDevices();
    int numDevices = rs485->deviceManager.getMaxDevices();
    for (int i = 0; i < numDevices; ++i) {
      if (devices[i] != nullptr &&
          devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID) {
        OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
        if (omron) {
          omron->enable(enabled);
        }
      }
    }
  }
#endif

#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
  if (pidManagerAmperage) {
    pidManagerAmperage->enable(enabled);
  }
#endif

#ifdef ENABLE_LOADCELL_0
  if (loadCell_0) {
    loadCell_0->enable(enabled);
  }
#endif

#ifdef ENABLE_LOADCELL_1
  if (loadCell_1) {
    loadCell_1->enable(enabled);
  }
#endif

#ifdef ENABLE_PROFILE_TEMPERATURE
  for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i) {
    if (tempProfiles[i] != nullptr) {
      tempProfiles[i]->enable(enabled);
    }
  }
#endif

  if (sync) {
    updateRuntimeState();
  }
  return E_OK;
}

bool PHApp::getAllOmronStop() const { return _all_omron_stop; }

short PHApp::setAllOmronStop(bool value, bool sync) {
  _all_omron_stop = value;

#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)
  if (!rs485) {
    L_INFO("PHApp: No RS485 found");
    return E_OK;
  }

  RTU_Base *const *devices = rs485->deviceManager.getDevices();
  int numDevices = rs485->deviceManager.getMaxDevices();
  for (int i = 0; i < numDevices; ++i) {
    if (devices[i] != nullptr &&
        devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID) {
      OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
      if (omron && omron->enabled()) {
        if (value) {
          omron->stop();
        } else {
          omron->run();
        }
      }
    }
  }

  updateRuntimeState();

#endif

  return E_OK;
}

bool PHApp::getAllOmronComWrite() const { return _all_omron_com_write; }

short PHApp::setAllOmronComWrite(bool value, bool sync) {
  _all_omron_com_write = value;

#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)
  if (!rs485) {
    L_INFO("PHApp: No RS485 found");
    return E_OK;
  }

  RTU_Base *const *devices = rs485->deviceManager.getDevices();
  int numDevices = rs485->deviceManager.getMaxDevices();
  for (int i = 0; i < numDevices; ++i) {
    if (devices[i] != nullptr &&
        devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID) {
      OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
      if (omron && omron->enabled()) {
        omron->setCommsWriting(value);
      }
    }
  }
#endif
  if (sync) {
    updateRuntimeState();
  }
  return E_OK;
}

short PHApp::setAppWarmup(bool value, bool sync) {
  _app_warmup = value;
  if (appSettings) {
    appSettings->set("heating", "ALWAYS_WARMUP", value);
    appSettings->save();
  }
  if (sync) {
    updateRuntimeState();
  }
  return E_OK;
}

short PHApp::setAppPidLag(bool value, bool sync) {
  _app_pid_lag = value;
  if (appSettings) {
    appSettings->set("heating", "PID_LAG_COMPENSATION", value);
    appSettings->save();
  }
  if (sync) {
    updateRuntimeState();
  }
  return E_OK;
}

void app_main() {
  // Arduino will still call setup()/loop()
}
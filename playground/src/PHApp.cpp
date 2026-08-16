#include <Arduino.h>
#include <macros.h>
#include <Component.h>
#include <enums.h>
#include <ArduinoLog.h>

#include "./PHApp.h"
#include "./config.h"
#include "./config_adv.h"
#include "./config-modbus.h"
#include "./features.h"


#include <components/OmronE5Types.h>
#include <components/OmronE5.h>

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
#include <profiles/SignalPlot.h>
#endif

#ifdef ENABLE_MODBUS_TCP
#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>
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
#define LOG_LEVEL LOG_LEVEL_NONE
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
#define ADD_RELAY(relayNum, relayPin, relayKey, relayAddr)           \
  relay_##relayNum = new Relay(this, relayPin, relayKey, relayAddr); \
  components.push_back(relay_##relayNum);

#define ADD_POT(potNum, potPin, potKey, potAddr)         \
  pot_##potNum = new POT(this, potPin, potKey, potAddr); \
  components.push_back(pot_##potNum);

#define ADD_POS3ANALOG(posNum, switchPin1, switchPin2, switchKey, switchAddr)                \
  pos3Analog_##posNum = new Pos3Analog(this, switchPin1, switchPin2, switchKey, switchAddr); \
  components.push_back(pos3Analog_##posNum);

#ifdef ENABLE_PID
#define ADD_PID(pidNum, nameStr, doPin, csPin, clkPin, outPin, key)                       \
  pidController_##pidNum = new PIDController(key, nameStr, doPin, csPin, clkPin, outPin); \
  components.push_back(pidController_##pidNum);
#endif

void PHApp::printRegisters()
{
  Log.verboseln(F("--- Entering PHApp::printRegisters ---"));

#if ENABLED(HAS_MODBUS_REGISTER_DESCRIPTIONS)
  Log.setShowLevel(false);
  Serial.print("| Name | ID  | Address | RW |  Function Code | Number Addresses |Register Description| \n");
  Serial.print("|------|----------|----|----|----|----|-------|\n");
  short size = components.size();
  Log.verboseln(F("PHApp::printRegisters - Processing %d components..."), size);
  for (int i = 0; i < size; i++)
  {
    Component *component = components[i];
    if (!component)
    {
      Log.errorln(F("PHApp::printRegisters - Found NULL component at index %d"), i);
      continue;
    }
    Log.verboseln(F("PHApp::printRegisters - Component %d: ID=%d, Name=%s"), i, component->id, component->name.c_str());
    // if (!(component->nFlags & 1 << OBJECT_NET_CAPS::E_NCAPS_MODBUS)) // <-- Modbus flag check might be different now
    // {
    //   continue;
    // }
    // Log.verbose("| %s | %d | %d | %s | %d | %d | %s |\n", // <-- Calls to removed ModbusGateway methods
    //             component->name.c_str(),
    //             component->id,
    //             component->getAddress(),
    //             component->getRegisterMode(),
    //             component->getFunctionCode(),
    //             component->getNumberAddresses(),
    //             component->getRegisterDescription().c_str());
    Log.verbose("| %s | %d | - | - | - | - | - |\n", // <-- Simplified output
                component->name.c_str(),
                component->id);
  }
  Log.setShowLevel(true);
#endif
  Log.verboseln(F("--- Exiting PHApp::printRegisters ---"));
}
short PHApp::reset(short val0, short val1)
{
  _state = APP_STATE::RESET;
  _error = E_OK;

#if defined(ESP32) || defined(ESP8266) // Use ESP.restart() for ESP32 and ESP8266
  ESP.restart();
#else
  return E_NOT_IMPLEMENTED;
#endif
  return E_OK;
}
short PHApp::list(short val0, short val1)
{
  uchar s = components.size();
  for (uchar i = 0; i < s; i++)
  {
    Component *component = components[i];
    if (component)
    {
      Log.verboseln("PHApp::list - %d | %s (ID: %d)", i, component->name.c_str(), component->id);
    }
    else
    {
      Log.warningln("PHApp::list - NULL component at index %d", i);
    }
  }
  return E_OK;
}
short PHApp::setup()
{
  Log.verbose("--------------------PHApp::setup() Begin.-------------------");
  _state = APP_STATE::RESET;
  _error = E_OK;
#ifdef ENABLE_PROFILER
  if (initialFreeHeap == 0 && initialCpuTicks == 0)
  {
    initialFreeHeap = ESP.getFreeHeap();
    initialCpuTicks = esp_cpu_get_ccount();
  }
#endif
#ifndef DISABLE_SERIAL_LOGGING
  // Serial Setup
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial && !Serial.available())
  {
  }
  // Log Setup
  Log.begin(LOG_LEVEL, &Serial);
  Log.setShowLevel(true);
#else
  // Log Setup (without Serial)
  Log.begin(LOG_LEVEL_WARNING, nullptr); // Or LOG_LEVEL_NONE, or some other target if available
  Log.setShowLevel(false);
#endif

  // Components
  bridge = new Bridge(this);
#ifndef DISABLE_SERIAL_LOGGING
  com_serial = new SerialMessage(Serial, bridge);
  components.push_back(com_serial);
#endif
  components.push_back(bridge);
  // Network
  short networkSetupResult = setupNetwork();
  if (networkSetupResult != E_OK)
  {
    Log.errorln("Network setup failed with error code: %d", networkSetupResult);
  }

  // Components
#ifdef ENABLE_RELAYS
#ifdef AUX_RELAY_0
  ADD_RELAY(0, AUX_RELAY_0, COMPONENT_KEY_RELAY_0, MB_ADDR_AUX_5);
#endif
#ifdef AUX_RELAY_1
  ADD_RELAY(1, AUX_RELAY_1, COMPONENT_KEY_RELAY_1, MB_ADDR_AUX_6);
#endif
#endif

#ifdef MB_ANALOG_0
  ADD_POT(0, MB_ANALOG_0, COMPONENT_KEY_ANALOG_0, MB_ADDR_AUX_2);
#endif

#ifdef MB_ANALOG_1
  ADD_POT(1, MB_ANALOG_1, COMPONENT_KEY_ANALOG_1, MB_ADDR_AUX_3);
#endif

#if (defined(AUX_ANALOG_3POS_SWITCH_0) && (defined(AUX_ANALOG_3POS_SWITCH_1)))
  ADD_POS3ANALOG(0, AUX_ANALOG_3POS_SWITCH_0, AUX_ANALOG_3POS_SWITCH_1, COMPONENT_KEY_MB_ANALOG_3POS_SWITCH_0, MB_ADDR_AUX_3);
#endif

#if (defined(MB_ANALOG_3POS_SWITCH_2) && (defined(MB_ANALOG_3POS_SWITCH_3)))
  // ADD_POS3ANALOG(1, MB_ANALOG_3POS_SWITCH_2, MB_ANALOG_3POS_SWITCH_3, COMPONENT_KEY_MB_ANALOG_3POS_SWITCH_1, MB_R_SWITCH_1); // <-- Temporarily disable
#endif

#ifdef MB_GPIO_MB_MAP_7
  // --- Define configuration for the MB_GPIO group ---
  std::vector<GPIO_PinConfig> gpioConfigs;
  gpioConfigs.reserve(2); // Reserve space for 2 elements
  gpioConfigs.push_back(
      GPIO_PinConfig(
          E_GPIO_7,                         // pinNumber: The physical pin to manage
          E_GPIO_TYPE_ANALOG_INPUT,         // pinType: Treat as analog input
          300,                              // startAddress: Modbus register address
          E_FN_CODE::FN_READ_HOLD_REGISTER, // type: Map to a Holding Register
          MB_ACCESS_READ_ONLY,              // access: Allow Modbus read only
          1000,                             // opIntervalMs: Update interval in milliseconds
          "GPIO_6",                         // name: Custom name for this pin
          "GPIO_Group"                      // group: Group  name for this pin
          ));

  gpioConfigs.push_back(
      GPIO_PinConfig(
          E_GPIO_15,                        // pinNumber: The physical pin to manage
          E_GPIO_TYPE_ANALOG_INPUT,         // pinType: Treat as analog input
          301,                              // startAddress: Modbus register address
          E_FN_CODE::FN_READ_HOLD_REGISTER, // type: Map to a Holding Register
          MB_ACCESS_READ_ONLY,              // access: Allow Modbus read only
          1000,                             // opIntervalMs: Update interval in milliseconds
          "GPIO_15",                        // name: Custom name for this pin
          "GPIO_Group"                      // group: Group name for this pin
          ));
  const short gpioGroupId = COMPONENT_KEY_GPIO_MAP; // Using defined key
  gpio_0 = new MB_GPIO(this, gpioGroupId, gpioConfigs);
  components.push_back(gpio_0);
#endif

#ifdef PIN_ANALOG_LEVEL_SWITCH_0
  analogLevelSwitch_0 = new AnalogLevelSwitch(
      this,                      // owner
      PIN_ANALOG_LEVEL_SWITCH_0, // analogPin
      ALS_0_NUM_LEVELS,          // numLevels
      ALS_0_ADC_STEP,            // levelStep
      ALS_0_ADC_OFFSET,          // adcValueOffset
      ID_ANALOG_LEVEL_SWITCH_0,  // id
      ALS_0_MB_ADDR              // modbusAddress
  );
  if (analogLevelSwitch_0)
  {
    components.push_back(analogLevelSwitch_0);
    Log.infoln(F("AnalogLevelSwitch_0 initialized. Pin:%d, ID:%d, Levels:%d, Step:%d, Offset:%d, Smooth:%d(Fixed), Debounce:%d(Fixed), MB:%d"),
               PIN_ANALOG_LEVEL_SWITCH_0, ID_ANALOG_LEVEL_SWITCH_0, ALS_0_NUM_LEVELS,
               ALS_0_ADC_STEP, ALS_0_ADC_OFFSET,
               ALS_SMOOTHING_SIZE,
               ALS_DEBOUNCE_COUNT,
               ALS_0_MB_ADDR);
  }
  else
  {
    Log.errorln(F("AnalogLevelSwitch_0 initialization failed."));
  }
#endif

#ifdef ENABLE_STATUS
  statusLight_0 = new StatusLight(this,
                                  STATUS_WARNING_PIN,
                                  COMPONENT_KEY_FEEDBACK_0,
                                  MB_MONITORING_STATUS_FEEDBACK_0); // Keep original address for now, add to config-modbus.h later if needed
  components.push_back(statusLight_0);
  statusLight_1 = new StatusLight(this,
                                  STATUS_ERROR_PIN,
                                  COMPONENT_KEY_FEEDBACK_1,
                                  MB_MONITORING_STATUS_FEEDBACK_1); // Keep original address for now
  components.push_back(statusLight_1);
#else
  statusLight_0 = NULL;
  statusLight_1 = NULL;
#endif
  Log.infoln("PHApp::setup - Base App::setup() called.");

#ifdef PIN_LED_FEEDBACK_0
  ledFeedback_0 = new LEDFeedback(
      this,                  // owner
      PIN_LED_FEEDBACK_0,    // pin
      LED_PIXEL_COUNT_0,     // pixelCount
      ID_LED_FEEDBACK_0,     // id
      LED_FEEDBACK_0_MB_ADDR // modbusAddress
  );
  if (ledFeedback_0)
  {
    components.push_back(ledFeedback_0);
    Log.infoln(F("LEDFeedback_0 initialized. Pin:%d, Count:%d, ID:%d, MB:%d"),
               PIN_LED_FEEDBACK_0, LED_PIXEL_COUNT_0,
               ID_LED_FEEDBACK_0, LED_FEEDBACK_0_MB_ADDR);
  }
  else
  {
    Log.errorln(F("LEDFeedback_0 initialization failed."));
  }
#endif

#ifdef ENABLE_JOYSTICK
  joystick_0 = new Joystick(
      this,               // owner
      PIN_JOYSTICK_UP,    // UP pin
      PIN_JOYSTICK_DOWN,  // DOWN pin
      PIN_JOYSTICK_LEFT,  // LEFT pin
      PIN_JOYSTICK_RIGHT, // RIGHT pin
      MB_ADDR_AUX_7       // modbusAddress
  );
  if (joystick_0)
  {
    components.push_back(joystick_0);
  }
  else
  {
    Log.errorln(F("Joystick_0 initialization failed."));
  }
#endif

#ifdef ENABLE_FEEDBACK_3C
  feedback3C_0 = new Feedback3C(this,
                                GPIO_PIN_CH1,
                                GPIO_PIN_CH2,
                                GPIO_PIN_CH3,
                                COMPONENT_KEY_FEEDBACK_0,
                                MB_ADDR_AUX_6);
  components.push_back(feedback3C_0);
#endif

#ifdef ENABLE_OPERATOR_SWITCH
  operatorSwitch_0 = new OperatorSwitch(this,
                                        PIN_OPERATOR_SWITCH_STOP,
                                        PIN_OPERATOR_SWITCH_CYCLE,
                                        COMPONENT_KEY_OPERATOR_SWITCH,
                                        MB_ADDR_AUX_7);
  components.push_back(operatorSwitch_0);
#endif

#ifdef ENABLE_NETWORK_VALUE_TEST
  if(true) { // TODO: add settings flag
    Log.infoln("PHApp::setup - Initializing NetworkValueTest...");
    networkValueTest = new NetworkValueTest(this, COMPONENT_KEY_TEST_NV, MB_ADDR_AUX_TEST_NV);
    components.push_back(networkValueTest);
  }
#endif

  // Motors
#ifdef ENABLE_SAKO_VFD
  vfd_0 = new SAKO_VFD(MB_SAKO_VFD_SLAVE_ID, MB_SAKO_VFD_READ_INTERVAL);
  components.push_back(vfd_0);
#endif
  // Temperature
#ifdef ENABLE_PROFILE_TEMPERATURE
  for (ushort i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
  {
    // Assign unique ID: COMPONENT_KEY_PROFILE_START (910) + slot index
    ushort profileComponentId = COMPONENT_KEY_PROFILE_START + i;
    tempProfiles[i] = new TemperatureProfile(this, i, profileComponentId);
    components.push_back(tempProfiles[i]);
    Log.infoln("PHApp::setup - Initialized TemperatureProfile Slot %d with Component ID %d", i, profileComponentId);
  }
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
  for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; i++) {
    ushort profileComponentId = COMPONENT_KEY_SIGNAL_PLOT_START + i;
    signalPlots[i] = new SignalPlot(this, i, profileComponentId);
    components.push_back(signalPlots[i]);
    Log.infoln("PHApp::setup - Initialized SignalPlot Slot %d with Component ID %d", i, profileComponentId);
  }
#endif


#ifdef ENABLE_PID
  const int8_t PID2_THERMO_DO = 19;  // Example MISO pin
  const int8_t PID2_THERMO_CS = 5;   // Example Chip Select pin
  const int8_t PID2_THERMO_CLK = 18; // Example SCK pin
  const int8_t PID2_OUTPUT_PIN = 23; // Example PWM/Output pin
  ADD_PID(0, "PID Temp Controller 2", PID2_THERMO_DO, PID2_THERMO_CS, PID2_THERMO_CLK, PID2_OUTPUT_PIN, COMPONENT_KEY_PID_2);
#endif
// RS485
#ifdef ENABLE_RS485
  rs485 = new RS485(this);
  components.push_back(rs485);
#endif
#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
  pidManagerAmperage = new AmperageBudgetManager(this);
  components.push_back(pidManagerAmperage);
#endif

// Systems : Extruder
#ifdef ENABLE_EXTRUDER
  extruder_0 = new Extruder(this, vfd_0, nullptr, nullptr, nullptr);
  components.push_back(extruder_0);
#endif

#ifdef ENABLE_PLUNGER
  plunger_0 = new Plunger(this, vfd_0, joystick_0, pot_0, pot_1);
  components.push_back(plunger_0);
#endif

  // Application stuff
  registerComponents(bridge);
  serial_register(bridge);
  App::setup();
  onRun();
  return E_OK;
}

short PHApp::onRun()
{
  App::onRun();

#ifdef ENABLE_MODBUS_TCP
  for (Component *comp : components)
  {
    if (comp && comp->hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS))
    {
      comp->mb_tcp_register(modbusManager);
    }
  }
#endif

#ifdef ENABLE_PROFILE_TEMPERATURE
  load(0, 0);
#endif

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
  for (int i = 0; i < numDevicesInManager; ++i)
  {
    if (devices[i] != nullptr)
    {
      Component *comp = devices[i];
      if (comp == nullptr || comp->type != COMPONENT_TYPE::COMPONENT_TYPE_PID)
      {
        continue;
      }
      
      if (!pidManagerAmperage->addManagedDevice(static_cast<OmronE5 *>(comp)))
      {
        Log.errorln("Failed to add OmronE5 device to AmperageBudgetManager");
      }
      
    }
  }
#endif


#if ENABLED(ENABLE_TEMPERATURE_PROFILES, ENABLE_OMRON_E5, ENABLE_MODBUS_TCP)
  tempProfiles[0]->disable();
  if (tempProfiles[0] && rs485)
  {
    RTU_Base *const *devices = rs485->deviceManager.getDevices();
    int numDevicesInManager = rs485->deviceManager.getMaxDevices();
    int targetRegisterIndex = 0; // Dedicated index for _targetRegisters
    for (int i = 0; i < numDevicesInManager; i++)
    {
      Component *comp = devices[i];
      if (comp == nullptr || comp->type != COMPONENT_TYPE::COMPONENT_TYPE_PID)
      {
        continue;
      }
      // Ensure we don't write out of bounds for _targetRegisters
      /*
      if (targetRegisterIndex < TEMP_PROFILE_MAX_TARGET_REGS)
      {
        uint16_t spCmdAddr = comp->mb_tcp_base_address() + static_cast<uint16_t>(E_OmronTcpOffset::CMD_SP);
        tempProfiles[0]->setTargetRegister(targetRegisterIndex, spCmdAddr);
        targetRegisterIndex++;
      }
      else
      {
        Log.warningln("Max target registers (%d) reached. Cannot set for Omron device (Slave ID: %d)", TEMP_PROFILE_MAX_TARGET_REGS, comp->slaveId);
      }
      */
    }
  }
#endif

#ifdef ENABLE_NETWORK_VALUE_TEST
  // networkValueTest->init();
#endif

  return E_OK;
}
short PHApp::serial_register(Bridge *bridge)
{
  bridge->registerMemberFunction(COMPONENT_KEY_BASE::COMPONENT_KEY_APP, this, C_STR("list"), (ComponentFnPtr)&PHApp::list);
  bridge->registerMemberFunction(COMPONENT_KEY_BASE::COMPONENT_KEY_APP, this, C_STR("reset"), (ComponentFnPtr)&PHApp::reset);
  bridge->registerMemberFunction(COMPONENT_KEY_BASE::COMPONENT_KEY_APP, this, C_STR("printRegisters"), (ComponentFnPtr)&PHApp::printRegisters);
  bridge->registerMemberFunction(COMPONENT_KEY_BASE::COMPONENT_KEY_APP, this, C_STR("load"), (ComponentFnPtr)&PHApp::load);
  return E_OK;
}
short PHApp::onWarning(short code)
{
  return E_OK;
}
short PHApp::onError(short id, short code)
{
  if (code == getLastError())
  {
    return code;
  }
  Log.error(F("* App:onError - component=%d code=%d" CR), id, code);
  setLastError(code);
#ifdef ENABLE_STATUS
  if (statusLight_0)
  {
    switch (id)
    {
    case COMPONENT_KEY_PLUNGER:
    {
      switch (code)
      {

#if defined(ENABLE_PLUNGER)
      case (short)PlungerState::IDLE:
      {
        statusLight_1->set(0, 0);
        break;
      }
      case (short)PlungerState::JAMMED:
      {
        statusLight_1->set(1, 1);
        break;
      }
      default:
      {
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

short PHApp::onStop(short val)
{
  return E_OK;
}
short PHApp::clearError()
{
  setLastError(E_OK);
  return E_OK;
}
short PHApp::loop()
{
  _loop_start_time_us = micros();
  App::loop();

  if (millis() - _last_cycle_loop_ms >= LOOP_CYCLE_INTVAL) {
    _last_cycle_loop_ms = millis();
    loopCycle();
  }

#ifdef ENABLE_WEBSERVER
  loopWeb();
#endif
#ifdef ENABLE_MODBUS_TCP
  loopModbus();
#endif
  _loop_duration_us = micros() - _loop_start_time_us;
  return E_OK;
}
short PHApp::loopWeb()
{
#if defined(ENABLE_WIFI) && defined(ENABLE_WEBSERVER)
  if (webServer != nullptr)
  {
    webServer->loop();
  }
#endif
  return E_OK;
}
short PHApp::getAppState(short val)
{
  return _state;
}
PHApp::PHApp() : App()
{
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
  feedback3C_0 = nullptr;
  operatorSwitch_0 = nullptr;
  _last_cycle_loop_ms = 0;
#ifdef ENABLE_PROFILE_TEMPERATURE
  // Initialize the array elements to nullptr
  for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
  {
    tempProfiles[i] = nullptr;
  }
#endif

  // WiFi settings are now initialized by WiFiNetworkSettings constructor
}
void PHApp::cleanupComponents()
{
  Log.infoln("PHApp::cleanupComponents - Cleaning up %d components...", components.size());
  for (Component *comp : components)
  {
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
  feedback3C_0 = nullptr;
  operatorSwitch_0 = nullptr;
#ifdef ENABLE_PROFILE_TEMPERATURE
  // Clean up temperature profiles (they were also added to components vector, so already deleted there)
  // Ensure the pointers in the array are nulled
  for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
  {
    tempProfiles[i] = nullptr;
  }
#endif
#ifdef ENABLE_WEBSERVER
  if (webServer)
  {
    delete webServer;
    webServer = nullptr;
  }
#endif
#ifdef ENABLE_RS485
  rs485 = nullptr; // RS485 interface was in the vector, already deleted
#endif
  Log.infoln("PHApp::cleanupComponents - Cleanup complete.");
}
PHApp::~PHApp()
{
  cleanupComponents();
}
short PHApp::setAppState(short newState)
{
  if (_state != newState)
  {
    _state = (APP_STATE)newState;
  }
  return E_OK;
}

#ifdef ENABLE_PID
short PHApp::getPid2Register(short offset, short unused)
{
  if (!pidController_0)
  {
    Log.errorln("Serial Command Error: PID Controller 2 not initialized.");
    return E_INVALID_PARAMETER; // Use defined error code
  }
  if (offset < 0 || offset >= PID_2_REGISTER_COUNT)
  {
    Log.errorln("Serial Command Error: Invalid PID2 offset %d.", offset);
    return E_INVALID_PARAMETER;
  }

  short address = MB_HREG_PID_2_BASE_ADDRESS + offset;
  short value = pidController_0->mb_tcp_read(address);
  Log.noticeln("PID2 Register Offset %d (Addr %d) Value: %d", offset, address, value);
  // Optionally send value back over serial if needed by the protocol
  return E_OK;
}

short PHApp::setPid2Register(short offset, short value)
{
  if (!pidController_0)
  {
    Log.errorln("Serial Command Error: PID Controller 2 not initialized.");
    return E_INVALID_PARAMETER; // Use defined error code
  }
  if (offset < 0 || offset >= PID_2_REGISTER_COUNT)
  {
    Log.errorln("Serial Command Error: Invalid PID2 offset %d.", offset);
    return E_INVALID_PARAMETER;
  }

  short address = MB_HREG_PID_2_BASE_ADDRESS + offset;
  short result = pidController_0->mb_tcp_write(address, value);

  if (result == E_OK)
  {
    Log.noticeln("PID2 Register Offset %d (Addr %d) set to: %d", offset, address, value);
  }
  else
  {
    Log.errorln("PID2 Register Offset %d (Addr %d) failed to set to %d. Error: %d", offset, address, value, result);
  }
  return result;
}

#endif

short PHApp::onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user, Component *src)
{
#if ENABLED(ENABLE_RS485, ENABLE_WEBSERVER, ENABLE_WEBSOCKET)
  if (verb == E_CALLS::EC_USER && user != nullptr && webServer != nullptr)
  {
    return webServer->onMessage(id, E_CALLS::EC_USER, E_MessageFlags::E_MF_NONE, user, this);
  }
#endif
  return App::onMessage(id, verb, flags, user, src);
}

/**
 * @brief Retrieves a component by its ID.
 *
 * @param id The ID of the component to retrieve.
 * @return A pointer to the component with the specified ID, or nullptr if not found.
 * @note Top-Level PHApp cant be part of components vector, so we need to handle it separately.
 */
Component *PHApp::byId(ushort id)
{
  Component *comp = App::byId(id);
  if (comp)
  {
    return comp;
  }
  else if (id == COMPONENT_KEY_APP)
  {
    return this;
  }
  return nullptr;
}

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
void PHApp::startSignalPlot(short slotId)
{
    if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT && signalPlots[slotId] != nullptr)
    {
        Log.infoln("PHApp: Starting SignalPlot in slot %d (triggered by TemperatureProfile).", slotId);
        signalPlots[slotId]->start();
    }
    else
    {
        Log.warningln("PHApp: Could not start SignalPlot. Invalid slotId %d or plot not initialized.", slotId);
    }
}

void PHApp::stopSignalPlot(short slotId)
{
    if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT && signalPlots[slotId] != nullptr)
    {
        Log.infoln("PHApp: Stopping SignalPlot in slot %d (triggered by TemperatureProfile).", slotId);
        signalPlots[slotId]->stop();
    }
    else
    {
        Log.warningln("PHApp: Could not stop SignalPlot. Invalid slotId %d or plot not initialized.", slotId);
    }
}

void PHApp::enableSignalPlot(short slotId, bool enable)
{
    if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT && signalPlots[slotId] != nullptr)
    {
        if (enable)
        {
            Log.infoln("PHApp: Enabling SignalPlot in slot %d (triggered by TemperatureProfile).", slotId);
            signalPlots[slotId]->enable();
        }
        else
        {
            Log.infoln("PHApp: Disabling SignalPlot in slot %d (triggered by TemperatureProfile).", slotId);
            signalPlots[slotId]->disable();
        }
    }
    else
    {
        Log.warningln("PHApp: Could not enable/disable SignalPlot. Invalid slotId %d or plot not initialized.", slotId);
    }
}

void PHApp::pauseSignalPlot(short slotId)
{
    if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT && signalPlots[slotId] != nullptr)
    {
        Log.infoln("PHApp: Pausing SignalPlot in slot %d (triggered by TemperatureProfile).", slotId);
        signalPlots[slotId]->pause();
    }
    else
    {
        Log.warningln("PHApp: Could not pause SignalPlot. Invalid slotId %d or plot not initialized.", slotId);
    }
}

void PHApp::resumeSignalPlot(short slotId)
{
    if (slotId >= 0 && slotId < PROFILE_SIGNAL_PLOT_COUNT && signalPlots[slotId] != nullptr)
    {
        Log.infoln("PHApp: Resuming SignalPlot in slot %d (triggered by TemperatureProfile).", slotId);
        signalPlots[slotId]->resume();
    }
    else
    {
        Log.warningln("PHApp: Could not resume SignalPlot. Invalid slotId %d or plot not initialized.", slotId);
    }
}
#endif // ENABLE_PROFILE_SIGNAL_PLOT

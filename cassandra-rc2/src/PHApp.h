#ifndef PHAPP_H
#define PHAPP_H

#include "config.h"
#include "config-modbus.h"
#include "features.h"

#include <enums.h>
#include <Vector.h>
#include <vector>
#include <xmath.h>
#include <macros.h>
#include <App.h>
#include <Component.h>
#include <Bridge.h>
#include <SerialMessage.h>
#include <ArduinoLog.h>

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <xstatistics.h>

#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>

#include <profiles/SignalPlot.h>
#include <profiles/WiFiNetworkSettings.h>
#include <profiles/IPlotEvents.h>
#include <profiles/IPlungerEvents.h>
#include <profiles/PressureProfile.h>
#include <components/Logger.h>

class POT;
class Relay;
class RS485;
class Pos3Analog;
class StatusLight;
class RESTServer;
class PIDController;
class TemperatureProfile;
class PressureProfile;
class SignalPlot;
class SAKO_VFD;
class MB_GPIO;
class AnalogLevelSwitch;
class LEDFeedback;
class Extruder;
class Plunger;
class Joystick;
class PushButton;
class PHApp;
class AmperageBudgetManager;
class NetworkValueTest;
class NetworkValueTestPB;
class Feedback3C;
class FeedbackBuzzer;
class OperatorSwitch;
class ModbusMirror;
class AsyncWebServerRequest;
class Solenoid;
class PressCylinder;
class RuntimeState;
class Loadcell;
class InfluxDB;
class IFTTTWebhook;
class OmronE5;
class DELTA_VFD;
class Settings;

#if defined(ENABLE_AMPERAGE_BUDGET_MANAGER)
bool PHApp_canUsePID(Component *owner, OmronE5 *device);
void PHApp_onWarmupComplete(Component *owner);
#endif

class PHApp : public App, public IPlotEvents, public IPlungerEvents
{
public:
    //////////////////////////////////////////////////////////////
    // Enums
    //////////////////////////////////////////////////////////////
    enum CONTROLLER_STATE
    {
        E_CS_OK = 0,
        E_CS_ERROR = 10
    };

    enum APP_STATE
    {
        RESET = 0,
        EXTRUDING = 1,
        STANDBY = 2,
        ERROR = 10,
        PID_TIMEOUT = 11,
        FEED_TIMEOUT = 12,
        CONTROL_PANEL_INVALID = 13,
        PID_ERROR = 20,
        FEED_ERROR = 40,
    };

    enum COMMANDS
    {
        CMD_NONE = 1,
        CMD_RESET = 2,
        CMD_PRINT_REGS = 3,
        CMD_PRINT_SLAVES = 4,
        CMD_PRINT_QUEUE = 5,
        CMD_PRINT_BACKOFF = 6,
        CMD_PRINT_RESET = 7,
        CMD_PRINT_VFD = 8,
    };

    //////////////////////////////////////////////////////////////
    // Constructor / Destructor
    //////////////////////////////////////////////////////////////
    PHApp();
    ~PHApp() override;

    //////////////////////////////////////////////////////////////
    // Core Application Logic
    //////////////////////////////////////////////////////////////
    virtual short setup();
    virtual short onRun();
    short loop() override;
    void loopCycle();
    void loopProfiles();
    short load(short val0 = 0, short val1 = 0);
    virtual short serial_register(Bridge *bridge);
    virtual Component *byId(ushort id);
    // App States & Error Handling
    short _state;
    short _cstate;
    short _error;
    short setAppState(short newState);
    short getAppState(short val);
    short getLastError() { return _error; }
    short setLastError(short val = 0)
    {
        _error = val;
        return _error;
    }
    short onError(short id, short code);
    short clearError();
    short reset(short arg1, short arg2);
    short init(short arg1, short arg2);
    void onSlaveModeChanged(bool enableSlave);
    void testInfluxDB();

    //////////////////////////////////////////////////////////////
    // Components
    //////////////////////////////////////////////////////////////
    SerialMessage *com_serial;
    RuntimeState *runtimeState;
    POT *pot_0;
    POT *pot_1;
    POT *pot_2;
    StatusLight *statusLight_0;
    StatusLight *statusLight_1;
    Relay *relay_0;
    Relay *relay_1;
    Relay *relay_2;
    Relay *relay_3;
    Relay *relay_4;
    Relay *relay_5;
    Relay *relay_6;
    Relay *relay_7;
    Pos3Analog *pos3Analog_0;
    Pos3Analog *pos3Analog_1;
    PIDController *pidController_0;
    SAKO_VFD *vfd_0;

    DELTA_VFD *vfd_2;

    Extruder *extruder_0;
    Plunger *plunger_0;
    MB_GPIO *gpio_0;
    AnalogLevelSwitch *analogLevelSwitch_0;
    LEDFeedback *ledFeedback_0;
    Joystick *joystick_0;
    PushButton *pushButton_1;
    AmperageBudgetManager *pidManagerAmperage;
    NetworkValueTest *networkValueTest;
    NetworkValueTestPB *networkValueTestPB;
    Feedback3C *feedback3C_0;
    FeedbackBuzzer *feedbackBuzzer_0;
    OperatorSwitch *operatorSwitch_0;
    Logger *logger_0;
    Solenoid *solenoid_0;
    Solenoid *solenoid_1;
    PressCylinder *pressCylinder_0;
    ModbusMirror *modbusMirror_0;
    InfluxDB *influxDb_0;
    IFTTTWebhook *iftttWebhook;
    Loadcell *loadCell_0;
    Loadcell *loadCell_1;
    // Component Callbacks/Control
    short onStop(short code = 0);
    short onWarning(short code);

    //////////////////////////////////////////////////////////////
    // Logging
    //////////////////////////////////////////////////////////////
    std::vector<String> logBuffer;
    size_t currentLogIndex = 0;
    CircularLogPrinter *logPrinter = nullptr;

    Settings *appSettings;
    //////////////////////////////////////////////////////////////
    // Network Management
    //////////////////////////////////////////////////////////////
    short setupNetwork();
    short loadNetworkSettings();
    short saveNetworkSettings(JsonObject &doc);
    WiFiNetworkSettings wifiSettings;

#ifdef ENABLE_SETTINGS
    short loadAppSettings();
#endif

    //////////////////////////////////////////////////////////////
    // Modbus TCP
    //////////////////////////////////////////////////////////////
    ModbusTCP *modbusManager;
    short loopModbus();
#ifdef ENABLE_MODBUS_TCP
    short setupModbus();

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;

    short mb_tcp_read(MB_Registers *reg) override;
    void mb_tcp_register(ModbusTCP *manager) override;
    ModbusBlockView *mb_tcp_blocks() const override;

    short runCommand(short command);

    int client_count;
    int client_max;
    int client_total;
    millis_t client_track_ts;

    short updateClientCount(short val0, short val1);
    short resetClientStats(short val0, short val1);
    short getClientStats(short val0, short val1);

    // Modbus PID Specific (Conditional)
    short getConnectedClients() const; // Returns number of currently connected Modbus TCP clients

    bool isSlave() const;
    short setSlave(bool value, bool sync);

    bool getAllOmronStop() const;
    short setAllOmronStop(bool value, bool sync);

    bool getAllOmronComWrite() const;
    short setAllOmronComWrite(bool value, bool sync);

    bool getAppWarmup() const { return _app_warmup; }
    short setAppWarmup(bool value, bool sync);

    bool getAppPidLag() const { return _app_pid_lag; }
    short setAppPidLag(bool value, bool sync);

#ifdef ENABLE_PID
    short getPid2Register(short offset, short unused);
    short setPid2Register(short offset, short value);
#endif // ENABLE_PID
#endif // ENABLE_MODBUS_TCP
    RESTServer *webServer;
    short loopWeb();

#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)
    friend class RS485Devices;
    OmronE5 *findOmronByTcpAddress(uint16_t tcpAddress);
#endif // ENABLE_RS485 && ENABLE_OMRON_E5

    //////////////////////////////////////////////////////////////
    // Component Overrides / Message Handling
    //////////////////////////////////////////////////////////////
    /**
     * @brief Handles incoming messages, including RTU updates via void*.
     */
    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user, Component *src) override;
    //////////////////////////////////////////////////////////////
    // Debugging & Utility Methods
    //////////////////////////////////////////////////////////////
    void printRegisters();
    short list(short val0, short val1);
    short print(short arg1, short arg2);

//////////////////////////////////////////////////////////////
// Profiling & Feature Specific (Conditional)
//////////////////////////////////////////////////////////////
#ifdef ENABLE_PROFILER
    static uint32_t initialFreeHeap;
    static uint64_t initialCpuTicks;
#endif // ENABLE_PROFILER

#ifdef ENABLE_PROFILE_TEMPERATURE
    TemperatureProfile *tempProfiles[PROFILE_TEMPERATURE_COUNT]; // Array to hold multiple temperature profiles
    void getProfilesHandler(AsyncWebServerRequest *request);
    void setProfilesHandler(AsyncWebServerRequest *request, JsonVariant &json, int slot); // Adjusted for body handling
    bool updateProfile(JsonObject &json, int slot);
    bool saveProfilesToJson();
    void checkProfileLag(TemperatureProfile *profile, uint32_t now, uint32_t loopDelta);
#endif // ENABLE_PROFILE_TEMPERATURE

#ifdef ENABLE_PROFILE_PRESSURE
    PressureProfile *pressureProfiles[PROFILE_PRESSURE_COUNT];
    void getPressureProfilesHandler(AsyncWebServerRequest *request);
    void setPressureProfilesHandler(AsyncWebServerRequest *request, JsonVariant &json, int slot);
    bool updatePressureProfile(JsonObject &json, int slot);
    bool savePressureProfilesToJson();
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    SignalPlot *signalPlots[PROFILE_SIGNAL_PLOT_COUNT]; // Array to hold multiple signal plot profiles
    void getSignalPlotsHandler(AsyncWebServerRequest *request);
    void setSignalPlotsHandler(AsyncWebServerRequest *request, JsonVariant &json, int slot);
    bool updateSignalPlot(JsonObject &json, int slot);
    bool saveSignalPlotsToJson();
    // Methods to control SignalPlot from TemperatureProfile
    void startSignalPlot(short slotId);
    void stopSignalPlot(short slotId);
    void enableSignalPlot(short slotId, bool enable);
    void pauseSignalPlot(short slotId);
    void resumeSignalPlot(short slotId);
#endif // ENABLE_PROFILE_SIGNAL_PLOT
#ifdef ENABLE_PROFILE_TEMPERATURE
    void onTemperatureProfileStarted(PlotBase *profile) override;
    void onTemperatureProfileStopped(PlotBase *profile) override;
    void onTemperatureProfilePaused(PlotBase *profile) override;
    void onTemperatureProfileResumed(PlotBase *profile) override;
    void onTemperatureProfileFinished(PlotBase *profile) override;
    void onTemperatureProfileReady(PlotBase *profile) override;
#endif
#ifdef ENABLE_PROFILE_PRESSURE
    void onPressureProfileStarted(PlotBase *profile) override;
    void onPressureProfileStopped(PlotBase *profile) override;
    void onPressureProfilePaused(PlotBase *profile) override;
    void onPressureProfileResumed(PlotBase *profile) override;
    void onPressureProfileFinished(PlotBase *profile) override;
#endif
#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    void onSignalPlotStarted(PlotBase *plot) override;
    void onSignalPlotStopped(PlotBase *plot) override;
    void onSignalPlotPaused(PlotBase *plot) override;
    void onSignalPlotResumed(PlotBase *plot) override;
    void onSignalPlotFinished(PlotBase *plot) override;
#endif

    void stopPids(short temperatureProfileSlot);
    void startPids(short temperatureProfileSlot);
    void setPIDs(short temperatureProfileSlot, uint8_t sp);

#ifdef ENABLE_PLUNGER
    void onPlungerEvent(Plunger *instance, PlungerEvent event, int16_t val = 0) override;
#endif

    //////////////////////////////////////////////////////////////
    // Web Server
    //////////////////////////////////////////////////////////////
    /**
     * @brief Register routes with the RESTServer. This will be called upon built-in RESTServer initialization.
     *
     * @param server The RESTServer instance to register routes with.
     * @return short The result of the operation.
     */
    short registerRoutes(RESTServer *instance);
    short registerWebSocketHandlers(RESTServer *instance);
    // Network settings handlers
#ifdef ENABLE_WEBSERVER_WIFI_SETTINGS
#endif
    void getSystemLogsHandler(AsyncWebServerRequest *request);
    void getBridgeMethodsHandler(AsyncWebServerRequest *request);

    // broadcast message relayer (dispayched to RESTServer, websocket)
    void broadcast(BroadcastMessageType type, const JsonDocument &data);

private:
#ifdef ENABLE_OPERATOR_SWITCH
    OperatorSwitch::State lastOperatorSwitchState = OperatorSwitch::State::IDLE;
#endif

    bool _is_slave = false;
    bool _all_omron_stop = false;
    bool _all_omron_com_write = false;
    uint32_t _delayed_omron_write_ts = 0;
    bool _delayed_omron_write_pending = false;
    bool _app_warmup = false;
    bool _app_pid_lag = false;
    //////////////////////////////////////////////////////////////
    // Private Methods
    //////////////////////////////////////////////////////////////
#ifdef ENABLE_PROFILE_TEMPERATURE
    short loadTempProfiles();
#endif
#ifdef ENABLE_PROFILE_PRESSURE
    short loadPressureProfiles();
#endif
#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    short loadSignalPlots();
#endif
    uint32_t _last_cycle_loop_ms;
    uint32_t _last_runtime_save_ms = 0;
#ifdef ENABLE_INFLUXDB
    uint32_t _last_influx_test_ms = 0;
#endif

    void updateRuntimeState();
    short restoreState();
    void handleSerialCommand(const String &command); // Moved here as it's private impl detail
    void cleanupComponents();                        // Moved here as it's private impl detail

    //////////////////////////////////////////////////////////////
    //
    // 3th party libraries & services
};

#endif
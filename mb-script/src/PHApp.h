#ifndef PHAPP_H
#define PHAPP_H

#include "config.h"
#include "config-modbus.h" 
#include "features.h"

#include <enums.h>
#include <vector>
#include <xmath.h>
#include <macros.h>
#include <App.h>
#include <Component.h>
#include <Bridge.h>
#include <SerialMessage.h>
#include <ArduinoLog.h>
#include <Logger.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <xstatistics.h>

#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>
#include <profiles/SignalPlot.h>
#include <profiles/WiFiNetworkSettings.h>

#include <components/OmronE5.h>


class POT;
class Relay;
class RS485;
class Pos3Analog;
class StatusLight;
class RESTServer;
class PIDController; 
class TemperatureProfile;
class SAKO_VFD;
class MB_GPIO;
class AnalogLevelSwitch;
class LEDFeedback;
class Extruder;
class Plunger;
class Joystick;
class PHApp;

class AsyncWebServerRequest;


class PHApp : public App
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
        ERROR = 5,
        PID_TIMEOUT = 11,
        FEED_TIMEOUT = 12,
        CONTROL_PANEL_INVALID = 13,
        PID_ERROR = 20,
        FEED_ERROR = 40,
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
    short loop() override;
    short load(short val0, short val1);
    virtual short serial_register(Bridge *bridge);
    virtual Component *byId(ushort id);
    // App States & Error Handling
    short _state;
    short _cstate;
    short _error;
    short setAppState(short newState);
    short getAppState(short val);
    short getLastError() { return _error; }
    short setLastError(short val = 0) { _error = val; return _error; }
    short onError(short id, short code);
    short clearError();
    short reset(short arg1, short arg2); // Related to resetting state?

    //////////////////////////////////////////////////////////////
    // Components
    //////////////////////////////////////////////////////////////
    SerialMessage *com_serial;
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
    Extruder *extruder_0;
    Plunger *plunger_0;
    
    MB_GPIO *gpio_0;

    AnalogLevelSwitch *analogLevelSwitch_0;
    LEDFeedback *ledFeedback_0;
    Joystick *joystick_0;
    
    // Component Callbacks/Control
    short onStop(short code = 0);
    short onRun(short code = 0);
    short onWarning(short code);

    //////////////////////////////////////////////////////////////
    // Logging
    //////////////////////////////////////////////////////////////
    std::vector<String> logBuffer;
    size_t currentLogIndex = 0;
    CircularLogPrinter *logPrinter = nullptr;    
    std::vector<String> getLogSnapshot()
    {
        std::vector<String> snapshot;
        snapshot.reserve(logBuffer.size());
        if (logBuffer.size() < LOG_BUFFER_LINES)
        { 
            for (size_t i = 0; i < logBuffer.size(); ++i)
            {
                snapshot.push_back(logBuffer[i]);
            }
        }
        else
        {
            // Buffer is full and circular
            size_t startIndex = (currentLogIndex + 1) % LOG_BUFFER_LINES; // <-- Note: LOG_BUFFER_LINES is now defined in Logger.h
            for (size_t i = 0; i < LOG_BUFFER_LINES; ++i)
            {
                snapshot.push_back(logBuffer[(startIndex + i) % LOG_BUFFER_LINES]);
            }
        }
        return snapshot;
    }

    //////////////////////////////////////////////////////////////
    // Network Management
    //////////////////////////////////////////////////////////////
    short setupNetwork();
    short loadNetworkSettings();
    short saveNetworkSettings(JsonObject& doc);
    WiFiNetworkSettings wifiSettings;
    //////////////////////////////////////////////////////////////
    // Modbus TCP
    //////////////////////////////////////////////////////////////
    ModbusTCP *modbusManager;
    short loopModbus();
#ifdef ENABLE_MODBUS_TCP
    short setupModbus();
    
    short mb_tcp_write(short address, short value) override;
    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(short address) override;
    
    void mb_tcp_register(ModbusTCP *manager) const override;
    ModbusBlockView *mb_tcp_blocks() const override;

    int client_count;
    int client_max;
    int client_total;
    millis_t client_track_ts;
    
    short updateClientCount(short val0, short val1);
    short resetClientStats(short val0, short val1);
    short getClientStats(short val0, short val1);

    // Modbus PID Specific (Conditional)
#ifdef ENABLE_PID
    short getPid2Register(short offset, short unused);
    short setPid2Register(short offset, short value);
#endif // ENABLE_PID
#endif // ENABLE_MODBUS_TCP
    RESTServer *webServer;
    short loopWeb();

#ifdef ENABLE_RS485
    friend class RS485Devices;    
#endif // ENABLE_RS485
    
    //////////////////////////////////////////////////////////////
    // Component Overrides / Message Handling
    /////////////////////////////////////////////////////////////
    /**
     * @brief Handles incoming messages, including RTU updates via void*.
     */
    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user, Component *src) override;
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
    TemperatureProfile* tempProfiles[PROFILE_TEMPERATURE_COUNT]; // Array to hold multiple temperature profiles
#endif // ENABLE_PROCESS_PROFILE
    
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
    // Network settings handlers
#ifdef ENABLE_WEBSERVER_WIFI_SETTINGS
    void handleGetNetworkSettings(AsyncWebServerRequest *request);
    void handleSetNetworkSettings(AsyncWebServerRequest *request, JsonVariant &json);
#endif
#ifdef ENABLE_PROFILE_TEMPERATURE
    void getProfilesHandler(AsyncWebServerRequest *request);
    void setProfileHandler(AsyncWebServerRequest *request, JsonVariant &json, int slot); // Adjusted for body handling
#endif
    void getSystemLogsHandler(AsyncWebServerRequest *request);

private:
    //////////////////////////////////////////////////////////////
    // Private Methods
    //////////////////////////////////////////////////////////////
    void handleSerialCommand(const String &command); // Moved here as it's private impl detail
    void cleanupComponents();                        // Moved here as it's private impl detail
};

#endif
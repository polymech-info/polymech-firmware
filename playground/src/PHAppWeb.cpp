#include "PHApp.h"
#include <components/RestServer.h>
#include <ESPAsyncWebServer.h>

void PHApp::broadcast(BroadcastMessageType type, const JsonDocument &data)
{    
    #if defined(ENABLE_WEBSERVER) && defined(ENABLE_WEBSOCKET)
        webServer->broadcast(type, data);
    #endif
}

short PHApp::registerRoutes(RESTServer *instance)
{

#ifdef ENABLE_PLUNGER
    instance->server.on("/api/v1/plunger/settings", HTTP_GET, [instance](AsyncWebServerRequest *request)
                        {
    Component* comp = instance->owner->byId(COMPONENT_KEY_PLUNGER); 
    if (!comp) {
        request->send(404, "application/json", "{\"success\":false,\"error\":\"Plunger component not found\"}");
        return;
    }
    Plunger* plunger = static_cast<Plunger*>(comp);        
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc; 
    plunger->getSettingsJson(doc);
    serializeJson(doc, *response);
    request->send(response); });
    AsyncCallbackJsonWebHandler *setPlungerSettingsHandler = new AsyncCallbackJsonWebHandler("/api/v1/plunger/settings",
                                                                                             [instance](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                             {
                                                                                                 Component *comp = instance->owner->byId(COMPONENT_KEY_PLUNGER);
                                                                                                 if (!comp)
                                                                                                 {
                                                                                                     request->send(404, "application/json", "{\"success\":false,\"error\":\"Plunger component not found\"}");
                                                                                                     return;
                                                                                                 }
                                                                                                 Plunger *plunger = static_cast<Plunger *>(comp);
                                                                                                 if (!json.is<JsonObject>())
                                                                                                 {
                                                                                                     request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: Expected an object.\"}");
                                                                                                     return;
                                                                                                 }
                                                                                                 JsonObject jsonObj = json.as<JsonObject>();
                                                                                                 if (plunger->updateSettingsFromJson(jsonObj))
                                                                                                 {
                                                                                                     request->send(200, "application/json", "{\"success\":true,\"message\":\"Plunger settings updated and saved.\"}");
                                                                                                 }
                                                                                                 else
                                                                                                 {
                                                                                                     request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to update or save Plunger settings.\"}");
                                                                                                 }
                                                                                             });

    setPlungerSettingsHandler->setMethod(HTTP_POST);
    instance->server.addHandler(setPlungerSettingsHandler);

    instance->server.on("/api/v1/plunger/settings/load-defaults", HTTP_POST, [instance](AsyncWebServerRequest *request)
                        {
    Component* comp = instance->owner->byId(COMPONENT_KEY_PLUNGER); 
    if (!comp) {
        request->send(404, "application/json", "{\"success\":false,\"error\":\"Plunger component not found\"}");
        return;
    }
    Plunger* plunger = static_cast<Plunger*>(comp);
    if (plunger->loadDefaultSettings()) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Plunger default settings loaded and applied to operational settings.\"}");
    } else {
        request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to load default settings or save them to operational path.\"}");
    } });

#endif

#ifdef ENABLE_PROFILE_TEMPERATURE
    // --- Temperature Profile Routes ---
    instance->server.on("/api/v1/profiles", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->getProfilesHandler(request); });

    // Handler for POST /api/v1/profiles
    // The slot is now taken from the JSON payload.
    AsyncCallbackJsonWebHandler* postProfileHandler = new AsyncCallbackJsonWebHandler("/api/v1/profiles", 
        [this](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!json.is<JsonObject>() || !json["slot"].is<int>()) {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: Must be JSON object containing an integer 'slot' field.\"}");
                return;
            }
            
            int slot = json["slot"].as<int>();

            // Basic validation for slot number (e.g., non-negative)
            // You might want to add an upper bound check against PROFILE_TEMPERATURE_COUNT if it's accessible here
            // or rely on setProfilesHandler to do more thorough validation.
            if (slot < 0) { 
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid profile slot number in payload.\"}");
                return;
            }

            // Call the actual handler, passing the parsed JSON and extracted slot number
            this->setProfilesHandler(request, json, slot);
        });
    postProfileHandler->setMethod(HTTP_POST); // Ensure it's set for POST
    instance->server.addHandler(postProfileHandler);
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    // --- Signal Plot Profile Routes ---
    instance->server.on("/api/v1/signalplots", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->getSignalPlotsHandler(request); });

    AsyncCallbackJsonWebHandler* postSignalPlotHandler = new AsyncCallbackJsonWebHandler("/api/v1/signalplots", 
        [this](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!json.is<JsonObject>() || !json["slot"].is<int>()) {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: Must be JSON object containing an integer 'slot' field.\"}");
                return;
            }
            int slot = json["slot"].as<int>();
            if (slot < 0) { 
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid profile slot number in payload.\"}");
                return;
            }
            this->setSignalPlotsHandler(request, json, slot); 
        });
    postSignalPlotHandler->setMethod(HTTP_POST);
    instance->server.addHandler(postSignalPlotHandler);
#endif

#ifdef ENABLE_WEBSERVER_WIFI_SETTINGS
    instance->server.on("/api/network/settings", HTTP_GET, std::bind(&PHApp::handleGetNetworkSettings, this, std::placeholders::_1));
    AsyncCallbackJsonWebHandler *setNetworkSettingsHandler = new AsyncCallbackJsonWebHandler("/api/network/settings",
                                                                                             std::bind(&PHApp::handleSetNetworkSettings, this, std::placeholders::_1, std::placeholders::_2));
    setNetworkSettingsHandler->setMethod(HTTP_POST);
    instance->server.addHandler(setNetworkSettingsHandler);
#endif

    instance->server.on("/api/v1/system/logs", HTTP_GET, [this](AsyncWebServerRequest *request)
                        { this->getSystemLogsHandler(request); });

    instance->server.on("/api/v1/methods", HTTP_GET, [this](AsyncWebServerRequest *request)
                        { this->getBridgeMethodsHandler(request); });

    AsyncCallbackJsonWebHandler* postMethodHandler = new AsyncCallbackJsonWebHandler("/api/v1/methods", 
        [this](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!json.is<JsonObject>() || !json["command"].is<String>()) {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: Must be JSON object containing a 'command' string field.\"}");
                return;
            }
            
            String cmdStr = json["command"].as<String>();
            Log.infoln("REST: Received method call command: %s", cmdStr.c_str());

            Bridge* bridge = static_cast<Bridge*>(byId(COMPONENT_KEY_MB_BRIDGE));
            if (!bridge) {
                Log.errorln("REST: Bridge component not found!");
                request->send(500, "application/json", "{\"success\":false,\"error\":\"Bridge component not found\"}");
                return;
            }

            CommandMessage msg;
            if (!msg.parse(cmdStr)) {
                Log.errorln("REST: Failed to parse command string: %s", cmdStr.c_str());
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid command string format\"}");
                return;
            }

            short result = bridge->onMessage(msg.id, msg.verb, msg.flags, msg.payload, this);
            if (result == E_OK) {
                request->send(200, "application/json", "{\"success\":true,\"message\":\"Method executed successfully\"}");
            } else {
                Log.errorln("REST: Method execution failed with error %d", result);
                request->send(500, "application/json", "{\"success\":false,\"error\":\"Method execution failed\"}");
            }
        });
    postMethodHandler->setMethod(HTTP_POST);
    instance->server.addHandler(postMethodHandler);

    return E_OK;
}

#ifdef ENABLE_WEBSERVER_WIFI_SETTINGS
void PHApp::handleGetNetworkSettings(AsyncWebServerRequest *request)
{
    JsonDocument doc = wifiSettings.toJSON();
    String responseStr;
    serializeJson(doc, responseStr);
    request->send(200, "application/json", responseStr);
}

void PHApp::handleSetNetworkSettings(AsyncWebServerRequest *request, JsonVariant &json)
{
    if (!json.is<JsonObject>())
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: Expected an object.\"}");
        return;
    }
    JsonObject jsonObj = json.as<JsonObject>();

    // Attempt to save the settings
    short saveResult = saveNetworkSettings(jsonObj);
    if (saveResult != E_OK)
    {
        Log.errorln("REST: Failed to save network settings, error: %d", saveResult);
        request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save network settings to persistent storage.\"}");
        return;
    }

    // Attempt to load and apply the new settings immediately
    short loadResult = loadNetworkSettings();
    if (loadResult != E_OK && loadResult != E_NOT_FOUND)
    { // E_NOT_FOUND is ok if we just saved it, means it was applied from the save buffer
        Log.warningln("REST: Issue loading network settings after save, error: %d. Settings might not be immediately active.", loadResult);
        // Decide if this is a critical failure for the response
    }
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Network settings saved. Device will attempt to apply them. A restart might be required for all changes to take effect.\"}");
}

#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
/**
 * @brief Handles GET requests to /api/v1/signalplots
 * Returns a list of available signal plot profiles.
 */
void PHApp::getSignalPlotsHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray profilesArray = doc["signalplots"].to<JsonArray>();

    for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; ++i)
    {
        SignalPlot *profile = this->signalPlots[i];
        if (profile)
        {
            Log.verboseln("    Processing SignalPlot Slot %d: %s", i, profile->name.c_str());
            JsonObject profileObj = profilesArray.add<JsonObject>();
            profileObj["slot"] = i;
            profileObj["name"] = profile->name;
            profileObj["duration"] = profile->getDuration();    
            profileObj["status"] = (int)profile->getCurrentStatus();
            profileObj["enabled"] = profile->enabled();
            profileObj["elapsed"] = profile->getElapsedMs();
            profileObj["remaining"] = profile->getRemainingTime();
            
            JsonArray pointsArray = profileObj["controlPoints"].to<JsonArray>();
            const S_SignalControlPoint *points = profile->getControlPoints(); 
            uint8_t numPoints = profile->getNumControlPoints(); 

            for (uint8_t j = 0; j < numPoints; ++j) 
            {
                const S_SignalControlPoint& cp = points[j]; 
                JsonObject pointObj = pointsArray.add<JsonObject>();
                pointObj["id"] = cp.id;
                pointObj["time"] = cp.time;
                pointObj["name"] = cp.name;
                pointObj["description"] = cp.description;
                pointObj["state"] = (int16_t)cp.state;
                pointObj["type"] = (int16_t)cp.type;
                pointObj["arg_0"] = cp.arg_0;
                pointObj["arg_1"] = cp.arg_1;
                pointObj["arg_2"] = cp.arg_2;
            }
        }
        else
        {
            Log.warningln("  SignalPlot slot %d is null", i);
        }
    }
    serializeJson(doc, *response);
    request->send(response);
}

/**
 * @brief Handles POST requests to /api/v1/signalplots (slot from payload)
 * Updates the specified signal plot profile using the provided JSON data.
 *
 * @param request The incoming web request.
 * @param json The parsed JSON body from the request.
 * @param slot The profile slot number extracted from the payload.
 */
void PHApp::setSignalPlotsHandler(AsyncWebServerRequest *request, JsonVariant &json, int slot)
{
    if (slot < 0 || slot >= PROFILE_SIGNAL_PLOT_COUNT) 
    {
        Log.warningln("REST: setSignalPlotsHandler - Invalid slot number %d provided.", slot);
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid profile slot number\"}");
        return;
    }

    SignalPlot *targetProfile = this->signalPlots[slot];
    if (!targetProfile)
    {
        Log.warningln("REST: setSignalPlotsHandler - No profile found for slot %d.", slot);
        request->send(404, "application/json", "{\"success\":false,\"error\":\"Profile slot not found or not initialized\"}");
        return;
    }

    if (!json.is<JsonObject>())
    {
        Log.warningln("REST: setSignalPlotsHandler - Invalid JSON payload (not an object) for slot %d.", slot);
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: must be an object.\"}");
        return;
    }
    JsonObject jsonObj = json.as<JsonObject>();

    bool success = targetProfile->load(jsonObj); 

    if (success)
    {
        Log.infoln("REST: SignalPlot slot %d updated successfully.", slot);
        if (saveSignalPlotsToJson()) { 
            Log.infoln("REST: All SignalPlot profiles saved to JSON successfully after update.");
            request->send(200, "application/json", "{\"success\":true, \"message\":\"SignalPlot profile updated and saved.\"}");
        } else {
            Log.errorln("REST: SignalPlot slot %d updated, but failed to save all profiles to JSON.", slot);
            request->send(500, "application/json", "{\"success\":true, \"message\":\"SignalPlot profile updated but failed to save configuration.\"}");
        }
    }
    else
    {
        Log.errorln("REST: Failed to update SignalPlot slot %d from JSON.", slot);
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Failed to load SignalPlot profile data. Check format and values.\"}");
    }
}
#endif // ENABLE_PROFILE_SIGNAL_PLOT

void PHApp::getBridgeMethodsHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray methodsArray = doc.to<JsonArray>();

    Bridge *bridge = static_cast<Bridge *>(byId(COMPONENT_KEY_MB_BRIDGE));
    if (!bridge)
    {
        Log.errorln(F("REST: Bridge component not found!"));
        request->send(500, "application/json", "{\"success\":false,\"error\":\"Bridge component not found\"}");
        return;
    }

    const Vector<SComponentInfo *> &componentList = bridge->getComponentList();
    for (size_t i = 0; i < componentList.size(); ++i)
    {
        SComponentInfo *compInfo = componentList.at(i);
        if (compInfo && compInfo->instance)
        {
            Component *component = static_cast<Component *>(compInfo->instance);
            JsonObject methodObj = methodsArray.add<JsonObject>();
            methodObj["id"] = compInfo->key;
            methodObj["component"] = component->name;
            methodObj["method"] = compInfo->methodName;
        }
    }
    serializeJson(doc, *response);
    request->send(response);
}

void PHApp::getSystemLogsHandler(AsyncWebServerRequest *request)
{
    String levelStr = "verbose"; // Default to verbose
    if (request->hasParam("level"))
    {
        levelStr = request->getParam("level")->value();
    }

    // Map string log levels to their integer values
    int requestedLevel = LOG_LEVEL_VERBOSE; // Default to verbose
    if (levelStr == "none")
        requestedLevel = LOG_LEVEL_SILENT;
    else if (levelStr == "error")
        requestedLevel = LOG_LEVEL_ERROR;
    else if (levelStr == "warning")
        requestedLevel = LOG_LEVEL_WARNING;
    else if (levelStr == "notice")
        requestedLevel = LOG_LEVEL_NOTICE;
    else if (levelStr == "trace")
        requestedLevel = LOG_LEVEL_TRACE;
    else if (levelStr == "verbose")
        requestedLevel = LOG_LEVEL_VERBOSE;
    else
    {
        request->send(400, "application/json", "{\"error\":\"Invalid log level\"}");
        return;
    }
    String response;
    // Get logs using existing logBuffer implementation in PHApp
    std::vector<String> logSnapshot = getLogSnapshot();

    // Begin JSON array response
    response = "[";
    bool first = true;

    // Function to escape special characters in JSON
    auto escapeJSON = [](const String &str) -> String
    {
        String result;
        for (size_t i = 0; i < str.length(); i++)
        {
            char c = str.charAt(i);
            switch (c)
            {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (c < ' ')
                {
                    char hex[7];
                    snprintf(hex, sizeof(hex), "\\u%04x", c);
                    result += hex;
                }
                else
                {
                    result += c;
                }
            }
        }
        return result;
    };

    // Function to determine log level from a log line
    auto getLogLevel = [](const String &line) -> int
    {
        if (line.startsWith("E:"))
            return LOG_LEVEL_ERROR;
        if (line.startsWith("W:"))
            return LOG_LEVEL_WARNING;
        if (line.startsWith("N:"))
            return LOG_LEVEL_NOTICE;
        if (line.startsWith("T:"))
            return LOG_LEVEL_TRACE;
        if (line.startsWith("V:"))
            return LOG_LEVEL_VERBOSE;
        if (line.startsWith("I:"))
            return LOG_LEVEL_INFO;
        return LOG_LEVEL_VERBOSE; // Default to verbose if no prefix found
    };

    // Add each log entry to the response if it meets the requested level
    for (const auto &logLine : logSnapshot)
    {
        int lineLevel = getLogLevel(logLine);
        if (lineLevel <= requestedLevel)
        {
            if (!first)
                response += ",";
            response += "\"" + escapeJSON(logLine) + "\"";
            first = false;
        }
    }
    response += "]";
    request->send(200, "application/json", response);
}

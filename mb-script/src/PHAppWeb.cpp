#include "PHApp.h"
#include <components/RestServer.h>
#include <ESPAsyncWebServer.h>
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
    server.on("/api/v1/profiles", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->getProfilesHandler(request); });

    // Use AsyncJsonRequestBodyHandler for POST with JSON body
    // Slot number is now expected in the JSON payload
    AsyncCallbackJsonWebHandler *postProfileHandler = new AsyncCallbackJsonWebHandler("/api/v1/profiles",
                                                                                      [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                      {
                                                                                          // Modern check: Use is<T>() which implicitly handles existence.
                                                                                          // If !json.is<JsonObject>(), the whole condition is true.
                                                                                          // If json is an object, then !json["slot"].is<int>() checks for existence AND integer type.
                                                                                          if (!json.is<JsonObject>() || !json["slot"].is<int>())
                                                                                          {
                                                                                              request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: Must be JSON object containing an integer 'slot' field.\"}");
                                                                                              return;
                                                                                          }

                                                                                          int slot = json["slot"].as<int>();

                                                                                          // Basic validation - check if slot is within a reasonable range
                                                                                          if (slot < 0 /*|| slot >= MAX_PROFILES - check MAX_PROFILES definition */)
                                                                                          { // Remove check against 0 if slot 0 is valid
                                                                                              request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid profile slot number in payload\"}");
                                                                                              return;
                                                                                          }
                                                                                          // Call the actual handler, passing the parsed JSON and extracted slot number
                                                                                          this->setProfileHandler(request, json, slot);
                                                                                      });
    instance->server.addHandler(postProfileHandler);
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

#endif

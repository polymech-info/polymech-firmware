#include "PHApp.h"
#include <components/RestServer.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <components/RS485.h>
#include <modbus/ModbusTypes.h>

void PHApp::broadcast(BroadcastMessageType type, const JsonDocument &data)
{
#if defined(ENABLE_WEBSERVER) && defined(ENABLE_WEBSOCKET)
    webServer->broadcast(type, data);
#endif
}

short PHApp::registerRoutes(RESTServer *instance)
{
#ifdef REST_SERVER_APP_ROUTES

#ifdef ENABLE_SETTINGS
    // GET /api/v1/settings
    instance->server.on("/api/v1/settings", HTTP_GET, [this](AsyncWebServerRequest *request)
                        {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        JsonDocument doc;
        this->appSettings->toJSON(doc.to<JsonVariant>());
        ArduinoJson::serializeJson(doc, *response);
        request->send(response); });

    // POST /api/v1/settings
    AsyncCallbackJsonWebHandler *setSettingsHandler = new AsyncCallbackJsonWebHandler("/api/v1/settings",
                                                                                      [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                      {
                                                                                          if (!json.is<JsonObject>())
                                                                                          {
                                                                                              request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: Expected an object.\"}");
                                                                                              return;
                                                                                          }
                                                                                          this->appSettings->fromJSON(json);
                                                                                          if (this->appSettings->save("/settings.json"))
                                                                                          {
                                                                                              request->send(200, "application/json", "{\"success\":true,\"message\":\"Settings updated and saved.\"}");
                                                                                          }
                                                                                          else
                                                                                          {
                                                                                              request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save settings.\"}");
                                                                                          }
                                                                                      });
    setSettingsHandler->setMethod(HTTP_POST);
    instance->server.addHandler(setSettingsHandler);
#endif

#ifdef ENABLE_RUNTIME_STATE
    instance->server.on("/api/v1/state", HTTP_GET, [this](AsyncWebServerRequest *request)
                        {
        if (!this->runtimeState) {
            request->send(500, "application/json", "{\"success\":false,\"error\":\"RuntimeState component not found\"}");
            return;
        }
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        JsonDocument doc;
        updateRuntimeState();
        this->runtimeState->toJSON(doc.to<JsonVariant>());
        ArduinoJson::serializeJson(doc, *response);
        request->send(response); });
#endif

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
    ArduinoJson::serializeJson(doc, *response);
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
    AsyncCallbackJsonWebHandler *postProfileHandler = new AsyncCallbackJsonWebHandler("/api/v1/profiles",
                                                                                      [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                      {
                                                                                          if (json.is<JsonObject>())
                                                                                          {
                                                                                              if (!json["slot"].is<int>())
                                                                                              {
                                                                                                  request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: JSON object must contain an integer 'slot' field.\"}");
                                                                                                  return;
                                                                                              }
                                                                                              int slot = json["slot"].as<int>();
                                                                                              this->setProfilesHandler(request, json, slot);
                                                                                          }
                                                                                          else if (json.is<JsonArray>())
                                                                                          {
                                                                                              JsonArray profiles = json.as<JsonArray>();
                                                                                              bool all_success = true;
                                                                                              for (JsonVariant v : profiles)
                                                                                              {
                                                                                                  if (!v.is<JsonObject>())
                                                                                                  {
                                                                                                      all_success = false;
                                                                                                      Log.warningln("REST: Array contains non-object element.");
                                                                                                      continue;
                                                                                                  }
                                                                                                  JsonObject profile_json = v.as<JsonObject>();
                                                                                                  if (!profile_json["slot"].is<int>())
                                                                                                  {
                                                                                                      all_success = false;
                                                                                                      Log.warningln("REST: Profile in array is missing 'slot'.");
                                                                                                      continue;
                                                                                                  }
                                                                                                  int slot = profile_json["slot"].as<int>();
                                                                                                  if (!updateProfile(profile_json, slot))
                                                                                                  {
                                                                                                      all_success = false;
                                                                                                      Log.warningln("REST: Failed to update profile in slot %d from array.", slot);
                                                                                                  }
                                                                                              }

                                                                                              if (saveProfilesToJson())
                                                                                              {
                                                                                                  if (all_success)
                                                                                                  {
                                                                                                      request->send(200, "application/json", "{\"success\":true, \"message\":\"All profiles updated and saved successfully.\"}");
                                                                                                  }
                                                                                                  else
                                                                                                  {
                                                                                                      request->send(207, "application/json", "{\"success\":true, \"message\":\"Partially updated profiles and saved successfully.\"}");
                                                                                                  }
                                                                                              }
                                                                                              else
                                                                                              {
                                                                                                  request->send(500, "application/json", "{\"success\":false, \"message\":\"Profiles updated but failed to save configuration.\"}");
                                                                                              }
                                                                                          }
                                                                                          else
                                                                                          {
                                                                                              request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: Must be a JSON object or an array of JSON objects.\"}");
                                                                                          }
                                                                                      });
    postProfileHandler->setMethod(HTTP_POST); // Ensure it's set for POST
    instance->server.addHandler(postProfileHandler);

    instance->server.on("/profiles/save", HTTP_GET, [this](AsyncWebServerRequest *request)
                        {
        if (saveProfilesToJson()) {
            request->send(200, "application/json", "{\"success\":true, \"message\":\"Profiles saved.\"}");
        } else {
            request->send(500, "application/json", "{\"success\":false, \"message\":\"Failed to save profiles. FS Locked.\"}");
        } });
#endif

#ifdef ENABLE_PROFILE_PRESSURE
    instance->server.on("/api/v1/pressure-profiles", HTTP_GET, [this](AsyncWebServerRequest *request)
                        { this->getPressureProfilesHandler(request); });

    AsyncCallbackJsonWebHandler *postPressureProfileHandler = new AsyncCallbackJsonWebHandler("/api/v1/pressure-profiles",
                                                                                              [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                              {
                                                                                                  if (json.is<JsonObject>())
                                                                                                  {
                                                                                                      if (!json["slot"].is<int>())
                                                                                                      {
                                                                                                          request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: JSON object must contain an integer 'slot' field.\"}");
                                                                                                          return;
                                                                                                      }
                                                                                                      int slot = json["slot"].as<int>();
                                                                                                      this->setPressureProfilesHandler(request, json, slot);
                                                                                                  }
                                                                                                  else if (json.is<JsonArray>())
                                                                                                  {
                                                                                                      JsonArray profiles = json.as<JsonArray>();
                                                                                                      bool all_success = true;
                                                                                                      for (JsonVariant v : profiles)
                                                                                                      {
                                                                                                          if (!v.is<JsonObject>())
                                                                                                          {
                                                                                                              all_success = false;
                                                                                                              continue;
                                                                                                          }
                                                                                                          JsonObject profile_json = v.as<JsonObject>();
                                                                                                          if (!profile_json["slot"].is<int>())
                                                                                                          {
                                                                                                              all_success = false;
                                                                                                              continue;
                                                                                                          }
                                                                                                          int slot = profile_json["slot"].as<int>();
                                                                                                          if (!updatePressureProfile(profile_json, slot))
                                                                                                          {
                                                                                                              all_success = false;
                                                                                                          }
                                                                                                      }

                                                                                                      if (savePressureProfilesToJson())
                                                                                                      {
                                                                                                          if (all_success)
                                                                                                          {
                                                                                                              request->send(200, "application/json", "{\"success\":true, \"message\":\"All profiles updated and saved successfully.\"}");
                                                                                                          }
                                                                                                          else
                                                                                                          {
                                                                                                              request->send(207, "application/json", "{\"success\":true, \"message\":\"Partially updated profiles and saved successfully.\"}");
                                                                                                          }
                                                                                                      }
                                                                                                      else
                                                                                                      {
                                                                                                          request->send(500, "application/json", "{\"success\":false, \"message\":\"Profiles updated but failed to save configuration.\"}");
                                                                                                      }
                                                                                                  }
                                                                                                  else
                                                                                                  {
                                                                                                      request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: Must be a JSON object or an array of JSON objects.\"}");
                                                                                                  }
                                                                                              });
    postPressureProfileHandler->setMethod(HTTP_POST);
    instance->server.addHandler(postPressureProfileHandler);
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    // --- Signal Plot Profile Routes ---
    instance->server.on("/api/v1/signalplots", HTTP_GET, [this](AsyncWebServerRequest *request)
                        { this->getSignalPlotsHandler(request); });

    AsyncCallbackJsonWebHandler *postSignalPlotHandler = new AsyncCallbackJsonWebHandler("/api/v1/signalplots",
                                                                                         [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                         {
                                                                                             if (json.is<JsonObject>())
                                                                                             {
                                                                                                 if (!json["slot"].is<int>())
                                                                                                 {
                                                                                                     request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: JSON object must contain an integer 'slot' field.\"}");
                                                                                                     return;
                                                                                                 }
                                                                                                 int slot = json["slot"].as<int>();
                                                                                                 this->setSignalPlotsHandler(request, json, slot);
                                                                                             }
                                                                                             else if (json.is<JsonArray>())
                                                                                             {
                                                                                                 JsonArray plots = json.as<JsonArray>();
                                                                                                 bool all_success = true;
                                                                                                 for (JsonVariant v : plots)
                                                                                                 {
                                                                                                     if (!v.is<JsonObject>())
                                                                                                     {
                                                                                                         all_success = false;
                                                                                                         Log.warningln("REST: Array contains non-object element.");
                                                                                                         continue;
                                                                                                     }
                                                                                                     JsonObject plot_json = v.as<JsonObject>();
                                                                                                     if (!plot_json["slot"].is<int>())
                                                                                                     {
                                                                                                         all_success = false;
                                                                                                         Log.warningln("REST: Signal plot in array is missing 'slot'.");
                                                                                                         continue;
                                                                                                     }
                                                                                                     int slot = plot_json["slot"].as<int>();
                                                                                                     if (!updateSignalPlot(plot_json, slot))
                                                                                                     {
                                                                                                         all_success = false;
                                                                                                         Log.warningln("REST: Failed to update signal plot in slot %d from array.", slot);
                                                                                                     }
                                                                                                 }

                                                                                                 if (saveSignalPlotsToJson())
                                                                                                 {
                                                                                                     if (all_success)
                                                                                                     {
                                                                                                         request->send(200, "application/json", "{\"success\":true, \"message\":\"All signal plots updated and saved successfully.\"}");
                                                                                                     }
                                                                                                     else
                                                                                                     {
                                                                                                         request->send(207, "application/json", "{\"success\":true, \"message\":\"Partially updated signal plots and saved successfully.\"}");
                                                                                                     }
                                                                                                 }
                                                                                                 else
                                                                                                 {
                                                                                                     request->send(500, "application/json", "{\"success\":false, \"message\":\"Signal plots updated but failed to save configuration.\"}");
                                                                                                 }
                                                                                             }
                                                                                             else
                                                                                             {
                                                                                                 request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: Must be a JSON object or an array of JSON objects.\"}");
                                                                                             }
                                                                                         });
    postSignalPlotHandler->setMethod(HTTP_POST);
    instance->server.addHandler(postSignalPlotHandler);
#endif

#ifdef ENABLE_WEBSERVER_WIFI_SETTINGS
    instance->server.on("/api/v1/network/settings", HTTP_GET, [this](AsyncWebServerRequest *request)
                        {
        JsonDocument doc = this->wifiSettings.toJSON();
        String responseStr;
        ArduinoJson::serializeJson(doc, responseStr);
        request->send(200, "application/json", responseStr); });
    AsyncCallbackJsonWebHandler *setNetworkSettingsHandler = new AsyncCallbackJsonWebHandler("/api/v1/network/settings",
                                                                                             [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                             {
                                                                                                 if (!json.is<JsonObject>())
                                                                                                 {
                                                                                                     request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: Expected an object.\"}");
                                                                                                     return;
                                                                                                 }
                                                                                                 JsonObject jsonObj = json.as<JsonObject>();
                                                                                                 short saveResult = this->saveNetworkSettings(jsonObj);
                                                                                                 if (saveResult != E_OK)
                                                                                                 {
                                                                                                     L_ERROR("REST: Failed to save network settings, error: %d", saveResult);
                                                                                                     request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save network settings to persistent storage.\"}");
                                                                                                     return;
                                                                                                 }
                                                                                                 short loadResult = this->loadNetworkSettings();
                                                                                                 if (loadResult != E_OK && loadResult != E_NOT_FOUND)
                                                                                                 {
                                                                                                     Log.warningln("REST: Issue loading network settings after save, error: %d. Settings might not be immediately active.", loadResult);
                                                                                                 }
                                                                                                 request->send(200, "application/json", "{\"success\":true,\"message\":\"Network settings saved. Device will attempt to apply them. A restart might be required for all changes to take effect.\"}");
                                                                                             });
    setNetworkSettingsHandler->setMethod(HTTP_POST);
    instance->server.addHandler(setNetworkSettingsHandler);
#endif

    instance->server.on("/api/v1/components", HTTP_GET, [this](AsyncWebServerRequest *request)
                        {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        JsonDocument doc;
        JsonArray componentsArray = doc.to<JsonArray>();

        for (size_t i = 0; i < this->components.size(); ++i)
        {
            Component* comp = this->components.at(i);
            if (!comp)
            {
                continue;
            }
            JsonObject compObj = componentsArray.add<JsonObject>();
            compObj["id"] = comp->id;
            compObj["name"] = comp->name;
            compObj["enabled"] = comp->enabled();
            
            JsonObject flagsObj = compObj["flags"].to<JsonObject>();
            flagsObj["run"] = comp->flags;
            flagsObj["network"] = comp->nFlags;
            flagsObj["feature"] = comp->fFlags;
        }
        JsonObject compObj = componentsArray.add<JsonObject>();
        compObj["id"] = id;
        compObj["name"] = "PHApp";
        compObj["enabled"] = true;
        
        JsonObject flagsObj = compObj["flags"].to<JsonObject>();
        flagsObj["run"] = flags;
        flagsObj["network"] = nFlags;
        flagsObj["feature"] = fFlags;
        
        ArduinoJson::serializeJson(doc, *response);
        request->send(response); });

    instance->server.on("/api/v1/features", HTTP_GET, [this](AsyncWebServerRequest *request)
                        {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        JsonDocument doc;
        JsonObject features = doc["features"].to<JsonObject>();

#ifdef ENABLE_FEEDBACK_BUZZER
        features["ENABLE_FEEDBACK_BUZZER"] = true;
#else
        features["ENABLE_FEEDBACK_BUZZER"] = false;
#endif

#ifdef ENABLE_FEEDBACK_3C
        features["ENABLE_FEEDBACK_3C"] = true;
#else
        features["ENABLE_FEEDBACK_3C"] = false;
#endif

#ifdef ENABLE_LOADCELL
        features["ENABLE_LOADCELL"] = true;
#else
        features["ENABLE_LOADCELL"] = false;
#endif

#ifdef ENABLE_SOLENOID_0
        features["ENABLE_SOLENOID_0"] = true;
#else
        features["ENABLE_SOLENOID_0"] = false;
#endif

#ifdef ENABLE_SOLENOID_1
        features["ENABLE_SOLENOID_1"] = true;
#else
        features["ENABLE_SOLENOID_1"] = false;
#endif

#ifdef ENABLE_PRESS_CYLINDER
        features["ENABLE_PRESS_CYLINDER"] = true;
#else
        features["ENABLE_PRESS_CYLINDER"] = false;
#endif

#ifdef ENABLE_OMRON_E5
        features["ENABLE_OMRON_E5"] = true;
#else
        features["ENABLE_OMRON_E5"] = false;
#endif

#ifdef ENABLE_JOYSTICK
        features["ENABLE_JOYSTICK"] = true;
#else
        features["ENABLE_JOYSTICK"] = false;
#endif

#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
        features["ENABLE_AMPERAGE_BUDGET_MANAGER"] = true;
#else
        features["ENABLE_AMPERAGE_BUDGET_MANAGER"] = false;
#endif

#ifdef ENABLE_PROFILE_TEMPERATURE
        features["ENABLE_PROFILE_TEMPERATURE"] = true;
#else
        features["ENABLE_PROFILE_TEMPERATURE"] = false;
#endif

#ifdef ENABLE_PROFILE_PRESSURE
        features["ENABLE_PROFILE_PRESSURE"] = true;
#else
        features["ENABLE_PROFILE_PRESSURE"] = false;
#endif

#ifdef ENABLE_PROFILE_SIGNAL_PLOT
        features["ENABLE_PROFILE_SIGNAL_PLOT"] = true;
#else
        features["ENABLE_PROFILE_SIGNAL_PLOT"] = false;
#endif

#ifdef ENABLE_SAKO_VFD
        features["ENABLE_SAKO_VFD"] = true;
#else
        features["ENABLE_SAKO_VFD"] = false;
#endif

#ifdef ENABLE_DELTA_VFD
        features["ENABLE_DELTA_VFD"] = true;
#else
        features["ENABLE_DELTA_VFD"] = false;
#endif

#ifdef ENABLE_PROFILE_PRESSURE
        features["ENABLE_PROFILE_PRESSURE"] = true;
#else
        features["ENABLE_PROFILE_PRESSURE"] = false;
#endif

#ifdef ENABLE_INFLUXDB
        features["ENABLE_INFLUXDB"] = true;
#else
        features["ENABLE_INFLUXDB"] = false;
#endif
        JsonObject settings = doc["settings"].to<JsonObject>();
#ifdef HMI_NAME
        settings["name"] = HMI_NAME;
#endif
        ArduinoJson::serializeJson(doc, *response);
        request->send(response); });

#ifdef ENABLE_SERIAL_BRIDGE

    instance->server.on("/api/v1/methods", HTTP_GET, [this](AsyncWebServerRequest *request)
                        { this->getBridgeMethodsHandler(request); });

    AsyncCallbackJsonWebHandler *postMethodHandler = new AsyncCallbackJsonWebHandler("/api/v1/methods",
                                                                                     [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                     {
                                                                                         if (!json.is<JsonObject>() || !json["command"].is<String>())
                                                                                         {
                                                                                             request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid payload: Must be JSON object containing a 'command' string field.\"}");
                                                                                             return;
                                                                                         }

                                                                                         String cmdStr = json["command"].as<String>();
                                                                                         L_INFO("REST: Received method call command: %s", cmdStr.c_str());

                                                                                         Bridge *bridge = static_cast<Bridge *>(byId(COMPONENT_KEY_MB_BRIDGE));
                                                                                         if (!bridge)
                                                                                         {
                                                                                             L_ERROR("REST: Bridge component not found!");
                                                                                             request->send(500, "application/json", "{\"success\":false,\"error\":\"Bridge component not found\"}");
                                                                                             return;
                                                                                         }

                                                                                         CommandMessage msg;
                                                                                         if (!msg.parse(cmdStr))
                                                                                         {
                                                                                             L_ERROR("REST: Failed to parse command string: %s", cmdStr.c_str());
                                                                                             request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid command string format\"}");
                                                                                             return;
                                                                                         }

                                                                                         short result = bridge->onMessage(msg.id, msg.verb, msg.flags, msg.payload, this);
                                                                                         if (result == E_OK)
                                                                                         {
                                                                                             request->send(200, "application/json", "{\"success\":true,\"message\":\"Method executed successfully\"}");
                                                                                         }
                                                                                         else
                                                                                         {
                                                                                             L_ERROR("REST: Method execution failed with error %d", result);
                                                                                             request->send(500, "application/json", "{\"success\":false,\"error\":\"Method execution failed\"}");
                                                                                         }
                                                                                     });
    postMethodHandler->setMethod(HTTP_POST);
    instance->server.addHandler(postMethodHandler);
#endif

#ifdef ENABLE_WEBSERVER_FS_API
    // GET /api/v1/fs - Read JSON file
    instance->server.on("/api/v1/fs", HTTP_GET, [this](AsyncWebServerRequest *request)
                        {
                            if (!request->hasParam("file"))
                            {
                                request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing 'file' query parameter\"}");
                                return;
                            }

                            String filename = request->getParam("file")->value();
                            if (filename.isEmpty() || filename.indexOf("..") != -1)
                            {
                                request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid filename\"}");
                                return;
                            }

                            if (!filename.startsWith("/"))
                            {
                                filename = "/" + filename;
                            }

#ifdef ENABLE_LITTLEFS
                            if (!LittleFS.begin(true))
                            {
                                request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to mount filesystem\"}");
                                return;
                            }

                            File file = LittleFS.open(filename, "r");
                            if (!file)
                            {
                                request->send(404, "application/json", "{\"success\":false,\"error\":\"File not found\"}");
                                return;
                            }

                            String content = file.readString();
                            file.close();

                            request->send(200, "application/json", content);
#else
                            request->send(500, "application/json", "{\"success\":false,\"error\":\"LittleFS not enabled\"}");
#endif
                        });

    // POST /api/v1/fs - Write JSON file
    AsyncCallbackJsonWebHandler *writeFileHandler = new AsyncCallbackJsonWebHandler("/api/v1/fs",
                                                                                    [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                    {
                                                                                        if (!json.is<JsonObject>())
                                                                                        {
                                                                                            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: Expected an object.\"}");
                                                                                            return;
                                                                                        }

                                                                                        JsonObject jsonObj = json.as<JsonObject>();
                                                                                        if (!jsonObj["filename"].is<String>() || !jsonObj["content"].is<String>())
                                                                                        {
                                                                                            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing 'filename' or 'content' fields\"}");
                                                                                            return;
                                                                                        }

                                                                                        String filename = jsonObj["filename"].as<String>();
                                                                                        String content = jsonObj["content"].as<String>();

                                                                                        if (filename.isEmpty() || filename.indexOf("..") != -1)
                                                                                        {
                                                                                            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid filename\"}");
                                                                                            return;
                                                                                        }

                                                                                        if (!filename.startsWith("/"))
                                                                                        {
                                                                                            filename = "/" + filename;
                                                                                        }

#ifdef ENABLE_LITTLEFS
                                                                                        if (!LittleFS.begin(true))
                                                                                        {
                                                                                            request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to mount filesystem\"}");
                                                                                            return;
                                                                                        }

                                                                                        File file = LittleFS.open(filename, "w");
                                                                                        if (!file)
                                                                                        {
                                                                                            request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to create file\"}");
                                                                                            return;
                                                                                        }

                                                                                        size_t bytesWritten = file.print(content);
                                                                                        file.close();

                                                                                        if (bytesWritten == content.length())
                                                                                        {
                                                                                            request->send(200, "application/json", "{\"success\":true,\"message\":\"File written successfully\"}");

                                                                                            if (filename.endsWith("state.json"))
                                                                                            {
                                                                                                L_INFO("REST: State file updated, restoring state...");
                                                                                                this->restoreState();
                                                                                            }
                                                                                        }
                                                                                        else
                                                                                        {
                                                                                            request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to write complete file\"}");
                                                                                        }
#else
                                                                                        request->send(500, "application/json", "{\"success\":false,\"error\":\"LittleFS not enabled\"}");
#endif
                                                                                    });
    writeFileHandler->setMethod(HTTP_POST);
    instance->server.addHandler(writeFileHandler);
#endif

#endif
    return E_OK;
}

short PHApp::registerWebSocketHandlers(RESTServer *instance)
{

#if defined(ENABLE_WEBSERVER) && defined(ENABLE_WEBSOCKET)
    instance->registerWsCommandHandler("get_components", [this](AsyncWebSocketClient *client, JsonVariant &json)
                                       {
        JsonDocument doc;
        JsonArray componentsArray = doc.to<JsonArray>();

        for (size_t i = 0; i < this->components.size(); ++i)
        {
            Component* comp = this->components.at(i);
            if (!comp)
            {
                continue;
            }
            JsonObject compObj = componentsArray.add<JsonObject>();
            compObj["id"] = comp->id;
            compObj["name"] = comp->name;
            compObj["enabled"] = comp->enabled();
            
            JsonObject flagsObj = compObj["flags"].to<JsonObject>();
            flagsObj["run"] = comp->flags;
            flagsObj["network"] = comp->nFlags;
            flagsObj["feature"] = comp->fFlags;
        }
        JsonObject compObj = componentsArray.add<JsonObject>();
        compObj["id"] = id;
        compObj["name"] = "PHApp";
        compObj["enabled"] = true;
        
        JsonObject flagsObj = compObj["flags"].to<JsonObject>();
        flagsObj["run"] = flags;
        flagsObj["network"] = nFlags;
        flagsObj["feature"] = fFlags;
        
        JsonDocument responseDoc;
        responseDoc["type"] = "components";
        responseDoc["data"] = doc;

        String responseStr;
        serializeJson(responseDoc, responseStr);
        client->text(responseStr); });

#ifdef ENABLE_RUNTIME_STATE
    instance->registerWsCommandHandler("get_state", [this](AsyncWebSocketClient *client, JsonVariant &json)
                                       {
        if (!this->runtimeState) {
            JsonDocument errorDoc;
            errorDoc["type"] = "error";
            errorDoc["data"]["message"] = "RuntimeState component not found";
            String responseStr;
            serializeJson(errorDoc, responseStr);
            client->text(responseStr);
            return;
        }
        
        JsonDocument doc;
        updateRuntimeState();
        this->runtimeState->toJSON(doc.to<JsonVariant>());        
        JsonDocument responseDoc;
        responseDoc["type"] = "state";
        responseDoc["data"] = doc;

        String responseStr;
        serializeJson(responseDoc, responseStr);
        client->text(responseStr); });
#endif

#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
    // Binary inspector for AmperageBudgetManager
    instance->registerWsCommandHandler("get_amp_state", [this](AsyncWebSocketClient *client, JsonVariant &json)
                                       {
        Component* comp = this->byId(COMPONENT_KEY_AMPERAGE_BUDGET_MANAGER);
        if (!comp)
        {
            JsonDocument errorDoc;
            errorDoc["type"] = "error";
            errorDoc["data"]["message"] = "AmperageBudgetManager component not found";
            String responseStr;
            serializeJson(errorDoc, responseStr);
            client->text(responseStr);
            return;
        }

        AmperageBudgetManager* ampMgr = static_cast<AmperageBudgetManager*>(comp);
        
        // Allocate buffer large enough for StatePacket
        // We use a static size if possible or calculate max size
        const size_t MAX_PACKET_SIZE = sizeof(AmperageBudgetManager::StatePacket);
        uint8_t buffer[MAX_PACKET_SIZE];
        
        size_t size = ampMgr->getBinaryState(buffer, MAX_PACKET_SIZE);
        
        if (size > 0)
        {
            client->binary(buffer, size);
        }
        else
        {
             client->text("{\"type\":\"error\",\"data\":{\"message\":\"Failed to get binary state\"}}");
        } });
#endif

#endif

#ifdef ENABLE_RS485
    instance->registerWsCommandHandler("get_pending_ops", [this](AsyncWebSocketClient *client, JsonVariant &json)
                                       {
        RS485* rs485 = static_cast<RS485*>(this->byId(COMPONENT_KEY_RS485));
        if (!rs485) {
            JsonDocument errorDoc;
            errorDoc["type"] = "error";
            errorDoc["data"]["message"] = "RS485 component not found";
            String responseStr;
            serializeJson(errorDoc, responseStr);
            client->text(responseStr);
            return;
        }

        uint8_t slaveId = 0;
        E_MB_OpStatus statusFilter;
        E_MB_OpStatus* statusPtr = nullptr;

        if (json.is<JsonObject>()) {
            JsonObject jsonObj = json.as<JsonObject>();
            if (!jsonObj["slaveId"].isNull()) {
                slaveId = jsonObj["slaveId"].as<uint8_t>();
            }
            if (!jsonObj["status"].isNull()) {
                statusFilter = (E_MB_OpStatus)jsonObj["status"].as<int>();
                statusPtr = &statusFilter;
            }
        }

        const uint8_t MAX_OPS = 32;
        ModbusOperation* results[MAX_OPS];
        uint8_t count = rs485->modbus.getOperations(results, MAX_OPS, slaveId, 0, statusPtr);

        JsonDocument doc;
        JsonArray opsArray = doc.to<JsonArray>();

        for (uint8_t i = 0; i < count; ++i) {
            ModbusOperation* op = results[i];
            if (op) {
                JsonObject opObj = opsArray.add<JsonObject>();
                opObj["token"] = op->token;
                opObj["slaveId"] = op->slaveId;
                opObj["address"] = op->address;
                opObj["type"] = (int)op->type;
                opObj["value"] = op->value;
                opObj["quantity"] = op->quantity;
                opObj["status"] = (int)op->status;
                opObj["retries"] = op->retries;
                opObj["timestamp"] = op->timestamp;
                opObj["high_priority"] = op->isHighPriority();
                opObj["in_progress"] = op->isInProgress();
            }
        }

        JsonDocument responseDoc;
        responseDoc["type"] = "pending_ops";
        responseDoc["data"] = doc;

        String responseStr;
        serializeJson(responseDoc, responseStr);
        client->text(responseStr); });

    instance->registerWsCommandHandler("get_rtu_client_queue", [this](AsyncWebSocketClient *client, JsonVariant &json)
                                       {
        RS485* rs485 = static_cast<RS485*>(this->byId(COMPONENT_KEY_RS485));
        if (!rs485) {
            JsonDocument errorDoc;
            errorDoc["type"] = "error";
            errorDoc["data"]["message"] = "RS485 component not found";
            String responseStr;
            serializeJson(errorDoc, responseStr);
            client->text(responseStr);
            return;
        }

        // This part is a bit tricky as it requires accessing the underlying eModbus client queue.
        // For now, we will just return a placeholder. A proper implementation would require
        // modifications to ModbusRTU to expose this information safely.

        JsonDocument doc;
        doc["message"] = "Placeholder for RTU client queue. Implementation requires ModbusRTU modification.";

        JsonDocument responseDoc;
        responseDoc["type"] = "rtu_client_queue";
        responseDoc["data"] = doc;

        String responseStr;
        serializeJson(responseDoc, responseStr);
        client->text(responseStr); });

    instance->registerWsCommandHandler("get_rtu_stats", [this](AsyncWebSocketClient *client, JsonVariant &json)
                                       {
        RS485* rs485 = static_cast<RS485*>(this->byId(COMPONENT_KEY_RS485));
        if (!rs485) {
            JsonDocument errorDoc;
            errorDoc["type"] = "error";
            errorDoc["data"]["message"] = "RS485 component not found";
            String responseStr;
            serializeJson(errorDoc, responseStr);
            client->text(responseStr);
            return;
        }

        JsonDocument doc;
        doc["lost_packets"] = rs485->modbus.getErrorCount();
        doc["timeouts"] = rs485->modbus.getTimeoutCount();
        doc["crc_errors"] = rs485->modbus.getCrcErrorCount();
        doc["other_errors"] = rs485->modbus.getOtherErrorCount();
        doc["packets_sent"] = rs485->modbus.getTotalPacketsSent();
        doc["success_count"] = rs485->modbus.getSuccessCount();

        JsonDocument responseDoc;
        responseDoc["type"] = "rtu_stats";
        responseDoc["data"] = doc;

        String responseStr;
        serializeJson(responseDoc, responseStr);
        client->text(responseStr); });
#endif
    return E_OK;
}

#ifdef ENABLE_WEBSERVER_WIFI_SETTINGS

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
    JsonArray plotsArray = doc.to<JsonArray>();

    for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; ++i)
    {
        SignalPlot *profile = this->signalPlots[i];
        if (profile)
        {
            JsonObject plotObj = plotsArray.add<JsonObject>();
            plotObj["slot"] = i;
            plotObj["name"] = profile->name;
            plotObj["duration"] = profile->getDuration();
            plotObj["status"] = (int)profile->getCurrentStatus();
            plotObj["enabled"] = profile->enabled();
            plotObj["elapsed"] = profile->getElapsedMs();
            plotObj["remaining"] = profile->getRemainingTime();

            JsonArray pointsArray = plotObj["controlPoints"].to<JsonArray>();
            const S_SignalControlPoint *points = profile->getControlPoints();
            uint8_t numPoints = profile->getNumControlPoints();

            for (uint8_t j = 0; j < numPoints; ++j)
            {
                JsonObject pointObj = pointsArray.add<JsonObject>();
                const S_SignalControlPoint &cp = points[j];
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

            JsonArray childrenArray = plotObj["children"].to<JsonArray>();
            const ushort *children = profile->getChildren();
            for (uint8_t j = 0; j < MAX_PLOTS; j++)
            {
                childrenArray.add(children[j]);
            }
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
#ifdef ENABLE_PROFILE_SIGNAL_PLOT
    if (!json.is<JsonObject>())
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON payload: must be an object.\"}");
        return;
    }
    JsonObject jsonObj = json.as<JsonObject>();

    if (updateSignalPlot(jsonObj, slot))
    {
        if (saveSignalPlotsToJson())
        {
            L_INFO("REST: SignalPlot updated and all profiles saved to JSON successfully after update.");
            request->send(200, "application/json", "{\"success\":true, \"message\":\"SignalPlot profile updated and saved.\"}");
        }
        else
        {
            L_ERROR("REST: SignalPlot slot %d updated, but failed to save all profiles to JSON.", slot);
            request->send(500, "application/json", "{\"success\":true, \"message\":\"SignalPlot profile updated but failed to save configuration.\"}");
        }
    }
    else
    {
        L_ERROR("REST: Failed to update SignalPlot slot %d from JSON.", slot);
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Failed to load SignalPlot profile data. Check format and values.\"}");
    }
#endif
}
#endif // ENABLE_PROFILE_SIGNAL_PLOT

void PHApp::getBridgeMethodsHandler(AsyncWebServerRequest *request)
{
#ifdef ENABLE_SERIAL_BRIDGE
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray methodsArray = doc.to<JsonArray>();

    Bridge *bridge = static_cast<Bridge *>(byId(COMPONENT_KEY_MB_BRIDGE));
    if (!bridge)
    {
        L_ERROR(F("REST: Bridge component not found!"));
        request->send(500, "application/json", "{\"success\":false,\"error\":\"Bridge component not found\"}");
        return;
    }

    const std::vector<SComponentInfo *> &componentList = bridge->getComponentList();
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
    ArduinoJson::serializeJson(doc, *response);
    request->send(response);
#endif
}

#include "config.h" // Application configuration
#include "RestServer.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <esp_cpu.h>
#include <esp_heap_caps.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <AsyncWebSocket.h>
#include "esp_system.h"

#include <Component.h>
#include <components/RS485.h>
#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>

#include <Logger.h>
#include "enums.h"

// Moved mountLittleFS definition here
short RESTServer::mountLittleFS()
{
#ifdef ENABLE_LITTLEFS
    Log.infoln("Attempting to mount LittleFS...");
    if (!LittleFS.begin(true)) // true = format_if_failed
    {
        Log.errorln("LittleFS mount failed (even after attempting format)! Web files will not be available.");
        return E_INVALID_PARAMETER;
    }
    else
    {
        Log.infoln("LittleFS mounted successfully.");
        return E_OK;
    }
#else
    Log.warningln("LittleFS is not enabled in this build. Mount skipped.");
    return E_NOT_SUPPORTED; // Or E_OK if not enabling it is not an error condition for the caller
#endif
}

static const int DEFAULT_WEBSOCKET_PAGE_SIZE = 20;

// Swagger UI HTML template
const char SWAGGER_UI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Modbus REST API Documentation</title>
  <link rel="stylesheet" type="text/css" href="https://cdn.jsdelivr.net/npm/swagger-ui-dist@4.5.0/swagger-ui.css" />
  <style>
    html { box-sizing: border-box; overflow: -moz-scrollbars-vertical; overflow-y: scroll; }
    *, *:before, *:after { box-sizing: inherit; }
    body { margin: 0; background: #fafafa; }
    .topbar { display: none; }
  </style>
</head>
<body>
  <div id="swagger-ui"></div>

  <script src="https://cdn.jsdelivr.net/npm/swagger-ui-dist@4.5.0/swagger-ui-bundle.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/swagger-ui-dist@4.5.0/swagger-ui-standalone-preset.js"></script>
  <script>
    window.onload = function() {
      const ui = SwaggerUIBundle({
        url: "/api/swagger.yaml",
        dom_id: '#swagger-ui',
        deepLinking: true,
        presets: [
          SwaggerUIBundle.presets.apis,
          SwaggerUIStandalonePreset
        ],
        layout: "BaseLayout",
        defaultModelsExpandDepth: -1,
        displayRequestDuration: true
      });
      window.ui = ui;
    };
  </script>
</body>
</html>
)rawliteral";

/**
 * @file RestServer.cpp
 * @brief REST Server implementation for Modbus communication
 *
 * This file implements the REST Server for Modbus communication.
 * It handles HTTP requests for reading and writing to Modbus registers.
 *
 * @todo : https://chatgpt.com/share/680c9ff3-4430-8001-bd79-d57f1d5fd3be (Protobuf / OpenAPI / AsyncAPI, RFC 6455 )
 *
 */

RESTServer::RESTServer(const IPAddress &ip, uint16_t port, ModbusTCP *manager, Component *owner)
    : Component("RESTServer", COMPONENT_KEY_REST_SERVER, COMPONENT_DEFAULT, owner),
      server(port),
#ifdef ENABLE_WEBSOCKET
      ws("/ws"), // Initialize WebSocket server on path /ws
#endif
      modbusManager(manager)
{
    if (!modbusManager)
    {
        Log.warningln("RESTServer initialized without a valid ModbusManager pointer!");
    }
}

short RESTServer::setup()
{
    setupRoutes();
#ifdef ENABLE_WEBSOCKET
    setupWebSocket();       // Set up WebSocket event handlers
    server.addHandler(&ws); // Attach WebSocket handler to the web server
#endif
    server.begin();

#ifdef ENABLE_LITTLEFS
    if (mountLittleFS() != E_OK)
    {
        // Optional: handle severe error, though mountLittleFS logs it.
    }
    File file = LittleFS.open("/index.html", "r");
    if (file)
    {
        String header = "";
        for (int i = 0; i < 30 && file.available(); i++)
        {
            header += (char)file.read();
        }
        file.close();
        Log.infoln("✅ index.html opened from LittleFS! File size: %d bytes", file.size());
    }
    else
    {
        Log.errorln("✗ index.html cannot be opened from LittleFS! File may be missing.");
    }
#endif
    return E_OK;
}

short RESTServer::onRun()
{
#ifdef ENABLE_LITTLEFS
    // Ensure LittleFS is mounted before trying to list files
    File rootTest = LittleFS.open("/");
    if (!rootTest)
    { // If root can't be opened, try to mount
        Log.infoln("Root not open, attempting to mount LittleFS in onRun...");
        if (mountLittleFS() != E_OK)
        {
            return E_INVALID_PARAMETER; // Mount failed, cannot proceed with listing
        }
    }
    else
    {
        rootTest.close(); // Close if it was opened successfully
    }

    File root = LittleFS.open("/");
    if (!root)
    {
        Log.errorln("Still failed to open root directory in LittleFS");
        return E_INVALID_PARAMETER;
    }

    if (!root.isDirectory())
    {
        Log.errorln("Root is not a directory in LittleFS");
        return E_INVALID_PARAMETER;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Log.infoln("  FS: DIR : %s", file.name());
        }
        else
        {
            Log.infoln("  FS: FILE: %s, SIZE: %d", file.name(), file.size());
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();
#else
    doc["error"] = "LittleFS is not enabled in this build.";
#endif
    return E_OK;
}

RESTServer::~RESTServer()
{
    server.end();
}

short RESTServer::loop()
{
#ifdef ENABLE_WEBSOCKET
    ws.cleanupClients(); // Periodically clean up disconnected WebSocket clients
#endif
    return E_OK;
}

void RESTServer::setupRoutes()
{
    // Set up CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
#ifdef ENABLE_LITTLEFS
        if (this->mountLittleFS() != E_OK) {
            Log.errorln("HANDLER /: LittleFS mount failed. Cannot serve index.html.");
            request->send(500, "text/plain", "Internal Server Error: Filesystem not available");
            return;
        }
#endif
        File file = LittleFS.open("/index.html", "r");
        if (file) {
            Log.infoln("HANDLER: /index.html opened successfully. Size: %d", file.size());
            String content = file.readString();
            file.close();
            request->send(200, "text/html", content);
        } else {
            Log.errorln("HANDLER: Failed to open /index.html for / request.");
            request->send(200, "text/html", 
                "<html><body><h1>ESP32 Web Server</h1><p>index.html file not found in filesystem (handler /)</p></body></html>"); // Modified error
        } });

    server.on("/index.html", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
#ifdef ENABLE_LITTLEFS
                if (this->mountLittleFS() != E_OK) {
                    Log.errorln("HANDLER /: LittleFS mount failed. Cannot serve index.html.");
                    request->send(500, "text/plain", "Internal Server Error: Filesystem not available");
                    return;
                }
#endif
        File file = LittleFS.open("/index.html", "r");
        if (file) {
            String content = file.readString();
            file.close();
            request->send(200, "text/html", content);
        } else {
            Log.errorln("HANDLER: Failed to open /index.html for /index.html request.");
            request->send(200, "text/html", 
                "<html><body><h1>ESP32 Web Server</h1><p>index.html file not found in filesystem (handler /index.html)</p></body></html>"); // Modified error
        } });

    server.on("/assets/*", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
#ifdef ENABLE_LITTLEFS
        if (this->mountLittleFS() != E_OK) {
            Log.errorln("HANDLER /assets/*: LittleFS mount failed. Cannot serve index.html.");
            request->send(500, "text/plain", "Internal Server Error: Filesystem not available");
            return;
        }
#endif
        String path = request->url();
        String filePath = path;
        
        File file = LittleFS.open(filePath, "r");
        if (file) {
            String contentType = "application/octet-stream";
            if (path.endsWith(".js")) contentType = "application/javascript";
            else if (path.endsWith(".css")) contentType = "text/css";
            
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, filePath, contentType);
            request->send(response);
        } else {
            Log.errorln("Failed to open asset: %s", filePath.c_str());
            request->send(404, "text/plain", "File not found: " + path);
        } });

#ifdef ENABLE_MODBUS_TCP
    server.on("/api/v1/modbus/rtu/queue", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->getRtuOperationQueueHandler(request); });
    server.on("/api/v1/mappings", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->getMappingsHandler(request); });
    server.on("/api/v1/coils", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->getCoilsHandler(request); });
    server.on("/api/v1/registers", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->getRegistersHandler(request); });
    server.on("/api/v1/coils", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->setCoilQueryHandler(request); });
    server.on("/api/v1/registers", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
                  String url = request->url(); // e.g., "/api/v1/registers/20?value=42"
                  String prefix = "/api/v1/registers";
                  int prefixEnd = url.indexOf(prefix) + prefix.length();
                  if (prefixEnd < prefix.length())
                  {
                      request->send(400, "application/json", "{\"success\":false,\"error\":\"Malformed URL (prefix mismatch)\"}");
                      return;
                  }
                  if (url.charAt(prefixEnd) != '/')
                  {
                      request->send(400, "application/json", "{\"success\":false,\"error\":\"Malformed URL (expected /<address>)\"}");
                      return;
                  }
                  int addressStart = prefixEnd + 1; // Position after the slash

                  int addressEnd = url.length(); // Default to end of string
                  int queryPos = url.indexOf('?', addressStart);
                  int slashPos = url.indexOf('/', addressStart);
                  if (queryPos != -1)
                  {
                      addressEnd = queryPos; // End before query string
                  }
                  if (slashPos != -1 && slashPos < addressEnd)
                  {
                      addressEnd = slashPos; // End before next slash (if any)
                  }

                  String addressStr = url.substring(addressStart, addressEnd);
                  int address = addressStr.toInt();
                  if (address == 0 && addressStr != "0")
                  {
                      request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid address format in URL\"}");
                      return;
                  }
                  if (!request->hasParam("value"))
                  {
                      request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing 'value' query parameter\"}");
                      return;
                  }
                  String valueStr = request->getParam("value")->value();
                  char *endptr;
                  long val_long = strtol(valueStr.c_str(), &endptr, 10);
                  if (endptr == valueStr.c_str() || *endptr != '\0' || val_long < -32768 || val_long > 32767)
                  {
                      request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid integer format or out of range for 'value' query parameter.\"}");
                      return;
                  }
                  short value = (short)val_long;
                  bool success = false;
                  short writeResult = E_INVALID_PARAMETER;
                  if (modbusManager)
                  {
                      MB_Registers *mapping = modbusManager->findMappingForAddress(address, E_FN_CODE::FN_WRITE_HOLD_REGISTER);
                      if (mapping)
                      {
                          Component *target = modbusManager->findComponentForAddress(address);
                          if (target)
                          {
                              writeResult = target->mb_tcp_write(mapping, value);
                          }
                          else
                          {
                              Log.errorln("REST: (Inline) setRegisterQueryHandler - No component found for address %d and componentId %d", address, mapping->componentId);
                              writeResult = E_INVALID_PARAMETER; // Address not found
                          }
                          if (writeResult == E_OK)
                          {
                              success = true;
                              // Log.verboseln("REST: (Inline) Set Register %d to %d via ModbusManager (Query)", address, value);
                          }
                          else
                          {
                              Log.errorln("REST: (Inline) Set Register %d failed via ModbusManager, error code: %d (Query)", address, writeResult);
                          }
                      }
                      else
                      {
                          writeResult = E_INVALID_PARAMETER; // Address not found
                      }
                  }
                  else
                  {
                      Log.warningln("REST: (Inline) setRegisterQueryHandler - No ModbusManager available.");
                      writeResult = E_INVALID_PARAMETER; // Cannot proceed
                  }

                  if (success)
                  {
                      AsyncResponseStream *response = request->beginResponseStream("application/json");
                      JsonDocument responseDoc;
                      responseDoc["success"] = true;
                      responseDoc["address"] = address;
                      responseDoc["value"] = value;
                      serializeJson(responseDoc, *response);
                      request->send(response);
                  }
                  else
                  {
                      int httpStatus = (writeResult == E_INVALID_PARAMETER) ? 404 : 400;
                      String errorMsg = (writeResult == E_INVALID_PARAMETER) ? "Address not found or manager unavailable" : "Failed to write register";
                      request->send(httpStatus, "application/json", "{\"success\":false,\"error\":\"" + errorMsg + "\"}");
                  }
                  // --- End of inlined logic ---
              });
#endif

    server.on("/api/v1/system/info", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->getSystemInfoHandler(request); });
    server.on("/api/v1/system/log-level", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        if (request->hasParam("level")) {
            this->setLogLevelHandler(request);
        } else {
            this->getLogLevelHandler(request);
        } });

    server.on("/api/v1/fs/list", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->listFsHandler(request); });

    // Handle OPTIONS requests for CORS
    server.onNotFound([](AsyncWebServerRequest *request)
                      {
    String url = request->url();
        // Log.errorln("REST: Not Found - Method: %s, URL: %s", request->methodToString(), url.c_str());
#ifdef ENABLE_LITTLEFS
        if (request->method() == HTTP_GET) {
            String path = url;
            // Check if the file exists in LittleFS
            if (LittleFS.exists(path)) {
                Log.errorln("File exists in LittleFS but handler not triggered: %s", path.c_str());
            } else {
                File root = LittleFS.open("/");
                if (root && root.isDirectory()) {
                    File file = root.openNextFile();
                    while (file) {
                        if (file.isDirectory()) {
                            // Log.infoln("  DIR: %s", file.name());
                        } else {
                            // Log.infoln("  FILE: %s, SIZE: %d", file.name(), file.size());
                        }
                        file = root.openNextFile();
                    }
                } else {
                    Log.errorln("Failed to open root directory");
                }
            }
        }
#endif
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404, "text/plain", "Not found: " + url);
        } });
    // Handle OPTIONS requests for CORS preflight (API routes)
    server.on("/api/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request)
              {
    // Send 200 OK for OPTIONS requests - CORS headers are added by DefaultHeaders
        request->send(200); });

    // Handle OPTIONS requests for CORS preflight (root/assets etc.)
    server.on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request)
              { request->send(200); });

    // Handle not found AFTER specific routes and OPTIONS
    server.onNotFound([](AsyncWebServerRequest *request)
                      {
    String url = request->url();
#ifdef ENABLE_LITTLEFS
        if (request->method() == HTTP_GET) {
            String path = url;
            // Check if the file exists in LittleFS
            if (LittleFS.exists(path)) {
                Log.errorln("File exists in LittleFS but handler not triggered: %s", path.c_str());
            } else {
                File root = LittleFS.open("/");
                if (root && root.isDirectory()) {
                    File file = root.openNextFile();
                    while (file) {
                        if (file.isDirectory()) {
                            // Log.infoln("  DIR: %s", file.name());
                        } else {
                            // Log.infoln("  FILE: %s, SIZE: %d", file.name(), file.size());
                        }
                        file = root.openNextFile();
                    }
                } else {
                    Log.errorln("Failed to open root directory");
                }
            }
        }
#endif
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404, "text/plain", "Not found: " + url);
        } });
}

void RESTServer::getSystemInfoHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["version"] = "3ce112f";
    doc["board"] = BOARD_NAME;
    doc["uptime"] = millis() / 1000;
    doc["timestamp"] = millis();
    doc["freeHeapKb"] = ESP.getFreeHeap() / 1024.0;
    doc["maxFreeBlockKb"] = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) / 1024.0;
    doc["cpuTicks"] = (uint32_t)esp_cpu_get_ccount();
    doc["loopDurationMs"] = owner->getLoopDurationUs();
    // --- Calculate Average CPU Load ---
    float cpuLoadPercent = -1.0; // Default value if stats are unavailable
    // Add CPU load to JSON, format to 1 decimal place if valid, else null
    if (cpuLoadPercent >= 0.0)
    {
        char loadStr[10];
        snprintf(loadStr, sizeof(loadStr), "%.1f", cpuLoadPercent);
        doc["cpuLoadPercent"] = loadStr; // Store as string for consistent formatting
    }
    else
    {
        doc["cpuLoadPercent"] = nullptr;
    }
    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::getCoilsHandler(AsyncWebServerRequest *request)
{
    if (request->hasParam("address"))
    {
        int address = request->getParam("address")->value().toInt();
        Log.verboseln("REST: getCoilsHandler - Request for single address: %d", address);

        bool value = false;
        bool found = false;
        if (modbusManager)
        {
            Component *target = modbusManager->findComponentForAddress(address);
            if (target)
            {
                value = (target->mb_tcp_read(address) != 0);
                found = true;
            }
            else
            {
                Log.warningln("REST: getCoilHandler - No component found for address %d", address);
            }
        }
        else
        {
            Log.warningln("REST: getCoilHandler - No ModbusManager available.");
        }

        if (found)
        {
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            JsonDocument doc;
            doc["address"] = address;
            doc["value"] = value;

            serializeJson(doc, *response);
            request->send(response);
        }
        else
        {
            request->send(404, "application/json", "{\"success\":false,\"error\":\"Address not found or manager unavailable\"}");
        }
        return; // Handled single request
    }

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray coilsArray = doc["coils"].to<JsonArray>();                        // Use updated syntax
    const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings(); // Use new getter
    for (const auto &info : mappings)                                           // Iterate MB_Registers
    {
        if (info.type != FN_READ_COIL && info.type != FN_WRITE_COIL)
        {
            continue;
        }
        Component *component = owner->byId(info.componentId);
        if (component)
        {
            for (short i = 0; i < info.count; ++i)
            {
                short currentAddr = info.startAddress + i;
                uint16_t coilValue = (uint16_t)component->mb_tcp_read(const_cast<MB_Registers *>(&info));
                JsonObject coilObj = coilsArray.add<JsonObject>();
                coilObj["address"] = currentAddr;
                coilObj["value"] = coilValue;
                coilObj["name"] = info.name;
                coilObj["component"] = component->name;
                coilObj["id"] = component->id;
                coilObj["type"] = info.type;
                coilObj["access"] = info.access;
                coilObj["flags"] = component->flags;
                coilObj["group"] = info.group;
            }
        }
    }
    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::getCoilHandler(AsyncWebServerRequest *request)
{
    String url = request->url();
    int lastSlash = url.lastIndexOf('/');
    if (lastSlash == -1 || lastSlash == url.length() - 1)
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing address in URL\"}");
        return;
    }
    String addressStr = url.substring(lastSlash + 1);
    int address = addressStr.toInt();
    if (address == 0 && addressStr != "0")
    { // Basic check for invalid integer conversion
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid address format\"}");
        return;
    }

    bool value = false; // Default/dummy value
    bool found = false;

    // --- Get real data if manager exists ---
    if (modbusManager)
    {
        Component *target = modbusManager->findComponentForAddress(address);
        if (target)
        {
            value = (target->mb_tcp_read(address) != 0);
            found = true;
        }
        else
        {
            Log.warningln("REST: getCoilHandler - No component found for address %d", address);
            // If not found, send 404
            request->send(404, "application/json", "{\"success\":false,\"error\":\"Address not found\"}");
            return; // Stop processing
        }
    }
    else
    {
        Log.warningln("REST: getCoilHandler - No ModbusManager available.");
        // If no manager, send 500 or other error
        request->send(500, "application/json", "{\"success\":false,\"error\":\"ModbusManager not available\"}");
        return; // Stop processing
    }

    // Create JSON response ONLY if found
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["address"] = address;
    doc["value"] = value;

    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::setCoilQueryHandler(AsyncWebServerRequest *request)
{
    Log.verboseln("REST: setCoilQueryHandler invoked for URL: %s", request->url().c_str());

    String url = request->url();     // e.g., "/api/v1/coils/51?value=1"
    String prefix = "/api/v1/coils"; // NOTE: No trailing slash
    int prefixEnd = url.indexOf(prefix) + prefix.length();
    // Check if prefix is immediately followed by '/' and then the address
    if (prefixEnd < prefix.length() || url.charAt(prefixEnd) != '/')
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Malformed URL (expected /<address>)\"}");
        return;
    }
    int addressStart = prefixEnd + 1; // Position after the slash
    int addressEnd = url.length();    // Default to end of string
    int queryPos = url.indexOf('?', addressStart);
    int slashPos = url.indexOf('/', addressStart);

    if (queryPos != -1)
    {
        addressEnd = queryPos; // End before query string
    }
    if (slashPos != -1 && slashPos < addressEnd)
    {
        addressEnd = slashPos; // End before next slash (if any)
    }

    String addressStr = url.substring(addressStart, addressEnd);

    int address = addressStr.toInt();
    if (address == 0 && addressStr != "0")
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid address format in URL\"}");
        return;
    }
    if (!request->hasParam("value"))
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing 'value' query parameter\"}");
        return;
    }
    String valueStr = request->getParam("value")->value();
    bool value;
    if (valueStr == "1" || valueStr.equalsIgnoreCase("true"))
    {
        value = true;
    }
    else if (valueStr == "0" || valueStr.equalsIgnoreCase("false"))
    {
        value = false;
    }
    else
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid 'value' query parameter. Use 1, 0, true, or false.\"}");
        return;
    }
    bool success = false;
    short writeResult = E_INVALID_PARAMETER;
    if (modbusManager)
    {
        MB_Registers *mapping = modbusManager->findMappingForAddress(address, E_FN_CODE::FN_WRITE_COIL);
        if (mapping)
        {
            Component *target = modbusManager->findComponentForAddress(address);
            if (target)
            {
                Log.verboseln("REST: setCoilQueryHandler - Writing value %d to address %d - | Component: %s (ID: %d)", value, address, target->name.c_str(), target->id);
                writeResult = target->mb_tcp_write(mapping, value ? 1 : 0);
            }
            else
            {
                Log.errorln("REST: (Inline) setRegisterQueryHandler - No component found for address %d and componentId %d", address, mapping->componentId);
                writeResult = E_INVALID_PARAMETER; // Address not found
            }
            if (writeResult == E_OK)
            {
                success = true;
            }
            else
            {
                Log.errorln("REST: Set Coil %d failed via ModbusManager, error code: %d (Query)", address, writeResult);
            }
        }
        else
        {
            Log.warningln("REST: setCoilQueryHandler - No mapping found for address %d", address);
            writeResult = E_INVALID_PARAMETER; // Address not found
        }
    }
    else
    {
        Log.warningln("REST: setCoilQueryHandler - No ModbusManager available.");
        writeResult = E_INVALID_PARAMETER; // Cannot proceed
    }

    // Create JSON response based on success
    if (success)
    {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        JsonDocument responseDoc;
        responseDoc["success"] = true;
        responseDoc["address"] = address;
        responseDoc["value"] = value;
        serializeJson(responseDoc, *response);
        request->send(response);
    }
    else
    {
        // Map internal error to HTTP status
        int httpStatus = (writeResult == E_INVALID_PARAMETER) ? 404 : 400;
        String errorMsg = (writeResult == E_INVALID_PARAMETER) ? "Address not found or manager unavailable" : "Failed to write coil";
        request->send(httpStatus, "application/json", "{\"success\":false,\"error\":\"" + errorMsg + "\"}");
    }
}

void RESTServer::getRegistersHandler(AsyncWebServerRequest *request)
{
    if (request->hasParam("address"))
    {
        int address = request->getParam("address")->value().toInt();

        // Use helper to check if address exists and get its data indirectly
        JsonDocument tempDoc;
        JsonArray tempArray = tempDoc.to<JsonArray>();
        _buildRegistersJson(tempArray, address);
        if (tempArray.size() > 0)
        {
            // Address found, extract data from the temporary array
            JsonObject regData = tempArray[0].as<JsonObject>();
            uint16_t value = regData["value"].as<uint16_t>(); // Extract value
            // Optionally extract other fields like name, error etc. if needed
            // short error = regData["error"].as<short>();

            // Build the simple response object required by the API spec
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            JsonDocument responseDoc;
            responseDoc["address"] = address;
            responseDoc["value"] = value;
            serializeJson(responseDoc, *response);
            request->send(response);
        }
        else
        {
            // Address not found by the helper
            request->send(404, "application/json", "{\"success\":false,\"error\":\"Address not found or manager unavailable\"}");
        }
        return; // Handled single request
    }

    // --- Handle request for all registers --- (This part remains the same)
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray registersArray = doc["registers"].to<JsonArray>();
    _buildRegistersJson(registersArray); // Call helper for all registers
    serializeJson(doc, *response);
    request->send(response);
}

// Helper function to build JSON for registers (all or specific)
void RESTServer::_buildRegistersJson(JsonArray &registersArray, int specificAddress)
{
    const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
    bool foundSpecific = false; // Flag to track if the specific address was found
    for (const auto &info : mappings)
    {
        // Skip non-register types
        if (info.type != E_FN_CODE::FN_READ_HOLD_REGISTER &&
            info.type != E_FN_CODE::FN_READ_INPUT_REGISTER && // Also include input registers
            info.type != E_FN_CODE::FN_WRITE_HOLD_REGISTER &&
            info.type != E_FN_CODE::FN_WRITE_MULT_REGISTERS)
        {
            continue;
        }

        Component *component = owner->byId(info.componentId);
        if (!component)
        {
            // Log.warningln("REST: _buildRegistersJson - Component not found for ID %d", info.componentId);
            continue; // Skip if component lookup fails
        }

        for (short i = 0; i < info.count; ++i)
        {
            short currentAddr = info.startAddress + i;

            // If a specific address is requested, skip if it doesn't match
            if (specificAddress != -1 && currentAddr != specificAddress)
            {
                continue;
            }

            // If we are here and specificAddress is set, we found it
            if (specificAddress != -1)
            {
                foundSpecific = true;
            }

            // CORRECTED: Use the mapping info (&info) for read/error calls
            uint16_t regValue = (uint16_t)component->mb_tcp_read(const_cast<MB_Registers *>(&info));
            short error = component->mb_tcp_error(const_cast<MB_Registers *>(&info));

            JsonObject regObj = registersArray.add<JsonObject>();
            regObj["error"] = error;
            regObj["address"] = currentAddr;
            regObj["value"] = regValue;
            regObj["name"] = info.name;
            regObj["component"] = component->name;
            regObj["id"] = component->id;
            regObj["type"] = (int)info.type;     // Cast enum to int
            regObj["access"] = (int)info.access; // Cast enum to int
            regObj["slaveId"] = info.slaveId;
            regObj["flags"] = component->flags;
            regObj["group"] = info.group;

            // If we were looking for a specific address and found it, we can stop
            if (foundSpecific)
            {
                return; // Exit the function early
            }
        }
        // If specificAddress was requested but not found within this mapping's range, continue to the next mapping
    }
    // If specificAddress was requested but not found after checking all mappings, the array will remain empty (or unchanged if called for all).
}

void RESTServer::listFsHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray filesArray = doc.to<JsonArray>();

#ifdef ENABLE_LITTLEFS
    mountLittleFS();
    File root = LittleFS.open("/");
    if (!root)
    {
        Log.errorln("Failed to open root directory in LittleFS");
        doc["error"] = "Failed to open root directory";
        serializeJson(doc, *response);
        request->send(response);
        return;
    }
    if (!root.isDirectory())
    {
        Log.errorln("Root is not a directory in LittleFS");
        doc["error"] = "Root is not a directory";
        serializeJson(doc, *response);
        request->send(response);
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        JsonObject fileObj = filesArray.add<JsonObject>();
        fileObj["name"] = String(file.name());
        if (file.isDirectory())
        {
            fileObj["type"] = "directory";
            fileObj["size"] = 0;
        }
        else
        {
            fileObj["type"] = "file";
            fileObj["size"] = file.size();
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();
#else
    doc["error"] = "LittleFS is not enabled in this build.";
#endif

    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::getRegisterHandler(AsyncWebServerRequest *request)
{
    String url = request->url();
    int lastSlash = url.lastIndexOf('/');
    if (lastSlash == -1 || lastSlash == url.length() - 1)
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing address in URL\"}");
        return;
    }
    String addressStr = url.substring(lastSlash + 1);
    // String addressStr = request->pathArg(0); // Removed regex path arg
    int address = addressStr.toInt();
    if (address == 0 && addressStr != "0")
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid address format\"}");
        return;
    }

    int value = 0; // Default/dummy value
    bool found = false;

    // --- Get real data if manager exists ---
    if (modbusManager)
    {
        Component *target = modbusManager->findComponentForAddress(address);
        if (target)
        {
            value = target->mb_tcp_read(address);
            found = true;
        }
        else
        {
            Log.warningln("REST: getRegisterHandler - No component found for address %d", address);
            // If not found, send 404
            request->send(404, "application/json", "{\"success\":false,\"error\":\"Address not found\"}");
            return; // Stop processing
        }
    }
    else
    {
        Log.warningln("REST: getRegisterHandler - No ModbusManager available.");
        // If no manager, send 500 or other error
        request->send(500, "application/json", "{\"success\":false,\"error\":\"ModbusManager not available\"}");
        return; // Stop processing
    }

    // Create JSON response ONLY if found
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["address"] = address;
    doc["value"] = value;

    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::getLogLevelHandler(AsyncWebServerRequest *request)
{
    int currentLevel = Log.getLevel();
    String levelStr;

    // Map level to string
    switch (currentLevel)
    {
    case LOG_LEVEL_SILENT:
        levelStr = "none";
        break;
    case LOG_LEVEL_ERROR:
        levelStr = "error";
        break;
    case LOG_LEVEL_WARNING:
        levelStr = "warning";
        break;
    case LOG_LEVEL_NOTICE:
        levelStr = "notice";
        break;
    case LOG_LEVEL_TRACE:
        levelStr = "trace";
        break;
    case LOG_LEVEL_VERBOSE:
        levelStr = "verbose";
        break;
    default:
        levelStr = "unknown";
        break;
    }

    // Return current level
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["success"] = true;
    doc["level"] = levelStr;
    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::setLogLevelHandler(AsyncWebServerRequest *request)
{
    if (!request->hasParam("level"))
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing 'level' parameter\"}");
        return;
    }
    String levelStr = request->getParam("level")->value();
    short newLevel = LOG_LEVEL_VERBOSE;
    if (levelStr == "none")
        newLevel = LOG_LEVEL_SILENT;
    else if (levelStr == "error")
        newLevel = LOG_LEVEL_ERROR;
    else if (levelStr == "info")
        newLevel = LOG_LEVEL_INFO;
    else if (levelStr == "warning")
        newLevel = LOG_LEVEL_WARNING;
    else if (levelStr == "notice")
        newLevel = LOG_LEVEL_NOTICE;
    else if (levelStr == "trace")
        newLevel = LOG_LEVEL_TRACE;
    else if (levelStr == "verbose")
        newLevel = LOG_LEVEL_VERBOSE;
    else
    {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid log level\"}");
        return;
    }
    Log.setLevel(newLevel);
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["success"] = true;
    doc["level"] = levelStr;
    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::getMappingsHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray mappingsArray = doc.to<JsonArray>();

    const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
    for (const auto &info : mappings)
    {
        JsonObject mapObj = mappingsArray.add<JsonObject>();
        mapObj["name"] = info.name;
        mapObj["componentId"] = info.componentId;
        Component *component = owner->byId(info.componentId);
        if (component)
        {
            mapObj["componentName"] = component->name;
        }
        else
        {
            mapObj["componentName"] = "Unknown";
        }
        mapObj["type"] = (int)info.type;
        mapObj["startAddress"] = info.startAddress;
        mapObj["count"] = info.count;
        mapObj["access"] = (int)info.access;
        mapObj["slaveId"] = info.slaveId;
        mapObj["group"] = info.group;
    }
    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::getRtuOperationQueueHandler(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;

#ifdef ENABLE_RS485
    if (owner->rs485)
    {
        ModbusRTU *rtuManager = &(owner->rs485->modbus);
        if (rtuManager)
        {
            JsonObject queueInfo = doc.to<JsonObject>();
            uint8_t opCount = rtuManager->getOperationCount();
            queueInfo["count"] = opCount;
            queueInfo["maxSize"] = MAX_PENDING_OPERATIONS;

            JsonArray queueArray = queueInfo["queue"].to<JsonArray>(); // Use new syntax
            const ModbusOperation *opQueue = rtuManager->getOperationQueue();

            for (int i = 0; i < MAX_PENDING_OPERATIONS; ++i)
            {
                if (TEST(opQueue[i].flags, OP_USED_BIT))
                {
                    const ModbusOperation &op = opQueue[i];
                    JsonObject opObj = queueArray.add<JsonObject>();
                    opObj["index"] = i;
                    opObj["token"] = op.token;
                    opObj["slaveId"] = op.slaveId;
                    opObj["address"] = op.address;
                    opObj["type"] = (int)op.type;
                    opObj["value"] = op.value;
                    opObj["quantity"] = op.quantity;
                    opObj["status"] = (int)op.status;
                    opObj["retries"] = op.retries;
                    opObj["timestamp"] = op.timestamp;

                    // Add individual flag evaluations
                    JsonObject flagsObj = opObj["flags"].to<JsonObject>(); // Use new syntax
                    flagsObj["isUsed"] = TEST(op.flags, OP_USED_BIT);      // Include for completeness, though loop condition implies true
                    flagsObj["isHighPriority"] = TEST(op.flags, OP_HIGH_PRIORITY_BIT);
                    flagsObj["isInProgress"] = TEST(op.flags, OP_IN_PROGRESS_BIT);
                    flagsObj["isBroadcast"] = TEST(op.flags, OP_BROADCAST_BIT);
                    flagsObj["isSynchronized"] = TEST(op.flags, OP_SYNCHRONIZED_BIT);
                }
            }
        }
        else
        {
            doc["error"] = "ModbusRTU manager not available via RS485 interface.";
        }
    }
    else
    {
        doc["error"] = "RS485 interface not enabled or not available.";
    }
#else
    doc["error"] = "RS485 support is not enabled in this build.";
#endif

    serializeJson(doc, *response);
    request->send(response);
}

void RESTServer::setupWebSocket()
{
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
               {
        switch (type) {
            case WS_EVT_CONNECT:
                Log.infoln("WS Client #%u connected from %s", client->id(), client->remoteIP().toString().c_str());
                client->text("{\"type\": \"welcome\", \"clientId\": " + String(client->id()) + "}"); 
                if (userMessageHistory.lines() > 0) {
                    client->text("{\"type\": \"user_message_history_start\"}");
                    for (size_t i = 0; i < userMessageHistory.lines(); ++i) {
                        client->text(userMessageHistory.getLine(i));
                    }
                    client->text("{\"type\": \"user_message_history_end\"}");
                }
                break;
            case WS_EVT_DISCONNECT:
                Log.infoln("WS Client #%u disconnected", client->id());
                break;
            case WS_EVT_DATA:
                handleWebSocketMessage(client, arg, data, len);
                break;
            case WS_EVT_PONG:
                Log.verboseln("WS Client #%u pong", client->id());
                break;
            case WS_EVT_ERROR:
                Log.errorln("WS Client #%u error(%u): %s", client->id(), *((uint16_t*)arg), (char*)data);
                break;
        } });
}

// Helper function to send JSON response to a specific client
void sendJsonResponse(AsyncWebSocketClient *client, const JsonDocument &doc, const char *type)
{
    if (!client || !client->canSend())
        return;

    // Create a wrapper object: {"type": "<type>", "data": ...original_doc...}
    JsonDocument responseDoc;
    responseDoc["type"] = type;
    responseDoc["data"] = doc; // Embed the original document

    String responseStr;
    serializeJson(responseDoc, responseStr);

    if (responseStr.length() > MAX_WEBSOCKET_MESSAGE_SIZE)
    {
        Log.warningln("WS #%u: JSON response too large (%u bytes), sending error instead.", client->id(), responseStr.length());
        JsonDocument errorDoc;
        errorDoc["type"] = "error";
        JsonObject errorDataObj = errorDoc["data"].to<JsonObject>();
        errorDataObj["message"] = "Response too large to send";
        errorDataObj["original_type"] = type;
        String errorStr;
        serializeJson(errorDoc, errorStr);
        client->text(errorStr);
        return;
    }

    client->text(responseStr);
}

// Updated handler to process commands and respond to the specific client
void RESTServer::handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0; // Null-terminate
        JsonDocument requestDoc;
        DeserializationError error = deserializeJson(requestDoc, (char *)data);
        if (error)
        {
            Log.errorln("WS #%u: Failed to parse JSON request: %s", client->id(), error.c_str());
            client->text("{\"type\": \"error\", \"message\": \"Invalid JSON request\"}");
            return;
        }

        // Check using recommended ArduinoJson method
        if (!requestDoc["command"].is<const char *>())
        {
            Log.errorln("WS #%u: Request missing or invalid 'command' field.", client->id());
            client->text("{\"type\": \"error\", \"message\": \"Missing or invalid 'command' field\"}");
            return;
        }

        String command = requestDoc["command"].as<String>();

        JsonDocument responseDoc; // Create doc here for reuse within scope

        if (command == "get_sysinfo")
        {
            // --- Generate System Info Response --- (Adapted from getSystemInfoHandler)
            responseDoc["version"] = "1.0.0"; // Replace with actual version if available
            responseDoc["board"] = BOARD_NAME;
            responseDoc["uptime"] = millis() / 1000;
            responseDoc["timestamp"] = millis();
            responseDoc["freeHeapKb"] = ESP.getFreeHeap() / 1024.0;
            responseDoc["maxFreeBlockKb"] = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) / 1024.0;
            // Add other fields like components if needed, adapting logic from getSystemInfoHandler
            sendJsonResponse(client, responseDoc, "sysinfo");
            // --- End System Info ---
        }
        else if (command == "get_logs")
        {
            // --- Generate Logs Response ---
            int requestedLevel = LOG_LEVEL_VERBOSE;
            if (requestDoc["level"].is<const char *>())
            {
                String levelStr = requestDoc["level"].as<String>();
                if (levelStr == "error")
                    requestedLevel = LOG_LEVEL_ERROR;
                else if (levelStr == "warning")
                    requestedLevel = LOG_LEVEL_WARNING;
                else if (levelStr == "notice")
                    requestedLevel = LOG_LEVEL_NOTICE;
                else if (levelStr == "info")
                    requestedLevel = LOG_LEVEL_INFO;
                // Add other levels if needed (debug, trace, etc.)
            }
            /*
                        if (appInstance)
                        {
                            std::vector<String> logSnapshot = owner->getLogSnapshot();
                            JsonArray logArray = responseDoc.to<JsonArray>();
                            // Simplified getLogLevel - assumes prefix is always present for filtering
                            auto getLogLevel = [](const String &line) -> int
                            {
                                if (line.startsWith("F:") || line.startsWith("E:"))
                                    return LOG_LEVEL_ERROR;
                                if (line.startsWith("W:"))
                                    return LOG_LEVEL_WARNING;
                                if (line.startsWith("N:"))
                                    return LOG_LEVEL_NOTICE;
                                if (line.startsWith("I:"))
                                    return LOG_LEVEL_INFO;
                                if (line.startsWith("T:"))
                                    return LOG_LEVEL_TRACE;
                                // Add D for debug if used
                                return LOG_LEVEL_VERBOSE; // Default
                            };

                            unsigned long currentMillis = millis(); // Get timestamp once for the batch

                            for (const auto &logLine : logSnapshot)
                            {
                                // Filter logs on the server before sending if needed
                                // if (getLogLevel(logLine) <= requestedLevel) {
                                JsonObject logObj = logArray.add<JsonObject>();
                                logObj["ts"] = currentMillis; // Add timestamp (can refine this)
                                logObj["msg"] = logLine;
                                // }
                            }
                        }
                        else
                        {
                            // Send an empty array or an error object?
                            responseDoc.to<JsonArray>(); // Send empty array
                                                         // JsonObject errObj = responseDoc.to<JsonObject>();
                                                         // errObj["error"] = "Log data unavailable";
                        }
                        sendJsonResponse(client, responseDoc, "logs");
                        */
        }
        else if (command == "get_coils")
        {
            JsonDocument responseDoc;
            JsonArray coilsArray = responseDoc.to<JsonArray>();
            int specificAddress = -1;

            if (requestDoc["address"].is<int>())
            {
                specificAddress = requestDoc["address"].as<int>();
            }

            if (modbusManager)
            {
                const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
                for (const auto &info : mappings)
                {
                    if (info.type != FN_READ_COIL && info.type != FN_WRITE_COIL)
                    {
                        continue;
                    }
                    Component *component = owner->byId(info.componentId);
                    if (component)
                    {
                        for (short i = 0; i < info.count; ++i)
                        {
                            short currentAddr = info.startAddress + i;
                            if (specificAddress != -1 && currentAddr != specificAddress)
                            {
                                continue;
                            }

                            uint16_t coilValue = (uint16_t)component->mb_tcp_read(const_cast<MB_Registers *>(&info));
                            JsonObject coilObj = coilsArray.add<JsonObject>();
                            coilObj["address"] = currentAddr;
                            coilObj["value"] = coilValue;
                            coilObj["name"] = info.name;
                            coilObj["component"] = component->name;
                            coilObj["id"] = component->id;
                            coilObj["type"] = (int)info.type;
                            coilObj["access"] = (int)info.access;
                            coilObj["flags"] = component->flags;
                            coilObj["group"] = info.group;

                            if (specificAddress != -1)
                            {
                                goto end_coil_search;
                            }
                        }
                    }
                }
            }
        end_coil_search:
            if (specificAddress != -1 && coilsArray.size() == 0)
            {
                JsonObject errorObj = responseDoc.to<JsonObject>();
                errorObj["success"] = false;
                errorObj["error"] = "Address not found or manager unavailable";
                sendJsonResponse(client, responseDoc, "error");
            }
            else
            {
                sendJsonResponse(client, responseDoc, "coils");
            }
        }
        else if (command == "get_registers")
        {
            int specificAddress = -1;
            if (requestDoc["address"].is<int>())
            {
                specificAddress = requestDoc["address"].as<int>();
            }

            // This array will hold all registers relevant to the current context
            // (either one specific, or all of them before pagination)
            JsonDocument registersSourceDoc; // Temporary document to hold results from _buildRegistersJson
            JsonArray registersSourceArray = registersSourceDoc.to<JsonArray>();
            _buildRegistersJson(registersSourceArray, specificAddress);

            int totalRegisters = registersSourceArray.size();
            int currentPage = 0;
            int currentPageSize = 0;
            int totalPages = 0;

            JsonDocument finalResponseDoc; // This is the root of the JSON to send
            finalResponseDoc["type"] = "registers";
            JsonArray dataArrayForClient = finalResponseDoc["data"].to<JsonArray>();
            JsonObject metaDataForClient = finalResponseDoc["meta"].to<JsonObject>();

            if (specificAddress != -1)
            {
                // For a specific address, all items returned by _buildRegistersJson are the response.
                for (JsonVariant v : registersSourceArray)
                { // Loop 0 or 1 time
                    dataArrayForClient.add(v);
                }
                currentPage = 0;
                currentPageSize = totalRegisters; // This will be 0 if not found, 1 if found.
                totalPages = (totalRegisters > 0) ? 1 : 0;
            }
            else
            {
                // For "all registers" context, apply pagination.
                currentPage = 0; // Default page is 0
                if (requestDoc["page"].is<int>())
                {
                    int reqPage = requestDoc["page"].as<int>();
                    if (reqPage >= 0)
                    {
                        currentPage = reqPage;
                    }
                }

                currentPageSize = DEFAULT_WEBSOCKET_PAGE_SIZE; // Default page size
                if (requestDoc["pageSize"].is<int>())
                {
                    int reqPageSize = requestDoc["pageSize"].as<int>();
                    if (reqPageSize > 0)
                    {
                        currentPageSize = reqPageSize;
                    }
                }

                if (totalRegisters == 0)
                {
                    totalPages = 0;
                    // dataArrayForClient remains empty, currentPageSize reflects requested/default
                }
                else
                {
                    totalPages = (totalRegisters + currentPageSize - 1) / currentPageSize;
                    int startIndex = currentPage * currentPageSize;
                    for (int i = startIndex; i < (startIndex + currentPageSize) && i < totalRegisters; ++i)
                    {
                        dataArrayForClient.add(registersSourceArray[i]);
                    }
                }
            }

            metaDataForClient["page"] = currentPage;
            metaDataForClient["pageSize"] = currentPageSize;
            metaDataForClient["totalRegisters"] = totalRegisters;
            metaDataForClient["totalPages"] = totalPages;

            // Serialize and send finalResponseDoc
            String responseStr;
            serializeJson(finalResponseDoc, responseStr);

            if (responseStr.length() > MAX_WEBSOCKET_MESSAGE_SIZE)
            {
                Log.warningln("WS #%u: JSON response too large (%u bytes), sending error instead. For command: get_registers", client->id(), responseStr.length());
                JsonDocument errorDoc;
                errorDoc["type"] = "error";
                // Maintain the {type: "error", data: {message:"..."}} structure for errors for consistency with sendJsonResponse
                JsonObject errorDataObj = errorDoc["data"].to<JsonObject>();
                errorDataObj["message"] = "Response too large to send";
                errorDataObj["original_type"] = "registers";
                String errorStr;
                serializeJson(errorDoc, errorStr);
                client->text(errorStr);
            }
            else
            {
                client->text(responseStr);
            }
        }
        else if (command == "write_register")
        {
            // --- Handle Write Register Command ---
            JsonDocument responseDoc;
            JsonObject dataObj = responseDoc.to<JsonObject>(); // Data object for the response
            bool success = false;
            String errorMsg = "";
            int address = -1;
            short value = 0;                      // Using short for register value
            JsonVariant reqId = requestDoc["id"]; // Get request ID if present

            // Validate required fields
            if (!requestDoc["address"].is<int>())
            {
                errorMsg = "Missing or invalid 'address' field (must be integer)";
            }
            else if (!requestDoc["value"].is<int>()) // Check if value is integer
            {
                errorMsg = "Missing or invalid 'value' field (must be integer)";
            }
            else
            {
                address = requestDoc["address"].as<int>();
                long val_long = requestDoc["value"].as<long>(); // Read as long for range check

                // Validate value range for a short
                if (val_long < -32768 || val_long > 32767)
                {
                    errorMsg = "Value out of range for 16-bit signed integer (-32768 to 32767)";
                }
                else
                {
                    value = (short)val_long;
                    short writeResult = E_INVALID_PARAMETER;
                    if (modbusManager)
                    {
                        MB_Registers *mapping = modbusManager->findMappingForAddress(address, E_FN_CODE::FN_WRITE_HOLD_REGISTER); // Check for writable holding reg
                        if (mapping)
                        {
                            Component *target = modbusManager->findComponentForAddress(address); // Reuse existing helper
                            if (target)
                            {
                                writeResult = target->mb_tcp_write(mapping, value);
                                if (writeResult == E_OK)
                                {
                                    success = true;
                                }
                                else
                                {
                                    errorMsg = "Failed to write register (Modbus Error: " + String(writeResult) + ")";
                                    Log.errorln("WS #%u: Set Register %d failed via ModbusManager, error code: %d", client->id(), address, writeResult);
                                }
                            }
                            else
                            {
                                errorMsg = "Component not found for address";
                                Log.errorln("WS #%u: write_register - No component found for address %d", client->id(), address);
                                writeResult = E_INVALID_PARAMETER; // Treat as not found
                            }
                        }
                        else
                        {
                            errorMsg = "Address not found or not writable";
                            Log.warningln("WS #%u: write_register - No writable mapping found for address %d", client->id(), address);
                            writeResult = E_INVALID_PARAMETER; // Address not found or not writable
                        }
                    }
                    else
                    {
                        errorMsg = "ModbusManager not available";
                        Log.warningln("WS #%u: write_register - No ModbusManager available.");
                        writeResult = E_INVALID_PARAMETER; // Cannot proceed
                    }
                }
            }
            dataObj["success"] = success;
            if (address != -1)
                dataObj["address"] = address;
            if (success)
                dataObj["value"] = value;
            if (!reqId.isNull())
                dataObj["id"] = reqId; // Include original ID if present
            if (!errorMsg.isEmpty())
                dataObj["error"] = errorMsg;

            sendJsonResponse(client, dataObj, "write_response");
        }
        else if (command == "write_coil")
        {
            JsonDocument responseDoc;
            JsonObject dataObj = responseDoc.to<JsonObject>();
            bool success = false;
            String errorMsg = "";
            int address = -1;
            bool value = false;
            JsonVariant reqId = requestDoc["id"];

            if (!requestDoc["address"].is<int>())
            {
                errorMsg = "Missing or invalid 'address' field (must be integer)";
            }
            else if (!requestDoc["value"].is<bool>() && !requestDoc["value"].is<int>())
            {
                errorMsg = "Missing or invalid 'value' field (must be boolean or integer)";
            }
            else
            {
                address = requestDoc["address"].as<int>();

                if (requestDoc["value"].is<bool>())
                {
                    value = requestDoc["value"].as<bool>();
                }
                else
                {
                    int intValue = requestDoc["value"].as<int>();
                    value = (intValue != 0);
                }

                short writeResult = E_INVALID_PARAMETER;
                if (modbusManager)
                {
                    MB_Registers *mapping = modbusManager->findMappingForAddress(address, E_FN_CODE::FN_WRITE_COIL);
                    if (mapping)
                    {
                        Component *target = modbusManager->findComponentForAddress(address);
                        if (target)
                        {
                            writeResult = target->mb_tcp_write(mapping, value ? 1 : 0);
                            Log.infoln("WS #%d: write_coil to %s: %d", client->id(), target->name.c_str(), value);
                            if (writeResult == E_OK)
                            {
                                success = true;
                            }
                            else
                            {
                                errorMsg = "Failed to write coil (Modbus Error: " + String(writeResult) + ")";
                                Log.errorln("WS #%u: Set Coil %d failed via ModbusManager, error code: %d", client->id(), address, writeResult);
                            }
                        }
                        else
                        {
                            errorMsg = "Component not found for address";
                            Log.errorln("WS #%u: write_coil - No component found for address %d", client->id(), address);
                            writeResult = E_INVALID_PARAMETER;
                        }
                    }
                    else
                    {
                        errorMsg = "Address not found or not writable";
                        Log.warningln("WS #%u: write_coil - No writable mapping found for address %d", client->id(), address);
                        writeResult = E_INVALID_PARAMETER;
                    }
                }
                else
                {
                    errorMsg = "ModbusManager not available";
                    Log.warningln("WS #%u: write_coil - No ModbusManager available.", client->id());
                    writeResult = E_INVALID_PARAMETER;
                }
            }

            dataObj["success"] = success;
            if (address != -1)
                dataObj["address"] = address;
            if (success)
                dataObj["value"] = value;
            if (!reqId.isNull())
                dataObj["id"] = reqId;
            if (!errorMsg.isEmpty())
                dataObj["error"] = errorMsg;

            sendJsonResponse(client, responseDoc, success ? "coil_update" : "error");
        }
        else
        {
            Log.warningln("WS #%u: Unknown command '%s'", client->id(), command.c_str());
            client->text("{\"type\": \"error\", \"message\": \"Unknown command\"}");
        }
    }
}
void RESTServer::broadcast(BroadcastMessageType type, const JsonDocument &data)
{
    JsonDocument doc;
    String typeStr = "unknown";
    switch (type)
    {
    case BROADCAST_COIL_UPDATE:
        typeStr = "coil_update";
        break;
    case BROADCAST_REGISTER_UPDATE:
        typeStr = "register_update";
        break;
    case BROADCAST_LOG_ENTRY:
        typeStr = "log_entry";
        break;
    case BROADCAST_SYSTEM_STATUS:
        typeStr = "system_status";
        break;
    case BROADCAST_USER_DEFINED:
        typeStr = "user_defined";
        break;
    case BROADCAST_USER_MESSAGE:
        typeStr = "user_message";
        break;
    default:
        break;
    }

    doc["type"] = typeStr;
    doc["data"] = data;

    if (type == BROADCAST_USER_MESSAGE)
    {
        doc["timestamp"] = millis();
    }

    String output;
    serializeJson(doc, output);

    if (type == BROADCAST_USER_MESSAGE)
    {
        userMessageHistory.addMessage(output.c_str());
    }

    ws.textAll(output);
}

short RESTServer::onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user, Component *src)
{
    if (verb == E_CALLS::EC_USER && user != nullptr)
    {
        MB_UpdateData *info = static_cast<MB_UpdateData *>(user);
        JsonDocument doc;
        doc["rtuSlaveId"] = info->slaveId;
        doc["address"] = info->address;
        doc["fc"] = info->functionCode;
        doc["count"] = info->count;
        doc["id"] = src ? src->id : 0;
        if(info->functionCode == E_FN_CODE::FN_WRITE_MULT_REGISTERS && info->userData != nullptr) {
            JsonArray values = doc["values"].to<JsonArray>();
            uint16_t* data = static_cast<uint16_t*>(info->userData);
            for(int i = 0; i < info->count; i++) {
                values.add(data[i]);
            }
        } else {
            doc["value"] = info->value;
        }
            
        BroadcastMessageType msgType = BROADCAST_UNKNOWN;
        switch (info->functionCode)
        {
        case E_FN_CODE::FN_READ_COIL:
        case E_FN_CODE::FN_READ_DISCR_INPUT:
        case E_FN_CODE::FN_WRITE_COIL:
        case E_FN_CODE::FN_WRITE_MULT_COILS:
            msgType = BROADCAST_COIL_UPDATE;
            break;
        case E_FN_CODE::FN_READ_HOLD_REGISTER:
        case E_FN_CODE::FN_READ_INPUT_REGISTER:
        case E_FN_CODE::FN_WRITE_HOLD_REGISTER:
        case E_FN_CODE::FN_WRITE_MULT_REGISTERS:
            msgType = BROADCAST_REGISTER_UPDATE;
            break;
        default:
            Log.warningln(F("[RESTServer] Unknown function code %d for broadcast mapping."), (int)info->functionCode);
            break;
        }

        if (msgType != BROADCAST_UNKNOWN)
        {
            this->broadcast(msgType, doc);
        }
        else
        {
            return E_INVALID_PARAMETER;
        }
        return E_OK;
    }
    else
    {
        return Component::onMessage(id, verb, flags, user, src);
    }
}

#ifdef ENABLE_WEBSOCKET
uint8_t RESTServer::getConnectedClientsCount() const {
    return ws.count();
}
#endif

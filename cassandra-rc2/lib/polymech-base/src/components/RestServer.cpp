#include "config.h" // Application configuration
#include "version.h"

// #define WS_MAX_QUEUED_MESSAGES 512

static const int HEAP_FRAGMENTATION_THRESHOLD = 65;
static const int DEFAULT_WEBSOCKET_PAGE_SIZE = 10;
static const int MAX_PAGE_SIZE = 20;

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
#include "esp32_compat.h"

#include <Component.h>
#include <components/RS485.h>
#include <components/Logger.h>
#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>
#include "net/commons.h"
#include <App.h>

#include <Logger.h>
#include "enums.h"
#include <vector>
#include "app-logger.h"

#undef ENABLE_WEBSOCKET_PB

#ifdef ENABLE_WEBSOCKET_PB
#include <pb_encode.h>
#include "proto/registers.pb.h"
#include <StreamString.h>

// Context for the nanopb encode callback
struct RegisterEncodeCtx
{
    ModbusTCP *modbusManager;
    Component *owner;
    int startIndex;
    int endIndex;
    int currentIndex;
};

// nanopb callback for encoding a stream of Register messages
bool encode_registers_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    RegisterEncodeCtx *ctx = (RegisterEncodeCtx *)*arg;
    if (!ctx || !ctx->modbusManager || !ctx->owner)
    {
        return false;
    }

    const Vector<MB_Registers> &mappings = ctx->modbusManager->getAddressMappings();

    // This loop continues where it left off on the last iteration because currentIndex is preserved in the context.
    for (size_t i = 0; i < mappings.size(); ++i)
    {
        const auto &info = mappings[i];
        if (ctx->currentIndex >= ctx->endIndex)
            break;

        if (info.type != E_FN_CODE::FN_READ_HOLD_REGISTER &&
            info.type != E_FN_CODE::FN_READ_INPUT_REGISTER &&
            info.type != E_FN_CODE::FN_WRITE_HOLD_REGISTER &&
            info.type != E_FN_CODE::FN_WRITE_MULT_REGISTERS)
        {
            continue;
        }

        Component *component = ctx->owner->byId(info.componentId);
        if (!component)
            continue;

        for (short j = 0; j < info.count; ++j)
        {
            if (ctx->currentIndex >= ctx->startIndex && ctx->currentIndex < ctx->endIndex)
            {
                Register reg_msg = Register_init_zero;
                short currentAddr = info.startAddress + j;

                reg_msg.address = currentAddr;
                reg_msg.has_error = true;
                reg_msg.error = component->mb_tcp_error(const_cast<MB_Registers *>(&info));
                reg_msg.has_value = true;
                reg_msg.value = (uint16_t)component->mb_tcp_read(const_cast<MB_Registers *>(&info));

                if (info.name)
                {
                    strncpy(reg_msg.name, info.name, sizeof(reg_msg.name));
                    reg_msg.name[sizeof(reg_msg.name) - 1] = '\0';
                    reg_msg.has_name = true;
                }

                strncpy(reg_msg.component, component->name.c_str(), sizeof(reg_msg.component));
                reg_msg.component[sizeof(reg_msg.component) - 1] = '\0';
                reg_msg.has_component = true;

                reg_msg.has_id = true;
                reg_msg.id = component->id;
                reg_msg.has_type = true;
                reg_msg.type = (int)info.type;
                reg_msg.has_slaveId = true;
                reg_msg.slaveId = info.slaveId;
                reg_msg.has_flags = true;
                reg_msg.flags = component->flags;

                if (info.group)
                {
                    strncpy(reg_msg.group, info.group, sizeof(reg_msg.group));
                    reg_msg.group[sizeof(reg_msg.group) - 1] = '\0';
                    reg_msg.has_group = true;
                }

                if (!pb_encode_tag_for_field(stream, field) || !pb_encode_submessage(stream, Register_fields, &reg_msg))
                {
                    LS_ERROR("WS PB: Failed to encode submessage for register %d", currentAddr);
                    return false; // Stop encoding on failure
                }
            }
            ctx->currentIndex++;
            if (ctx->currentIndex >= ctx->endIndex)
                break;
        }
    }
    return true;
}
#endif // ENABLE_WEBSOCKET_PB

// Moved mountLittleFS definition here
short RESTServer::mountLittleFS()
{
#ifdef ENABLE_LITTLEFS
    if (!LittleFS.begin(true)) // true = format_if_failed
    {
        LS_ERROR("LittleFS mount failed (even after attempting format)! Web files will not be available.");
        return E_INVALID_PARAMETER;
    }
    else
    {
        return E_OK;
    }
#else
    return E_NOT_SUPPORTED; // Or E_OK if not enabling it is not an error condition for the caller
#endif
}

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
#ifdef ENABLE_WEBSOCKET
    wsCommandHandlers.reserve(DEFAULT_WS_COMMAND_HANDLER_CAPACITY);
#endif
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
    }
    else
    {
        LS_ERROR("✗ index.html cannot be opened from LittleFS! File may be missing.");
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
        LS_ERROR("Still failed to open root directory in LittleFS");
        return E_INVALID_PARAMETER;
    }

    if (!root.isDirectory())
    {
        LS_ERROR("Root is not a directory in LittleFS");
        return E_INVALID_PARAMETER;
    }
/*
    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            L_INFO("  FS: DIR : %s", file.name());
        }
        else
        {
            L_INFO("  FS: FILE: %s, SIZE: %d", file.name(), file.size());
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();
*/
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
    // ws.cleanupClients(); // Periodically clean up disconnected WebSocket clients
    processWsQueue();
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
                    if (static_cast<App*>(owner)->getHeapFragmentation() > HEAP_FRAGMENTATION_THRESHOLD) {
                        Log.warningln("[MEM] High Heap Fragmentation > %d%% detected on %s. Aborting request.", HEAP_FRAGMENTATION_THRESHOLD, request->url().c_str());
                        request->send(503, "text/plain", "Server is busy, please try again later.");
                        return;
                    }
#ifdef ENABLE_LITTLEFS
        if (this->mountLittleFS() != E_OK) {
            LS_ERROR("HANDLER /: LittleFS mount failed. Cannot serve index.html.");
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
            LS_ERROR("HANDLER: Failed to open /index.html for / request.");
            request->send(200, "text/html", 
                "<html><body><h1>ESP32 Web Server</h1><p>index.html file not found in filesystem (handler /)</p></body></html>"); // Modified error
        } });

    server.on("/index.html", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
                    if (static_cast<App*>(owner)->getHeapFragmentation() > HEAP_FRAGMENTATION_THRESHOLD) {
                        Log.warningln("[MEM] High Heap Fragmentation > %d%% detected on %s. Aborting request.", HEAP_FRAGMENTATION_THRESHOLD, request->url().c_str());
                        request->send(503, "text/plain", "Server is busy, please try again later.");
                        return;
                    }
#ifdef ENABLE_LITTLEFS
                if (this->mountLittleFS() != E_OK) {
                    LS_ERROR("HANDLER /: LittleFS mount failed. Cannot serve index.html.");
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
            LS_ERROR("HANDLER: Failed to open /index.html for /index.html request.");
            request->send(200, "text/html", 
                "<html><body><h1>ESP32 Web Server</h1><p>index.html file not found in filesystem (handler /index.html)</p></body></html>"); // Modified error
        } });

    //--------------------------------
    //--------------------------------

    server.on("/assets/*", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
                  if (static_cast<App *>(owner)->getHeapFragmentation() > HEAP_FRAGMENTATION_THRESHOLD)
                  {
                      Log.warningln("[MEM] High Heap Fragmentation > %d%% detected on %s. Aborting request.", HEAP_FRAGMENTATION_THRESHOLD, request->url().c_str());
                      request->send(503, "text/plain", "Server is busy, please try again later.");
                      return;
                  }
#ifdef ENABLE_LITTLEFS
                  if (this->mountLittleFS() != E_OK)
                  {
                      LS_ERROR("HANDLER /assets/*: LittleFS mount failed. Cannot serve asset.");
                      request->send(500, "text/plain", "Internal Server Error: Filesystem not available");
                      return;
                  }
                  String path = request->url();

                  if (path.endsWith(".js") || path.endsWith(".css"))
                  {
                      String contentType = path.endsWith(".js") ? "application/javascript" : "text/css";

                      // 1. Determine the final path of the content to be served.
                      String content_path = path;
                      bool isGzipped = false;
                      String path_gz = path + ".gz";
                      bool acceptsGzip = false;
                      if (request->hasHeader("Accept-Encoding") && request->getHeader("Accept-Encoding")->value().indexOf("gzip") != -1)
                      {
                          acceptsGzip = true;
                      }

                      if (acceptsGzip && LittleFS.exists(path_gz))
                      {
                          content_path = path_gz;
                          isGzipped = true;
                      }
                      else if (!LittleFS.exists(path))
                      {
                          request->send(404, "text/plain", "Not found: " + path);
                          return;
                      }

                      // 2. Generate ETag from file metadata (safely).
                      String etag = "";
                      File f = LittleFS.open(content_path, "r");
                      if (f)
                      {
                          etag = String(f.size(), HEX) + "-" + String(f.getLastWrite(), HEX);
                          f.close(); // Immediately close the file.
                      }

                      // 3. Check client's cache and send 304 if it matches.
                      if (request->hasHeader("If-None-Match") && request->getHeader("If-None-Match")->value() == etag)
                      {
                          request->send(304);
                          return;
                      }

                      // 4. Send the full response with caching headers.
                      AsyncWebServerResponse *response = request->beginResponse(LittleFS, content_path, contentType);
                      if (isGzipped)
                      {
                          response->addHeader("Content-Encoding", "gzip");
                      }
                      response->addHeader("Content-Type", contentType);
                      response->addHeader("ETag", etag);
                      response->addHeader("Cache-Control", "no-cache");
                      request->send(response);
                      return;
                  }

                  // Generic handler for other files
                  String filePath = path;
                  File file = LittleFS.open(filePath, "r");
                  if (file)
                  {
                      String contentType = "application/octet-stream";
                      AsyncWebServerResponse *response = request->beginResponse(LittleFS, filePath, contentType);
                      request->send(response);
                  }
                  else
                  {
                      LS_ERROR("Failed to open asset: %s", filePath.c_str());
                      request->send(404, "text/plain", "File not found: " + path);
                  }
#endif
              });

#ifdef ENABLE_MODBUS_TCP
    // server.on("/api/v1/modbus/rtu/queue", HTTP_GET, [this](AsyncWebServerRequest *request)
    //           { this->getRtuOperationQueueHandler(request); });
    // server.on("/api/v1/mappings", HTTP_GET, [this](AsyncWebServerRequest *request)
    //           { this->getMappingsHandler(request); });
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
                              LS_ERROR("REST: (Inline) setRegisterQueryHandler - No component found for address %d and componentId %d", address, mapping->componentId);
                              writeResult = E_INVALID_PARAMETER; // Address not found
                          }
                          if (writeResult == E_OK)
                          {
                              success = true;
                          }
                          else
                          {
                              LS_ERROR("REST: (Inline) Set Register %d failed via ModbusManager, error code: %d (Query)", address, writeResult);
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
    /*
    server.on("/api/v1/system/log-level", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
        if (request->hasParam("level")) {
            this->setLogLevelHandler(request);
        } else {
            this->getLogLevelHandler(request);
        } });
    */
    server.on("/api/v1/fs/list", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->listFsHandler(request); });

    // Handle OPTIONS requests for CORS
    server.onNotFound([](AsyncWebServerRequest *request)
                      {
    String url = request->url();
        // LS_ERROR("REST: Not Found - Method: %s, URL: %s", request->methodToString(), url.c_str());
#ifdef ENABLE_LITTLEFS
        if (request->method() == HTTP_GET) {
            String path = url;
            // Check if the file exists in LittleFS
            if (LittleFS.exists(path)) {
                LS_ERROR("File exists in LittleFS but handler not triggered: %s", path.c_str());
            } else {
                File root = LittleFS.open("/");
                if (root && root.isDirectory()) {
                    File file = root.openNextFile();
                    while (file) {
                        if (file.isDirectory()) {
                            // L_INFO("  DIR: %s", file.name());
                        } else {
                            // L_INFO("  FILE: %s, SIZE: %d", file.name(), file.size());
                        }
                        file = root.openNextFile();
                    }
                } else {
                    LS_ERROR("Failed to open root directory");
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
                LS_ERROR("File exists in LittleFS but handler not triggered: %s", path.c_str());
            } else {
                File root = LittleFS.open("/");
                if (root && root.isDirectory()) {
                    File file = root.openNextFile();
                    while (file) {
                        if (file.isDirectory()) {
                            // L_INFO("  DIR: %s", file.name());
                        } else {
                            // L_INFO("  FILE: %s, SIZE: %d", file.name(), file.size());
                        }
                        file = root.openNextFile();
                    }
                } else {
                    LS_ERROR("Failed to open root directory");
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
#ifdef VERSION
    doc["version"] = VERSION;
#else
    doc["version"] = "unknown";
#endif
#ifdef PACKAGE_URL
    doc["url"] = PACKAGE_URL;
#else
    doc["url"] = "unknown";
#endif
    doc["board"] = BOARD_NAME;
    doc["uptime"] = millis() / 1000;
    doc["timestamp"] = millis();
    doc["freeHeapKb"] = ESP.getFreeHeap() / 1024;
    doc["maxFreeBlockKb"] = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) / 1024;
    doc["fragmentationPercent"] = static_cast<App *>(owner)->getHeapFragmentation();
    doc["cpuTicks"] = (uint32_t)esp_cpu_get_cycle_count();
    doc["loopDurationMs"] = owner->getLoopDurationUs();
    // --- Calculate Average CPU Load ---
    float cpuLoadPercent = -1.0; // Default value if stats are unavailable
    // Add CPU load to JSON, format to 1 decimal place if valid, else null
    if (cpuLoadPercent >= 0.0)
    {
        char loadStr[10];
        // snprintf with %f is not supported on all platforms
        int integerPart = (int)cpuLoadPercent;
        int fractionalPart = abs((int)((cpuLoadPercent - integerPart) * 10));
        snprintf(loadStr, sizeof(loadStr), "%d.%d", integerPart, fractionalPart);
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
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    int totalCoils = 0;
    if (modbusManager)
    {
        const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
        for (const auto &info : mappings)
        {
            if (info.type == E_FN_CODE::FN_READ_COIL || info.type == E_FN_CODE::FN_WRITE_COIL)
            {
                totalCoils += info.count;
            }
        }
    }

    int currentPage = request->hasParam("page") ? request->getParam("page")->value().toInt() : 0;
    int pageSize = request->hasParam("pageSize") ? request->getParam("pageSize")->value().toInt() : DEFAULT_WEBSOCKET_PAGE_SIZE;
    if (pageSize <= 0)
        pageSize = DEFAULT_WEBSOCKET_PAGE_SIZE;
    if (pageSize > MAX_PAGE_SIZE)
        pageSize = MAX_PAGE_SIZE;

    response->print(F("{\"meta\":{"));
    response->print(F("\"page\":"));
    response->print(currentPage);
    response->print(F(",\"pageSize\":"));
    response->print(pageSize);
    response->print(F(",\"totalCoils\":"));
    response->print(totalCoils);
    response->print(F(",\"totalPages\":"));
    response->print((pageSize > 0) ? (totalCoils + pageSize - 1) / pageSize : 0);
    response->print(F("},\"coils\":["));

    int startIndex = currentPage * pageSize;
    int endIndex = startIndex + pageSize;
    int currentIndex = 0;
    bool firstItem = true;

    if (modbusManager)
    {
        const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
        for (const auto &info : mappings)
        {
            if (currentIndex >= endIndex)
                break;

            if (info.type != E_FN_CODE::FN_READ_COIL && info.type != E_FN_CODE::FN_WRITE_COIL)
            {
                continue;
            }

            Component *component = owner->byId(info.componentId);
            if (!component)
            {
                // Skip all coils for this component if it's not found, but advance index
                currentIndex += info.count;
                continue;
            }

            for (short i = 0; i < info.count; ++i)
            {
                if (currentIndex >= startIndex && currentIndex < endIndex)
                {
                    if (!firstItem)
                    {
                        response->print(F(","));
                    }

                    short currentAddr = info.startAddress + i;

                    response->print(F("{"));
                    response->print(F("\"address\":"));
                    response->print(currentAddr);
                    response->print(F(",\"value\":"));
                    response->print(component->mb_tcp_read(const_cast<MB_Registers *>(&info)));
                    response->print(F(",\"name\":\""));
                    response->print(info.name ? info.name : "");
                    response->print(F("\",\"component\":\""));
                    response->print(component->name.c_str());
                    response->print(F("\",\"id\":"));
                    response->print(component->id);
                    response->print(F(",\"group\":\""));
                    response->print(info.group ? info.group : "");
                    response->print(F("\"}"));

                    firstItem = false;
                }
                currentIndex++;
                if (currentIndex >= endIndex)
                    break;
            }
        }
    }

    response->print(F("]}"));
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
        MB_Registers *mapping = modbusManager->findMappingForAddress(address, FN_READ_COIL);
        if (!mapping)
        {
            mapping = modbusManager->findMappingForAddress(address, FN_WRITE_COIL);
        }

        if (mapping)
        {
            Component *target = owner->byId(mapping->componentId);
            if (target)
            {
                value = (target->mb_tcp_read(mapping) != 0);
                found = true;
            }
        }

        if (!found)
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
                writeResult = target->mb_tcp_write(mapping, value ? 1 : 0);
            }
            else
            {
                LS_ERROR("REST: (Inline) setRegisterQueryHandler - No component found for address %d and componentId %d", address, mapping->componentId);
                writeResult = E_INVALID_PARAMETER; // Address not found
            }
            if (writeResult == E_OK)
            {
                success = true;
            }
            else
            {
                LS_ERROR("REST: Set Coil %d failed via ModbusManager, error code: %d (Query)", address, writeResult);
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
    // Handle single address request separately as it's a different JSON structure
    if (request->hasParam("address"))
    {
        // This part seems okay for a single address, as memory impact is minimal.
        int address = request->getParam("address")->value().toInt();
        JsonDocument tempDoc;
        JsonArray tempArray = tempDoc.to<JsonArray>();
        _buildRegistersJson(tempArray, address);

        if (tempArray.size() > 0)
        {
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            serializeJson(tempArray[0], *response);
            request->send(response);
        }
        else
        {
            request->send(404, "application/json", "{\"success\":false,\"error\":\"Address not found or manager unavailable\"}");
        }
        return;
    }

    // --- Chunked Response for All Registers with Pagination ---
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    int totalRegisters = 0;
    if (modbusManager)
    {
        const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
        for (const auto &info : mappings)
        {
            if (info.type == E_FN_CODE::FN_READ_HOLD_REGISTER ||
                info.type == E_FN_CODE::FN_READ_INPUT_REGISTER ||
                info.type == E_FN_CODE::FN_WRITE_HOLD_REGISTER ||
                info.type == E_FN_CODE::FN_WRITE_MULT_REGISTERS)
            {
                totalRegisters += info.count;
            }
        }
    }

    int currentPage = request->hasParam("page") ? request->getParam("page")->value().toInt() : 0;
    int pageSize = request->hasParam("pageSize") ? request->getParam("pageSize")->value().toInt() : DEFAULT_WEBSOCKET_PAGE_SIZE;
    if (pageSize <= 0)
        pageSize = DEFAULT_WEBSOCKET_PAGE_SIZE;
    if (pageSize > MAX_PAGE_SIZE)
        pageSize = MAX_PAGE_SIZE;

    response->print(F("{\"meta\":{"));
    response->print(F("\"page\":"));
    response->print(currentPage);
    response->print(F(",\"pageSize\":"));
    response->print(pageSize);
    response->print(F(",\"totalRegisters\":"));
    response->print(totalRegisters);
    response->print(F(",\"totalPages\":"));
    response->print((pageSize > 0) ? (totalRegisters + pageSize - 1) / pageSize : 0);
    response->print(F("},\"registers\":["));

    int startIndex = currentPage * pageSize;
    int endIndex = startIndex + pageSize;
    int currentIndex = 0;
    bool firstItem = true;

    if (modbusManager)
    {
        const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
        for (const auto &info : mappings)
        {
            if (currentIndex >= endIndex)
                break;

            if (info.type != E_FN_CODE::FN_READ_HOLD_REGISTER &&
                info.type != E_FN_CODE::FN_READ_INPUT_REGISTER &&
                info.type != E_FN_CODE::FN_WRITE_HOLD_REGISTER &&
                info.type != E_FN_CODE::FN_WRITE_MULT_REGISTERS)
            {
                continue;
            }
            Component *component = owner->byId(info.componentId);
            if (!component)
                continue;

            for (short i = 0; i < info.count; ++i)
            {
                if (currentIndex >= startIndex && currentIndex < endIndex)
                {
                    if (!firstItem)
                    {
                        response->print(F(","));
                    }

                    short currentAddr = info.startAddress + i;
                    uint16_t regValue = (uint16_t)component->mb_tcp_read(const_cast<MB_Registers *>(&info));
                    short error = component->mb_tcp_error(const_cast<MB_Registers *>(&info));

                    response->print(F("{"));
                    response->print(F("\"error\":"));
                    response->print(error);
                    response->print(F(",\"address\":"));
                    response->print(currentAddr);
                    response->print(F(",\"value\":"));
                    response->print(regValue);
                    response->print(F(",\"name\":\""));
                    response->print(info.name ? info.name : "");
                    response->print(F("\",\"component\":\""));
                    response->print(component->name.c_str());
                    response->print(F("\",\"id\":"));
                    response->print(component->id);
                    response->print(F(",\"type\":"));
                    response->print((int)info.type);
                    response->print(F(",\"slaveId\":"));
                    response->print(info.slaveId);
                    response->print(F(",\"flags\":"));
                    response->print(component->flags);
                    response->print(F(",\"group\":\""));
                    response->print(info.group ? info.group : "");
                    response->print(F("\"}"));

                    firstItem = false;
                }
                currentIndex++;
                if (currentIndex >= endIndex)
                    break;
            }
        }
    }

    response->print(F("]}"));
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
            continue;
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
            regObj["type"] = (int)info.type; // Cast enum to int
            regObj["slaveId"] = info.slaveId;
            regObj["flags"] = component->flags;
            regObj["group"] = info.group;

            // If we were looking for a specific address and found it, we can stop
            if (foundSpecific)
            {
                return; // Exit the function early
            }
        }
        // If specificAddress was requested but not found after checking all mappings, continue to the next mapping
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
        LS_ERROR("Failed to open root directory in LittleFS");
        doc["error"] = "Failed to open root directory";
        serializeJson(doc, *response);
        request->send(response);
        return;
    }
    if (!root.isDirectory())
    {
        LS_ERROR("Root is not a directory in LittleFS");
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
                Serial.printf("WS Client #%u connected from %s. Total clients: %u\n", client->id(), client->remoteIP().toString().c_str(), ws.count());
            
            // Enable TCP Keepalive to detect dead clients
            if (client->client()) {
                client->client()->setKeepAlive(1000, 1); // 5s idle, 2 probes
            }
               // client->text("{\"type\": \"welcome\", \"clientId\": " + String(client->id()) + "}"); 
                if (userMessageHistory.lines() > 0) {
                    for (size_t i = 0; i < userMessageHistory.lines(); ++i) {
                       // client->text(userMessageHistory.getLine(i));
                    }                    
                }
                break;
            case WS_EVT_DISCONNECT:
                Serial.printf("WS Client #%u from %s disconnected. Total clients: %u\n", client->id(), client->remoteIP().toString().c_str(), ws.count());
                break;
            case WS_EVT_DATA:
                handleWebSocketMessage(client, arg, data, len);
                break;
            case WS_EVT_PONG:
                Serial.printf("WS Client #%u PONG\n", client->id());
                break;
            case WS_EVT_ERROR:
                Serial.printf("WS Client #%u from %s error(%u) len(%d): %s\n", client->id(), client->remoteIP().toString().c_str(), *((uint16_t*)arg), len, (char*)data);
                break;
        } });
}

#ifdef ENABLE_WEBSOCKET
void RESTServer::registerWsCommandHandler(const String &command, WsCommandHandler handler)
{
    for (auto &pair : wsCommandHandlers)
    {
        if (pair.first == command)
        {
            pair.second = handler;
            return;
        }
    }
    wsCommandHandlers.push_back({command, handler});
}
#endif

// Helper function to send JSON response to a specific client
#ifdef ENABLE_WEBSOCKET
// Updated handler to process commands and respond to the specific client
void RESTServer::sendJsonResponse(AsyncWebSocketClient *client, const JsonDocument &doc, const char *type)
{
    if (!client)
        return;

    // Create a wrapper object: {"type": "<type>", "data": ...original_doc...}
    JsonDocument responseDoc;
    responseDoc["type"] = type;
    responseDoc["data"] = doc; // Embed the original document

    String responseStr;
    serializeJson(responseDoc, responseStr);

    if (responseStr.length() > MAX_WEBSOCKET_MESSAGE_SIZE)
    {
        JsonDocument errorDoc;
        errorDoc["type"] = "error";
        JsonObject errorDataObj = errorDoc["data"].to<JsonObject>();
        errorDataObj["message"] = "Response too large to send";
        errorDataObj["original_type"] = type;
        String errorStr;
        serializeJson(errorDoc, errorStr);
        queueWsMessage(client->id(), errorStr);
        L_ERROR("WS: Response too large to send to client #%u. Size: %d", client->id(), responseStr.length());
        return;
    }

    queueWsMessage(client->id(), responseStr);
}
#endif

// Updated handler to process commands and respond to the specific client
void RESTServer::handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;

    if (len > 0 && Log.getLevel() >= LOG_LEVEL_VERBOSE)
    {
        // Log.verbose("WS Message len: %d", len); - unsafe string build removed
    }

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0; // Null-terminate
        JsonDocument requestDoc;
        DeserializationError error = deserializeJson(requestDoc, (char *)data);
        if (error)
        {
            LS_ERROR("WS #%u: Failed to parse JSON request: %s", client->id(), error.c_str());
            queueWsMessage(client->id(), "{\"type\": \"error\", \"message\": \"Invalid JSON request\"}");
            return;
        }

        // Check using recommended ArduinoJson method
        if (!requestDoc["command"].is<const char *>())
        {
            LS_ERROR("WS #%u: Request missing or invalid 'command' field.", client->id());
            queueWsMessage(client->id(), "{\"type\": \"error\", \"message\": \"Missing or invalid 'command' field\"}");
            return;
        }

        String command = requestDoc["command"].as<String>();

#ifdef ENABLE_WEBSOCKET
        // Check for registered handlers first
        for (const auto &pair : wsCommandHandlers)
        {
            if (pair.first == command)
            {
                JsonVariant json = requestDoc.as<JsonVariant>();
                pair.second(client, json); // Call the handler
                return;                    // Exit after handling
            }
        }
#endif

        JsonDocument responseDoc; // Create doc here for reuse within scope

        if (command == "get_sysinfo")
        {
            // --- Generate System Info Response --- (Adapted from getSystemInfoHandler)
            responseDoc["version"] = "3ce112f";
            responseDoc["board"] = BOARD_NAME;
            responseDoc["uptime"] = millis() / 1000;
            responseDoc["timestamp"] = millis();
            responseDoc["freeHeapKb"] = ESP.getFreeHeap() / 1024.0;
            responseDoc["maxFreeBlockKb"] = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) / 1024.0;
            responseDoc["fragmentationPercent"] = static_cast<App *>(owner)->getHeapFragmentation();
            responseDoc["cpuTicks"] = (uint32_t)esp_cpu_get_cycle_count();
            responseDoc["loopDurationMs"] = owner->getLoopDurationUs();
            // --- Calculate Average CPU Load ---
            float cpuLoadPercent = -1.0; // Default value if stats are unavailable
            // Add CPU load to JSON, format to 1 decimal place if valid, else null
            if (cpuLoadPercent >= 0.0)
            {
                char loadStr[10];
                // snprintf with %f is not supported on all platforms
                int integerPart = (int)cpuLoadPercent;
                int fractionalPart = abs((int)((cpuLoadPercent - integerPart) * 10));
                snprintf(loadStr, sizeof(loadStr), "%d.%d", integerPart, fractionalPart);
                responseDoc["cpuLoadPercent"] = loadStr; // Store as string for consistent formatting
            }
            else
            {
                responseDoc["cpuLoadPercent"] = nullptr;
            }
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
            }
        }
        else if (command == "get_coils")
        {
            JsonDocument finalResponseDoc;
            finalResponseDoc["type"] = "coils";
            JsonArray dataArrayForClient = finalResponseDoc["data"].to<JsonArray>();
            JsonObject metaDataForClient = finalResponseDoc["meta"].to<JsonObject>();

            int specificAddress = -1;
            if (requestDoc["address"].is<int>())
            {
                specificAddress = requestDoc["address"].as<int>();
            }

            if (modbusManager)
            {
                if (specificAddress != -1)
                {
                    const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
                    bool found = false;
                    for (const auto &info : mappings)
                    {
                        if (info.type != FN_READ_COIL && info.type != FN_WRITE_COIL && info.type != FN_WRITE_MULT_COILS)
                        {
                            continue;
                        }

                        if (specificAddress >= info.startAddress && specificAddress < info.startAddress + info.count)
                        {
                            Component *component = owner->byId(info.componentId);
                            if (component)
                            {
                                short currentAddr = specificAddress;
                                uint16_t coilValue = (uint16_t)component->mb_tcp_read(const_cast<MB_Registers *>(&info));
                                JsonObject coilObj = dataArrayForClient.add<JsonObject>();
                                coilObj["address"] = currentAddr;
                                coilObj["value"] = coilValue;
                                coilObj["name"] = info.name;
                                coilObj["component"] = component->name;
                                coilObj["id"] = component->id;
                                coilObj["type"] = (int)info.type;
                                coilObj["flags"] = component->flags;
                                coilObj["group"] = info.group;
                                found = true;
                                break; // Found the mapping, no need to continue
                            }
                        }
                    }

                    metaDataForClient["totalCoils"] = found ? 1 : 0;
                    metaDataForClient["page"] = 0;
                    metaDataForClient["pageSize"] = found ? 1 : 0;
                    metaDataForClient["totalPages"] = found ? 1 : 0;

                    if (!found)
                    {
                        finalResponseDoc["error"] = "Address not found";
                    }
                }
                else
                {
                    // Paginated request for all coils
                    int currentPage = 0;
                    if (requestDoc["page"].is<int>())
                    {
                        int reqPage = requestDoc["page"].as<int>();
                        if (reqPage >= 0)
                            currentPage = reqPage;
                    }

                    int pageSize = DEFAULT_WEBSOCKET_PAGE_SIZE;
                    if (requestDoc["pageSize"].is<int>())
                    {
                        pageSize = requestDoc["pageSize"].as<int>();
                    }
                    if (pageSize > MAX_PAGE_SIZE)
                        pageSize = MAX_PAGE_SIZE;
                    if (pageSize <= 0)
                        pageSize = DEFAULT_WEBSOCKET_PAGE_SIZE;

                    // 1st pass: Count total coils
                    int totalCoils = 0;
                    const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
                    for (const auto &info : mappings)
                    {
                        if (info.type == FN_READ_COIL || info.type == FN_WRITE_COIL || info.type == FN_WRITE_MULT_COILS)
                        {
                            totalCoils += info.count;
                        }
                    }

                    int startIndex = currentPage * pageSize;
                    metaDataForClient["page"] = currentPage;
                    metaDataForClient["pageSize"] = pageSize;
                    metaDataForClient["totalCoils"] = totalCoils;
                    metaDataForClient["totalPages"] = (pageSize > 0) ? (totalCoils + pageSize - 1) / pageSize : 0;

                    int currentIndex = 0;
                    bool pageFilled = false;
                    // 2nd pass: Build the JSON for the current page
                    for (const auto &info : mappings)
                    {
                        if (info.type != FN_READ_COIL && info.type != FN_WRITE_COIL && info.type != FN_WRITE_MULT_COILS)
                            continue;

                        Component *component = owner->byId(info.componentId);
                        if (!component)
                            continue;

                        for (short i = 0; i < info.count; ++i)
                        {
                            if (currentIndex >= startIndex && currentIndex < (startIndex + pageSize))
                            {
                                short currentAddr = info.startAddress + i;
                                uint16_t coilValue = (uint16_t)component->mb_tcp_read(const_cast<MB_Registers *>(&info));
                                JsonObject coilObj = dataArrayForClient.add<JsonObject>();
                                coilObj["address"] = currentAddr;
                                coilObj["value"] = coilValue;
                                coilObj["name"] = info.name;
                                coilObj["component"] = component->name;
                                coilObj["id"] = component->id;
                                coilObj["type"] = (int)info.type;
                                coilObj["flags"] = component->flags;
                                coilObj["group"] = info.group;
                            }
                            currentIndex++;
                            if (currentIndex >= (startIndex + pageSize))
                            {
                                pageFilled = true;
                                break;
                            }
                        }
                        if (pageFilled)
                            break;
                    }
                }
            }

            String responseStr;
            serializeJson(finalResponseDoc, responseStr);
            queueWsMessage(client->id(), responseStr);
        }
        else if (command == "get_registers")
        {
            JsonDocument finalResponseDoc;
            finalResponseDoc["type"] = "registers";
            JsonArray dataArrayForClient = finalResponseDoc["data"].to<JsonArray>();
            JsonObject metaDataForClient = finalResponseDoc["meta"].to<JsonObject>();

            int specificAddress = -1;
            if (requestDoc["address"].is<int>())
            {
                specificAddress = requestDoc["address"].as<int>();
            }

            if (modbusManager)
            {
                if (specificAddress != -1)
                {
                    // If a specific address is requested, just use the existing helper.
                    // This is not memory-intensive as it's for one address.
                    _buildRegistersJson(dataArrayForClient, specificAddress);
                    metaDataForClient["page"] = 0;
                    metaDataForClient["pageSize"] = dataArrayForClient.size();
                    metaDataForClient["totalRegisters"] = dataArrayForClient.size();
                    metaDataForClient["totalPages"] = dataArrayForClient.size() > 0 ? 1 : 0;
                }
                else
                {
                    // Paginated request for all registers
                    int currentPage = 0;
                    if (requestDoc["page"].is<int>())
                    {
                        int reqPage = requestDoc["page"].as<int>();
                        if (reqPage >= 0)
                            currentPage = reqPage;
                    }

                    int pageSize = DEFAULT_WEBSOCKET_PAGE_SIZE;
                    if (requestDoc["pageSize"].is<int>())
                    {
                        pageSize = requestDoc["pageSize"].as<int>();
                    }
                    if (pageSize > MAX_PAGE_SIZE)
                        pageSize = MAX_PAGE_SIZE;
                    if (pageSize <= 0)
                        pageSize = DEFAULT_WEBSOCKET_PAGE_SIZE;

                    // 1st pass: Count total registers for pagination metadata
                    int totalRegisters = 0;
                    const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
                    for (const auto &info : mappings)
                    {
                        if (info.type == E_FN_CODE::FN_READ_HOLD_REGISTER ||
                            info.type == E_FN_CODE::FN_READ_INPUT_REGISTER ||
                            info.type == E_FN_CODE::FN_WRITE_HOLD_REGISTER ||
                            info.type == E_FN_CODE::FN_WRITE_MULT_REGISTERS)
                        {
                            totalRegisters += info.count;
                        }
                    }

                    int startIndex = currentPage * pageSize;
                    metaDataForClient["page"] = currentPage;
                    metaDataForClient["pageSize"] = pageSize;
                    metaDataForClient["totalRegisters"] = totalRegisters;
                    metaDataForClient["totalPages"] = (pageSize > 0) ? (totalRegisters + pageSize - 1) / pageSize : 0;

                    int currentIndex = 0;
                    bool pageFilled = false;
                    // 2nd pass: Build the JSON for just the registers on the current page
                    for (const auto &info : mappings)
                    {
                        if (info.type != E_FN_CODE::FN_READ_HOLD_REGISTER &&
                            info.type != E_FN_CODE::FN_READ_INPUT_REGISTER &&
                            info.type != E_FN_CODE::FN_WRITE_HOLD_REGISTER &&
                            info.type != E_FN_CODE::FN_WRITE_MULT_REGISTERS)
                        {
                            continue;
                        }

                        Component *component = owner->byId(info.componentId);
                        if (!component)
                            continue;

                        for (short i = 0; i < info.count; ++i)
                        {
                            if (currentIndex >= startIndex && currentIndex < (startIndex + pageSize))
                            {
                                short currentAddr = info.startAddress + i;
                                // Read value and error
                                uint16_t regValue = (uint16_t)component->mb_tcp_read(const_cast<MB_Registers *>(&info));
                                short error = component->mb_tcp_error(const_cast<MB_Registers *>(&info));

                                JsonObject regObj = dataArrayForClient.add<JsonObject>();
                                regObj["error"] = error;
                                regObj["address"] = currentAddr;
                                regObj["value"] = regValue;
                                regObj["name"] = info.name;
                                regObj["component"] = component->name;
                                regObj["id"] = component->id;
                                regObj["type"] = (int)info.type;
                                regObj["slaveId"] = info.slaveId;
                                regObj["flags"] = component->flags;
                                regObj["group"] = info.group;
                            }
                            currentIndex++;
                            // Optimization: if we've filled the page, we can stop iterating
                            if (currentIndex >= (startIndex + pageSize))
                            {
                                pageFilled = true;
                                break;
                            }
                        }
                        if (pageFilled)
                        {
                            break;
                        }
                    }
                }
            }

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
                queueWsMessage(client->id(), errorStr);
            }
            else
            {
                queueWsMessage(client->id(), responseStr);
            }
        }
#ifdef ENABLE_WEBSOCKET_PB
        else if (command == "get_registers_pb")
        {
            int currentPage = !requestDoc["page"].isNull() ? requestDoc["page"].as<int>() : 0;
            int pageSize = !requestDoc["pageSize"].isNull() ? requestDoc["pageSize"].as<int>() : DEFAULT_WEBSOCKET_PAGE_SIZE;
            if (pageSize <= 0)
                pageSize = DEFAULT_WEBSOCKET_PAGE_SIZE;
            if (pageSize > MAX_PAGE_SIZE)
                pageSize = MAX_PAGE_SIZE;

            int totalRegisters = 0;
            if (modbusManager)
            {
                const Vector<MB_Registers> &mappings = modbusManager->getAddressMappings();
                for (const auto &info : mappings)
                {
                    if (info.type == E_FN_CODE::FN_READ_HOLD_REGISTER ||
                        info.type == E_FN_CODE::FN_READ_INPUT_REGISTER ||
                        info.type == E_FN_CODE::FN_WRITE_HOLD_REGISTER ||
                        info.type == E_FN_CODE::FN_WRITE_MULT_REGISTERS)
                    {
                        totalRegisters += info.count;
                    }
                }
            }

            RegistersResponse response_pb = RegistersResponse_init_zero;
            response_pb.meta.page = currentPage;
            response_pb.meta.pageSize = pageSize;
            response_pb.meta.totalRegisters = totalRegisters;
            response_pb.meta.totalPages = (pageSize > 0) ? (totalRegisters + pageSize - 1) / pageSize : 0;

            RegisterEncodeCtx ctx;
            ctx.modbusManager = this->modbusManager;
            ctx.owner = this->owner;
            ctx.startIndex = currentPage * pageSize;
            ctx.endIndex = ctx.startIndex + pageSize;
            ctx.currentIndex = 0;

            response_pb.data.funcs.encode = &encode_registers_callback;
            response_pb.data.arg = &ctx;

            // Use a vector for the buffer to avoid stack overflow and manage memory safely.
            std::vector<uint8_t> buffer(MAX_WEBSOCKET_MESSAGE_SIZE);
            pb_ostream_t stream = pb_ostream_from_buffer(buffer.data(), buffer.size());

            bool status = pb_encode(&stream, RegistersResponse_fields, &response_pb);

            if (status)
            {
                client->binary(buffer.data(), stream.bytes_written);
            }
            else
            {
                // LS_ERROR("WS #%u: Failed to encode registers protobuf: %s. Buffer might be too small (%d bytes).", client->id(), PB_GET_ERROR(&stream), (int)buffer.size());
                queueWsMessage(client->id(), "{\"type\": \"error\", \"message\": \"Failed to encode protobuf response. Buffer too small?\"}");
            }
        }
#endif // ENABLE_WEBSOCKET_PB
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
                                    LS_ERROR("WS #%u: Set Register %d failed via ModbusManager, error code: %d", client->id(), address, writeResult);
                                }
                            }
                            else
                            {
                                errorMsg = "Component not found for address";
                                LS_ERROR("WS #%u: write_register - No component found for address %d", client->id(), address);
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
                        Log.warningln("WS #%u: write_register - No ModbusManager available.", client->id());
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
                            if (writeResult == E_OK)
                            {
                                success = true;
                            }
                            else
                            {
                                errorMsg = "Failed to write coil (Modbus Error: " + String(writeResult) + ")";
                                LS_ERROR("WS #%u: Set Coil %d failed via ModbusManager, error code: %d", client->id(), address, writeResult);
                            }
                        }
                        else
                        {
                            errorMsg = "Component not found for address";
                            LS_ERROR("WS #%u: write_coil - No component found for address %d", client->id(), address);
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
        else if (command == "test_payload")
        {
            int size = 100;
            if (requestDoc["size"].is<int>())
            {
                size = requestDoc["size"].as<int>();
            }

            if (size > MAX_WEBSOCKET_MESSAGE_SIZE)
            {
                size = MAX_WEBSOCKET_MESSAGE_SIZE;
            }

            uint32_t startHeap = ESP.getFreeHeap();
            JsonDocument responseDoc;
            String payload;
            if (payload.reserve(size))
            {
                for (int i = 0; i < size; i++)
                {
                    payload += 'A';
                }
            }
            else
            {
                payload = "Failed to allocate memory";
            }

            responseDoc["payload"] = payload;
            responseDoc["size"] = size;
            responseDoc["heap_before"] = startHeap;
            responseDoc["heap_after"] = ESP.getFreeHeap();

            sendJsonResponse(client, responseDoc, "test_payload");
        }
        else
        {
            Log.warningln("WS #%u: Unknown command '%s'", client->id(), command.c_str());
            queueWsMessage(client->id(), "{\"type\": \"error\", \"message\": \"Unknown command\"}");
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
    case BROADCAST_ERROR_MESSAGE:
        typeStr = "error_message";
        break;
    default:
        break;
    }

    doc["type"] = typeStr;
    doc["data"] = data;
    String output;
    serializeJson(doc, output);

    if (type == BROADCAST_USER_MESSAGE || type == BROADCAST_ERROR_MESSAGE)
    {
        doc["timestamp"] = millis();
        userMessageHistory.addMessage(output.c_str());
    }

    queueWsMessage(0, output);
}

#ifdef ENABLE_WEBSOCKET
void RESTServer::broadcastBinary(const uint8_t *data, size_t len)
{
    // Check rate limit if needed, or simply broadcast
    if (ws.count() > 0 && ws.availableForWriteAll())
    {
        ws.binaryAll((uint8_t *)data, len);
    }
}
#endif

short RESTServer::onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user, Component *src)
{
    if (verb == E_CALLS::EC_PROTOBUF_UPDATE && user != nullptr)
    {
        PB_UpdateData *info = static_cast<PB_UpdateData *>(user);
        this->broadcastBinary(info->data, info->len);
        return E_OK;
    }

    if (verb == E_CALLS::EC_USER && user != nullptr)
    {
        MB_UpdateData *info = static_cast<MB_UpdateData *>(user);

        // --- Binary Protocol Implementation ---
#ifdef USE_BINARY_UPDATES
        if (info->functionCode == E_FN_CODE::FN_READ_HOLD_REGISTER ||
            info->functionCode == E_FN_CODE::FN_READ_INPUT_REGISTER ||
            info->functionCode == E_FN_CODE::FN_WRITE_HOLD_REGISTER ||
            info->functionCode == E_FN_CODE::FN_WRITE_MULT_REGISTERS)
        {
            BinaryRegisterUpdate update;
            // update.type is initialized to 0x02
            update.slaveId = info->slaveId;
            update.fc = (uint8_t)info->functionCode;
            update.componentId = src ? src->id : 0;
            update.address = info->address;
            update.count = info->count;
            update.reserved = 0;

            // Handle value extraction
            if (info->functionCode == E_FN_CODE::FN_WRITE_MULT_REGISTERS && info->userData != nullptr)
            {
                // For multi-write, we might need a different packet or just send the first one?
                // The current spec assumes single register update mostly.
                // Let's take the first value for now as the 'value' field.
                uint16_t *data = static_cast<uint16_t *>(info->userData);
                update.value = data[0];
            }
            else
            {
                update.value = (uint16_t)info->value;
            }

            this->broadcastBinary((uint8_t *)&update, sizeof(update));
            lastBroadcastTime = millis();
            return E_OK;
        }
        else if (info->functionCode == E_FN_CODE::FN_READ_COIL ||
                 info->functionCode == E_FN_CODE::FN_READ_DISCR_INPUT ||
                 info->functionCode == E_FN_CODE::FN_WRITE_COIL ||
                 info->functionCode == E_FN_CODE::FN_WRITE_MULT_COILS)
        {
            BinaryCoilUpdate update;
            // update.type is initialized to 0x01
            update.slaveId = info->slaveId;
            update.fc = (uint8_t)info->functionCode;
            update.componentId = src ? src->id : 0;
            update.address = info->address;
            update.count = info->count;

            if (info->functionCode == E_FN_CODE::FN_WRITE_MULT_COILS && info->userData != nullptr)
            {
                bool *data = static_cast<bool *>(info->userData);
                update.value = data[0] ? 1 : 0;
            }
            else
            {
                update.value = info->value ? 1 : 0;
            }

            this->broadcastBinary((uint8_t *)&update, sizeof(update));
            lastBroadcastTime = millis();
            return E_OK;
        }
#endif
        // --- End Binary Protocol ---

        // --- JSON Fallback (Original Logic) ---
        // If USE_BINARY_UPDATES is undefined, OR if the message type wasn't handled above (though all types are covered)
        // we fall through to here. Note: If USE_BINARY_UPDATES is defined, we returned E_OK above.

        JsonDocument doc;
        doc["slaveId"] = info->slaveId;
        doc["address"] = info->address;
        doc["fc"] = info->functionCode;
        doc["count"] = info->count;
        doc["id"] = src ? src->id : 0;
        if ((info->functionCode == E_FN_CODE::FN_WRITE_MULT_REGISTERS || info->functionCode == E_FN_CODE::FN_WRITE_MULT_COILS) && info->userData != nullptr)
        {
            JsonArray values = doc["values"].to<JsonArray>();
            if (info->functionCode == E_FN_CODE::FN_WRITE_MULT_REGISTERS)
            {
                uint16_t *data = static_cast<uint16_t *>(info->userData);
                for (int i = 0; i < info->count; i++)
                {
                    values.add(data[i]);
                }
            }
            else
            { // It's FN_WRITE_MULT_COILS
                bool *data = static_cast<bool *>(info->userData);
                for (int i = 0; i < info->count; i++)
                {
                    values.add(data[i]);
                }
            }
        }
        else
        {
            doc["value"] = info->value;
            doc["count"] = 1;
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
            // Log.warningln(F("[RESTServer] Unknown function code %d for broadcast mapping from component %s. Slave: %d, Address: %d, Count: %d"), (int)info->functionCode, src ? src->name.c_str() : "null", info->slaveId, info->address, info->count);
            break;
        }
        if (msgType != BROADCAST_UNKNOWN)
        {
            this->broadcast(msgType, doc);
            lastBroadcastTime = millis();
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
uint8_t RESTServer::getConnectedClientsCount() const
{
    return ws.count();
}
#endif

#ifdef ENABLE_WEBSOCKET
void RESTServer::queueWsMessage(uint32_t clientId, const String &message)
{
    WsMessage msg = {clientId, message};
    // Serial.printf("WS: Queueing message for client #%u %s", clientId, message.c_str());
    if (!wsTxQueue.push(msg))
    {
        static unsigned long lastWarn = 0;
        if (millis() - lastWarn > 1000)
        {
            // L_WARN("WS: Queue full, dropping message for client #%u", clientId);
            lastWarn = millis();
        }
    }
}

void RESTServer::processWsQueue()
{
    if (millis() - lastWsSendTime < 100)
    {
        return;
    }

    WsMessage msg;
    if (wsTxQueue.peek(msg))
    {
        if (msg.clientId == 0)
        {
            // Broadcast message
            if (ws.availableForWriteAll())
            {
                ws.textAll(msg.message);
                wsTxQueue.pop(msg);
                lastWsSendTime = millis();
            }
            else
            {
                // Retry later
            }
        }
        else
        {
            AsyncWebSocketClient *client = ws.client(msg.clientId);
            if (client && client->status() == WS_CONNECTED)
            {
                if (client->canSend())
                {
                    if (client->text(msg.message))
                    {
                        wsTxQueue.pop(msg); // Remove after sending
                        lastWsSendTime = millis();
                    }
                    else
                    {
                        // Send failed (buffer full?), retry later
                    }
                }
                else
                {
                    // Client busy, wait.
                }
            }
            else
            {
                // Client disconnected, drop message
                wsTxQueue.pop(msg);
            }
        }
    }
}
#endif

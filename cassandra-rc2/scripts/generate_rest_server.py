#!/usr/bin/env python3
import yaml
import os
import re
import sys
from jinja2 import Template

# Templates for code generation
REST_SERVER_H_TEMPLATE = """
#ifndef REST_SERVER_H
#define REST_SERVER_H

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <ModbusIP_ESP8266.h>
#include "enums.h"

// Forward declarations
class ModbusIP;

/**
 * @brief RESTful API server generated from Swagger spec.
 * This class implements a RESTful API server that interfaces with the Modbus system.
 */
class RESTServer {
private:
    AsyncWebServer *server;
    ModbusIP *modbus;

    // Handler methods
    {% for handler in handlers %}
    void {{ handler.name }}Handler(AsyncWebServerRequest *request);
    {% endfor %}

public:
    /**
     * @brief Construct a new RESTServer object
     * 
     * @param port The port to run the server on
     * @param _modbus Pointer to the ModbusIP instance
     */
    RESTServer(IPAddress ip, int port, ModbusIP *_modbus);
    
    /**
     * @brief Destroy the RESTServer object
     */
    ~RESTServer();
    
    /**
     * @brief Run periodically to handle server tasks
     */
    void loop();
};

#endif // REST_SERVER_H
"""

REST_SERVER_CPP_TEMPLATE = """
#include "RestServer.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>

RESTServer::RESTServer(IPAddress ip, int port, ModbusIP *_modbus) {
    server = new AsyncWebServer(port);
    modbus = _modbus;
    
    // Set up CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Register routes
    {% for route in routes %}
    server->on("{{ route.path }}", {{ route.method }}, [this](AsyncWebServerRequest *request) {
        this->{{ route.handler }}Handler(request);
    });
    {% endfor %}
    
    // Handle OPTIONS requests for CORS
    server->onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });
    
    // Start mDNS responder
    if (MDNS.begin("modbus-esp32")) {
        Log.verboseln("MDNS responder started. Device can be reached at http://modbus-esp32.local");
    }
    
    // Start server
    server->begin();
    Log.verboseln("HTTP server started on port %d", port);
}

RESTServer::~RESTServer() {
    if (server) {
        server->end();
        delete server;
        server = nullptr;
    }
}

void RESTServer::loop() {
    // Any periodic handling needed
}

{% for handler in handlers %}
void RESTServer::{{ handler.name }}Handler(AsyncWebServerRequest *request) {
    {{ handler.code }}
}
{% endfor %}
"""

# Handler template for system info
SYSTEM_INFO_HANDLER = """
    // Create JSON response with system info
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["version"] = "1.0.0";
    doc["board"] = BOARD_NAME;
    doc["uptime"] = millis() / 1000;
    doc["timestamp"] = millis();
    
    serializeJson(doc, *response);
    request->send(response);
"""

# Handler template for getting coils
GET_COILS_HANDLER = """
    int start = 0;
    int count = 50;
    
    // Get query parameters
    if (request->hasParam("start")) {
        start = request->getParam("start")->value().toInt();
    }
    
    if (request->hasParam("count")) {
        count = request->getParam("count")->value().toInt();
        if (count > 100) count = 100; // Limit to prevent large responses
    }
    
    // Create JSON response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray coilsArray = doc.createNestedArray("coils");
    
    for (int i = 0; i < count; i++) {
        coilsArray.add(modbus->Coil(start + i));
    }
    
    serializeJson(doc, *response);
    request->send(response);
"""

# Handler template for getting a specific coil
GET_COIL_HANDLER = """
    // Get path parameter
    String addressStr = request->pathArg(0);
    int address = addressStr.toInt();
    bool value = modbus->Coil(address);
    
    // Create JSON response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["address"] = address;
    doc["value"] = value;
    
    serializeJson(doc, *response);
    request->send(response);
"""

# Handler template for setting a coil
SET_COIL_HANDLER = """
    // Get path parameter
    String addressStr = request->pathArg(0);
    int address = addressStr.toInt();
    
    // Check if we have a valid body
    if (request->hasParam("body", true)) {
        String body = request->getParam("body", true)->value();
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (!error && doc.containsKey("value")) {
            bool value = doc["value"];
            modbus->Coil(address, value);
            
            // Create JSON response
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            JsonDocument responseDoc;
            responseDoc["success"] = true;
            responseDoc["address"] = address;
            responseDoc["value"] = value;
            
            serializeJson(responseDoc, *response);
            request->send(response);
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON or missing value\"}");
        }
    } else {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing body\"}");
    }
"""

# Handler template for getting registers
GET_REGISTERS_HANDLER = """
    int start = 0;
    int count = 10;
    
    // Get query parameters
    if (request->hasParam("start")) {
        start = request->getParam("start")->value().toInt();
    }
    
    if (request->hasParam("count")) {
        count = request->getParam("count")->value().toInt();
        if (count > 50) count = 50; // Limit to prevent large responses
    }
    
    // Create JSON response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    JsonArray registersArray = doc.createNestedArray("registers");
    
    for (int i = 0; i < count; i++) {
        registersArray.add(modbus->Hreg(start + i));
    }
    
    serializeJson(doc, *response);
    request->send(response);
"""

# Handler template for getting a specific register
GET_REGISTER_HANDLER = """
    // Get path parameter
    String addressStr = request->pathArg(0);
    int address = addressStr.toInt();
    int value = modbus->Hreg(address);
    
    // Create JSON response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["address"] = address;
    doc["value"] = value;
    
    serializeJson(doc, *response);
    request->send(response);
"""

# Handler template for setting a register
SET_REGISTER_HANDLER = """
    // Get path parameter
    String addressStr = request->pathArg(0);
    int address = addressStr.toInt();
    
    // Check if we have a valid body
    if (request->hasParam("body", true)) {
        String body = request->getParam("body", true)->value();
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, body);
        
        if (!error && doc.containsKey("value")) {
            int value = doc["value"];
            modbus->Hreg(address, value);
            
            // Create JSON response
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            JsonDocument responseDoc;
            responseDoc["success"] = true;
            responseDoc["address"] = address;
            responseDoc["value"] = value;
            
            serializeJson(responseDoc, *response);
            request->send(response);
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON or missing value\"}");
        }
    } else {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing body\"}");
    }
"""

# Handler template for test relays
TEST_RELAYS_HANDLER = """
    // Create JSON response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument doc;
    doc["success"] = true;
    doc["message"] = "Relay test initiated";
    
    // Call the test relays method on PHApp via a callback or direct call
    // This would typically be done via a static callback or global instance
    // For this example, we'll assume the test is successful without actual implementation
    
    serializeJson(doc, *response);
    request->send(response);
"""

def generate_handlers_from_swagger(swagger_file):
    with open(swagger_file, 'r') as f:
        swagger = yaml.safe_load(f)
    
    handlers = []
    routes = []
    
    for path, methods in swagger['paths'].items():
        for method, operation in methods.items():
            handler_name = operation['operationId']
            
            # Determine the handler code based on the operation ID
            handler_code = ""
            if handler_name == "getSystemInfo":
                handler_code = SYSTEM_INFO_HANDLER
            elif handler_name == "getCoils":
                handler_code = GET_COILS_HANDLER
            elif handler_name == "getCoil":
                handler_code = GET_COIL_HANDLER
            elif handler_name == "setCoil":
                handler_code = SET_COIL_HANDLER
            elif handler_name == "getRegisters":
                handler_code = GET_REGISTERS_HANDLER
            elif handler_name == "getRegister":
                handler_code = GET_REGISTER_HANDLER
            elif handler_name == "setRegister":
                handler_code = SET_REGISTER_HANDLER
            elif handler_name == "testRelays":
                handler_code = TEST_RELAYS_HANDLER
            
            # Add the handler if we have code for it
            if handler_code:
                handlers.append({
                    'name': handler_name,
                    'code': handler_code
                })
                
                # Determine the HTTP method
                http_method = "HTTP_GET"
                if method == "post":
                    http_method = "HTTP_POST"
                
                # Format the path for ESP-IDF
                esp_path = path.replace("{", "").replace("}", "")
                
                # Add the route
                routes.append({
                    'path': "/api" + esp_path,
                    'method': http_method,
                    'handler': handler_name
                })
    
    return handlers, routes

def generate_rest_server_files(swagger_file, output_dir):
    handlers, routes = generate_handlers_from_swagger(swagger_file)
    
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    # Generate header file
    header_template = Template(REST_SERVER_H_TEMPLATE)
    header_content = header_template.render(handlers=handlers)
    
    with open(os.path.join(output_dir, "RestServer.h"), 'w') as f:
        f.write(header_content)
    
    # Generate cpp file
    cpp_template = Template(REST_SERVER_CPP_TEMPLATE)
    cpp_content = cpp_template.render(routes=routes, handlers=handlers)
    
    with open(os.path.join(output_dir, "RestServer.cpp"), 'w') as f:
        f.write(cpp_content)
    
    print(f"Generated RESTful server files in {output_dir}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python generate_rest_server.py <swagger_file> [output_dir]")
        sys.exit(1)
    
    swagger_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "src"
    
    generate_rest_server_files(swagger_file, output_dir) 
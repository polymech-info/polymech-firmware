#ifndef REST_SERVER_H
#define REST_SERVER_H

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <AsyncTCP.h>
#include <enums.h>
#include "MessageQueue.h"

#include <Component.h>
#include <Bridge.h>

#ifdef ENABLE_WEBSOCKET
#include <AsyncWebSocket.h>
#endif


#define MAX_JSON_DOCUMENT_SIZE 1024

#define MAX_WEBSOCKET_MESSAGE_SIZE 4096 * 3

// Define BroadcastMessageType enum here
typedef enum : uint8_t {
    BROADCAST_UNKNOWN = 0,
    BROADCAST_COIL_UPDATE,
    BROADCAST_REGISTER_UPDATE,
    BROADCAST_LOG_ENTRY,
    BROADCAST_SYSTEM_STATUS,
    BROADCAST_USER_DEFINED,
    BROADCAST_USER_MESSAGE
} BroadcastMessageType;

class ModbusTCP;

/**
 * @brief RESTful API server generated from Swagger spec.
 * This class implements a RESTful API server that interfaces with the Modbus system.
 */
class RESTServer : public Component { // Inherit from Component

public:
    AsyncWebServer server;
    ModbusTCP* modbusManager;   

#ifdef ENABLE_WEBSOCKET
    AsyncWebSocket ws;
    void setupWebSocket(); // Initialize WebSocket handlers
    void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len);
    uint8_t getConnectedClientsCount() const; // Get number of connected WebSocket clients
#endif
    // Handler methods    
    void getSystemInfoHandler(AsyncWebServerRequest *request);    
    void getCoilsHandler(AsyncWebServerRequest *request);    
    void getCoilHandler(AsyncWebServerRequest *request);    
    void setCoilQueryHandler(AsyncWebServerRequest *request);    
    void getRegistersHandler(AsyncWebServerRequest *request);    
    void getRegisterHandler(AsyncWebServerRequest *request);    
    void getLogLevelHandler(AsyncWebServerRequest *request);
    void setLogLevelHandler(AsyncWebServerRequest *request);    
    void getRtuOperationQueueHandler(AsyncWebServerRequest *request);
    void getMappingsHandler(AsyncWebServerRequest *request);
    void listFsHandler(AsyncWebServerRequest *request);
    void setupRoutes();

public:
    short onRun() override;
    /**
     * @brief Construct a new RESTServer object
     * 
     * @param port The port to run the server on
     * @param manager Pointer to the ModbusManager instance.
     * @param app Pointer to the PHApp instance.
     */
    RESTServer(const IPAddress& ip, uint16_t port, ModbusTCP* manager, Component *owner);
    
    /**
     * @brief Destroy the RESTServer object
     */
    ~RESTServer();
    
    /**
     * @brief Run periodically to handle server tasks
     */
    short loop() override;

    /**
     * @brief Setup the RESTServer
     */
    short setup() override;

    /**
     * @brief Handles incoming messages (e.g., from PHApp for broadcasts).
     */
    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user, Component *src) override;

#ifdef ENABLE_WEBSOCKET
    /**
     * @brief Broadcast a message to all connected WebSocket clients.
     * 
     * @param type The type of message to send.
     * @param data The data to send with the message.
     */
    void broadcast(BroadcastMessageType type, const JsonDocument& data); // New broadcast method
#endif

private:
    // Message History
    MessageQueue userMessageHistory;

    void _buildRegistersJson(JsonArray& registersArray, int specificAddress = -1);
    short mountLittleFS();
};
#endif // REST_SERVER_H

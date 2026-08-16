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

#include <functional>
#include <vector>
#include <utility>

#ifdef ENABLE_WEBSOCKET
#include <AsyncWebSocket.h>
#endif

#define MAX_JSON_DOCUMENT_SIZE 256
#define WS_BROADCAST_RATE_LIMIT_MS 100
#define MAX_WEBSOCKET_MESSAGE_SIZE 2048
#define DEFAULT_WS_COMMAND_HANDLER_CAPACITY 3

typedef enum : uint8_t
{
    BROADCAST_UNKNOWN = 0,
    BROADCAST_COIL_UPDATE,
    BROADCAST_REGISTER_UPDATE,
    BROADCAST_LOG_ENTRY,
    BROADCAST_SYSTEM_STATUS,
    BROADCAST_USER_DEFINED,
    BROADCAST_USER_MESSAGE,
    BROADCAST_ERROR_MESSAGE
} BroadcastMessageType;

struct WsMessage
{
    uint32_t clientId;
    String message;
};

// --- Binary Protocol Structs ---
#pragma pack(push, 1)
struct BinaryCoilUpdate
{
    uint8_t type = 0x01; // BROADCAST_COIL_UPDATE
    uint8_t slaveId;
    uint8_t fc;
    uint8_t value; // 0 or 1
    uint16_t componentId;
    uint16_t address;
    uint16_t count;
};

struct BinaryRegisterUpdate
{
    uint8_t type = 0x02; // BROADCAST_REGISTER_UPDATE
    uint8_t slaveId;
    uint8_t fc;
    uint8_t reserved; // Padding
    uint16_t componentId;
    uint16_t address;
    uint16_t count;
    uint16_t value;
};
#pragma pack(pop)
// ----------------------------

class WsTxQueue
{
public:
    WsTxQueue() : head(0), tail(0), count(0) {}

    bool push(const WsMessage &msg)
    {
        if (count >= WS_SEND_QUEUE_SIZE)
            return false;
        buffer[tail] = msg;
        tail = (tail + 1) % WS_SEND_QUEUE_SIZE;
        count++;
        return true;
    }

    bool pop(WsMessage &msg)
    {
        if (count == 0)
            return false;
        msg = buffer[head];
        head = (head + 1) % WS_SEND_QUEUE_SIZE;
        count--;
        return true;
    }

    bool peek(WsMessage &msg)
    {
        if (count == 0)
            return false;
        msg = buffer[head];
        return true;
    }

    bool isEmpty() const { return count == 0; }
    bool isFull() const { return count >= WS_SEND_QUEUE_SIZE; }
    size_t size() const { return count; }

private:
    WsMessage buffer[WS_SEND_QUEUE_SIZE];
    size_t head;
    size_t tail;
    size_t count;
};

class ModbusTCP;

/**
 * @brief RESTful API server generated from Swagger spec.
 * This class implements a RESTful API server that interfaces with the Modbus system.
 */
class RESTServer : public Component
{
public:
#ifdef ENABLE_WEBSOCKET
    using WsCommandHandler = std::function<void(AsyncWebSocketClient *client, JsonVariant &json)>;
#endif
    AsyncWebServer server;
    ModbusTCP *modbusManager;

#ifdef ENABLE_WEBSOCKET
    AsyncWebSocket ws;
    void setupWebSocket(); // Initialize WebSocket handlers
    void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len);
    uint8_t getConnectedClientsCount() const; // Get number of connected WebSocket clients
    unsigned long lastBroadcastTime = 0;
    void registerWsCommandHandler(const String &command, WsCommandHandler handler);
    void queueWsMessage(uint32_t clientId, const String &message);
    void processWsQueue();
    void sendJsonResponse(AsyncWebSocketClient *client, const JsonDocument &doc, const char *type);
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
    RESTServer(const IPAddress &ip, uint16_t port, ModbusTCP *manager, Component *owner);

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
    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user, Component *src) override;

#ifdef ENABLE_WEBSOCKET
    /**
     * @brief Broadcast a message to all connected WebSocket clients.
     *
     * @param type The type of message to send.
     * @param data The data to send with the message.
     */
    void broadcast(BroadcastMessageType type, const JsonDocument &data); // New broadcast method
    void broadcastBinary(const uint8_t *data, size_t len);
#endif

private:
#ifdef ENABLE_WEBSOCKET
    std::vector<std::pair<String, WsCommandHandler>> wsCommandHandlers;
    WsTxQueue wsTxQueue;
    unsigned long lastWsSendTime = 0;
#endif
    // Message History
    MessageQueue userMessageHistory;

    void _buildRegistersJson(JsonArray &registersArray, int specificAddress = -1);
    short mountLittleFS();
};
#endif // REST_SERVER_H

#include "Logger.h"
#include "config.h"

#include <modbus/ModbusTCP.h>
#include <modbus/Modbus.h>
#include <modbus/ModbusTypes.h>

// This define is needed here because the library doesn't know about the app's config.h
// It must come *before* RestServer.h is included to expose the WebSocket methods.

#ifndef DISABLE_LOGGING
#include <components/RestServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

class PrintTarget : public ILogTarget {
public:
    explicit PrintTarget(Print* output) : _output(output) {}
    void log(const Component* sender, LogLevel level, const char* message) override {
        if (_output) {
            // Re-add prefix for display
            const char* level_str = "";
            switch(level) {
                case L_FATAL:   level_str = "FATAL: ";   break;
                case L_ERROR:   level_str = "ERROR: ";   break;
                case L_WARNING: level_str = "WARNING: "; break;
                case L_LEVEL_INFO:    level_str = "INFO: ";    break;
                case L_TRACE:   level_str = "TRACE: ";   break;
                case L_VERBOSE: level_str = "VERBOSE: "; break;
                default: break;
            }
            _output->print(level_str);
            if (sender) {
                _output->printf("[%s:%d] ", sender->name.c_str(), sender->id);
            }
            _output->println(message);
        }
    }
    LogTargetType getType() const override { return TARGET_TYPE_PRINT; }
private:
    Print* _output;
};

#ifdef ENABLE_WEBSOCKET

class WebSocketTarget : public ILogTarget {
public:
    explicit WebSocketTarget(RESTServer* server) : _server(server), _lastBroadcastTime(0) {
        for (int i = 0; i < LOG_QUEUE_SIZE; ++i) {
            _logQueue[i].flags = 0;
        }
    }

    void log(const Component* sender, LogLevel level, const char* message) override {
        if (level > getLevel()) return;

        // Find a free slot
        for (int i = 0; i < LOG_QUEUE_SIZE; ++i) {
            if (_logQueue[i].flags == 0) { // is free
                _logQueue[i].level = level;
                _logQueue[i].message = message;
                _logQueue[i].componentId = sender ? (uint16_t)sender->id : (uint16_t)0;
                _logQueue[i].componentName = sender ? sender->name : String("");
                _logQueue[i].flags = FLAG_USED;
                return;
            }
        }
    }
    
    void checkAndBroadcast() {
        if (_isBroadcasting || !_server) return;
        
        if (millis() - _lastBroadcastTime < BROADCAST_INTERVAL_MS) {
            return;
        }

        _isBroadcasting = true;

        for (int i = 0; i < LOG_QUEUE_SIZE; ++i) {
            if (_logQueue[i].flags == FLAG_USED) {
                JsonDocument doc;
                doc["level"] = _logQueue[i].level;
                doc["message"] = _logQueue[i].message;
                if (_logQueue[i].componentId != 0) {
                    doc["id"] = _logQueue[i].componentId;
                    doc["name"] = _logQueue[i].componentName;
                }
                _server->broadcast(BROADCAST_LOG_ENTRY, doc);
                _logQueue[i].flags = 0; // Mark as free
                _lastBroadcastTime = millis();
                break; // Send one at a time
            }
        }
        
        _isBroadcasting = false;
    }

    LogTargetType getType() const override { return TARGET_TYPE_WEBSOCKET; }
private:
    RESTServer* _server;
    enum LogEntryFlags {
        FLAG_USED = 1
    };
    struct LogEntry {
        LogLevel level;
        String message;
        uint16_t componentId;
        String componentName;
        uint8_t flags;
    };
    static const uint8_t LOG_QUEUE_SIZE = 40;
    static const unsigned long BROADCAST_INTERVAL_MS = 30;
    LogEntry _logQueue[LOG_QUEUE_SIZE];
    unsigned long _lastBroadcastTime;
    
    static bool _isBroadcasting;
};
bool WebSocketTarget::_isBroadcasting = false;
#endif

#ifdef ENABLE_LITTLEFS
class FileTarget : public ILogTarget {
public:
    FileTarget() {
        _logBuffer.reserve(BUFFER_SIZE);
    }

    ~FileTarget() {
        if (!_logBuffer.empty()) {
            dumpToFile();
        }
    }

    void log(const Component* sender, LogLevel level, const char* message) override {
        if (level > getLevel() || _logBuffer.size() >= BUFFER_SIZE) return;


        _logBuffer.push_back({level, String(message), millis(), sender ? (uint16_t)sender->id : (uint16_t)0, sender ? sender->name : String("")});

    }

    void checkAndDump() {
        if (_logBuffer.size() >= BUFFER_SIZE) {
            dumpToFile();
        }
    }

    LogTargetType getType() const override { return TARGET_TYPE_FILE; }

private:
    void dumpToFile() {
        

        if (!LittleFS.begin(true)) {
            
            return;
        }

        File file = LittleFS.open("/log-last.json", "w");
        if (!file) {
            
            return;
        }

        JsonDocument doc;
        JsonArray array = doc.to<JsonArray>();

        for (const auto& entry : _logBuffer) {
            JsonObject obj = array.add<JsonObject>();
            obj["level"] = entry.level;
            obj["msg"] = entry.message;
            obj["ts"] = entry.timestamp;
            if (entry.componentId != 0) {
                obj["id"] = entry.componentId;
                obj["name"] = entry.componentName;
            }
        }
        
        serializeJson(doc, file);
        file.close();
        _logBuffer.clear();

        
    }

    struct LogEntry {
        LogLevel level;
        String message;
        unsigned long timestamp;
        uint16_t componentId;
        String componentName;
    };
    static const uint8_t BUFFER_SIZE = 100;
    std::vector<LogEntry> _logBuffer;
    
};
#endif

#endif // DISABLE_LOGGING

#ifndef DISABLE_LOGGING
Logger* Logger::s_esp_log_instance = nullptr;
#endif

Logger::Logger(Component *owner, ushort componentId, ushort modbusAddress)
    : Component("Logger", componentId, Component::COMPONENT_DEFAULT, owner, 0)
    , modbusAddress(modbusAddress)
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
#ifndef DISABLE_LOGGING
    clear();
    
    setLevel(L_VERBOSE);
#endif

    m_modbus_block[(ushort)E_Logger_MB_Offset::MB_LOG_LEVEL] = INIT_MODBUS_BLOCK_TCP(
        this->modbusAddress, 
        (ushort)E_Logger_MB_Offset::MB_LOG_LEVEL, 
        E_FN_CODE::FN_WRITE_HOLD_REGISTER, 
        MB_ACCESS_READ_WRITE,
        "Log Level (0:Silent, 1:Fatal, ... 6:Verbose)",
        "Logger"
    );
    m_modbus_block[(ushort)E_Logger_MB_Offset::COIL_TARGET_PRINT] = INIT_MODBUS_BLOCK_TCP(
        this->modbusAddress,
        (ushort)E_Logger_MB_Offset::COIL_TARGET_PRINT,
        E_FN_CODE::FN_WRITE_COIL,
        MB_ACCESS_READ_WRITE,
        "Enable Print Target",
        "Logger"
    );
    m_modbus_block[(ushort)E_Logger_MB_Offset::COIL_TARGET_WEBSOCKET] = INIT_MODBUS_BLOCK_TCP(
        this->modbusAddress,
        (ushort)E_Logger_MB_Offset::COIL_TARGET_WEBSOCKET,
        E_FN_CODE::FN_WRITE_COIL,
        MB_ACCESS_READ_WRITE,
        "Enable WebSocket Target",
        "Logger"
    );
    m_modbus_block[(ushort)E_Logger_MB_Offset::COIL_TARGET_FILE] = INIT_MODBUS_BLOCK_TCP(
        this->modbusAddress,
        (ushort)E_Logger_MB_Offset::COIL_TARGET_FILE,
        E_FN_CODE::FN_WRITE_COIL,
        MB_ACCESS_READ_WRITE,
        "Enable File Target",
        "Logger"
    );

    m_modbus_view.data = m_modbus_block;
    m_modbus_view.count = (ushort)E_Logger_MB_Offset::COUNT;
}

Logger::~Logger()
{
#ifndef DISABLE_LOGGING
    if (_webSocketTarget) {
        delete _webSocketTarget;
    }
    if (_fileTarget) {
        delete _fileTarget;
    }
#endif
}

short Logger::setup()
{
    return 0;
}

short Logger::loop()
{
    Component::loop();
#ifndef DISABLE_LOGGING
    if (_webSocketTarget) {
        _webSocketTarget->checkAndBroadcast();
    }
    if (_fileTarget) {
        _fileTarget->checkAndDump();
    }
#endif
    return E_OK;
}

// --- Modbus Overrides ---
void Logger::mb_tcp_register(ModbusTCP *manager)
{
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS))
        return;

    modbusTCP = manager;
    uint16_t instanceBaseAddr = mb_tcp_base_address();
    ModbusBlockView *blocksView = mb_tcp_blocks();
    
    for (int i = 0; i < blocksView->count; ++i)
    {
        MB_Registers info = blocksView->data[i];
        info.componentId = this->id;
        manager->registerModbus(this, info);
    }
}

ModbusBlockView *Logger::mb_tcp_blocks() const
{
    return &m_modbus_view;
}

short Logger::mb_tcp_read(MB_Registers *reg)
{
    ushort offset = reg->startAddress - modbusAddress;
    switch(offset) {
        case (ushort)E_Logger_MB_Offset::MB_LOG_LEVEL:
            return (short)_level;
        case (ushort)E_Logger_MB_Offset::COIL_TARGET_PRINT:
            return TEST(_targetMask, TARGET_TYPE_PRINT);
        case (ushort)E_Logger_MB_Offset::COIL_TARGET_WEBSOCKET:
            return TEST(_targetMask, TARGET_TYPE_WEBSOCKET);
        case (ushort)E_Logger_MB_Offset::COIL_TARGET_FILE:
            return TEST(_targetMask, TARGET_TYPE_FILE);
    }
    return 0;
}

short Logger::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    ushort offset = reg->startAddress - modbusAddress;
    switch(offset) {
        case (ushort)E_Logger_MB_Offset::MB_LOG_LEVEL:
            if (networkValue >= L_SILENT && networkValue <= L_VERBOSE) {
                setLevel((LogLevel)networkValue);
                return E_OK;
            }
            return E_INVALID_PARAMETER;
        case (ushort)E_Logger_MB_Offset::COIL_TARGET_PRINT:
            SET_BIT_TO(_targetMask, TARGET_TYPE_PRINT, networkValue);
            return E_OK;
        case (ushort)E_Logger_MB_Offset::COIL_TARGET_WEBSOCKET:
            SET_BIT_TO(_targetMask, TARGET_TYPE_WEBSOCKET, networkValue);
            return E_OK;
        case (ushort)E_Logger_MB_Offset::COIL_TARGET_FILE:
            SET_BIT_TO(_targetMask, TARGET_TYPE_FILE, networkValue);
            return E_OK;
    }
    return E_INVALID_PARAMETER;
}

uint16_t Logger::mb_tcp_base_address() const
{
    return modbusAddress;
}

// --- Logging ---
#ifndef DISABLE_LOGGING
void Logger::setLevel(LogLevel level) {
    _level = level;
}

void Logger::setTargetMask(uint8_t mask) {
    _targetMask = mask;
}

uint8_t Logger::getTargetMask() const {
    return _targetMask;
}

bool Logger::addTarget(ILogTarget* target) {
    if (_numTargets < MAX_TARGETS && target) {
        _targets[_numTargets++] = target;
        return true;
    }
    return false;
}

bool Logger::addPrintTarget(Print* output) {
    if (output) {
        PrintTarget* printTarget = new PrintTarget(output);
        return addTarget(printTarget);
    }
    return false;
}

bool Logger::addWebSocketTarget(RESTServer* server) {
#ifdef ENABLE_WEBSOCKET
    if (server && !_webSocketTarget) {
        _webSocketTarget = new WebSocketTarget(server);
        return addTarget(_webSocketTarget);
    }
    return _webSocketTarget != nullptr;
#else
    return false;
#endif
}

bool Logger::addFileTarget() {
#ifdef ENABLE_LITTLEFS
    if (!_fileTarget) {
        _fileTarget = new FileTarget();
        return addTarget(_fileTarget);
    }
    return true; // Already added
#else
    return false;
#endif
}

void Logger::log(const Component* sender, LogLevel level, const char* format, ...) {
    if (level > _level || level == L_SILENT) return;

    char temp_buffer[LOG_BUFFER_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(temp_buffer, LOG_BUFFER_LINE_LENGTH, format, args);
    va_end(args);

    dispatch(sender, level, temp_buffer);
}

void Logger::log(const Component* sender, LogLevel level, const __FlashStringHelper* format, ...) {
    if (level > _level || level == L_SILENT) return;

    char temp_buffer[LOG_BUFFER_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf_P(temp_buffer, LOG_BUFFER_LINE_LENGTH, (PGM_P)format, args);
    va_end(args);
    
    dispatch(sender, level, temp_buffer);
}

void Logger::log(const Component* sender, LogLevel level, const String& format, ...) {
    if (level > _level || level == L_SILENT) return;

    char temp_buffer[LOG_BUFFER_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(temp_buffer, LOG_BUFFER_LINE_LENGTH, format.c_str(), args);
    va_end(args);

    dispatch(sender, level, temp_buffer);
}

#define GEN_LOG_METHOD(name, level) \
    void Logger::name(const Component* sender, const char* format, ...) { \
        if (level > _level || level == L_SILENT) return; \
        va_list args; \
        va_start(args, format); \
        char temp[LOG_BUFFER_LINE_LENGTH]; \
        vsnprintf(temp, sizeof(temp), format, args); \
        va_end(args); \
        log(sender, level, temp); \
    }

#define GEN_LOG_METHOD_F(name, level) \
    void Logger::name(const Component* sender, const __FlashStringHelper* format, ...) { \
        if (level > _level || level == L_SILENT) return; \
        va_list args; \
        va_start(args, format); \
        char temp[LOG_BUFFER_LINE_LENGTH]; \
        vsnprintf_P(temp, sizeof(temp), (PGM_P)format, args); \
        va_end(args); \
        log(sender, level, temp); \
    }

#define GEN_LOG_METHOD_S(name, level) \
    void Logger::name(const Component* sender, const String& format, ...) { \
        if (level > _level || level == L_SILENT) return; \
        va_list args; \
        va_start(args, format); \
        char temp[LOG_BUFFER_LINE_LENGTH]; \
        vsnprintf(temp, sizeof(temp), format.c_str(), args); \
        va_end(args); \
        log(sender, level, temp); \
    }

GEN_LOG_METHOD(fatal, L_FATAL)
GEN_LOG_METHOD_F(fatal, L_FATAL)
GEN_LOG_METHOD(error, L_ERROR)
GEN_LOG_METHOD_F(error, L_ERROR)
GEN_LOG_METHOD(warn, L_WARNING)
GEN_LOG_METHOD_F(warn, L_WARNING)
GEN_LOG_METHOD(info, L_LEVEL_INFO)
GEN_LOG_METHOD_F(info, L_LEVEL_INFO)
GEN_LOG_METHOD(trace, L_TRACE)
GEN_LOG_METHOD_F(trace, L_TRACE)
GEN_LOG_METHOD(verbose, L_VERBOSE)
GEN_LOG_METHOD_F(verbose, L_VERBOSE)

#define GEN_LOG_METHOD_NULL_SENDER(name, level) \
    void Logger::name(const char* format, ...) { \
        if (level > _level || level == L_SILENT) return; \
        va_list args; \
        va_start(args, format); \
        char temp[LOG_BUFFER_LINE_LENGTH]; \
        vsnprintf(temp, sizeof(temp), format, args); \
        va_end(args); \
        dispatch(nullptr, level, temp); \
    }

#define GEN_LOG_METHOD_F_NULL_SENDER(name, level) \
    void Logger::name(const __FlashStringHelper* format, ...) { \
        if (level > _level || level == L_SILENT) return; \
        va_list args; \
        va_start(args, format); \
        char temp[LOG_BUFFER_LINE_LENGTH]; \
        vsnprintf_P(temp, sizeof(temp), (PGM_P)format, args); \
        va_end(args); \
        dispatch(nullptr, level, temp); \
    }

#define GEN_LOG_METHOD_S_NULL_SENDER(name, level) \
    void Logger::name(const String& format, ...) { \
        if (level > _level || level == L_SILENT) return; \
        va_list args; \
        va_start(args, format); \
        char temp[LOG_BUFFER_LINE_LENGTH]; \
        vsnprintf(temp, sizeof(temp), format.c_str(), args); \
        va_end(args); \
        dispatch(nullptr, level, temp); \
    }

GEN_LOG_METHOD_NULL_SENDER(fatal, L_FATAL)
GEN_LOG_METHOD_F_NULL_SENDER(fatal, L_FATAL)
GEN_LOG_METHOD_S_NULL_SENDER(fatal, L_FATAL)
GEN_LOG_METHOD_NULL_SENDER(error, L_ERROR)
GEN_LOG_METHOD_F_NULL_SENDER(error, L_ERROR)
GEN_LOG_METHOD_S_NULL_SENDER(error, L_ERROR)
GEN_LOG_METHOD_NULL_SENDER(warn, L_WARNING)
GEN_LOG_METHOD_F_NULL_SENDER(warn, L_WARNING)
GEN_LOG_METHOD_S_NULL_SENDER(warn, L_WARNING)
GEN_LOG_METHOD_NULL_SENDER(info, L_LEVEL_INFO)
GEN_LOG_METHOD_F_NULL_SENDER(info, L_LEVEL_INFO)
GEN_LOG_METHOD_S_NULL_SENDER(info, L_LEVEL_INFO)
GEN_LOG_METHOD_NULL_SENDER(trace, L_TRACE)
GEN_LOG_METHOD_F_NULL_SENDER(trace, L_TRACE)
GEN_LOG_METHOD_S_NULL_SENDER(trace, L_TRACE)
GEN_LOG_METHOD_NULL_SENDER(verbose, L_VERBOSE)
GEN_LOG_METHOD_F_NULL_SENDER(verbose, L_VERBOSE)
GEN_LOG_METHOD_S_NULL_SENDER(verbose, L_VERBOSE)
#endif

// --- Ring Buffer ---
#ifndef DISABLE_LOGGING
void Logger::setOutput(Print* out) { 
    // This is now a bit weird. Maybe add a new PrintTarget?
    // For now, let's make it replace the first target if it's a PrintTarget.
    // Or just ignore it. Let's ignore it for now to avoid complexity.
}

void Logger::clear() {
    memset(_buf, 0, sizeof(_buf));
    _head   = 0;
    _filled = 0;
    _idx    = 0;
}

const char* Logger::getLine(size_t i) const {
    if (i >= lines()) return nullptr;
    size_t index = (_head + LOG_BUFFER_LINES - i - 1U) % LOG_BUFFER_LINES;
    return _buf[index];
}

size_t Logger::lines() const { return (_filled < LOG_BUFFER_LINES) ? _filled : LOG_BUFFER_LINES; }

void Logger::attachToEspLog()
{
    s_esp_log_instance = this;
    esp_log_set_vprintf(&vprintfShim);
}

size_t Logger::write(uint8_t c) {
    size_t res = writeByte(static_cast<char>(c));
    return res;
}

size_t Logger::write(const uint8_t* data, size_t size) {
    size_t i = 0U;
    while (i < size) {
        if (static_cast<char>(data[i]) == '\n') {
            commitLine();
            ++i; // skip newline
            continue;
        }
        appendChar(static_cast<char>(data[i++]));
    }
    return size;
}

void Logger::appendChar(char c) {
    if (_idx < LOG_BUFFER_LINE_LENGTH - 1U) {
        _line[_idx++] = c;
    }
}

void Logger::commitLine() {
    _line[_idx] = '\0';
    _idx = 0;

    int prefixLen = 0;
    LogLevel level = parseLevelFromPrefix(_line, prefixLen);

    // The rest of the line is the message
    char* message = _line + prefixLen;

    // We are inside write(), which is already in a critical section
    dispatch(level, message);
}

size_t Logger::writeByte(char c) {
    if (c == '\r') return 1U; // ignore CR
    if (c == '\n') {
        commitLine();
    } else {
        appendChar(c);
    }
    return 1U;
}

int Logger::vprintfShim(const char* fmt, va_list args) {
    if (!s_esp_log_instance) return 0;

    char tmp[LOG_BUFFER_LINE_LENGTH];
    int len = vsnprintf(tmp, sizeof(tmp), fmt, args);
    if (len < 0) return len;
    
    for (int i = 0; i < len; ++i) {
        s_esp_log_instance->write(static_cast<uint8_t>(tmp[i]));
    }
    return len;
}
#endif

// --- New private methods ---
#ifndef DISABLE_LOGGING
void Logger::dispatch(LogLevel level, const char* message) {
    dispatch(nullptr, level, message);
}

void Logger::dispatch(const Component* sender, LogLevel level, const char* message) {
    if (level > _level || level == L_SILENT) return;

    char final_buffer[LOG_BUFFER_LINE_LENGTH];
    const char* level_str = "";
    switch(level) {
        case L_FATAL:   level_str = "FATAL: ";   break;
        case L_ERROR:   level_str = "ERROR: ";   break;
        case L_WARNING: level_str = "WARNING: "; break;
        case L_LEVEL_INFO:    level_str = "INFO: ";    break;
        case L_TRACE:   level_str = "TRACE: ";   break;
        case L_VERBOSE: level_str = "VERBOSE: "; break;
        default: break;
    }

    if (sender) {
        snprintf(final_buffer, LOG_BUFFER_LINE_LENGTH, "%s[%s:%d] %s", level_str, sender->name.c_str(), sender->id, message);
    } else {
        snprintf(final_buffer, LOG_BUFFER_LINE_LENGTH, "%s%s", level_str, message);
    }

    // Serial.println(final_buffer);

    // Store prefixed message in ring buffer
    strncpy(_buf[_head], final_buffer, LOG_BUFFER_LINE_LENGTH);
    _head = (_head + 1U) % LOG_BUFFER_LINES;
    if (_filled < LOG_BUFFER_LINES) ++_filled;

    // Rate limit target logging
    if (millis() - _lastLogTime < _logInterval) {
        // return;
    }
    _lastLogTime = millis();

    // Dispatch raw message to targets
    for (uint8_t i = 0; i < _numTargets; i++) {
        ILogTarget* target = _targets[i];
        if (target && level <= target->getLevel()) {
            if (TEST(_targetMask, target->getType())) {
                target->log(sender, level, message);
            }
        }
    }
}

LogLevel Logger::parseLevelFromPrefix(const char* message, int& prefixLen) {
    prefixLen = 0;
    if (strstr(message, "FATAL: ") == message) { prefixLen = 7; return L_FATAL; }
    if (strstr(message, "ERROR: ") == message) { prefixLen = 7; return L_ERROR; }
    if (strstr(message, "WARNING: ") == message) { prefixLen = 9; return L_WARNING; }
    if (strstr(message, "INFO: ") == message) { prefixLen = 6; return L_LEVEL_INFO; }
    if (strstr(message, "TRACE: ") == message) { prefixLen = 7; return L_TRACE; }
    if (strstr(message, "VERBOSE: ") == message) { prefixLen = 9; return L_VERBOSE; }
    
    return L_LEVEL_INFO; // Default for non-prefixed messages
}
#endif
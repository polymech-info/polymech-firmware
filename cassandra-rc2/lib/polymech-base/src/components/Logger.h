#ifndef LOGGER_COMPONENT_H
#define LOGGER_COMPONENT_H

#include <Arduino.h>
#include <Component.h>
#include <ArduinoLog.h>
#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>
#include <Print.h>
#include <esp_log.h>
#include <macros.h>

enum LogLevel {
    L_SILENT = 0,
    L_FATAL,
    L_ERROR,
    L_WARNING,
    L_LEVEL_INFO,
    L_TRACE,
    L_VERBOSE
};

enum LogTargetType {
    TARGET_TYPE_PRINT = 0,
    TARGET_TYPE_WEBSOCKET,
    TARGET_TYPE_FILE
};

enum LogTargetMask {
    TARGET_MASK_PRINT     = __BV(TARGET_TYPE_PRINT),
    TARGET_MASK_WEBSOCKET = __BV(TARGET_TYPE_WEBSOCKET),
    TARGET_MASK_FILE      = __BV(TARGET_TYPE_FILE),
    TARGET_MASK_ALL       = TARGET_MASK_PRINT | TARGET_MASK_WEBSOCKET | TARGET_MASK_FILE
};

enum class E_Logger_MB_Offset : ushort {
    MB_LOG_LEVEL = 0,
    COIL_TARGET_PRINT,
    COIL_TARGET_WEBSOCKET,
    COIL_TARGET_FILE,
    COUNT
};

class ILogTarget {
public:
    virtual ~ILogTarget() = default;
    virtual void log(const Component* sender, LogLevel level, const char* message) = 0;
    virtual void setLevel(LogLevel level) { _level = level; }
    virtual LogLevel getLevel() const { return _level; }
    virtual LogTargetType getType() const = 0;
protected:
    LogLevel _level = L_VERBOSE;
};

class RESTServer; // Forward declaration
class WebSocketTarget; // Forward declaration
class FileTarget; // Forward declaration

// -----------------------------------------------------------------------------
// Configuration macros ---------------------------------------------------------
// -----------------------------------------------------------------------------
#ifndef LOG_BUFFER_LINES
#define LOG_BUFFER_LINES 64
#endif

#ifndef LOG_BUFFER_LINE_LENGTH
#define LOG_BUFFER_LINE_LENGTH 128
#endif

#ifndef LOG_BUFFER_THREAD_SAFE
// #define LOG_BUFFER_THREAD_SAFE 1
#endif

using LogRingBuffer = char[LOG_BUFFER_LINES][LOG_BUFFER_LINE_LENGTH];

class Logger : public Component, public Print
{
public:
    Logger(Component *owner, ushort componentId, ushort modbusAddress);
    ~Logger();

    // --- Component Overrides (Optional) ---
    short setup() override;
    short loop() override;
    
    // --- Modbus Overrides ---
    void mb_tcp_register(ModbusTCP *manager) override;
    ModbusBlockView *mb_tcp_blocks() const override;
    short mb_tcp_read(MB_Registers *reg) override;
    short mb_tcp_write(MB_Registers *reg, short value) override;
    uint16_t mb_tcp_base_address() const override;

    // --- Logging ---
#ifndef DISABLE_LOGGING
    void setLevel(LogLevel level);
    void setTargetMask(uint8_t mask);
    uint8_t getTargetMask() const;
    bool addTarget(ILogTarget* target);
    bool addPrintTarget(Print* output);
    bool addWebSocketTarget(RESTServer* server);
    bool addFileTarget();
    void log(const Component* sender, LogLevel level, const char* format, ...);
    void log(const Component* sender, LogLevel level, const __FlashStringHelper* format, ...);
    void fatal(const Component* sender, const char* format, ...);
    void fatal(const Component* sender, const __FlashStringHelper* format, ...);
    void error(const Component* sender, const char* format, ...);
    void error(const Component* sender, const __FlashStringHelper* format, ...);
    void warn(const Component* sender, const char* format, ...);
    void warn(const Component* sender, const __FlashStringHelper* format, ...);
    void info(const Component* sender, const char* format, ...);
    void info(const Component* sender, const __FlashStringHelper* format, ...);
    void trace(const Component* sender, const char* format, ...);
    void trace(const Component* sender, const __FlashStringHelper* format, ...);
    void verbose(const Component* sender, const char* format, ...);
    void verbose(const Component* sender, const __FlashStringHelper* format, ...);

    // Overloads for String
    void log(const Component* sender, LogLevel level, const String& format, ...);
    void fatal(const Component* sender, const String& format, ...);
    void error(const Component* sender, const String& format, ...);
    void warn(const Component* sender, const String& format, ...);
    void info(const Component* sender, const String& format, ...);
    void trace(const Component* sender, const String& format, ...);
    void verbose(const Component* sender, const String& format, ...);

    // Overloads without sender context
    void fatal(const char* format, ...);
    void fatal(const __FlashStringHelper* format, ...);
    void error(const char* format, ...);
    void error(const __FlashStringHelper* format, ...);
    void warn(const char* format, ...);
    void warn(const __FlashStringHelper* format, ...);
    void info(const char* format, ...);
    void info(const __FlashStringHelper* format, ...);
    void trace(const char* format, ...);
    void trace(const __FlashStringHelper* format, ...);
    void verbose(const char* format, ...);
    void verbose(const __FlashStringHelper* format, ...);

    // Overloads without sender context for String
    void fatal(const String& format, ...);
    void error(const String& format, ...);
    void warn(const String& format, ...);
    void info(const String& format, ...);
    void trace(const String& format, ...);
    void verbose(const String& format, ...);

    // --- Ring Buffer ---
    void setOutput(Print* out);
    void clear();
    const char* getLine(size_t i) const;
    size_t lines() const;
    void attachToEspLog();
    size_t write(uint8_t c) override;
    size_t write(const uint8_t* data, size_t size) override;
#else
    // Stub implementations when logging is disabled
    inline void setLevel(LogLevel level) {}
    inline void setTargetMask(uint8_t mask) {}
    inline uint8_t getTargetMask() const { return 0; }
    inline bool addTarget(ILogTarget* target) { return false; }
    inline bool addPrintTarget(Print* output) { return false; }
    inline bool addWebSocketTarget(RESTServer* server) { return false; }
    inline bool addFileTarget() { return false; }
    inline void log(const Component* sender, LogLevel level, const char* format, ...) {}
    inline void log(const Component* sender, LogLevel level, const __FlashStringHelper* format, ...) {}
    inline void fatal(const Component* sender, const char* format, ...) {}
    inline void fatal(const Component* sender, const __FlashStringHelper* format, ...) {}
    inline void error(const Component* sender, const char* format, ...) {}
    inline void error(const Component* sender, const __FlashStringHelper* format, ...) {}
    inline void warn(const Component* sender, const char* format, ...) {}
    inline void warn(const Component* sender, const __FlashStringHelper* format, ...) {}
    inline void info(const Component* sender, const char* format, ...) {}
    inline void info(const Component* sender, const __FlashStringHelper* format, ...) {}
    inline void trace(const Component* sender, const char* format, ...) {}
    inline void trace(const Component* sender, const __FlashStringHelper* format, ...) {}
    inline void verbose(const Component* sender, const char* format, ...) {}
    inline void verbose(const Component* sender, const __FlashStringHelper* format, ...) {}
    inline void log(const Component* sender, LogLevel level, const String& format, ...) {}
    inline void fatal(const Component* sender, const String& format, ...) {}
    inline void error(const Component* sender, const String& format, ...) {}
    inline void warn(const Component* sender, const String& format, ...) {}
    inline void info(const Component* sender, const String& format, ...) {}
    inline void trace(const Component* sender, const String& format, ...) {}
    inline void verbose(const Component* sender, const String& format, ...) {}
    inline void fatal(const char* format, ...) {}
    inline void fatal(const __FlashStringHelper* format, ...) {}
    inline void error(const char* format, ...) {}
    inline void error(const __FlashStringHelper* format, ...) {}
    inline void warn(const char* format, ...) {}
    inline void warn(const __FlashStringHelper* format, ...) {}
    inline void info(const char* format, ...) {}
    inline void info(const __FlashStringHelper* format, ...) {}
    inline void trace(const char* format, ...) {}
    inline void trace(const __FlashStringHelper* format, ...) {}
    inline void verbose(const char* format, ...) {}
    inline void verbose(const __FlashStringHelper* format, ...) {}
    inline void fatal(const String& format, ...) {}
    inline void error(const String& format, ...) {}
    inline void warn(const String& format, ...) {}
    inline void info(const String& format, ...) {}
    inline void trace(const String& format, ...) {}
    inline void verbose(const String& format, ...) {}
    inline void setOutput(Print* out) {}
    inline void clear() {}
    inline const char* getLine(size_t i) const { return nullptr; }
    inline size_t lines() const { return 0; }
    inline void attachToEspLog() {}
    inline size_t write(uint8_t c) override { return 1; }
    inline size_t write(const uint8_t* data, size_t size) override { return size; }
#endif

private:
#ifndef DISABLE_LOGGING
    // --- Ring Buffer ---
    void appendChar(char c);
    void commitLine();
    size_t writeByte(char c);

    static int vprintfShim(const char* fmt, va_list args);
    static Logger* s_esp_log_instance;

    // --- New logging system ---
    void dispatch(const Component* sender, LogLevel level, const char* message);
    void dispatch(LogLevel level, const char* message);
    LogLevel parseLevelFromPrefix(const char* message, int& prefixLen);

    const unsigned long _logInterval = 50; // Rate limit interval in ms
    unsigned long _lastLogTime = 0;

    uint8_t _targetMask = TARGET_MASK_ALL;
    LogLevel _level = L_VERBOSE;
    static const uint8_t MAX_TARGETS = 3;
    ILogTarget* _targets[MAX_TARGETS]{};
    uint8_t _numTargets = 0;

    LogRingBuffer _buf{};
    size_t        _head  = 0U;
    size_t        _filled = 0U;
    char          _line[LOG_BUFFER_LINE_LENGTH];
    size_t        _idx   = 0U;
#if LOG_BUFFER_THREAD_SAFE
    portMUX_TYPE  _mux   = portMUX_INITIALIZER_UNLOCKED;
#endif

    WebSocketTarget* _webSocketTarget = nullptr;
    FileTarget* _fileTarget = nullptr;
#else
    // Minimal state when logging is disabled
    uint8_t _targetMask = 0;
    LogLevel _level = L_SILENT;
#endif

    const short modbusAddress;
    MB_Registers m_modbus_block[(ushort)E_Logger_MB_Offset::COUNT];
    mutable ModbusBlockView m_modbus_view;
    ModbusTCP *modbusTCP;
};

#endif
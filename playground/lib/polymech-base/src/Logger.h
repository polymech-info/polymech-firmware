#ifndef CIRCULAR_LOG_PRINTER_H
#define CIRCULAR_LOG_PRINTER_H

#include <Arduino.h>
#include <Print.h>
#include <esp_log.h>

// -----------------------------------------------------------------------------
// Configuration macros ---------------------------------------------------------
//   • LOG_BUFFER_LINES        – number of lines kept in RAM (default 100)
//   • LOG_BUFFER_LINE_LENGTH  – max chars per line incl. null (default 120)
//   • LOG_BUFFER_THREAD_SAFE  – 1 = wrap writes in a FreeRTOS critical section
// -----------------------------------------------------------------------------
#ifndef LOG_BUFFER_LINES
#define LOG_BUFFER_LINES 100
#endif

#ifndef LOG_BUFFER_LINE_LENGTH
#define LOG_BUFFER_LINE_LENGTH 120
#endif

#ifndef LOG_BUFFER_THREAD_SAFE
#define LOG_BUFFER_THREAD_SAFE 1
#endif

using LogRingBuffer = char[LOG_BUFFER_LINES][LOG_BUFFER_LINE_LENGTH];

// -----------------------------------------------------------------------------
// CircularLogPrinter -----------------------------------------------------------
// Stores the last N lines in a ring buffer and optionally mirrors everything to
// an arbitrary Print stream. 100 % reinterpret‑cast‑free for MISRA/CPPCHECK
// compliance: the code never converts between pointer types, only between
// scalar values.
// -----------------------------------------------------------------------------
class CircularLogPrinter final : public Print {
public:
    explicit CircularLogPrinter(Print* out = &Serial) : _out(out) { clear(); }

    void setOutput(Print* out) { _out = out; }

    void clear() {
        memset(_buf, 0, sizeof(_buf));
        _head   = 0;
        _filled = 0;
        _idx    = 0;
    }

    const char* getLine(size_t i) const {
        if (i >= lines()) return nullptr;
        size_t index = (_head + LOG_BUFFER_LINES - i - 1U) % LOG_BUFFER_LINES;
        return _buf[index];
    }

    size_t lines() const { return (_filled < LOG_BUFFER_LINES) ? _filled : LOG_BUFFER_LINES; }

    // Redirect ESP‑IDF logging macros (ESP_LOGx) into this printer
    void attachToEspLog() { esp_log_set_vprintf(&vprintfShim); }

    // ---------------------------------------------------------------------
    // Print single byte
    // ---------------------------------------------------------------------
    size_t write(uint8_t c) override {
#if LOG_BUFFER_THREAD_SAFE
        portENTER_CRITICAL(&_mux);
#endif
        size_t res = writeByte(static_cast<char>(c));
#if LOG_BUFFER_THREAD_SAFE
        portEXIT_CRITICAL(&_mux);
#endif
        return res;
    }

    // ---------------------------------------------------------------------
    // Faster multi‑byte write – no pointer casting
    // ---------------------------------------------------------------------
    size_t write(const uint8_t* data, size_t size) override {
#if LOG_BUFFER_THREAD_SAFE
        portENTER_CRITICAL(&_mux);
#endif
        size_t i = 0U;
        while (i < size) {
            if (static_cast<char>(data[i]) == '\n') {
                commitLine();
                ++i; // skip newline
                continue;
            }
            appendChar(static_cast<char>(data[i++]));
        }
#if LOG_BUFFER_THREAD_SAFE
        portEXIT_CRITICAL(&_mux);
#endif
        return size;
    }

private:
    // ---------------------------------------------------------------------
    // ESP‑IDF vprintf shim – feeds characters individually (no cast needed)
    // ---------------------------------------------------------------------
    static int vprintfShim(const char* fmt, va_list args) {
        char tmp[LOG_BUFFER_LINE_LENGTH];
        int len = vsnprintf(tmp, sizeof(tmp), fmt, args);
        if (len < 0) return len;
        auto& logger = instance();
        for (int i = 0; i < len; ++i) {
            logger.write(static_cast<uint8_t>(tmp[i]));
        }
        return len;
    }

    static CircularLogPrinter& instance() {
        static CircularLogPrinter logger;
        return logger;
    }

    void appendChar(char c) {
        if (_idx < LOG_BUFFER_LINE_LENGTH - 1U) {
            _line[_idx++] = c;
        }
    }

    void commitLine() {
        _line[_idx] = '\0';
        if (_out) _out->println(_line);
        strncpy(_buf[_head], _line, LOG_BUFFER_LINE_LENGTH);
        _head = (_head + 1U) % LOG_BUFFER_LINES;
        if (_filled < LOG_BUFFER_LINES) ++_filled;
        _idx = 0U;
    }

    size_t writeByte(char c) {
        if (c == '\r') return 1U; // ignore CR
        if (c == '\n') {
            commitLine();
        } else {
            appendChar(c);
        }
        return 1U;
    }

    // ---------------------------------------------------------------------
    // Members
    // ---------------------------------------------------------------------
    LogRingBuffer _buf{};
    Print*        _out   = nullptr;
    size_t        _head  = 0U;
    size_t        _filled = 0U;
    char          _line[LOG_BUFFER_LINE_LENGTH];
    size_t        _idx   = 0U;
#if LOG_BUFFER_THREAD_SAFE
    portMUX_TYPE  _mux   = portMUX_INITIALIZER_UNLOCKED;
#endif
};

#endif  // CIRCULAR_LOG_PRINTER_H

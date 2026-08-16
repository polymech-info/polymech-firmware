#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <Arduino.h>

#ifndef MESSAGE_QUEUE_CAPACITY
#define MESSAGE_QUEUE_CAPACITY 5
#endif

#ifndef MESSAGE_MAX_LENGTH
#define MESSAGE_MAX_LENGTH 128 
#endif

#ifndef MESSAGE_QUEUE_THREAD_SAFE
#define MESSAGE_QUEUE_THREAD_SAFE 1
#endif

using MessageBuffer = char[MESSAGE_QUEUE_CAPACITY][MESSAGE_MAX_LENGTH];

class MessageQueue final {
public:
    MessageQueue() { clear(); }

    void clear() {
        memset(_buf, 0, sizeof(_buf));
        _head   = 0;
        _filled = 0;
    }

    void addMessage(const char* message) {
#if MESSAGE_QUEUE_THREAD_SAFE
        portENTER_CRITICAL(&_mux);
#endif
        strncpy(_buf[_head], message, MESSAGE_MAX_LENGTH - 1);
        _buf[_head][MESSAGE_MAX_LENGTH - 1] = '\0'; // Ensure null termination
        _head = (_head + 1) % MESSAGE_QUEUE_CAPACITY;
        if (_filled < MESSAGE_QUEUE_CAPACITY) {
            _filled++;
        }
#if MESSAGE_QUEUE_THREAD_SAFE
        portEXIT_CRITICAL(&_mux);
#endif
    }

    const char* getLine(size_t i) const {
        if (i >= lines()) return nullptr;
        size_t index = (_head + MESSAGE_QUEUE_CAPACITY - i - 1) % MESSAGE_QUEUE_CAPACITY;
        return _buf[index];
    }

    size_t lines() const { 
        return (_filled < MESSAGE_QUEUE_CAPACITY) ? _filled : MESSAGE_QUEUE_CAPACITY;
    }

private:
    MessageBuffer _buf{};
    size_t        _head  = 0U;
    size_t        _filled = 0U;
#if MESSAGE_QUEUE_THREAD_SAFE
    portMUX_TYPE  _mux   = portMUX_INITIALIZER_UNLOCKED;
#endif
};

#endif  // MESSAGE_QUEUE_H 
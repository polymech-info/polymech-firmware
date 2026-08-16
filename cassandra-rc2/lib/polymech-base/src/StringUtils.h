#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "./xtypes.h"

// stricmp() is not available in Visual Studio 2013 and earlier
# if defined(_MSC_VER)
#define strtok_r strtok_s
# ifndef _CRT_SECURE_NO_DEPRECATE
# define _CRT_SECURE_NO_DEPRECATE (1)
# endif
# pragma warning(disable : 4996)
# endif

typedef enum E_VALUE_TYPE {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_UNKNOWN
} E_VALUE_TYPE;


// Function to convert a string to a native type
template<typename T> T convertTo(cchar* str);
// Specialization for int
template<> int convertTo<int>(cchar* str);
// Specialization for long int
template<> long int convertTo<long int>(cchar* str);
// Specialization for float
template<> float convertTo<float>(cchar* str);
// Specialization for bool
template<> bool convertTo<bool>(cchar* str);

// Function to detect if a string is a valid integer
bool isInteger(cchar* str);
// Function to detect if a string is a valid float
bool isFloat(cchar* str);
// Function to detect if a string is a valid bool
bool isBool(cchar* str);

E_VALUE_TYPE detectType(cchar* str);

void printHex(uint8_t *data, uint8_t length);

template<size_t SIZE>
class XString {
private:
    char m_buffer[SIZE];

public:
    XString() {
        m_buffer[0] = '\0';
    }

    XString(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(m_buffer, SIZE, fmt, args);
        va_end(args);
    }

    void format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(m_buffer, SIZE, fmt, args);
        va_end(args);
    }
    
    void format(const __FlashStringHelper* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vsnprintf_P(m_buffer, SIZE, (const char*)fmt, args);
        va_end(args);
    }

    const char* c_str() const {
        return m_buffer;
    }

    operator const char*() const {
        return m_buffer;
    }
};


// Inline attribute macro for readability
#define ALWAYS_INLINE __attribute__((always_inline)) inline

// Format from RAM (normal const char* format)
void formatToBuffer(char* buffer, size_t size, const char* fmt, ...);

// Format from PROGMEM (Flash string)
void formatToBuffer(char* buffer, size_t size, const __FlashStringHelper* fmtF, ...);

// --- Macros for convenience ---

// Format from RAM format string
#define FORMAT_BUFFER(buf, fmt, ...) \
    formatToBuffer(buf, sizeof(buf), fmt, ##__VA_ARGS__)

// Format from Flash (PROGMEM) format string
#define FORMAT_BUFFER_F(buf, fmt, ...) \
    formatToBuffer(buf, sizeof(buf), F(fmt), ##__VA_ARGS__)


#endif

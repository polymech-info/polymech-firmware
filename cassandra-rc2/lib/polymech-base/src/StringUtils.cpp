#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "StringUtils.h"
#include "xtypes.h"


// Specialization for int
template<> int convertTo<int>(cchar* str) {
    return atoi(str);
}
template<> short convertTo<short>(cchar* str) {
    return atoi(str);
}

// Specialization for long int
template<> long int convertTo<long int>(cchar* str) {
    return strtol(str, nullptr, 10);
}

// Specialization for long long int
/*
template<> long long int convertTo<long long int>(cchar* str) {
    return strtoll(str, nullptr, 10);
}
*/

// Specialization for float
template<> float convertTo<float>(cchar* str) {
    return (float)atof(str);
}

// Specialization for bool
template<> bool convertTo<bool>(cchar* str) {
    return (strcmp(str, "true") == 0 || strcmp(str, "1") == 0 || strcmp(str, "on") == 0);
}

// Specializations for int, long long int, float, bool, and possibly other types...

// Function to detect if a string is a valid bool
bool isBool(cchar* str) {
    if (!str) return false;
    return (strcmp(str, "true") == 0 || strcmp(str, "false") == 0 ||
            strcmp(str, "1") == 0 || strcmp(str, "0") == 0 ||
            strcmp(str, "on") == 0 || strcmp(str, "off") == 0);
}

// Function to detect if a string is a valid integer
bool isInteger(cchar* str) {
    if (!str || *str == '\0') return false;
    char* end;
    strtol(str, &end, 10);
    return *end == '\0';
}

// Function to detect if a string is a valid float
bool isFloat(cchar* str) {
    if (!str || *str == '\0') return false;
    char* end;
    strtof(str, &end);
    return *end == '\0' && strchr(str, '.') != nullptr;
}

E_VALUE_TYPE detectType(cchar* str) {
    if (isInteger(str)) return TYPE_INT;
    if (isFloat(str)) return TYPE_FLOAT;
    if (isBool(str)) return TYPE_BOOL;
    return TYPE_UNKNOWN;
}


void printHex(uint8_t *data, uint8_t length)
{
    
}

void formatToBuffer(char* buffer, size_t size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);

    if (strlen(buffer) >= size - 1) {
        Serial.println(F("[WARN] formatToBuffer(): string truncated"));
    }
}

void formatToBuffer(char* buffer, size_t size, const __FlashStringHelper* fmtF, ...) {
    va_list args;
    va_start(args, fmtF);
    vsnprintf_P(buffer, size, (const char*)fmtF, args);
    va_end(args);
}

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
    return (strcmp(str, "true") == 0 || strcmp(str, "false") == 0 || strcmp(str, "1") == 0 || strcmp(str, "0") == 0 || strcmp(str, "on") == 0 || strcmp(str, "off") == 0);
}

// Specializations for int, long long int, float, bool, and possibly other types...

// Function to detect if a string is a valid integer
bool isInteger(cchar* str) {
    if (*str == '-' || *str == '+') str++;  // Skip sign
    while (*str) {
        if (!isdigit(*str)) return false; // Check if all characters are digits
        str++;
    }
    return true;
}

// Function to detect if a string is a valid float
bool isFloat(cchar* str) {
    bool hasPeriod = false;
    if (*str == '-' || *str == '+') str++;  // Skip sign
    while (*str) {
        if (*str == '.') {
            if (hasPeriod) return false; // Only one period allowed
            hasPeriod = true;
        } else if (!isdigit(*str)) {
            return false; // All other characters must be digits
        }
        str++;
    }
    return true;
}

E_VALUE_TYPE detectType(cchar* str) {
    if (isInteger(str)) {
        return TYPE_INT;
    } else if (isFloat(str)) {
        return TYPE_FLOAT;
    }else if (*str != '\0') { // Non-empty string considered as TYPE_STRING
        return TYPE_STRING;
    }
    return TYPE_UNKNOWN;
}


void printHex(uint8_t *data, uint8_t length)
{
    
}

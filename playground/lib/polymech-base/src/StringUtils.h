#ifndef STRINGUTILS_H
#define STRINGUTILS_H

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

E_VALUE_TYPE detectType(cchar* str);

void printHex(uint8_t *data, uint8_t length);

#endif

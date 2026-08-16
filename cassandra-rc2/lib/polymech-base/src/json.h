#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <functional>

namespace JsonUtils {

/**
 * @brief Handles and logs JSON deserialization errors in a generic way.
 * 
 * @param error The DeserializationError object from ArduinoJson.
 * @param context A string describing the context of the operation (e.g., function name).
 * @param sourceName A name for the JSON source (e.g., filename).
 * @param sampleProvider A lambda function that returns a String sample of the JSON source for logging. Can be nullptr.
 * @param sourceSize The size of the JSON source, if known.
 * @return true if an error occurred, false otherwise.
 */
inline bool handleDeserializationError(
    const DeserializationError& error, 
    const char* context, 
    const char* sourceName, 
    std::function<String()> sampleProvider = nullptr,
    size_t sourceSize = 0) 
{
    if (error) {
        Log.errorln(F("[%s] Failed to parse JSON from '%s'. Error: %s"), context, sourceName, error.c_str());
        if (sourceSize > 0) {
            Log.errorln(F("  Source size: %d bytes."), sourceSize);
        }
        if (sampleProvider) {
            String sample = sampleProvider();
            Log.errorln(F("  Source content sample: %s"), sample.c_str());
        }
        return true; // Error occurred
    }
    return false; // No error
}

inline void parseJsonFieldUint32(const JsonObject& json, const char* key, uint32_t& targetValue, const char* fieldName, const char* componentName) {
    JsonVariantConst value = json[key];
    if (value.is<uint32_t>()) {
        targetValue = value.as<uint32_t>();
    } else {
        if (!value.isNull()) {
            Log.traceln(F("[%s] WARN: '%s' in JSON is not uint32_t. Using default: %lu"), 
                componentName, fieldName, targetValue);
        }
    }
}

inline void parseJsonFieldUint8(const JsonObject& json, const char* key, uint8_t& targetValue, const char* fieldName, const char* componentName) {
    JsonVariantConst value = json[key];
    if (value.is<uint8_t>()) {
        targetValue = value.as<uint8_t>();
    } else {
        if (!value.isNull()) {
            Log.traceln(F("[%s] WARN: '%s' in JSON is not uint8_t. Using default: %u"), 
                componentName, fieldName, targetValue);
        }
    }
}

inline void parseJsonFieldBool(const JsonObject& json, const char* key, bool& targetValue, const char* fieldName, const char* componentName) {
    JsonVariantConst value = json[key];
    if (value.is<bool>()) {
        targetValue = value.as<bool>();
    } else {
        if (!value.isNull()) {
            Log.traceln(F("[%s] WARN: '%s' in JSON is not bool. Using default: %s"), 
                componentName, fieldName, targetValue ? "true" : "false");
        }
    }
}

} // namespace JsonUtils

#endif // JSON_UTILS_H 
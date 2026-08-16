#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <ArduinoJson.h>
#include <ArduinoLog.h>

namespace JsonUtils {

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
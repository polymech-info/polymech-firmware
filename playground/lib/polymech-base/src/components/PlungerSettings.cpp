#include "PlungerSettings.h"
#include "Plunger.h" 
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ArduinoLog.h> 

PlungerSettings::PlungerSettings() :
    speedSlowHz(PLUNGER_SPEED_SLOW_HZ),
    speedMediumHz(PLUNGER_SPEED_MEDIUM_HZ),
    speedFastHz(PLUNGER_SPEED_FAST_HZ),
    speedFillPlungeHz(PLUNGER_SPEED_FILL_PLUNGE_HZ),
    speedFillHomeHz(PLUNGER_SPEED_FILL_HOME_HZ),
    currentJamThresholdMa(PLUNGER_CURRENT_JAM_THRESHOLD_MA),
    jammedDurationHomingMs(PLUNGER_JAMMED_DURATION_HOMING_MS),
    jammedDurationMs(PLUNGER_JAMMED_DURATION_MS),
    autoModeHoldDurationMs(PLUNGER_AUTO_MODE_HOLD_DURATION_MS),
    maxUniversalJamTimeMs(PLUNGER_MAX_UNIVERSAL_JAM_TIME_MS),
    fillJoystickHoldDurationMs(PLUNGER_FILL_JOYSTICK_HOLD_DURATION_MS),
    fillPlungedWaitDurationMs(PLUNGER_FILL_PLUNGED_WAIT_DURATION_MS),
    fillHomedWaitDurationMs(PLUNGER_FILL_HOMED_WAIT_DURATION_MS),
    recordHoldDurationMs(PLUNGER_RECORD_HOLD_DURATION_MS),
    maxRecordDurationMs(PLUNGER_MAX_RECORD_DURATION_MS),
    replayDurationMs(PLUNGER_DEFAULT_REPLAY_DURATION_MS),
    enablePostFlow(PLUNGER_DEFAULT_ENABLE_POST_FLOW),
    postFlowDurationMs(PLUNGER_POST_FLOW_DURATION_MS),
    postFlowSpeedHz(PLUNGER_POST_FLOW_SPEED_HZ),
    currentPostFlowMa(PLUNGER_CURRENT_POST_FLOW_MA),
    postFlowStoppingWaitMs(PLUNGER_POST_FLOW_STOPPING_WAIT_MS),
    postFlowCompleteWaitMs(PLUNGER_POST_FLOW_COMPLETE_WAIT_MS),
    defaultMaxOperationDurationMs(PLUNGER_DEFAULT_MAX_OPERATION_DURATION_MS)
{
    
}

void PlungerSettings::toJson(JsonDocument& doc) const {
    JsonObject obj = doc.to<JsonObject>(); // Or doc.as<JsonObject>() if doc is already object type

    obj["speedSlowHz"] = speedSlowHz;
    obj["speedMediumHz"] = speedMediumHz;
    obj["speedFastHz"] = speedFastHz;
    obj["speedFillPlungeHz"] = speedFillPlungeHz;
    obj["speedFillHomeHz"] = speedFillHomeHz;
    obj["currentJamThresholdMa"] = currentJamThresholdMa;
    obj["jammedDurationHomingMs"] = jammedDurationHomingMs;
    obj["jammedDurationMs"] = jammedDurationMs;
    obj["autoModeHoldDurationMs"] = autoModeHoldDurationMs;
    obj["maxUniversalJamTimeMs"] = maxUniversalJamTimeMs;
    obj["fillJoystickHoldDurationMs"] = fillJoystickHoldDurationMs;
    obj["fillPlungedWaitDurationMs"] = fillPlungedWaitDurationMs;
    obj["fillHomedWaitDurationMs"] = fillHomedWaitDurationMs;
    obj["recordHoldDurationMs"] = recordHoldDurationMs;
    obj["maxRecordDurationMs"] = maxRecordDurationMs;
    obj["replayDurationMs"] = replayDurationMs;
    obj["enablePostFlow"] = enablePostFlow;
    obj["postFlowDurationMs"] = postFlowDurationMs;
    obj["postFlowSpeedHz"] = postFlowSpeedHz;
    obj["currentPostFlowMa"] = currentPostFlowMa;
    obj["postFlowStoppingWaitMs"] = postFlowStoppingWaitMs;
    obj["postFlowCompleteWaitMs"] = postFlowCompleteWaitMs;
    obj["defaultMaxOperationDurationMs"] = defaultMaxOperationDurationMs;
}

// --- Helper functions for fromJson --- 
// (Defined static or as private members if preferred; static here for simplicity)
static void _parseJsonField(const JsonObject& json, const char* key, uint16_t& targetValue, const char* fieldName) {
    JsonVariantConst value = json[key];
    if (value.is<uint16_t>()) {
        targetValue = value.as<uint16_t>();
    } else {
        if (!value.isNull()) { // Key exists, but has the wrong type
            Log.warningln("[PlungerSettings] WARN: '%s' in JSON is not uint16_t. Using default: %u", fieldName, targetValue);
        }
    }
}

static void _parseJsonField(const JsonObject& json, const char* key, uint32_t& targetValue, const char* fieldName) {
    JsonVariantConst value = json[key];
    if (value.is<uint32_t>()) {
        targetValue = value.as<uint32_t>();
    } else {
        if (!value.isNull()) { // Key exists, but has the wrong type
            Log.warningln("[PlungerSettings] WARN: '%s' in JSON is not uint32_t. Using default: %lu", fieldName, targetValue);
        }
    }
}

static void _parseJsonField(const JsonObject& json, const char* key, bool& targetValue, const char* fieldName) {
    JsonVariantConst value = json[key];
    if (value.is<bool>()) {
        targetValue = value.as<bool>();
    } else {
        if (!value.isNull()) { // Key exists, but has the wrong type
            Log.warningln("[PlungerSettings] WARN: '%s' in JSON is not bool. Using default: %s", fieldName, targetValue ? "true" : "false");
        }
    }
}

bool PlungerSettings::fromJson(const JsonObject& json) {
    if (json.isNull()) {
        Log.warningln("[PlungerSettings] fromJson: Provided JSON object is null. Using defaults.");
        return false; 
    }
    _parseJsonField(json, "speedSlowHz", speedSlowHz, "speedSlowHz");
    _parseJsonField(json, "speedMediumHz", speedMediumHz, "speedMediumHz");
    _parseJsonField(json, "speedFastHz", speedFastHz, "speedFastHz");
    _parseJsonField(json, "speedFillPlungeHz", speedFillPlungeHz, "speedFillPlungeHz");
    _parseJsonField(json, "speedFillHomeHz", speedFillHomeHz, "speedFillHomeHz");
    _parseJsonField(json, "currentJamThresholdMa", currentJamThresholdMa, "currentJamThresholdMa");
    _parseJsonField(json, "jammedDurationHomingMs", jammedDurationHomingMs, "jammedDurationHomingMs");
    _parseJsonField(json, "jammedDurationMs", jammedDurationMs, "jammedDurationMs");
    _parseJsonField(json, "autoModeHoldDurationMs", autoModeHoldDurationMs, "autoModeHoldDurationMs");
    _parseJsonField(json, "maxUniversalJamTimeMs", maxUniversalJamTimeMs, "maxUniversalJamTimeMs");
    _parseJsonField(json, "fillJoystickHoldDurationMs", fillJoystickHoldDurationMs, "fillJoystickHoldDurationMs");
    _parseJsonField(json, "fillPlungedWaitDurationMs", fillPlungedWaitDurationMs, "fillPlungedWaitDurationMs");
    _parseJsonField(json, "fillHomedWaitDurationMs", fillHomedWaitDurationMs, "fillHomedWaitDurationMs");
    _parseJsonField(json, "recordHoldDurationMs", recordHoldDurationMs, "recordHoldDurationMs");
    _parseJsonField(json, "maxRecordDurationMs", maxRecordDurationMs, "maxRecordDurationMs");
    _parseJsonField(json, "replayDurationMs", replayDurationMs, "replayDurationMs");
    _parseJsonField(json, "postFlowDurationMs", postFlowDurationMs, "postFlowDurationMs");
    _parseJsonField(json, "postFlowStoppingWaitMs", postFlowStoppingWaitMs, "postFlowStoppingWaitMs");
    _parseJsonField(json, "postFlowCompleteWaitMs", postFlowCompleteWaitMs, "postFlowCompleteWaitMs");
    _parseJsonField(json, "defaultMaxOperationDurationMs", defaultMaxOperationDurationMs, "defaultMaxOperationDurationMs");

    _parseJsonField(json, "enablePostFlow", enablePostFlow, "enablePostFlow");
    
    _parseJsonField(json, "postFlowSpeedHz", postFlowSpeedHz, "postFlowSpeedHz");
    _parseJsonField(json, "currentPostFlowMa", currentPostFlowMa, "currentPostFlowMa");
    
    Log.infoln("[PlungerSettings] Settings parsed from JSON (check warnings above for issues). Call print() to see final values.");
    return true;
}

bool PlungerSettings::load(const char* path) {
    if (!LittleFS.begin()) {
        Log.errorln("[PlungerSettings] Failed to initialize LittleFS for load.");
        return false;
    }

    File configFile = LittleFS.open(path, "r");
    if (!configFile) {
        Log.warningln("[PlungerSettings] Settings file not found: %s. Using current (default) settings.", path);
        // Optionally, save current (default) settings to create the file:
        // save(path); 
        return false; // Indicate that loading from file did not happen, defaults remain.
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        Log.errorln("[PlungerSettings] Failed to deserialize settings file %s: %s", path, error.c_str());
        return false;
    }
    
    Log.infoln("[PlungerSettings] Successfully deserialized %s.", path);
    return fromJson(doc.as<JsonObject>());
}

bool PlungerSettings::save(const char* path) const {
    if (!LittleFS.begin()) {
        Log.errorln("[PlungerSettings] Failed to initialize LittleFS for save.");
        return false;
    }

    JsonDocument doc;
    toJson(doc); // Populate the document with current settings

    File configFile = LittleFS.open(path, "w");
    if (!configFile) {
        Log.errorln("[PlungerSettings] Failed to open settings file for writing: %s", path);
        return false;
    }

    size_t bytesWritten = serializeJson(doc, configFile);
    configFile.close();

    if (bytesWritten == 0) {
        Log.errorln("[PlungerSettings] Failed to write settings to file: %s", path);
        return false;
    }

    Log.infoln("[PlungerSettings] Settings successfully saved to %s (%u bytes).", path, bytesWritten);
    return true;
}

void PlungerSettings::print() const {
    Log.infoln("--- PlungerSettings Values ---");
    Log.infoln("  speedSlowHz: %u", speedSlowHz);
    Log.infoln("  speedMediumHz: %u", speedMediumHz);
    Log.infoln("  speedFastHz: %u", speedFastHz);
    Log.infoln("  speedFillPlungeHz: %u", speedFillPlungeHz);
    Log.infoln("  speedFillHomeHz: %u", speedFillHomeHz);
    Log.infoln("  currentJamThresholdMa: %u", currentJamThresholdMa);
    Log.infoln("  jammedDurationHomingMs: %lu", jammedDurationHomingMs);
    Log.infoln("  jammedDurationMs: %lu", jammedDurationMs);
    Log.infoln("  autoModeHoldDurationMs: %lu", autoModeHoldDurationMs);
    Log.infoln("  maxUniversalJamTimeMs: %lu", maxUniversalJamTimeMs);
    Log.infoln("  fillJoystickHoldDurationMs: %lu", fillJoystickHoldDurationMs);
    Log.infoln("  fillPlungedWaitDurationMs: %lu", fillPlungedWaitDurationMs);
    Log.infoln("  fillHomedWaitDurationMs: %lu", fillHomedWaitDurationMs);
    Log.infoln("  recordHoldDurationMs: %lu", recordHoldDurationMs);
    Log.infoln("  maxRecordDurationMs: %lu", maxRecordDurationMs);
    Log.infoln("  replayDurationMs: %lu", replayDurationMs);
    Log.infoln("  enablePostFlow: %s", enablePostFlow ? "Yes" : "No");
    Log.infoln("  postFlowDurationMs: %lu", postFlowDurationMs);
    Log.infoln("  postFlowSpeedHz: %u", postFlowSpeedHz);
    Log.infoln("  currentPostFlowMa: %u", currentPostFlowMa);
    Log.infoln("  postFlowStoppingWaitMs: %lu", postFlowStoppingWaitMs);
    Log.infoln("  postFlowCompleteWaitMs: %lu", postFlowCompleteWaitMs);
    Log.infoln("  defaultMaxOperationDurationMs: %lu", defaultMaxOperationDurationMs);
    Log.infoln("--- End PlungerSettings Values ---");
} 
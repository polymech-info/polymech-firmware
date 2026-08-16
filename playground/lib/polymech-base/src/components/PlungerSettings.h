#ifndef PLUNGER_SETTINGS_H
#define PLUNGER_SETTINGS_H

#include <stdint.h> // For uint16_t, uint32_t
#include <Arduino.h>  // For bool type
#include <ArduinoJson.h> // For JSON operations
#include <LittleFS.h>    // For file operations

// No include "Plunger.h" here to avoid circular dependencies for this file's primary role.
// The .cpp file for PlungerSettings will include Plunger.h for constants if needed for default constructor.

struct PlungerSettings {
    uint16_t speedSlowHz;
    uint16_t speedMediumHz;
    uint16_t speedFastHz;
    uint16_t speedFillPlungeHz;
    uint16_t speedFillHomeHz;

    uint16_t currentJamThresholdMa;
    
    uint32_t jammedDurationHomingMs;
    uint32_t jammedDurationMs;
    uint32_t autoModeHoldDurationMs;
    uint32_t maxUniversalJamTimeMs;

    uint32_t fillJoystickHoldDurationMs;
    uint32_t fillPlungedWaitDurationMs;
    uint32_t fillHomedWaitDurationMs;

    uint32_t recordHoldDurationMs;
    uint32_t maxRecordDurationMs;
    uint32_t replayDurationMs;

    bool enablePostFlow;
    uint32_t postFlowDurationMs;
    uint16_t postFlowSpeedHz;
    uint16_t currentPostFlowMa;
    uint32_t postFlowStoppingWaitMs;
    uint32_t postFlowCompleteWaitMs;

    uint32_t defaultMaxOperationDurationMs;

    // Default constructor - Declaration only, defined in .cpp
    PlungerSettings();

    // Constructor to initialize with default values from Plunger.h constants
    PlungerSettings(
        uint16_t defSpeedSlowHz,
        uint16_t defSpeedMediumHz,
        uint16_t defSpeedFastHz,
        uint16_t defSpeedFillPlungeHz,
        uint16_t defSpeedFillHomeHz,
        uint16_t defCurrentJamThresholdMa,
        uint32_t defJammedDurationHomingMs,
        uint32_t defJammedDurationMs,
        uint32_t defAutoModeHoldDurationMs,
        uint32_t defMaxUniversalJamTimeMs,
        uint32_t defFillJoystickHoldDurationMs,
        uint32_t defFillPlungedWaitDurationMs,
        uint32_t defFillHomedWaitDurationMs,
        uint32_t defRecordHoldDurationMs,
        uint32_t defMaxRecordDurationMs,
        uint32_t defReplayDurationMs,
        bool defEnablePostFlow,
        uint32_t defPostFlowDurationMs,
        uint16_t defPostFlowSpeedHz,
        uint16_t defCurrentPostFlowMa,
        uint32_t defPostFlowStoppingWaitMs,
        uint32_t defPostFlowCompleteWaitMs,
        uint32_t defDefaultMaxOperationDurationMs
    ) :
        speedSlowHz(defSpeedSlowHz),
        speedMediumHz(defSpeedMediumHz),
        speedFastHz(defSpeedFastHz),
        speedFillPlungeHz(defSpeedFillPlungeHz),
        speedFillHomeHz(defSpeedFillHomeHz),
        currentJamThresholdMa(defCurrentJamThresholdMa),
        jammedDurationHomingMs(defJammedDurationHomingMs),
        jammedDurationMs(defJammedDurationMs),
        autoModeHoldDurationMs(defAutoModeHoldDurationMs),
        maxUniversalJamTimeMs(defMaxUniversalJamTimeMs),
        fillJoystickHoldDurationMs(defFillJoystickHoldDurationMs),
        fillPlungedWaitDurationMs(defFillPlungedWaitDurationMs),
        fillHomedWaitDurationMs(defFillHomedWaitDurationMs),
        recordHoldDurationMs(defRecordHoldDurationMs),
        maxRecordDurationMs(defMaxRecordDurationMs),
        replayDurationMs(defReplayDurationMs),
        enablePostFlow(defEnablePostFlow),
        postFlowDurationMs(defPostFlowDurationMs),
        postFlowSpeedHz(defPostFlowSpeedHz),
        currentPostFlowMa(defCurrentPostFlowMa),
        postFlowStoppingWaitMs(defPostFlowStoppingWaitMs),
        postFlowCompleteWaitMs(defPostFlowCompleteWaitMs),
        defaultMaxOperationDurationMs(defDefaultMaxOperationDurationMs)
    {}

    // Persistence methods
    void toJson(JsonDocument& doc) const;
    bool fromJson(const JsonObject& json);
    bool load(const char* path = "/plunger.json");
    bool save(const char* path = "/plunger.json") const;

    // Debug method
    void print() const;
};

#endif // PLUNGER_SETTINGS_H 
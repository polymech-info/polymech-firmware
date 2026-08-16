#ifndef RUNTIME_STATE_H
#define RUNTIME_STATE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <component.h>
#include "config.h"
#include <vector>

#define RUNTIME_STATE_TOTAL_HEATING_TIME_PID_COUNT 8

struct TempProfileRuntime
{
    uint32_t elapsedSec = 0;
    bool enabled = true;
    bool running = false;
    void fromJSON(JsonObjectConst jsonObj);
    void toJSON(JsonObject jsonObj) const;
};

struct OmronRuntime
{
    uint8_t slaveId = 0;
    bool enabled = true;
    void fromJSON(JsonObjectConst jsonObj);
    void toJSON(JsonObject jsonObj) const;
};

struct ManagedComponentState
{
    uint16_t id;
    bool enabled;
};

struct GPIORuntime
{
    uint8_t pin = 0;
    uint8_t type = 0;
    void fromJSON(JsonObjectConst jsonObj);
    void toJSON(JsonObject jsonObj) const;
};

class RuntimeState : public Component
{
public:
    RuntimeState(Component *parent, int id = COMPONENT_KEY_RUNTIME_STATE);

    TempProfileRuntime tempProfiles[PROFILE_TEMPERATURE_COUNT];
    OmronRuntime omronPids[NUM_OMRON_DEVICES];
    std::vector<ManagedComponentState> loadedManagedComponents;
    std::vector<GPIORuntime> gpioPins;

    uint32_t totalKwh = 0;
    uint32_t totalCents = 0;
    uint32_t totalRuntimeHours = 0;
    uint32_t totalHeatingTimeS = 0;
    uint32_t totalHeatingTimePidS[RUNTIME_STATE_TOTAL_HEATING_TIME_PID_COUNT];
    uint32_t totalCycleCount = 0;

    bool isSlave = false;
    bool allOmronStop = false;
    bool allOmronComWrite = false;

    bool fromJSON(JsonVariantConst json);
    bool toJSON(JsonVariant json) const;

    bool load(const char *filename = RUNTIME_STATE_FILENAME);
    bool save(const char *filename = RUNTIME_STATE_FILENAME);

    short setup() override;

    void addManagedComponent(Component *component);

private:
    std::vector<Component *> _managedComponents;
    class PHApp *_app;
};

#endif // RUNTIME_STATE_H
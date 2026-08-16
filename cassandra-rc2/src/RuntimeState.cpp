#include "RuntimeState.h"
#include <LittleFS.h>
#include <ArduinoLog.h>
#include "json.h"
#include "app-logger.h"
#include "PHApp.h"

extern PHApp app;

short RuntimeState::setup()
{
#ifdef ENABLE_PROFILE_TEMPERATURE
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        tempProfiles[i].elapsedSec = 0;
        tempProfiles[i].enabled = false;
        tempProfiles[i].running = false;
    }
#endif
#ifdef ENABLE_OMRON_E5
    for (int i = 0; i < NUM_OMRON_DEVICES; ++i)
    {
        omronPids[i].slaveId = OMRON_E5_SLAVE_ID_BASE + i;
        omronPids[i].enabled = false;
    }
#endif
    totalKwh = 0;
    totalCents = 0;
    totalRuntimeHours = 0;
    totalHeatingTimeS = 0;
    for (int i = 0; i < RUNTIME_STATE_TOTAL_HEATING_TIME_PID_COUNT; ++i)
    {
        totalHeatingTimePidS[i] = 0;
    }
    totalCycleCount = 0;

    isSlave = false;
    allOmronStop = false;
    allOmronComWrite = false;
    return E_OK;
}

RuntimeState::RuntimeState(Component *parent, int id) : Component("RuntimeState", COMPONENT_KEY_RUNTIME_STATE, Component::COMPONENT_DEFAULT, parent)
{
    _app = static_cast<PHApp *>(parent);
    _managedComponents.reserve(20);
}

// TempProfileRuntime implementation
void TempProfileRuntime::fromJSON(JsonObjectConst jsonObj)
{
    if (!jsonObj["elapsedSec"].isNull())
    {
        elapsedSec = jsonObj["elapsedSec"].as<uint32_t>();
    }
    if (!jsonObj["enabled"].isNull())
    {
        enabled = jsonObj["enabled"].as<bool>();
    }
    if (!jsonObj["running"].isNull())
    {
        running = jsonObj["running"].as<bool>();
    }
}

void TempProfileRuntime::toJSON(JsonObject jsonObj) const
{
    jsonObj["elapsedSec"] = elapsedSec;
    jsonObj["enabled"] = enabled;
    jsonObj["running"] = running;
}

// OmronRuntime implementation
void OmronRuntime::fromJSON(JsonObjectConst jsonObj)
{
    if (!jsonObj["slaveId"].isNull())
    {
        slaveId = jsonObj["slaveId"].as<uint8_t>();
    }
    if (!jsonObj["enabled"].isNull())
    {
        enabled = jsonObj["enabled"].as<bool>();
    }
}

void OmronRuntime::toJSON(JsonObject jsonObj) const
{
    jsonObj["slaveId"] = slaveId;
    jsonObj["enabled"] = enabled;
}

void GPIORuntime::fromJSON(JsonObjectConst jsonObj)
{
    if (!jsonObj["pin"].isNull())
    {
        pin = jsonObj["pin"].as<uint8_t>();
    }
    if (!jsonObj["type"].isNull())
    {
        type = jsonObj["type"].as<uint8_t>();
    }
}

void GPIORuntime::toJSON(JsonObject jsonObj) const
{
    jsonObj["pin"] = pin;
    jsonObj["type"] = type;
}

// RuntimeState implementation
bool RuntimeState::fromJSON(JsonVariantConst json)
{
    JsonObjectConst obj = json.as<JsonObjectConst>();
    if (obj.isNull())
    {
        return false;
    }

#ifdef ENABLE_PROFILE_TEMPERATURE
    if (!obj["tempProfiles"].isNull())
    {
        JsonArrayConst arr = obj["tempProfiles"].as<JsonArrayConst>();
        int i = 0;
        for (JsonVariantConst v : arr)
        {
            if (i < PROFILE_TEMPERATURE_COUNT)
            {
                tempProfiles[i].fromJSON(v.as<JsonObjectConst>());
                i++;
            }
        }
    }
#endif
#ifdef ENABLE_OMRON_E5
    if (!obj["omronPids"].isNull())
    {
        JsonArrayConst arr = obj["omronPids"].as<JsonArrayConst>();
        int i = 0;
        for (JsonVariantConst v : arr)
        {
            if (i < NUM_OMRON_DEVICES)
            {
                omronPids[i].fromJSON(v.as<JsonObjectConst>());
                i++;
            }
        }
    }
#endif
    if (!obj["totalKwh"].isNull())
        totalKwh = obj["totalKwh"].as<uint32_t>();
    if (!obj["totalCents"].isNull())
        totalCents = obj["totalCents"].as<uint32_t>();
    if (!obj["totalRuntimeHours"].isNull())
        totalRuntimeHours = obj["totalRuntimeHours"].as<uint32_t>();
    if (!obj["totalHeatingTimeS"].isNull())
        totalHeatingTimeS = obj["totalHeatingTimeS"].as<uint32_t>();
#ifdef ENABLE_OMRON_E5
    if (!obj["totalHeatingTimePidS"].isNull())
    {
        JsonArrayConst arr = obj["totalHeatingTimePidS"].as<JsonArrayConst>();
        int i = 0;
        for (JsonVariantConst v : arr)
        {
            if (i < RUNTIME_STATE_TOTAL_HEATING_TIME_PID_COUNT)
            {
                totalHeatingTimePidS[i] = v.as<uint32_t>();
                i++;
            }
        }
    }
#endif
    if (!obj["totalCycleCount"].isNull())
        totalCycleCount = obj["totalCycleCount"].as<uint32_t>();

    if (!obj["isSlave"].isNull())
        isSlave = obj["isSlave"].as<bool>();
    if (!obj["allOmronStop"].isNull())
        allOmronStop = obj["allOmronStop"].as<bool>();
    if (!obj["allOmronComWrite"].isNull())
        allOmronComWrite = obj["allOmronComWrite"].as<bool>();

    gpioPins.clear();
    if (!obj["gpioPins"].isNull())
    {
        JsonArrayConst arr = obj["gpioPins"].as<JsonArrayConst>();
        for (JsonVariantConst v : arr)
        {
            JsonObjectConst gpioState = v.as<JsonObjectConst>();
            if (!gpioState.isNull())
            {
                GPIORuntime state;
                state.fromJSON(gpioState);
                gpioPins.push_back(state);
            }
        }
    }

    if (!obj["managedComponents"].isNull())
    {
        JsonArrayConst arr = obj["managedComponents"].as<JsonArrayConst>();
        for (JsonVariantConst v : arr)
        {
            JsonObjectConst compState = v.as<JsonObjectConst>();
            if (!compState.isNull())
            {
                ManagedComponentState state;
                state.id = compState["id"].as<uint16_t>();
                state.enabled = compState["enabled"].as<bool>();
                loadedManagedComponents.push_back(state);
            }
        }
    }
    return true;
}

bool RuntimeState::toJSON(JsonVariant json) const
{
    JsonObject obj = json.to<JsonObject>();

#ifdef ENABLE_PROFILE_TEMPERATURE
    JsonArray tempProfilesArr = obj["tempProfiles"].to<JsonArray>();
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; i++)
    {
        tempProfiles[i].toJSON(tempProfilesArr.add<JsonObject>());
    }
#endif
#ifdef ENABLE_OMRON_E5
    JsonArray omronPidsArr = obj["omronPids"].to<JsonArray>();
    for (int i = 0; i < NUM_OMRON_DEVICES; i++)
    {
        omronPids[i].toJSON(omronPidsArr.add<JsonObject>());
    }
#endif
    obj["totalKwh"] = totalKwh;
    obj["totalCents"] = totalCents;
    obj["totalRuntimeHours"] = totalRuntimeHours;
    obj["totalHeatingTimeS"] = totalHeatingTimeS;
#ifdef ENABLE_OMRON_E5
    JsonArray heatingTimePidArr = obj["totalHeatingTimePidS"].to<JsonArray>();
    for (int i = 0; i < RUNTIME_STATE_TOTAL_HEATING_TIME_PID_COUNT; i++)
    {
        heatingTimePidArr.add(totalHeatingTimePidS[i]);
    }
#endif
    obj["totalCycleCount"] = totalCycleCount;

    obj["isSlave"] = isSlave;
    obj["allOmronStop"] = allOmronStop;
    obj["allOmronComWrite"] = allOmronComWrite;

    JsonArray gpioPinsArr = obj["gpioPins"].to<JsonArray>();
    for (const auto &state : gpioPins)
    {
        state.toJSON(gpioPinsArr.add<JsonObject>());
    }

    JsonArray managedCompArr = obj["managedComponents"].to<JsonArray>();
    // L_INFO("RuntimeState::toJSON - Managed components count: %d", _managedComponents.size());
    for (const auto *comp : _managedComponents)
    {
        if (comp)
        {
            // L_INFO("RuntimeState::toJSON - Processing component %p", comp);
            JsonObject compState = managedCompArr.add<JsonObject>();
            compState["id"] = comp->id;
            compState["enabled"] = const_cast<Component *>(comp)->enabled();
        }
        else
        {
            // L_ERROR("RuntimeState::toJSON - Found null component pointer");
        }
    }
    return true;
}

bool RuntimeState::load(const char *path)
{
    if (!LittleFS.begin())
    {
        LS_ERROR(F("Failed to mount LittleFS"));
        return false;
    }

    File file = LittleFS.open(path);
    if (!file)
    {
        LS_ERROR(F("Failed to open runtime state file: %s"), path);
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        LS_ERROR(F("Failed to parse runtime state file '%s': %s"), path, error.c_str());
        return false;
    }
    return fromJSON(doc.as<JsonVariant>());
}

bool RuntimeState::save(const char *path)
{
    const char *stateFilePath = (path == nullptr) ? RUNTIME_STATE_FILENAME : path;

    if (!LittleFS.begin())
    {
        L_ERROR("Failed to initialize LittleFS for save.");
        return false;
    }

    JsonDocument doc;
    toJSON(doc.to<JsonVariant>());

    File file = LittleFS.open(stateFilePath, "w");
    if (!file)
    {
        L_ERROR("Failed to open file for writing: %s", stateFilePath);
        return false;
    }

    size_t bytesWritten = serializeJson(doc, file);
    file.close();

    if (bytesWritten == 0)
    {
        L_ERROR("Failed to write to file: %s", stateFilePath);
        return false;
    }
    // L_INFO("Saved runtime state to %s", stateFilePath);
    return true;
}

void RuntimeState::addManagedComponent(Component *component)
{
    if (component)
    {
        for (auto *c : _managedComponents)
        {
            if (c->id == component->id)
            {
                return; // Already present
            }
        }
        _managedComponents.push_back(component);
    }
}

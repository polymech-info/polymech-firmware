#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "modbus/NetworkComponent.h"
#include "config.h"
#include "enums.h"
#include <LittleFS.h>
#include <ArduinoLog.h>
#include "json.h"
#include "NetworkValue.h"

#define MAX_CONTROLLERS_PER_PARTITION 4
#define MAX_PARTITIONS 2
#define MAX_SLAVES 1
#define MAX_SETTINGS 50

struct ControllerConfig
{
    uint8_t slaveId = 0;
    String name;
    bool enabled = true;
    void fromJSON(JsonObjectConst jsonObj);
    void toJSON(JsonObject jsonObj) const;
};

struct PartitionConfig
{
    String name;
    ControllerConfig controllers[MAX_CONTROLLERS_PER_PARTITION];
    uint8_t controllerCount = 0;

    // For auto-generation from UI, if needed
    uint8_t startSlaveId = 0;
    uint8_t numControllers = 0;

    void fromJSON(JsonObjectConst jsonObj);
    void toJSON(JsonObject jsonObj) const;
};

enum class SettingValueType
{
    SVT_NONE,
    SVT_STRING,
    SVT_LONG,
    SVT_BOOL
};

class NetworkValueBase;

struct Setting
{
    bool enabled = true;
    String name;
    String group;
    int flags = 0;
    int parent = 0;

    SettingValueType type = SettingValueType::SVT_NONE;
    NetworkValueBase *nv = nullptr;

    Setting();
    void fromJSON(JsonObjectConst jsonObj, Component *owner);
    void toJSON(JsonObject jsonObj) const;
};

class Settings : public NetworkComponent<MAX_SETTINGS>
{
public:
    PartitionConfig partitions[MAX_PARTITIONS];
    uint8_t partitionCount = 0;

    String Master;
    String Slaves[MAX_SLAVES];
    uint8_t slaveCount = 0;

    std::vector<Setting> settings;

    Settings(Component *owner);
    ~Settings();

    void fromJSON(JsonVariantConst json);
    void toJSON(JsonVariant json) const;

    bool load(const char *filename = "/partitions.json");
    bool save(const char *filename = "/partitions.json");
    void migrate(const char *filename = "/partitions.json");

    void addDefaults();

    bool get(const char *name, bool defaultValue) const;
    uint32_t get(const char *name, uint32_t defaultValue) const;
    String get(const char *name, const char *defaultValue) const;

    void set(const char *group, const char *name, bool value);
    void set(const char *group, const char *name, uint32_t value);
    void set(const char *group, const char *name, const String &value);

    short setup() override;

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;
};

#endif // SETTINGS_H
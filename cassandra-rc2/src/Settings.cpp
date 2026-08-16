#include "Settings.h"
#include "NetworkValue.h"

#define ADD_SETTING_BOOL(setting_name, setting_group, default_value)    \
    do                                                                  \
    {                                                                   \
        Setting s;                                                      \
        s.name = setting_name;                                          \
        s.group = setting_group;                                        \
        s.type = SettingValueType::SVT_BOOL;                            \
        s.nv = new NetworkValue<bool>(this, id, s.name.c_str());        \
        static_cast<NetworkValue<bool> *>(s.nv)->update(default_value); \
        settings.push_back(s);                                          \
    } while (0)

#define ADD_SETTING_LONG(setting_name, setting_group, default_value)        \
    do                                                                      \
    {                                                                       \
        Setting s;                                                          \
        s.name = setting_name;                                              \
        s.group = setting_group;                                            \
        s.type = SettingValueType::SVT_LONG;                                \
        s.nv = new NetworkValue<uint32_t>(this, id, s.name.c_str());        \
        static_cast<NetworkValue<uint32_t> *>(s.nv)->update(default_value); \
        settings.push_back(s);                                              \
    } while (0)

void ControllerConfig::fromJSON(JsonObjectConst jsonObj)
{
    if (!jsonObj["slaveid"].isNull())
    {
        slaveId = jsonObj["slaveid"].as<uint8_t>();
    }
    if (!jsonObj["enabled"].isNull())
    {
        enabled = jsonObj["enabled"].as<bool>();
    }
    if (!jsonObj["name"].isNull())
    {
        name = jsonObj["name"].as<String>();
    }
    else
    {
        name = "Controller " + String(slaveId);
    }
}

void ControllerConfig::toJSON(JsonObject jsonObj) const
{
    jsonObj["slaveid"] = slaveId;
    jsonObj["name"] = name;
    jsonObj["enabled"] = enabled;
}

// PartitionConfig implementation
void PartitionConfig::fromJSON(JsonObjectConst jsonObj)
{
    if (!jsonObj["name"].isNull())
    {
        name = jsonObj["name"].as<String>();
    }
    else
    {
        name = "Default Partition";
    }

    if (!jsonObj["startslaveid"].isNull())
    {
        startSlaveId = jsonObj["startslaveid"].as<uint8_t>();
    }
    if (!jsonObj["numcontrollers"].isNull())
    {
        numControllers = jsonObj["numcontrollers"].as<uint8_t>();
    }

    controllerCount = 0;
    if (!jsonObj["controllers"].isNull())
    {
        JsonArrayConst controllersArr = jsonObj["controllers"].as<JsonArrayConst>();
        for (JsonVariantConst controllerJson : controllersArr)
        {
            if (controllerCount < MAX_CONTROLLERS_PER_PARTITION)
            {
                controllers[controllerCount].fromJSON(controllerJson.as<JsonObjectConst>());
                controllerCount++;
            }
        }
    }
    else if (numControllers > 0)
    {
        // Auto-generate controllers
        for (uint8_t i = 0; i < numControllers && i < MAX_CONTROLLERS_PER_PARTITION; ++i)
        {
            controllers[i].slaveId = startSlaveId + i;
            controllers[i].name = "Controller " + String(controllers[i].slaveId);
            controllerCount++;
        }
    }
}

void PartitionConfig::toJSON(JsonObject jsonObj) const
{
    jsonObj["name"] = name;

    JsonArray controllersArr = jsonObj["controllers"].to<JsonArray>();
    for (uint8_t i = 0; i < controllerCount; i++)
    {
        controllers[i].toJSON(controllersArr.add<JsonObject>());
    }

    if (startSlaveId > 0 || numControllers > 0)
    {
        jsonObj["startslaveid"] = startSlaveId;
        jsonObj["numcontrollers"] = numControllers;
    }
}

// Setting implementation
Setting::Setting() {}

void Setting::fromJSON(JsonObjectConst jsonObj, Component *owner)
{
    if (!jsonObj["enabled"].isNull())
    {
        enabled = jsonObj["enabled"].as<bool>();
    }
    if (!jsonObj["name"].isNull())
    {
        name = jsonObj["name"].as<String>();
    }
    if (!jsonObj["group"].isNull())
    {
        group = jsonObj["group"].as<String>();
    }
    if (!jsonObj["flags"].isNull())
    {
        flags = jsonObj["flags"].as<int>();
    }
    if (!jsonObj["parent"].isNull())
    {
        parent = jsonObj["parent"].as<int>();
    }

    if (!jsonObj["type"].isNull())
    {
        String typeStr = jsonObj["type"].as<String>();
        if (typeStr == "string")
        {
            // Not implemented for network value
            if (type != SettingValueType::SVT_STRING && nv != nullptr)
            {
                delete nv;
                nv = nullptr;
            }
            type = SettingValueType::SVT_STRING;
        }
        else if (typeStr == "long")
        {
            if (type != SettingValueType::SVT_LONG && nv != nullptr)
            {
                delete nv;
                nv = nullptr;
            }
            type = SettingValueType::SVT_LONG;
            if (!nv)
            {
                nv = new NetworkValue<uint32_t>(owner, owner->id, name.c_str());
            }
            if (!jsonObj["value"].isNull())
            {
                static_cast<NetworkValue<uint32_t> *>(nv)->update(jsonObj["value"].as<long>());
            }
        }
        else if (typeStr == "bool")
        {
            if (type != SettingValueType::SVT_BOOL && nv != nullptr)
            {
                delete nv;
                nv = nullptr;
            }
            type = SettingValueType::SVT_BOOL;
            if (!nv)
            {
                nv = new NetworkValue<bool>(owner, owner->id, name.c_str());
            }
            if (!jsonObj["value"].isNull())
            {
                static_cast<NetworkValue<bool> *>(nv)->update(jsonObj["value"].as<bool>());
            }
        }
    }
}

void Setting::toJSON(JsonObject jsonObj) const
{
    jsonObj["enabled"] = enabled;
    jsonObj["name"] = name;
    jsonObj["group"] = group;
    jsonObj["flags"] = flags;
    jsonObj["parent"] = parent;

    if (nv)
    {
        if (type == SettingValueType::SVT_LONG)
        {
            jsonObj["type"] = "long";
            jsonObj["value"] = static_cast<NetworkValue<uint32_t> *>(nv)->getValue();
        }
        else if (type == SettingValueType::SVT_BOOL)
        {
            jsonObj["type"] = "bool";
            jsonObj["value"] = static_cast<NetworkValue<bool> *>(nv)->getValue();
        }
    }
}

// Settings implementation
Settings::Settings(Component *owner) : NetworkComponent(MB_ADDR_SETTINGS_BASE, "Settings", COMPONENT_KEY_SETTINGS, Component::COMPONENT_DEFAULT, owner), partitionCount(0), slaveCount(0)
{
    addDefaults();
}

Settings::~Settings()
{
    for (auto &setting : settings)
    {
        delete setting.nv;
    }
}

void Settings::fromJSON(JsonVariantConst json)
{
    JsonObjectConst obj = json.as<JsonObjectConst>();
    if (obj.isNull())
    {
        // Fallback for old array-based config
        JsonArrayConst arr = json.as<JsonArrayConst>();
        partitionCount = 0;
        for (JsonVariantConst v : arr)
        {
            if (partitionCount < MAX_PARTITIONS)
            {
                partitions[partitionCount].fromJSON(v.as<JsonObjectConst>());
                partitionCount++;
            }
            else
            {
                Log.warningln(F("Maximum number of partitions (%d) reached. Ignoring extra partitions in config."), MAX_PARTITIONS);
                break;
            }
        }
        return;
    }
    if (!obj["master"].isNull())
    {
        Master = obj["master"].as<String>();
    }
    slaveCount = 0;
    if (!obj["slaves"].isNull())
    {
        JsonArrayConst slavesArr = obj["slaves"].as<JsonArrayConst>();
        for (JsonVariantConst v : slavesArr)
        {
            if (slaveCount < MAX_SLAVES)
            {
                Slaves[slaveCount++] = v.as<String>();
            }
            else
            {
                Log.warningln(F("Maximum number of slaves (%d) reached. Ignoring extra slaves."), MAX_SLAVES);
                break;
            }
        }
    }

    partitionCount = 0;
    if (!obj["partitions"].isNull())
    {
        JsonArrayConst arr = obj["partitions"].as<JsonArrayConst>();
        for (JsonVariantConst v : arr)
        {
            if (partitionCount < MAX_PARTITIONS)
            {
                partitions[partitionCount].fromJSON(v.as<JsonObjectConst>());
                partitionCount++;
            }
            else
            {
                Log.warningln(F("Maximum number of partitions (%d) reached. Ignoring extra partitions in config."), MAX_PARTITIONS);
                break;
            }
        }
    }

    if (!obj["settings"].isNull())
    {
        JsonArrayConst settingsArr = obj["settings"].as<JsonArrayConst>();
        for (JsonVariantConst v : settingsArr)
        {
            JsonObjectConst settingJson = v.as<JsonObjectConst>();
            if (settingJson.isNull() || settingJson["name"].isNull())
            {
                continue;
            }

            String name = settingJson["name"].as<String>();
            bool found = false;

            for (auto &existingSetting : settings)
            {
                if (existingSetting.name == name)
                {
                    existingSetting.fromJSON(settingJson, this);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                Setting newSetting;
                newSetting.fromJSON(settingJson, this);
                settings.push_back(newSetting);
            }
        }
    }
}

void Settings::toJSON(JsonVariant json) const
{
    JsonObject obj = json.to<JsonObject>();

    obj["master"] = Master;

    JsonArray slavesArr = obj["slaves"].to<JsonArray>();
    for (uint8_t i = 0; i < slaveCount; i++)
    {
        slavesArr.add(Slaves[i]);
    }

    JsonArray arr = obj["partitions"].to<JsonArray>();
    for (uint8_t i = 0; i < partitionCount; i++)
    {

        partitions[i].toJSON(arr.add<JsonObject>());
    }

    JsonArray settingsArr = obj["settings"].to<JsonArray>();
    for (const auto &setting : settings)
    {
        setting.toJSON(settingsArr.add<JsonObject>());
    }
}

void Settings::addDefaults()
{
    Master = "plc";
    slaveCount = 1;
    Slaves[0] = "cassandra-1";

    partitionCount = 2;
    // Partition 1
    partitions[0].name = "Cassandra Left";
    partitions[0].controllerCount = 4;
    partitions[0].controllers[0].slaveId = 10;
    partitions[0].controllers[0].name = "Carina";
    partitions[0].controllers[1].slaveId = 11;
    partitions[0].controllers[1].name = "Castor";
    partitions[0].controllers[2].slaveId = 12;
    partitions[0].controllers[2].name = "Cetus";
    partitions[0].controllers[3].slaveId = 13;
    partitions[0].controllers[3].name = "Corona";

    // Partition 2
    partitions[1].name = "Cassandra Right";
    partitions[1].startSlaveId = 14;
    partitions[1].numControllers = 4;
    partitions[1].controllerCount = 4;
    partitions[1].controllers[0].slaveId = 14;
    partitions[1].controllers[0].name = "Coma B";
    partitions[1].controllers[1].slaveId = 15;
    partitions[1].controllers[1].name = "Corvus";
    partitions[1].controllers[2].slaveId = 16;
    partitions[1].controllers[2].name = "Crater";
    partitions[1].controllers[3].slaveId = 17;
    partitions[1].controllers[3].name = "Crux";

    settings.clear();
    ADD_SETTING_BOOL("ALWAYS_WARMUP", "heating", true);
    ADD_SETTING_BOOL("PID_LAG_COMPENSATION", "heating", true);
    ADD_SETTING_BOOL("ALWAYS_STOP_PIDS_ON_PROFILE_FINISHED", "heating", true);
    ADD_SETTING_LONG("MAX_TEMPERATURE", "heating", 280);
    ADD_SETTING_LONG("MIN_TEMPERATURE", "heating", 10);
    ADD_SETTING_LONG("SP_DEADBAND", "heating", 5);
    ADD_SETTING_BOOL("SEQUENTIAL_HEATING_EXCLUSIVE", "heating", false);

    ADD_SETTING_LONG("LOADCELL_SLAVE_ID_0", "loadcell", (uint32_t)LOADCELL_SLAVE_ID_0);
    ADD_SETTING_LONG("LOADCELL_SLAVE_ID_1", "loadcell", (uint32_t)LOADCELL_SLAVE_ID_1);
    ADD_SETTING_LONG("SOLENOID_0_MB_ADDR", "solenoid", (uint32_t)MB_ADDR_SOLENOID_0);

    ADD_SETTING_LONG("OPERATION_TIMEOUT", "Modbus RTU", (uint32_t)OPERATION_TIMEOUT);
    ADD_SETTING_LONG("OMRON_E5_READ_BLOCK_INTERVAL", "Modbus RTU", (uint32_t)OMRON_E5_READ_BLOCK_INTERVAL);
}

short Settings::setup()
{
    NetworkComponent::setup();
    uint16_t offset = E_NVC_USER;
    for (auto &setting : settings)
    {
        if (setting.nv)
        {
            if (setting.type == SettingValueType::SVT_LONG)
            {
                auto *nv_long = static_cast<NetworkValue<uint32_t> *>(setting.nv);
                nv_long->initModbus(_baseAddress + offset, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_HOLD_REGISTER, setting.name.c_str(), this->name.c_str());
                nv_long->initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
                registerBlock(nv_long->getRegisterInfo());
            }
            else if (setting.type == SettingValueType::SVT_BOOL)
            {
                auto *nv_bool = static_cast<NetworkValue<bool> *>(setting.nv);
                nv_bool->initModbus(_baseAddress + offset, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_COIL, setting.name.c_str(), this->name.c_str());
                nv_bool->initNotify(false, true, NetworkValue_ThresholdMode::DIFFERENCE);
                registerBlock(nv_bool->getRegisterInfo());
            }
            offset++;
        }
    }
    return E_OK;
}

short Settings::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    short result = NetworkComponent::mb_tcp_write(reg, networkValue);
    if (result != E_NOT_IMPLEMENTED)
    {
        return result;
    }

    uint16_t offset = reg->startAddress - _baseAddress;
    if (offset >= E_NVC_USER)
    {
        uint16_t settingIndex = offset - E_NVC_USER;
        if (settingIndex < settings.size())
        {
            auto &setting = settings[settingIndex];
            if (setting.type == SettingValueType::SVT_BOOL)
            {
                static_cast<NetworkValue<bool> *>(setting.nv)->update(networkValue);
                return E_OK;
            }
            else if (setting.type == SettingValueType::SVT_LONG)
            {
                static_cast<NetworkValue<uint32_t> *>(setting.nv)->update(networkValue);
                return E_OK;
            }
        }
    }

    return E_INVALID_PARAMETER;
}

short Settings::mb_tcp_read(MB_Registers *reg)
{
    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED)
    {
        return result;
    }

    uint16_t offset = reg->startAddress - _baseAddress;
    if (offset >= E_NVC_USER)
    {
        uint16_t settingIndex = offset - E_NVC_USER;
        if (settingIndex < settings.size())
        {
            const auto &setting = settings[settingIndex];
            if (setting.type == SettingValueType::SVT_BOOL)
            {
                return static_cast<const NetworkValue<bool> *>(setting.nv)->getValue();
            }
            else if (setting.type == SettingValueType::SVT_LONG)
            {
                return static_cast<const NetworkValue<uint32_t> *>(setting.nv)->getValue();
            }
        }
    }

    return E_INVALID_PARAMETER;
}

void Settings::migrate(const char *filename)
{
    if (!LittleFS.begin())
    {
        LS_ERROR(F("Settings::migrate: Failed to mount LittleFS for migration check"));
        return;
    }

    if (!LittleFS.exists(filename))
    {
        LS_INFO(F("Settings::migrate: Settings file not found: %s. Creating with default settings."), filename);
        addDefaults();
        save(filename);
    }
}

bool Settings::load(const char *filename)
{
    if (!LittleFS.begin())
    {
        LS_ERROR(F("Settings::load: Failed to mount LittleFS"));
        return false;
    }

    File file = LittleFS.open(filename);
    if (!file)
    {
        LS_WARN(F("Settings::load: Settings file not found: %s. Using default settings."), filename);
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        LS_ERROR(F("Settings::load: Failed to parse settings file '%s': %s"), filename, error.c_str());
        return false;
    }

    fromJSON(doc.as<JsonVariant>());
    return true;
}

bool Settings::save(const char *filename)
{
    if (!LittleFS.begin())
    {
        LS_ERROR(F("Settings::save: Failed to mount LittleFS for saving"));
        return false;
    }

    File file = LittleFS.open(filename, "w");
    if (!file)
    {
        LS_ERROR(F("Settings::save: Failed to open settings file for writing: %s"), filename);
        return false;
    }

    JsonDocument doc;
    toJSON(doc.to<JsonVariant>());

    if (serializeJson(doc, file) == 0)
    {
        LS_ERROR(F("Settings::save: Failed to write to settings file: %s"), filename);
        file.close();
        return false;
    }

    file.close();
    LS_INFO(F("Settings::save: Settings saved to %s"), filename);
    return true;
}

bool Settings::get(const char *name, bool defaultValue) const
{
    for (const auto &setting : settings)
    {
        if (setting.name == name)
        {
            if (setting.type == SettingValueType::SVT_BOOL)
            {
                return static_cast<const NetworkValue<bool> *>(setting.nv)->getValue();
            }
        }
    }
    return defaultValue;
}

uint32_t Settings::get(const char *name, uint32_t defaultValue) const
{
    for (const auto &setting : settings)
    {
        if (setting.name == name)
        {
            if (setting.type == SettingValueType::SVT_LONG)
            {
                return static_cast<const NetworkValue<uint32_t> *>(setting.nv)->getValue();
            }
        }
    }
    return defaultValue;
}

String Settings::get(const char *name, const char *defaultValue) const
{
    // String settings not implemented with NetworkValue
    return defaultValue;
}

void Settings::set(const char *group, const char *name, bool value)
{
    for (auto &setting : settings)
    {
        if (setting.name == name)
        {
            if (setting.type == SettingValueType::SVT_BOOL && setting.nv)
            {
                static_cast<NetworkValue<bool> *>(setting.nv)->update(value);
            }
            return;
        }
    }
    // If not found, create it? For now, we assume it exists if we are setting it.
    // Or we could verify group matches too if searching by name collision potential.
}

void Settings::set(const char *group, const char *name, uint32_t value)
{
    for (auto &setting : settings)
    {
        if (setting.name == name)
        {
            if (setting.type == SettingValueType::SVT_LONG && setting.nv)
            {
                static_cast<NetworkValue<uint32_t> *>(setting.nv)->update(value);
            }
            return;
        }
    }
}

void Settings::set(const char *group, const char *name, const String &value)
{
    // String settings not implemented with NetworkValue yet in this codebase context
    // according to `get`. So we do nothing or log.
}

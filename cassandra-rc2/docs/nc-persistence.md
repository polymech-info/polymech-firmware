# `Persistence` Mixin for `NetworkComponent`

This document proposes a `Persistence` mixin for `NetworkComponent` to enable automatic saving and loading of `NetworkValue` states to/from disk.

## 1. Overview

The `Persistence` mixin adds functionality to any `NetworkComponent` derivative to persist its `NetworkValue`s. This is useful for retaining state across reboots, such as configuration, calibration data, or last known operational parameters.

The functionality is modeled after the `Settings` component but is designed to be generic and reusable.

## 2. Design

The persistence logic will be encapsulated in a template class `Persistent`, which inherits from the `NetworkComponent` it extends. This allows for compile-time addition of the persistence feature without modifying the core `NetworkComponent` for components that do not require it.

To support serialization, we propose adding virtual methods to `NetworkValueBase` for JSON serialization.

### 2.1. Changes to `NetworkValue.h`

`NetworkValueBase` will be augmented with virtual methods for type identification and JSON handling.

```cpp
// In NetworkValueBase
enum class NetworkValueType {
  NV_NONE,
  NV_BOOL,
  NV_UINT32,
  NV_INT32,
  NV_FLOAT,
  NV_STRING
  // ... other types
};

class NetworkValueBase {
public:
    // ... existing members
    virtual void toJSON(JsonObject& obj) const = 0;
    virtual void fromJSON(JsonObjectConst& obj) = 0;
    virtual NetworkValueType getType() const = 0;
    virtual const char* getName() const = 0;
};
```

The templated `NetworkValue<T>` class will implement these virtual methods.

```cpp
// In NetworkValue<T>

template<typename T>
class NetworkValue : public NetworkValueBase {
public:
    // ... existing members

    void toJSON(JsonObject& obj) const override {
        obj["name"] = this->name.c_str();
        obj["value"] = m_value;
        obj["type"] = static_cast<int>(getType());
    }

    void fromJSON(JsonObjectConst& obj) override {
        if (obj["value"].is<T>()) {
            update(obj["value"].as<T>());
        }
    }

    const char* getName() const override {
        return name.c_str();
    }

    NetworkValueType getType() const override {
        // Specialize this for each supported type
        if (std::is_same<T, bool>::value) return NetworkValueType::NV_BOOL;
        if (std::is_same<T, uint32_t>::value) return NetworkValueType::NV_UINT32;
        // ...
        return NetworkValueType::NV_NONE;
    }
};
```

### 2.2. The `Persistent` Mixin

The mixin will manage a filename and provide `load()` and `save()` methods.

```cpp
#include "NetworkComponent.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

template <class TBase>
class Persistent : public TBase {
protected:
    String _persistenceFile;

public:
    template <typename... Args>
    Persistent(const char* persistenceFile, Args&&... args)
        : TBase(std::forward<Args>(args)...),
          _persistenceFile(persistenceFile) {}

    short setup() override {
        TBase::setup();
        load(); // Automatically load on setup
        return E_OK;
    }

    bool save() {
        if (!LittleFS.begin()) {
            Log.errorln(F("Failed to mount LittleFS for saving %s"), this->name.c_str());
            return false;
        }

        JsonDocument doc;
        JsonArray nvArray = doc.to<JsonArray>();

        for (const auto* nv : this->_networkValues) {
            if (nv) {
                nv->toJSON(nvArray.add<JsonObject>());
            }
        }

        File file = LittleFS.open(_persistenceFile, "w");
        if (!file) {
            Log.errorln(F("Failed to open file for writing: %s"), _persistenceFile.c_str());
            LittleFS.end();
            return false;
        }

        size_t bytesWritten = serializeJson(doc, file);
        file.close();
        LittleFS.end();

        return bytesWritten > 0;
    }

    bool load() {
        if (!LittleFS.begin(true)) {
            Log.errorln(F("Failed to mount LittleFS for loading %s"), this->name.c_str());
            return false;
        }

        if (!LittleFS.exists(_persistenceFile)) {
            Log.infoln(F("Persistence file not found: %s. Using defaults."), _persistenceFile.c_str());
            LittleFS.end();
            return false;
        }

        File file = LittleFS.open(_persistenceFile, "r");
        if (!file) {
             Log.errorln(F("Failed to open file for reading: %s"), _persistenceFile.c_str());
             LittleFS.end();
             return false;
        }

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            Log.errorln(F("Failed to parse JSON from %s: %s"), _persistenceFile.c_str(), error.c_str());
            LittleFS.end();
            return false;
        }

        JsonArrayConst nvArray = doc.as<JsonArrayConst>();
        for (JsonObjectConst nvJson : nvArray) {
            const char* name = nvJson["name"];
            if (!name) continue;

            for (auto* nv : this->_networkValues) {
                if (nv && strcmp(nv->getName(), name) == 0) {
                    nv->fromJSON(nvJson);
                    break;
                }
            }
        }
        LittleFS.end();
        return true;
    }
};
```

## 3. Usage Example

Here's how a component `MyComponent` would use the `Persistent` mixin.

### 3.1. Component Definition

Instead of inheriting from `NetworkComponent` directly, it inherits from `Persistent<NetworkComponent<...>>`.

```cpp
// MyComponent.h

#include "modbus/NetworkComponent.h"
#include "mixins/Persistent.h"

const size_t MYCOMPONENT_BLOCK_COUNT = 10;

class MyComponent : public Persistent<NetworkComponent<MYCOMPONENT_BLOCK_COUNT>> {
private:
    NetworkValue<float> _calibrationFactor;
    NetworkValue<int> _threshold;

public:
    MyComponent(Component* owner);
    short setup() override;
};
```

### 3.2. Component Implementation

The constructor passes the persistence filename and other `NetworkComponent` arguments up to the `Persistent` mixin constructor.

```cpp
// MyComponent.cpp

#include "MyComponent.h"

MyComponent::MyComponent(Component* owner)
    : Persistent(
        "/mycomponent.json",
        MB_ADDR_MYCOMPONENT_BASE,
        "MyComponent",
        "my_comp_key",
        owner),
      _calibrationFactor(this, this->id, "CalibrationFactor", 1.0f),
      _threshold(this, this->id, "Threshold", 100)
{
    addNetworkValue(&_calibrationFactor);
    addNetworkValue(&_threshold);
}

short MyComponent::setup() {
    Persistent::setup();

    _calibrationFactor.initModbus(...);
    registerBlock(_calibrationFactor.getRegisterInfo());
    _threshold.initModbus(...);
    registerBlock(_threshold.getRegisterInfo());

    return E_OK;
}

void MyComponent::updateAndSaveChanges() {
    _calibrationFactor.update(1.23f);
    if (!save()) {
        Log.errorln(F("Failed to save MyComponent settings!"));
    }
}
```

## 4. JSON File Structure

The persisted file (e.g., `/mycomponent.json`) would look like this:

```json
[
  {
    "name": "Enabled",
    "value": true,
    "type": 1
  },
  {
    "name": "CalibrationFactor",
    "value": 1.23,
    "type": 4
  },
  {
    "name": "Threshold",
    "value": 100,
    "type": 3
  }
]
```

This structure is a flat array of all `NetworkValue` objects managed by the component. Each object contains its name, value, and a type identifier. 
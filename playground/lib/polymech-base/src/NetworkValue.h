#ifndef NETWORK_VALUE_H
#define NETWORK_VALUE_H

#include "Component.h"
#include "modbus/ModbusTypes.h"
#include <ArduinoLog.h>
#include <functional>
#include <type_traits> // for std::conditional
#include <components/commons.h>
#include "macros.h"
#include "enums.h"

#define NETWORKVALUE_PERSISTENCE_ENABLED

#ifdef NETWORKVALUE_PERSISTENCE_ENABLED
    #include <ArduinoJson.h>
#endif

/*
 * =====================================================================================
 *                                   FEATURE FLAGS
 * =====================================================================================
 * These flags control which features are compiled into the NetworkValue class.
 * They can be overridden in `platformio.ini` via `build_flags`.
 * Example: build_flags = -DNETWORKVALUE_ENABLE_LOGGING=0
 * By default, all features are enabled.
 * =====================================================================================
 */
#ifndef NETWORKVALUE_ENABLE_LOGGING
#define NETWORKVALUE_ENABLE_LOGGING 1
#endif
#ifndef NETWORKVALUE_ENABLE_MODBUS
#define NETWORKVALUE_ENABLE_MODBUS 1
#endif
#ifndef NETWORKVALUE_ENABLE_NOTIFY
#define NETWORKVALUE_ENABLE_NOTIFY 1
#endif

enum class NetworkValue_ThresholdMode
{
    DIFFERENCE,   // Trigger if abs(newVal - oldVal) >= threshold
    INTERVAL_STEP // Trigger if floor(newVal / threshold) != floor(oldVal / threshold)
};

/*
 * =====================================================================================
 *                                   FEATURE DEFINITIONS
 * =====================================================================================
 * These are modular classes that provide specific functionalities (e.g., logging,
 * network exposure, value tracking). They are composed by the NetworkValue class.
 * =====================================================================================
 */

/**
 * @class NV_Logging
 * @brief A feature that adds logging capabilities.
 */
class NV_Logging
{
protected:
    bool m_loggingEnabled = false;

public:
    void init_feature(bool enable)
    {
        enableLogging(enable);
    }

    void enableLogging(bool enable)
    {
        m_loggingEnabled = enable;
    }

    void clear_feature()
    {
        m_loggingEnabled = false;
    }

    template <typename... Args>
    void log(int level, const char *componentName, const char *fmt, Args... args) const
    {
        if (m_loggingEnabled)
        {
            // ArduinoLog requires a different pattern.
            // We can't easily variadically wrap it without a more complex helper.
            // For this POC, we'll log a static message.
            Log.traceln("[%s] Change detected.", componentName);
        }
    }

    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const
    {
        Log.traceln(F("  [Feature: Logging] %s"), m_loggingEnabled ? "Enabled" : "Disabled");
    }
};

/**
 * @class NV_Modbus
 * @brief A feature that makes the value available over Modbus.
 */
class NV_Modbus
{
protected:
    MB_Registers m_regInfo;

public:
    void init_feature(ushort startAddress, ushort count, ushort componentId, ushort slaveId, E_FN_CODE type, const char *name, const char *group)
    {
        configureModbus(startAddress, count, componentId, slaveId, type, name, group);
    }

    void configureModbus(ushort startAddress, ushort count, ushort componentId, ushort slaveId, E_FN_CODE type, const char *name, const char *group)
    {
        m_regInfo = MB_Registers(startAddress, count, type, MB_ACCESS_READ_WRITE, componentId, slaveId, name, group);
    }

    void clear_feature()
    {
        m_regInfo = {}; // Default constructs the struct
    }

    MB_Registers getRegisterInfo() const { return m_regInfo; }

    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const
    {
        Log.infoln(F("  [Feature: Modbus] Addr: %d, Name: %s"), m_regInfo.startAddress, m_regInfo.name);
    }
};

/**
 * @class NV_Notify
 * @brief A feature that handles wrapping and tracking a value.
 */
template <typename T>
class NV_Notify
{
private:
    // Helper for threshold comparison - DIFFERENCE mode, enum types
    template <typename U = T, typename std::enable_if<std::is_enum<U>::value, int>::type = 0>
    static bool checkThresholdDifference(U newVal, U oldVal, U threshold)
    {
        if (static_cast<typename std::underlying_type<U>::type>(threshold) == 1)
        {
            return newVal != oldVal;
        }
        else
        {
            return (std::abs(static_cast<long long>(static_cast<typename std::underlying_type<U>::type>(newVal)) - static_cast<long long>(static_cast<typename std::underlying_type<U>::type>(oldVal))) >=
                    std::abs(static_cast<long long>(static_cast<typename std::underlying_type<U>::type>(threshold))));
        }
    }

    // Helper for threshold comparison - DIFFERENCE mode, non-enum, signed types
    template <typename U = T, typename std::enable_if<!std::is_enum<U>::value && std::is_signed<U>::value, int>::type = 0>
    static bool checkThresholdDifference(U newVal, U oldVal, U threshold)
    {
        return std::abs(newVal - oldVal) >= threshold;
    }

    // Helper for threshold comparison - DIFFERENCE mode, non-enum, unsigned types
    template <typename U = T, typename std::enable_if<!std::is_enum<U>::value && std::is_unsigned<U>::value, int>::type = 0>
    static bool checkThresholdDifference(U newVal, U oldVal, U threshold)
    {
        U diff = (newVal > oldVal) ? (newVal - oldVal) : (oldVal - newVal);
        return diff >= threshold;
    }

    // Helper for threshold comparison - INTERVAL_STEP mode (for arithmetic types)
    template <typename U = T, typename std::enable_if<std::is_arithmetic<U>::value, int>::type = 0>
    static bool checkThresholdIntervalStep(U newVal, U oldVal, U stepInterval)
    {
        if (stepInterval == 0)
            return false; // Avoid division by zero
        return (static_cast<long long>(newVal / stepInterval)) != (static_cast<long long>(oldVal / stepInterval));
    }

    // Fallback for non-arithmetic types with INTERVAL_STEP (should ideally not be chosen or error)
    template <typename U = T, typename std::enable_if<!std::is_arithmetic<U>::value, int>::type = 0>
    static bool checkThresholdIntervalStep(U newVal, U oldVal, U stepInterval)
    {
        return false; // Or throw, or static_assert(false, ...)
    }

protected:
    T m_value{};
    T m_lastValueOnUpdate{};
    T m_threshold{};
    NetworkValue_ThresholdMode m_thresholdMode = NetworkValue_ThresholdMode::DIFFERENCE;
    std::function<void(const T &, const T &)> m_onUpdateCallback = nullptr;

public:
    void init_feature(T initial, T threshold, NetworkValue_ThresholdMode mode, std::function<void(const T &, const T &)> cb = nullptr)
    {
        configureValue(initial, threshold, mode, cb);
    }

    void configureValue(T initial, T threshold, NetworkValue_ThresholdMode mode, std::function<void(const T &, const T &)> cb = nullptr)
    {
        m_value = initial;
        m_lastValueOnUpdate = initial;
        m_threshold = threshold;
        m_thresholdMode = mode;
        m_onUpdateCallback = cb;
    }

    void clear_feature()
    {
        m_value = T{};
        m_lastValueOnUpdate = T{};
        m_threshold = T{};
        m_onUpdateCallback = nullptr;
        m_thresholdMode = NetworkValue_ThresholdMode::DIFFERENCE;
    }

    bool checkChanged(const T &newValue) const
    {
        // Compare the new value to the value at the time of the last update.
        bool changed = false;
        if (m_thresholdMode == NetworkValue_ThresholdMode::INTERVAL_STEP)
        {
            changed = checkThresholdIntervalStep(newValue, m_lastValueOnUpdate, m_threshold);
        }
        else
        {
            changed = checkThresholdDifference(newValue, m_lastValueOnUpdate, m_threshold);
        }
        return changed;
    }

    void applyUpdate(const T &newValue)
    {
        T oldValue = m_lastValueOnUpdate;
        m_value = newValue;
        m_lastValueOnUpdate = newValue;
        if (m_onUpdateCallback)
        {
            m_onUpdateCallback(oldValue, m_value);
        }
    }

    T getValue() const { return m_value; }
    T& getValueRef() { return m_value; }

    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const {}
};

// Template specialization of NV_Notify for std::array
template <typename T, size_t N>
class NV_Notify<std::array<T, N>>
{
protected:
    std::array<T, N> m_value{};
    std::array<T, N> m_lastValueOnUpdate{};
    std::array<T, N> m_threshold{}; // Not used for arrays, but kept for signature compatibility
    NetworkValue_ThresholdMode m_thresholdMode = NetworkValue_ThresholdMode::DIFFERENCE;
    std::function<void(const std::array<T, N> &, const std::array<T, N> &)> m_onUpdateCallback = nullptr;

public:
    void init_feature(const std::array<T, N> &initial, const std::array<T, N> &threshold, NetworkValue_ThresholdMode mode, std::function<void(const std::array<T, N> &, const std::array<T, N> &)> cb = nullptr)
    {
        configureValue(initial, threshold, mode, cb);
    }

    void configureValue(const std::array<T, N> &initial, const std::array<T, N> &threshold, NetworkValue_ThresholdMode mode, std::function<void(const std::array<T, N> &, const std::array<T, N> &)> cb = nullptr)
    {
        m_value = initial;
        m_lastValueOnUpdate = initial;
        m_threshold = threshold;
        m_thresholdMode = mode;
        m_onUpdateCallback = cb;
    }

    void clear_feature()
    {
        m_value.fill(T{});
        m_lastValueOnUpdate.fill(T{});
        m_threshold.fill(T{});
        m_onUpdateCallback = nullptr;
        m_thresholdMode = NetworkValue_ThresholdMode::DIFFERENCE;
    }

    // This specialized version of checkChanged now compares the new value against the last known state.
    bool checkChanged(const std::array<T, N> &newValue) const
    {
        for (size_t i = 0; i < N; ++i) {
            if (newValue[i] != m_lastValueOnUpdate[i]) {
                return true;
            }
        }
        return false;
    }

    void applyUpdate(const std::array<T, N> &newValue)
    {
        std::array<T, N> oldValue = m_lastValueOnUpdate;
        m_value = newValue;
        m_lastValueOnUpdate = newValue;
        if (m_onUpdateCallback)
        {
            m_onUpdateCallback(oldValue, m_value);
        }
    }

    std::array<T, N> getValue() const { return m_value; }
    std::array<T, N>& getValueRef() { return m_value; }

    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const {}
};

/*
 * =====================================================================================
 *                                  NetworkValue AGGREGATOR
 * =====================================================================================
 * This class composes the features into a single Component.
 * =====================================================================================
 */

class NetworkValueBase : public Component
{
public:
    using Component::Component;
    virtual ~NetworkValueBase() = default;
};

template <typename T>
class NetworkValue : public NetworkValueBase,
                     public maybe<NETWORKVALUE_ENABLE_LOGGING, NV_Logging>,
                     public maybe<NETWORKVALUE_ENABLE_MODBUS, NV_Modbus>,
                     public maybe<NETWORKVALUE_ENABLE_NOTIFY, NV_Notify<T>>
{
private:
    // --- Tag Dispatching for update() method ---
    struct regular_type_tag
    {
    };
    struct array_type_tag
    {
    };

    template <typename U>
    struct get_update_tag
    {
        using type = regular_type_tag;
    };
    template <typename U, size_t M>
    struct get_update_tag<std::array<U, M>>
    {
        using type = array_type_tag;
    };

    // Implementation for regular types (sends onMessage)
    void update_impl(const T &newValue, regular_type_tag)
    {
        if (this->owner)
        {
            MB_UpdateData update_msg{}; // Zero-initialize
            if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            {
                MB_Registers regInfo = this->getRegisterInfo();
                update_msg.address = regInfo.startAddress;
                update_msg.value = getModbusValueHelper(newValue);
                update_msg.count = 1;
                update_msg.userData = nullptr;
                update_msg.slaveId = regInfo.slaveId;
                update_msg.functionCode = regInfo.type;
                // Log.infoln(F("NetworkValue '%s' updating Modbus. Addr: %d, Value: %d"), this->name.c_str(), update_msg.address, update_msg.value);
            }else{
                Log.warningln(F("  [update_impl] Modbus is disabled for '%s', cannot send notification."), this->name.c_str());
            }
            update_msg.componentId = this->id;
            this->owner->onMessage(this->id, E_CALLS::EC_USER, E_MessageFlags::E_MF_NONE, &update_msg, this);
        }
        else
        {
            Log.warningln(F("  [update_impl] No owner for '%s', cannot send notification."), this->name.c_str());
        }
        this->applyUpdate(newValue);
    }

    // Implementation for std::array (now sends onMessage)
    template <typename U, size_t M>
    void update_impl(const std::array<U, M> &newValue, array_type_tag)
    {
        // First, apply the update to our internal state. This copies the data
        // from the (potentially temporary) newValue into our stable m_value.
        this->applyUpdate(newValue); 

        if (this->owner)
        {
            MB_UpdateData update_msg{}; // Zero-initialize
            if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            {
                MB_Registers regInfo = this->getRegisterInfo();
                update_msg.address = regInfo.startAddress;
                update_msg.count = this->m_value.size();
                // Now, point userData to our stable internal m_value buffer.
                update_msg.userData = const_cast<void *>(static_cast<const void *>(this->m_value.data()));
                update_msg.slaveId = regInfo.slaveId;
                update_msg.functionCode = regInfo.type;
            }
            else
            {
                Log.warningln(F("  [update_impl] Modbus is disabled for '%s', cannot send notification."), this->name.c_str());
            }
            update_msg.componentId = this->id;
            this->owner->onMessage(this->id, E_CALLS::EC_USER, E_MessageFlags::E_MF_NONE, &update_msg, this);
        }
        else
        {
            Log.warningln(F("  [update_impl] No owner for '%s', cannot send notification."), this->name.c_str());
        }
    }

    // Helper to get value for Modbus message (short) - enabled for enum types
    template <typename U = T, typename std::enable_if<std::is_enum<U>::value, int>::type = 0>
    static short getModbusValueHelper(U val)
    {
        return static_cast<short>(static_cast<typename std::underlying_type<U>::type>(val));
    }

    // Helper to get value for Modbus message (short) - enabled for non-enum types
    template <typename U = T, typename std::enable_if<!std::is_enum<U>::value, int>::type = 0>
    static short getModbusValueHelper(U val)
    {
        return static_cast<short>(val);
    }

public:
    
    
    NetworkValue(Component *owner, ushort id, 
        const char *name, 
        std::function<void(NetworkValueBase*)> reg_fn = nullptr,
        uint8_t featureFlags = static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL))
        : NetworkValueBase(name, id, COMPONENT_DEFAULT, owner, featureFlags)
    {
        if (reg_fn) {
            reg_fn(this);
        }
/*
        uint8_t availableFlags = static_cast<uint8_t>(NetworkValueFeatureFlags::NONE);
        if (NETWORKVALUE_ENABLE_LOGGING)
            availableFlags |= static_cast<uint8_t>(NetworkValueFeatureFlags::LOGGING);
        if (NETWORKVALUE_ENABLE_MODBUS)
            availableFlags |= static_cast<uint8_t>(NetworkValueFeatureFlags::MODBUS);
        if (NETWORKVALUE_ENABLE_NOTIFY)
            availableFlags |= static_cast<uint8_t>(NetworkValueFeatureFlags::NOTIFY);

        if (featureFlags == static_cast<uint8_t>(NetworkValueFeatureFlags::ALL)) {
            this->fFlags = availableFlags;
        } else {
            // Only enable features that are also compiled in
            this->fFlags = featureFlags & availableFlags;
        }
        */

        if (owner && owner->hasFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG))
        {
            this->setFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG);
            // Logging feature is enabled by default if owner has DEBUG flag
            // and the LOGGING feature flag is set on this instance.
            if (NETWORKVALUE_ENABLE_LOGGING && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            {
                this->initLogging(true);
            }
        }
    }

    void clear()
    {
        this->flags = 0;
        this->nFlags = 0;
        if (NETWORKVALUE_ENABLE_LOGGING && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            this->NV_Logging::clear_feature();
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            this->NV_Modbus::clear_feature();
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
            this->NV_Notify<T>::clear_feature();
    }

    short setup() override
    {
        if (NETWORKVALUE_ENABLE_LOGGING && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            this->NV_Logging::setup_feature();
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            this->NV_Modbus::setup_feature();
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
            this->NV_Notify<T>::setup_feature();
        return E_OK;
    }

    short loop() override
    {
        Component::loop();
        if (NETWORKVALUE_ENABLE_LOGGING && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            this->NV_Logging::loop_feature();
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            this->NV_Modbus::loop_feature();
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
            this->NV_Notify<T>::loop_feature();
        return E_OK;
    }

    short info() override {
        Component::info();
        if (NETWORKVALUE_ENABLE_LOGGING && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            this->NV_Logging::info_feature();
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            this->NV_Modbus::info_feature();
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
            this->NV_Notify<T>::info_feature();
        return E_OK;
    }

    template <typename U = T>
    void update(const U &newValue)
    {
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY) && this->checkChanged(newValue))
        {
            if (NETWORKVALUE_ENABLE_LOGGING && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            {
                this->log(LOG_LEVEL_TRACE, this->name.c_str(), "Value is changing.");
            }
            // Dispatch to the correct implementation based on whether T is an array
            update_impl(newValue, typename get_update_tag<U>::type());
        }
        else if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
        {
            this->m_value = newValue;
        }
        else if (NETWORKVALUE_ENABLE_NOTIFY && !hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
        {
        }
    }

    // --- Feature-specific API ---

    // Logging API
    void initLogging(bool enableLogging)
    {
        if (NETWORKVALUE_ENABLE_LOGGING && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
        {
            NV_Logging::init_feature(enableLogging);
        }
    }

    // Modbus API
    void initModbus(ushort startAddress, ushort count, ushort componentId, ushort slaveId, E_FN_CODE type, const char *name, const char *group)
    {
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
        {
            NV_Modbus::init_feature(startAddress, count, componentId, slaveId, type, name, group);
        }
    }

    MB_Registers getRegisterInfo() const
    {
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
        {
            return NV_Modbus::getRegisterInfo();
        }
        return {};
    }

    // Notify API
    void initNotify(T initial, T threshold, NetworkValue_ThresholdMode mode, std::function<void(const T &, const T &)> cb = nullptr)
    {
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
        {
            NV_Notify<T>::init_feature(initial, threshold, mode, cb);
        }
    }

    T getValue() const { return NV_Notify<T>::getValue(); }
    T& getValueRef() { return NV_Notify<T>::getValueRef(); }

    using NV_Notify<T>::getValue;
    using NV_Notify<T>::getValueRef;
    using NV_Modbus::getRegisterInfo;

    template <typename JsonObjT>
    void toJSON(JsonObjT &obj) const
    {
        obj["id"] = this->id;
        obj["name"] = this->name;
        obj["flags"] = this->flags;
        obj["nFlags"] = this->nFlags;
        obj["fFlags"] = this->fFlags;

#if NETWORKVALUE_ENABLE_LOGGING
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
        {
            obj["logging_enabled"] = this->m_loggingEnabled;
        }
#endif

#if NETWORKVALUE_ENABLE_MODBUS
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
        {
            auto mod = obj.createNestedObject("modbus");
            MB_Registers reg = this->getRegisterInfo();
            mod["startAddress"] = reg.startAddress;
            mod["count"] = reg.count;
            mod["slaveId"] = reg.slaveId;
            mod["type"] = reg.type;
            mod["name"] = reg.name ? reg.name : "";
            mod["group"] = reg.group ? reg.group : "";
        }
#endif

#if NETWORKVALUE_ENABLE_NOTIFY
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
        {
            auto notify = obj.createNestedObject("notify");
            notify["threshold"] = this->m_threshold;
            notify["mode"] = static_cast<int>(this->m_thresholdMode);
        }
#endif
    }

    template <typename JsonObjT>
    void fromJSON(const JsonObjT &obj)
    {
#if NETWORKVALUE_ENABLE_LOGGING
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING) && obj.containsKey("logging_enabled"))
        {
            this->enableLogging(obj["logging_enabled"].template as<bool>());
        }
#endif

#if NETWORKVALUE_ENABLE_MODBUS
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS) && obj.containsKey("modbus"))
        {
            auto mod = obj["modbus"].template as<ArduinoJson::JsonObjectConst>();
            ushort startAddress = mod["startAddress"].template as<ushort>();
            ushort count = mod["count"].template as<ushort>();
            ushort slaveId = mod["slaveId"].template as<ushort>();
            E_FN_CODE fn = static_cast<E_FN_CODE>(mod["type"].template as<uint8_t>());
            const char *mname = mod["name"].template as<const char *>();
            const char *group = mod["group"].template as<const char *>();
            this->configureModbus(startAddress, count, this->id, slaveId, fn, mname, group);
        }
#endif

#if NETWORKVALUE_ENABLE_NOTIFY
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY) && obj.containsKey("notify"))
        {
            auto notify = obj["notify"].template as<ArduinoJson::JsonObjectConst>();
            T threshold = notify["threshold"].template as<T>();
            NetworkValue_ThresholdMode mode = static_cast<NetworkValue_ThresholdMode>(notify["mode"].template as<uint8_t>());
            this->configureValue(this->getValue(), threshold, mode);
        }
#endif
    }
};

/*
 * =====================================================================================
 *                                  EXAMPLE USAGE (POC)
 * =====================================================================================
 * To disable a feature at compile-time, add e.g. `-DNETWORKVALUE_ENABLE_LOGGING=0` to build_flags.
 * To disable a feature at runtime, use the new API.
 * =====================================================================================
 * In TemperatureProfile.h:
 *
 * #include "NetworkValue.h"
 *
 * using StatusValue = NetworkValue<PlotStatus>;
 *
 * // ...
 *
 * =====================================================================================
 * In TemperatureProfile.cpp constructor:
 *
 * // Instantiate with only Modbus and Notify features enabled at runtime
 * _statusValue(this, this->id, "TProf Status",
 *              static_cast<uint8_t>(NetworkValueFeatureFlags::MODBUS) |
 *              static_cast<uint8_t>(NetworkValueFeatureFlags::NOTIFY));
 *
 * // Configure the enabled features
 * _statusValue.initNotify(...);
 * _statusValue.initModbus(...);
 * _statusValue.initLogging(true);
 *
 * // You can also toggle features later
 * _statusValue.enableFeature(NetworkValueFeatureFlags::LOGGING);
 *
 */

#endif // NETWORK_VALUE_H
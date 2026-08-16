#ifndef NETWORK_VALUE_H
#define NETWORK_VALUE_H

#include "Component.h"
#include "modbus/ModbusTypes.h"
#include <ArduinoLog.h>
#include <functional>
#include <type_traits>
#include <components/commons.h>
#include "macros.h"
#include "enums.h"
#include <net/commons.h>

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
#define NETWORKVALUE_ENABLE_LOGGING 0
#endif
#ifndef NETWORKVALUE_ENABLE_MODBUS
#define NETWORKVALUE_ENABLE_MODBUS 1
#endif
#ifndef NETWORKVALUE_ENABLE_NOTIFY
#define NETWORKVALUE_ENABLE_NOTIFY 1
#endif
#ifndef NETWORKVALUE_ENABLE_PROTOBUF
#define NETWORKVALUE_ENABLE_PROTOBUF 0
#endif

#if (NETWORKVALUE_ENABLE_PROTOBUF == 1)
#include "NetworkValuePB.h"
#define NETWORKVALUE_PROTOBUF_INHERITANCE , public maybe<NETWORKVALUE_ENABLE_PROTOBUF, NV_Protobuf>
#else
#define NETWORKVALUE_PROTOBUF_INHERITANCE
#endif

enum class NetworkValue_ThresholdMode : uint8_t
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
    void log(int level, const char *componentName, const char *fmt, Args... args) const {}
    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const {}
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
        return newVal != oldVal;
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
    void (*m_onUpdateCallback)(const T &, const T &) = nullptr;

public:
    void init_feature(T initial, T threshold, NetworkValue_ThresholdMode mode, void (*cb)(const T &, const T &) = nullptr)
    {
        configureValue(initial, threshold, mode, cb);
    }

    void configureValue(T initial, T threshold, NetworkValue_ThresholdMode mode, void (*cb)(const T &, const T &) = nullptr)
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

    bool applyUpdate(const T &newValue)
    {
        if (!checkChanged(newValue))
        {
            return false;
        }
        T oldValue = m_lastValueOnUpdate;
        m_value = newValue;
        m_lastValueOnUpdate = newValue;
        if (m_onUpdateCallback)
        {
            m_onUpdateCallback(oldValue, m_value);
        }
        return true;
    }

    T getValue() const { return m_value; }
    T &getValueRef() { return m_value; }

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
    void (*m_onUpdateCallback)(const std::array<T, N> &, const std::array<T, N> &) = nullptr;

public:
    void init_feature(const std::array<T, N> &initial, const std::array<T, N> &threshold, NetworkValue_ThresholdMode mode, void (*cb)(const std::array<T, N> &, const std::array<T, N> &) = nullptr)
    {
        configureValue(initial, threshold, mode, cb);
    }

    void configureValue(const std::array<T, N> &initial, const std::array<T, N> &threshold, NetworkValue_ThresholdMode mode, void (*cb)(const std::array<T, N> &, const std::array<T, N> &) = nullptr)
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
        for (size_t i = 0; i < N; ++i)
        {
            if (newValue[i] != m_lastValueOnUpdate[i])
            {
                return true;
            }
        }
        return false;
    }

    bool applyUpdate(const std::array<T, N> &newValue)
    {
        if (!checkChanged(newValue))
        {
            return false;
        }
        std::array<T, N> oldValue = m_lastValueOnUpdate;
        m_value = newValue;
        m_lastValueOnUpdate = newValue;
        if (m_onUpdateCallback)
        {
            m_onUpdateCallback(oldValue, m_value);
        }
        return true;
    }

    std::array<T, N> getValue() const { return m_value; }
    std::array<T, N> &getValueRef() { return m_value; }

    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const {}
};

// Template specialization of NV_Notify for std::array<bool, N>
template <size_t N>
class NV_Notify<std::array<bool, N>>
{
protected:
    std::array<bool, N> m_value{};
    std::array<bool, N> m_lastValueOnUpdate{};
    std::array<bool, N> m_threshold{}; // Not used for bool arrays
    NetworkValue_ThresholdMode m_thresholdMode = NetworkValue_ThresholdMode::DIFFERENCE;
    void (*m_onUpdateCallback)(const std::array<bool, N> &, const std::array<bool, N> &) = nullptr;

public:
    void init_feature(const std::array<bool, N> &initial, const std::array<bool, N> &threshold, NetworkValue_ThresholdMode mode, void (*cb)(const std::array<bool, N> &, const std::array<bool, N> &) = nullptr)
    {
        configureValue(initial, threshold, mode, cb);
    }

    void configureValue(const std::array<bool, N> &initial, const std::array<bool, N> &threshold, NetworkValue_ThresholdMode mode, void (*cb)(const std::array<bool, N> &, const std::array<bool, N> &) = nullptr)
    {
        m_value = initial;
        m_lastValueOnUpdate = initial;
        m_threshold = threshold; // Not used, but kept for signature compatibility
        m_thresholdMode = mode;
        m_onUpdateCallback = cb;
    }

    void clear_feature()
    {
        m_value.fill(false);
        m_lastValueOnUpdate.fill(false);
        m_threshold.fill(false);
        m_onUpdateCallback = nullptr;
        m_thresholdMode = NetworkValue_ThresholdMode::DIFFERENCE;
    }

    bool checkChanged(const std::array<bool, N> &newValue) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (newValue[i] != m_lastValueOnUpdate[i])
            {
                return true;
            }
        }
        return false;
    }

    bool applyUpdate(const std::array<bool, N> &newValue)
    {
        if (!checkChanged(newValue))
        {
            return false;
        }
        std::array<bool, N> oldValue = m_lastValueOnUpdate;
        m_value = newValue;
        m_lastValueOnUpdate = newValue;
        if (m_onUpdateCallback)
        {
            m_onUpdateCallback(oldValue, m_value);
        }
        return true;
    }

    std::array<bool, N> getValue() const { return m_value; }
    std::array<bool, N> &getValueRef() { return m_value; }

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
                     public maybe<NETWORKVALUE_ENABLE_NOTIFY, NV_Notify<T>> NETWORKVALUE_PROTOBUF_INHERITANCE
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
    void update_impl(const T &newValue, regular_type_tag, E_PRIORITY priority)
    {
        // The value has already been applied by update(), so we just send notifications.
        if (this->owner)
        {
#if (NETWORKVALUE_ENABLE_PROTOBUF == 1)
            if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_PROTOBUF))
            {
                uint8_t buffer[64]; // Static buffer for PB encoding
                pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

                MB_Registers regInfo = this->getRegisterInfo();
                if (this->encode(&stream, regInfo, newValue))
                {
                    PB_UpdateData pb_msg;
                    pb_msg.data = buffer;
                    pb_msg.len = stream.bytes_written;
                    pb_msg.componentId = this->id;
                    this->owner->onMessage(this->id, E_CALLS::EC_PROTOBUF_UPDATE, E_MessageFlags::E_MF_NONE, &pb_msg, this->owner);
                }
                else
                {
                    Log.warningln(F("  [update_impl] Protobuf encoding failed for '%s'"), this->name.c_str());
                }
            }
#endif
            if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            {
                MB_UpdateData update_msg{}; // Zero-initialize
                MB_Registers regInfo = this->getRegisterInfo();
                update_msg.address = regInfo.startAddress;
                // For single values, we point to the stable internal value.
                update_msg.userData = const_cast<void *>(static_cast<const void *>(&this->m_value));
                update_msg.count = 1;
                update_msg.value = getModbusValueHelper(newValue); // Keep for coil compatibility
                update_msg.slaveId = regInfo.slaveId;
                update_msg.functionCode = regInfo.type;
                update_msg.priority = priority;
                update_msg.componentId = this->id;
                this->owner->onMessage(this->id, E_CALLS::EC_USER, E_MessageFlags::E_MF_NONE, &update_msg, this->owner);
            }
        }
    }

    // Implementation for std::array (now sends onMessage)
    template <typename U, size_t M>
    void update_impl(const std::array<U, M> &newValue, array_type_tag, E_PRIORITY priority)
    {
        // The value has already been applied by update(), so we just send notifications.
        if (this->owner)
        {
#if (NETWORKVALUE_ENABLE_PROTOBUF == 1)
            if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_PROTOBUF))
            {
                uint8_t buffer[256]; // Larger buffer for arrays
                pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

                MB_Registers regInfo = this->getRegisterInfo();
                if (this->encode(&stream, regInfo, newValue))
                {
                    PB_UpdateData pb_msg;
                    pb_msg.data = buffer;
                    pb_msg.len = stream.bytes_written;
                    pb_msg.componentId = this->id;
                    this->owner->onMessage(this->id, E_CALLS::EC_PROTOBUF_UPDATE, E_MessageFlags::E_MF_NONE, &pb_msg, this->owner);
                }
                else
                {
                    Log.warningln(F("  [update_impl] Protobuf encoding failed for array '%s'"), this->name.c_str());
                }
            }
#endif
            if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            {
                MB_UpdateData update_msg{}; // Zero-initialize
                MB_Registers regInfo = this->getRegisterInfo();
                update_msg.address = regInfo.startAddress;
                update_msg.count = this->m_value.size();
                update_msg.userData = const_cast<void *>(static_cast<const void *>(this->m_value.data()));
                update_msg.slaveId = regInfo.slaveId;
                update_msg.functionCode = regInfo.type;
                update_msg.priority = priority;
                update_msg.componentId = this->id;
                this->owner->onMessage(this->id, E_CALLS::EC_USER, E_MessageFlags::E_MF_NONE, &update_msg, this->owner);
            }
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
                 T initial, T threshold, NetworkValue_ThresholdMode mode, void (*cb)(const T &, const T &) = nullptr,
                 uint8_t featureFlags = static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL))
        : NetworkValueBase(name, id, COMPONENT_DEFAULT, owner, featureFlags)
    {
        initNotify(initial, threshold, mode, cb);

#if (NETWORKVALUE_ENABLE_LOGGING == 1)
        if (owner && owner->hasFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG))
        {
            this->setFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG);
            if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            {
                this->initLogging(true);
            }
        }
#endif
    }

    NetworkValue(Component *owner, ushort id,
                 const char *name,
                 uint8_t featureFlags = static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL))
        : NetworkValueBase(name, id, COMPONENT_DEFAULT, owner, featureFlags)
    {
#if (NETWORKVALUE_ENABLE_LOGGING == 1)
        if (owner && owner->hasFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG))
        {
            this->setFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG);
            if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            {
                this->initLogging(true);
            }
        }
#endif
    }

    void clear()
    {
        this->flags = 0;
        this->nFlags = 0;
#if (NETWORKVALUE_ENABLE_LOGGING == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            this->NV_Logging::clear_feature();
#endif
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            this->NV_Modbus::clear_feature();
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
            this->NV_Notify<T>::clear_feature();
#if (NETWORKVALUE_ENABLE_PROTOBUF == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_PROTOBUF))
            this->NV_Protobuf::clear_feature();
#endif
    }

    short setup() override
    {
#if (NETWORKVALUE_ENABLE_LOGGING == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            this->NV_Logging::setup_feature();
#endif
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            this->NV_Modbus::setup_feature();
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
            this->NV_Notify<T>::setup_feature();
#if (NETWORKVALUE_ENABLE_PROTOBUF == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_PROTOBUF))
            this->NV_Protobuf::setup_feature();
#endif
        return E_OK;
    }

    short loop() override
    {
        Component::loop();
#if (NETWORKVALUE_ENABLE_LOGGING == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            this->NV_Logging::loop_feature();
#endif
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            this->NV_Modbus::loop_feature();
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
            this->NV_Notify<T>::loop_feature();
#if (NETWORKVALUE_ENABLE_PROTOBUF == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_PROTOBUF))
            this->NV_Protobuf::loop_feature();
#endif
        return E_OK;
    }

    short info() override
    {
        Component::info();
#if (NETWORKVALUE_ENABLE_LOGGING == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
            this->NV_Logging::info_feature();
#endif
        if (NETWORKVALUE_ENABLE_MODBUS && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS))
            this->NV_Modbus::info_feature();
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
            this->NV_Notify<T>::info_feature();
#if (NETWORKVALUE_ENABLE_PROTOBUF == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_PROTOBUF))
            this->NV_Protobuf::info_feature();
#endif
        return E_OK;
    }
    template <typename U = T>
    bool update(const U &newValue, E_PRIORITY priority = E_PRIORITY::E_PRIORITY_LOWEST)
    {
        if (!this->applyUpdate(newValue))
        {
            return false; // If no change, do nothing further.
        }

#if (NETWORKVALUE_ENABLE_LOGGING == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
        {
            // this->log(LOG_LEVEL_TRACE, this->name.c_str(), "Value is changing.");
        }
#endif

        // Dispatch notifications
        if (this->owner)
        {
            update_impl(newValue, typename get_update_tag<U>::type(), priority);
        }

        return true;
    }

    // --- Feature-specific API ---

    // Logging API
    void initLogging(bool enableLogging)
    {
#if (NETWORKVALUE_ENABLE_LOGGING == 1)
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING))
        {
            NV_Logging::init_feature(enableLogging);
        }
#endif
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
    void initNotify(T initial, T threshold, NetworkValue_ThresholdMode mode, void (*cb)(const T &, const T &) = nullptr)
    {
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
        {
            NV_Notify<T>::init_feature(initial, threshold, mode, cb);
        }
    }

    bool applyUpdate(const T &newValue)
    {
        if (NETWORKVALUE_ENABLE_NOTIFY && hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY))
        {
            return NV_Notify<T>::applyUpdate(newValue);
        }
        return false;
    }

    T getValue() const { return NV_Notify<T>::getValue(); }
    T &getValueRef() { return NV_Notify<T>::getValueRef(); }

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

#if (NETWORKVALUE_ENABLE_PROTOBUF == 1)
        // Protobuf feature is stateless, nothing to serialize to JSON.
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
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_LOGGING) && !obj["logging_enabled"].isNull())
        {
            this->enableLogging(obj["logging_enabled"].template as<bool>());
        }
#endif

#if NETWORKVALUE_ENABLE_MODBUS
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_MODBUS) && !obj["modbus"].isNull())
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
        if (hasFeature(E_NetworkValueFeatureFlags::E_NVFF_NOTIFY) && !obj["notify"].isNull())
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
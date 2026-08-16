#ifndef NETWORK_VALUE_H
#define NETWORK_VALUE_H

#include "Component.h"
#include "modbus/ModbusTypes.hh"
#include "ValueWrapper.h" // For ThresholdMode enum
#include <ArduinoLog.h>
#include <functional>
#include <type_traits> // for std::is_base_of_v

/*
 * =====================================================================================
 *                                   FEATURE DEFINITIONS
 * =====================================================================================
 * These are modular classes that provide specific functionalities (e.g., logging,
 * network exposure, value tracking). They are composed by the NetworkValue class.
 * =====================================================================================
 */

/**
 * @class Loggable
 * @brief A feature that adds logging capabilities to a NetworkValue instance.
 */
class Loggable {
protected:
    bool m_loggingEnabled = false;

public:
    void enableLogging(bool enable) {
        m_loggingEnabled = enable;
    }

    template<typename... Args>
    void log(int level, const char* componentName, const char* fmt, Args... args) const {
        if (m_loggingEnabled) {
            Log.print(level, F("[%s] "), componentName);
            Log.println(level, fmt, args...);
        }
    }

    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const {
        Log.traceln(F("  [Feature: Logging] %s"), m_loggingEnabled ? "Enabled" : "Disabled");
    }
};


/**
 * @class ModbusExposable
 * @brief A feature that makes the NetworkValue's data available over Modbus.
 */
class ModbusExposable {
protected:
    MB_Registers m_regInfo;

public:
    void configureModbus(ushort startAddress, ushort componentId, ushort slaveId, E_FN_CODE type, const char* name, const char* group) {
        m_regInfo = MB_Registers(startAddress, 1, type, MB_ACCESS_READ_WRITE, componentId, slaveId, name, group);
    }
    
    MB_Registers getRegisterInfo() const { return m_regInfo; }
    
    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const {
        Log.traceln(F("  [Feature: Modbus] Addr: %d, Name: %s"), m_regInfo.startAddress, m_regInfo.name);
    }
};


/**
 * @class ValueWrappable
 * @brief A feature that handles the core logic of wrapping and tracking a value.
 */
template<typename T>
class ValueWrappable {
protected:
    T m_value;
    T m_lastValueOnUpdate;
    T m_threshold;
    typename ValueWrapper<T>::ThresholdMode m_thresholdMode;
    std::function<void(const T&, const T&)> m_onUpdateCallback;

public:
    void configureValue(T initial, T threshold, typename ValueWrapper<T>::ThresholdMode mode, std::function<void(const T&, const T&)> cb = nullptr) {
        m_value = initial;
        m_lastValueOnUpdate = initial;
        m_threshold = threshold;
        m_thresholdMode = mode;
        m_onUpdateCallback = cb;
    }

    bool checkChanged(const T& newValue) const {
        switch (m_thresholdMode) {
            case ValueWrapper<T>::ThresholdMode::DIFFERENCE:
                return memcmp(&newValue, &m_value, sizeof(T)) != 0;
            case ValueWrapper<T>::ThresholdMode::INTERVAL_STEP:
                return abs(static_cast<long>(newValue) - static_cast<long>(m_lastValueOnUpdate)) >= static_cast<long>(m_threshold);
            default:
                return memcmp(&newValue, &m_value, sizeof(T)) != 0;
        }
    }

    void applyUpdate(const T& newValue) {
        T oldValue = m_value;
        m_value = newValue;
        m_lastValueOnUpdate = newValue;
        if (m_onUpdateCallback) {
            m_onUpdateCallback(oldValue, m_value);
        }
    }

    T getValue() const { return m_value; }

    void setup_feature() {}
    void loop_feature() {}
    void info_feature() const {
        // Can't print T generically, so we'll just confirm the feature is present.
        Log.traceln(F("  [Feature: ValueWrappable<T>] Active."));
    }
};


/*
 * =====================================================================================
 *                                  NetworkValue AGGREGATOR
 * =====================================================================================
 * This class composes the features into a single Component.
 * =====================================================================================
 */

template <typename T, typename... Features>
class NetworkValue : public Component, public Features... {
public:
    // Ensure that one of the features is ValueWrappable<T>
    static_assert((std::is_base_of_v<ValueWrappable<T>, Features> || ...), "NetworkValue must include a ValueWrappable<T> feature.");

    NetworkValue(Component* owner, ushort id, const char* name)
        : Component(name, id, COMPONENT_DEFAULT, owner)
    {
        if (owner && owner->hasFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG)) {
            this->setFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG);
            // If Loggable feature exists, enable its logging.
            if constexpr ((std::is_base_of_v<Loggable, Features> || ...)) {
                this->enableLogging(true);
            }
        }
    }

    short setup() override {
        (this->Features::setup_feature(), ...);
        return E_OK;
    }

    short loop() override {
        Component::loop();
        (this->Features::loop_feature(), ...);
        return E_OK;
    }

    void update(const T& newValue) {
        if (this->checkChanged(newValue)) {
            if constexpr ((std::is_base_of_v<Loggable, Features> || ...)) {
                // Cannot easily log old/new value of generic T
                this->log(LOG_LEVEL_TRACE, this->name.c_str(), "Value is changing.");
            }
            this->applyUpdate(newValue);
        }
    }

    // Expose getRegisterInfo if ModbusExposable is a feature
    template <typename F = ModbusExposable,
              typename = std::enable_if_t<(std::is_base_of_v<F, Features> || ...)>>
    MB_Registers getRegisterInfo() const {
        return ModbusExposable::getRegisterInfo();
    }
};

/*
 * =====================================================================================
 *                                  EXAMPLE USAGE (POC)
 * =====================================================================================
 * This demonstrates how TemperatureProfile could use the new composable NetworkValue.
 * =====================================================================================
 * In TemperatureProfile.h:
 *
 * #include "NetworkValue.h"
 *
 * // Define a type alias for the specific composition we need.
 * using StatusValue = NetworkValue<PlotStatus, 
 *                                  ValueWrappable<PlotStatus>, 
 *                                  ModbusExposable, 
 *                                  Loggable>;
 *
 * class TemperatureProfile : public PlotBase {
 * private:
 *     // ... other members
 *
 *     // Declare an instance of our composed NetworkValue
 *     StatusValue _statusValue;
 * 
 * public:
 *     TemperatureProfile(Component *owner, short slot, ushort componentId);
 * };
 * 
 * =====================================================================================
 * In TemperatureProfile.cpp constructor:
 *
 * TemperatureProfile::TemperatureProfile(Component *owner, short slot, ushort componentId) 
 *     : PlotBase(owner, componentId),
 *       // Initialize _statusValue in the member initializer list
 *       _statusValue(this, this->id, "TProf Status")
 * {
 *     // ... other constructor logic
 *
 *     // Configure the features of the NetworkValue instance
 *     _statusValue.configureValue(
 *          PlotStatus::IDLE, 
 *          PlotStatus::RUNNING, // Threshold (placeholder for DIFFERENCE mode)
 *          ValueWrapper<PlotStatus>::ThresholdMode::DIFFERENCE
 *     );
 * 
 *     _statusValue.configureModbus(
 *          mb_tcp_base_address() + static_cast<uint16_t>(TemperatureProfileRegisterOffset::STATUS),
 *          this->id,
 *          this->slaveId,
 *          E_FN_CODE::FN_READ_HOLD_REGISTER,
 *          "TProf Status",
 *          name.c_str() // group
 *     );
 *
 *     // If Loggable is part of the composition, logging is enabled by default if the
 *     // owner (TemperatureProfile) has the DEBUG flag set. We can also toggle it manually.
 *     // _statusValue.enableLogging(true);
 *
 *     // To register with Modbus manager, you'd get the info:
 *     MB_Registers statusReg = _statusValue.getRegisterInfo();
 *     // ...and add it to your array/view for the Modbus manager.
 * }
 *
 * // In TemperatureProfile::loop():
 * void TemperatureProfile::loop() {
 *     // ...
 *     PlotStatus currentStatus = getCurrentStatus();
 *     _statusValue.update(currentStatus); // This will check threshold, log, and trigger callbacks
 *     // ...
 * }
 */

#endif // NETWORK_VALUE_H 
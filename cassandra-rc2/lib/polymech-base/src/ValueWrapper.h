#ifndef VALUE_WRAPPER_H
#define VALUE_WRAPPER_H

#include <functional>
#include <cmath> // For std::abs
#include <type_traits> // For std::is_enum, std::underlying_type, std::enable_if, std::is_arithmetic

#include "Component.h"       
#include "modbus/ModbusTypes.h" 
#include "enums.h"           
#include <ArduinoLog.h>
#include <net/commons.h>

template<typename T>
class ValueWrapper {
public:
    enum class ThresholdMode {
        DIFFERENCE,      // Trigger if abs(newVal - oldVal) >= threshold
        INTERVAL_STEP    // Trigger if floor(newVal / threshold) != floor(oldVal / threshold)
    };

private:
    // Helper for threshold comparison - DIFFERENCE mode, enum types
    template<typename U = T, typename std::enable_if<std::is_enum<U>::value, int>::type = 0>
    static bool checkThresholdDifference(U newVal, U oldVal, U threshold) {
        if (static_cast<typename std::underlying_type<U>::type>(threshold) == 1) {
            return newVal != oldVal;
        } else {
            return (std::abs(static_cast<long long>(static_cast<typename std::underlying_type<U>::type>(newVal)) - static_cast<long long>(static_cast<typename std::underlying_type<U>::type>(oldVal))) >= 
                    std::abs(static_cast<long long>(static_cast<typename std::underlying_type<U>::type>(threshold))));
        }
    }

    // Helper for threshold comparison - DIFFERENCE mode, non-enum, signed types
    template<typename U = T, typename std::enable_if<!std::is_enum<U>::value && std::is_signed<U>::value, int>::type = 0>
    static bool checkThresholdDifference(U newVal, U oldVal, U threshold) {
        return std::abs(newVal - oldVal) >= threshold;
    }

    // Helper for threshold comparison - DIFFERENCE mode, non-enum, unsigned types
    template<typename U = T, typename std::enable_if<!std::is_enum<U>::value && std::is_unsigned<U>::value, int>::type = 0>
    static bool checkThresholdDifference(U newVal, U oldVal, U threshold) {
        U diff = (newVal > oldVal) ? (newVal - oldVal) : (oldVal - newVal);
        return diff >= threshold;
    }
    
    // Helper for threshold comparison - INTERVAL_STEP mode (for arithmetic types)
    template<typename U = T, typename std::enable_if<std::is_arithmetic<U>::value, int>::type = 0 >
    static bool checkThresholdIntervalStep(U newVal, U oldVal, U stepInterval) {
        if (stepInterval == 0) return false; // Avoid division by zero
        return (static_cast<long long>(newVal / stepInterval)) != (static_cast<long long>(oldVal / stepInterval));
    }
    // Fallback for non-arithmetic types with INTERVAL_STEP (should ideally not be chosen or error)
    template<typename U = T, typename std::enable_if<!std::is_arithmetic<U>::value, int>::type = 0 >
    static bool checkThresholdIntervalStep(U newVal, U oldVal, U stepInterval) {
        return false; // Or throw, or static_assert(false, ...)
    }

    // Helper to get value for Modbus message (short) - enabled for enum types
    template<typename U = T, typename std::enable_if<std::is_enum<U>::value, int>::type = 0>
    static short getModbusValueHelper(U val) {
        return static_cast<short>(static_cast<typename std::underlying_type<U>::type>(val));
    }

    // Helper to get value for Modbus message (short) - enabled for non-enum types
    template<typename U = T, typename std::enable_if<!std::is_enum<U>::value, int>::type = 0>
    static short getModbusValueHelper(U val) {
        return static_cast<short>(val);
    }

public:
    ValueWrapper()
        : m_owner(nullptr),
          m_sourceComponentId(0),
          m_getModbusBaseAddressLambda(nullptr),
          m_modbusRegisterOffset(0),
          m_notificationFunctionCode(E_FN_CODE::FN_NONE),
          m_notificationSlaveId(0),
          m_value(),
          m_threshold(),
          m_thresholdMode(ThresholdMode::DIFFERENCE),
          m_onNotifiedPostCallback(nullptr) {}

    ValueWrapper(
        Component* owner,                                           
        ushort sourceComponentId,                                   
        std::function<uint16_t()> getModbusBaseAddressLambda,       
        uint16_t modbusRegisterOffset,                              
        E_FN_CODE notificationFunctionCode,                         
        uint8_t notificationSlaveId,                                
        T initialValue,                                             
        T threshold, // For INTERVAL_STEP mode, this is the step interval
        ThresholdMode mode = ThresholdMode::DIFFERENCE,
        std::function<void(const T& newValue, const T& oldValue)> onNotifiedPostCallback = nullptr 
    )
        : m_owner(owner),
          m_sourceComponentId(sourceComponentId),
          m_getModbusBaseAddressLambda(getModbusBaseAddressLambda),
          m_modbusRegisterOffset(modbusRegisterOffset),
          m_notificationFunctionCode(notificationFunctionCode),
          m_notificationSlaveId(notificationSlaveId),
          m_value(initialValue),
          m_threshold(threshold),
          m_thresholdMode(mode),
          m_onNotifiedPostCallback(onNotifiedPostCallback) {}

    // Copy constructor
    ValueWrapper(const ValueWrapper& other)
        : m_owner(other.m_owner),
          m_sourceComponentId(other.m_sourceComponentId),
          m_getModbusBaseAddressLambda(other.m_getModbusBaseAddressLambda),
          m_modbusRegisterOffset(other.m_modbusRegisterOffset),
          m_notificationFunctionCode(other.m_notificationFunctionCode),
          m_notificationSlaveId(other.m_notificationSlaveId),
          m_value(other.m_value),
          m_threshold(other.m_threshold),
          m_thresholdMode(other.m_thresholdMode),
          m_onNotifiedPostCallback(other.m_onNotifiedPostCallback) {}

    void update(const T& newValue, E_PRIORITY priority = E_PRIORITY::E_PRIORITY_LOWEST) {
        bool thresholdExceeded = false;
        if (m_thresholdMode == ThresholdMode::DIFFERENCE) {
            thresholdExceeded = checkThresholdDifference(newValue, m_value, m_threshold);
        } else { 
            thresholdExceeded = checkThresholdIntervalStep(newValue, m_value, m_threshold);
        }
        if (thresholdExceeded) {
            T oldValue = m_value;
            m_value = newValue;
            if(!m_owner) {
                Log.errorln("ValueWrapper: changed but no owner found");
                return;
            }
            if(!m_getModbusBaseAddressLambda) {
                Log.errorln("ValueWrapper: changed but no getModbusBaseAddressLambda found");
                return;
            }

            if (m_owner && m_getModbusBaseAddressLambda) {
                MB_UpdateData update_msg;
                update_msg.address = m_getModbusBaseAddressLambda() + m_modbusRegisterOffset;
                update_msg.value = getModbusValueHelper(newValue);
                update_msg.slaveId = m_notificationSlaveId;
                update_msg.functionCode = m_notificationFunctionCode;
                update_msg.componentId = m_sourceComponentId;
                update_msg.priority = priority;
                m_owner->onMessage(m_sourceComponentId, E_CALLS::EC_USER, E_MessageFlags::E_MF_NONE, &update_msg, m_owner );
                if (m_onNotifiedPostCallback) {
                    m_onNotifiedPostCallback(newValue, oldValue);
                }
            } else {
                if (m_onNotifiedPostCallback) {
                    m_onNotifiedPostCallback(newValue, oldValue);
                }
            }
        } else {
            m_value = newValue; 
        }
    }

    T get() const {
        return m_value;
    }

    operator T() const {
        return m_value;
    }

    void setValueWithoutNotification(const T& newValue) {
        m_value = newValue;
    }

    // Public getter for the calculated Modbus address used in notifications
    uint16_t getNotificationModbusAddress() const {
        if (m_getModbusBaseAddressLambda) {
            return m_getModbusBaseAddressLambda() + m_modbusRegisterOffset;
        }
        // Return a sensible default or error indicator if lambda is null, though it shouldn't be with current usage
        // For now, returning offset itself might indicate an issue or be a raw offset if base is 0.
        // Or, more robustly, this scenario should ideally not happen if constructed correctly.
        Log.warningln("ValueWrapper (CompID: %u): getNotificationModbusAddress() called but base address lambda is null! Returning raw offset.", m_sourceComponentId);
        return m_modbusRegisterOffset; 
    }

    // Public getter for the function code used in notifications
    E_FN_CODE getNotificationFunctionCode() const {
        return m_notificationFunctionCode;
    }

private:
    Component* m_owner;
    ushort m_sourceComponentId;
    std::function<uint16_t()> m_getModbusBaseAddressLambda;
    uint16_t m_modbusRegisterOffset;
    E_FN_CODE m_notificationFunctionCode;
    uint8_t m_notificationSlaveId;

    T m_value;
    T m_threshold;
    ThresholdMode m_thresholdMode;
    std::function<void(const T& newValue, const T& oldValue)> m_onNotifiedPostCallback;
};

// Helper macro to generate the standard post-notification lambda
#define DEFAULT_VW_POST_NOTIFY_LAMBDA(VW_TYPE, VW_LOG_PREFIX_STRING, VW_LOG_OLD_VAL_EXPR, VW_LOG_NEW_VAL_EXPR) \
    [this](const VW_TYPE& newValue, const VW_TYPE& oldValue) { \
        Log.infoln("%s: %s changed from %d to %d. Post-callback.", \
                       this->name.c_str(), \
                       VW_LOG_PREFIX_STRING, \
                       (VW_LOG_OLD_VAL_EXPR), \
                       (VW_LOG_NEW_VAL_EXPR)); \
    }

// Macro to simplify ValueWrapper initialization
#define INIT_COMPONENT_VALUE_WRAPPER(VW_TYPE, VW_REG_OFFSET, VW_NOTIF_FC, VW_INITIAL_VAL, VW_THRESHOLD_VAL, VW_THRESHOLD_MODE, VW_CALLBACK) \
ValueWrapper<VW_TYPE>( \
    this->owner, \
    this->id, \
    [this]() -> uint16_t { return this->mb_tcp_base_address(); }, \
    static_cast<uint16_t>(VW_REG_OFFSET), \
    VW_NOTIF_FC, \
    static_cast<uint8_t>(this->slaveId), /* Default slave ID */ \
    VW_INITIAL_VAL, \
    VW_THRESHOLD_VAL, \
    VW_THRESHOLD_MODE, \
    VW_CALLBACK \
)

/* Conceptual Usage Example:
// In a class like TemperatureProfile:
// Constructor:
// _myWrappedValue([this](){ return this->owner; } // Lambda to get the owner
//                this->id,
//                [this](){ return this->mb_tcp_base_address(); },
//                TEMP_REG_OFFSET,
//                E_FN_CODE::FN_WRITE_HOLD_REGISTER,
//                1, // slaveId
//                INITIAL_SENSOR_VAL,
//                TEMP_THRESHOLD,
//                [this](const int& newVal, const int& oldVal){
//                    Log.infoln("Temp specific log: %d -> %d", oldVal, newVal);
//                });
//
// In loop:
// _myWrappedValue.update(readSensor());
*/

#endif // VALUE_WRAPPER_H

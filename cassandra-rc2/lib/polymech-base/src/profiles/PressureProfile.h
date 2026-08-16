#ifndef PRESSURE_PROFILE_H
#define PRESSURE_PROFILE_H

#include "./PlotBase.h"
#include <modbus/ModbusTCP.h> // Include for ModbusManager type
#include <enums.h>            // Include for error codes (like E_OK)
#include <macros.h>           // Include for LOW_WORD/HIGH_WORD (if defined there)
#include <ArduinoJson.h>
#include <Component.h>
#include <modbus/ModbusTypes.h>
#include <vector>
#include <modbus/Modbus.h>
#include "config-modbus.h"

class ModbusTCP;

#define PRESSURE_PROFILE_LOOP_INTERVAL_MS 500
#define PRESSURE_PROFILE_ID_BASE 350
#define PRESSURE_REGISTER_NAME_PREFIX "PProf"

const uint8_t PRESSURE_PROFILE_REGISTER_COUNT = static_cast<uint8_t>(PlotBaseRegisterOffset::_COUNT);

enum class PressureProfileCommand : uint8_t
{
    NONE = 0, // Or IDLE, or some other default
    START = 1,
    STOP = 2,
    PAUSE = 3,
    RESUME = 4
    // Add other commands as needed
};

/**
 * @brief Represents a pressure profile using interpolated segments.
 * Inherits from PlotBase.
 */
class PressureProfile : public PlotBase
{
public:
    PressureProfile(Component *owner, short slot, ushort componentId, ushort modbusBaseAddress);
    virtual ~PressureProfile() = default;

    short setup() override;
    short loop() override;

    /**
     * @brief Populates the profile with sample data for testing/defaults.
     * Overwrites any existing control points.
     */
    void sample();

    // --- PlotBase / Component Overrides ---
    bool load(const JsonObject &config) override;

    // --- Target Registers ---
    const std::vector<uint16_t> &getTargetRegisters() const { return _targetRegisters; }
    uint8_t getTargetRegisterCount() const { return _targetRegisters.size(); }
    uint16_t getTargetRegister(uint8_t index) const
    {
        if (index < _targetRegisters.size())
        {
            return _targetRegisters[index];
        }
        Log.errorln("PressureProfile::getTargetRegister - Index %d out of bounds (size: %d). Returning 0.", index, _targetRegisters.size());
        return 0; // Or some other indicator of an error/invalid value
    }
    void setTargetRegister(uint8_t index, uint16_t value)
    {
        if (index < _targetRegisters.size())
        {
            _targetRegisters[index] = value;
        }
        else
        {
            Log.errorln("PressureProfile::setTargetRegister - Index %d out of bounds (size: %d). Cannot set value %d.", index, _targetRegisters.size(), value);
        }
    }
    void clearTargetRegisters()
    {
        for (uint8_t i = 0; i < _targetRegisters.size(); i++)
        {
            _targetRegisters[i] = 0;
        }
    }

protected:
    short status();
    const char *getOwnPrefix() const override;
    // --- Profile Data ---
    ushort slaveId; // Modbus Slave ID

private:
    static const char *MODBUS_PREFIX;
    // Vector to hold the specific target Modbus register addresses
    std::vector<uint16_t> _targetRegisters;

    // Timestamp of the last loop execution
    uint32_t _lastLoopExecutionMs;
    uint32_t _lastLogMs; // Timestamp for logging

    PlotStatus _previousStatus;
};

#endif // PRESSURE_PROFILE_H
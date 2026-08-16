#ifndef TEMPERATURE_PROFILE_H
#define TEMPERATURE_PROFILE_H

#include "PlotBase.h"
#include <modbus/ModbusTCP.h> // Include for ModbusManager type
#include <enums.h>            // Include for error codes (like E_OK)
#include <macros.h>           // Include for LOW_WORD/HIGH_WORD (if defined there)
#include <ArduinoJson.h>
#include <Component.h>
#include <modbus/ModbusTypes.h>
#include <ValueWrapper.h>
#include <vector>
#include <map>
#include <modbus/Modbus.h>
#include "config-modbus.h"

class ModbusTCP;

#define TEMPERATURE_PROFILE_LOOP_INTERVAL_MS 500
#define TEMP_PROFILE_ID_BASE 2000
#define TEMP_REGISTER_NAME_PREFIX "TProf"
#define TEMP_MAX_LAG 1000 * 60 * 5 // 5 minutes

const uint16_t TEMP_PROFILE_REGISTER_COUNT = static_cast<uint16_t>(PlotBaseRegisterOffset::_COUNT);

enum class TemperatureProfileCommand : uint16_t
{
    NONE = 0, // Or IDLE, or some other default
    START = 1,
    STOP = 2,
    PAUSE = 3,
    RESUME = 4
    // Add other commands as needed
};

/**
 * @brief Represents a temperature profile using interpolated segments.
 * Inherits from PlotBase.
 */
class TemperatureProfile : public PlotBase
{
public:
    uint32_t getLagDuration() const { return _lagDurationMs; }
    void setLagDuration(uint32_t duration) { _lagDurationMs = duration; }
    uint32_t getLastLogMs() const { return _lastLogMs; }
    void setLastLogMs(uint32_t timestamp) { _lastLogMs = timestamp; }
    TemperatureProfile(Component *owner, short slot, ushort componentId, ushort modbusBaseAddress);
    virtual ~TemperatureProfile() = default;

    short setup() override;
    short loop() override;
    short info() override;
    // short start() override;
    void onStart() override;

    void sample();

    // void mb_tcp_register(ModbusTCP *manager) override;
    // short mb_tcp_write(MB_Registers *reg, short value) override;

    /**
     * @brief Loads temperature profile specific data (controlPoints) from JSON.
     * Called by PlotBase::loadFromJsonObject.
     */
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
        Log.errorln("TemperatureProfile::getTargetRegister - Index %d out of bounds (size: %d). Returning 0.", index, _targetRegisters.size());
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
            Log.errorln("TemperatureProfile::setTargetRegister - Index %d out of bounds (size: %d). Cannot set value %d.", index, _targetRegisters.size(), value);
        }
    }
    void clearTargetRegisters()
    {
        for (uint8_t i = 0; i < _targetRegisters.size(); i++)
        {
            _targetRegisters[i] = 0;
        }
    }

    // --- Target Offsets ---
    void setTargetOffset(uint16_t reg, int16_t offset);
    int16_t getTargetOffset(uint16_t reg) const;
    void clearTargetOffsets();

    // --- Associated Signal Plot ---
    short getSignalPlotSlotId() const { return _signalPlotSlotId; }
    void setSignalPlotSlotId(short slotId) { _signalPlotSlotId = slotId; }

    // --- Associated Pressure Profile ---
    short getPressureProfileSlotId() const { return _pressureProfileSlotId; }
    void setPressureProfileSlotId(short slotId) { _pressureProfileSlotId = slotId; }

    // void updateOmronSetpoints(PHApp* app, uint16_t value);
    void resolveLinkedProfiles();

protected:
    const char *getOwnPrefix() const override;

    // --- Profile Data ---
    ushort slaveId; // Modbus Slave ID

private:
    static const char *MODBUS_PREFIX;
    // Vector to hold the specific target Modbus register addresses
    std::vector<uint16_t> _targetRegisters;

    struct TargetOffset
    {
        uint16_t targetReg;
        int16_t offset;
    };
    std::vector<TargetOffset> _targetOffsets;

    // Modbus block definitions are now managed by NetworkComponent base class

    // Timestamp of the last loop execution
    uint32_t _lastLoopExecutionMs;
    uint32_t _lastLogMs; // Timestamp for logging

    // Associated Signal Plot Slot ID
    short _signalPlotSlotId;

    // Associated Pressure Profile Slot ID
    short _pressureProfileSlotId;

    PlotStatus _previousStatus;

    // Cached pointer to OmronE5 device for lag compensation
    Component *_omron = nullptr;

    // Track how long we have been slipping due to lag
    uint32_t _lagDurationMs = 0;
};

#endif // TEMPERATURE_PROFILE_H
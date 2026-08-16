#ifndef TEMPERATURE_PROFILE_H
#define TEMPERATURE_PROFILE_H

#include "PlotBase.h"
#include <modbus/ModbusTCP.h> // Include for ModbusManager type
#include <enums.h>         // Include for error codes (like E_OK)
#include <macros.h>        // Include for LOW_WORD/HIGH_WORD (if defined there)
#include <ArduinoJson.h>
#include <Component.h>
#include <modbus/ModbusTypes.h>
#include <ValueWrapper.h>
#include <vector>
#include <modbus/Modbus.h>

class ModbusTCP;

#define TEMP_PROFILE_ID_BASE 2000
#define REGISTER_NAME_PREFIX "TProf"

enum class TemperatureProfileRegisterOffset : uint16_t {
    STATUS,
    CURRENT_TEMP,
    DURATION,
    ELAPSED_LW,
    ELAPSED_HW,
    REMAINING,
    COMMAND,
    ENABLE_CMD,
    _COUNT
};

enum class TemperatureProfileCommand : uint16_t {
    NONE = 0, // Or IDLE, or some other default
    START = 1,
    STOP = 2,
    PAUSE = 3,
    RESUME = 4
    // Add other commands as needed
};

// Calculate the number of registers per profile instance based on the enum
const uint16_t TEMP_PROFILE_REGISTER_COUNT = static_cast<uint16_t>(TemperatureProfileRegisterOffset::_COUNT);

// Define max size for control point data array
#define MAX_TEMP_CONTROL_POINTS 10 // Adjust as needed

enum class TempProfileControlType : uint8_t {
    LINEAR = 0,
    CUBIC = 1
};

struct TempControlPoint {
    int16_t x;       // Time proportion (scaled 0-PROFILE_SCALE)
    int16_t y;       // Temperature value (scaled 0-PROFILE_SCALE)    
};

/**
 * @brief Represents a temperature profile using interpolated segments.
 * Inherits from PlotBase.
 */
class TemperatureProfile : public PlotBase {
public:
    TemperatureProfile(Component* owner, short slot, ushort componentId);
    virtual ~TemperatureProfile() = default;

    short setup() override;
    short loop() override;
    void start() override;

    void enable();
    void disable();

    // --- Profile Specific Methods --- 
    /**
     * @brief Gets the interpolated temperature value for the current time.
     * @param elapsedMs The elapsed time in milliseconds.
     * @return The interpolated temperature (scaled 0-PROFILE_SCALE) or 0 if not running/invalid.
     */
    int16_t getTemperature(uint32_t elapsedMs) const;

    /**
     * @brief Gets a pointer to the internal array of control points.
     * @return Const pointer to the TempControlPoint array.
     */
    const TempControlPoint* getTempControlPoints() const;

    /**
     * @brief Gets the number of currently defined control points.
     * @return Number of control points.
     */
    uint8_t getNumTempControlPoints() const;

    // --- TemperatureProfile Max Temperature ---
    ushort max;

    /**
     * @brief Populates the profile with sample data for testing/defaults.
     * Overwrites any existing control points.
     */
    void sample();

    /**
     * @brief Sets the control points for the temperature profile.
     *
     * @param points An array of TempControlPoint structures.
     * @param numPoints The number of points in the array.
     * @param durationMs The total duration of the profile in milliseconds.
     * @return True if the points were set successfully, false otherwise (e.g., invalid number of points).
     */
    bool setControlPoints(const TempControlPoint points[], uint8_t numPoints, uint32_t durationMs);

    // --- PlotBase / Component Overrides ---
    bool getCurrentControlPointInfo(uint8_t& outId, uint32_t& outTimeMs, int16_t& outValue, int16_t& outUser) const override;
    void mb_tcp_register(ModbusTCP* manager) override;
    ModbusBlockView* mb_tcp_blocks() const override;
    short mb_tcp_read(MB_Registers *reg) override;
    short mb_tcp_write(MB_Registers *reg, short value) override;
    short serial_register(Bridge *bridge) override;
    uint16_t mb_tcp_base_address() const override;    /**
     * @brief Loads temperature profile specific data (controlPoints) from JSON.
     * Called by PlotBase::loadFromJsonObject.
     */
    bool load(const JsonObject& config) override;
    
    // --- Target Registers ---
    const std::vector<uint16_t>& getTargetRegisters() const { return _targetRegisters; }
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

    // --- Associated Signal Plot ---
    short getSignalPlotSlotId() const { return _signalPlotSlotId; }
    void setSignalPlotSlotId(short slotId) { _signalPlotSlotId = slotId; }

protected:
    
    short status();
    // --- TemperatureProfile Slot ---
    ushort slot;
    // --- Profile Data ---
    ushort slaveId; // Modbus Slave ID

private:
    // Vector to hold the specific target Modbus register addresses
    std::vector<uint16_t> _targetRegisters;

    MB_WVAR_H(
        _statusWrapper, PlotStatus,
        TemperatureProfileRegisterOffset, STATUS, E_FN_CODE::FN_READ_HOLD_REGISTER,
        REGISTER_NAME_PREFIX,
        PlotStatus::IDLE, PlotStatus::RUNNING, ValueWrapper<PlotStatus>::ThresholdMode::DIFFERENCE
    );

    ValueWrapper<int16_t> _currentTemperatureWrapper;
    ValueWrapper<bool> _enabledStateWrapper;
    ValueWrapper<uint32_t> _elapsedTimeWrapper;
    ValueWrapper<uint16_t> _elapsedTimeHwWrapper;

    // Modbus block definitions (instance-specific)
    MB_Registers _modbusBlocks[TEMP_PROFILE_REGISTER_COUNT];
    ModbusBlockView _modbusBlockView;

    TempControlPoint _controlPoints[MAX_TEMP_CONTROL_POINTS];
    uint8_t _numControlPoints;

    // Pointer to the Modbus manager (set during registration)
    ModbusTCP* modbusTCP;

    // Timestamp of the last loop execution
    uint32_t _lastLoopExecutionMs;
    uint32_t _lastLogMs; // Timestamp for logging

    // Associated Signal Plot Slot ID
    short _signalPlotSlotId;

    // Helper methods for interpolation
    int16_t lerp(int16_t y0, int16_t y1, uint16_t t) const;
    int16_t cubicBezier(int16_t y0, int16_t y1, int16_t y2, int16_t y3, uint16_t t_norm) const;
    int16_t cubicBezierInt(int16_t y0, int16_t y1, int16_t y2, int16_t y3, uint16_t t_norm) const;
    void _initializeControlPoints();
};

#endif // TEMPERATURE_PROFILE_H 
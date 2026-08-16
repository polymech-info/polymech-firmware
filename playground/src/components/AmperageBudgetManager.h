#ifndef AMPERAGE_BUDGET_MANAGER_H
#define AMPERAGE_BUDGET_MANAGER_H

#include <Component.h>
#include <ArduinoLog.h>

#include "components/OmronE5.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "config-modbus.h"
#include "config.h"

// Default values
#define MIN_HEATING_DURATION_MS (3 * 1000)    // 3 seconds
#define MAX_HEATING_DURATION_MS (5 * 1000)    // 5 seconds
#define DEFAULT_MAX_SIMULTANEOUS_HEATING 2    // Default number of devices that can heat simultaneously
#define DEFAULT_WINDOW_OFFSET 1               // Default window offset
#define DEFAULT_START_INDEX 0                 // Default start index for cycling
#define DEFAULT_END_INDEX (MAX_MANAGED_DEVICES - 1) // Default end index for cycling

// Modbus write boundaries
#define MB_MAX_TIME_MIN_MS 500                // Minimum max time: 500ms
#define MB_MAX_TIME_MAX_MS (20 * 60 * 1000)   // Maximum max time: 20 minutes

class AmperageBudgetManager : public Component
{
public:
    AmperageBudgetManager(Component *owner);
    virtual ~AmperageBudgetManager() = default;

    bool addManagedDevice(OmronE5 *device);
    virtual short setup() override;
    virtual short loop() override;
    virtual short info(short val0, short val1) override;

    // Max simultaneous heating control
    uint8_t getMaxSimultaneousHeating() const { return _maxSimultaneousHeating; }
    void setMaxSimultaneousHeating(uint8_t value) { 
        if (value > 0 && value <= MAX_MANAGED_DEVICES) {
            _maxSimultaneousHeating = value;
            Log.infoln(F("[%s] Max simultaneous heating set to %d"), _name, value);
        }
    }

    // Modbus interface
    virtual short mb_tcp_write(MB_Registers *reg, short value) override;
    virtual short mb_tcp_read(MB_Registers *reg) override;
    virtual void mb_tcp_register(ModbusTCP* manager) override;
    virtual ModbusBlockView* mb_tcp_blocks() const override;
    virtual short serial_register(Bridge *bridge) override;

    // Persistence methods
    void toJson(JsonDocument& doc) const;
    bool fromJson(const JsonObject& json);
    bool load(const char* path);
    bool save(const char* path) const;
    void print() const;

protected:
    virtual void notifyStateChange() override;  // Override to handle device stopping

private:
    static const uint8_t MAX_MANAGED_DEVICES = NUM_OMRON_DEVICES;
    OmronE5* _devices[MAX_MANAGED_DEVICES];
    uint8_t _numDevices;
    uint8_t _currentIndex;  // Current device index in the round-robin

    // Configurable parameters
    uint32_t _minHeatingDurationMs;
    uint32_t _maxHeatingDurationMs;
    uint8_t _maxSimultaneousHeating;
    uint8_t _windowOffset;

    uint8_t _startIndex; // Index of the first device in the round-robin cycle
    uint8_t _endIndex;   // Index of the last device in the round-robin cycle

    uint32_t _deviceStartTimes[MAX_MANAGED_DEVICES];
    bool _deviceHeating[MAX_MANAGED_DEVICES];
    String _name;

    bool _checkHeatup(OmronE5* device);
    void _updateDevice(uint8_t deviceIndex, uint32_t currentTime);
    bool _canStartHeating(uint8_t deviceIndex) const;
    void _stopAllDevices();
    void _resetBudgetState();

    // Modbus blocks
    MB_Registers m_modbus_block_enable;
    MB_Registers m_modbus_block_info;
    MB_Registers m_modbus_block_min_time;
    MB_Registers m_modbus_block_max_time;
    MB_Registers m_modbus_block_max_sim;
    MB_Registers m_modbus_block_offset;
    MB_Registers m_modbus_block_start_index;
    MB_Registers m_modbus_block_end_index;
    mutable ModbusBlockView m_modbus_view;

    // Validation methods
    bool _validateMaxTime(short value) const { 
        return value >= MB_MAX_TIME_MIN_MS && value <= MB_MAX_TIME_MAX_MS; 
    }
    bool _validateMinTime(short value) const { 
        return value >= 1000 && value <= 60000;  // 1s to 60s
    }
    bool _validateMaxSim(short value) const { 
        return value >= 1 && value <= MAX_MANAGED_DEVICES; 
    }
    bool _validateOffset(short value) const { 
        return value >= 1 && value <= MAX_MANAGED_DEVICES; 
    }
    bool _validateStartIndex(short value) const {
        return value >= 0 && value < MAX_MANAGED_DEVICES && value <= _endIndex;
    }
    bool _validateEndIndex(short value) const {
        return value >= 0 && value < MAX_MANAGED_DEVICES && value >= _startIndex;
    }

    // Helper methods
    void _enableComponent() {
        enable();
        _currentIndex = _startIndex;  // Reset to start index when enabling
        Log.infoln(F("[%s] Enabled and reset to start index %d."), _name, _startIndex);
    }

    void _disableComponent() {
        _stopAllDevices();  // Stop all devices before disabling
        disable();
        Log.infoln(F("[%s] Disabled and stopped all devices."), _name);
    }
};

#endif
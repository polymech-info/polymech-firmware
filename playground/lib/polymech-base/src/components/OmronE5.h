#ifndef OMRON_E5_H
#define OMRON_E5_H

#include "config.h"

#ifdef ENABLE_RS485 

#include <ArduinoLog.h>
#include <Component.h>
#include <modbus/ModbusRTU.h>
#include <modbus/ModbusTypes.h> 
#include "components/OmronE5Types.h"
#include <xstatistics.h>
#include "../ValueWrapper.h"

#include <NetworkValue.h>
using BoolNetValue = NetworkValue<bool>;

#define ENABLE_TRUTH_COLLECTOR

#ifdef ENABLE_TRUTH_COLLECTOR
#include <xstatistics.h>
#endif

#define OMRON_E5_READ_BLOCK_START_ADDR 0x0000
#define OMRON_E5_READ_BLOCK_REG_COUNT  6
#define OMRON_E5_READ_BLOCK_INTERVAL 300

enum class E_ExecuteCommands : short {
    INFO = 1,
    RESET_STATS = 2
};


enum class E_OmronTcpOffset : ushort {
    PV = 1,
    STATUS_HIGH = 2,
    STATUS_LOW = 3,
#ifdef ENABLE_TRUTH_COLLECTOR
    MEAN_ERROR = 4,
    SP = 5,
    HEAT_RATE = 6,
    WH_LOW = 7,
    WH_HIGH = 8,
    PV_SP_LAG = 9, // Tenths of PV Unit per minute (PV SP Lag)
    TOTAL_COST_CENTS = 10, // Estimated total cost in Euro Cents
    LONGEST_HEAT_60S = 15, // Longest heating duration in last 60s (seconds)
#else
    SP = 5,
#endif
    CMD_SP = 11,
    CMD_STOP = 12,
    CMD_EXECUTE = 13, // Write 1 for info, 2 for reset stats
    IS_HEATING = 14,      // Read-only status: 1 if heating, 0 otherwise
    IS_HEATUP = 16,        // Read-only status: 1 if PV < SP - Deadband
    CMD_EXECUTE_INFO = 17
};

class OmronE5 : public RTU_Base
{
public:
    // Calculate the number of TCP blocks based on conditional compilation
    #ifdef ENABLE_TRUTH_COLLECTOR
        static constexpr int OMRON_TCP_BLOCK_COUNT = 16;
    #else
        #ifdef ENABLE_COOLING // Added check for cooling when truth collector is off
            static constexpr int OMRON_TCP_BLOCK_COUNT = 8; // Basic + CMDs + Heatup + CoolingMV
        #else
            static constexpr int OMRON_TCP_BLOCK_COUNT = 7; // Basic + CMDs + Heatup
        #endif
    #endif

    // Constructor
    OmronE5(Component* owner, uint8_t slaveId, millis_t readInterval = OMRON_E5_READ_BLOCK_INTERVAL);
    virtual ~OmronE5() = default;

    // --- Component Interface ---
    virtual short setup() override;
    virtual short loop() override;
    virtual short info() override;

    // --- Modbus Register Update Notification ---
    virtual void onRegisterUpdate(uint16_t address, uint16_t newValue) override; // Override base method

    // --- Getters for Specific Values ---
    bool getPV(uint16_t& value) const;
    bool getSP(uint16_t& value) const;
    bool getStatusLow(uint16_t& value) const;
    bool getStatusHigh(uint16_t& value) const;
    bool isRunning() const;
    bool isHeating() const;
    bool isCooling() const;
    bool isAutoTuning() const;
    bool isHeatup() const;
#ifdef ENABLE_TRUTH_COLLECTOR
    float getMeanError() const;
    float getHeatRate() const;
    float getTotalWh() const;
    uint16_t getLongestHeatDuration60s() const; // Getter returns seconds
#endif
    // --- Setters (Optional - Implement if needed) ---
    bool setSP(uint16_t value);
    bool run();
    bool stop();
    uint32_t getConsumption() const;

#ifdef ENABLE_COOLING
    // --- Cooling Specific Getters --- 
    // Requires ENABLE_COOLING define in OmronE5.cpp and manual Omron device setup for cooling.
    /**
     * @brief Gets the raw 32-bit value of the Cooling Manipulated Variable (MV) monitor.
     * Requires reading registers 0x2005 (Low) and 0x2006 (High).
     * @param value Reference to store the combined 32-bit value.
     * @return True if the value is valid (has been successfully read), false otherwise.
     */
    bool getCoolingMVRaw(uint32_t& value) const;
    // --- Cooling State (Requires ENABLE_COOLING in .cpp & manual device setup) ---
    uint16_t _currentCoolingMVLow = 0;
    uint16_t _currentCoolingMVHigh = 0;
    bool _coolingMvValid = false;
#endif

    // --- Modbus Block Definitions ---
    virtual ModbusBlockView* mb_tcp_blocks() const override;
    virtual short mb_tcp_read(MB_Registers * reg) override;
    virtual short mb_tcp_write(MB_Registers * reg, short value) override;

    // --- Modbus TCP Mapping Overrides ---
    /**
     * @brief Gets the base Modbus TCP address allocated for this RTU device instance.
     * @return The base TCP address for this device instance.
     */
    uint16_t mb_tcp_base_address() const override;

    /**
     * @brief Calculates the Modbus TCP offset corresponding to a given RTU address update.
     * @param rtuAddress The RTU register address that was updated.
     * @return The corresponding TCP offset (relative to mb_tcp_base_address), or 0 if no direct mapping exists for broadcast.
     */
    uint16_t mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const override;

private:
    millis_t _readInterval;

    // --- Local State Storage ---
    uint16_t _currentPV = 0;
    uint16_t _currentSP = 0;
    uint16_t _currentStatusLow = 0;
    uint16_t _currentStatusHigh = 0;
    bool _pvValid = false;
    bool _spValid = false;
    bool _statusValid = false; // Combined flag for low/high status pair
    uint32_t _consumption = 2700; // Added consumption member (Watts) with default value
    ValueWrapper<bool> _runStateWrapper; // Wrapper for Run/Stop state

#ifdef ENABLE_TRUTH_COLLECTOR
    Statistic _errorStats;
    Statistic _heatingIntervalStats;
    bool _wasHeating = false;
    millis_t _heatOnStartTime = 0;
    float _totalWh = 0.0f;

    // PV Rate of Change Calculation (PV SP Lag)
    uint16_t _previousPV = 0;
    millis_t _lastPvUpdateTime = 0;
    int16_t _pvSpLag = 0; // Tenths of PV Unit per Minute (PV SP Lag)
    bool _hasPreviousPv = false;

    millis_t _lastHeatingLoopTime = 0; // Timestamp for continuous Wh calculation

    // Longest Heating Duration Tracking (60s window)
    millis_t _currentHeatStartTime = 0; // Start time of the current continuous heating period
    millis_t _windowStartTime = 0; // Start time of the 60-second tracking window
    uint16_t _maxHeatDurationInWindowSecs = 0; // Longest duration (seconds) found in the current window
    /**
     * @brief Resets all runtime statistics and timestamps used by the truth collector.
     * Triggered by writing 2 to the CMD_EXECUTE register.
     */
    void _resetRuntimeStats();
#endif

    // NOTE: Size updated to match OMRON_TCP_BLOCK_COUNT
    MB_Registers _modbusBlocks[OMRON_TCP_BLOCK_COUNT];
    ModbusBlockView _modbusBlockView;

    BoolNetValue _runStateValue;
};

#endif // ENABLE_RS485
#endif // OMRON_E5_H 
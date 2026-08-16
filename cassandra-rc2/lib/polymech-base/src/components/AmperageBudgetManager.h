#ifndef AMPERAGE_BUDGET_MANAGER_H
#define AMPERAGE_BUDGET_MANAGER_H

#include <Component.h>
#include <ArduinoLog.h>
#include <vector>
#include <algorithm>
#include <enums.h>

#include "components/OmronE5.h"
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "config-modbus.h"
#include "config.h"

// Forward declaration
class OmronE5;

// Default values
#define MIN_HEATING_DURATION_S 3                    // 3 seconds
#define MAX_HEATING_DURATION_S 5                    // 5 seconds
#define DEFAULT_MAX_SIMULTANEOUS_HEATING 2          // Default number of devices that can heat simultaneously
#define DEFAULT_WINDOW_OFFSET 1                     // Default window offset
#define DEFAULT_START_INDEX 0                       // Default start index for cycling
#define DEFAULT_END_INDEX (MAX_MANAGED_DEVICES - 1) // Default end index for cycling

#define STOP_ALL_DEVICES_WAIT_MS 50 // Wait time after stopping all devices

// Modbus write boundaries
#define MB_MAX_TIME_MIN_S 1          // Minimum max time: 1s
#define MB_MAX_TIME_MAX_S (120 * 60) // Maximum max time: 2 hours

#define MB_MIN_TIME_MIN_S 1  // Minimum min time: 1s
#define MB_MIN_TIME_MAX_S 60 // Maximum min time: 60s

#define AMP_BUDGET_MB_COUNT 11 // m_enabled + 10 custom values

enum E_AMPERAGE_MODE
{
    E_AM_CYCLE_ALL,           // Cycle through all devices
    E_AM_CYCLE_SP,            // Cycle through partitions (REG_OFFSET_MAX_SIM), advance when SP - DEADBAND is reached
    E_AM_CYCLE_SP_ANY,        // Heat any N devices that require it
    E_AM_CYCLE_SP_MOST_URGENT // Heat most urgent N devices, within window
};

enum E_OP_FLAGS : uint16_t
{
    E_SQ_NONE = 0,
    E_SQ_RUNNING_PROFILES_ONLY = 1 << 0,
    E_SQ_VERBOSE = 1 << 1,
    E_SQ_USER = 1 << 2,
};

class AmperageBudgetManager : public NetworkComponent<AMP_BUDGET_MB_COUNT>
{
public:
    typedef bool (*CanUsePIDCallback)(Component *owner, OmronE5 *device);
    typedef void (*OnWarmupCompleteCallback)(Component *owner);

    enum E_MB_OFFSETS
    {
        REG_OFFSET_INFO = E_NVC_USER,
        REG_OFFSET_MIN_TIME,
        REG_OFFSET_MAX_TIME,
        REG_OFFSET_MAX_SIM,
        REG_OFFSET_OFFSET,
        REG_OFFSET_START_INDEX,
        REG_OFFSET_END_INDEX,
        REG_OFFSET_MODE,
        REG_OFFSET_OP_FLAGS
    };

    AmperageBudgetManager(Component *owner, uint16_t baseAddress = MB_ADDR_AMPERAGE_BUDGET_BASE);
    virtual ~AmperageBudgetManager() = default;

    void setCanUseCallback(CanUsePIDCallback cb)
    {
        _canUseCallback = cb;
    }

    void setOnWarmupCompleteCallback(OnWarmupCompleteCallback cb)
    {
        _onWarmupCompleteCallback = cb;
    }

    bool addManagedDevice(OmronE5 *device);
    virtual short setup() override;
    virtual short loop() override;
    virtual short info(short val0, short val1) override;
    virtual short reset() override;

    virtual void onCycleStart(const std::vector<OmronE5 *> &activeDevices);
    virtual void onCycleEnd(const std::vector<OmronE5 *> &activeDevices);
    virtual void onHeatupComplete();

    struct DeviceStatePacket
    {
        uint8_t index;
        uint8_t enabled;
        uint8_t heating;
        uint8_t heatup;
        uint32_t elapsed;
    } __attribute__((packed));

    struct StatePacket
    {
        uint32_t timestamp;
        uint8_t currentIdx;
        uint8_t numDevices;
        uint8_t heatupComplete;
        DeviceStatePacket devices[NUM_OMRON_DEVICES];
    } __attribute__((packed));

    size_t getBinaryState(uint8_t *buffer, size_t maxLen);

    // Max simultaneous heating control
    uint8_t getMaxSimultaneousHeating() const { return m_maxSimultaneousHeating.getValue(); }
    void setMaxSimultaneousHeating(uint8_t value)
    {
        if (value > 0 && value <= MAX_MANAGED_DEVICES)
        {
            m_maxSimultaneousHeating.update(value);
        }
    }

    // Modbus interface
    virtual short mb_tcp_write(MB_Registers *reg, short value) override;
    virtual short mb_tcp_read(MB_Registers *reg) override;
    virtual short serial_register(Bridge *bridge) override;

    // Persistence methods
    void toJson(JsonDocument &doc) const;
    bool fromJson(const JsonObject &json);
    bool load(const char *path);
    bool save(const char *path) const;
    void print() const;

protected:
    virtual void notifyStateChange() override; // Override to handle device stopping

private:
    CanUsePIDCallback _canUseCallback;
    OnWarmupCompleteCallback _onWarmupCompleteCallback;
    static const uint8_t MAX_MANAGED_DEVICES = NUM_OMRON_DEVICES;
    OmronE5 *_devices[MAX_MANAGED_DEVICES];
    uint8_t _numDevices;
    uint8_t _currentIndex; // Current device index in the round-robin

    // Configurable parameters
    NetworkValue<uint16_t> m_minHeatingDurationS;
    NetworkValue<uint16_t> m_maxHeatingDurationS;
    NetworkValue<uint8_t> m_maxSimultaneousHeating;
    NetworkValue<uint8_t> m_windowOffset;
    NetworkValue<E_AMPERAGE_MODE> m_mode;
    NetworkValue<uint8_t> m_startIndex;
    NetworkValue<uint8_t> m_endIndex;
    NetworkValue<uint16_t> m_opFlags;

    E_AMPERAGE_MODE _initialMode;

    uint32_t _deviceStartTimes[MAX_MANAGED_DEVICES];
    bool _deviceHeating[MAX_MANAGED_DEVICES];
    bool _deviceInHeatup[MAX_MANAGED_DEVICES];
    bool _heatupPhaseComplete;
    String _name;

    millis_t _lastLoopTime;
    millis_t _lastDebugLogTime;
    uint32_t _lastStopTimeLog[MAX_MANAGED_DEVICES];
    std::vector<OmronE5 *> _activeDevices;

    bool _checkHeatup(OmronE5 *device);
    void _stopAllDevices();
    int16_t _stoppingIndex;
    uint32_t _lastStopTimestamp;
    void _stopDevice(uint8_t deviceIndex, const char *reason);
    void _checkAllDevicesForHeatupCompletion();

    void _loopCycleAll();
    void _loopCycleSp();
    void _loopCycleSpAny();
    void _loopCycleSpMostUrgent();

    // Batch mode debounce
    uint16_t _batchDoneConfirmationCount;
    static const uint16_t BATCH_DONE_THRESHOLD = 20; // ~1.2s at 60ms loop

    // Validation methods
    bool _validateMaxTime(short value) const
    {
        return value >= MB_MAX_TIME_MIN_S && value <= MB_MAX_TIME_MAX_S;
    }
    bool _validateMinTime(short value) const
    {
        return value >= MB_MIN_TIME_MIN_S && value <= MB_MIN_TIME_MAX_S;
    }

    bool _validateMaxSim(short value) const
    {
        return value >= 1 && value <= MAX_MANAGED_DEVICES;
    }
    bool _validateOffset(short value) const
    {
        return value >= 1 && value <= MAX_MANAGED_DEVICES;
    }
    bool _validateStartIndex(short value) const
    {
        return value >= 0 && value < MAX_MANAGED_DEVICES && value <= m_endIndex.getValue();
    }
    bool _validateEndIndex(short value) const
    {
        return value >= 0 && value < MAX_MANAGED_DEVICES && value >= m_startIndex.getValue();
    }
};

#endif
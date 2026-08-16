#include "config.h"

#ifdef ENABLE_AMPERAGE_BUDGET_MANAGER
#include <vector>
#include <pid_constants.h>
#include <Bridge.h>
#include <modbus/ModbusTCP.h>
#include <json.h>
#include <functional>

#include "components/AmperageBudgetManager.h"

using namespace JsonUtils;

#define SEQ_LOOP_INTERVAL_MS 60
#define REVERT_TO_HEATUP_DEADBAND 15

short AmperageBudgetManager::reset()
{
    _stopAllDevices();
    _currentIndex = m_startIndex.getValue();
    _heatupPhaseComplete = false;
    for (uint8_t i = 0; i < MAX_MANAGED_DEVICES; ++i)
    {
        _deviceInHeatup[i] = false;
    }
    _batchDoneConfirmationCount = 0;
    return E_OK;
}

void AmperageBudgetManager::_stopAllDevices()
{
    if (_stoppingIndex == -1)
    {
        _stoppingIndex = 0;
        _lastStopTimestamp = 0; // Force immediate start
    }
}

AmperageBudgetManager::AmperageBudgetManager(Component *owner, uint16_t baseAddress)
    : NetworkComponent(baseAddress, "AmperageBudgetManager", COMPONENT_KEY_AMPERAGE_BUDGET_MANAGER, Component::COMPONENT_DEFAULT, owner),
      _numDevices(0),
      _currentIndex(0),
      m_minHeatingDurationS(this, this->id, "MinHeatingDurationS"),
      m_maxHeatingDurationS(this, this->id, "MaxHeatingDurationS"),
      m_maxSimultaneousHeating(this, this->id, "MaxSimultaneousHeating"),
      m_windowOffset(this, this->id, "WindowOffset"),
      m_mode(this, this->id, "Mode(0:Cycle All,1:Cycle SP,2:Any SP,3:Most Urgent (Recommended))"),
      m_startIndex(this, this->id, "StartIndex"),
      m_endIndex(this, this->id, "EndIndex"),
      m_opFlags(this, this->id, "OpFlags"),
      _lastLoopTime(0),
      _initialMode(E_AM_CYCLE_SP_MOST_URGENT),
      _heatupPhaseComplete(false),
      _canUseCallback(nullptr),
      _stoppingIndex(-1),
      _lastStopTimestamp(0),
      _batchDoneConfirmationCount(0)
{
    pFlags = E_PersistenceFlags::E_PF_ENABLED;
    for (uint8_t i = 0; i < MAX_MANAGED_DEVICES; ++i)
    {
        _devices[i] = nullptr;
        _deviceStartTimes[i] = 0;
        _deviceHeating[i] = false;
        _deviceInHeatup[i] = false;
    }
}

bool AmperageBudgetManager::addManagedDevice(OmronE5 *device)
{
    if (_numDevices >= MAX_MANAGED_DEVICES)
    {
        L_ERROR(F("[%s] Cannot add more devices, manager full (%d)."), _name.c_str(), MAX_MANAGED_DEVICES);
        return false;
    }
    if (device == nullptr)
    {
        L_ERROR(F("[%s] Cannot add null device pointer."), _name.c_str());
        return false;
    }

    for (uint8_t i = 0; i < _numDevices; ++i)
    {
        if (_devices[i] == device)
        {
            Log.warningln(F("[%s] Device already added (Index %d). Ignoring."), _name.c_str(), i);
            return true;
        }
    }

    _devices[_numDevices++] = device;
    Log.traceln(F("[%s] Added device %d."), _name.c_str(), _numDevices - 1);
    return true;
}

short AmperageBudgetManager::setup()
{
    NetworkComponent::setup();

    const uint16_t baseAddr = mb_tcp_base_address();

    m_minHeatingDurationS.initNotify(MIN_HEATING_DURATION_S, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_minHeatingDurationS.initModbus(baseAddr + REG_OFFSET_MIN_TIME, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "MinTime", this->name.c_str());
    registerBlock(m_minHeatingDurationS.getRegisterInfo());

    m_maxHeatingDurationS.initNotify(MAX_HEATING_DURATION_S, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_maxHeatingDurationS.initModbus(baseAddr + REG_OFFSET_MAX_TIME, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "MaxTime", this->name.c_str());
    registerBlock(m_maxHeatingDurationS.getRegisterInfo());

    m_maxSimultaneousHeating.initNotify(DEFAULT_MAX_SIMULTANEOUS_HEATING, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_maxSimultaneousHeating.initModbus(baseAddr + REG_OFFSET_MAX_SIM, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "MaxSim", this->name.c_str());
    registerBlock(m_maxSimultaneousHeating.getRegisterInfo());

    m_windowOffset.initNotify(DEFAULT_WINDOW_OFFSET, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_windowOffset.initModbus(baseAddr + REG_OFFSET_OFFSET, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "Offset", this->name.c_str());
    registerBlock(m_windowOffset.getRegisterInfo());

    m_mode.initNotify(E_AM_CYCLE_ALL, (E_AMPERAGE_MODE)1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_mode.initModbus(baseAddr + REG_OFFSET_MODE, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "Mode(0:Cycle All,1:Cycle SP,2:Any SP,3:Most Urgent)", this->name.c_str());
    registerBlock(m_mode.getRegisterInfo());

    m_startIndex.initNotify(DEFAULT_START_INDEX, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_startIndex.initModbus(baseAddr + REG_OFFSET_START_INDEX, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "StartIndex", this->name.c_str());
    registerBlock(m_startIndex.getRegisterInfo());

    m_endIndex.initNotify(DEFAULT_END_INDEX, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_endIndex.initModbus(baseAddr + REG_OFFSET_END_INDEX, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "EndIndex", this->name.c_str());
    registerBlock(m_endIndex.getRegisterInfo());

    m_opFlags.initNotify(E_SQ_NONE, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_opFlags.initModbus(baseAddr + REG_OFFSET_OP_FLAGS, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "OpFlags", this->name.c_str());
    registerBlock(m_opFlags.getRegisterInfo());

    MB_Registers infoReg(baseAddr + REG_OFFSET_INFO, 1, FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY, this->id, this->slaveId, "Info", this->name.c_str());
    registerBlock(infoReg);

    _stopAllDevices(); // Ensure all devices are stopped during setup
    _activeDevices.reserve(MAX_MANAGED_DEVICES);

    _currentIndex = m_startIndex.getValue(); // Initialize current index to start index
    // Try to load configuration
    if (!load("/amperage_budget.json"))
    {
        L_WARN(F("[%s] Using default configuration"), _name.c_str());
    }
    disable();
    return E_OK;
}

bool AmperageBudgetManager::_checkHeatup(OmronE5 *device)
{
    if (!device->enabled())
    {
        return false;
    }
    // Fix: hasError() is cumulative. We only want to stop if the device is truly unreachable (Timeout).
    // Other errors (CRC, Collision) are transient and should be ignored to prevent premature abortion.
    if (device->getLastErrorCode() == (uint16_t)MB_Error::Timeout)
    {
        return false;
    }
    if (_canUseCallback && !_canUseCallback(this->owner, device))
    {
        return false;
    }
    return device->isHeatup();
}

void AmperageBudgetManager::_loopCycleAll()
{
    uint32_t now_s = now / 1000;
    auto canStartHeating = [&](uint8_t deviceIndex) -> bool
    {
        if (_numDevices == 0)
            return false;
        if (deviceIndex < m_startIndex.getValue() || deviceIndex > m_endIndex.getValue())
            return false; // Device is outside the cycle range

        // Calculate the number of active devices in the cycle range
        uint8_t numActiveDevices = 0;
        for (uint8_t i = m_startIndex.getValue(); i <= m_endIndex.getValue(); ++i)
        {
            if (i < _numDevices && _devices[i] && _devices[i]->enabled())
            { // Ensure we don't go out of bounds of actual devices
                numActiveDevices++;
            }
        }
        if (numActiveDevices == 0)
            return false;

        // Calculate window based on current index and max simultaneous heating
        // The window is within the [_startIndex, _endIndex] range
        uint8_t windowStart = _currentIndex;
        // Ensure maxSimultaneousHeating does not exceed the number of devices in the current range
        uint8_t effectiveMaxSimultaneous = m_maxSimultaneousHeating.getValue() > numActiveDevices ? numActiveDevices : m_maxSimultaneousHeating.getValue();

        uint8_t devicesInWindow = 0;
        uint8_t tempIndex = windowStart;
        std::vector<uint8_t> windowDeviceIndices;

        while (devicesInWindow < effectiveMaxSimultaneous)
        {
            if (tempIndex < _numDevices && _devices[tempIndex] && _devices[tempIndex]->enabled() && _checkHeatup(_devices[tempIndex]))
            {
                windowDeviceIndices.push_back(tempIndex);
                devicesInWindow++;
            }

            tempIndex++;
            if (tempIndex > m_endIndex.getValue())
            {
                tempIndex = m_startIndex.getValue(); // Wrap around to the start index
            }
            if (tempIndex == windowStart)
            { // Full circle
                break;
            }
        }

        // Check if deviceIndex is in the calculated windowDeviceIndices
        for (uint8_t idx : windowDeviceIndices)
        {
            if (deviceIndex == idx)
            {
                return true;
            }
        }
        return false;
    };

    auto advanceCurrentIndex = [&]()
    {
        uint8_t oldIndex = _currentIndex;
        uint8_t numActiveDevicesInRange = 0;
        for (uint8_t i = m_startIndex.getValue(); i <= m_endIndex.getValue(); ++i)
        {
            if (i < _numDevices && _devices[i] && _devices[i]->enabled())
                numActiveDevicesInRange++;
        }

        if (numActiveDevicesInRange > 0)
        {
            uint8_t currentOffset = 0;
            uint8_t nextIndex = _currentIndex;
            do
            {
                nextIndex++;
                if (nextIndex > m_endIndex.getValue())
                {
                    nextIndex = m_startIndex.getValue(); // Wrap around to the start index
                }
                if (nextIndex < _numDevices && _devices[nextIndex] && _devices[nextIndex]->enabled())
                {
                    currentOffset++;
                }
            } while (currentOffset < m_windowOffset.getValue() && nextIndex != _currentIndex);
            _currentIndex = nextIndex;
        }
    };

    auto updateDevice = [&](uint8_t deviceIndex) -> bool
    {
        OmronE5 *device = _devices[deviceIndex];
        if (deviceIndex >= _numDevices || device == nullptr)
            return false;

        if (deviceIndex < m_startIndex.getValue() || deviceIndex > m_endIndex.getValue())
        {
            return false;
        }

        if (_deviceHeating[deviceIndex])
        {
            // If the device is no longer allowed to be used (e.g., its profile was stopped), stop it immediately.
            if (!_checkHeatup(device))
            {
                device->stop();
                _deviceHeating[deviceIndex] = false;
                if (deviceIndex == _currentIndex)
                {
                    advanceCurrentIndex();
                }
                return true; // Action taken
            }

            bool isPastHeatup = !_deviceInHeatup[deviceIndex];
            uint32_t maxDuration = m_maxHeatingDurationS.getValue();

            if (isPastHeatup && !device->isHeating())
            {
                device->stop();
                _deviceHeating[deviceIndex] = false;
                if (deviceIndex == _currentIndex)
                {
                    advanceCurrentIndex();
                }
                return true; // Action taken
            }
            else if (now_s - _deviceStartTimes[deviceIndex] >= maxDuration)
            {
                device->stop();
                _deviceHeating[deviceIndex] = false;
                if (deviceIndex == _currentIndex)
                {
                    advanceCurrentIndex();
                }
                return true; // Action taken
            }
            else if (now_s - _deviceStartTimes[deviceIndex] < m_minHeatingDurationS.getValue())
            {
                return false;
            }
        }
        else
        {
            if (canStartHeating(deviceIndex) && _deviceInHeatup[deviceIndex])
            {
                device->run();
                _deviceStartTimes[deviceIndex] = now_s;
                _deviceHeating[deviceIndex] = true;
                return true; // Action taken
            }
        }
        return false;
    };

    // If the device at the current index isn't heating and doesn't need to, advance the index.
    if (_currentIndex < _numDevices && _devices[_currentIndex] != nullptr && !_deviceHeating[_currentIndex])
    {
        if (!_checkHeatup(_devices[_currentIndex]))
        {
            advanceCurrentIndex();
        }
    }

    for (uint8_t i = 0; i < _numDevices; ++i)
    {
        if (updateDevice(i))
        {
            return; // Limit to one action per cycle
        }
    }
}

void AmperageBudgetManager::_loopCycleSp()
{
    auto getWindowDeviceIndices = [&]() -> std::vector<uint8_t>
    {
        std::vector<uint8_t> indices;
        uint8_t numActiveDevicesInRange = 0;
        for (uint8_t i = m_startIndex.getValue(); i <= m_endIndex.getValue(); ++i)
        {
            if (i < _numDevices && _devices[i] && _devices[i]->enabled())
            {
                numActiveDevicesInRange++;
            }
        }

        if (numActiveDevicesInRange == 0)
            return indices;

        uint8_t effectiveMaxSimultaneous = m_maxSimultaneousHeating.getValue() > numActiveDevicesInRange ? numActiveDevicesInRange : m_maxSimultaneousHeating.getValue();
        uint8_t devicesInWindow = 0;
        uint8_t tempIndex = _currentIndex;

        while (devicesInWindow < effectiveMaxSimultaneous)
        {
            if (tempIndex < _numDevices && _devices[tempIndex] && _devices[tempIndex]->enabled() && _checkHeatup(_devices[tempIndex]))
            {
                indices.push_back(tempIndex);
                devicesInWindow++;
            }

            tempIndex++;
            if (tempIndex > m_endIndex.getValue())
            {
                tempIndex = m_startIndex.getValue(); // Wrap around
            }
            if (tempIndex == _currentIndex)
            { // Full circle
                break;
            }
        }
        return indices;
    };

    auto advanceCurrentIndex = [&]()
    {
        uint8_t numActiveDevicesInRange = 0;
        for (uint8_t i = m_startIndex.getValue(); i <= m_endIndex.getValue(); ++i)
        {
            if (i < _numDevices && _devices[i] && _devices[i]->enabled())
                numActiveDevicesInRange++;
        }

        if (numActiveDevicesInRange > 0)
        {
            uint8_t currentOffset = 0;
            uint8_t nextIndex = _currentIndex;
            do
            {
                nextIndex++;
                if (nextIndex > m_endIndex.getValue())
                {
                    nextIndex = m_startIndex.getValue(); // Wrap around to the start index
                }
                if (nextIndex < _numDevices && _devices[nextIndex] && _devices[nextIndex]->enabled())
                {
                    currentOffset++;
                }
            } while (currentOffset < m_windowOffset.getValue() && nextIndex != _currentIndex);
            _currentIndex = nextIndex;
        }
    };

    std::vector<uint8_t> windowIndices = getWindowDeviceIndices();

    bool allInWindowAreDone = true;
    if (!windowIndices.empty())
    {
        for (uint8_t idx : windowIndices)
        {
            if (_checkHeatup(_devices[idx]))
            {
                allInWindowAreDone = false;
                break;
            }
        }
    }

    if (allInWindowAreDone)
    {
        // Debounce: Only advance if the "Done" state persists to avoid deadband flutter
        _batchDoneConfirmationCount++;
        if (_batchDoneConfirmationCount >= BATCH_DONE_THRESHOLD)
        {
            advanceCurrentIndex();
            windowIndices = getWindowDeviceIndices();
            _batchDoneConfirmationCount = 0;
        }
    }
    else
    {
        _batchDoneConfirmationCount = 0;
    }

    for (uint8_t i = 0; i < _numDevices; i++)
    {
        if (_devices[i] == nullptr)
            continue;

        if (!_devices[i]->enabled())
        {
            _devices[i]->stop();
            _deviceHeating[i] = false;
            continue;
        }

        bool isInWindow = false;
        for (uint8_t windowIdx : windowIndices)
        {
            if (i == windowIdx)
            {
                isInWindow = true;
                break;
            }
        }

        if (isInWindow && _checkHeatup(_devices[i]))
        {
            _devices[i]->run();
            _deviceHeating[i] = true;
            return; // One action per cycle
        }
        else
        {
            _devices[i]->stop();
            _deviceHeating[i] = false;
            return;
        }
    }
}

void AmperageBudgetManager::_loopCycleSpAny()
{
    std::vector<uint8_t> needsHeatingIndices;
    // Find all enabled devices that need heating within the specified range
    for (uint8_t i = m_startIndex.getValue(); i <= m_endIndex.getValue(); ++i)
    {
        if (i < _numDevices && _devices[i] && _devices[i]->enabled() && _checkHeatup(_devices[i]))
        {
            needsHeatingIndices.push_back(i);
        }
    }

    // Now, iterate through all devices and decide if they should be on or off.
    for (uint8_t i = 0; i < _numDevices; i++)
    {
        if (!_devices[i])
            continue;

        bool shouldBeHeating = false;
        // Check if this device 'i' is one of the ones that needs heating
        for (size_t j = 0; j < needsHeatingIndices.size(); ++j)
        {
            // And if its position in the 'needs list' is within our budget
            if (needsHeatingIndices[j] == i && j < m_maxSimultaneousHeating.getValue())
            {
                shouldBeHeating = true;
                break;
            }
        }

        if (shouldBeHeating)
        {
            if (!_deviceHeating[i])
            {
                _devices[i]->run();
                _deviceHeating[i] = true;
                return; // One action per cycle
            }
        }
        else
        {
            if (_deviceHeating[i])
            {
                // Enforce minimum heating duration to prevent rapid cycling
                uint32_t now_s = now / 1000;
                if (now_s - _deviceStartTimes[i] < m_minHeatingDurationS.getValue())
                {
                    continue; // Keep running
                }

                _devices[i]->stop();
                _deviceHeating[i] = false;
                return; // One action per cycle
            }
        }
    }
}

void AmperageBudgetManager::_loopCycleSpMostUrgent()
{
    struct DeviceUrgency
    {
        uint8_t index;
        int32_t urgency; // Difference between SP and PV

        bool operator<(const DeviceUrgency &other) const
        {
            return urgency > other.urgency; // Higher urgency (bigger diff) comes first
        }
    };

    std::vector<DeviceUrgency> urgentDevices;
    for (uint8_t i = m_startIndex.getValue(); i <= m_endIndex.getValue(); ++i)
    {
        if (i < _numDevices && _devices[i] && _devices[i]->enabled() && _checkHeatup(_devices[i]))
        {
            uint16_t pv, sp;
            if (_devices[i]->getPV(pv) && _devices[i]->getSP(sp))
            {
                if (sp > pv)
                {
                    urgentDevices.push_back({i, (int32_t)sp - pv});
                }
            }
        }
    }

    std::sort(urgentDevices.begin(), urgentDevices.end());

    uint32_t now_s = now / 1000;
    std::vector<uint8_t> lingeringIndices;

    // 1. Identify "Lingering" devices (must keep running due to MinDuration)
    for (uint8_t i = 0; i < _numDevices; i++)
    {
        if (_devices[i] && _devices[i]->enabled() && _deviceHeating[i])
        {
            if (now_s - _deviceStartTimes[i] < m_minHeatingDurationS.getValue())
            {
                lingeringIndices.push_back(i);
            }
        }
    }

    std::vector<uint8_t> devicesToHeat;
    // 2. Reserve slots for lingering devices
    for (uint8_t idx : lingeringIndices)
    {
        devicesToHeat.push_back(idx);
    }

    // 3. Fill remaining slots with most urgent devices
    for (size_t i = 0; i < urgentDevices.size(); ++i)
    {
        if (devicesToHeat.size() >= m_maxSimultaneousHeating.getValue())
        {
            break;
        }

        bool isLingering = false;
        for (uint8_t lIdx : lingeringIndices)
        {
            if (urgentDevices[i].index == lIdx)
            {
                isLingering = true;
                break;
            }
        }

        if (!isLingering)
        {
            devicesToHeat.push_back(urgentDevices[i].index);
        }
    }

    for (uint8_t i = 0; i < _numDevices; i++)
    {
        if (_devices[i] == nullptr || !_devices[i]->enabled())
        {
            if (_deviceHeating[i])
            {
                _devices[i]->stop();
                _deviceHeating[i] = false;
            }
            continue;
        }

        bool shouldHeat = false;
        for (uint8_t heatIndex : devicesToHeat)
        {
            if (i == heatIndex)
            {
                shouldHeat = true;
                break;
            }
        }

        if (_deviceHeating[i])
        {
            bool isPastHeatup = !_deviceInHeatup[i];
            uint32_t maxDuration = m_maxHeatingDurationS.getValue();
            uint32_t elapsed = now_s - _deviceStartTimes[i];

            if (elapsed >= maxDuration)
            {
                _devices[i]->stop();
                _deviceHeating[i] = false;
                return; // Action taken
            }

            if (!shouldHeat)
            {
                // Enforce minimum heating duration
                if (elapsed < m_minHeatingDurationS.getValue())
                {
                    continue; // Keep running
                }

                _devices[i]->stop();
                _deviceHeating[i] = false;
                return; // Action taken
            }
        }
        else
        {
            if (shouldHeat)
            {
                _devices[i]->run();
                _deviceStartTimes[i] = now_s;
                _deviceHeating[i] = true;
                return; // One action per cycle
            }
        }
    }
}

void AmperageBudgetManager::onCycleStart(const std::vector<OmronE5 *> &activeDevices)
{
    for (uint8_t i = 0; i < _numDevices; ++i)
    {
        if (_devices[i] == nullptr)
            continue;

        // Check if the current device is in the list of devices that should be active.
        bool found = (std::find(activeDevices.begin(), activeDevices.end(), _devices[i]) != activeDevices.end());

        // If it's not in the list, it shouldn't be heating. Stop it for safety.
        if (!found)
        {
            _devices[i]->stop();
            _deviceHeating[i] = false;
        }
    }
}

void AmperageBudgetManager::onCycleEnd(const std::vector<OmronE5 *> &activeDevices)
{
    for (const auto &device : activeDevices)
    {
        if (device)
        {
            // L_INFO(F("  - Device Active: ID %d"), device->slaveId);
        }
    }
}

short AmperageBudgetManager::loop()
{
    Component::loop();

    // Handle non-blocking sequential device stopping
    if (_stoppingIndex >= 0)
    {
        uint32_t now_ms = millis();
        if (now_ms - _lastStopTimestamp >= STOP_ALL_DEVICES_WAIT_MS)
        {
            if (_stoppingIndex < _numDevices)
            {
                if (_deviceHeating[_stoppingIndex] && _devices[_stoppingIndex] != nullptr)
                {
                    _devices[_stoppingIndex]->stop();
                    _deviceHeating[_stoppingIndex] = false;
                }
                _stoppingIndex++;
                _lastStopTimestamp = now_ms;
            }
            else
            {
                _stoppingIndex = -1; // Sequence complete
            }
        }
        return 0; // Block other logic while stopping
    }

    if (millis() - _lastLoopTime < SEQ_LOOP_INTERVAL_MS)
    {
        return 0;
    }
    _lastLoopTime = millis();
    _checkAllDevicesForHeatupCompletion();

    _activeDevices.clear();
    for (uint8_t i = 0; i < _numDevices; ++i)
    {
        if (_deviceHeating[i])
        {
            _activeDevices.push_back(_devices[i]);
        }
    }
    onCycleStart(_activeDevices);

    if (!enabled() || _numDevices == 0)
    {
        onCycleEnd(_activeDevices);
        return 0;
    }

    switch (m_mode.getValue())
    {
    case E_AM_CYCLE_ALL:
        _loopCycleAll();
        break;
    case E_AM_CYCLE_SP:
        _loopCycleSp();
        break;
    case E_AM_CYCLE_SP_ANY:
        _loopCycleSpAny();
        break;
    case E_AM_CYCLE_SP_MOST_URGENT:
        _loopCycleSpMostUrgent();
        break;
    }

    _activeDevices.clear();
    for (uint8_t i = 0; i < _numDevices; ++i)
    {
        if (_deviceHeating[i])
        {
            _activeDevices.push_back(_devices[i]);
        }
    }
    onCycleEnd(_activeDevices);

    return 0;
}

short AmperageBudgetManager::info(short val0, short val1)
{
    /*
    L_INFO(F("[%s] Devices: %d/%d, Current Index: %d\n"),
           _name.c_str(), _numDevices, MAX_MANAGED_DEVICES, _currentIndex);

    L_INFO(F("  Min Time: %lu s, Max Time: %lu s\n"),
           m_minHeatingDurationS.getValue(), m_maxHeatingDurationS.getValue());
    L_INFO(F("  Max Simultaneous: %d, Window Offset: %d\n"),
           m_maxSimultaneousHeating.getValue(), m_windowOffset.getValue());
    L_INFO(F("  Start Index: %d, End Index: %d\n"),
           m_startIndex.getValue(), m_endIndex.getValue());
    L_INFO(F("  OpFlags: %d\n"), m_opFlags.getValue());

    // Show the current window
    uint8_t windowEnd = (_currentIndex + m_maxSimultaneousHeating.getValue() - 1);
    if (_numDevices > 0)
    { // Prevent division by zero or incorrect modulo with 0 devices
        if (m_endIndex.getValue() < m_startIndex.getValue() || _numDevices <= m_startIndex.getValue())
        {                              // Handle invalid or empty range
            windowEnd = _currentIndex; // Default to current index if range is bad
        }
        else
        {
            // Calculate the effective number of devices in the custom range
            uint8_t devicesInRangeCount = 0;
            for (uint8_t i = m_startIndex.getValue(); i <= m_endIndex.getValue(); ++i)
            {
                if (i < _numDevices)
                    devicesInRangeCount++;
            }

            if (devicesInRangeCount > 0)
            {
                // Adjust windowEnd based on the custom range [_startIndex, _endIndex]
                // and wrap around within this range.
                uint8_t currentPosInRange = 0;
                uint8_t tempIdx = m_startIndex.getValue();
                while (tempIdx != _currentIndex && tempIdx <= m_endIndex.getValue())
                {
                    if (tempIdx < _numDevices)
                        currentPosInRange++;
                    tempIdx++;
                    if (tempIdx > m_endIndex.getValue() && _currentIndex != m_startIndex.getValue())
                        tempIdx = m_startIndex.getValue(); // Wrap
                    if (tempIdx == m_startIndex.getValue() && _currentIndex == m_startIndex.getValue())
                        break; // Optimization for CI == SI
                }

                uint8_t effectiveMaxSim = m_maxSimultaneousHeating.getValue() > devicesInRangeCount ? devicesInRangeCount : m_maxSimultaneousHeating.getValue();
                windowEnd = m_startIndex.getValue() + (currentPosInRange + effectiveMaxSim - 1) % devicesInRangeCount;

                // Ensure windowEnd does not exceed _endIndex by wrapping if necessary
                // This calculation seems complex and might need further refinement for wrapping
                // For now, let's try to keep it simple and show the logical end without complex wrapping visual
                // The _canStartHeating function correctly determines who can heat.
                // This is more for display.
                uint8_t displayWindowEnd = _currentIndex;
                uint8_t count = 0;
                uint8_t temp_idx = _currentIndex;
                while (count < m_maxSimultaneousHeating.getValue() && count < devicesInRangeCount)
                {
                    displayWindowEnd = temp_idx;
                    count++;
                    temp_idx++;
                    if (temp_idx > m_endIndex.getValue())
                        temp_idx = m_startIndex.getValue();
                    if (temp_idx == _currentIndex && count < m_maxSimultaneousHeating.getValue())
                        break; // full loop
                }
                Log.notice(F("  Active Window: Start %d, Logical End %d (MaxSim: %d, Range: %d-%d)"),
                           _currentIndex, displayWindowEnd, m_maxSimultaneousHeating.getValue(), m_startIndex.getValue(), m_endIndex.getValue());
            }
            else
            {
                Log.notice(F("  Active Window: No devices in range %d-%d"), m_startIndex.getValue(), m_endIndex.getValue());
            }
        }
    }
    else
    {
        Log.notice(F("  Active Window: No devices managed.\n"));
    }

    for (uint8_t i = 0; i < _numDevices; ++i)
    {
        if (_deviceHeating[i])
        {
            uint32_t elapsed_s = (millis() / 1000) - _deviceStartTimes[i];
            Log.notice(F("  Device %d: Heating for %lus\n"), i, elapsed_s);
        }
        else
        {
            Log.notice(F("  Device %d: Not heating\n"), i);
        }
    }
*/
    return 0;
}

short AmperageBudgetManager::mb_tcp_write(MB_Registers *reg, short value)
{
    short result = NetworkComponent::mb_tcp_write(reg, value);
    if (result != E_NOT_IMPLEMENTED)
        return result;

    uint16_t address = reg->startAddress;
    bool changed = false;

    if (address == (_baseAddress + REG_OFFSET_MIN_TIME))
    {
        if (_validateMinTime(value))
        {
            if (m_minHeatingDurationS.getValue() != (uint16_t)value)
            {
                m_minHeatingDurationS.update(value);
                reset();
                changed = true;
            }
        }
        else
        {
            return E_INVALID_PARAMETER;
        }
    }
    else if (address == (_baseAddress + REG_OFFSET_MAX_TIME))
    {
        if (_validateMaxTime(value))
        {
            if (m_maxHeatingDurationS.getValue() != (uint16_t)value)
            {
                m_maxHeatingDurationS.update(value);
                reset();
                changed = true;
            }
        }
        else
        {
            return E_INVALID_PARAMETER;
        }
    }
    else if (address == (_baseAddress + REG_OFFSET_MAX_SIM))
    {
        if (_validateMaxSim(value))
        {
            if (m_maxSimultaneousHeating.getValue() != (uint8_t)value)
            {
                m_maxSimultaneousHeating.update(value);
                reset();
                changed = true;
            }
        }
        else
        {
            return E_INVALID_PARAMETER;
        }
    }
    else if (address == (_baseAddress + REG_OFFSET_OFFSET))
    {
        if (_validateOffset(value))
        {
            if (m_windowOffset.getValue() != (uint8_t)value)
            {
                m_windowOffset.update(value);
                reset();
                changed = true;
            }
        }
        else
        {
            return E_INVALID_PARAMETER;
        }
    }
    else if (address == (_baseAddress + REG_OFFSET_START_INDEX))
    {
        if (_validateStartIndex(value))
        {
            if (m_startIndex.getValue() != (uint8_t)value)
            {
                m_startIndex.update(value);
                reset();
                changed = true;
            }
        }
        else
        {
            return E_INVALID_PARAMETER;
        }
    }
    else if (address == (_baseAddress + REG_OFFSET_END_INDEX))
    {
        if (_validateEndIndex(value))
        {
            if (m_endIndex.getValue() != (uint8_t)value)
            {
                m_endIndex.update(value);
                reset();
                changed = true;
            }
        }
        else
        {
            return E_INVALID_PARAMETER;
        }
    }
    else if (address == (_baseAddress + REG_OFFSET_MODE))
    {
        if (value >= E_AM_CYCLE_ALL && value <= E_AM_CYCLE_SP_MOST_URGENT)
        {
            if (m_mode.getValue() != (E_AMPERAGE_MODE)value)
            {
                m_mode.update((E_AMPERAGE_MODE)value);
                reset();
                changed = true;
            }
        }
        else
        {
            return E_INVALID_PARAMETER;
        }
    }

    else if (address == (_baseAddress + REG_OFFSET_OP_FLAGS))
    {
        if (m_opFlags.getValue() != (uint16_t)value)
        {
            m_opFlags.update(value);
            changed = true;
        }
    }
    else
    {
        return E_INVALID_PARAMETER;
    }

    if (changed)
    {
        save("/amperage_budget.json");
    }

    return E_OK;
}

short AmperageBudgetManager::mb_tcp_read(MB_Registers *reg)
{
    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED)
        return result;

    uint16_t address = reg->startAddress;

    if (address == (_baseAddress + REG_OFFSET_INFO))
    {
        // Return a bit-packed status:
        // Bit 0: Enabled
        // Bits 1-8: Heating status for each device (1 if heating, 0 if not)
        // Bits 9-11: Number of devices
        // Bits 12-14: Current index
        // Bit 15: Reserved
        uint16_t status = enabled() ? 1 : 0;
        for (uint8_t i = 0; i < _numDevices && i < 8; ++i)
        {
            if (_deviceHeating[i])
            {
                status |= (1 << (i + 1));
            }
        }
        status |= (_numDevices & 0x07) << 9;
        status |= (_currentIndex & 0x07) << 12;
        return status;
    }
    if (address == (_baseAddress + REG_OFFSET_MIN_TIME))
    {
        return m_minHeatingDurationS.getValue();
    }
    if (address == (_baseAddress + REG_OFFSET_MAX_TIME))
    {
        return m_maxHeatingDurationS.getValue();
    }
    if (address == (_baseAddress + REG_OFFSET_MAX_SIM))
    {
        return m_maxSimultaneousHeating.getValue();
    }
    if (address == (_baseAddress + REG_OFFSET_OFFSET))
    {
        return m_windowOffset.getValue();
    }
    if (address == (_baseAddress + REG_OFFSET_START_INDEX))
    {
        return m_startIndex.getValue();
    }
    if (address == (_baseAddress + REG_OFFSET_END_INDEX))
    {
        return m_endIndex.getValue();
    }
    if (address == (_baseAddress + REG_OFFSET_MODE))
    {
        return m_mode.getValue();
    }

    if (address == (_baseAddress + REG_OFFSET_OP_FLAGS))
    {
        return m_opFlags.getValue();
    }
    return 0;
}

short AmperageBudgetManager::serial_register(Bridge *bridge)
{
    Component::serial_register(bridge);
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&AmperageBudgetManager::info);
    return E_OK;
}

void AmperageBudgetManager::notifyStateChange()
{
    Component::notifyStateChange();
    if (enabled())
    {
        reset();
    }
    else
    {
        _stopAllDevices();
    }
}

void AmperageBudgetManager::toJson(JsonDocument &doc) const
{
    JsonObject obj = doc.to<JsonObject>();

    obj["minHeatingDurationS"] = m_minHeatingDurationS.getValue();
    obj["maxHeatingDurationS"] = m_maxHeatingDurationS.getValue();

    obj["maxSimultaneousHeating"] = m_maxSimultaneousHeating.getValue();
    obj["windowOffset"] = m_windowOffset.getValue();
    obj["enabled"] = const_cast<AmperageBudgetManager *>(this)->enabled();
    obj["startIndex"] = m_startIndex.getValue();
    obj["endIndex"] = m_endIndex.getValue();
    if (_heatupPhaseComplete)
    {
        obj["mode"] = _initialMode;
    }
    else
    {
        obj["mode"] = m_mode.getValue();
    }
    obj["opFlags"] = m_opFlags.getValue();
}

bool AmperageBudgetManager::fromJson(const JsonObject &json)
{
    if (json.isNull())
    {
        Log.warningln(F("[%s] fromJson: Provided JSON object is null. Using defaults."), _name.c_str());
        return false;
    }
    uint32_t minHeatingDurationS = m_minHeatingDurationS.getValue();
    uint32_t maxHeatingDurationS = m_maxHeatingDurationS.getValue();

    uint8_t maxSimultaneousHeating = m_maxSimultaneousHeating.getValue();
    uint8_t windowOffset = m_windowOffset.getValue();

    JsonUtils::parseJsonFieldUint32(json, "minHeatingDurationS", minHeatingDurationS, "minHeatingDurationS", _name.c_str());
    m_minHeatingDurationS.applyUpdate(minHeatingDurationS);
    JsonUtils::parseJsonFieldUint32(json, "maxHeatingDurationS", maxHeatingDurationS, "maxHeatingDurationS", _name.c_str());
    m_maxHeatingDurationS.applyUpdate(maxHeatingDurationS);

    JsonUtils::parseJsonFieldUint8(json, "maxSimultaneousHeating", maxSimultaneousHeating, "maxSimultaneousHeating", _name.c_str());
    m_maxSimultaneousHeating.applyUpdate(maxSimultaneousHeating);
    JsonUtils::parseJsonFieldUint8(json, "windowOffset", windowOffset, "windowOffset", _name.c_str());
    m_windowOffset.applyUpdate(windowOffset);

    uint8_t tempMode = m_mode.getValue();
    JsonUtils::parseJsonFieldUint8(json, "mode", tempMode, "mode", _name.c_str());
    if (tempMode >= E_AM_CYCLE_ALL && tempMode <= E_AM_CYCLE_SP_MOST_URGENT)
    {
        m_mode.applyUpdate((E_AMPERAGE_MODE)tempMode);
    }

    uint32_t tempOpFlags32 = m_opFlags.getValue();
    JsonUtils::parseJsonFieldUint32(json, "opFlags", tempOpFlags32, "opFlags", _name.c_str());
    m_opFlags.applyUpdate((uint16_t)tempOpFlags32);

    uint8_t tempStartIndex = m_startIndex.getValue();
    uint8_t tempEndIndex = m_endIndex.getValue();
    JsonUtils::parseJsonFieldUint8(json, "startIndex", tempStartIndex, "startIndex", _name.c_str());
    JsonUtils::parseJsonFieldUint8(json, "endIndex", tempEndIndex, "endIndex", _name.c_str());

    // Validate and apply start/end index carefully
    if (tempStartIndex >= 0 && tempStartIndex < MAX_MANAGED_DEVICES &&
        tempEndIndex >= 0 && tempEndIndex < MAX_MANAGED_DEVICES &&
        tempStartIndex <= tempEndIndex)
    {
        m_startIndex.applyUpdate(tempStartIndex);
        m_endIndex.applyUpdate(tempEndIndex);
        // Ensure currentIndex is valid after loading new start/end
        if (_currentIndex < m_startIndex.getValue() || _currentIndex > m_endIndex.getValue())
        {
            _currentIndex = m_startIndex.getValue();
        }
    }
    else
    {
        Log.warningln(F("[%s] Invalid startIndex (%u) or endIndex (%u) from JSON. Using existing values: %u, %u"), _name.c_str(), tempStartIndex, tempEndIndex, m_startIndex.getValue(), m_endIndex.getValue());
    }

    bool wasEnabled = const_cast<AmperageBudgetManager *>(this)->enabled();
    bool newEnabled = wasEnabled;
    JsonUtils::parseJsonFieldBool(json, "enabled", newEnabled, "enabled", _name.c_str());
    if (newEnabled != wasEnabled)
    {
        enable(newEnabled);
    }
    return true;
}

bool AmperageBudgetManager::load(const char *path)
{
    if (!LittleFS.begin())
    {
        L_ERROR(F("[%s] Failed to initialize LittleFS for load."), _name.c_str());
        return false;
    }

    File configFile = LittleFS.open(path, "r");
    if (!configFile)
    {
        Log.warningln(F("[%s] Settings file not found: %s. Using current (default) settings."), _name.c_str(), path);
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error)
    {
        L_ERROR(F("[%s] Failed to deserialize settings file %s: %s"), _name.c_str(), path, error.c_str());
        return false;
    }
    return fromJson(doc.as<JsonObject>());
}

bool AmperageBudgetManager::save(const char *path) const
{
    if (!LittleFS.begin())
    {
        L_ERROR(F("[%s] Failed to initialize LittleFS for save."), _name.c_str());
        return false;
    }

    JsonDocument doc;
    toJson(doc);

    File configFile = LittleFS.open(path, "w");
    if (!configFile)
    {
        L_ERROR(F("[%s] Failed to open settings file for writing: %s"), _name.c_str(), path);
        return false;
    }

    size_t bytesWritten = serializeJson(doc, configFile);
    configFile.close();

    if (bytesWritten == 0)
    {
        L_ERROR(F("[%s] Failed to write settings to file: %s"), _name.c_str(), path);
        return false;
    }

    return true;
}

void AmperageBudgetManager::print() const
{
    /*
    L_INFO(F("--- AmperageBudgetManager Values ---"));
    L_INFO(F("  minHeatingDurationMs: %lu"), _minHeatingDurationMs);
    L_INFO(F("  maxHeatingDurationMs: %lu"), _maxHeatingDurationMs);
    L_INFO(F("  maxSimultaneousHeating: %u"), _maxSimultaneousHeating);
    L_INFO(F("  windowOffset: %u"), _windowOffset);
    L_INFO(F("  enabled: %s"), const_cast<AmperageBudgetManager*>(this)->enabled() ? "Yes" : "No");
    L_INFO(F("  startIndex: %u"), _startIndex);
    L_INFO(F("  endIndex: %u"), _endIndex);
    L_INFO(F("  currentIndex: %u"), _currentIndex);
    L_INFO(F("  numDevices: %u"), _numDevices);
    L_INFO(F("--- End AmperageBudgetManager Values ---"));
    */
}

void AmperageBudgetManager::onHeatupComplete()
{
    if (!_heatupPhaseComplete)
    {
        _heatupPhaseComplete = true;
    }
}

void AmperageBudgetManager::_checkAllDevicesForHeatupCompletion()
{
    bool anyDeviceInHeatup = false;
    bool excessiveDropDetected = false;

    for (uint8_t i = 0; i < _numDevices; ++i)
    {
        OmronE5 *device = _devices[i];
        if (device == nullptr)
            continue;

        bool isCurrentlyInHeatup = _checkHeatup(device);
        _deviceInHeatup[i] = isCurrentlyInHeatup;

        if (isCurrentlyInHeatup)
        {
            anyDeviceInHeatup = true;

            if (_heatupPhaseComplete)
            {
                // A device has fallen behind after the initial heatup phase was complete.
                // Only revert if the drop is significant to avoid oscillation.
                uint16_t sp, pv;
                // Access OmronE5 SP/PV safely
                if (device->getSP(sp) && device->getPV(pv))
                {
                    // Unsigned arithmetic safety check
                    if (sp > pv && (sp - pv) > REVERT_TO_HEATUP_DEADBAND)
                    {
                        excessiveDropDetected = true;
                        L_INFO(F("[%s] Device %d dropped significantly below SP (Diff > %d). Reverting to initial mode %d"), _name.c_str(), i, REVERT_TO_HEATUP_DEADBAND, _initialMode);
                    }
                }
            }
        }
    }

    if (!_heatupPhaseComplete)
    {
        if (!anyDeviceInHeatup)
        {
            onHeatupComplete();
        }
    }
    else
    {
        if (excessiveDropDetected)
        {
            m_mode.update(_initialMode);
            _heatupPhaseComplete = false; // Allow the cycle to complete again
        }
    }
}

size_t AmperageBudgetManager::getBinaryState(uint8_t *buffer, size_t maxLen)
{
    if (maxLen < sizeof(StatePacket))
    {
        return 0;
    }

    StatePacket *packet = (StatePacket *)buffer;
    packet->timestamp = millis();
    packet->currentIdx = _currentIndex;
    packet->numDevices = _numDevices;
    packet->heatupComplete = _heatupPhaseComplete ? 1 : 0;

    // Zero out devices first
    memset(packet->devices, 0, sizeof(packet->devices));

    uint32_t now_s = millis() / 1000;

    for (uint8_t i = 0; i < _numDevices && i < NUM_OMRON_DEVICES; ++i)
    {
        packet->devices[i].index = i;
        if (_devices[i])
        {
            packet->devices[i].enabled = _devices[i]->enabled() ? 1 : 0;
        }
        else
        {
            packet->devices[i].enabled = 0;
        }
        packet->devices[i].heating = _deviceHeating[i] ? 1 : 0;
        packet->devices[i].heatup = _deviceInHeatup[i] ? 1 : 0;
        if (_deviceHeating[i])
        {
            packet->devices[i].elapsed = now_s - _deviceStartTimes[i];
        }
        else
        {
            packet->devices[i].elapsed = 0;
        }
    }

    return sizeof(StatePacket);
}

#endif
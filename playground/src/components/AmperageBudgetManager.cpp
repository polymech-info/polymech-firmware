#include <vector> 
#include <pid_constants.h>
#include <Bridge.h>
#include <modbus/Modbus.h>
#include <modbus/ModbusTCP.h>
#include <json.h>

#include "components/AmperageBudgetManager.h"

using namespace JsonUtils;

void AmperageBudgetManager::_resetBudgetState() {
    _stopAllDevices();
    _currentIndex = _startIndex;
}

void AmperageBudgetManager::_stopAllDevices() {
    for (uint8_t i = 0; i < _numDevices; ++i) {
        if (_deviceHeating[i] && _devices[i] != nullptr) {
            _devices[i]->stop();
            _deviceHeating[i] = false;
        }
    }
}

AmperageBudgetManager::AmperageBudgetManager(Component *owner)
    : Component("AmperageBudgetManager", COMPONENT_KEY_AMPERAGE_BUDGET_MANAGER, Component::COMPONENT_DEFAULT, owner),
      _numDevices(0),
      _currentIndex(0),
      _minHeatingDurationMs(MIN_HEATING_DURATION_MS),
      _maxHeatingDurationMs(MAX_HEATING_DURATION_MS),
      _maxSimultaneousHeating(DEFAULT_MAX_SIMULTANEOUS_HEATING),
      _windowOffset(DEFAULT_WINDOW_OFFSET),
      _startIndex(DEFAULT_START_INDEX),
      _endIndex(DEFAULT_END_INDEX)
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    disable();

    // Initialize Modbus blocks
    m_modbus_block_enable = INIT_MODBUS_BLOCK_TCP(
        MB_ADDR_AMPERAGE_BUDGET_ENABLE,  // Base address for enable coil
        0,                               // Offset
        E_FN_CODE::FN_WRITE_COIL,        // Function code
        MB_ACCESS_READ_WRITE,            // Access type
        "AmperageBudgetEnable",          // Name
        "AmperageBudget"                 // Group
    );

    m_modbus_block_info = INIT_MODBUS_BLOCK_TCP(
        MB_ADDR_AMPERAGE_BUDGET_INFO,    // Base address for info register
        0,                               // Offset
        E_FN_CODE::FN_READ_HOLD_REGISTER, // Function code
        MB_ACCESS_READ_ONLY,             // Access type
        "AmperageBudgetInfo",            // Name
        "AmperageBudget"                 // Group
    );

    m_modbus_block_min_time = INIT_MODBUS_BLOCK_TCP(
        MB_ADDR_AMPERAGE_BUDGET_MIN_TIME, // Base address for min time
        0,                               // Offset
        E_FN_CODE::FN_READ_HOLD_REGISTER, // Function code
        MB_ACCESS_READ_WRITE,            // Access type
        "AmperageBudgetMinTime",         // Name
        "AmperageBudget"                 // Group
    );

    m_modbus_block_max_time = INIT_MODBUS_BLOCK_TCP(
        MB_ADDR_AMPERAGE_BUDGET_MAX_TIME, // Base address for max time
        0,                               // Offset
        E_FN_CODE::FN_READ_HOLD_REGISTER, // Function code
        MB_ACCESS_READ_WRITE,            // Access type
        "AmperageBudgetMaxTime",         // Name
        "AmperageBudget"                 // Group
    );

    m_modbus_block_max_sim = INIT_MODBUS_BLOCK_TCP(
        MB_ADDR_AMPERAGE_BUDGET_MAX_SIM, // Base address for max simultaneous
        0,                               // Offset
        E_FN_CODE::FN_READ_HOLD_REGISTER, // Function code
        MB_ACCESS_READ_WRITE,            // Access type
        "AmperageBudgetMaxSim",          // Name
        "AmperageBudget"                 // Group
    );

    m_modbus_block_offset = INIT_MODBUS_BLOCK_TCP(
        MB_ADDR_AMPERAGE_BUDGET_OFFSET,  // Base address for window offset
        0,                               // Offset
        E_FN_CODE::FN_READ_HOLD_REGISTER, // Function code
        MB_ACCESS_READ_WRITE,            // Access type
        "AmperageBudgetOffset",          // Name
        "AmperageBudget"                 // Group
    );

    m_modbus_block_start_index = INIT_MODBUS_BLOCK_TCP(
        MB_ADDR_AMPERAGE_BUDGET_START_INDEX, // Base address for start index
        0,                               // Offset
        E_FN_CODE::FN_READ_HOLD_REGISTER, // Function code
        MB_ACCESS_READ_WRITE,            // Access type
        "AmperageBudgetStartIndex",      // Name
        "AmperageBudget"                 // Group
    );

    m_modbus_block_end_index = INIT_MODBUS_BLOCK_TCP(
        MB_ADDR_AMPERAGE_BUDGET_END_INDEX,   // Base address for end index
        0,                               // Offset
        E_FN_CODE::FN_READ_HOLD_REGISTER, // Function code
        MB_ACCESS_READ_WRITE,            // Access type
        "AmperageBudgetEndIndex",        // Name
        "AmperageBudget"                 // Group
    );

    // Initialize the view to point to all blocks
    static MB_Registers blocks[] = {
        m_modbus_block_enable,
        m_modbus_block_info,
        m_modbus_block_min_time,
        m_modbus_block_max_time,
        m_modbus_block_max_sim,
        m_modbus_block_offset,
        m_modbus_block_start_index,
        m_modbus_block_end_index
    };
    m_modbus_view.data = blocks;
    m_modbus_view.count = 8;

    for (uint8_t i = 0; i < MAX_MANAGED_DEVICES; ++i) {
        _devices[i] = nullptr;
        _deviceStartTimes[i] = 0;
        _deviceHeating[i] = false;
    }
}

bool AmperageBudgetManager::addManagedDevice(OmronE5* device)
{
    if (_numDevices >= MAX_MANAGED_DEVICES) {
        Log.errorln(F("[%s] Cannot add more devices, manager full (%d)."), _name, MAX_MANAGED_DEVICES);
        return false;
    }
    if (device == nullptr) {
        Log.errorln(F("[%s] Cannot add null device pointer."), _name);
        return false;
    }

    // Stop all devices before adding a new one
    _stopAllDevices();

    for(uint8_t i = 0; i < _numDevices; ++i) {
        if (_devices[i] == device) {
            Log.warningln(F("[%s] Device already added (Index %d). Ignoring."), _name, i);
            return true;
        }
    }

    _devices[_numDevices++] = device;
    Log.traceln(F("[%s] Added device %d."), _name, _numDevices - 1);
    return true;
}

short AmperageBudgetManager::setup()
{
    _stopAllDevices();  // Ensure all devices are stopped during setup
    
    _currentIndex = _startIndex; // Initialize current index to start index
    // Try to load configuration
    if (load("/amperage_budget.json")) {
        Log.infoln(F("[%s] Loaded configuration from /config/amperage_budget.json"), _name);
    } else {
        Log.warningln(F("[%s] Using default configuration"), _name);
    }
    return E_OK;
}

bool AmperageBudgetManager::_checkHeatup(OmronE5* device)
{
    return device->isHeatup();
}

bool AmperageBudgetManager::_canStartHeating(uint8_t deviceIndex) const {
    if (_numDevices == 0) return false;
    if (deviceIndex < _startIndex || deviceIndex > _endIndex) return false; // Device is outside the cycle range

    // Calculate the number of active devices in the cycle range
    uint8_t numActiveDevices = 0;
    for (uint8_t i = _startIndex; i <= _endIndex; ++i) {
        if (i < _numDevices) { // Ensure we don't go out of bounds of actual devices
            numActiveDevices++;
        }
    }
    if (numActiveDevices == 0) return false;


    // Calculate window based on current index and max simultaneous heating
    // The window is within the [_startIndex, _endIndex] range
    uint8_t windowStart = _currentIndex;
    // Ensure maxSimultaneousHeating does not exceed the number of devices in the current range
    uint8_t effectiveMaxSimultaneous = _maxSimultaneousHeating > numActiveDevices ? numActiveDevices : _maxSimultaneousHeating;


    uint8_t devicesInWindow = 0;
    uint8_t tempIndex = windowStart;
    std::vector<uint8_t> windowDeviceIndices;


    while(devicesInWindow < effectiveMaxSimultaneous) {
        windowDeviceIndices.push_back(tempIndex);
        devicesInWindow++;
        tempIndex++;
        if (tempIndex > _endIndex) {
            tempIndex = _startIndex; // Wrap around to the start index
        }
        if (tempIndex == windowStart && devicesInWindow < effectiveMaxSimultaneous) { // Full circle, but not enough devices
             break;
        }
    }

    // Check if deviceIndex is in the calculated windowDeviceIndices
    for (uint8_t idx : windowDeviceIndices) {
        if (deviceIndex == idx) {
            return true;
        }
    }
    return false;
}

void AmperageBudgetManager::_updateDevice(uint8_t deviceIndex, uint32_t currentTime)
{
    if (deviceIndex >= _numDevices || _devices[deviceIndex] == nullptr) return;
    if (deviceIndex < _startIndex || deviceIndex > _endIndex) return; // Skip if outside the defined range


    if (_deviceHeating[deviceIndex]) {
        // Check if device should stop heating
        if (!_checkHeatup(_devices[deviceIndex]) || 
            (currentTime - _deviceStartTimes[deviceIndex] >= _maxHeatingDurationMs)) {
            
            // Stop device
            _devices[deviceIndex]->stop();
            _deviceHeating[deviceIndex] = false;
            // If this was the first device in the window (_currentIndex), advance _currentIndex
            if (deviceIndex == _currentIndex) {
                // Calculate the number of active devices in the cycle range
                uint8_t numActiveDevicesInRange = 0;
                for (uint8_t i = _startIndex; i <= _endIndex; ++i) {
                    if (i < _numDevices) numActiveDevicesInRange++;
                }

                if (numActiveDevicesInRange > 0) {
                    uint8_t currentOffset = 0;
                    uint8_t nextIndex = _currentIndex;
                    while(currentOffset < _windowOffset) {
                        nextIndex++;
                        if (nextIndex > _endIndex) {
                            nextIndex = _startIndex; // Wrap around to the start index
                        }
                        // Ensure nextIndex is a valid device index within the actual number of devices
                        if (nextIndex < _numDevices) {
                             currentOffset++;
                        }
                        // Break if we've looped through all devices in range and haven't found enough valid next steps
                        if (nextIndex == _currentIndex && currentOffset < _windowOffset) break; 
                    }
                     _currentIndex = nextIndex;
                }
            }
        }
        // If still heating and min time not met, keep heating
        else if (currentTime - _deviceStartTimes[deviceIndex] < _minHeatingDurationMs) {
            return;
        }
    }

    // Check if device is in the current window and needs heating
    if (!_deviceHeating[deviceIndex] && 
        _canStartHeating(deviceIndex) && 
        _checkHeatup(_devices[deviceIndex])) {
        
        _devices[deviceIndex]->run();
        _deviceStartTimes[deviceIndex] = currentTime;
        _deviceHeating[deviceIndex] = true;
    }
}

short AmperageBudgetManager::loop()
{
    if (!enabled() || _numDevices == 0) return 0;

    uint32_t currentTime = millis();

    // Update each device
    for (uint8_t i = 0; i < _numDevices; ++i) {
        _updateDevice(i, currentTime);
    }

    return 0;
}

short AmperageBudgetManager::info(short val0, short val1)
{
    Log.notice(F("[%s] Devices: %d/%d, Current Index: %d\n"), 
        _name, _numDevices, MAX_MANAGED_DEVICES, _currentIndex);
    
    Log.notice(F("  Min Time: %lu ms, Max Time: %lu ms\n"), 
        _minHeatingDurationMs, _maxHeatingDurationMs);
    Log.notice(F("  Max Simultaneous: %d, Window Offset: %d\n"), 
        _maxSimultaneousHeating, _windowOffset);
    Log.notice(F("  Start Index: %d, End Index: %d\n"),
        _startIndex, _endIndex);
    
    // Show the current window
    uint8_t windowEnd = (_currentIndex + _maxSimultaneousHeating - 1);
    if (_numDevices > 0) { // Prevent division by zero or incorrect modulo with 0 devices
      if (_endIndex < _startIndex || _numDevices <= _startIndex) { // Handle invalid or empty range
        windowEnd = _currentIndex; // Default to current index if range is bad
      } else {
         // Calculate the effective number of devices in the custom range
        uint8_t devicesInRangeCount = 0;
        for(uint8_t i = _startIndex; i <= _endIndex; ++i) {
            if (i < _numDevices) devicesInRangeCount++;
        }

        if (devicesInRangeCount > 0) {
            // Adjust windowEnd based on the custom range [_startIndex, _endIndex]
            // and wrap around within this range.
            uint8_t currentPosInRange = 0;
            uint8_t tempIdx = _startIndex;
            while(tempIdx != _currentIndex && tempIdx <=_endIndex) {
                if (tempIdx < _numDevices) currentPosInRange++;
                tempIdx++;
                 if (tempIdx > _endIndex && _currentIndex != _startIndex) tempIdx = _startIndex; // Wrap
                 if (tempIdx == _startIndex && _currentIndex == _startIndex) break; // Optimization for CI == SI
            }
            
            uint8_t effectiveMaxSim = _maxSimultaneousHeating > devicesInRangeCount ? devicesInRangeCount : _maxSimultaneousHeating;
            windowEnd = _startIndex + (currentPosInRange + effectiveMaxSim -1) % devicesInRangeCount;

            // Ensure windowEnd does not exceed _endIndex by wrapping if necessary
            // This calculation seems complex and might need further refinement for wrapping
            // For now, let's try to keep it simple and show the logical end without complex wrapping visual
            // The _canStartHeating function correctly determines who can heat.
            // This is more for display.
            uint8_t displayWindowEnd = _currentIndex;
            uint8_t count = 0;
            uint8_t temp_idx = _currentIndex;
            while(count < _maxSimultaneousHeating && count < devicesInRangeCount) {
                displayWindowEnd = temp_idx;
                count++;
                temp_idx++;
                if (temp_idx > _endIndex) temp_idx = _startIndex;
                if (temp_idx == _currentIndex && count < _maxSimultaneousHeating) break; // full loop
            }
             Log.notice(F("  Active Window: Start %d, Logical End %d (MaxSim: %d, Range: %d-%d)"), 
                _currentIndex, displayWindowEnd, _maxSimultaneousHeating, _startIndex, _endIndex);
        } else {
             Log.notice(F("  Active Window: No devices in range %d-%d"), _startIndex, _endIndex);
        }
      }
    } else {
        Log.notice(F("  Active Window: No devices managed.\n"));
    }
        
    for (uint8_t i = 0; i < _numDevices; ++i) {
        if (_deviceHeating[i]) {
            uint32_t elapsed = millis() - _deviceStartTimes[i];
            Log.notice(F("  Device %d: Heating for %lums\n"), i, elapsed);
        } else {
            Log.notice(F("  Device %d: Not heating\n"), i);
        }
    }
    
    return 0;
}

short AmperageBudgetManager::mb_tcp_write(MB_Registers *reg, short value)
{
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_ENABLE) {
        if (value == 1) {
            _enableComponent();
            _resetBudgetState();
            return E_OK;
        }
        else if (value == 0) {
            _disableComponent();
            _currentIndex = _startIndex;
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }
    else if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_MIN_TIME) {
        if (_validateMinTime(value)) {
            _minHeatingDurationMs = value;
            _resetBudgetState();
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }
    else if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_MAX_TIME) {
        if (_validateMaxTime(value)) {
            _maxHeatingDurationMs = value;
            _resetBudgetState();
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }
    else if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_MAX_SIM) {
        if (_validateMaxSim(value)) {
            _maxSimultaneousHeating = value;
            _resetBudgetState();
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }
    else if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_OFFSET) {
        if (_validateOffset(value)) {
            _windowOffset = value;
            _resetBudgetState();
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }
    else if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_START_INDEX) {
        if (_validateStartIndex(value)) {
            _startIndex = value;
            _resetBudgetState();
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }
    else if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_END_INDEX) {
        if (_validateEndIndex(value)) {
            _endIndex = value;
            _resetBudgetState();
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }
    return E_INVALID_PARAMETER;
}

short AmperageBudgetManager::mb_tcp_read(MB_Registers *reg)
{
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_ENABLE) {
        return enabled() ? 1 : 0;
    }
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_INFO) {
        // Return a bit-packed status:
        // Bit 0: Enabled
        // Bits 1-8: Heating status for each device (1 if heating, 0 if not)
        // Bits 9-11: Number of devices
        // Bits 12-14: Current index
        // Bit 15: Reserved
        uint16_t status = enabled() ? 1 : 0;
        for (uint8_t i = 0; i < _numDevices && i < 8; ++i) {
            if (_deviceHeating[i]) {
                status |= (1 << (i + 1));
            }
        }
        status |= (_numDevices & 0x07) << 9;
        status |= (_currentIndex & 0x07) << 12;
        return status;
    }
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_MIN_TIME) {
        return _minHeatingDurationMs;
    }
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_MAX_TIME) {
        return _maxHeatingDurationMs;
    }
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_MAX_SIM) {
        return _maxSimultaneousHeating;
    }
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_OFFSET) {
        return _windowOffset;
    }
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_START_INDEX) {
        return _startIndex;
    }
    if (reg->startAddress == MB_ADDR_AMPERAGE_BUDGET_END_INDEX) {
        return _endIndex;
    }
    return 0;
}

void AmperageBudgetManager::mb_tcp_register(ModbusTCP* manager)
{
    ModbusBlockView* blocksView = mb_tcp_blocks();
    Component* thiz = const_cast<AmperageBudgetManager*>(this);
    for (int i = 0; i < blocksView->count; ++i) {
        MB_Registers info = blocksView->data[i];
        manager->registerModbus(thiz, info);
    }
}

ModbusBlockView* AmperageBudgetManager::mb_tcp_blocks() const
{
    return &m_modbus_view;
}

short AmperageBudgetManager::serial_register(Bridge* bridge)
{
    Component::serial_register(bridge);
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&AmperageBudgetManager::info);
    return E_OK;
}

void AmperageBudgetManager::notifyStateChange() {
    // Stop all devices when component is disabled
    if (!enabled()) {
        _disableComponent();
    }
}

void AmperageBudgetManager::toJson(JsonDocument& doc) const {
    JsonObject obj = doc.to<JsonObject>();

    obj["minHeatingDurationMs"] = _minHeatingDurationMs;
    obj["maxHeatingDurationMs"] = _maxHeatingDurationMs;
    obj["maxSimultaneousHeating"] = _maxSimultaneousHeating;
    obj["windowOffset"] = _windowOffset;
    obj["enabled"] = const_cast<AmperageBudgetManager*>(this)->enabled();
    obj["startIndex"] = _startIndex;
    obj["endIndex"] = _endIndex;
}

bool AmperageBudgetManager::fromJson(const JsonObject& json) {
    if (json.isNull()) {
        Log.warningln(F("[%s] fromJson: Provided JSON object is null. Using defaults."), _name);
        return false;
    }

    JsonUtils::parseJsonFieldUint32(json, "minHeatingDurationMs", _minHeatingDurationMs, "minHeatingDurationMs", _name.c_str());
    JsonUtils::parseJsonFieldUint32(json, "maxHeatingDurationMs", _maxHeatingDurationMs, "maxHeatingDurationMs", _name.c_str());
    JsonUtils::parseJsonFieldUint8(json, "maxSimultaneousHeating", _maxSimultaneousHeating, "maxSimultaneousHeating", _name.c_str());
    JsonUtils::parseJsonFieldUint8(json, "windowOffset", _windowOffset, "windowOffset", _name.c_str());

    uint8_t tempStartIndex = _startIndex;
    uint8_t tempEndIndex = _endIndex;
    JsonUtils::parseJsonFieldUint8(json, "startIndex", tempStartIndex, "startIndex", _name.c_str());
    JsonUtils::parseJsonFieldUint8(json, "endIndex", tempEndIndex, "endIndex", _name.c_str());

    // Validate and apply start/end index carefully
    if (tempStartIndex >= 0 && tempStartIndex < MAX_MANAGED_DEVICES && 
        tempEndIndex >= 0 && tempEndIndex < MAX_MANAGED_DEVICES && 
        tempStartIndex <= tempEndIndex) {
        _startIndex = tempStartIndex;
        _endIndex = tempEndIndex;
        // Ensure currentIndex is valid after loading new start/end
        if (_currentIndex < _startIndex || _currentIndex > _endIndex) {
            _currentIndex = _startIndex;
        }
    } else {
        Log.warningln(F("[%s] Invalid startIndex (%u) or endIndex (%u) from JSON. Using existing values: %u, %u"), _name, tempStartIndex, tempEndIndex, _startIndex, _endIndex);
    }

    bool wasEnabled = const_cast<AmperageBudgetManager*>(this)->enabled();
    bool newEnabled = wasEnabled;
    JsonUtils::parseJsonFieldBool(json, "enabled", newEnabled, "enabled", _name.c_str());
    
    if (newEnabled != wasEnabled) {
        if (newEnabled) {
            _enableComponent();
        } else {
            _disableComponent();
        }
    }

    Log.infoln(F("[%s] Settings parsed from JSON (check warnings above for issues). Call print() to see final values."), _name);
    return true;
}

bool AmperageBudgetManager::load(const char* path) {
    if (!LittleFS.begin()) {
        Log.errorln(F("[%s] Failed to initialize LittleFS for load."), _name);
        return false;
    }

    File configFile = LittleFS.open(path, "r");
    if (!configFile) {
        Log.warningln(F("[%s] Settings file not found: %s. Using current (default) settings."), _name, path);
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        Log.errorln(F("[%s] Failed to deserialize settings file %s: %s"), _name, path, error.c_str());
        return false;
    }
    
    Log.infoln(F("[%s] Successfully deserialized %s."), _name, path);
    return fromJson(doc.as<JsonObject>());
}

bool AmperageBudgetManager::save(const char* path) const {
    if (!LittleFS.begin()) {
        Log.errorln(F("[%s] Failed to initialize LittleFS for save."), _name);
        return false;
    }

    JsonDocument doc;
    toJson(doc);

    File configFile = LittleFS.open(path, "w");
    if (!configFile) {
        Log.errorln(F("[%s] Failed to open settings file for writing: %s"), _name, path);
        return false;
    }

    size_t bytesWritten = serializeJson(doc, configFile);
    configFile.close();

    if (bytesWritten == 0) {
        Log.errorln(F("[%s] Failed to write settings to file: %s"), _name, path);
        return false;
    }

    Log.infoln(F("[%s] Settings successfully saved to %s (%u bytes)."), _name, path, bytesWritten);
    return true;
}

void AmperageBudgetManager::print() const {
    Log.infoln(F("--- AmperageBudgetManager Values ---"));
    Log.infoln(F("  minHeatingDurationMs: %lu"), _minHeatingDurationMs);
    Log.infoln(F("  maxHeatingDurationMs: %lu"), _maxHeatingDurationMs);
    Log.infoln(F("  maxSimultaneousHeating: %u"), _maxSimultaneousHeating);
    Log.infoln(F("  windowOffset: %u"), _windowOffset);
    Log.infoln(F("  enabled: %s"), const_cast<AmperageBudgetManager*>(this)->enabled() ? "Yes" : "No");
    Log.infoln(F("  startIndex: %u"), _startIndex);
    Log.infoln(F("  endIndex: %u"), _endIndex);
    Log.infoln(F("  currentIndex: %u"), _currentIndex);
    Log.infoln(F("  numDevices: %u"), _numDevices);
    Log.infoln(F("--- End AmperageBudgetManager Values ---"));
} 
#include <math.h>
#include <Bridge.h>
#include <Logger.h>
#include <enums.h>
#include <NetworkValue.h>
#include <modbus/Modbus.h>
#include <modbus/ModbusTypes.h>
#include <modbus/ModbusTCP.h>
#include <modbus/Modbus.h>
#include "TemperatureProfile.h"
#include "config-modbus.h"
#include "config.h"
#include "PHApp.h"

#define ENABLE_PROFILE_TEMPERATURE

#ifdef ENABLE_PROFILE_TEMPERATURE

#define REGISTER_NAME_PREFIX "TProf"

// start : <<900;2;64;start:0:0>>
// stop : <<900;2;64;stop:0:0>>
// pause : <<900;2;64;pause:0:0>>
// resume : <<900;2;64;resume:0:0>>

#define TP_MB_REG_EX(vw_member, vw_type, offset, fn_code, ...) \
    MB_REG_EX(vw_member, vw_type, TemperatureProfileRegisterOffset::offset, fn_code, REGISTER_NAME_PREFIX " " #offset, name.c_str(), __VA_ARGS__)

#define TP_MB_BLOCK(offset, fn_code) \
    MB_BLOCK(TemperatureProfileRegisterOffset::offset, fn_code, REGISTER_NAME_PREFIX " " #offset, name.c_str())
   

TemperatureProfile::TemperatureProfile(Component *owner, short slot, ushort componentId) 
    : PlotBase(owner, componentId),
      _numControlPoints(0),
      slot(slot),
      modbusTCP(nullptr),
      max(150),
      _lastLoopExecutionMs(0),
      _lastLogMs(0),
      _targetRegisters(TEMP_PROFILE_MAX_TARGET_REGS, 0),
      _signalPlotSlotId(-1)
{
    name = "TempProfile_" + String(this->id) + "_Slot_" + String(slot);
    for (int i = 0; i < MAX_TEMP_CONTROL_POINTS; ++i)
    {
        _controlPoints[i] = {};
    }
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

    const uint16_t tcpBaseAddr = mb_tcp_base_address();
    const char* group = name.c_str();    
    init__statusWrapper();
    MB_REG_EX_A(
        _currentTemperatureWrapper, int16_t,
        TemperatureProfileRegisterOffset, CURRENT_TEMP, E_FN_CODE::FN_WRITE_HOLD_REGISTER,
        REGISTER_NAME_PREFIX, group,
        INT16_MIN, 1, ValueWrapper<int16_t>::ThresholdMode::DIFFERENCE
    );
    MB_REG_EX_A(
        _enabledStateWrapper, bool,
        TemperatureProfileRegisterOffset, ENABLE_CMD, E_FN_CODE::FN_WRITE_COIL,
        REGISTER_NAME_PREFIX, group,
        false, true, ValueWrapper<bool>::ThresholdMode::DIFFERENCE
    );
    MB_REG_EX_A(
        _elapsedTimeWrapper, uint32_t,
        TemperatureProfileRegisterOffset, ELAPSED_LW, E_FN_CODE::FN_READ_HOLD_REGISTER,
        REGISTER_NAME_PREFIX, group,
        0, 1000, ValueWrapper<uint32_t>::ThresholdMode::INTERVAL_STEP,
        [this](const uint32_t&, const uint32_t&) { 
            // Log on interval cross if needed
        }
    );
    MB_REG_EX_A(
        _elapsedTimeHwWrapper, uint16_t,
        TemperatureProfileRegisterOffset, ELAPSED_HW, E_FN_CODE::FN_READ_HOLD_REGISTER,
        REGISTER_NAME_PREFIX, group,
        0, 1, ValueWrapper<uint16_t>::ThresholdMode::DIFFERENCE
    );

    // Initialize non-wrapped blocks
    _modbusBlocks[static_cast<uint16_t>(TemperatureProfileRegisterOffset::DURATION)] =
        MB_BLOCK(TemperatureProfileRegisterOffset::DURATION, E_FN_CODE::FN_READ_HOLD_REGISTER, "TProf Duration", group);
    _modbusBlocks[static_cast<uint16_t>(TemperatureProfileRegisterOffset::REMAINING)] =
        MB_BLOCK(TemperatureProfileRegisterOffset::REMAINING, E_FN_CODE::FN_READ_HOLD_REGISTER, "TProf Remaining", group);
    _modbusBlocks[static_cast<uint16_t>(TemperatureProfileRegisterOffset::COMMAND)] =
        MB_BLOCK(TemperatureProfileRegisterOffset::COMMAND, E_FN_CODE::FN_WRITE_HOLD_REGISTER, "TProf Command", group);
    
    _modbusBlockView = {_modbusBlocks, TEMP_PROFILE_REGISTER_COUNT};
}

short TemperatureProfile::setup()
{
    return E_OK;
}

void TemperatureProfile::start()
{
    #if ENABLED(ENABLE_PROFILE_SIGNAL_PLOT, ENABLE_PROFILE_TEMPERATURE)
    if (_signalPlotSlotId >= 0) {
        PHApp* phApp = static_cast<PHApp*>(owner);
        if (phApp) {
            for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i) {
                TemperatureProfile* otherProfile = phApp->tempProfiles[i];
                if (otherProfile && otherProfile != this && 
                    otherProfile->isRunning() && 
                    otherProfile->getSignalPlotSlotId() == _signalPlotSlotId) {
                    Log.warningln("%s: Cannot start - Signal plot slot %d is already in use by running profile %s", 
                                  name.c_str(), _signalPlotSlotId, otherProfile->name.c_str());
                    return;
                }
            }
        }
    }
    #endif // ENABLED(ENABLE_PROFILE_SIGNAL_PLOT, ENABLE_PROFILE_TEMPERATURE)
    PlotBase::start();
}

short TemperatureProfile::loop()
{
    PlotBase::loop();
    uint32_t now = millis();

    PlotStatus currentStatus = getCurrentStatus(); 
    bool currentEnabledState = this->enabled();

    _statusWrapper.update(currentStatus); 
    _enabledStateWrapper.update(currentEnabledState);

    // ValueWrapper now handles the interval logic for elapsed time
    if (currentStatus == PlotStatus::RUNNING && currentEnabledState) {
        uint32_t currentElapsedMs = getElapsedMs();
        _elapsedTimeWrapper.update(currentElapsedMs);
        _elapsedTimeHwWrapper.update(HIGH_WORD(currentElapsedMs));
    } else {
        _elapsedTimeWrapper.setValueWithoutNotification(0);
        _elapsedTimeHwWrapper.setValueWithoutNotification(0);
    }

    // Logic below uses the fresh local currentStatus and currentEnabledState variables
    if (modbusTCP == nullptr || _targetRegisters.empty() || !currentEnabledState) { 
        if (currentStatus != PlotStatus::RUNNING) { // Check against fresh currentStatus
             _currentTemperatureWrapper.setValueWithoutNotification(INT16_MIN);
        }
        if (!currentEnabledState) { // Check against fresh currentEnabledState
             _currentTemperatureWrapper.setValueWithoutNotification(INT16_MIN);
             _elapsedTimeWrapper.setValueWithoutNotification(0); 
        }
        return E_OK;
    }
    if (now - _lastLoopExecutionMs < TEMPERATURE_PROFILE_LOOP_INTERVAL_MS)
    {
        return E_OK;
    }
    _lastLoopExecutionMs = now;
    
    if (currentStatus == PlotStatus::RUNNING)
    {
        int16_t currentTemp = getTemperature(getElapsedMs());
        if (currentTemp >= 0) 
        {
            _currentTemperatureWrapper.update(currentTemp);
            uint16_t tempValue = static_cast<uint16_t>(currentTemp);
            for (uint16_t targetRegAddr : _targetRegisters)
            {
                if (targetRegAddr == 0) continue;
                ModbusMessage resp;
                ModbusMessage req;
                Error err = SUCCESS;
                req.setMessage(static_cast<uint8_t>(1), static_cast<uint8_t>(E_FN_CODE::FN_WRITE_HOLD_REGISTER), targetRegAddr, tempValue); 
                resp = modbusTCP->modbusServer->localRequest(req);
                if ((err = resp.getError()) != SUCCESS)
                {
                    ModbusError me(err);
                    Log.errorln("%s: Error writing temp %u to slave 1, register %u: %02d - %s",
                                name.c_str(), tempValue, targetRegAddr, err, (const char *)me);
                }
            }
        }
        else
        {
             Log.infoln("%s: getTemperature returned -1, skipping update", name.c_str());
             _currentTemperatureWrapper.setValueWithoutNotification(INT16_MIN);
        }
    }
    else 
    {
         _currentTemperatureWrapper.setValueWithoutNotification(INT16_MIN);
         _elapsedTimeWrapper.setValueWithoutNotification(0);
        if (currentStatus == PlotStatus::FINISHED && static_cast<PlotStatus>(_statusWrapper) != PlotStatus::FINISHED)
        {
            Log.infoln("%s: Profile finished. Setting target SPs to 0.", name.c_str());
            for (uint16_t targetRegAddr : _targetRegisters)
            {
                if (targetRegAddr == 0) continue;
                ModbusMessage resp;
                ModbusMessage req;
                Error err = SUCCESS;
                req.setMessage(static_cast<uint8_t>(1), static_cast<uint8_t>(E_FN_CODE::FN_WRITE_HOLD_REGISTER), targetRegAddr, static_cast<uint16_t>(0));
                resp = modbusTCP->modbusServer->localRequest(req);
                if ((err = resp.getError()) != SUCCESS)
                {
                    ModbusError me(err);
                    Log.errorln("%s: Error writing SP=0 (finished) to slave 1, register %u: %02d - %s",
                                name.c_str(), targetRegAddr, err, (const char *)me);
                }
            }
        }
        else if (currentStatus == PlotStatus::STOPPED && static_cast<PlotStatus>(_statusWrapper) != PlotStatus::STOPPED)
        {
            _currentTemperatureWrapper.setValueWithoutNotification(INT16_MIN);
            for (uint16_t targetRegAddr : _targetRegisters)
            {
                if (targetRegAddr == 0) continue;
                ModbusMessage resp; 
                ModbusMessage req;  
                Error err = SUCCESS;
                req.setMessage(static_cast<uint8_t>(1), static_cast<uint8_t>(E_FN_CODE::FN_WRITE_HOLD_REGISTER), targetRegAddr, static_cast<uint16_t>(0));
                resp = modbusTCP->modbusServer->localRequest(req);
                if ((err = resp.getError()) != SUCCESS)
                {
                    ModbusError me(err);
                    Log.errorln("%s: Error writing SP=0 (stopped) to slave 1, register %u: %02d - %s",
                                name.c_str(), targetRegAddr, err, (const char *)me);
                }
            }
        }
    }
    return E_OK;
}

void TemperatureProfile::sample()
{
    _numControlPoints = 0;
    for (int i = 0; i < MAX_TEMP_CONTROL_POINTS; ++i)
    {
        _controlPoints[i] = {};
    }
    stop();
    _durationMs = 1000 * 10 * 20;
    // Point 0: Start at 20% (scaled) temp at time 0
    _controlPoints[0].x = 0;   // 0% time
    _controlPoints[0].y = 200; // 20% of PROFILE_SCALE

    // Point 1: Ramp up linearly to 80% temp by 30% time (18 seconds)
    _controlPoints[1].x = 500; // 30% time (scaled)
    _controlPoints[1].y = 800; // 80% temp (scaled)

    // Point 2: Hold at 80% temp until 70% time (42 seconds)
    _controlPoints[2].x = 700; // 70% time (scaled)
    _controlPoints[2].y = 400; // 80% temp (scaled) - same as previous

    // Point 3: Ramp down linearly to 30% temp by 90% time (54 seconds)
    _controlPoints[3].x = 900; // 90% time (scaled)
    _controlPoints[3].y = 300; // 30% temp (scaled)

    // Point 4: End at 25% temp at 100% time (60 seconds)
    _controlPoints[4].x = 1000; // 100% time (Use macro)
    _controlPoints[4].y = 200;  // 25% temp (scaled)
    _numControlPoints = 5;
    max = 150;
    Log.traceln("%s: Sample profile generated with %d points, duration %lu ms.", name.c_str(), _numControlPoints, _durationMs);
}
bool TemperatureProfile::load(const JsonObject &config)
{
    if (config["name"].is<String>())
    {
        name = config["name"].as<String>();
    }

    if (config["description"].is<String>())
    {
        setDescription(config["description"].as<String>());
    }

    // Load duration (in seconds) if present and convert to milliseconds
    if (config["duration"].is<uint32_t>())
    {
        uint32_t duration_s = config["duration"].as<uint32_t>();
        _durationMs = duration_s;        
        for (int i = 0; i < MAX_PLOTS; ++i) {
            if (_plots[i]) {
                _plots[i]->setDuration(_durationMs);
            }
        }
    }
    else
    {
        Log.warningln("%s: Duration not found in config, using default %lu ms", name.c_str(), _durationMs);
        // Keep the default _durationMs if not specified
    }

// Load duration (in seconds) if present and convert to milliseconds
    if (config["max"].is<uint32_t>())
    {
        uint32_t max_s = config["max"].as<uint32_t>();
        max = max_s;
    }
    else
    {
        Log.warningln("%s: Max not found in config, using default %u", name.c_str(), max);
    }

    _numControlPoints = 0;
    for (int i = 0; i < MAX_TEMP_CONTROL_POINTS; ++i)
    {
        _controlPoints[i] = {};
    }
    JsonArray pointsArray = config["controlPoints"].as<JsonArray>();
    _numControlPoints = min((uint8_t)pointsArray.size(), (uint8_t)MAX_TEMP_CONTROL_POINTS);
    if (_numControlPoints < 2)
    {
        Log.warningln("%s: Load failed - Not enough control points (found %d, need at least 2). Using empty profile.", name.c_str(), _numControlPoints);
        _numControlPoints = 0; // Need at least 2 points for interpolation
        return false;          // Consider it a failure if not enough points
    }
    Log.traceln("%s: Loading %d control points...", name.c_str(), _numControlPoints);
    for (uint8_t i = 0; i < _numControlPoints; ++i)
    {
        JsonObject pointObj = pointsArray[i].as<JsonObject>();
        // Ensure keys exist before trying to read, and provide defaults or log errors
        if (!pointObj["x"].is<int>() || !pointObj["y"].is<int>()) { 
            Log.errorln("%s: Control point %d in JSON is missing 'x' or 'y' or they are not integers. Skipping point.", name.c_str(), i);
            // For now, let's assume it might load with default 0,0 if as<int16_t> handles missing keys that way, or fails later.
            // A more robust approach would be to decrement _numControlPoints or flag an error.
        }
        int16_t x_from_json = pointObj["x"].as<int16_t>();
        int16_t y_scaled = pointObj["y"].as<int16_t>();
         _controlPoints[i].x = x_from_json;
        _controlPoints[i].y = y_scaled;
    }

    // --- Load Target Registers (PID Affinity) ---
    clearTargetRegisters();
    if (config["targetRegisters"].is<JsonArray>())
    {
        JsonArray targetArray = config["targetRegisters"].as<JsonArray>();
        uint8_t numTargets = min((uint8_t)targetArray.size(), (uint8_t)TEMP_PROFILE_MAX_TARGET_REGS);
        if (numTargets > 0)
        {
            for (uint8_t i = 0; i < numTargets; ++i)
            {
                uint16_t targetReg = targetArray[i].as<uint16_t>();
                if (targetReg > 0)
                {
                    setTargetRegister(i, targetReg);
                }
            }
        }
        if (targetArray.size() > TEMP_PROFILE_MAX_TARGET_REGS)
        {
            Log.warningln("%s: Config specified %d targets, but limited to %d.", name.c_str(), targetArray.size(), TEMP_PROFILE_MAX_TARGET_REGS);
        }
    }
    // Load associated signal plot slot ID if present
    if (config["signalPlot"].is<int>()) 
    {
        _signalPlotSlotId = config["signalPlot"].as<short>();
    }
    else
    {
        _signalPlotSlotId = -1; 
    }

    return true;
}
int16_t TemperatureProfile::getTemperature(uint32_t elapsedMs) const
{
    const float X_SCALE_MULTIPLIER = static_cast<float>(PROFILE_SCALE) / 1000.0f;
    if (static_cast<PlotStatus>(_statusWrapper) != PlotStatus::RUNNING || _numControlPoints < 2)
    {
        return 0;
    }

    uint32_t lastPointTimeMs = static_cast<uint32_t>(((uint64_t)(_controlPoints[_numControlPoints - 1].x * X_SCALE_MULTIPLIER) * (uint64_t)_durationMs) / PROFILE_SCALE);
    if (elapsedMs >= lastPointTimeMs)
    {
        int16_t normalizedTemp = _controlPoints[_numControlPoints - 1].y;
        int32_t temp_scaled_by_10 = ((int64_t)normalizedTemp * (int32_t)max * 10) / PROFILE_SCALE;
        if (temp_scaled_by_10 > INT16_MAX) temp_scaled_by_10 = INT16_MAX;
        if (temp_scaled_by_10 < 0) temp_scaled_by_10 = 0; // Assuming temperature doesn't go negative, adjust if it can
        return static_cast<int16_t>(temp_scaled_by_10);
    }

    uint8_t segmentIndex = 1; 
    while (segmentIndex < _numControlPoints)
    {
        uint32_t pointTimeMs = static_cast<uint32_t>(((uint64_t)(_controlPoints[segmentIndex].x * X_SCALE_MULTIPLIER) * (uint64_t)_durationMs) / PROFILE_SCALE);
        if (elapsedMs < pointTimeMs)
        {
            break;
        }
        segmentIndex++;
    }
    const TempControlPoint &p0 = _controlPoints[segmentIndex - 1];
    const TempControlPoint &p1 = _controlPoints[segmentIndex];

    uint32_t segmentStartTime = static_cast<uint32_t>(((uint64_t)(p0.x * X_SCALE_MULTIPLIER) * (uint64_t)_durationMs) / PROFILE_SCALE);
    uint32_t segmentEndTime = static_cast<uint32_t>(((uint64_t)(p1.x * X_SCALE_MULTIPLIER) * (uint64_t)_durationMs) / PROFILE_SCALE);
    uint32_t segmentDuration = segmentEndTime - segmentStartTime;

    if (segmentDuration == 0)
    {
        int16_t normalizedTemp = p0.y;
        int32_t temp_scaled_by_10 = ((int64_t)normalizedTemp * (int32_t)max * 10) / PROFILE_SCALE;
        if (temp_scaled_by_10 > INT16_MAX) temp_scaled_by_10 = INT16_MAX;
        if (temp_scaled_by_10 < 0) temp_scaled_by_10 = 0;
        return static_cast<int16_t>(temp_scaled_by_10);
    }
    uint32_t timeInSegment = elapsedMs - segmentStartTime;
    uint16_t t_norm = static_cast<uint16_t>(((uint64_t)timeInSegment * (uint64_t)PROFILE_SCALE) / segmentDuration);
    int16_t normalizedTemp = lerp(p0.y, p1.y, t_norm);

    int32_t temp_scaled_by_10 = ((int64_t)normalizedTemp * (int32_t)max * 10) / PROFILE_SCALE;

    if (temp_scaled_by_10 > INT16_MAX) temp_scaled_by_10 = INT16_MAX;
    if (temp_scaled_by_10 < 0) temp_scaled_by_10 = 0; // Assuming temperature doesn't go negative
/*
    if (millis() - _lastLogMs > 2000) { 
        const_cast<TemperatureProfile*>(this)->_lastLogMs = millis(); 
        Log.infoln("[%s] getTemperature(%lu ms): numPoints=%d, running=%d, paused=%d, max=%d", 
                      name.c_str(), elapsedMs, _numControlPoints, _running, _paused, max);
        if (_numControlPoints > 0 && _numControlPoints <= MAX_TEMP_CONTROL_POINTS) { 
            for (uint8_t i = 0; i < _numControlPoints; ++i) {
                Log.infoln("  CP[%d]: x=%d, y=%d", i, _controlPoints[i].x, _controlPoints[i].y);
            }
        }
        Log.infoln("  Duration: %d ms", _durationMs);
        Log.infoln("  Elapsed: %d ms", elapsedMs);
        Log.infoln("  Remaining: %d ms", _durationMs - elapsedMs);
        Log.infoln("  Scaled Temp: %d", scaledTemp);
        Log.infoln("  Normalized Temp: %d", normalizedTemp);
        Log.infoln("  Time in Segment: %d ms", timeInSegment);
        Log.infoln("  Segment Start: %d ms", segmentStartTime);
        Log.infoln("  Segment End: %d ms", segmentEndTime);
        Log.infoln("  Segment Duration: %d ms", segmentDuration);
    }*/
    return static_cast<int16_t>(temp_scaled_by_10);
}
int16_t TemperatureProfile::lerp(int16_t y0, int16_t y1, uint16_t t) const
{
    // t is scaled 0-PROFILE_SCALE
    int32_t deltaY = (int32_t)y1 - (int32_t)y0;
    int32_t interpolated = (int32_t)y0 + ((int64_t)deltaY * t) / PROFILE_SCALE;
    return static_cast<int16_t>(interpolated);
}
int16_t TemperatureProfile::cubicBezier(int16_t y0, int16_t y1, int16_t y2, int16_t y3, uint16_t t_norm) const
{
    // t_norm is scaled 0-PROFILE_SCALE
    float t = (float)t_norm / (float)PROFILE_SCALE;
    if (t < 0.0f)
        t = 0.0f; // Ensure t is in [0, 1]
    if (t > 1.0f)
        t = 1.0f;
    float u = 1.0f - t;

    // Using float calculations for Bezier curve
    float u3 = u * u * u;
    float u2t = 3.0f * u * u * t;
    float ut2 = 3.0f * u * t * t;
    float t3 = t * t * t;

    float result = (u3 * y0) + (u2t * y1) + (ut2 * y2) + (t3 * y3);

    // Clamp result to the PROFILE_SCALE range before rounding
    if (result < 0.0f)
        result = 0.0f;
    if (result > (float)PROFILE_SCALE)
        result = (float)PROFILE_SCALE;

    return static_cast<int16_t>(round(result));
}
int16_t TemperatureProfile::cubicBezierInt(int16_t y0, int16_t y1, int16_t y2, int16_t y3, uint16_t t_norm) const
{
    // Ensure t_norm is within bounds (though uint16_t >= 0)
    if (t_norm > PROFILE_SCALE)
        t_norm = PROFILE_SCALE;

    int64_t t = t_norm;
    int64_t u = PROFILE_SCALE - t;

    // Calculate terms using int64_t to avoid overflow.
    // We apply scaling progressively to try and manage the magnitude.
    // Result needs to be eventually divided by PROFILE_SCALE^3.

    // Term 0: u^3 * y0
    int64_t term0 = u;                   // u
    term0 = (term0 * u) / PROFILE_SCALE; // u^2 / S
    term0 = (term0 * u) / PROFILE_SCALE; // u^3 / S^2
    term0 = (term0 * y0);

    // Term 1: 3 * u^2 * t * y1
    int64_t term1 = 3 * u;               // 3u
    term1 = (term1 * u) / PROFILE_SCALE; // 3u^2 / S
    term1 = (term1 * t) / PROFILE_SCALE; // 3u^2*t / S^2
    term1 = (term1 * y1);

    // Term 2: 3 * u * t^2 * y2
    int64_t term2 = 3 * u;               // 3u
    term2 = (term2 * t) / PROFILE_SCALE; // 3ut / S
    term2 = (term2 * t) / PROFILE_SCALE; // 3ut^2 / S^2
    term2 = (term2 * y2);

    // Term 3: t^3 * y3
    int64_t term3 = t;                   // t
    term3 = (term3 * t) / PROFILE_SCALE; // t^2 / S
    term3 = (term3 * t) / PROFILE_SCALE; // t^3 / S^2
    term3 = (term3 * y3);

    // Combine terms (already scaled by S^2 implicitly through divisions)
    int64_t result_scaled_by_S2 = term0 + term1 + term2 + term3;

    // Final division to get the result back to original scale
    // Add PROFILE_SCALE / 2 for rounding before integer division
    int16_t final_result = static_cast<int16_t>((result_scaled_by_S2 + (PROFILE_SCALE / 2)) / PROFILE_SCALE);

    // Clamp final result (although intermediate math should prevent exceeding bounds if inputs are valid)
    if (final_result < 0)
        final_result = 0;
    if (final_result > PROFILE_SCALE)
        final_result = PROFILE_SCALE;

    return final_result;
}
const TempControlPoint *TemperatureProfile::getTempControlPoints() const
{
    return _controlPoints;
}
uint8_t TemperatureProfile::getNumTempControlPoints() const
{
    return _numControlPoints;
}
short TemperatureProfile::status()
{
    uint32_t duration = getDuration();
    uint32_t remaining = getRemainingTime();
    PlotStatus status = getCurrentStatus();
    int16_t temp = getTemperature(getElapsedMs());

    Log.noticeln("  Status: %d (%s)", (int)status,
                 status == PlotStatus::IDLE ? "IDLE" : status == PlotStatus::RUNNING ? "RUNNING"
                                                   : status == PlotStatus::PAUSED    ? "PAUSED"
                                                                                     : "FINISHED");
    Log.noticeln("  Duration: %lu ms", duration);
    Log.noticeln("  Elapsed: %lu ms", duration - remaining);
    Log.noticeln("  Remaining: %lu ms", remaining);
    Log.noticeln("  Current Temp (scaled): %d", temp);
    return E_OK;
}
short TemperatureProfile::serial_register(Bridge *bridge)
{
    bridge->registerMemberFunction(id, this, C_STR("status"), (ComponentFnPtr)&TemperatureProfile::status);
    bridge->registerMemberFunction(id, this, C_STR("start"), (ComponentFnPtr)&TemperatureProfile::start);
    bridge->registerMemberFunction(id, this, C_STR("pause"), (ComponentFnPtr)&TemperatureProfile::pause);
    bridge->registerMemberFunction(id, this, C_STR("stop"), (ComponentFnPtr)&TemperatureProfile::stop);
    bridge->registerMemberFunction(id, this, C_STR("resume"), (ComponentFnPtr)&TemperatureProfile::resume);
    return E_OK;
}
ModbusBlockView *TemperatureProfile::mb_tcp_blocks() const
{
    return const_cast<ModbusBlockView *>(&_modbusBlockView);
}
void TemperatureProfile::mb_tcp_register(ModbusTCP *manager)
{
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS)) return;

    const_cast<TemperatureProfile *>(this)->modbusTCP = manager;
    uint16_t instanceCalculatedBaseAddr = MB_HREG_TEMP_PROFILE_BASE + (this->slot * TEMP_PROFILE_REGISTER_COUNT);
    Log.verboseln("TempProf Slot %d: mb_tcp_register. CompID: %d. InstanceBaseAddr: %d. Name: %s", 
                  this->slot, this->id, instanceCalculatedBaseAddr, this->name.c_str());

    ModbusBlockView *blocksView = mb_tcp_blocks(); 
    Component *thiz = const_cast<TemperatureProfile *>(this);

    for (int i = 0; i < blocksView->count; ++i)
    {
        MB_Registers info = blocksView->data[i]; 
        info.componentId = this->id;
        uint16_t absoluteAddressForThisRegister = instanceCalculatedBaseAddr + info.startAddress; 

        Log.verboseln("  Registering for %s (Slot %d): RegName='%s', OffsetInBlock=%d. Calculated AbsoluteAddr=%d", 
                      this->name.c_str(), this->slot, info.name, info.startAddress, absoluteAddressForThisRegister);
        
        manager->registerModbus(thiz, info); 
    }
}
uint16_t TemperatureProfile::mb_tcp_base_address() const
{
    return MB_HREG_TEMP_PROFILE_BASE + (this->slot * TEMP_PROFILE_REGISTER_COUNT); 
}
short TemperatureProfile::mb_tcp_read(MB_Registers *reg)
{
    uint32_t val32 = 0;
    short requestedAddress = reg->startAddress;
    const uint16_t instanceBaseAddr = MB_HREG_TEMP_PROFILE_BASE + (this->slot * TEMP_PROFILE_REGISTER_COUNT);
    const uint16_t instanceEndAddr = instanceBaseAddr + TEMP_PROFILE_REGISTER_COUNT; 

    short offset = requestedAddress - instanceBaseAddr;

    if (requestedAddress < instanceBaseAddr || requestedAddress >= instanceEndAddr)
    {
        return 0xFFFF; 
    }

    TemperatureProfileRegisterOffset regOffset = static_cast<TemperatureProfileRegisterOffset>(offset);
    switch (regOffset) 
    {
    case TemperatureProfileRegisterOffset::STATUS:
        return static_cast<short>(static_cast<PlotStatus>(_statusWrapper));
    case TemperatureProfileRegisterOffset::CURRENT_TEMP:
        return getTemperature(getElapsedMs());
    case TemperatureProfileRegisterOffset::DURATION:
        val32 = getDuration();
        return (uint16_t)HIGH_WORD(val32);
    case TemperatureProfileRegisterOffset::ELAPSED_LW:
        val32 = getElapsedMs();
        return (uint16_t)LOW_WORD(val32);
    case TemperatureProfileRegisterOffset::ELAPSED_HW:
        val32 = getElapsedMs();
        return (uint16_t)HIGH_WORD(val32);
    case TemperatureProfileRegisterOffset::REMAINING:
        val32 = getRemainingTime();
        return (uint16_t)LOW_WORD(val32);
    case TemperatureProfileRegisterOffset::COMMAND:
        return 0xFFFF; 
    case TemperatureProfileRegisterOffset::ENABLE_CMD:
        return enabled() ? 1 : 0; 
    default:
        Log.errorln("%s: Read default case for offset %d (addr %d). Logic error.", name.c_str(), offset, requestedAddress);
        return 0xFFFF; 
    }
}
short TemperatureProfile::mb_tcp_write(MB_Registers *reg, short value)
{
    short requestedAddress = reg->startAddress;

    const uint16_t instanceBaseAddr = MB_HREG_TEMP_PROFILE_BASE + (this->slot * TEMP_PROFILE_REGISTER_COUNT);
    const uint16_t instanceEndAddr = instanceBaseAddr + TEMP_PROFILE_REGISTER_COUNT;
    short offset = requestedAddress - instanceBaseAddr;

    if (requestedAddress < instanceBaseAddr || requestedAddress >= instanceEndAddr)
    {
        Log.warningln("%s (Slot %d, ID %d): Write req for addr %d OUTSIDE instance block [%d - %d). Offset: %d. RegName: %s", 
                      this->name.c_str(), this->slot, this->id, requestedAddress, instanceBaseAddr, instanceEndAddr -1 , offset, reg->name);
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS; 
    }

    TemperatureProfileRegisterOffset regOffset = static_cast<TemperatureProfileRegisterOffset>(offset);

    switch (regOffset)
    {
    case TemperatureProfileRegisterOffset::COMMAND:
        Log.infoln("%s: Received command via Modbus (Offset %d, Addr %d): %d", name.c_str(), offset, requestedAddress, value);
        switch (static_cast<TemperatureProfileCommand>(value))
        {
        case TemperatureProfileCommand::START:
            this->start();
            break;
        case TemperatureProfileCommand::STOP:
            this->stop();
            break;
        case TemperatureProfileCommand::PAUSE:
            this->pause();
            break;
        case TemperatureProfileCommand::RESUME:
            this->resume();
            break;
        default:
            Log.warningln("%s: Invalid command value %d for COMMAND register.", name.c_str(), value);
            return MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        return E_OK;
    case TemperatureProfileRegisterOffset::ENABLE_CMD:
        Log.infoln("%s: Received enable/disable (Offset %d, Addr %d): %d", name.c_str(), offset, requestedAddress, value);
        if (value) 
        {
            this->enable();
        } 
        else 
        {
            this->disable();
        }
        return E_OK;
    default:
        Log.errorln("%s: Write default case for offset %d (Addr %d). Logic error.", name.c_str(), offset, requestedAddress);
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }
}
bool TemperatureProfile::getCurrentControlPointInfo(uint8_t &outId, uint32_t &outTimeMs, int16_t &outValue, int16_t &outUser) const
{
    if (static_cast<PlotStatus>(_statusWrapper) != PlotStatus::RUNNING || _numControlPoints < 2)
    { // Need >= 2 points for a segment
        return false;
    }

    uint32_t elapsedMs = getElapsedMs();

    // Find the segment
    uint8_t segmentIndex = 1; // Index of the *end* point of the segment
    while (segmentIndex < _numControlPoints)
    {
        // Calculate the time for the current segment end point
        uint32_t pointTimeMs = static_cast<uint32_t>(((uint64_t)_controlPoints[segmentIndex].x * (uint64_t)_durationMs) / PROFILE_SCALE);
        if (elapsedMs < pointTimeMs)
        {
            // Found the segment: elapsedMs is before this point's time
            break;
        }
        segmentIndex++;
    }

    // If elapsedMs is beyond or exactly at the last point's time
    // Check against the calculated time of the last point
    uint32_t lastPointTimeMs = static_cast<uint32_t>(((uint64_t)_controlPoints[_numControlPoints - 1].x * (uint64_t)_durationMs) / PROFILE_SCALE);
    if (segmentIndex >= _numControlPoints || elapsedMs >= lastPointTimeMs)
    {
        return _controlPoints[_numControlPoints - 1].y; // Return last point's value
    }

    // Now, the segment is between controlPoints[segmentIndex - 1] and controlPoints[segmentIndex].
    const TempControlPoint &p0 = _controlPoints[segmentIndex - 1];
    const TempControlPoint &p1 = _controlPoints[segmentIndex];

    // Calculate segment times based on x values
    uint32_t segmentStartTime = static_cast<uint32_t>(((uint64_t)p0.x * (uint64_t)_durationMs) / PROFILE_SCALE);
    uint32_t segmentEndTime = static_cast<uint32_t>(((uint64_t)p1.x * (uint64_t)_durationMs) / PROFILE_SCALE);
    uint32_t segmentDuration = segmentEndTime - segmentStartTime;

    // Handle coincident points (based on calculated time)
    if (segmentDuration == 0)
    {
        // Use value of the point at the start, but scale it before returning
        int16_t normalizedTemp = p0.y;
        int32_t scaledTemp = ((int32_t)normalizedTemp * (int32_t)max) / PROFILE_SCALE;
        if (scaledTemp > INT16_MAX)
            scaledTemp = INT16_MAX;
        if (scaledTemp < INT16_MIN)
            scaledTemp = INT16_MIN;
        return static_cast<int16_t>(scaledTemp);
    }
    // Calculate progress within the segment (0-PROFILE_SCALE)
    uint32_t timeInSegment = elapsedMs - segmentStartTime;
    uint16_t t_norm = static_cast<uint16_t>(((uint64_t)timeInSegment * (uint64_t)PROFILE_SCALE) / segmentDuration);
    return lerp(p0.y, p1.y, t_norm);
}

void TemperatureProfile::enable()
{
    PlotBase::enable();
    for (int i = 0; i < MAX_PLOTS; ++i) {
        if (_plots[i]) {
            _plots[i]->enable();
        }
    }
}

void TemperatureProfile::disable()
{
    PlotBase::disable();
    for (int i = 0; i < MAX_PLOTS; ++i) {
        if (_plots[i]) {
            _plots[i]->disable();
        }
    }
}

#endif // ENABLE_PROFILE_TEMPERATURE

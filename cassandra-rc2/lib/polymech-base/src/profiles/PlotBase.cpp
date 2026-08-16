#include "PlotBase.h"
#include "xmath.h"

//------------------------------------------------------------------------------
// Base Class: PlotBase Implementation
//------------------------------------------------------------------------------
short PlotBase::start()
{
    if (_durationMs > 0)
    {
        _startTimeMs = millis();
        _elapsedMsAtPause = 0;
        _running = true;
        _paused = false;
        _explicitlyStopped = false;
        onStart();
    }
    else
    {
        _running = false;
        _paused = false;
        _explicitlyStopped = false;
    }
    return E_OK;
}

short PlotBase::onReady()
{
    setStatus(PlotStatus::RUNNING);
    seek(0);
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i] != nullptr)
        {
            _plots[i]->start();
        }
    }
    return E_OK;
}

short PlotBase::stop()
{
    _running = false;
    _paused = false;
    _explicitlyStopped = true; // Mark as explicitly stopped
    _elapsedMsAtPause = 0;
    _startTimeMs = 0; // Reset startTime to ensure IDLE/STOPPED state if duration not met
    onStop();
    return E_OK;
}

short PlotBase::pause()
{
    if (_running && !_paused)
    {
        uint32_t now = millis();
        uint32_t currentElapsed = now - _startTimeMs;
        _elapsedMsAtPause = min(currentElapsed, _durationMs);
        _paused = true;
        _explicitlyStopped = false;
        onPause();
    }
    return E_OK;
}

short PlotBase::resume()
{
    if (_running && _paused)
    {
        uint32_t now = millis();
        _startTimeMs = now - _elapsedMsAtPause;
        _paused = false;
        _explicitlyStopped = false;
        onResume();
    }
    return E_OK;
}

short PlotBase::loop()
{
    PlotStatus currentStatus = getCurrentStatus();
    m_status.update(currentStatus);
    m_duration.update(getDuration() / 1000);
    if (currentStatus == PlotStatus::RUNNING)
    {
        m_currentValue.update(getValue(getElapsedMs()));
        m_remaining.update(getRemainingTime() / 1000);
        m_elapsed.update(getElapsedMs() / 1000);
    }
    return NetworkComponent::loop();
}

void PlotBase::seek(uint32_t targetMs)
{
    // Clamp target time to valid range
    if (targetMs > _durationMs)
    {
        L_INFO(F("PlotBase::seek() - Target time %d exceeds duration %d. Clamping to duration."), targetMs, _durationMs);
        targetMs = _durationMs;
    }

    if (!_running)
    {
        // Don't allow seeking if the plot hasn't even been started.
        // Or maybe set _elapsedMsAtPause and allow start() to pick it up?
        // For now, do nothing if IDLE.
        return;
    }

    if (_paused)
    {
        // If paused, just update the stored pause time.
        // resume() will use this value later.
        _elapsedMsAtPause = targetMs;
    }
    else
    {
        // If running, adjust the start time to reflect the seek.
        uint32_t now = millis();
        if (now >= targetMs)
        {
            _startTimeMs = now - targetMs;
        }
        else
        {
            _startTimeMs = ULONG_MAX - (targetMs - now - 1);
        }
    }

    // Recursively seek children
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i] != nullptr)
        {
            _plots[i]->seek(targetMs);
        }
    }
}

void PlotBase::slipTime(uint32_t deltaMs)
{
    if (_running && !_paused)
    {
        _startTimeMs += deltaMs;
    }
    // Recursively slip children
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i] != nullptr)
        {
            _plots[i]->slipTime(deltaMs);
        }
    }
}

uint32_t PlotBase::getElapsedMs() const
{
    if (_paused)
    {
        // If paused, return the time elapsed when pause was called
        return _elapsedMsAtPause;
    }

    if (!_running)
    {
        return 0; // Not running, not paused -> 0 elapsed
    }

    // Running and not paused: calculate current elapsed time
    uint32_t currentTime = millis();
    uint32_t elapsedMs = currentTime - _startTimeMs;

    // Clamp to duration
    return min(elapsedMs, _durationMs);
}

uint32_t PlotBase::getRemainingTime() const
{
    if (!_running)
    {
        return 0; // Not running, no time remaining
    }
    uint32_t elapsed = getElapsedMs(); // Already clamped to duration
    if (elapsed >= _durationMs)
    {
        return 0; // Already finished
    }
    return _durationMs - elapsed;
}

PlotStatus PlotBase::getCurrentStatus() const
{
    if (_explicitlyStopped)
    {
        return PlotStatus::STOPPED; // Highest priority: if explicitly stopped, it's STOPPED
    }

    if (m_status.getValue() == PlotStatus::INITIALIZING)
    {
        return PlotStatus::INITIALIZING;
    }

    if (_durationMs == 0)
    {
        return PlotStatus::IDLE;
    }

    uint32_t currentTime = millis();
    uint32_t elapsedMsUnclamped = 0;
    bool hasProgressed = (_startTimeMs != 0 || _elapsedMsAtPause > 0);

    if (!hasProgressed && !_running)
    {
        return PlotStatus::IDLE;
    }

    if (_paused)
    {
        elapsedMsUnclamped = _elapsedMsAtPause;
    }
    else if (_running)
    {
        elapsedMsUnclamped = millis() - _startTimeMs;
    }
    else
    {
        if (_startTimeMs != 0)
        {
            elapsedMsUnclamped = millis() - _startTimeMs;
        }
        else
        {
            elapsedMsUnclamped = _elapsedMsAtPause;
        }
    }
    if (elapsedMsUnclamped >= _durationMs && _durationMs > 0)
    {
        return PlotStatus::FINISHED;
    }
    if (_paused)
    {
        return PlotStatus::PAUSED;
    }
    if (_waiting && _running)
    {
        return PlotStatus::WAITING;
    }
    if (_running)
    {
        return PlotStatus::RUNNING;
    }

    return PlotStatus::IDLE;
}

bool PlotBase::addPlot(PlotBase *plot)
{
    if (!plot)
        return false;

    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (!_plots[i])
        {
            _plots[i] = plot;
            plot->setParent(this);
            plot->setDuration(_durationMs);
            return true;
        }
    }
    return false;
}

PlotBase *PlotBase::getPlot(uint8_t index) const
{
    if (index < MAX_PLOTS)
    {
        return _plots[index];
    }
    return nullptr;
}

void PlotBase::clear()
{
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i])
        {
            _plots[i]->setParent(nullptr);
            _plots[i] = nullptr;
        }
    }
}

void PlotBase::onStart()
{
    resolvePlots();
    if (_eventsDelegate)
    {
        switch (_plotType)
        {
        case PlotType::Temperature:
            _eventsDelegate->onTemperatureProfileStarted(this);
            break;
        case PlotType::Signal:
            _eventsDelegate->onSignalPlotStarted(this);
            break;
        case PlotType::Pressure:
            _eventsDelegate->onPressureProfileStarted(this);
            break;
        default:
            break;
        }
    }

    // If the profile is now in INITIALIZING state (e.g., waiting for warmup),
    // don't start the child plots yet. They will be started by the application
    // logic once the warmup/initialization is complete.
    if (getCurrentStatus() == PlotStatus::INITIALIZING)
    {
        return;
    }
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i] != nullptr)
        {
            _plots[i]->start();
        }
    }
}
short PlotBase::startPlots()
{
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i] != nullptr)
        {
            _plots[i]->start();
        }
    }
    return E_OK;
}
void PlotBase::onStop()
{
    if (_eventsDelegate)
    {
        switch (_plotType)
        {
        case PlotType::Temperature:
            _eventsDelegate->onTemperatureProfileStopped(this);
            break;
        case PlotType::Signal:
            _eventsDelegate->onSignalPlotStopped(this);
            break;
        case PlotType::Pressure:
            _eventsDelegate->onPressureProfileStopped(this);
            break;
        default:
            break;
        }
    }
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i] != nullptr)
        {
            _plots[i]->stop();
        }
    }
}

void PlotBase::onPause()
{
    if (_eventsDelegate)
    {
        switch (_plotType)
        {
        case PlotType::Temperature:
            _eventsDelegate->onTemperatureProfilePaused(this);
            break;
        case PlotType::Signal:
            _eventsDelegate->onSignalPlotPaused(this);
            break;
        case PlotType::Pressure:
            _eventsDelegate->onPressureProfilePaused(this);
            break;
        default:
            break;
        }
    }
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i] != nullptr)
        {
            _plots[i]->pause();
        }
    }
    if (getParent())
    {
        getParent()->pause();
    }
}

void PlotBase::onResume()
{
    if (_eventsDelegate)
    {
        switch (_plotType)
        {
        case PlotType::Temperature:
            _eventsDelegate->onTemperatureProfileResumed(this);
            break;
        case PlotType::Signal:
            _eventsDelegate->onSignalPlotResumed(this);
            break;
        case PlotType::Pressure:
            _eventsDelegate->onPressureProfileResumed(this);
            break;
        default:
            break;
        }
    }
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i] != nullptr)
        {
            _plots[i]->resume();
        }
    }
}

void PlotBase::onFinished()
{
    if (_eventsDelegate)
    {
        switch (_plotType)
        {
        case PlotType::Temperature:
            _eventsDelegate->onTemperatureProfileFinished(this);
            break;
        case PlotType::Signal:
            _eventsDelegate->onSignalPlotFinished(this);
            break;
        case PlotType::Pressure:
            _eventsDelegate->onPressureProfileFinished(this);
            break;
        default:
            break;
        }
    }
}

void PlotBase::setElapsedTime(uint32_t ms)
{
    _running = true;
    _paused = true;
    _explicitlyStopped = false;
    _elapsedMsAtPause = ms > _durationMs ? _durationMs : ms;
}

void PlotBase::_initializeControlPoints()
{
    _numControlPoints = 0;
    for (int i = 0; i < MAX_CONTROL_POINTS; ++i)
    {
        _controlPoints[i] = {0, 0};
    }
}

bool PlotBase::setControlPoints(const ControlPoint points[], uint8_t numPoints, uint32_t durationMs)
{
    if (numPoints > MAX_CONTROL_POINTS)
    {
        // Or log an error
        return false;
    }
    _initializeControlPoints(); // Clear existing points
    _numControlPoints = numPoints;
    for (uint8_t i = 0; i < _numControlPoints; ++i)
    {
        _controlPoints[i] = points[i];
    }
    _durationMs = durationMs;
    return true;
}

const ControlPoint *PlotBase::getControlPoints() const
{
    return _controlPoints;
}

uint8_t PlotBase::getNumControlPoints() const
{
    return _numControlPoints;
}

int16_t PlotBase::getValue(uint32_t elapsedMs) const
{
    const float X_SCALE_MULTIPLIER = static_cast<float>(PROFILE_SCALE) / 1000.0f;
    if (getCurrentStatus() != PlotStatus::RUNNING || _numControlPoints < 2)
    {
        return 0;
    }

    uint32_t lastPointTimeMs = static_cast<uint32_t>(((uint64_t)(_controlPoints[_numControlPoints - 1].x * X_SCALE_MULTIPLIER) * (uint64_t)_durationMs) / PROFILE_SCALE);
    if (elapsedMs >= lastPointTimeMs)
    {
        int16_t normalizedValue = _controlPoints[_numControlPoints - 1].y;
        int32_t value_scaled = ((int64_t)normalizedValue * (int32_t)max * 10) / PROFILE_SCALE;
        value_scaled = clamp<int32_t>(value_scaled, 0, INT16_MAX);
        return static_cast<int16_t>(value_scaled);
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
    const ControlPoint &p0 = _controlPoints[segmentIndex - 1];
    const ControlPoint &p1 = _controlPoints[segmentIndex];

    uint32_t segmentStartTime = static_cast<uint32_t>(((uint64_t)(p0.x * X_SCALE_MULTIPLIER) * (uint64_t)_durationMs) / PROFILE_SCALE);
    uint32_t segmentEndTime = static_cast<uint32_t>(((uint64_t)(p1.x * X_SCALE_MULTIPLIER) * (uint64_t)_durationMs) / PROFILE_SCALE);
    uint32_t segmentDuration = segmentEndTime - segmentStartTime;

    if (segmentDuration == 0)
    {
        int16_t normalizedValue = p0.y;
        int32_t value_scaled = ((int64_t)normalizedValue * (int32_t)max * 10) / PROFILE_SCALE;
        value_scaled = clamp<int32_t>(value_scaled, 0, INT16_MAX);
        return static_cast<int16_t>(value_scaled);
    }
    uint32_t timeInSegment = elapsedMs - segmentStartTime;
    uint16_t t_norm = static_cast<uint16_t>(((uint64_t)timeInSegment * (uint64_t)PROFILE_SCALE) / segmentDuration);
    int16_t normalizedValue = lerp(p0.y, p1.y, t_norm);

    int32_t value_scaled = ((int64_t)normalizedValue * (int32_t)max * 10) / PROFILE_SCALE;

    value_scaled = clamp<int32_t>(value_scaled, 0, INT16_MAX);

    return static_cast<int16_t>(value_scaled);
}

bool PlotBase::getCurrentControlPointInfo(uint8_t &outId, uint32_t &outTimeMs, int16_t &outValue, int16_t &outUser) const
{
    if (getCurrentStatus() != PlotStatus::RUNNING || _numControlPoints < 2)
    {
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

    uint32_t lastPointTimeMs = static_cast<uint32_t>(((uint64_t)_controlPoints[_numControlPoints - 1].x * (uint64_t)_durationMs) / PROFILE_SCALE);
    if (segmentIndex >= _numControlPoints || elapsedMs >= lastPointTimeMs)
    {
        outValue = _controlPoints[_numControlPoints - 1].y;
        return true;
    }

    const ControlPoint &p0 = _controlPoints[segmentIndex - 1];
    const ControlPoint &p1 = _controlPoints[segmentIndex];

    uint32_t segmentStartTime = static_cast<uint32_t>(((uint64_t)p0.x * (uint64_t)_durationMs) / PROFILE_SCALE);
    uint32_t segmentEndTime = static_cast<uint32_t>(((uint64_t)p1.x * (uint64_t)_durationMs) / PROFILE_SCALE);
    uint32_t segmentDuration = segmentEndTime - segmentStartTime;

    if (segmentDuration == 0)
    {
        int16_t normalizedValue = p0.y;
        int32_t scaledValue = ((int32_t)normalizedValue * (int32_t)max) / PROFILE_SCALE;
        scaledValue = clamp<int32_t>(scaledValue, INT16_MIN, INT16_MAX);
        outValue = static_cast<int16_t>(scaledValue);
        return true;
    }
    // Calculate progress within the segment (0-PROFILE_SCALE)
    uint32_t timeInSegment = elapsedMs - segmentStartTime;
    uint16_t t_norm = static_cast<uint16_t>(((uint64_t)timeInSegment * (uint64_t)PROFILE_SCALE) / segmentDuration);
    outValue = lerp(p0.y, p1.y, t_norm);
    return true;
}

int16_t PlotBase::lerp(uint16_t y0, uint16_t y1, uint16_t t) const
{
    // t is scaled 0-PROFILE_SCALE
    int32_t deltaY = (int32_t)y1 - (int32_t)y0;
    int32_t interpolated = (int32_t)y0 + ((int64_t)deltaY * t) / PROFILE_SCALE;
    return static_cast<int16_t>(interpolated);
}
int16_t PlotBase::cubicBezier(uint16_t y0, uint16_t y1, uint16_t y2, uint16_t y3, uint16_t t_norm) const
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
int16_t PlotBase::cubicBezierInt(uint16_t y0, uint16_t y1, uint16_t y2, uint16_t y3, uint16_t t_norm) const
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

short PlotBase::mb_tcp_read(MB_Registers *reg)
{
    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED)
    {
        return result;
    }

    short requestedAddress = reg->startAddress;
    short offset = requestedAddress - _baseAddress;

    PlotBaseRegisterOffset regOffset = static_cast<PlotBaseRegisterOffset>(offset);

    switch (regOffset)
    {
    case PlotBaseRegisterOffset::STATUS:
        return static_cast<short>(m_status.getValue());
    case PlotBaseRegisterOffset::CURRENT_VALUE:
        return m_currentValue.getValue();
    case PlotBaseRegisterOffset::DURATION:
        return m_duration.getValue();
    case PlotBaseRegisterOffset::ELAPSED:
        return m_elapsed.getValue();
    case PlotBaseRegisterOffset::REMAINING:
        return m_remaining.getValue();
    default:
        break;
    }

    return E_NOT_IMPLEMENTED;
}

void PlotBase::printBlocks()
{
    Log.noticeln(F("--- Modbus Blocks for %s (ID: %d) ---"), name.c_str(), id);
    Log.noticeln(F("Total blocks registered: %d / %d"), _nextIndex, PLOT_BASE_BLOCK_COUNT);
    for (size_t i = 0; i < _nextIndex; ++i)
    {
        const MB_Registers &block = _modbusBlocks[i];
        Log.noticeln(F("  [%d] Address: %d, Name: %s"), i, block.startAddress, block.name);
    }
    Log.noticeln(F("------------------------------------"));
}

void PlotBase::mb_tcp_register(ModbusTCP *manager)
{
    NetworkComponent::mb_tcp_register(manager);
    this->modbusTCP = manager;
}

short PlotBase::mb_tcp_write(MB_Registers *reg, short value)
{
    short offset = reg->startAddress - _baseAddress;

    // First, check if this is a command write, which has special side-effects.
    if (offset == static_cast<short>(PlotBaseRegisterOffset::COMMAND))
    {
        m_command.update(value);
        switch (static_cast<PlotCommand>(value))
        {
        case PlotCommand::START:
            L_INFO(F("PlotBase::mb_tcp_write() - Starting plot %d."), slot);
            this->start();
            break;
        case PlotCommand::STOP:
            L_INFO(F("PlotBase::mb_tcp_write() - Stopping plot %d."), slot);
            this->stop();
            break;
        case PlotCommand::PAUSE:
            L_INFO(F("PlotBase::mb_tcp_write() - Pausing plot %d."), slot);
            this->pause();
            break;
        case PlotCommand::RESUME:
            L_INFO(F("PlotBase::mb_tcp_write() - Resuming plot %d."), slot);
            this->resume();
            break;
        default:
            Log.warningln("%s: Invalid command value %d for COMMAND register.", name.c_str(), value);
            return MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        return E_OK;
    }
    else if (offset == static_cast<short>(PlotBaseRegisterOffset::ELAPSED))
    {

        // Seek to the specified time in seconds
        this->seek(value * 1000);
        m_elapsed.update(value);
        return E_OK;
    }
    else if (offset == static_cast<short>(PlotBaseRegisterOffset::TIME_OVERRIDE))
    {
        int16_t currentOverride = m_timeOverride.getValue();
        int16_t newOverride = static_cast<int16_t>(value);
        int16_t deltaMinutes = newOverride - currentOverride;

        if (deltaMinutes != 0)
        {
            // Calculate delta in milliseconds
            int32_t deltaMs = (int32_t)deltaMinutes * 60000;
            slipTime(deltaMs);
            m_timeOverride.update(newOverride);
            Log.noticeln("PlotBase %s: Time override applied. Delta: %d min, New Override: %d min", name.c_str(), deltaMinutes, newOverride);
        }
        return E_OK;
    }

    // For any other write, let the base NetworkComponent handle it.
    // This will find the corresponding NetworkValue and update it.
    return NetworkComponent::mb_tcp_write(reg, value);
}

short PlotBase::setup()
{

    NetworkComponent::setup();

    const char *group = getModbusGroupName();

    m_status.initModbus(_baseAddress + (ushort)PlotBaseRegisterOffset::STATUS, 1, this->id, this->slaveId, E_FN_CODE::FN_READ_HOLD_REGISTER, "Status(0:IDLE, 1:INITIALIZING, 2:RUNNING, 3:PAUSED, 4:STOPPED, 5:FINISHED, 6:WAITING)", group);
    m_status.initNotify(PlotStatus::IDLE, PlotStatus::IDLE, NetworkValue_ThresholdMode::DIFFERENCE);
    registerBlock(m_status.getRegisterInfo());

    m_currentValue.initModbus(_baseAddress + (ushort)PlotBaseRegisterOffset::CURRENT_VALUE, 1, this->id, this->slaveId, E_FN_CODE::FN_READ_HOLD_REGISTER, "CurrentValue", group);
    m_currentValue.initNotify(0, 5, NetworkValue_ThresholdMode::DIFFERENCE);
    registerBlock(m_currentValue.getRegisterInfo());

    m_duration.initModbus(_baseAddress + (ushort)PlotBaseRegisterOffset::DURATION, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_HOLD_REGISTER, "Duration", group);
    m_duration.initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    registerBlock(m_duration.getRegisterInfo());

    m_timeOverride.initModbus(_baseAddress + (ushort)PlotBaseRegisterOffset::TIME_OVERRIDE, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_HOLD_REGISTER, "TimeOverride", group);
    m_timeOverride.initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    registerBlock(m_timeOverride.getRegisterInfo());
    m_timeOverride.update(0);

    m_elapsed.initModbus(_baseAddress + (ushort)PlotBaseRegisterOffset::ELAPSED, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_HOLD_REGISTER, "Elapsed", group);
    m_elapsed.initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    registerBlock(m_elapsed.getRegisterInfo());

    m_remaining.initModbus(_baseAddress + (ushort)PlotBaseRegisterOffset::REMAINING, 1, this->id, this->slaveId, E_FN_CODE::FN_READ_HOLD_REGISTER, "Remaining", group);
    m_remaining.initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    registerBlock(m_remaining.getRegisterInfo());

    m_command.initModbus(_baseAddress + (ushort)PlotBaseRegisterOffset::COMMAND, 1, this->id, this->slaveId, E_FN_CODE::FN_WRITE_HOLD_REGISTER, "Command", group);
    m_command.initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    registerBlock(m_command.getRegisterInfo());
    m_command.update(0);

    return E_OK;
}

bool PlotBase::resolvePlots()
{
    for (uint8_t i = 0; i < MAX_PLOTS; ++i)
    {
        if (children[i] != 0 && owner)
        {
            Component *comp = owner->byId(children[i]);
            if (comp)
            {
                if (comp->type == COMPONENT_TYPE_PLOT)
                {
                    PlotBase *plot = static_cast<PlotBase *>(comp);
                    addPlot(plot);
                }
                else
                {
                    Log.warningln("PlotBase::setup - Child %d is not a plot for %s (ID: %d)", children[i], name.c_str(), id);
                }
            }
            else
            {
                Log.warningln("PlotBase::setup - Child %d is not found for %s (ID: %d) : invalid component", children[i], name.c_str(), id);
            }
        }
    }
    return true;
}

bool PlotBase::load(const JsonObject &config)
{
    if (config["name"].is<String>())
    {
        name = config["name"].as<String>();
    }
    if (config["enabled"].is<bool>())
    {
        enable(config["enabled"].as<bool>());
    }
    if (config["max"].is<uint32_t>())
    {
        max = config["max"].as<uint32_t>();
    }
    if (config["duration"].is<uint32_t>())
    {
        _durationMs = config["duration"].as<uint32_t>();
        m_duration.update(_durationMs / 1000);
    }
    if (config["children"].is<JsonArray>())
    {

        for (uint8_t i = 0; i < MAX_PLOTS; ++i)
        {
            children[i] = 0;
        }
        JsonArray childrenArray = config["children"].as<JsonArray>();
        uint8_t numChildren = min((uint8_t)childrenArray.size(), (uint8_t)MAX_PLOTS);
        for (uint8_t i = 0; i < numChildren; ++i)
        {
            children[i] = childrenArray[i].as<ushort>();
        }
    }
    return true;
}

void PlotBase::enable(bool enabled)
{
    NetworkComponent::enable(enabled);
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i])
        {
            _plots[i]->enable(enabled);
        }
    }
}
void PlotBase::disable()
{
    Component::disable();
    for (int i = 0; i < MAX_PLOTS; ++i)
    {
        if (_plots[i])
        {
            _plots[i]->disable();
        }
    }
}
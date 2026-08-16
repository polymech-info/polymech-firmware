#include "PlotBase.h"

//------------------------------------------------------------------------------
// Base Class: PlotBase Implementation
//------------------------------------------------------------------------------

// Default implementations for Component methods if needed
// void PlotBase::setup() { /* Base setup if any */ }
// void PlotBase::loop() { /* Base loop if any */ }

bool PlotBase::loadFromJsonObject(const JsonObject& config) {
    // Reset state
    _running = false;
    _durationMs = 0;
    _startTimeMs = 0;

    // Assume 'duration' exists and is valid uint32_t > 0
    // Optional: Add check config.containsKey("duration") if data isn't guaranteed
    _durationMs = config["duration"].as<uint32_t>();
    if (_durationMs == 0) {
        // A plot with zero duration is generally invalid
        // Serial.println(F("[PlotBase] Error: Duration cannot be zero.")); // Optional logging
        return false;
    }

    // Call derived class implementation for specific fields
    return load(config);
}

void PlotBase::start() {
    if (_durationMs > 0) {
        _startTimeMs = millis();
        _elapsedMsAtPause = 0;
        _running = true;
        _paused = false;
        _explicitlyStopped = false; // Ensure explicitlyStopped is false when starting
        onStart();
    } else {
        _running = false;
        _paused = false;
        _explicitlyStopped = false; // Also false if start fails
    }
}

void PlotBase::stop() {
    _running = false;
    _paused = false;
    _explicitlyStopped = true; // Mark as explicitly stopped
    _elapsedMsAtPause = 0;
    _startTimeMs = 0;      // Reset startTime to ensure IDLE/STOPPED state if duration not met
    onStop();
}

void PlotBase::pause() {
    if (_running && !_paused) {
        uint32_t now = millis();
        uint32_t currentElapsed = 0;
        if (now >= _startTimeMs) {
            currentElapsed = now - _startTimeMs;
        } else { 
            currentElapsed = (ULONG_MAX - _startTimeMs) + now + 1;
        }
        _elapsedMsAtPause = min(currentElapsed, _durationMs);
        _paused = true;
        _explicitlyStopped = false; // Pausing implies it's not in a fully stopped state
        onPause();
    }
}

void PlotBase::resume() {
    if (_running && _paused) {
        uint32_t now = millis();
        if (now >= _elapsedMsAtPause) {
            _startTimeMs = now - _elapsedMsAtPause;
        } else { 
             _startTimeMs = ULONG_MAX - (_elapsedMsAtPause - now - 1);
        }
        _paused = false;
        _explicitlyStopped = false; // Resuming implies it's not stopped
        onResume();
    }
}

void PlotBase::seek(uint32_t targetMs) {
    // Clamp target time to valid range
    if (targetMs > _durationMs) {
        targetMs = _durationMs;
    }

    if (!_running) {
        // Don't allow seeking if the plot hasn't even been started.
        // Or maybe set _elapsedMsAtPause and allow start() to pick it up?
        // For now, do nothing if IDLE.
        return; 
    }

    if (_paused) {
        // If paused, just update the stored pause time.
        // resume() will use this value later.
        _elapsedMsAtPause = targetMs;
    } else {
        // If running, adjust the start time to reflect the seek.
        uint32_t now = millis();
        if (now >= targetMs) {
            _startTimeMs = now - targetMs;
        } else {
            _startTimeMs = ULONG_MAX - (targetMs - now - 1);
        }
    }
}

uint32_t PlotBase::getElapsedMs() const {
    if (_paused) {
        // If paused, return the time elapsed when pause was called
        return _elapsedMsAtPause;
    }
    
    if (!_running) {
        return 0; // Not running, not paused -> 0 elapsed
    }

    // Running and not paused: calculate current elapsed time
    uint32_t currentTime = millis();
    uint32_t elapsedMs = 0;

    // Handle millis() rollover
    if (currentTime >= _startTimeMs) {
        elapsedMs = currentTime - _startTimeMs;
    } else {
        // Rollover occurred
        elapsedMs = (ULONG_MAX - _startTimeMs) + currentTime + 1;
    }

    // Clamp to duration
    return min(elapsedMs, _durationMs);
}

uint32_t PlotBase::getRemainingTime() const {
    if (!_running) {
        return 0; // Not running, no time remaining
    }
    uint32_t elapsed = getElapsedMs(); // Already clamped to duration
    if (elapsed >= _durationMs) {
        return 0; // Already finished
    }
    return _durationMs - elapsed;
}

PlotStatus PlotBase::getCurrentStatus() const {
    if (_explicitlyStopped) {
        return PlotStatus::STOPPED; // Highest priority: if explicitly stopped, it's STOPPED
    }

    if (_durationMs == 0) { 
        return PlotStatus::IDLE;
    }

    uint32_t currentTime = millis();
    uint32_t elapsedMsUnclamped = 0;
    // Determine if the plot has ever been in a state where time could have progressed.
    // This includes being explicitly started (_startTimeMs != 0 after a start()), 
    // or having some time accumulated at pause (_elapsedMsAtPause > 0).
    bool hasProgressed = (_startTimeMs != 0 || _elapsedMsAtPause > 0);

    if (!hasProgressed && !_running) { // If it never started and isn't running, it's IDLE
        return PlotStatus::IDLE;
    }

    if (_paused) {
         elapsedMsUnclamped = _elapsedMsAtPause; 
    } else if (_running) {
         if (currentTime >= _startTimeMs) {
             elapsedMsUnclamped = currentTime - _startTimeMs;
         } else {
             elapsedMsUnclamped = (ULONG_MAX - _startTimeMs) + currentTime + 1;
         }
    } else { // Not running, not paused, not explicitly stopped - implies it was running then finished or stopped implicitly
         // This path is tricky if stop() doesn't record final elapsed time.
         // Using _startTimeMs relies on it reflecting the *start* of the last run segment.
         // If _startTimeMs is 0 (due to stop()), this path might not be hit if hasProgressed is false.
         // If _startTimeMs is non-zero (e.g. never stopped), calculate potential elapsed time.
         if (_startTimeMs != 0) { 
             if (currentTime >= _startTimeMs) {
                 elapsedMsUnclamped = currentTime - _startTimeMs;
             } else {
                 elapsedMsUnclamped = (ULONG_MAX - _startTimeMs) + currentTime + 1;
             }
         } else { // _startTimeMs is 0, likely due to stop(). Use _elapsedMsAtPause if it has a value.
            elapsedMsUnclamped = _elapsedMsAtPause; // This was set to 0 on stop(), so if stop() was last, this is 0.
         }
    }

    if (elapsedMsUnclamped >= _durationMs && _durationMs > 0) { // Ensure duration > 0 for FINISHED state
        return PlotStatus::FINISHED;
    }
    // After checking STOPPED and FINISHED:
    if (_paused) {
        return PlotStatus::PAUSED;
    }
    if (_running) {
        return PlotStatus::RUNNING;
    }
    
    return PlotStatus::IDLE; // Default if no other state matches
}

bool PlotBase::addPlot(PlotBase* plot) {
    if (!plot) return false;
    for (int i = 0; i < MAX_PLOTS; ++i) {
        if (!_plots[i]) {
            _plots[i] = plot;
            plot->setParent(this);
            plot->setDuration(_durationMs);
            return true;
        }
    }
    return false; // No available slot
}

PlotBase* PlotBase::getPlot(uint8_t index) const {
    if (index < MAX_PLOTS) {
        return _plots[index];
    }
    return nullptr;
}

void PlotBase::onStart() {
    for (int i = 0; i < MAX_PLOTS; ++i) {
        if (_plots[i]) {
            _plots[i]->start();
        }
    }
}

void PlotBase::onStop() {
    for (int i = 0; i < MAX_PLOTS; ++i) {
        if (_plots[i]) {
            _plots[i]->stop();
        }
    }
}

void PlotBase::onPause() {
    for (int i = 0; i < MAX_PLOTS; ++i) {
        if (_plots[i]) {
            _plots[i]->pause();
        }
    }
    if(getParent()) {
        getParent()->pause();
    }
}

void PlotBase::onResume() {
    for (int i = 0; i < MAX_PLOTS; ++i) {
        if (_plots[i]) {
            _plots[i]->resume();
        }
    }
} 
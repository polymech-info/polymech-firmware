#include "components/AmperageBudgetManager.h"
#include <vector> // Ensure vector is included
#include <pid_constants.h>

// Define a threshold (e.g., 5 minutes) after which we consider a device budget-limited
#define ERROR_STATE_THRESHOLD_MS (5 * 60 * 1000) 

AmperageBudgetManager::AmperageBudgetManager(uint32_t wattBudget, uint32_t minOnTimeMs, uint32_t maxContiguousRunTimeMs)
    : Component("AmperageBudgetManager"),
      _wattBudget(wattBudget),
      _minOnTimeMs(minOnTimeMs),
      _maxContiguousRunTimeMs(maxContiguousRunTimeMs),
      _numDevices(0),
      _nextDeviceIndex(0),
      _lastLoopTime(0) {
    // Initialize the managed devices array (optional, as default constructors handle it)
    for (uint8_t i = 0; i < MAX_MANAGED_DEVICES; ++i) {
        _managedDevices[i] = ManagedDevice{}; // Explicitly default construct
    }
}

bool AmperageBudgetManager::addManagedDevice(OmronE5* device, uint8_t priority) {
    if (_numDevices >= MAX_MANAGED_DEVICES) {
        Log.errorln(F("[%s] Cannot add more devices, manager full (%d)."), _name, MAX_MANAGED_DEVICES);
        return false;
    }
    if (device == nullptr) {
         Log.errorln(F("[%s] Cannot add null device pointer."), _name);
        return false;
    }

    // Check if device already added
    for(uint8_t i = 0; i < _numDevices; ++i) {
        if (_managedDevices[i].device == device) {
            Log.warningln(F("[%s] Device already added (Index %d). Ignoring."), _name, i);
            return true; // Or false depending on desired behavior
        }
    }

    _managedDevices[_numDevices].device = device;
    _managedDevices[_numDevices].state = ManagedState::UNKNOWN;
    _managedDevices[_numDevices].originalIndex = _numDevices;
    _managedDevices[_numDevices].priority = priority; // Store priority
    _managedDevices[_numDevices].lastGrantedTime = 0;
    _managedDevices[_numDevices].heatRequestStartTime = 0;
    _managedDevices[_numDevices].wantsHeat = false;
    // Get initial consumption - might be better to do this in setup/loop
    _managedDevices[_numDevices].consumption = device->getConsumption(); 

    Log.traceln(F("[%s] Added device %d (Priority: %d, Consumption: %d W)."), _name, _numDevices, priority, _managedDevices[_numDevices].consumption);
    _numDevices++;
    return true;
}

bool AmperageBudgetManager::setDevicePriority(OmronE5* device, uint8_t priority) {
    if (device == nullptr) return false;
    for (uint8_t i = 0; i < _numDevices; ++i) {
        if (_managedDevices[i].device == device) {
            _managedDevices[i].priority = priority;
            Log.infoln(F("[%s] Set priority for device %d to %d."), _name, _managedDevices[i].originalIndex, priority);
            return true;
        }
    }
    Log.warningln(F("[%s] setDevicePriority: Device not found."), _name);
    return false;
}

short AmperageBudgetManager::setup() {
    Log.infoln(F("[%s] Setting up with Budget: %d W, MinOnTime: %d ms, MaxContigRun: %d ms."), 
        _name, _wattBudget, _minOnTimeMs, _maxContiguousRunTimeMs);
    if (_numDevices == 0) {
        Log.warningln(F("[%s] No devices added to manage."), _name);
    }
    // Initial state update could happen here, but loop will handle it
    _lastLoopTime = millis();
    return 0; // Success
}

short AmperageBudgetManager::loop() {
    if (_numDevices == 0) {
        return 0; // Nothing to do
    }

    // Optional: Add a delay or check elapsed time to control loop frequency
    // millis_t currentTime = millis();
    // if (currentTime - _lastLoopTime < BUDGET_LOOP_INTERVAL_MS) { 
    //     return 0;
    // }
    // _lastLoopTime = currentTime;

    _updateDeviceStates();
    _allocateBudget();

    return 0; // Success
}

void AmperageBudgetManager::_updateDeviceStates() {
    millis_t currentTime = millis();
    for (uint8_t i = 0; i < _numDevices; ++i) {
        ManagedDevice& managed = _managedDevices[i];
        if (!managed.device) continue;

        uint16_t pv = 0, sp = 0;
        bool pvValid = managed.device->getPV(pv);
        bool spValid = managed.device->getSP(sp);
        managed.consumption = managed.device->getConsumption(); // Update consumption potentially

        if (pvValid && spValid) {
            bool previousWantsHeat = managed.wantsHeat;
            managed.wantsHeat = (pv < sp);

            // Track when heat request starts
            if (managed.wantsHeat && !previousWantsHeat) {
                managed.heatRequestStartTime = currentTime;
            } else if (!managed.wantsHeat) {
                managed.heatRequestStartTime = 0; // Reset if no longer wants heat
            }

            // State transitions based *only* on PV/SP (actual run/stop in _allocateBudget)
            if (!managed.wantsHeat) {
                // If it was heating or requesting, it now wants to stop.
                // Let _allocateBudget handle stopping based on min/max times.
                // If it wasn't heating/requesting, ensure it's IDLE.
                if (managed.state != ManagedState::HEATING && managed.state != ManagedState::REQUESTING_HEAT) {
                     managed.state = ManagedState::IDLE;
                }
            } else { // wantsHeat is true
                if (managed.state == ManagedState::IDLE || managed.state == ManagedState::UNKNOWN) {
                    managed.state = ManagedState::REQUESTING_HEAT;
                    Log.verboseln(F("[%s] Device %d (Pri: %d) requesting heat (PV:%d < SP:%d)."), 
                        _name, managed.originalIndex, managed.priority, pv, sp);
                } else if (managed.state == ManagedState::REQUESTING_HEAT) {
                    // Check if it's been requesting heat for too long without getting budget
                    if (managed.heatRequestStartTime > 0 && (currentTime - managed.heatRequestStartTime > ERROR_STATE_THRESHOLD_MS)) {
                        Log.warningln(F("[%s] Device %d (Pri: %d) timed out requesting heat. Moving to ERROR_BUDGET_LIMITED state."),
                            _name, managed.originalIndex, managed.priority);
                        managed.state = ManagedState::ERROR_BUDGET_LIMITED;
                    }
                } else if (managed.state == ManagedState::ERROR_BUDGET_LIMITED) {
                    // Remains in error state as long as it wants heat
                    // Could add logic here to potentially retry or alert
                }
            }
        } else {
            Log.warningln(F("[%s] Failed to read PV/SP for device %d."), _name, managed.originalIndex);
            managed.wantsHeat = false; // Assume doesn't want heat if reading fails
            managed.heatRequestStartTime = 0;
            // Could potentially set state to UNKNOWN or leave as is.
            // Setting to IDLE might be safest if reads fail.
            if (managed.state == ManagedState::HEATING || managed.state == ManagedState::ERROR_BUDGET_LIMITED) {
                 Log.warningln(F("[%s] Stopping device %d due to read failure while %s."), 
                    _name, managed.originalIndex, _getStateStr(managed.state));
                 managed.device->stop();
            }
            managed.state = ManagedState::IDLE; 
        }
    }
}

void AmperageBudgetManager::_allocateBudget() {
    uint32_t currentWattage = 0;
    std::vector<uint8_t> potentialRunnerIndices; // Indices of devices that *could* run
    std::vector<uint8_t> willRunIndices;         // Indices of devices that *will* run this cycle
    millis_t currentTime = millis();

    // --- Pass 1: Identify who *must* keep running (min on-time) and who *can* be stopped (max contiguous time) --- 
    for (uint8_t i = 0; i < _numDevices; ++i) {
        ManagedDevice& managed = _managedDevices[i];
        if (managed.state == ManagedState::HEATING) {
            bool minTimeMet = (currentTime - managed.lastGrantedTime >= _minOnTimeMs);
            bool maxTimeExceeded = (_maxContiguousRunTimeMs > 0) && (currentTime - managed.lastGrantedTime >= _maxContiguousRunTimeMs);

            if (managed.wantsHeat && !minTimeMet) { // Still wants heat, min time NOT met
                // MUST RUN (unless budget forces stop)
                if (currentWattage + managed.consumption <= _wattBudget) {
                    currentWattage += managed.consumption;
                    willRunIndices.push_back(managed.originalIndex); 
                    Log.verboseln(F("[%s] Device %d (Pri: %d) MUST run (MinOnTime). Budget: %d/%d W."), 
                        _name, managed.originalIndex, managed.priority, currentWattage, _wattBudget);
                } else {
                    Log.warningln(F("[%s] Budget %d W exceeded by forced run of Dev %d (Pri: %d, %d W). Stopping!."), 
                        _name, _wattBudget, managed.originalIndex, managed.priority, managed.consumption);
                    managed.device->stop(); 
                    managed.state = ManagedState::REQUESTING_HEAT; // Still wants heat but lost budget
                }
            } else if (managed.wantsHeat && maxTimeExceeded) { // Still wants heat, min time MET, max time EXCEEDED
                // CAN BE STOPPED for fairness, add to potential runners
                 Log.verboseln(F("[%s] Device %d (Pri: %d) preempted (MaxContigTime). Adding to potential."), 
                        _name, managed.originalIndex, managed.priority);
                 potentialRunnerIndices.push_back(managed.originalIndex);
                 // Don't stop it yet, let Pass 3 handle it if it doesn't get budget again
            } else if (managed.wantsHeat) { // Still wants heat, min time MET, max time NOT exceeded (or disabled)
                // CAN BE STOPPED, but prefers to run. Add to potential runners.
                potentialRunnerIndices.push_back(managed.originalIndex); 
            } else { // Does NOT want heat anymore (PV >= SP)
                 // Min time met doesn't matter if it doesn't want heat
                 Log.verboseln(F("[%s] Device %d (Pri: %d) stopping (PV >= SP)."), _name, managed.originalIndex, managed.priority);
                 managed.device->stop();
                 managed.state = ManagedState::IDLE;
            }
        } else if (managed.state == ManagedState::REQUESTING_HEAT && managed.wantsHeat) {
            potentialRunnerIndices.push_back(managed.originalIndex); // Wants heat, wasn't running.
        } else if (!managed.wantsHeat && (managed.state == ManagedState::REQUESTING_HEAT || managed.state == ManagedState::HEATING) ) {
             // This covers cases where it was requesting but PV rose, or it was heating but PV rose (handled above)
             if (managed.state == ManagedState::HEATING) { 
                 // Should have been stopped in the HEATING block above if PV rose
                 Log.warningln(F("[%s] Device %d logic error? Was HEATING but !wantsHeat here."),_name, managed.originalIndex);
                 managed.device->stop(); // Ensure stopped
             }
             managed.state = ManagedState::IDLE;
        }
    }

    // --- Pass 2: Allocate remaining budget using Priority + Round-Robin --- 
    uint8_t numPotential = potentialRunnerIndices.size();
    if (numPotential > 0) {
        
        // Find the highest priority (lowest number) among potential runners
        uint8_t highestPriority = std::numeric_limits<uint8_t>::max();
        for(uint8_t index : potentialRunnerIndices) {
            if (_managedDevices[index].priority < highestPriority) {
                highestPriority = _managedDevices[index].priority;
            }
        }

        // Create list of highest priority candidates for this cycle
        std::vector<uint8_t> highPriorityCandidates;
        for(uint8_t index : potentialRunnerIndices) {
            if (_managedDevices[index].priority == highestPriority) {
                highPriorityCandidates.push_back(index);
            }
        }

        // Sort the highest priority candidates using round-robin
         if (!highPriorityCandidates.empty()) {
            std::sort(highPriorityCandidates.begin(), highPriorityCandidates.end(), 
                [&](uint8_t a, uint8_t b) {
                    uint8_t effectiveA = (a >= _nextDeviceIndex) ? (a - _nextDeviceIndex) : (a + _numDevices - _nextDeviceIndex);
                    uint8_t effectiveB = (b >= _nextDeviceIndex) ? (b - _nextDeviceIndex) : (b + _numDevices - _nextDeviceIndex);
                    return effectiveA < effectiveB;
                });

            Log.verboseln(F("[%s] High Priority (%d) Candidates (RR start %d): "), _name, highestPriority, _nextDeviceIndex);
            // for(uint8_t idx : highPriorityCandidates) { Log.verboseln(F(" %d"), idx); } Log.verboseln("");

            // Try allocating budget to these high-priority, round-robin sorted candidates
            for (uint8_t originalIdx : highPriorityCandidates) {
                ManagedDevice& potential = _managedDevices[originalIdx];
                if (currentWattage + potential.consumption <= _wattBudget) {
                    // Check if not already running (from Pass 1 - min on time)
                    bool alreadyRunning = false;
                    for(uint8_t runningIdx : willRunIndices) { if (runningIdx == originalIdx) { alreadyRunning = true; break; } }
                    
                    if (!alreadyRunning) {
                        currentWattage += potential.consumption;
                        willRunIndices.push_back(originalIdx);
                        Log.verboseln(F("[%s] Device %d (Pri: %d) WILL run. Budget: %d/%d W."), 
                            _name, originalIdx, potential.priority, currentWattage, _wattBudget);
                    }
                } else {
                    Log.verboseln(F("[%s] Device %d (Pri: %d) cannot run (Budget %d/%d W, needs %d W)."), 
                        _name, originalIdx, potential.priority, currentWattage, _wattBudget, potential.consumption);
                }
            }
        }
        // Note: This implementation only allocates to the *highest* priority group requesting heat each cycle.
        // If budget remains after satisfying the highest priority group, lower priority groups 
        // will only get a chance in subsequent cycles when they become the highest priority *requesting* group.
    }

    // --- Pass 3: Apply changes - Start/Stop devices based on willRun list --- 
    for (uint8_t i = 0; i < _numDevices; ++i) {
        ManagedDevice& managed = _managedDevices[i];
        bool shouldBeRunning = false;
        for (uint8_t runIdx : willRunIndices) {
            if (managed.originalIndex == runIdx) {
                shouldBeRunning = true;
                break;
            }
        }

        if (shouldBeRunning) {
            if (managed.state != ManagedState::HEATING) {
                Log.infoln(F("[%s] Starting device %d (Pri: %d)."), _name, managed.originalIndex, managed.priority);
                if (managed.device->run()) { // Check if run command succeeded (optional)
                    managed.state = ManagedState::HEATING;
                    managed.lastGrantedTime = currentTime;
                } else {
                    Log.errorln(F("[%s] Failed to execute run() command for device %d!"), _name, managed.originalIndex);
                    // Keep state as REQUESTING_HEAT or move to UNKNOWN/ERROR?
                }
            }
            // If it was already HEATING, just let it continue (lastGrantedTime might update implicitly if needed later)
        } else { // Should NOT be running
            if (managed.state == ManagedState::HEATING) {
                 Log.infoln(F("[%s] Stopping device %d (Pri: %d, Budget revoked/expired/preempted)."), _name, managed.originalIndex, managed.priority);
                 if (managed.device->stop()) { // Check if stop command succeeded (optional)
                     managed.state = managed.wantsHeat ? ManagedState::REQUESTING_HEAT : ManagedState::IDLE;
                 } else {
                     Log.errorln(F("[%s] Failed to execute stop() command for device %d!"), _name, managed.originalIndex);
                     // State remains HEATING, potentially problematic. Maybe add an error state?
                 }
            } else if (managed.state == ManagedState::REQUESTING_HEAT && !managed.wantsHeat) {
                managed.state = ManagedState::IDLE;
            }
        }
    }

    // Update round-robin index for next cycle
    if (_numDevices > 0) {
         _nextDeviceIndex = (_nextDeviceIndex + 1) % _numDevices;
    }
}

short AmperageBudgetManager::info() {
    Log.notice(F("[%s] Budget:%d W, MinOn:%dms, MaxRun:%dms, Devs:%d/%d, RR_Idx:%d\n"), 
        _name, _wattBudget, _minOnTimeMs, _maxContiguousRunTimeMs, _numDevices, MAX_MANAGED_DEVICES, _nextDeviceIndex);
    uint32_t currentWattage = 0;
    uint32_t potentialWattage = 0;
    for (uint8_t i = 0; i < _numDevices; ++i) {
        const ManagedDevice& managed = _managedDevices[i];
        if (managed.state == ManagedState::HEATING) {
            currentWattage += managed.consumption;
        }
        if (managed.wantsHeat) {
            potentialWattage += managed.consumption;
        }
        millis_t timeSinceGrant = (managed.lastGrantedTime == 0) ? 0 : (millis() - managed.lastGrantedTime);
        millis_t timeSinceRequest = (managed.heatRequestStartTime == 0) ? 0 : (millis() - managed.heatRequestStartTime);

        Log.notice(F("  Dev %d: Pri=%d, State=%s, Wants=%T, Cons=%d W, LastRun=%lums ago, ReqFor=%lums\n"), 
            managed.originalIndex,
            managed.priority,
            _getStateStr(managed.state),
            managed.wantsHeat,
            managed.consumption, 
            timeSinceGrant,
            managed.wantsHeat ? timeSinceRequest : 0 // Only show request time if currently wants heat
            );
    }
     Log.notice(F("  Current Wattage: %d W | Potential Demand: %d W\n"), currentWattage, potentialWattage);
    return 0;
}

const char* AmperageBudgetManager::_getStateStr(ManagedState state) {
    switch (state) {
        case ManagedState::UNKNOWN:         return "UNKNOWN";
        case ManagedState::IDLE:            return "IDLE";
        case ManagedState::REQUESTING_HEAT: return "REQUESTING_HEAT";
        case ManagedState::HEATING:         return "HEATING";
        case ManagedState::ERROR_BUDGET_LIMITED: return "ERR_BUDGET_LTD"; // Added new state string
        default:                            return "INVALID";
    }
} 
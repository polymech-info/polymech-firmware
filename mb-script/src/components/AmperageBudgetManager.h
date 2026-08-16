#ifndef AMPERAGE_BUDGET_MANAGER_H
#define AMPERAGE_BUDGET_MANAGER_H

#include "config.h"
#include <Component.h>
#include <ArduinoLog.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <limits> 
#include "components/OmronE5.h"

// Define the maximum number of devices this manager can handle.
// Adjust based on expected system size and memory constraints.
#ifndef MAX_MANAGED_DEVICES
#define MAX_MANAGED_DEVICES 8
#endif

// Ensure these are defined in your config.h or secrets.h
#ifndef DEFAULT_POWER_BUDGET_WATTS
#define DEFAULT_POWER_BUDGET_WATTS 10000
#endif
#ifndef DEFAULT_MIN_ON_TIME_MS
#define DEFAULT_MIN_ON_TIME_MS 5000
#endif
#ifndef DEFAULT_MAX_CONTIGUOUS_RUN_TIME_MS
#define DEFAULT_MAX_CONTIGUOUS_RUN_TIME_MS 0 // 0 means disabled by default
#endif

// Define default priority - lower number is higher priority
#define DEFAULT_DEVICE_PRIORITY 10

class AmperageBudgetManager : public Component {
public:
    // Enum to track the state of managed devices from the budget manager's perspective.
    enum class ManagedState {
        UNKNOWN,            // Initial state before first check
        IDLE,               // PV >= SP, not requesting heat
        REQUESTING_HEAT,    // PV < SP, wants to heat but might not have budget yet
        HEATING,            // PV < SP and currently allocated budget (run() called)
        ERROR_BUDGET_LIMITED // PV < SP for extended period, but consistently denied budget
    };

    // Structure to hold information about each managed device.
    struct ManagedDevice {
        OmronE5* device = nullptr;
        ManagedState state = ManagedState::UNKNOWN;
        uint8_t originalIndex = 0; // Index when added, for stable sorting
        uint8_t priority = DEFAULT_DEVICE_PRIORITY; // Lower value = higher priority
        uint32_t lastGrantedTime = 0; // millis() when budget was last granted
        uint32_t heatRequestStartTime = 0; // millis() when wantsHeat first became true
        bool wantsHeat = false; // Cached desire based on PV/SP
        uint32_t consumption = 0; // Cached consumption
    };

    /**
     * @brief Constructor for the AmperageBudgetManager.
     * @param wattBudget The maximum total wattage allowed for active heaters.
     * @param minOnTimeMs Minimum time (milliseconds) a device should run once started.
     * @param maxContiguousRunTimeMs Max time (ms) a device can run continuously before being preempted for fairness (0=disabled).
     */
    AmperageBudgetManager(uint32_t wattBudget = DEFAULT_POWER_BUDGET_WATTS, 
                          uint32_t minOnTimeMs = DEFAULT_MIN_ON_TIME_MS,
                          uint32_t maxContiguousRunTimeMs = DEFAULT_MAX_CONTIGUOUS_RUN_TIME_MS);
    virtual ~AmperageBudgetManager() = default;

    /**
     * @brief Registers an OmronE5 device to be managed by the budget controller.
     * @param device Pointer to the OmronE5 instance.
     * @param priority Priority for budget allocation (lower number is higher priority).
     * @return True if successfully added, false if the manager is full.
     */
    bool addManagedDevice(OmronE5* device, uint8_t priority = DEFAULT_DEVICE_PRIORITY);

    /**
     * @brief Sets the priority for an already added device.
     * @param device Pointer to the OmronE5 instance.
     * @param priority The new priority value.
     * @return True if device was found and priority set, false otherwise.
     */
    bool setDevicePriority(OmronE5* device, uint8_t priority);

    // --- Component Interface --- 
    virtual short setup() override;
    virtual short loop() override;
    virtual short info() override; // For printing status

private:
    uint32_t _wattBudget;
    uint32_t _minOnTimeMs; // Minimum time to keep a heater on once started
    uint32_t _maxContiguousRunTimeMs; // Max time to run before preemption for fairness
    ManagedDevice _managedDevices[MAX_MANAGED_DEVICES];
    uint8_t _numDevices;
    uint8_t _nextDeviceIndex; // Index for round-robin starting point
    millis_t _lastLoopTime;
    String _name;

    // --- Internal Helper Methods ---

    /**
     * @brief Updates the state (wantsHeat, consumption) of all managed devices.
     */
    void _updateDeviceStates();

    /**
     * @brief Allocates the wattage budget to devices requesting heat.
     * Implements the round-robin scheduling with preemption and min-on-time.
     */
    void _allocateBudget();

    /**
     * @brief Gets the state of a managed device as a string.
     * @param state The state enum value.
     * @return A const char* representation of the state.
     */
    static const char* _getStateStr(ManagedState state);
};

#endif // AMPERAGE_BUDGET_MANAGER_H 
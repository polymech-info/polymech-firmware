#ifndef PLOT_BASE_H
#define PLOT_BASE_H

#include <stdint.h>
#include <ArduinoJson.h>
#include <Arduino.h> // For millis(), min()
#include <limits.h>  // For ULONG_MAX
#include <Component.h> 
#include "config.h"

#define PROFILE_SCALE 10000
#define PROFILE_TIME_SCALE 1000
#define MAX_PLOTS 1

//------------------------------------------------------------------------------
// Status Enum
//------------------------------------------------------------------------------
enum class PlotStatus {
    IDLE,     // Not started or stopped before completion
    RUNNING,  // Actively running
    PAUSED,   // Started, but currently paused
    STOPPED,  // Explicitly stopped by user/logic
    FINISHED  // Reached or exceeded duration
};

//------------------------------------------------------------------------------
// Base Class: PlotBase
//------------------------------------------------------------------------------

/**
 * @brief Base class for representing time-based signal plots.
 * Inherits from Component and handles common timeline aspects like duration,
 * running state, and loading the duration from JSON.
 */
class PlotBase : public Component { // Ensure inheritance is active
public:
    PlotBase(Component* owner, ushort componentId) : 
    Component("PlotBase", componentId, Component::COMPONENT_DEFAULT, owner),
    _durationMs(0), _startTimeMs(0), _elapsedMsAtPause(0), _running(false), _paused(false), _explicitlyStopped(false), _userData(nullptr),
    _parent(nullptr) {
        for (int i = 0; i < MAX_PLOTS; ++i) {
            _plots[i] = nullptr;
        }
    }
    
    virtual ~PlotBase() = default;
    /**
     * @brief Loads configuration from a JSON object.
     * Parses common field 'duration' and calls the pure virtual load.
     * Assumes the caller provides the correct JSON object for the specific derived type.
     * Assumes data is sanitized/valid as per user request.
     *
     * @param config The JsonObject containing the configuration for this plot.
     * @return true if duration parsing was okay and specific loading succeeded, false otherwise.
     */
    virtual bool loadFromJsonObject(const JsonObject& config);

    // --- Plot Control --- 

    /**
     * @brief Starts the plot execution timer.
     */
    virtual void start();

    /**
     * @brief Stops the plot execution timer and resets pause state.
     */
    virtual void stop();

    /**
     * @brief Pauses the plot execution timer if running.
     * Stores the elapsed time at the moment of pausing.
     */
    virtual void pause();

    /**
     * @brief Resumes the plot execution timer if paused.
     * Calculates a new start time based on the time elapsed before pausing.
     */
    virtual void resume();

    /**
     * @brief Sets the current position within the plot to a specific time.
     * If the plot is running, it adjusts the start time.
     * If the plot is paused, it adjusts the elapsed time stored at pause.
     * Does nothing if the plot is IDLE.
     * 
     * @param targetMs The target elapsed time in milliseconds to seek to.
     *                 Value will be clamped between 0 and the plot duration.
     */
    virtual void seek(uint32_t targetMs);

    // --- Sub-Plot Management ---
    bool addPlot(PlotBase* plot);
    PlotBase* getPlot(uint8_t index) const;

    /**
     * @brief Checks if the plot is currently running (actively progressing).
     * @return true if running, false otherwise.
     */
    bool isRunning() const { return _running; }

    /**
     * @brief Checks if the plot is currently paused.
     * @return true if paused, false otherwise.
     */
    bool isPaused() const { return _paused; }

    /**
     * @brief Gets the total duration of the plot.
     * @return Plot duration in milliseconds.
     */
    uint32_t getDuration() const { return _durationMs; }

    /**
     * @brief Sets the total duration of the plot.
     * @param durationMs The new duration in milliseconds.
     */
    virtual void setDuration(uint32_t durationMs) { _durationMs = durationMs; }

    /**
     * @brief Gets the remaining time in the plot based on the current elapsed time.
     * @return Remaining time in milliseconds. Returns 0 if the plot is not running or has finished.
     */
    uint32_t getRemainingTime() const;

    /**
     * @brief Gets the current operational status of the plot.
     * @return PlotStatus enum value (IDLE, RUNNING, PAUSED, STOPPED, FINISHED).
     */
    PlotStatus getCurrentStatus() const;

    /**
     * @brief Gets information about the control point defining the current state/value.
     * Derived classes implement this to return details about the active point/segment.
     *
     * @param[out] outId ID of the active point (or relevant signal). Set appropriately by derived class.
     * @param[out] outTimeMs Start time (timeMs or absolute time) of the active point/segment.
     * @param[out] outValue Current calculated/active value or state.
     * @param[out] outUser Custom user value associated with the active point (if applicable).
     * @return true if currently running and a point/segment is active, false otherwise.
     */
    virtual bool getCurrentControlPointInfo(uint8_t& outId, uint32_t& outTimeMs, int16_t& outValue, int16_t& outUser) const = 0;

    /**
     * @brief Sets the user data pointer associated with this plot.
     * The PlotBase class does not manage the lifetime of this data.
     * @param data Pointer to user data.
     */
    inline void setUserData(void* data) {
        _userData = data;
    }

    /**
     * @brief Gets the user data pointer, casting it to the specified type.
     * It's the user's responsibility to ensure the requested type T matches
     * the type originally stored.
     * @tparam T The type to cast the user data pointer to.
     * @return Pointer to the user data as type T*, or nullptr if no user data was set.
     */
    template<typename T>
    inline T* getUserData() const {
        return static_cast<T*>(_userData);
    }

    /**
     * @brief Loads type-specific configuration from the JSON object.
     * To be implemented by derived classes (e.g., parse 'controlPoints').
     *
     * @param config The JsonObject containing the configuration.
     * @return true if specific loading was successful, false otherwise.
     */
    virtual bool load(const JsonObject& config) = 0;
    
    /**
     * @brief Calculates the elapsed time since start(), handling rollover.
     * @return Elapsed time in milliseconds, clamped to duration. Returns 0 if not running.
     */
    uint32_t getElapsedMs() const;
    
    PlotBase* getParent() const { return _parent; }
    void setParent(PlotBase* parent) { _parent = parent; }

    String getDescription() const { return _description; }
    void setDescription(const String& description) { _description = description; }
    virtual short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user = nullptr, Component *src = nullptr){
        return this->owner->onMessage(id, verb, flags, user, src);
    }
protected:
    virtual void onStart();
    virtual void onStop();
    virtual void onPause();
    virtual void onResume();
    
    uint32_t _durationMs;
    uint32_t _startTimeMs;
    uint32_t _elapsedMsAtPause; // Stores elapsed time when pause() is called
    bool _running; // True if started and not stopped
    bool _paused;  // True if pause() called while running
    bool _explicitlyStopped; // True if stop() was called and not superseded by start()
    void* _userData; // Pointer for arbitrary user data

    PlotBase* _plots[MAX_PLOTS];
    PlotBase* _parent;
    String _description;
};

#endif // PLOT_BASE_H 
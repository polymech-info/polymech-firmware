#ifndef PLOT_BASE_H
#define PLOT_BASE_H

#include <stdint.h>
#include <ArduinoJson.h>
#include <Arduino.h> // For millis(), min()
#include <limits.h>  // For ULONG_MAX
#include "modbus/NetworkComponent.h"
#include "IPlotEvents.h" // Include the new delegate interface

#define PROFILE_SCALE 10000
#define PROFILE_TIME_SCALE 1000
#define MAX_PLOTS 4
#define MAX_CONTROL_POINTS 10 // Moved from derived classes

#include "config.h"
#include "net/commons.h"

#define PLOT_BASE_BLOCK_COUNT (static_cast<uint16_t>(PlotBaseRegisterOffset::_COUNT) + 3)

//------------------------------------------------------------------------------
// Status Enum
//------------------------------------------------------------------------------
enum class PlotStatus : uint8_t
{
    IDLE,         // Not started or stopped before completion
    INITIALIZING, // Initializing
    RUNNING,      // Actively running
    PAUSED,       // Started, but currently paused
    STOPPED,      // Explicitly stopped by user/logic
    FINISHED,     // Reached or exceeded duration
    WAITING,      // Waiting for active delays to finish
};

enum class PlotType : uint8_t
{
    Base,
    Temperature,
    Pressure,
    Signal
};

enum class PlotCommand : uint8_t
{
    NONE = 0, // Or IDLE, or some other default
    START = 1,
    STOP = 2,
    PAUSE = 3,
    RESUME = 4
};

enum class PlotBaseRegisterOffset : uint8_t
{
    // E_NVC_ENABLED (0) is handled by NetworkComponent
    STATUS = E_NVC_USER, // From NetworkComponent, typically 1
    CURRENT_VALUE,       // The result of getValue()
    DURATION,
    ELAPSED,
    REMAINING,
    COMMAND,
    TIME_OVERRIDE,
    _COUNT
};

// Generic Control Point structure
struct ControlPoint
{
    uint16_t x; // Time proportion (scaled 0-PROFILE_SCALE)
    uint16_t y; // Value (scaled 0-PROFILE_SCALE)
};

//------------------------------------------------------------------------------
// Base Class: PlotBase
//------------------------------------------------------------------------------

/**
 * @brief Base class for representing time-based signal plots.
 * Inherits from NetworkComponent and handles common timeline aspects like duration,
 * running state, and loading the duration from JSON.
 */
class PlotBase : public NetworkComponent<PLOT_BASE_BLOCK_COUNT>
{ // +1 for m_enabled, +1 for m_status + others
public:
    PlotBase(Component *owner, ushort componentId, ushort modbusAddress, PlotType plotType = PlotType::Base) : NetworkComponent<PLOT_BASE_BLOCK_COUNT>(modbusAddress, "PlotBase", componentId, Component::COMPONENT_DEFAULT, owner),
                                                                                                               _durationMs(0), _startTimeMs(0), _elapsedMsAtPause(0), _running(false), _paused(false), _explicitlyStopped(false), _userData(nullptr),
                                                                                                               _parent(nullptr), _eventsDelegate(nullptr), _plotType(plotType), _numControlPoints(0), max(0),
                                                                                                               m_status(this, componentId, "Status(0:IDLE, 1:INITIALIZING, 2:RUNNING, 3:PAUSED, 4:STOPPED, 5:FINISHED, 6:WAITING)", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
                                                                                                               m_currentValue(this, componentId, "CurrentValue", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
                                                                                                               m_duration(this, componentId, "Duration", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
                                                                                                               m_elapsed(this, componentId, "Elapsed", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
                                                                                                               m_remaining(this, componentId, "Remaining", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
                                                                                                               m_command(this, componentId, "Command", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
                                                                                                               m_timeOverride(this, componentId, "TimeOverride", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL))
    {
        this->type = COMPONENT_TYPE_PLOT;
        for (int i = 0; i < MAX_PLOTS; ++i)
        {
            _plots[i] = nullptr;
            children[i] = 0;
        }
        _initializeControlPoints();

        addNetworkValue(&m_status);
        addNetworkValue(&m_currentValue);
        addNetworkValue(&m_duration);
        addNetworkValue(&m_elapsed);
        addNetworkValue(&m_remaining);
        addNetworkValue(&m_command);
        addNetworkValue(&m_timeOverride);
    }

    virtual ~PlotBase() = default;
    // --- Plot Control ---

    /**
     * @brief Starts the plot execution timer.
     */
    virtual short start() override;

    /**
     * @brief Starts the plot execution timer.
     */
    virtual short startPlots();

    /**
     * @brief Called when the plot is ready to start.
     */
    virtual short onReady();

    /**
     * @brief Stops the plot execution timer and resets pause state.
     */
    virtual short stop() override;

    /**
     * @brief Pauses the plot execution timer if running.
     * Stores the elapsed time at the moment of pausing.
     */
    virtual short pause() override;

    /**
     * @brief Resumes the plot execution timer if paused.
     * Calculates a new start time based on the time elapsed before pausing.
     */
    virtual short resume() override;

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

    /**
     * @brief Slips the start time by the specified delta to compensate for lag.
     * Recursively slips all child plots.
     * @param deltaMs The amount of time to slip in milliseconds.
     */
    virtual void slipTime(uint32_t deltaMs);

    void setEventsDelegate(IPlotEvents *delegate) { _eventsDelegate = delegate; }

    // --- Sub-Plot Management ---
    bool addPlot(PlotBase *plot);
    PlotBase *getPlot(uint8_t index) const;
    void clear();

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
     * @brief Checks if the plot is currently waiting (lagging).
     * @return true if waiting, false otherwise.
     */
    bool isWaiting() const { return _waiting; }

    /**
     * @brief Sets the waiting status of the plot.
     * @param waiting True if waiting, false otherwise.
     */
    void setWaiting(bool waiting) { _waiting = waiting; }
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
    virtual bool getCurrentControlPointInfo(uint8_t &outId, uint32_t &outTimeMs, int16_t &outValue, int16_t &outUser) const;

    /**
     * @brief Sets the user data pointer associated with this plot.
     * The PlotBase class does not manage the lifetime of this data.
     * @param data Pointer to user data.
     */
    inline void setUserData(void *data)
    {
        _userData = data;
    }

    /**
     * @brief Gets the user data pointer, casting it to the specified type.
     * It's the user's responsibility to ensure the requested type T matches
     * the type originally stored.
     * @tparam T The type to cast the user data pointer to.
     * @return Pointer to the user data as type T*, or nullptr if no user data was set.
     */
    template <typename T>
    inline T *getUserData() const
    {
        return static_cast<T *>(_userData);
    }

    /**
     * @brief Loads type-specific configuration from the JSON object.
     * To be implemented by derived classes (e.g., parse 'controlPoints').
     *
     * @param config The JsonObject containing the configuration.
     * @return true if specific loading was successful, false otherwise.
     */
    virtual bool load(const JsonObject &config);

    /**
     * @brief Calculates the elapsed time since start(), handling rollover.
     * @return Elapsed time in milliseconds, clamped to duration. Returns 0 if not running.
     */
    uint32_t getElapsedMs() const;
    void setElapsedTime(uint32_t ms);

    PlotBase *getParent() const { return _parent; }
    void setParent(PlotBase *parent) { _parent = parent; }

    String getDescription() const { return _description; }
    void setDescription(const String &description) { _description = description; }

    PlotType getPlotType() const { return _plotType; }
    short setStatus(PlotStatus status) { return m_status.update(status); }

    ushort getSlot() const { return slot; }
    void setSlot(ushort s) { slot = s; }
    uint32_t _elapsedMsAtPause; // Stores elapsed time when pause() is called
    ushort slot;
    ushort max;
    virtual int16_t getValue(uint32_t elapsedMs) const;
    const ControlPoint *getControlPoints() const;
    uint8_t getNumControlPoints() const;
    virtual bool setControlPoints(const ControlPoint points[], uint8_t numPoints, uint32_t durationMs);
    /**
     * @brief Sets up child plot links using the children array. Derived classes must call this.
     */
    virtual short setup() override; // This is now handled by NetworkComponent
    virtual short loop() override;
    virtual void disable() override;
    virtual void enable(bool enabled) override;

    // --- NetworkComponent virtual function implementations ---
    short mb_tcp_read(MB_Registers *reg) override;
    short mb_tcp_write(MB_Registers *reg, short value) override;
    void mb_tcp_register(ModbusTCP *manager) override;
    void setModbusTCP(ModbusTCP *manager) { modbusTCP = manager; }

    // --- Debugging
    void printBlocks();

    const ushort *getChildren() const { return children; }

protected:
    NetworkValue<PlotStatus> m_status;
    NetworkValue<int16_t> m_currentValue;
    NetworkValue<uint16_t> m_duration;
    NetworkValue<uint16_t> m_elapsed;
    NetworkValue<uint16_t> m_remaining;
    NetworkValue<uint16_t> m_command;
    NetworkValue<int16_t> m_timeOverride;

    virtual const char *getModbusNamePrefix() const { return getOwnPrefix(); }
    virtual const char *getModbusGroupName() const { return this->name.c_str(); }

    virtual short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user = nullptr, Component *src = nullptr) override
    {
        return this->owner->onMessage(id, verb, flags, user, src);
    }

protected:
    virtual const char *getOwnPrefix() const { return "PlotBase"; }

    virtual void onStart();
    virtual void onStop();
    virtual void onPause();
    virtual void onResume();
    virtual void onFinished();

    uint32_t _durationMs;
    uint32_t _startTimeMs;
    bool _running;           // True if started and not stopped
    bool _paused;            // True if pause() called while running
    bool _waiting = false;   // True if waiting for lag compensation
    bool _explicitlyStopped; // True if stop() was called and not superseded by start()
    void *_userData;         // Pointer for arbitrary user data

    PlotBase *_plots[MAX_PLOTS];
    ushort children[MAX_PLOTS];
    bool resolvePlots();
    PlotBase *_parent;
    String _description;
    IPlotEvents *_eventsDelegate;
    PlotType _plotType;
    ControlPoint _controlPoints[MAX_CONTROL_POINTS];
    uint8_t _numControlPoints;

    // Helper methods for interpolation
    int16_t lerp(uint16_t y0, uint16_t y1, uint16_t t) const;
    int16_t cubicBezier(uint16_t y0, uint16_t y1, uint16_t y2, uint16_t y3, uint16_t t_norm) const;
    int16_t cubicBezierInt(uint16_t y0, uint16_t y1, uint16_t y2, uint16_t y3, uint16_t t_norm) const;
    void _initializeControlPoints();
};

#endif // PLOT_BASE_H
#ifndef SIGNAL_PLOT_H
#define SIGNAL_PLOT_H

#include "PlotBase.h"
#include <modbus/ModbusTCP.h>
#include <modbus/ModbusTypes.h>
#include <NetworkValue.h>

#define MAX_SIGNAL_POINTS 20

using ElapsedValue = NetworkValue<uint16_t>;

// --- Modbus Register Definitions ---
const uint16_t SIGNAL_PLOT_REGISTER_COUNT = PLOT_BASE_BLOCK_COUNT;

enum class SignalPlotCommand : uint8_t
{
    NONE = 0,
    START = 1,
    STOP = 2,
    PAUSE = 3,
    RESUME = 4
};

enum class E_SIGNAL_TYPE : uint8_t
{
    NONE = 0,
    MB_WRITE_COIL = 1,
    MB_WRITE_HOLDING_REGISTER = 2,
    CALL_METHOD = 3,       
    CALL_FUNCTION = 4,     
    CALL_REST = 5,         
    GPIO_WRITE = 6,        
    DISPLAY_MESSAGE = 7,
    USER_DEFINED = 8,
    PAUSE = 9,
    STOP_PIDS = 10,
    START_PIDS = 11,
    BUZZER_OFF = 12,
    BUZZER_SOLID = 13,
    BUZZER_SLOW_BLINK = 14,
    BUZZER_FAST_BLINK = 15,
    BUZZER_LONG_BEEP_SHORT_PAUSE = 16,
    IFTTT_WEBHOOK = 17
};

// New enum for GPIO Write Modes
enum class E_GpioWriteMode : uint8_t {
    DIGITAL = 0,
    ANALOG_PWM = 1
    // Add other modes like TONE, SERVO if ever needed
};

enum class E_SIGNAL_STATE : uint8_t
{
    STATE_NONE = 0,      // not hit yet
    STATE_ERROR = 1,     // error
    STATE_ON = 2,        // on - has been hit
    STATE_OFF = 3,       // off - disabled by user
    STATE_CUSTOM_1 = 100 // custom
};

struct S_SignalControlPoint
{
    uint8_t id;           
    uint32_t time;        
    E_SIGNAL_STATE state; 
    E_SIGNAL_TYPE type;   
    String name;          
    String description;   

    /**
     * @brief Argument 0, meaning depends on E_SIGNAL_TYPE.
     * - MB_WRITE_COIL: Modbus coil address (uint16_t).
     * - MB_WRITE_HOLDING_REGISTER: Modbus register address (uint16_t).
     * - GPIO_WRITE: GPIO pin number (uint8_t).
     * - CALL_METHOD: Component ID (ushort) // TODO: not implemented yet
     * - CALL_FUNCTION: Function ID (ushort) // TODO: not implemented yet
     * - CALL_REST: Not used directly, path/params likely in name/description or separate storage. // TODO: not implemented yet
     * - USER_DEFINED: User-specific. // TODO: not implemented yet
     * - DISPLAY_MESSAGE: Not currently used.
     * - PAUSE: Not used.
     */
    uint16_t arg_0; 

    /**
     * @brief Argument 1, meaning depends on E_SIGNAL_TYPE.
     * - MB_WRITE_COIL: Coil value (0 for OFF, 1 for ON).
     * - MB_WRITE_HOLDING_REGISTER: Value to write to register (int16_t).
     * - GPIO_WRITE: Write mode (see E_GpioWriteMode: DIGITAL = 0, ANALOG_PWM = 1).
     * - CALL_METHOD: Method index/ID. // TODO: not implemented yet
     * - CALL_FUNCTION: Not used typically, or first param. // TODO: not implemented yet
     * - USER_DEFINED: User-specific. // TODO: not implemented yet
     * - DISPLAY_MESSAGE: Not currently used.
     * - PAUSE: Not used.
     */
    uint16_t arg_1; 
    
    /**
     * @brief Argument 2, meaning depends on E_SIGNAL_TYPE.
     * - GPIO_WRITE: Value to write (e.g., 0/1 for digital; 0-255 for analog/PWM).
     * - CALL_METHOD: First method parameter (if any). // TODO: not implemented yet
     * - CALL_FUNCTION: Second param / etc. // TODO: not implemented yet
     * - USER_DEFINED: User-specific. // TODO: not implemented yet
     * - DISPLAY_MESSAGE: Not currently used.
     * - PAUSE: Not used.
     * - Others: Not used (typically).
     */
    uint16_t arg_2; 

    /**
     * @brief User pointer. For CALL_METHOD type, stores a command string (e.g. "<<1;2;64;reset:0:0>>").
     * For other types, usage is type-specific.
     */
    void *user; 

    S_SignalControlPoint() : id(0),
                             time(0),
                             state(E_SIGNAL_STATE::STATE_NONE),
                             type(E_SIGNAL_TYPE::NONE), 
                             user(nullptr),
                             arg_0(0),
                             arg_1(0),
                             arg_2(0),
                             name(""),
                             description("") {}
};

/**
 * @brief Represents a single signal with discrete state changes plotted over time.
 * Inherits from PlotBase.
 * Assumes control points in the source JSON are sorted chronologically by 'time'.
 */
class SignalPlot : public PlotBase
{
public:
    SignalPlot(Component *owner, ushort slot, ushort componentId, ushort modbusAddress);
    virtual ~SignalPlot() = default;

    // --- Component Overrides (Optional) ---
    short setup() override;
    short loop() override;
    short start() override;

    // --- Profile Specific Methods ---

    /**
     * @brief Gets the active state for the signal at the current elapsed time.
     *
     * Finds the latest control point that occurred at or before
     * the current time and returns its state.
     *
     * @param defaultState The state to return if the plot isn't running or no point has occurred yet.
     * @return The determined state (SignalState).
     */
    E_SIGNAL_STATE getState(E_SIGNAL_STATE defaultState = E_SIGNAL_STATE::STATE_OFF) const;

    /**
     * @brief Gets the user-defined integer associated with the active state at the current time.
     *
     * Finds the latest control point that occurred at or before
     * the current time and returns its associated user value.
     *
     * @param defaultValue The value to return if the plot isn't running or no point has occurred yet.
     * @return The determined user value (int16_t).
     */
    int16_t getUserValue(int16_t defaultValue = 0) const;

    // --- PlotBase Overrides ---
    bool getCurrentControlPointInfo(uint8_t &outId, uint32_t &outTimeMs, int16_t &outValue, int16_t &outUser) const override;

    /**
     * @brief Loads the controlPoints array (discrete state changes) from the JSON config.
     * Called by PlotBase::from.
     * Assumes points are sorted chronologically by 'time' in the JSON.
     * Expected JSON format within the config object:
     * "controlPoints": [
     *   { "id": <uint8>, "time": <uint32>, "state": <int16>, "user": <int16> },
     *   ...
     * ]
     */
    bool load(const JsonObject& config) override;

    // --- Getters for control points ---
    const S_SignalControlPoint* getControlPoints() const;
    uint8_t getNumControlPoints() const;

protected:
    
    // --- SignalPlot Slot ---
    
    S_SignalControlPoint _controlPoints[MAX_SIGNAL_POINTS];
    uint8_t _numControlPoints;

    // Helper to find the applicable control point
    // Returns nullptr if no point is applicable yet
    const S_SignalControlPoint *findActivePoint(uint32_t elapsedMs) const;
    // short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void* user = nullptr, Component *src = nullptr) override;
private:
    void executeControlPointAction(uint8_t cpIndex);
    uint32_t _lastLogMs = 0;
};

#endif // SIGNAL_PLOT_H
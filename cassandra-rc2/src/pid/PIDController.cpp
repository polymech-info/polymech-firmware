#ifdef ENABLE_PID
#include "pid/PIDController.h"
#include <ArduinoLog.h>
#include <modbus/ModbusTCP.h>

PIDController::PIDController(uint8_t id, const char *name, int8_t thermoDO, int8_t thermoCS, int8_t thermoCLK, int8_t outputPin)
    : Component(name, id),
      thermocouple(thermoCLK, thermoCS, thermoDO), // CLK, CS, DO
      _thermoDO(thermoDO), _thermoCS(thermoCS), _thermoCLK(thermoCLK),
      _setpoint(25.0), _input(0.0), _output(0.0),                 // Default setpoint
      _kp(2.0), _ki(5.0), _kd(1.0),                               // Default PID gains (example values)
      _pid(&_input, &_output, &_setpoint, _kp, _ki, _kd, DIRECT), // DIRECT or REVERSE depending on heating/cooling
      _aTune(&_input, &_output),
      _autotuning(false),
      _autotuneStatus(AUTOTUNE_OFF),
      _aTuneStartValue(0.0),
      _aTuneNoiseBand(0.5),  // Noise band for autotune (adjust based on system)
      _aTuneLookbackSec(20), // Autotune lookback seconds (adjust)
      _lastKp(0.0), _lastKi(0.0), _lastKd(0.0),
      _outputPin(outputPin),
      _windowStartTime(0),
      _pidModeAuto(false), // Start in MANUAL mode
      _sensorError(false)
{
}

PIDController::~PIDController()
{
    // Nothing specific to delete here
}

short PIDController::setup()
{
    L_INFO("Setting up PIDController '%s'...", name);
    pinMode(_outputPin, OUTPUT);
    digitalWrite(_outputPin, LOW); // Ensure output is off initially

    // Initialize PID
    _pid.SetOutputLimits(0, 255);    // PID output range (for PWM-like control)
    _pid.SetSampleTime(_windowSize); // Set PID sample time = window size
    setPIDMode(_pidModeAuto);        // Apply initial mode

    // Initialize Autotune
    _aTune.SetNoiseBand(_aTuneNoiseBand);
    _aTune.SetOutputStep(100); // Example output step for autotune
    _aTune.SetLookbackSec(_aTuneLookbackSec);
    _aTune.SetControlType(1); // PID type

    // Wait for MAX6675 to stabilize (optional, may remove if blocking is bad)
    delay(500);
    updateTemperature(); // Initial temperature read
    if (_sensorError)
    {
        L_ERROR("PID %s: Initial thermocouple read failed!", name);
    }
    else
    {
        L_INFO("PID %s: Initial temp = %.2f C", name, _input);
    }

    _windowStartTime = millis(); // Initialize window timing
    L_INFO("PIDController '%s' setup complete.", name);
    return E_OK;
}

short PIDController::loop()
{
    unsigned long now = millis();

    updateTemperature(); // Read temperature input

    if (_autotuning)
    {
        runAutotune();
    }
    else if (_pidModeAuto)
    {
        if (!_sensorError)
        {
            _pid.Compute(); // Only compute if temperature reading is valid
        }
        else
        {
            _output = 0; // Turn off output if sensor fails
        }
    }
    // In MANUAL mode, _output is set via Modbus and not changed here

    applyOutput(); // Apply the calculated or manually set output
    return E_OK;
}

void PIDController::updateTemperature()
{
    double temp = thermocouple.readCelsius();
    if (isnan(temp))
    {
        if (!_sensorError)
        { // Log only on transition to error state
            L_ERROR("PID %s: Failed to read temperature from MAX6675!", name);
            _sensorError = true;
            _input = -999.0; // Indicate error state clearly
            // Optionally handle sensor error (e.g., turn off PID, set alarm)
            setPIDMode(false); // Force manual mode on sensor error
        }
    }
    else
    {
        if (_sensorError)
        { // Log recovery
            Log.noticeln("PID %s: Thermocouple reading recovered.", name);
        }
        _sensorError = false;
        _input = temp;
    }
}

void PIDController::runAutotune()
{
    unsigned long now = millis();
    if (!_autotuning)
        return; // Should not happen, but safety check

    byte val = _aTune.Runtime();
    if (val != 0)
    { // Autotune finished
        bool success = (val == 1);
        finishAutotune(success);
    }
    else
    {
        // Autotune still running, apply its output directly
        applyOutput(); // Use the _output value calculated by aTune.Runtime()
    }
}

void PIDController::applyOutput()
{
    unsigned long now = millis();
    // Time-proportional output control (PWM-like over _windowSize)
    if (now - _windowStartTime > _windowSize)
    {
        _windowStartTime += _windowSize;
    }

    double currentOutput = _autotuning ? _output : (_pidModeAuto ? _output : _output); // Use ATune output if running, else PID/Manual output

    // Map 0-255 output to on/off time within the window
    if (currentOutput > (now - _windowStartTime) * 255.0 / _windowSize)
    {
        digitalWrite(_outputPin, HIGH);
    }
    else
    {
        digitalWrite(_outputPin, LOW);
    }
}

void PIDController::setPIDMode(bool autoMode)
{
    if (_pidModeAuto == autoMode && !_sensorError)
        return; // No change needed unless recovering from error

    // If switching to auto mode, ensure sensor is working
    if (autoMode && _sensorError)
    {
        L_ERROR("PID %s: Cannot switch to AUTO mode, sensor error active.", name);
        _pidModeAuto = false; // Stay in MANUAL
        _pid.SetMode(MANUAL);
        return;
    }

    _pidModeAuto = autoMode;
    _pid.SetMode(_pidModeAuto ? AUTOMATIC : MANUAL);
    L_INFO("PID %s: Mode set to %s", name, _pidModeAuto ? "AUTO" : "MANUAL");

    // If switching to MANUAL, potentially reset output or keep last auto value?
    // Current behavior: manual output must be set via Modbus.
}

void PIDController::startAutotune()
{
    if (_autotuning)
    {
        Log.warningln("PID %s: Autotune already running.", name);
        return;
    }
    if (_sensorError)
    {
        L_ERROR("PID %s: Cannot start Autotune, sensor error active.", name);
        _autotuneStatus = AUTOTUNE_FAILED;
        return;
    }

    Log.noticeln("PID %s: Starting Autotune...", name);
    _aTuneStartValue = _output; // Remember the output value before starting
    _aTune.Cancel();            // Reset autotune logic just in case
    _autotuning = true;
    _autotuneStatus = AUTOTUNE_RUNNING;
    // ATune output range might need adjustment depending on expected tuning behavior
    _aTune.SetOutputStep(abs(_setpoint - _input) > 10 ? 80 : 40); // Example dynamic step
}

void PIDController::cancelAutotune()
{
    if (!_autotuning)
        return;
    Log.noticeln("PID %s: Cancelling Autotune.", name);
    _aTune.Cancel();
    _autotuning = false;
    _autotuneStatus = AUTOTUNE_OFF;
    _output = _aTuneStartValue; // Restore output to value before autotune
    applyOutput();              // Apply the restored output immediately
    setPIDMode(_pidModeAuto);   // Re-apply original PID mode
}

void PIDController::finishAutotune(bool success)
{
    _autotuning = false;
    if (success)
    {
        _lastKp = _aTune.GetKp();
        _lastKi = _aTune.GetKi();
        _lastKd = _aTune.GetKd();
        Log.noticeln("PID %s: Autotune finished successfully!", name);
        Log.noticeln("  > Kp: %.2f, Ki: %.2f, Kd: %.2f", _lastKp, _lastKi, _lastKd);
        _autotuneStatus = AUTOTUNE_FINISHED_OK;

        // Optionally apply the new tunings immediately
        _kp = _lastKp;
        _ki = _lastKi;
        _kd = _lastKd;
        _pid.SetTunings(_kp, _ki, _kd);
    }
    else
    {
        L_ERROR("PID %s: Autotune failed!", name);
        _autotuneStatus = AUTOTUNE_FAILED;
        // Keep old PID values
    }

    _output = _aTuneStartValue; // Restore output
    applyOutput();
    setPIDMode(_pidModeAuto); // Re-apply original PID mode
}

// Scale Modbus value (e.g., 1234 means 12.34) to double
double PIDController::scalePIDParam(uint16_t modbusValue)
{
    return (double)modbusValue / 100.0;
}

// Scale double (e.g., 12.34) to Modbus value (1234)
uint16_t PIDController::unscalePIDParam(double pidValue)
{
    return (uint16_t)(pidValue * 100.0 + 0.5); // Add 0.5 for rounding
}

// --- Modbus Read/Write ---

short PIDController::mb_tcp_read(short address)
{
    // Calculate offset from base address
    short offset = address - MB_HREG_PID_2_BASE_ADDRESS;

    switch (offset)
    {
    case 0: // PV (Process Value * 100)
        return _sensorError ? 0xFFFF : (uint16_t)(_input * 100.0 + 0.5);
    case 1: // SP (Setpoint * 100)
        return (uint16_t)(_setpoint * 100.0 + 0.5);
    case 2: // Output (0-255)
        return (uint16_t)(_output + 0.5);
    case 3: // Kp (* 100)
        return unscalePIDParam(_kp);
    case 4: // Ki (* 100)
        return unscalePIDParam(_ki);
    case 5: // Kd (* 100)
        return unscalePIDParam(_kd);
    case 6: // Mode (0:Manual, 1:Auto)
        return _pidModeAuto ? 1 : 0;
    case 7: // Autotune Status
        return (uint16_t)_autotuneStatus;
    case 8: // Autotune Control (Read is always 0)
        return 0;
    case 9: // Autotune Kp Result (* 100)
        return unscalePIDParam(_lastKp);
    case 10: // Autotune Ki Result (* 100)
        return unscalePIDParam(_lastKi);
    case 11: // Autotune Kd Result (* 100)
        return unscalePIDParam(_lastKd);
    default:
        Log.warningln("PID %s: Read from unhandled address offset %d (Abs: %d)", name, offset, address);
        return 0xFFFF; // Indicate invalid address
    }
}

short PIDController::mb_tcp_write(short address, short value)
{
    // Calculate offset from base address
    short offset = address - MB_HREG_PID_2_BASE_ADDRESS;

    switch (offset)
    {
    case 1: // SP (Setpoint * 100)
        _setpoint = (double)value / 100.0;
        L_VERBOSE("PID %s: Setpoint updated to %.2f", name, _setpoint);
        return E_OK;
    case 2: // Output (0-255) - Only allowed in MANUAL mode
        if (!_pidModeAuto && !_autotuning)
        {
            // Clamp value to 0-255
            _output = constrain(value, 0, 255);
            L_VERBOSE("PID %s: Manual output set to %.1f", name, _output);
            applyOutput(); // Apply manual change immediately if possible
            return E_OK;
        }
        else
        {
            Log.warningln("PID %s: Cannot set output manually while in AUTO mode or during Autotune.", name);
            return E_INVALID_PARAMETER; // Use defined error code
        }
    case 3: // Kp (* 100)
        _kp = scalePIDParam(value);
        _pid.SetTunings(_kp, _ki, _kd);
        L_VERBOSE("PID %s: Kp updated to %.2f", name, _kp);
        return E_OK;
    case 4: // Ki (* 100)
        _ki = scalePIDParam(value);
        _pid.SetTunings(_kp, _ki, _kd);
        L_VERBOSE("PID %s: Ki updated to %.2f", name, _ki);
        return E_OK;
    case 5: // Kd (* 100)
        _kd = scalePIDParam(value);
        _pid.SetTunings(_kp, _ki, _kd);
        L_VERBOSE("PID %s: Kd updated to %.2f", name, _kd);
        return E_OK;
    case 6: // Mode (0:Manual, 1:Auto)
        setPIDMode(value == 1);
        return E_OK;
    case 8: // Autotune Control (Write 1 to start, 0 to stop)
        if (value == 1)
        {
            startAutotune();
        }
        else
        {
            cancelAutotune();
        }
        return E_OK;

    // Read-only registers or invalid address
    case 0:  // PV
    case 7:  // Autotune Status
    case 9:  // Autotune Kp Result
    case 10: // Autotune Ki Result
    case 11: // Autotune Kd Result
        Log.warningln("PID %s: Attempt to write read-only address offset %d (Abs: %d)", name, offset, address);
        return E_INVALID_PARAMETER; // Use defined error code
    default:
        Log.warningln("PID %s: Write to unhandled address offset %d (Abs: %d)", name, offset, address);
        return E_INVALID_PARAMETER; // Use defined error code
    }
}

void PIDController::mb_tcp_register(ModbusTCP *manager)
{
    if (!manager)
    {
        L_ERROR("PID %s: Cannot register Modbus blocks - ModbusTCP is null.", name);
        return;
    }
    // Define the Modbus block information for this PID controller
    MB_Registers info(MB_HREG_PID_2_BASE_ADDRESS,
                      PID_2_REGISTER_COUNT,
                      E_FN_CODE::FN_READ_COIL,
                      MB_ACCESS_READ_WRITE); // Assuming most registers are R/W, actual access enforced in read/write methods

    // Attempt to register the block
    if (!manager->registerModbus(const_cast<PIDController *>(this), info))
    {
        L_ERROR("PID %s: Failed to register Modbus block (Addr: %d, Count: %d)", name, info.startAddress, info.count);
    }
}
#endif // ENABLE_PID
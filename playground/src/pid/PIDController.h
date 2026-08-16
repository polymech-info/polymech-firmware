#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#if ENABLE_PID

#include <Component.h>
#include "config-modbus.h"
#include <enums.h>
#include <max6675.h>
#include <PID_v1.h> // From br3ttb/PID library
#include <PID_AutoTune_v0.h> // From br3ttb/PID Autotune Library

// Define Autotune Status Enum (aligned with Modbus register)
enum AutotuneStatus : uint8_t {
    AUTOTUNE_OFF = 0,
    AUTOTUNE_RUNNING = 1,
    AUTOTUNE_FINISHED_OK = 2,
    AUTOTUNE_FAILED = 3
};

class PIDController : public Component {
public:
    PIDController(uint8_t id, const char* name, int8_t thermoDO, int8_t thermoCS, int8_t thermoCLK, int8_t outputPin);
    ~PIDController();

    short setup() override;
    short loop() override;
    short mb_tcp_read(short address) override;
    short mb_tcp_write(short address, short value) override;
    void mb_tcp_register(ModbusTCP* manager) override;

private:
    // MAX6675 Thermocouple Sensor
    MAX6675 thermocouple;
    int8_t _thermoDO, _thermoCS, _thermoCLK;

    // PID Controller
    double _setpoint, _input, _output;
    double _kp, _ki, _kd;
    PID _pid;

    // PID Autotune
    PID_ATune _aTune;
    bool _autotuning;
    AutotuneStatus _autotuneStatus;
    double _aTuneStartValue; // Where the output was when AT started
    double _aTuneNoiseBand;
    unsigned int _aTuneLookbackSec;
    double _lastKp, _lastKi, _lastKd; // Store results from autotune

    // Output Control
    int8_t _outputPin;
    unsigned long _windowStartTime;
    const unsigned long _windowSize = 1000; // PID cycle time (e.g., 1 second)
    bool _pidModeAuto; // true = AUTO, false = MANUAL

    // Helper methods
    void updateTemperature();
    void runAutotune();
    void applyOutput();
    void setPIDMode(bool autoMode);
    void startAutotune();
    void cancelAutotune();
    void finishAutotune(bool success);
    double scalePIDParam(uint16_t modbusValue);
    uint16_t unscalePIDParam(double pidValue);

    // Error handling
    bool _sensorError;
};

#endif // PIDCONTROLLER_H
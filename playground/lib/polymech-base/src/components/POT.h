#ifndef POT_H
#define POT_H

#include <Arduino.h>          // millis(), pinMode(), analogRead(), map()
#include <ArduinoLog.h>
#include <App.h>
#include <Component.h>
#include <enums.h>
#include "config.h"
#include "config-modbus.h"
#include <modbus/Modbus.h>
#include <modbus/ModbusTCP.h>

/*
 * Hardware Smoothing Suggestion (RC Low-Pass Filter):
 *
 * @link : https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
 * To further reduce noise on the analog input signal before it reaches the ADC,
 * a simple RC low-pass filter can be implemented using the potentiometer itself
 * as the resistor (R) and adding a capacitor (C) between the potentiometer's
 * wiper output and ground.
 *
 * Potentiometer Resistance (R): 5 kΩ (5000 Ω)
 * Input Voltage: 5V
 *
 * Cutoff Frequency (f_c) = 1 / (2 * π * R * C)
 * Capacitor (C) = 1 / (2 * π * R * f_c)
 *
 * Example Capacitor Values for R = 5 kΩ:
 * - For f_c ≈ 30 Hz (Faster response): C ≈ 1 / (2 * π * 5000 * 30) ≈ 1.06 µF -> Use 1 µF
 * - For f_c ≈ 10 Hz (Moderate):     C ≈ 1 / (2 * π * 5000 * 10) ≈ 3.18 µF -> Use 2.2 µF or 3.3 µF
 * - For f_c ≈ 5 Hz (More smoothing): C ≈ 1 / (2 * π * 5000 * 5)  ≈ 6.37 µF -> Use 4.7 µF or 6.8 µF
 *
 * Recommendation: Start with C = 1 µF to 3.3 µF and adjust based on observed
 * noise reduction and response time requirements. Use a ceramic capacitor
 * rated for at least the input voltage (e.g., 10V or 16V).
 */

/* --------------------------------------------------------------------------
 *  POT  –– Analogue potentiometer reader with lightweight digital filtering
 * --------------------------------------------------------------------------
 *  ▸ Supports three damping modes: NONE, MOVING‑AVERAGE (box‑car) and EMA.
 *  ▸ Moving‑average implemented as an incremental ring buffer (O(1)).
 *  ▸ EMA uses a 1‑pole IIR:  y[n] = y[n‑1] + (x[n] − y[n‑1]) / 2^k.
 *  ▸ Dead‑band suppresses ±1‑LSB chatter after scaling to 0‑100.
 *  ▸ Designed for small AVR/ESP32 class MCUs – no malloc, no floats.
 * ----------------------------------------------------------------------- */

// -------------------------------------------------------------------------
//  Compile‑time knobs (override in config.h if desired)
// -------------------------------------------------------------------------
#ifndef POT_SAMPLE_INTERVAL
#define POT_SAMPLE_INTERVAL    10        // ms between ADC reads
#endif

#ifndef POT_DA_WIN_LEN
#define POT_DA_WIN_LEN         8         // box‑car window length (must be power‑of‑two)
#endif

// Compile-time check for power-of-two window length for moving average
static_assert((POT_DA_WIN_LEN > 0) && ((POT_DA_WIN_LEN & (POT_DA_WIN_LEN - 1)) == 0),
              "POT_DA_WIN_LEN must be a power of two for efficient moving average calculation.");

#ifndef POT_DA_EMA_SHIFT
#define POT_DA_EMA_SHIFT       3         // alpha = 1 / (2^shift)   (shift = 3 ⇒ α = 0.125)
#endif

#ifndef POT_DEADBAND
#define POT_DEADBAND           1         // change (0‑100 scale) required to notify
#endif

#ifndef POT_RAW_MAX_VALUE
#define POT_RAW_MAX_VALUE      1023      // 10‑bit ADC full‑scale
#endif

#ifndef POT_SCALED_MAX_VALUE
#define POT_SCALED_MAX_VALUE   100       // application‑level full‑scale
#endif

#ifndef POT_DAMPING_WINDOW_SIZE
#define POT_DAMPING_WINDOW_SIZE POT_DA_WIN_LEN // alias for legacy code
#endif

// -------------------------------------------------------------------------
//  Damping algorithms
// -------------------------------------------------------------------------
enum class POTDampingAlgorithm : uint8_t
{
    DAMPING_NONE          = 0,
    DAMPING_MOVING_AVERAGE,
    DAMPING_EMA
};

// -------------------------------------------------------------------------
//  Control modes
// -------------------------------------------------------------------------
enum class E_POTControlMode : uint8_t
{
    E_AUX_LOCAL  = 0,
    E_AUX_REMOTE = 1
};

// Address offsets for Modbus registers
enum E_POT_REGISTER_OFFSET : uint16_t
{
    OFFSET_VALUE = 0,
    OFFSET_MODE = 1,
    OFFSET_REMOTE_VALUE = 2
};

class Bridge;
class POT : public Component
{
public:
    POT(Component *owner,
        uint16_t _pin,
        uint16_t _id,
        uint16_t _modbusAddress,
        POTDampingAlgorithm _algo = POTDampingAlgorithm::DAMPING_NONE)
        : Component("POT", _id, Component::COMPONENT_DEFAULT, owner),
          pin(_pin),
          modbusAddress(_modbusAddress),
          algo(_algo)
    {
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

        // Initialize instance-specific Modbus blocks.
        // INIT_MODBUS_BLOCK is assumed to use `this->modbusAddress` and potentially `this->id`
        // (e.g., for slaveId if that's how it's intended) from the current POT instance.
        m_modbus_blocks[0] = INIT_MODBUS_BLOCK_TCP(modbusAddress, OFFSET_VALUE, E_FN_CODE::FN_READ_HOLD_REGISTER,
                                               MB_ACCESS_READ_ONLY, "POT Value", "aux");
        m_modbus_blocks[1] = INIT_MODBUS_BLOCK_TCP(modbusAddress, OFFSET_MODE, E_FN_CODE::FN_READ_HOLD_REGISTER,
                                               MB_ACCESS_READ_WRITE, "POT Mode (0=Local,1=Remote)", "aux");
        m_modbus_blocks[2] = INIT_MODBUS_BLOCK_TCP(modbusAddress, OFFSET_REMOTE_VALUE, E_FN_CODE::FN_READ_HOLD_REGISTER,
                                               MB_ACCESS_READ_WRITE, "POT Remote Value", "aux");

        // Initialize the view to point to these instance-specific blocks
        m_modbus_view.data = m_modbus_blocks; // m_modbus_blocks is MB_Registers[3], so this is MB_Registers*
        m_modbus_view.count = sizeof(m_modbus_blocks) / sizeof(m_modbus_blocks[0]);
        
    }

    // --------------------------------------------------- lifecycle -------
    short setup() override
    {
        Component::setup();
        pinMode(pin, INPUT);
        lastSample = millis();
        return E_OK;
    }

    short loop() override
    {
        uint32_t now = millis();
        if (now - lastSample < POT_SAMPLE_INTERVAL) return E_OK;
        lastSample = now;
        uint16_t raw = analogRead(pin);
        // ---------------------- filtering -------------------------------
        switch (algo)
        {
            case POTDampingAlgorithm::DAMPING_MOVING_AVERAGE:
                acc -= ring[idx];
                acc += raw;
                ring[idx] = raw;
                idx = (idx + 1) & (POT_DA_WIN_LEN - 1); // power‑of‑two length
                filtRaw = acc >> log2_WIN_LEN;           // divide by window length
                break;

            case POTDampingAlgorithm::DAMPING_EMA:
                // y += (x - y) / 2^k  -- shift = k
                filtRaw += (raw - filtRaw) >> POT_DA_EMA_SHIFT;
                break;

            case POTDampingAlgorithm::DAMPING_NONE:
            default:
                filtRaw = raw;
                break;
        }

        // ------------------- scale & dead‑band ---------------------------
        uint16_t scaledLocalValue = map(filtRaw, 0, POT_RAW_MAX_VALUE, 0, POT_SCALED_MAX_VALUE);
        uint16_t newValue = value; // Start with current value

        if (controlMode == E_POTControlMode::E_AUX_LOCAL)
        {
            // LOCAL Mode: Use filtered/scaled value with deadband
            if (abs(int16_t(scaledLocalValue) - int16_t(value)) > POT_DEADBAND)
            {
                newValue = scaledLocalValue;
            }
        }
        else // E_AUX_REMOTE Mode
        {
            newValue = remoteValue; // Already clamped in mb_tcp_write
        }

        if (newValue != value)
        {
            value = newValue;
            notifyStateChange();
        }

        return E_OK;
    }

    // --------------------------------------------------- diagnostics ----
    short debug() override { return info(); }

    short info(short = 0, short = 0) override
    {
        Log.verboseln("POT::info - ID:%d Pin:%d Val:%d Algo:%d Mode:%d RemoteVal:%d Raw:%d Address:%d",
                      id, pin, value, static_cast<uint8_t>(algo),
                      static_cast<uint8_t>(controlMode), remoteValue, analogRead(pin), modbusAddress);
        
        return E_OK;
    }

    // --------------------------------------------------- Modbus ---------
    short mb_tcp_write(MB_Registers* reg, short networkValue) override
    {
        uint16_t addr = reg->startAddress;
        bool changed = false;

        if (addr == modbusAddress + OFFSET_MODE) // Write to Mode register
        {
            E_POTControlMode newMode = (networkValue == 0) ? E_POTControlMode::E_AUX_LOCAL : E_POTControlMode::E_AUX_REMOTE;
            if (newMode != controlMode)
            {
                Log.verboseln("POT::mb_write - ID:%d Mode change %d -> %d", id, static_cast<uint8_t>(controlMode), static_cast<uint8_t>(newMode));
                controlMode = newMode;
                changed = true;
            }
        }
        else if (addr == modbusAddress + OFFSET_REMOTE_VALUE) // Write to Remote Value register
        {
            // Clamp remote value to valid scaled range (0-100 or defined max)
            uint16_t clampedValue = constrain(networkValue, 0, POT_SCALED_MAX_VALUE);
            if (clampedValue != remoteValue)
            {
                 Log.verboseln("POT::mb_write - ID:%d Remote value change %d -> %d", id, remoteValue, clampedValue);
                remoteValue = clampedValue;
                if (controlMode == E_POTControlMode::E_AUX_REMOTE) {
                     changed = true; // Trigger update in loop if remote
                }
            }
        }
        else if (addr == modbusAddress + OFFSET_VALUE) // Writing to Value register (read-only)
        {
            // Writing to the main value register is not allowed directly
            return MODBUS_ERROR_ILLEGAL_FUNCTION;
        }
        else
        {
             return E_INVALID_PARAMETER; // Address doesn't match any known register
        }

        // If in remote mode and either mode or remote value changed, update main value immediately
        if (changed && controlMode == E_POTControlMode::E_AUX_REMOTE)
        {
            if (value != remoteValue)
            {
                value = remoteValue;
                notifyStateChange();
            }
        }
        // If mode changed to local, next loop will update value based on pot

        return E_OK;
    }

    short mb_tcp_read(MB_Registers* reg) override
    {
        uint16_t addr = reg->startAddress;
        if (addr == modbusAddress + OFFSET_VALUE) {
            return value;
        }
        else if (addr == modbusAddress + OFFSET_MODE) { // Read Mode
            return static_cast<uint16_t>(controlMode);
        }
        else if (addr == modbusAddress + OFFSET_REMOTE_VALUE) { // Read Remote Value
            return remoteValue;
        }
        return 0; // Default or error for unknown address
    }

    void mb_tcp_register(ModbusTCP *mgr) override
    {
        ModbusBlockView* view = mb_tcp_blocks();
        for (int i=0; i < view->count; ++i)
        {
             mgr->registerModbus(const_cast<POT*>(this), view->data[i]); // view->data[i] is MB_Registers
        }
    }

    // Returns a pointer to the instance-specific Modbus block definitions.
    // The virtual function in Component base class is likely `virtual ModbusBlockView* mb_tcp_blocks() const;`
    ModbusBlockView* mb_tcp_blocks() const override
    {
        // m_modbus_view is initialized in constructor and doesn't change.
        // Returning a non-const pointer from a const method is allowed if the member is mutable.
        return &m_modbus_view;
    }

    short serial_register(Bridge *b) override
    {
        b->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&POT::info);
        return E_OK;
    }

    // --------------------------------------------------- public helpers -
    uint16_t getValue() const { return value; }

private:
    // ----------------------------- constants ----------------------------
    static constexpr uint8_t log2_WIN_LEN =
        (POT_DA_WIN_LEN == 1) ? 0 :
        (POT_DA_WIN_LEN == 2) ? 1 :
        (POT_DA_WIN_LEN == 4) ? 2 :
        (POT_DA_WIN_LEN == 8) ? 3 :
        (POT_DA_WIN_LEN == 16)? 4 : 0; // extend if larger windows used

    // ----------------------------- members ------------------------------
    const uint16_t pin;
    const uint16_t modbusAddress;
    const POTDampingAlgorithm algo;

    // --- Control Mode ---
    E_POTControlMode controlMode = E_POTControlMode::E_AUX_LOCAL;
    uint16_t remoteValue = 0; // Value set via Modbus in E_AUX_REMOTE mode
    // ---------------------

    uint16_t ring[POT_DA_WIN_LEN] = {0};  // circular buffer
    uint32_t acc          = 0;            // running sum
    uint8_t  idx          = 0;            // buffer index
    uint16_t filtRaw      = 0;            // filtered raw sample
    uint16_t value        = 0;            // scaled, debounced value (0‑100)

    uint32_t lastSample   = 0;            // ms

    // Instance-specific storage for Modbus block definitions
    MB_Registers m_modbus_blocks[3];
    // m_modbus_view needs to be mutable to be returned as ModbusBlockView* from a const method.
    mutable ModbusBlockView m_modbus_view;
};

#endif // POT_H

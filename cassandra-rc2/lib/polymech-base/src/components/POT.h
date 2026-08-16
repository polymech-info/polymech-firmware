#ifndef POT_H
#define POT_H

#include <Arduino.h>          // millis(), pinMode(), analogRead(), map()
#include <ArduinoLog.h>
#include <App.h>
#include <enums.h>
#include "config.h"
#include "config-modbus.h"
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"

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
#define POT_DEADBAND           5         // change (0‑100 scale) required to notify
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

#define POT_MB_COUNT 4 // m_enabled (from NetworkComponent) + m_value, m_mode, m_remoteValue

class Bridge;
class POT : public NetworkComponent<POT_MB_COUNT>
{
public:
    enum E_MB_Offset {
        MB_OFS_HR_VALUE = E_NVC_USER + 0,
        MB_OFS_HR_MODE = E_NVC_USER + 1,
        MB_OFS_HR_REMOTE_VALUE = E_NVC_USER + 2,
    };
private:
    uint32_t lastSample = 0;

public:
    const uint16_t pin;
    const POTDampingAlgorithm algo;
    const bool reverse;
    NetworkValue<uint16_t> m_value;
    NetworkValue<uint16_t> m_mode; 
    NetworkValue<uint16_t> m_remoteValue;
    
    POT(Component *owner,
        uint16_t _pin,
        uint16_t _id,
        uint16_t _modbusAddress,
        POTDampingAlgorithm _algo = POTDampingAlgorithm::DAMPING_NONE,
        bool _reverse = false)
        : NetworkComponent(_modbusAddress, "POT(" + String(_modbusAddress) + ")", _id, Component::COMPONENT_DEFAULT, owner),
          pin(_pin),
          algo(POTDampingAlgorithm::DAMPING_EMA),
          reverse(_reverse),
          m_value(this, _id, "POT Value"),
          m_mode(this, _id, "POT Mode"),
          m_remoteValue(this, _id, "POT Remote Value")
    {
        name = "POT[" + String(_modbusAddress) + "]";
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    }

    // --------------------------------------------------- lifecycle -------
    short setup() override
    {
        NetworkComponent::setup();
        pinMode(pin, INPUT);
        lastSample = millis();
        
        const uint16_t baseAddr = mb_tcp_base_address();

        m_value.initNotify(0, 5, NetworkValue_ThresholdMode::DIFFERENCE);
        m_value.initModbus(baseAddr + MB_OFS_HR_VALUE, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, m_value.name.c_str(), this->name.c_str());
        registerBlock(m_value.getRegisterInfo());

        m_mode.initNotify(static_cast<uint16_t>(E_POTControlMode::E_AUX_LOCAL), 5, NetworkValue_ThresholdMode::DIFFERENCE);
        m_mode.initModbus(baseAddr + MB_OFS_HR_MODE, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, m_mode.name.c_str(), this->name.c_str());
        registerBlock(m_mode.getRegisterInfo());

        m_remoteValue.initNotify(0, 5, NetworkValue_ThresholdMode::DIFFERENCE);
        m_remoteValue.initModbus(baseAddr + MB_OFS_HR_REMOTE_VALUE, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, m_remoteValue.name.c_str(), this->name.c_str());
        registerBlock(m_remoteValue.getRegisterInfo());

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
        if (reverse) {
            scaledLocalValue = POT_SCALED_MAX_VALUE - scaledLocalValue;
        }
        uint16_t newValue = m_value.getValue(); // Start with current value

        E_POTControlMode currentMode = static_cast<E_POTControlMode>(m_mode.getValue());
        if (currentMode == E_POTControlMode::E_AUX_LOCAL)
        {
            // LOCAL Mode: Use filtered/scaled value with deadband
            if (abs(int16_t(scaledLocalValue) - int16_t(m_value.getValue())) > POT_DEADBAND)
            {
                newValue = scaledLocalValue;
            }
        }
        else // E_AUX_REMOTE Mode
        {
            newValue = m_remoteValue.getValue(); // Already clamped in mb_tcp_write
        }

        if (m_value.update(newValue))
        {
            notifyStateChange();
        }

        return E_OK;
    }

    // --------------------------------------------------- diagnostics ----
    short debug() override { return info(); }

    short info(short = 0, short = 0) override
    {
        /*
        Log.verboseln("POT::info - ID:%d Pin:%d Val:%d Algo:%d Mode:%d RemoteVal:%d Raw:%d Reverse:%s Address:%d",
                      id, pin, m_value.getValue(), static_cast<uint8_t>(algo),
                      m_mode.getValue(), m_remoteValue.getValue(), analogRead(pin), reverse ? "Yes" : "No", mb_tcp_base_address());
        */
        return E_OK;
    }

    // --------------------------------------------------- Modbus ---------
    short mb_tcp_write(MB_Registers* reg, short networkValue) override
    {
        short result = NetworkComponent::mb_tcp_write(reg, networkValue);
        if (result != E_NOT_IMPLEMENTED) {
            return result;
        }

        uint16_t address = reg->startAddress;
        const uint16_t baseAddr = mb_tcp_base_address();

        if (address == (baseAddr + MB_OFS_HR_VALUE)) 
        {
            // Value register is read-only
            return MODBUS_ERROR_ILLEGAL_FUNCTION;
        }
        else if (address == (baseAddr + MB_OFS_HR_MODE))
        {
            uint16_t clampedMode = (networkValue == 0) ? static_cast<uint16_t>(E_POTControlMode::E_AUX_LOCAL) : static_cast<uint16_t>(E_POTControlMode::E_AUX_REMOTE);
            m_mode.update(clampedMode);
            return E_OK;
        }
        else if (address == (baseAddr + MB_OFS_HR_REMOTE_VALUE))
        {
            uint16_t clampedValue = constrain(networkValue, 0, POT_SCALED_MAX_VALUE);
            m_remoteValue.update(clampedValue);
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }

    short mb_tcp_read(MB_Registers* reg) override
    {
        short result = NetworkComponent::mb_tcp_read(reg);
        if (result != E_NOT_IMPLEMENTED) {
            return result;
        }

        uint16_t address = reg->startAddress;
        const uint16_t baseAddr = mb_tcp_base_address();
        
        if (address == (baseAddr + MB_OFS_HR_VALUE))
        {
            return m_value.getValue();
        }
        else if (address == (baseAddr + MB_OFS_HR_MODE))
        {
            return m_mode.getValue();
        }
        else if (address == (baseAddr + MB_OFS_HR_REMOTE_VALUE))
        {
            return m_remoteValue.getValue();
        }
        return 0;
    }

    // --------------------------------------------------- public helpers -
    uint16_t getValue() const { return m_value.getValue(); }

private:
    // ----------------------------- constants ----------------------------
    static constexpr uint8_t log2_WIN_LEN =
        (POT_DA_WIN_LEN == 1) ? 0 :
        (POT_DA_WIN_LEN == 2) ? 1 :
        (POT_DA_WIN_LEN == 4) ? 2 :
        (POT_DA_WIN_LEN == 8) ? 3 :
        (POT_DA_WIN_LEN == 16)? 4 : 0; // extend if larger windows used

    // ----------------------------- members ------------------------------
    uint16_t ring[POT_DA_WIN_LEN] = {0};  // circular buffer
    uint32_t acc          = 0;            // running sum
    uint8_t  idx          = 0;            // buffer index
    uint16_t filtRaw      = 0;            // filtered raw sample
};

#endif // POT_H

/**
 * @file AnalogLevelSwitch.h
 * @brief Component to read an analog input as a multi-position switch.
 *
 * --- Resistor Selection for Voltage Divider Setup ---
 *
 * This component is designed to interpret an analog voltage as a discrete position or slot.
 * It allows for an initial ADC offset (adcValueOffset), meaning the first slot does not
 * necessarily start at an ADC reading of 0.
 *
 * Principle:
 * A common way to achieve this is with a voltage divider. You'll have one analog input pin.
 * The circuit typically involves one fixed resistor (R_fixed) and a set of switched resistors
 * (R_sw0, R_sw1, ..., R_swN-1), one for each of the N slots.
 *
 * Example Case:
 * - Number of slots (numLevels): 4
 * - ADC Value Offset (adcValueOffset): 200 (e.g., readings 0-199 are effectively below the first slot)
 * - Slot Width (levelStep): 800 ADC counts per slot.
 * - System Voltage (V_in): 5V
 * - ADC Range: 0-4095 (0V -> 0, 5V -> 4095)
 *
 * Component Constructor Parameters:
 * - numLevels: 4
 * - levelStep: 800
 * - adcValueOffset: 200
 *
 *   This means the ADC windows for slots are:
 *     - Slot 0: ADC readings from 200 to (200 + 800 - 1) = 999
 *     - Slot 1: ADC readings from (200 + 800) = 1000 to (200 + 2*800 - 1) = 1799
 *     - Slot 2: ADC readings from (200 + 2*800) = 1800 to (200 + 3*800 - 1) = 2599
 *     - Slot 3: ADC readings from (200 + 3*800) = 2600 to (200 + 4*800 - 1) = 3399
 *   The highest ADC value mapped to a slot is 3399. Readings above this (e.g. > 3399)
 *   will be clamped to the last slot (Slot 3). Readings below the offset (e.g. < 200)
 *   will be clamped to the first slot (Slot 0) by the component's logic.
 *
 * Circuit Configuration Example:
 * - R_fixed is connected from the Analog Input Pin to Ground (GND).
 * - For each slot, a different resistor (R_sw0, R_sw1, etc.) is connected from the
 *   Analog Input Pin to V_in (5V).
 * - The voltage at the Analog Input Pin (V_out) is given by:
 *   V_out = V_in * (R_fixed / (R_sw_current + R_fixed))  (if R_sw to V_in, R_fixed to GND)
 *   Alternatively, if R_fixed to V_in and R_sw to GND (less common for increasing voltage with switch):
 *   V_out = V_in * (R_sw_current / (R_fixed + R_sw_current))
 * - Assuming the first configuration (R_fixed to GND, R_sw to V_in):
 *   The ADC reading is: ADC_value = (V_out / V_in) * 4095 (for this example's V_in and ADC range)
 *
 * Target ADC Values & Resistor Calculation (Adjusted for offset):
 * Target midpoints for ADC windows:
 *   - Slot 0 (200-999):  Midpoint ~ (200+999)/2 = 599  => V_out = (599/4095)*5V  ~ 0.731V
 *   - Slot 1 (1000-1799): Midpoint ~ (1000+1799)/2 = 1399 => V_out = (1399/4095)*5V ~ 1.708V
 *   - Slot 2 (1800-2599): Midpoint ~ (1800+2599)/2 = 2199 => V_out = (2199/4095)*5V ~ 2.685V
 *   - Slot 3 (2600-3399): Midpoint ~ (2600+3399)/2 = 2999 => V_out = (2999/4095)*5V ~ 3.662V
 *
 * Let R_fixed = 10 kOhm (to GND). R_sw_current connects Analog Pin to 5V.
 * V_out / V_in = R_fixed / (R_sw_current + R_fixed)  => This formula gives decreasing V_out for increasing R_sw.
 * For increasing V_out with slots, it's usually R_fixed to VCC and R_sw_current to GND for each slot,
 * where V_out = VCC * (R_sw_current / (R_fixed + R_sw_current)).
 * Or, a ladder network. The example below assumes R_fixed to GND, and R_sw to V_in, which creates HIGHER voltages for LOWER R_sw.
 * This means R_sw needs to DECREASE to get higher voltages / higher slot numbers.
 * V_out = V_in * R_fixed / (R_sw + R_fixed) is not what we want if R_sw is the switched part to V_in for *increasing* voltage steps.
 * Let's re-evaluate the voltage divider formula application for this common use case:
 *
 * Corrected Circuit Configuration for Increasing Voltage with Slot Index:
 * A simple way is multiple resistors (R0, R1, R2, R3) connected via a rotary switch to the analog pin.
 * The other end of these resistors goes to V_in (5V). A single resistor R_pull_down goes from analog pin to GND.
 * V_out = V_in * (R_pull_down / (R_current_switched_to_Vin + R_pull_down)).
 * This means R_current_switched_to_Vin must DECREASE for V_out to INCREASE.
 * Example Values (R_pull_down = 10k Ohm from Analog Pin to GND):
 *
 *   - Slot 0 (V_out ~ 0.731V -> R_sw0 to 5V should be large):
 *     0.731V = 5V * (10k / (R_sw0 + 10k)) => R_sw0 + 10k = 5V/0.731V * 10k = 68.4k => R_sw0 ~ 58.4 kOhm. (Std: 56k)
 *     Using 56k: V_out = 5V * (10k / (56k+10k)) ~ 0.757V; ADC ~ 620. (Slot 0: 200-999)
 *
 *   - Slot 1 (V_out ~ 1.708V -> R_sw1 to 5V should be smaller):
 *     1.708V = 5V * (10k / (R_sw1 + 10k)) => R_sw1 + 10k = 5V/1.708V * 10k = 29.27k => R_sw1 ~ 19.27 kOhm. (Std: 20k or 18k)
 *     Using 20k: V_out = 5V * (10k / (20k+10k)) ~ 1.667V; ADC ~ 1365. (Slot 1: 1000-1799)
 *
 *   - Slot 2 (V_out ~ 2.685V -> R_sw2 to 5V should be smaller still):
 *     2.685V = 5V * (10k / (R_sw2 + 10k)) => R_sw2 + 10k = 5V/2.685V * 10k = 18.62k => R_sw2 ~ 8.62 kOhm. (Std: 8.2k)
 *     Using 8.2k: V_out = 5V * (10k / (8.2k+10k)) ~ 2.747V; ADC ~ 2250. (Slot 2: 1800-2599)
 *
 *   - Slot 3 (V_out ~ 3.662V -> R_sw3 to 5V should be smallest):
 *     3.662V = 5V * (10k / (R_sw3 + 10k)) => R_sw3 + 10k = 5V/3.662V * 10k = 13.65k => R_sw3 ~ 3.65 kOhm. (Std: 3.6k)
 *     Using 3.6k: V_out = 5V * (10k / (3.6k+10k)) ~ 3.676V; ADC ~ 3011. (Slot 3: 2600-3399)
 *
 * Summary of example resistor values (R_pull_down = 10k to GND, V_in=5V, ADC Offset=200, Step=800):
 * Each R_swX is connected between 5V and the Analog Input pin when its slot is active.
 *   - Slot 0: R_sw0 = 56k
 *   - Slot 1: R_sw1 = 20k
 *   - Slot 2: R_sw2 = 8.2k
 *   - Slot 3: R_sw3 = 3.6k
 *
 * Important Considerations:
 * - Resistor Tolerances: Use 1% or better resistors if possible, or account for tolerances
 *   to ensure ADC readings for different slots don't overlap.
 * - ADC Linearity & Noise: Real-world ADCs have non-linearities and noise. Provide sufficient
 *   margin between the target ADC values for each slot.
 * - ADC Input Impedance: Ensure the equivalent resistance of the voltage divider is not too high,
 *   as it can affect ADC reading accuracy due to the ADC's input impedance and sample/hold capacitor charging.
 *   Typically, values in the 1k to 100k range for the overall divider are fine for many MCUs.
 * - Debouncing: If the switch is mechanical, you might need software or hardware debouncing,
 *   though this component reads periodically based on ANALOG_SWITCH_READ_INTERVAL.
 */

#ifndef ANALOG_LEVEL_SWITCH_H
#define ANALOG_LEVEL_SWITCH_H

#include <ArduinoLog.h>
#include "config.h" // For ANALOG_SWITCH_READ_INTERVAL if not overridden by ALS_READ_INTERVAL_MS
#include <App.h>
#include <Component.h>
#include <modbus/ModbusTCP.h> 
#include "config-modbus.h"
#include <stdint.h> // For uint16_t, uint32_t

// ─────────────────────────────────────────────────────────────────────────────
//  Compile‑time configuration (moved from .cpp)
// ─────────────────────────────────────────────────────────────────────────────
#ifndef ANALOG_LVL_SLOTS_MAX
#define ANALOG_LVL_SLOTS_MAX      32      // upper bound enforced at runtime
#endif

#ifndef ALS_SMOOTHING_SIZE
#define ALS_SMOOTHING_SIZE        8       // samples in moving‑average buffer
#endif

#ifndef ALS_DEBOUNCE_COUNT
#define ALS_DEBOUNCE_COUNT        3       // identical detections before commit
#endif

#ifndef ALS_HYSTERESIS_CODES
#define ALS_HYSTERESIS_CODES      4       // ±ADC codes guard‑band
#endif

#ifndef ALS_READ_INTERVAL_MS
#define ALS_READ_INTERVAL_MS      25      // Override for ANALOG_SWITCH_READ_INTERVAL
#endif

#ifndef ALS_USE_EMA            // undef to keep simple moving average
#define ALS_USE_EMA               0       // 0 = MA, 1 = EMA(α = 1/ALS_SMOOTHING_SIZE)
#endif
// ─────────────────────────────────────────────────────────────────────────────

class Bridge;

class AnalogLevelSwitch : public Component
{
public:
    // Removed old static const members, replaced by defines above
    // static const short SMOOTHING_ARRAY_SIZE = 10; 
    // static const short MAX_ANALOG_LEVELS = 16;
    // static const short DEBOUNCE_CONFIRMATIONS_COUNT = 3;

    enum class AnalogLevelRegOffset : uint16_t { // Changed underlying type to uint16_t
        DETECTED_LEVEL = 0,
        RAW_ANALOG_VALUE = 1,
        LEVEL_STATE_START = 2 
    };

private:
    const short m_pin;
    const uint16_t m_slotCount;
    const uint16_t m_adcStepPerSlot; // Changed from int
    const uint16_t m_adcOffset;      // Changed from int
    const uint16_t m_modbusAddr;
    
    uint16_t m_activeSlot;
    uint16_t m_adcRaw;

    // Smoothing data (fixed array)
    uint16_t m_adcBuffer[ALS_SMOOTHING_SIZE]; // Use new define
    uint16_t m_bufferIdx = 0;
    uint32_t m_bufferSum = 0; // Changed from long to uint32_t for consistency
    uint16_t m_adcSmoothed = 0;

    // Debouncing data
    uint16_t m_proposedSlot = 0;
    uint16_t m_confirmCount = 0;

    // Modbus definitions 
    MB_Registers m_modbusBlocks[2 + ANALOG_LVL_SLOTS_MAX]; // Use new define
    uint16_t m_modbusBlockCount = 0;           
    ModbusBlockView m_modbusView;                      

    // Private helpers
    void buildModbusBlocks(); // Added declaration
    // Updated signature for determineSlotFromValue
    uint16_t determineSlotFromValue(uint16_t adcVal, uint16_t currentSlot = UINT16_MAX) const;


public:
    AnalogLevelSwitch(
        Component *owner,
        short _analogPin,
        uint16_t _numLevels,     // Changed from ushort/short
        uint16_t _levelStep,     // Changed from int
        uint16_t _adcValueOffset,// Changed from int
        short _id,
        uint16_t _modbusAddress); // Changed from ushort

    short setup() override;
    short loop() override;
    short info(short val0 = 0, short val1 = 0) override;
    short debug() override { return info(0, 0); }

    // Return types adjusted
    uint16_t getActiveSlot() const { return m_activeSlot; } // Changed from ushort
    uint16_t getRawAdc() const { return m_adcRaw; }         // Changed from ushort
    uint16_t getSmoothedAdc() const { return m_adcSmoothed; } // Changed from ushort

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;
    void mb_tcp_register(ModbusTCP *manager) override;
    ModbusBlockView *mb_tcp_blocks() const override;

    short serial_register(Bridge *bridge) override;

protected:
    void notifyStateChange() override;
    unsigned long m_lastReadMs = 0;
};

#endif // ANALOG_LEVEL_SWITCH_H 
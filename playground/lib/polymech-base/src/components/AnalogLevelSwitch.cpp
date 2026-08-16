// ============================================================================
//  AnalogLevelSwitch – *revisited*
//  --------------------------------------------------------------------------
//  Drop‑in replacement for the original AnalogLevelSwitch class.
//  • Adds static hysteresis around level boundaries to prevent dithering.
//  • Uses constexpr, PROGMEM strings and wider arithmetic to avoid overflow.
//  • Allows optional exponential smoothing at compile‑time.
//  • Keeps the public API identical, so existing sketches don't break.
//  ----------------------------------------------------------------------------
//  © 2025  (MIT‑licensed)
// ============================================================================

#include "AnalogLevelSwitch.h"
#include <Arduino.h>
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
//  Compile‑time configuration
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
#define ALS_READ_INTERVAL_MS      25
#endif

#ifndef ALS_USE_EMA            // undef to keep simple moving average
#define ALS_USE_EMA               0       // 0 = MA, 1 = EMA(α = 1/ALS_SMOOTHING_SIZE)
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers / macro sugar
// ─────────────────────────────────────────────────────────────────────────────
#if defined(ARDUINO_ARCH_AVR)
  #define ALS_L(FSTR)              F(FSTR)
#else
  #define ALS_L(FSTR)              (FSTR)
#endif

static constexpr uint16_t ADC_MAX_VALUE = 4095;    // 12‑bit default

// Forward declarations (private helpers) …………………………………………
namespace {
    template <typename T, typename U>
    static constexpr T clamp(T val, U lo, U hi) {
        return val < lo ? lo : (val > hi ? hi : val);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  CTOR
// ─────────────────────────────────────────────────────────────────────────────
AnalogLevelSwitch::AnalogLevelSwitch(Component *owner,
                                     short _analogPin,
                                     uint16_t _numLevels,
                                     uint16_t _levelStep,
                                     uint16_t _adcValueOffset,
                                     short _id,
                                     uint16_t _modbusAddress)
    : Component("AnalogLevelSwitch", _id, Component::COMPONENT_DEFAULT, owner),
      m_pin              (_analogPin),
      m_slotCount        ((_numLevels > 0 && _numLevels <= ANALOG_LVL_SLOTS_MAX) ? _numLevels
                               : (_numLevels > ANALOG_LVL_SLOTS_MAX ? ANALOG_LVL_SLOTS_MAX : 1)),
      m_adcStepPerSlot   (_levelStep  > 0          ? _levelStep : 1),
      m_adcOffset        (_adcValueOffset),
      m_modbusAddr       (_modbusAddress),
      m_activeSlot       (0),
      m_adcRaw           (0),
      m_bufferIdx        (0),
      m_bufferSum        (0),
      m_adcSmoothed      (0),
      m_proposedSlot     (0),
      m_confirmCount     (0),
      m_modbusBlockCount (0)
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

    // ── Sanity logging ──────────────────────────────────────────────
    if (_numLevels <= 0)
        Log.warningln(ALS_L("ALS ID:%d: numLevels invalid, using 1."), id);
    else if (_numLevels > ANALOG_LVL_SLOTS_MAX)
        Log.warningln(ALS_L("ALS ID:%d: numLevels %d > MAX %d, clamping to MAX."),
                       id, _numLevels, ANALOG_LVL_SLOTS_MAX);

    if (_levelStep <= 0)
        Log.warningln(ALS_L("ALS ID:%d: levelStep invalid, using 1."), id);

    uint32_t maxCfgAdc = (uint32_t)m_adcOffset + (uint32_t)m_slotCount * (uint32_t)m_adcStepPerSlot - 1;
    if (maxCfgAdc > ADC_MAX_VALUE)
        Log.warningln(ALS_L("ALS ID:%d: Max slot ADC val (%lu) > ADC_MAX (%d)."), id, maxCfgAdc, ADC_MAX_VALUE);

    if (m_adcOffset >= ADC_MAX_VALUE)
        Log.errorln(ALS_L("ALS ID:%d: adcOffset (%d) >= ADC_MAX (%d). Slots unreadable."), id, m_adcOffset, ADC_MAX_VALUE);

    // ── Initialise smoothing buffer ─────────────────────────────────
    memset(m_adcBuffer, 0, sizeof(m_adcBuffer));
    uint16_t initialRaw = analogRead(m_pin);
    for (uint8_t i = 0; i < ALS_SMOOTHING_SIZE; ++i) m_adcBuffer[i] = initialRaw;
    m_bufferSum  = (uint32_t)initialRaw * ALS_SMOOTHING_SIZE;
    m_adcSmoothed = initialRaw;
    m_adcRaw      = initialRaw;

    m_activeSlot   = determineSlotFromValue(m_adcSmoothed);
    m_proposedSlot = m_activeSlot;
    m_confirmCount = ALS_DEBOUNCE_COUNT;

    // ── Build static Modbus block table (unchanged) ─────────────────
    buildModbusBlocks();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Private: buildModbusBlocks()
// ─────────────────────────────────────────────────────────────────────────────
void AnalogLevelSwitch::buildModbusBlocks() {
    memset(m_modbusBlocks, 0, sizeof(m_modbusBlocks));
    const char *anaLvlGroup = "AnaLvl";
    const char *coilGroup   = "Coil";

    uint8_t idx = 0;

    // Detected level register
    m_modbusBlocks[idx++] = {
        static_cast<uint16_t>(m_modbusAddr + static_cast<uint16_t>(AnalogLevelRegOffset::DETECTED_LEVEL)),
        1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY,
        static_cast<uint16_t>(id), static_cast<uint16_t>(AnalogLevelRegOffset::DETECTED_LEVEL),
        "Detected Level", anaLvlGroup };

    // Raw ADC value register
    m_modbusBlocks[idx++] = {
        static_cast<uint16_t>(m_modbusAddr + static_cast<uint16_t>(AnalogLevelRegOffset::RAW_ANALOG_VALUE)),
        1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY,
        static_cast<uint16_t>(id), static_cast<uint16_t>(AnalogLevelRegOffset::RAW_ANALOG_VALUE),
        "Raw Analog Value", anaLvlGroup };

    // Per‑slot coil‑style bits
    for (uint16_t s = 0; s < m_slotCount && idx < (2 + ANALOG_LVL_SLOTS_MAX); ++s) {
        uint16_t regOff = static_cast<uint16_t>(AnalogLevelRegOffset::LEVEL_STATE_START) + s;
        m_modbusBlocks[idx++] = {
            static_cast<uint16_t>(m_modbusAddr + regOff),
            1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY,
            static_cast<uint16_t>(id), regOff,
            "Level Slot Active", coilGroup };
    }

    m_modbusBlockCount = idx;
    m_modbusView.data  = m_modbusBlocks;
    m_modbusView.count = m_modbusBlockCount;
}

// ─────────────────────────────────────────────────────────────────────────────
//  setup()
// ─────────────────────────────────────────────────────────────────────────────
short AnalogLevelSwitch::setup() {
    Component::setup();

#if defined(ESP32)
    analogReadResolution(12);
    analogSetPinAttenuation(m_pin, ADC_11db);     // 0‑3.6 V full‑scale
#endif

    pinMode(m_pin, INPUT);

    // Prime moving average / EMA buffer
    uint16_t first = analogRead(m_pin);
#if ALS_USE_EMA
    m_adcSmoothed = first;
#else
    for (uint8_t i = 0; i < ALS_SMOOTHING_SIZE; ++i) m_adcBuffer[i] = first;
    m_bufferSum   = (uint32_t)first * ALS_SMOOTHING_SIZE;
    m_adcSmoothed = first;
#endif
    m_adcRaw      = first;
    m_activeSlot  = determineSlotFromValue(m_adcSmoothed);
    m_proposedSlot= m_activeSlot;
    m_confirmCount= ALS_DEBOUNCE_COUNT;

    Log.verboseln(ALS_L("ALS ID:%d setup. Pin:%d, Slots:%d, Step:%d, Offset:%d, Smooth:%d, Debounce:%d, Hysteresis:%d, Initial Slot:%d, Raw:%d"),
                  id, m_pin, m_slotCount, m_adcStepPerSlot, m_adcOffset,
                  ALS_SMOOTHING_SIZE, ALS_DEBOUNCE_COUNT, ALS_HYSTERESIS_CODES,
                  m_activeSlot, m_adcRaw);
    return E_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
//  determineSlotFromValue()  – with static hysteresis
// ─────────────────────────────────────────────────────────────────────────────
uint16_t AnalogLevelSwitch::determineSlotFromValue(uint16_t adcVal, uint16_t currentSlot /*= UINT16_MAX*/) const {
    // If caller provides currentSlot we clip searches to ±1 neighbor for speed.
    uint16_t slotLo = 0;
    uint16_t slotHi = m_slotCount - 1;
    if (currentSlot != UINT16_MAX) {
        slotLo = (currentSlot > 0)            ? currentSlot - 1 : 0;
        slotHi = (currentSlot + 1 < m_slotCount) ? currentSlot + 1 : m_slotCount - 1;
    }

    for (uint16_t s = slotLo; s <= slotHi; ++s) {
        uint32_t lowThr  = (uint32_t)m_adcOffset + (uint32_t)m_adcStepPerSlot * s - ALS_HYSTERESIS_CODES;
        uint32_t highThr = lowThr + m_adcStepPerSlot + 2 * ALS_HYSTERESIS_CODES;
        if (adcVal < lowThr) continue;
        if (adcVal < highThr) return s;
    }
    // If we fall through, clamp to ends.
    return (adcVal < (uint32_t)m_adcOffset) ? 0 : (m_slotCount - 1);
}

// ─────────────────────────────────────────────────────────────────────────────
//  loop()
// ─────────────────────────────────────────────────────────────────────────────
short AnalogLevelSwitch::loop() {
    Component::loop();
    if (now - m_lastReadMs < ALS_READ_INTERVAL_MS) return E_OK;
    m_lastReadMs = now;

    // ── Acquire & smooth sample ─────────────────────────────────────
    m_adcRaw = analogRead(m_pin);

#if ALS_USE_EMA
    // α = 1/n (n = smoothing size)
    m_adcSmoothed = m_adcSmoothed - (m_adcSmoothed / ALS_SMOOTHING_SIZE) + (m_adcRaw / ALS_SMOOTHING_SIZE);
#else
    m_bufferSum -= m_adcBuffer[m_bufferIdx];
    m_adcBuffer[m_bufferIdx] = m_adcRaw;
    m_bufferSum += m_adcRaw;
    m_bufferIdx = (m_bufferIdx + 1) % ALS_SMOOTHING_SIZE;
    m_adcSmoothed = m_bufferSum / ALS_SMOOTHING_SIZE;
#endif

    // ── Hysteresis + debounce ──────────────────────────────────────
    uint16_t candidate = determineSlotFromValue(m_adcSmoothed, m_proposedSlot);

    if (candidate == m_proposedSlot) {
        if (m_confirmCount < ALS_DEBOUNCE_COUNT) ++m_confirmCount;
    } else {
        m_proposedSlot = candidate;
        m_confirmCount = 1;
    }

    if (m_confirmCount >= ALS_DEBOUNCE_COUNT && m_proposedSlot != m_activeSlot) {
        Log.verboseln(ALS_L("ALS ID:%d: Slot %d → %d (Raw:%d Smooth:%d)"),
                      id, m_activeSlot, m_proposedSlot, m_adcRaw, m_adcSmoothed);
        m_activeSlot = m_proposedSlot;
        notifyStateChange();
    }
    return E_OK;
}

// ─────────────────────────────────────────────────────────────────────────────
//  info(), Modbus stubs, etc.  (mostly unchanged, minor formatting tweaks)
// ─────────────────────────────────────────────────────────────────────────────
short AnalogLevelSwitch::info(short, short) {
    Log.infoln(ALS_L("AnalogLevelSwitch::info – ID:%d"), id);
    Log.infoln(ALS_L("  Pin:%d  Slots:%d  Step:%d  Offset:%d"), m_pin, m_slotCount, m_adcStepPerSlot, m_adcOffset);
    Log.infoln(ALS_L("  Smooth:%d  Debounce:%d  Hyst:%d  ADCMax:%d"),
               ALS_SMOOTHING_SIZE, ALS_DEBOUNCE_COUNT, ALS_HYSTERESIS_CODES, ADC_MAX_VALUE);
    Log.infoln(ALS_L("  Current:%d  Raw:%d  Smoothed:%d"), m_activeSlot, m_adcRaw, m_adcSmoothed);
    return E_OK;
}

void AnalogLevelSwitch::notifyStateChange() {
    Component::notifyStateChange();
}

//  Modbus read/write/registration – unchanged from original version …
short AnalogLevelSwitch::mb_tcp_write(MB_Registers *reg, short netVal) {
    uint16_t addr = reg->startAddress;
    uint16_t base = m_modbusAddr + static_cast<uint16_t>(AnalogLevelRegOffset::LEVEL_STATE_START);
    uint16_t end  = base + m_slotCount;

    if (addr >= base && addr < end) {
        Log.warningln(ALS_L("ALS ID:%d: Write to slot‑state address %d not supported."), id, addr);
        return MODBUS_ERROR_ILLEGAL_FUNCTION;
    }
    if (addr == m_modbusAddr + static_cast<uint16_t>(AnalogLevelRegOffset::DETECTED_LEVEL) ||
        addr == m_modbusAddr + static_cast<uint16_t>(AnalogLevelRegOffset::RAW_ANALOG_VALUE)) {
        Log.warningln(ALS_L("ALS ID:%d: Write to read‑only address %d."), id, addr);
        return MODBUS_ERROR_ILLEGAL_FUNCTION;
    }
    Log.warningln(ALS_L("ALS ID:%d: Write to unknown address %d."), id, addr);
    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
}

short AnalogLevelSwitch::mb_tcp_read(MB_Registers *reg) {
    uint16_t addr = reg->startAddress;
    if (addr == m_modbusAddr + static_cast<uint16_t>(AnalogLevelRegOffset::DETECTED_LEVEL))       return m_activeSlot;
    if (addr == m_modbusAddr + static_cast<uint16_t>(AnalogLevelRegOffset::RAW_ANALOG_VALUE))     return m_adcRaw;

    uint16_t base = m_modbusAddr + static_cast<uint16_t>(AnalogLevelRegOffset::LEVEL_STATE_START);
    uint16_t end  = base + m_slotCount;
    if (addr >= base && addr < end) {
        uint16_t slotIdx = addr - base;
        return (slotIdx == m_activeSlot) ? 1 : 0;
    }
    Log.warningln(ALS_L("ALS ID:%d: Read from unknown address %d."), id, addr);
    return 0;
}

void AnalogLevelSwitch::mb_tcp_register(ModbusTCP *mgr) {
    auto *blocks = mb_tcp_blocks();
    if (!blocks || !blocks->data) return;
    Component *thiz = const_cast<AnalogLevelSwitch*>(this);
    for (uint8_t i = 0; i < blocks->count; ++i) mgr->registerModbus(thiz, blocks->data[i]);
}

ModbusBlockView *AnalogLevelSwitch::mb_tcp_blocks() const {
    return const_cast<ModbusBlockView*>(&m_modbusView);
}

short AnalogLevelSwitch::serial_register(Bridge *bridge) {
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&AnalogLevelSwitch::info);
    return E_OK;
}

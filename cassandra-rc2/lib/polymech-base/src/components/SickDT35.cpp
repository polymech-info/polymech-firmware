#include <App.h>
#include <Bridge.h>
#include <modbus/Modbus.h>
#include <cstring>
#include "./SickDT35.h"

namespace
{
constexpr uint16_t kFactoryAnalogNear = 50;
constexpr uint16_t kFactoryAnalogFar = 10000;
constexpr uint16_t kFactoryAnalogCenter = 5025;
constexpr uint16_t kFactoryHyst = 25;
constexpr uint16_t kFactoryQ1Far = 10000;
constexpr uint16_t kFactoryDto = 10000;
}

const SickDT35::RegDesc *SickDT35::regTable(size_t &count)
{
    static const RegDesc kTable[] = {
        {MB_OFS_HR_DISTANCE, "Distance mm", false, 0, 0, 50000},
        {MB_OFS_HR_RAW_ADC, "Raw ADC", false, 0, 0, SICK_DT35_ADC_MAX},
        {MB_OFS_HR_Q1, "Q1", false, 0, 0, 1},
        {MB_OFS_HR_Q2, "Q2", false, 0, 0, 1},
        {MB_OFS_HR_VALID, "Valid", false, 0, 0, 1},
        {MB_OFS_HR_STATUS, "Status", false, 0, 0, 0xFFFF},
        {MB_OFS_HR_MODE, "Mode", true, E_AUX_LOCAL, 0, 1},
        {MB_OFS_HR_REMOTE_DISTANCE, "Remote Distance", true, 0, 0, SICK_DT35_RANGE_MAX_MM},
        {MB_OFS_HR_ADC_NEAR, "ADC Near", true, 0, 0, SICK_DT35_ADC_MAX},
        {MB_OFS_HR_ADC_FAR, "ADC Far", true, SICK_DT35_ADC_MAX, 0, SICK_DT35_ADC_MAX},
        {MB_OFS_HR_Q1_FUNCTION, "Q1 Function", true, SW_DTO, 0, SW_ALARM},
        {MB_OFS_HR_Q1_NEAR, "Q1 Near", true, kFactoryDto, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_Q1_FAR, "Q1 Far", true, kFactoryQ1Far, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_Q1_HYST_NEAR, "Q1 Hyst Near", true, kFactoryHyst, 0, 49550},
        {MB_OFS_HR_Q1_HYST_FAR, "Q1 Hyst Far", true, kFactoryHyst, 0, 49550},
        {MB_OFS_HR_Q1_CENTER, "Q1 Center", true, kFactoryAnalogCenter, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_Q2_OUTPUT, "Q2 Output", true, Q2_CURRENT_4_20, 0, Q2_SWITCHING},
        {MB_OFS_HR_Q2_FUNCTION, "Q2 Function", true, SW_DTO, 0, SW_ALARM},
        {MB_OFS_HR_Q2_NEAR, "Q2 Near", true, kFactoryDto, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_Q2_FAR, "Q2 Far", true, kFactoryQ1Far, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_Q2_HYST_NEAR, "Q2 Hyst Near", true, kFactoryHyst, 0, 49550},
        {MB_OFS_HR_Q2_HYST_FAR, "Q2 Hyst Far", true, kFactoryHyst, 0, 49550},
        {MB_OFS_HR_Q2_CENTER, "Q2 Center", true, kFactoryAnalogCenter, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_ANALOG_NEAR, "Analog Near", true, kFactoryAnalogNear, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_ANALOG_FAR, "Analog Far", true, kFactoryAnalogFar, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_ANALOG_CENTER, "Analog Center", true, kFactoryAnalogCenter, SICK_DT35_RANGE_MIN_MM, 50000},
        {MB_OFS_HR_VMA_THRESHOLD, "VMA Threshold", true, 0, 0, 65535},
        {MB_OFS_HR_INVERSION, "Inversion", true, 0, 0, 3},
        {MB_OFS_HR_DISTANCE_OFFSET, "Distance Offset", true, 0, 0, 50000},
        {MB_OFS_HR_TIMER_FN, "Timer Function", true, TIMER_OFF, 0, TIMER_ONESHOT},
        {MB_OFS_HR_TIMER_MS, "Timer ms", true, 0, 0, 255},
        {MB_OFS_HR_SPEED, "Speed", true, SPEED_SLOW, 0, SPEED_SUPER_FAST},
        {MB_OFS_HR_INTEGRATION, "Integration", true, 0, 0, 8},
        {MB_OFS_HR_AVERAGING, "Averaging", true, 1, 1, SICK_DT35_AVG_MAX},
        {MB_OFS_HR_BIT_FILTER, "Bit Filter", true, 0, 0, SICK_DT35_AVG_MAX},
        {MB_OFS_HR_TEACH, "Teach", true, 0, 0, TEACH_FINE_MINUS_10},
        {MB_OFS_HR_PD_STRUCTURE, "PD Structure", true, 3, 0, 4},
        {MB_OFS_HR_PD_RESOLUTION, "PD Resolution", true, 1, 0, 2},
        {MB_OFS_HR_PD_NORM, "PD Norm", true, 0, 0, 50000},
        {MB_OFS_HR_MF_FUNCTION, "MF Function", true, MF_LASER, 0, MF_OFF},
        {MB_OFS_HR_MF_LEVEL, "MF Level", true, 0, 0, 3},
        {MB_OFS_HR_ALARM_FN, "Alarm Function", true, ALARM_CLAMP, 0, ALARM_HOLD},
        {MB_OFS_HR_PB_LOCK, "Pushbutton Lock", true, 0, 0, 1},
        {MB_OFS_HR_LASER, "Laser", true, 1, 0, 1},
        {MB_OFS_HR_COMMAND, "Command", true, 0, 0, 255},
    };
    count = sizeof(kTable) / sizeof(kTable[0]);
    return kTable;
}

const SickDT35::RegDesc *SickDT35::findReg(uint16_t offset)
{
    size_t count = 0;
    const RegDesc *table = regTable(count);
    for (size_t i = 0; i < count; ++i)
    {
        if (table[i].offset == offset)
        {
            return &table[i];
        }
    }
    return nullptr;
}

SickDT35::SickDT35(Component *owner,
                   uint16_t _analogPin,
                   uint16_t _id,
                   uint16_t _modbusAddress,
                   uint16_t _q1Pin,
                   uint16_t _mfPin)
    : NetworkComponent(_modbusAddress, "SickDT35(" + String(_modbusAddress) + ")", _id, Component::COMPONENT_DEFAULT, owner),
      analogPin(_analogPin),
      q1Pin(_q1Pin),
      mfPin(_mfPin),
      m_distance(this, _id, "DT35 Distance"),
      m_rawAdc(this, _id, "DT35 Raw ADC"),
      m_q1(this, _id, "DT35 Q1"),
      m_q2(this, _id, "DT35 Q2"),
      m_valid(this, _id, "DT35 Valid"),
      m_status(this, _id, "DT35 Status"),
      m_mode(this, _id, "DT35 Mode"),
      m_remoteDistance(this, _id, "DT35 Remote Distance"),
      m_lastTeachTarget(MB_OFS_HR_Q1_NEAR),
      m_avgSum(0),
      m_avgIdx(0),
      m_avgFill(0),
      m_filtAdc(0),
      m_heldDistance(0),
      m_q1Streak(0),
      m_q2Streak(0),
      m_q1EdgeMs(0),
      m_q2EdgeMs(0),
      m_q1Oneshot(false),
      m_q2Oneshot(false),
      m_q1On(false),
      m_q2On(false),
      m_lastSample(0),
      m_mfPulseUntil(0)
{
    name = "SickDT35[" + String(_modbusAddress) + "]";
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    memset(m_cfg, 0, sizeof(m_cfg));
    memset(m_avgRing, 0, sizeof(m_avgRing));
    applyFactoryDefaults();
}

void SickDT35::applyFactoryDefaults()
{
    size_t count = 0;
    const RegDesc *table = regTable(count);
    for (size_t i = 0; i < count; ++i)
    {
        m_cfg[table[i].offset] = table[i].def;
    }
    m_avgSum = 0;
    m_avgIdx = 0;
    m_avgFill = 0;
    m_filtAdc = 0;
    m_heldDistance = 0;
    m_q1Streak = 0;
    m_q2Streak = 0;
    m_q1Oneshot = false;
    m_q2Oneshot = false;
    m_q1On = false;
    m_q2On = false;
    m_lastTeachTarget = MB_OFS_HR_Q1_NEAR;
    m_distance.applyUpdate(0);
    m_rawAdc.applyUpdate(0);
    m_q1.applyUpdate(0);
    m_q2.applyUpdate(0);
    m_valid.applyUpdate(0);
    m_status.applyUpdate(0);
    m_mode.applyUpdate(E_AUX_LOCAL);
    m_remoteDistance.applyUpdate(0);
}

short SickDT35::setup()
{
    NetworkComponent::setup();

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    analogReadResolution(12);
#endif
    if (pinUsed(analogPin))
    {
        pinMode(analogPin, INPUT);
    }
    if (pinUsed(q1Pin))
    {
        pinMode(q1Pin, INPUT);
    }
    if (pinUsed(mfPin))
    {
        pinMode(mfPin, OUTPUT);
        applyMf();
    }

    const uint16_t baseAddr = mb_tcp_base_address();
    size_t count = 0;
    const RegDesc *table = regTable(count);

    auto bindNv = [&](NetworkValue<uint16_t> &nv, uint16_t offset, E_FN_CODE fn, uint16_t initial, uint16_t threshold) {
        nv.initNotify(initial, threshold, NetworkValue_ThresholdMode::DIFFERENCE);
        nv.initModbus(baseAddr + offset, 1, this->id, this->slaveId, fn, nv.name.c_str(), this->name.c_str());
        registerBlock(nv.getRegisterInfo());
    };

    bindNv(m_distance, MB_OFS_HR_DISTANCE, FN_READ_HOLD_REGISTER, 0, 1);
    bindNv(m_rawAdc, MB_OFS_HR_RAW_ADC, FN_READ_HOLD_REGISTER, 0, 8);
    bindNv(m_q1, MB_OFS_HR_Q1, FN_READ_HOLD_REGISTER, 0, 1);
    bindNv(m_q2, MB_OFS_HR_Q2, FN_READ_HOLD_REGISTER, 0, 1);
    bindNv(m_valid, MB_OFS_HR_VALID, FN_READ_HOLD_REGISTER, 0, 1);
    bindNv(m_status, MB_OFS_HR_STATUS, FN_READ_HOLD_REGISTER, 0, 1);
    bindNv(m_mode, MB_OFS_HR_MODE, FN_WRITE_HOLD_REGISTER, E_AUX_LOCAL, 1);
    bindNv(m_remoteDistance, MB_OFS_HR_REMOTE_DISTANCE, FN_WRITE_HOLD_REGISTER, 0, 1);

    for (size_t i = 0; i < count; ++i)
    {
        const uint16_t offset = table[i].offset;
        if (offset <= MB_OFS_HR_REMOTE_DISTANCE)
        {
            continue;
        }
        const E_FN_CODE fn = table[i].writable ? FN_WRITE_HOLD_REGISTER : FN_READ_HOLD_REGISTER;
        registerBlock(MB_Registers(
            static_cast<uint16_t>(baseAddr + offset),
            1,
            fn,
            table[i].writable ? MB_ACCESS_READ_WRITE : MB_ACCESS_READ_ONLY,
            static_cast<uint16_t>(this->id),
            this->slaveId,
            table[i].name,
            this->name.c_str()));
    }

    m_lastSample = millis();
    return E_OK;
}

uint16_t SickDT35::cfg(uint16_t offset) const
{
    if (offset > MB_OFS_HR_LAST)
    {
        return 0;
    }
    return m_cfg[offset];
}

void SickDT35::setCfg(uint16_t offset, uint16_t value)
{
    if (offset <= MB_OFS_HR_LAST)
    {
        m_cfg[offset] = value;
    }
}

uint16_t SickDT35::clampReg(const RegDesc *desc, int32_t value) const
{
    if (!desc)
    {
        return 0;
    }
    if (value < static_cast<int32_t>(desc->minv))
    {
        return desc->minv;
    }
    if (value > static_cast<int32_t>(desc->maxv))
    {
        return desc->maxv;
    }
    return static_cast<uint16_t>(value);
}

uint16_t SickDT35::sampleIntervalMs() const
{
    switch (cfg(MB_OFS_HR_SPEED))
    {
    case SPEED_SUPER_FAST:
        return 5;
    case SPEED_FAST:
        return 13;
    case SPEED_MEDIUM:
        return 25;
    case SPEED_SUPER_SLOW:
        return 193;
    case SPEED_EXPERT:
    {
        const uint16_t n = cfg(MB_OFS_HR_INTEGRATION);
        uint16_t interval = 2;
        for (uint16_t i = 0; i < n && interval < 512; ++i)
        {
            interval <<= 1;
        }
        return interval;
    }
    case SPEED_SLOW:
    default:
        return 49;
    }
}

uint16_t SickDT35::adcToDistance(uint16_t adc) const
{
    const int32_t adcNear = cfg(MB_OFS_HR_ADC_NEAR);
    const int32_t adcFar = cfg(MB_OFS_HR_ADC_FAR);
    const int32_t mmNear = cfg(MB_OFS_HR_ANALOG_NEAR);
    const int32_t mmFar = cfg(MB_OFS_HR_ANALOG_FAR);
    const int32_t spanAdc = adcFar - adcNear;
    if (spanAdc == 0)
    {
        return static_cast<uint16_t>(mmNear);
    }
    int32_t mm = mmNear + (static_cast<int32_t>(adc) - adcNear) * (mmFar - mmNear) / spanAdc;
    mm += cfg(MB_OFS_HR_DISTANCE_OFFSET);
    if (mm < 0)
    {
        mm = 0;
    }
    if (mm > 50000)
    {
        mm = 50000;
    }
    return static_cast<uint16_t>(mm);
}

uint16_t SickDT35::publishDistance(uint16_t measuredMm, bool valid) const
{
    int32_t mm = measuredMm;
    if (!valid)
    {
        if (cfg(MB_OFS_HR_ALARM_FN) == ALARM_CLAMP)
        {
            mm = 0;
        }
        else
        {
            mm = m_heldDistance;
        }
    }
    mm -= cfg(MB_OFS_HR_PD_NORM);
    if (mm < 0)
    {
        mm = 0;
    }
    switch (cfg(MB_OFS_HR_PD_RESOLUTION))
    {
    case 0:
        if (mm > 6553)
        {
            mm = 6553;
        }
        mm *= 10;
        break;
    case 2:
        mm /= 10;
        break;
    default:
        break;
    }
    if (mm > 65535)
    {
        mm = 65535;
    }
    return static_cast<uint16_t>(mm);
}

bool SickDT35::evaluateSwitch(uint16_t distanceMm,
                              bool valid,
                              uint16_t fn,
                              uint16_t nearMm,
                              uint16_t farMm,
                              uint16_t hystNear,
                              uint16_t hystFar,
                              bool inverted,
                              bool currentOn) const
{
    bool raw = false;
    switch (fn)
    {
    case SW_OBSB:
        raw = valid && (distanceMm < (currentOn ? static_cast<uint32_t>(farMm) + hystFar : farMm));
        break;
    case SW_WINDOW:
    {
        const uint16_t lo = currentOn ? nearMm : static_cast<uint16_t>(nearMm + hystNear);
        const uint16_t hi = currentOn ? farMm : (farMm > hystFar ? static_cast<uint16_t>(farMm - hystFar) : 0);
        raw = valid && (distanceMm >= lo) && (distanceMm <= hi);
        break;
    }
    case SW_VMA:
    case SW_ALARM:
        raw = !valid;
        break;
    case SW_DTO:
    default:
        raw = valid && (distanceMm <= (currentOn ? static_cast<uint32_t>(nearMm) + hystNear : nearMm));
        break;
    }
    return inverted ? !raw : raw;
}

bool SickDT35::applyBitFilter(bool desired, uint8_t &streak, bool currentOn, uint16_t depth) const
{
    if (depth <= 1)
    {
        streak = 0;
        return desired;
    }
    if (desired == currentOn)
    {
        streak = 0;
        return currentOn;
    }
    if (streak < 255)
    {
        ++streak;
    }
    if (streak >= depth)
    {
        streak = 0;
        return desired;
    }
    return currentOn;
}

bool SickDT35::applyTimer(bool filteredOn, bool currentOn, uint32_t &edgeMs, bool &oneshotLatched) const
{
    const uint16_t fn = cfg(MB_OFS_HR_TIMER_FN);
    const uint16_t delayMs = cfg(MB_OFS_HR_TIMER_MS);
    if (fn == TIMER_OFF || delayMs == 0)
    {
        oneshotLatched = false;
        edgeMs = 0;
        return filteredOn;
    }

    const uint32_t nowMs = millis();
    if (fn == TIMER_ONESHOT)
    {
        if (filteredOn && !currentOn && !oneshotLatched)
        {
            oneshotLatched = true;
            edgeMs = nowMs;
            return true;
        }
        if (oneshotLatched)
        {
            if ((nowMs - edgeMs) >= delayMs)
            {
                oneshotLatched = false;
                edgeMs = 0;
                return false;
            }
            return true;
        }
        if (!filteredOn)
        {
            oneshotLatched = false;
        }
        return false;
    }

    if (filteredOn == currentOn)
    {
        edgeMs = 0;
        return currentOn;
    }

    const bool rising = filteredOn && !currentOn;
    const bool delayThisEdge =
        (fn == TIMER_ON_OFF_DELAY) ||
        (rising && fn == TIMER_ON_DELAY) ||
        (!rising && fn == TIMER_OFF_DELAY);

    if (!delayThisEdge)
    {
        edgeMs = 0;
        return filteredOn;
    }
    if (edgeMs == 0)
    {
        edgeMs = nowMs;
    }
    if ((nowMs - edgeMs) >= delayMs)
    {
        edgeMs = 0;
        return filteredOn;
    }
    return currentOn;
}

void SickDT35::applyTeach(uint16_t teach)
{
    const uint16_t live = m_distance.getValue();
    const uint16_t adc = m_rawAdc.getValue();

    auto setTarget = [&](uint16_t offset, uint16_t value) {
        const RegDesc *desc = findReg(offset);
        setCfg(offset, clampReg(desc, value));
        m_lastTeachTarget = offset;
    };

    switch (teach)
    {
    case TEACH_Q1_DTO:
        setCfg(MB_OFS_HR_Q1_FUNCTION, SW_DTO);
        setTarget(MB_OFS_HR_Q1_NEAR, live);
        break;
    case TEACH_Q2_DTO:
        setCfg(MB_OFS_HR_Q2_FUNCTION, SW_DTO);
        setCfg(MB_OFS_HR_Q2_OUTPUT, Q2_SWITCHING);
        setTarget(MB_OFS_HR_Q2_NEAR, live);
        break;
    case TEACH_Q1_NEAR:
        setTarget(MB_OFS_HR_Q1_NEAR, live);
        break;
    case TEACH_Q1_FAR:
        setTarget(MB_OFS_HR_Q1_FAR, live);
        break;
    case TEACH_Q1_CENTER:
        setTarget(MB_OFS_HR_Q1_CENTER, live);
        break;
    case TEACH_Q2_NEAR:
        setTarget(MB_OFS_HR_Q2_NEAR, live);
        break;
    case TEACH_Q2_FAR:
        setTarget(MB_OFS_HR_Q2_FAR, live);
        break;
    case TEACH_Q2_CENTER:
        setTarget(MB_OFS_HR_Q2_CENTER, live);
        break;
    case TEACH_Q1_OBSB:
        setCfg(MB_OFS_HR_Q1_FUNCTION, SW_OBSB);
        setTarget(MB_OFS_HR_Q1_FAR, live);
        break;
    case TEACH_Q2_OBSB:
        setCfg(MB_OFS_HR_Q2_FUNCTION, SW_OBSB);
        setCfg(MB_OFS_HR_Q2_OUTPUT, Q2_SWITCHING);
        setTarget(MB_OFS_HR_Q2_FAR, live);
        break;
    case TEACH_Q2_4MA:
    case TEACH_Q2_0V:
        setTarget(MB_OFS_HR_ANALOG_NEAR, live);
        setCfg(MB_OFS_HR_ADC_NEAR, adc);
        setCfg(MB_OFS_HR_Q2_OUTPUT, (teach == TEACH_Q2_0V) ? Q2_VOLTAGE_0_10 : Q2_CURRENT_4_20);
        break;
    case TEACH_Q2_20MA:
    case TEACH_Q2_10V:
        setTarget(MB_OFS_HR_ANALOG_FAR, live);
        setCfg(MB_OFS_HR_ADC_FAR, adc);
        setCfg(MB_OFS_HR_Q2_OUTPUT, (teach == TEACH_Q2_10V) ? Q2_VOLTAGE_0_10 : Q2_CURRENT_4_20);
        break;
    case TEACH_Q2_ANALOG_CENTER:
        setTarget(MB_OFS_HR_ANALOG_CENTER, live);
        break;
    case TEACH_FINE_PLUS_10:
    case TEACH_FINE_MINUS_10:
    {
        const int32_t delta = (teach == TEACH_FINE_PLUS_10) ? 10 : -10;
        const RegDesc *desc = findReg(m_lastTeachTarget);
        const uint16_t next = clampReg(desc, static_cast<int32_t>(cfg(m_lastTeachTarget)) + delta);
        setCfg(m_lastTeachTarget, next);
        break;
    }
    default:
        break;
    }

    if (cfg(MB_OFS_HR_MF_FUNCTION) == MF_TEACH)
    {
        pulseMfTeach();
    }
}

void SickDT35::pulseMfTeach()
{
    if (!pinUsed(mfPin))
    {
        return;
    }
    const bool highActive = (cfg(MB_OFS_HR_MF_LEVEL) & 0x01) != 0;
    digitalWrite(mfPin, highActive ? HIGH : LOW);
    m_mfPulseUntil = millis() + SICK_DT35_MF_TEACH_PULSE_MS;
}

void SickDT35::applyMf()
{
    if (!pinUsed(mfPin))
    {
        return;
    }

    const uint32_t nowMs = millis();
    if (m_mfPulseUntil != 0)
    {
        if (static_cast<int32_t>(nowMs - m_mfPulseUntil) < 0)
        {
            return;
        }
        m_mfPulseUntil = 0;
    }

    const uint16_t fn = cfg(MB_OFS_HR_MF_FUNCTION);
    const bool highActive = (cfg(MB_OFS_HR_MF_LEVEL) & 0x01) != 0;
    bool assertMf = false;
    if (fn == MF_LASER)
    {
        assertMf = (cfg(MB_OFS_HR_LASER) == 0);
    }
    const bool level = assertMf ? highActive : !highActive;
    digitalWrite(mfPin, level ? HIGH : LOW);
}

short SickDT35::loop()
{
    Component::loop();
    if (!enabled())
    {
        return E_OK;
    }

    applyMf();

    const uint32_t nowMs = millis();
    if ((nowMs - m_lastSample) < sampleIntervalMs())
    {
        return E_OK;
    }
    m_lastSample = nowMs;

    uint16_t raw = m_filtAdc;
    if (pinUsed(analogPin))
    {
        raw = static_cast<uint16_t>(analogRead(analogPin));
    }

    uint16_t avgDepth = cfg(MB_OFS_HR_AVERAGING);
    if (cfg(MB_OFS_HR_SPEED) != SPEED_EXPERT)
    {
        avgDepth = 1;
    }
    if (avgDepth < 1)
    {
        avgDepth = 1;
    }
    if (avgDepth > SICK_DT35_AVG_MAX)
    {
        avgDepth = SICK_DT35_AVG_MAX;
    }

    if (avgDepth <= 1)
    {
        m_filtAdc = raw;
        m_avgSum = 0;
        m_avgIdx = 0;
        m_avgFill = 0;
    }
    else
    {
        if (m_avgFill == avgDepth)
        {
            m_avgSum -= m_avgRing[m_avgIdx];
        }
        else
        {
            ++m_avgFill;
        }
        m_avgRing[m_avgIdx] = raw;
        m_avgSum += raw;
        m_avgIdx = static_cast<uint8_t>((m_avgIdx + 1) % avgDepth);
        m_filtAdc = static_cast<uint16_t>(m_avgSum / m_avgFill);
    }

    const uint16_t adcNear = cfg(MB_OFS_HR_ADC_NEAR);
    const uint16_t adcFar = cfg(MB_OFS_HR_ADC_FAR);
    const uint16_t adcLo = (adcNear < adcFar) ? adcNear : adcFar;
    const uint16_t adcHi = (adcNear < adcFar) ? adcFar : adcNear;
    bool valid = pinUsed(analogPin) && (m_filtAdc + 8 >= adcLo) && (m_filtAdc <= adcHi + 8);
    if (cfg(MB_OFS_HR_LASER) == 0)
    {
        valid = false;
    }

    uint16_t measured = adcToDistance(m_filtAdc);
    if (valid)
    {
        m_heldDistance = measured;
    }

    uint16_t published = publishDistance(measured, valid);
    if (m_mode.getValue() == E_AUX_REMOTE)
    {
        published = m_remoteDistance.getValue();
        valid = true;
    }

    const uint16_t inversion = cfg(MB_OFS_HR_INVERSION);
    const uint16_t bitDepth = (cfg(MB_OFS_HR_SPEED) == SPEED_EXPERT) ? cfg(MB_OFS_HR_BIT_FILTER) : 0;

    bool q1Desired = evaluateSwitch(published, valid,
                                    cfg(MB_OFS_HR_Q1_FUNCTION),
                                    cfg(MB_OFS_HR_Q1_NEAR),
                                    cfg(MB_OFS_HR_Q1_FAR),
                                    cfg(MB_OFS_HR_Q1_HYST_NEAR),
                                    cfg(MB_OFS_HR_Q1_HYST_FAR),
                                    (inversion & 0x01) != 0,
                                    m_q1On);
    q1Desired = applyBitFilter(q1Desired, m_q1Streak, m_q1On, bitDepth);
    m_q1On = applyTimer(q1Desired, m_q1On, m_q1EdgeMs, m_q1Oneshot);

    bool q2Desired = false;
    if (cfg(MB_OFS_HR_Q2_OUTPUT) == Q2_SWITCHING)
    {
        q2Desired = evaluateSwitch(published, valid,
                                   cfg(MB_OFS_HR_Q2_FUNCTION),
                                   cfg(MB_OFS_HR_Q2_NEAR),
                                   cfg(MB_OFS_HR_Q2_FAR),
                                   cfg(MB_OFS_HR_Q2_HYST_NEAR),
                                   cfg(MB_OFS_HR_Q2_HYST_FAR),
                                   (inversion & 0x02) != 0,
                                   m_q2On);
        q2Desired = applyBitFilter(q2Desired, m_q2Streak, m_q2On, bitDepth);
        m_q2On = applyTimer(q2Desired, m_q2On, m_q2EdgeMs, m_q2Oneshot);
    }
    else
    {
        m_q2On = false;
        m_q2Streak = 0;
    }

    bool q1Hw = m_q1On;
    if (pinUsed(q1Pin))
    {
        q1Hw = digitalRead(q1Pin) == HIGH;
    }

    uint16_t status = 0;
    if (valid)
    {
        status |= ST_VALID;
    }
    if (cfg(MB_OFS_HR_LASER) != 0)
    {
        status |= ST_LASER;
    }
    if (q1Hw)
    {
        status |= ST_Q1_HW;
    }
    if (m_q1On)
    {
        status |= ST_Q1;
    }
    if (m_q2On)
    {
        status |= ST_Q2;
    }
    if (!valid)
    {
        status |= ST_ALARM;
    }

    bool changed = false;
    changed |= m_rawAdc.update(m_filtAdc);
    changed |= m_distance.update(published);
    changed |= m_valid.update(valid ? 1 : 0);
    changed |= m_q1.update(m_q1On ? 1 : 0);
    changed |= m_q2.update(m_q2On ? 1 : 0);
    changed |= m_status.update(status);
    if (changed)
    {
        notifyStateChange();
    }
    return E_OK;
}

short SickDT35::info(short, short)
{
    Log.verboseln("SickDT35::info ID:%d Dist:%d ADC:%d Q1:%d Q2:%d Valid:%d Mode:%d Speed:%d Addr:%d",
                  id,
                  m_distance.getValue(),
                  m_rawAdc.getValue(),
                  m_q1.getValue(),
                  m_q2.getValue(),
                  m_valid.getValue(),
                  m_mode.getValue(),
                  cfg(MB_OFS_HR_SPEED),
                  mb_tcp_base_address());
    return E_OK;
}

short SickDT35::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    short result = NetworkComponent::mb_tcp_write(reg, networkValue);
    if (result != E_NOT_IMPLEMENTED)
    {
        return result;
    }

    const uint16_t offset = static_cast<uint16_t>(reg->startAddress - mb_tcp_base_address());
    const RegDesc *desc = findReg(offset);
    if (!desc)
    {
        return E_INVALID_PARAMETER;
    }
    if (!desc->writable)
    {
        return MODBUS_ERROR_ILLEGAL_FUNCTION;
    }

    const uint16_t clamped = clampReg(desc, networkValue);

    if (offset == MB_OFS_HR_MODE)
    {
        m_mode.update(clamped);
        setCfg(offset, clamped);
        return E_OK;
    }
    if (offset == MB_OFS_HR_REMOTE_DISTANCE)
    {
        m_remoteDistance.update(clamped);
        setCfg(offset, clamped);
        return E_OK;
    }
    if (offset == MB_OFS_HR_TEACH)
    {
        setCfg(offset, clamped);
        applyTeach(clamped);
        return E_OK;
    }
    if (offset == MB_OFS_HR_COMMAND)
    {
        setCfg(offset, clamped);
        if (clamped == CMD_HOME)
        {
            cmd_home();
        }
        else if (clamped == CMD_FACTORY_RESET)
        {
            applyFactoryDefaults();
        }
        setCfg(MB_OFS_HR_COMMAND, 0);
        return E_OK;
    }
    if (offset == MB_OFS_HR_AVERAGING)
    {
        uint16_t depth = clamped;
        if (depth != 1 && depth != 2 && depth != 4 && depth != 8 && depth != 16)
        {
            depth = 1;
        }
        setCfg(offset, depth);
        m_avgSum = 0;
        m_avgIdx = 0;
        m_avgFill = 0;
        return E_OK;
    }
    if (offset == MB_OFS_HR_BIT_FILTER)
    {
        uint16_t depth = clamped;
        if (depth != 0 && depth != 2 && depth != 4 && depth != 8 && depth != 16)
        {
            depth = 0;
        }
        setCfg(offset, depth);
        m_q1Streak = 0;
        m_q2Streak = 0;
        return E_OK;
    }
    if (offset == MB_OFS_HR_LASER || offset == MB_OFS_HR_MF_FUNCTION || offset == MB_OFS_HR_MF_LEVEL)
    {
        setCfg(offset, clamped);
        applyMf();
        return E_OK;
    }

    setCfg(offset, clamped);
    return E_OK;
}

short SickDT35::mb_tcp_read(MB_Registers *reg)
{
    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED)
    {
        return result;
    }

    const uint16_t offset = static_cast<uint16_t>(reg->startAddress - mb_tcp_base_address());
    switch (offset)
    {
    case MB_OFS_HR_DISTANCE:
        return static_cast<short>(m_distance.getValue());
    case MB_OFS_HR_RAW_ADC:
        return static_cast<short>(m_rawAdc.getValue());
    case MB_OFS_HR_Q1:
        return static_cast<short>(m_q1.getValue());
    case MB_OFS_HR_Q2:
        return static_cast<short>(m_q2.getValue());
    case MB_OFS_HR_VALID:
        return static_cast<short>(m_valid.getValue());
    case MB_OFS_HR_STATUS:
        return static_cast<short>(m_status.getValue());
    case MB_OFS_HR_MODE:
        return static_cast<short>(m_mode.getValue());
    case MB_OFS_HR_REMOTE_DISTANCE:
        return static_cast<short>(m_remoteDistance.getValue());
    default:
        if (offset <= MB_OFS_HR_LAST)
        {
            return static_cast<short>(cfg(offset));
        }
        return 0;
    }
}

short SickDT35::cmd_home(short, short)
{
    const uint16_t measured = adcToDistance(m_filtAdc);
    setCfg(MB_OFS_HR_PD_NORM, measured);
    Log.verboseln("SickDT35::cmd_home ID:%d home=%d mm ADC:%d", id, measured, m_filtAdc);
    notifyStateChange();
    return E_OK;
}

short SickDT35::serial_register(Bridge *bridge)
{
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&SickDT35::info);
    bridge->registerMemberFunction(id, this, C_STR("home"), (ComponentFnPtr)&SickDT35::cmd_home);
    return E_OK;
}

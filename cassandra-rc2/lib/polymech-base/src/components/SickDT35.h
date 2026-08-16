#ifndef SICK_DT35_H
#define SICK_DT35_H

#include <Arduino.h>
#include <ArduinoLog.h>
#include <App.h>
#include <enums.h>
#include "config.h"
#include "config-modbus.h"
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"

/*
 * SICK DT35-B15551 (Dx35) mid-range time-of-flight distance sensor
 *
 * Datasheet : https://www.sick.com/media/pdf/2/62/362/dataSheet_DT35-B15551_1057651_en.pdf
 * Manual    : https://www.sick.com/media/docs/3/43/743/Operating_instructions_Dx35_Distance_Sensors_en_IM0052743.PDF
 *
 * This component does not speak IO-Link. It reads the analog Q2 output (and
 * optionally Q1 / drives MF) and mirrors the sensor's IO-Link parameter set
 * as Modbus holding registers so a PLC can configure every mode.
 *
 * M12 5-pin:
 *   1 brown  L+  12–30 V
 *   2 white  QA/Q2  analog 4–20 mA / 0–10 V, or switching
 *   3 blue   M   0 V
 *   4 black  Q1/C  push-pull / IO-Link
 *   5 gray   MF    multifunctional input
 *
 * Analog path: Q2 → shunt or divider → MCU ADC. Map ADC_NEAR/ADC_FAR to
 * ANALOG_NEAR/ANALOG_FAR (factory 50 mm @ 4 mA/0 V, 10 000 mm @ 20 mA/10 V).
 *
 * Switching modes (Q1 / Q2, IO-Link 69 / 74):
 *   DtO    Distance to Object — ON when target is closer than the near point
 *   ObSB   Object between Sensor and Background — ON while distance < background
 *   Window ON while near < distance < far
 *   VMA    Signal-level warning (no photodiode here; follows VALID)
 *   Alarm  Fault output — ON when measurement is invalid
 *
 * Speed (IO-Link 103) sets the sample interval (DT35-B15551 response times):
 *   Super-fast 4.5 ms, Fast 12.5 ms, Medium 24.5 ms, Slow 48.5 ms, Super-slow 192.5 ms
 *
 * Control mode (same idea as POT):
 *   LOCAL  — distance from ADC
 *   REMOTE — distance from REMOTE_DISTANCE (switching still evaluated)
 */

#ifndef SICK_DT35_PIN_UNUSED
#define SICK_DT35_PIN_UNUSED 0xFF
#endif

#ifndef SICK_DT35_ADC_MAX
#define SICK_DT35_ADC_MAX 4095
#endif

#ifndef SICK_DT35_RANGE_MIN_MM
#define SICK_DT35_RANGE_MIN_MM 50
#endif

#ifndef SICK_DT35_RANGE_MAX_MM
#define SICK_DT35_RANGE_MAX_MM 12000
#endif

#ifndef SICK_DT35_AVG_MAX
#define SICK_DT35_AVG_MAX 16
#endif

#ifndef SICK_DT35_MF_TEACH_PULSE_MS
#define SICK_DT35_MF_TEACH_PULSE_MS 60
#endif

#define SICK_DT35_MB_COUNT 46

class Bridge;

class SickDT35 : public NetworkComponent<SICK_DT35_MB_COUNT>
{
public:
    enum E_ControlMode : uint16_t
    {
        E_AUX_LOCAL = 0,
        E_AUX_REMOTE = 1
    };

    // IO-Link 69 / 74
    enum E_SwitchFn : uint16_t
    {
        SW_DTO = 0,
        SW_OBSB = 1,
        SW_WINDOW = 2,
        SW_VMA = 3,
        SW_ALARM = 4
    };

    // IO-Link 92
    enum E_Q2Output : uint16_t
    {
        Q2_CURRENT_4_20 = 0,
        Q2_VOLTAGE_0_10 = 1,
        Q2_SWITCHING = 2
    };

    // IO-Link 103 — factory Slow
    enum E_Speed : uint16_t
    {
        SPEED_EXPERT = 0,
        SPEED_SUPER_SLOW = 1,
        SPEED_SLOW = 2,
        SPEED_MEDIUM = 3,
        SPEED_FAST = 4,
        SPEED_SUPER_FAST = 5
    };

    // IO-Link 97
    enum E_TimerFn : uint16_t
    {
        TIMER_OFF = 0,
        TIMER_ON_DELAY = 1,
        TIMER_OFF_DELAY = 2,
        TIMER_ON_OFF_DELAY = 3,
        TIMER_ONESHOT = 4
    };

    // IO-Link 81
    enum E_MfFn : uint16_t
    {
        MF_TEACH = 0,
        MF_LASER = 1,
        MF_OFF = 2
    };

    // IO-Link 104
    enum E_AlarmFn : uint16_t
    {
        ALARM_CLAMP = 0,
        ALARM_HOLD = 1
    };

    // IO-Link 130
    enum E_Teach : uint16_t
    {
        TEACH_Q1_DTO = 0,
        TEACH_Q2_DTO = 1,
        TEACH_Q1_NEAR = 2,
        TEACH_Q1_FAR = 3,
        TEACH_Q1_CENTER = 4,
        TEACH_Q2_NEAR = 5,
        TEACH_Q2_FAR = 6,
        TEACH_Q2_CENTER = 7,
        TEACH_Q1_OBSB = 8,
        TEACH_Q2_OBSB = 9,
        TEACH_Q2_4MA = 10,
        TEACH_Q2_20MA = 11,
        TEACH_Q2_0V = 12,
        TEACH_Q2_10V = 13,
        TEACH_Q2_ANALOG_CENTER = 14,
        TEACH_FINE_PLUS_10 = 15,
        TEACH_FINE_MINUS_10 = 16
    };

    // IO-Link 2
    enum E_Command : uint16_t
    {
        CMD_NONE = 0,
        CMD_HOME = 1,
        CMD_FACTORY_RESET = 130
    };

    enum E_StatusBit : uint16_t
    {
        ST_VALID = 1 << 0,
        ST_LASER = 1 << 1,
        ST_Q1_HW = 1 << 2,
        ST_Q1 = 1 << 3,
        ST_Q2 = 1 << 4,
        ST_ALARM = 1 << 5
    };

    enum E_MB_Offset : uint16_t
    {
        MB_OFS_HR_DISTANCE = E_NVC_USER + 0,
        MB_OFS_HR_RAW_ADC = E_NVC_USER + 1,
        MB_OFS_HR_Q1 = E_NVC_USER + 2,
        MB_OFS_HR_Q2 = E_NVC_USER + 3,
        MB_OFS_HR_VALID = E_NVC_USER + 4,
        MB_OFS_HR_STATUS = E_NVC_USER + 5,
        MB_OFS_HR_MODE = E_NVC_USER + 6,
        MB_OFS_HR_REMOTE_DISTANCE = E_NVC_USER + 7,
        MB_OFS_HR_ADC_NEAR = E_NVC_USER + 8,
        MB_OFS_HR_ADC_FAR = E_NVC_USER + 9,
        MB_OFS_HR_Q1_FUNCTION = E_NVC_USER + 10,
        MB_OFS_HR_Q1_NEAR = E_NVC_USER + 11,
        MB_OFS_HR_Q1_FAR = E_NVC_USER + 12,
        MB_OFS_HR_Q1_HYST_NEAR = E_NVC_USER + 13,
        MB_OFS_HR_Q1_HYST_FAR = E_NVC_USER + 14,
        MB_OFS_HR_Q1_CENTER = E_NVC_USER + 15,
        MB_OFS_HR_Q2_OUTPUT = E_NVC_USER + 16,
        MB_OFS_HR_Q2_FUNCTION = E_NVC_USER + 17,
        MB_OFS_HR_Q2_NEAR = E_NVC_USER + 18,
        MB_OFS_HR_Q2_FAR = E_NVC_USER + 19,
        MB_OFS_HR_Q2_HYST_NEAR = E_NVC_USER + 20,
        MB_OFS_HR_Q2_HYST_FAR = E_NVC_USER + 21,
        MB_OFS_HR_Q2_CENTER = E_NVC_USER + 22,
        MB_OFS_HR_ANALOG_NEAR = E_NVC_USER + 23,
        MB_OFS_HR_ANALOG_FAR = E_NVC_USER + 24,
        MB_OFS_HR_ANALOG_CENTER = E_NVC_USER + 25,
        MB_OFS_HR_VMA_THRESHOLD = E_NVC_USER + 26,
        MB_OFS_HR_INVERSION = E_NVC_USER + 27,
        MB_OFS_HR_DISTANCE_OFFSET = E_NVC_USER + 28,
        MB_OFS_HR_TIMER_FN = E_NVC_USER + 29,
        MB_OFS_HR_TIMER_MS = E_NVC_USER + 30,
        MB_OFS_HR_SPEED = E_NVC_USER + 31,
        MB_OFS_HR_INTEGRATION = E_NVC_USER + 32,
        MB_OFS_HR_AVERAGING = E_NVC_USER + 33,
        MB_OFS_HR_BIT_FILTER = E_NVC_USER + 34,
        MB_OFS_HR_TEACH = E_NVC_USER + 35,
        MB_OFS_HR_PD_STRUCTURE = E_NVC_USER + 36,
        MB_OFS_HR_PD_RESOLUTION = E_NVC_USER + 37,
        MB_OFS_HR_PD_NORM = E_NVC_USER + 38,
        MB_OFS_HR_MF_FUNCTION = E_NVC_USER + 39,
        MB_OFS_HR_MF_LEVEL = E_NVC_USER + 40,
        MB_OFS_HR_ALARM_FN = E_NVC_USER + 41,
        MB_OFS_HR_PB_LOCK = E_NVC_USER + 42,
        MB_OFS_HR_LASER = E_NVC_USER + 43,
        MB_OFS_HR_COMMAND = E_NVC_USER + 44,
        MB_OFS_HR_LAST = MB_OFS_HR_COMMAND
    };

    const uint16_t analogPin;
    const uint16_t q1Pin;
    const uint16_t mfPin;

    NetworkValue<uint16_t> m_distance;
    NetworkValue<uint16_t> m_rawAdc;
    NetworkValue<uint16_t> m_q1;
    NetworkValue<uint16_t> m_q2;
    NetworkValue<uint16_t> m_valid;
    NetworkValue<uint16_t> m_status;
    NetworkValue<uint16_t> m_mode;
    NetworkValue<uint16_t> m_remoteDistance;

    SickDT35(Component *owner,
             uint16_t _analogPin,
             uint16_t _id,
             uint16_t _modbusAddress,
             uint16_t _q1Pin = SICK_DT35_PIN_UNUSED,
             uint16_t _mfPin = SICK_DT35_PIN_UNUSED);

    short setup() override;
    short loop() override;
    short debug() override { return info(); }
    short info(short val0 = 0, short val1 = 0) override;
    short cmd_home(short val0 = 0, short val1 = 0);

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;
    short serial_register(Bridge *bridge) override;

    uint16_t getDistance() const { return m_distance.getValue(); }
    uint16_t getQ1() const { return m_q1.getValue(); }
    uint16_t getQ2() const { return m_q2.getValue(); }
    uint16_t getRawAdc() const { return m_rawAdc.getValue(); }
    bool isValid() const { return m_valid.getValue() != 0; }

    void applyFactoryDefaults();

private:
    struct RegDesc
    {
        uint16_t offset;
        const char *name;
        bool writable;
        uint16_t def;
        uint16_t minv;
        uint16_t maxv;
    };

    static const RegDesc *regTable(size_t &count);
    static const RegDesc *findReg(uint16_t offset);

    uint16_t cfg(uint16_t offset) const;
    void setCfg(uint16_t offset, uint16_t value);
    uint16_t clampReg(const RegDesc *desc, int32_t value) const;

    uint16_t sampleIntervalMs() const;
    uint16_t adcToDistance(uint16_t adc) const;
    uint16_t publishDistance(uint16_t measuredMm, bool valid) const;
    bool evaluateSwitch(uint16_t distanceMm,
                        bool valid,
                        uint16_t fn,
                        uint16_t nearMm,
                        uint16_t farMm,
                        uint16_t hystNear,
                        uint16_t hystFar,
                        bool inverted,
                        bool currentOn) const;
    bool applyBitFilter(bool desired, uint8_t &streak, bool currentOn, uint16_t depth) const;
    bool applyTimer(bool filteredOn, bool currentOn, uint32_t &edgeMs, bool &oneshotLatched) const;
    void applyTeach(uint16_t teach);
    void applyMf();
    void pulseMfTeach();
    bool pinUsed(uint16_t pin) const { return pin != SICK_DT35_PIN_UNUSED; }

    uint16_t m_cfg[MB_OFS_HR_LAST + 1];
    uint16_t m_lastTeachTarget;

    uint16_t m_avgRing[SICK_DT35_AVG_MAX];
    uint32_t m_avgSum;
    uint8_t m_avgIdx;
    uint8_t m_avgFill;
    uint16_t m_filtAdc;
    uint16_t m_heldDistance;

    uint8_t m_q1Streak;
    uint8_t m_q2Streak;
    uint32_t m_q1EdgeMs;
    uint32_t m_q2EdgeMs;
    bool m_q1Oneshot;
    bool m_q2Oneshot;
    bool m_q1On;
    bool m_q2On;

    uint32_t m_lastSample;
    uint32_t m_mfPulseUntil;
};

#endif

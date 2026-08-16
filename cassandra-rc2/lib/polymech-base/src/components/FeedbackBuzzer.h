#ifndef FEEDBACK_BUZZER_H
#define FEEDBACK_BUZZER_H

#include <ArduinoLog.h>
#include <App.h>
#include <Component.h>
#include <enums.h>
#include "config.h"
#include <modbus/ModbusTCP.h>
#include "config-modbus.h"
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"

#define FEEDBACK_BUZZER_MAX_PLAY_TIME_MS 2500

// Local offsets, starting from the USER position defined in the base class
#define BUZZER_MB_STATE_OFFSET (E_NVC_USER + 0)
#define BUZZER_MB_MODE_OFFSET  (E_NVC_USER + 1)
#define BUZZER_MB_COUNT 3 // m_enabled, m_state, m_mode

struct BuzzerPattern
{
    ushort on_ms;
    ushort off_ms;
};

class FeedbackBuzzer : public NetworkComponent<BUZZER_MB_COUNT>
{
public:
    enum E_BuzzerMode
    {
        MODE_OFF,
        MODE_SOLID,
        MODE_SLOW_BLINK,         // 1s on, 1s off
        MODE_FAST_BLINK,         // 250ms on, 250ms off
        MODE_LONG_BEEP_SHORT_PAUSE // 1s on, 250ms off
    };

private:
    const short pin;

    unsigned long lastToggleTime;
    uint8_t pattern_step;
    uint32_t _start_time_ms;
    uint32_t _max_play_time_ms;

    NetworkValue<bool> m_state;
    NetworkValue<ushort> m_mode;

    const BuzzerPattern patterns[3] = {
        {1000, 1000}, // SLOW_BLINK
        {250, 250},   // FAST_BLINK
        {1000, 250},  // LONG_BEEP_SHORT_PAUSE
    };

    void updatePinState() {
        digitalWrite(this->pin, m_state.getValue() ? HIGH : LOW);
    }

public:
    FeedbackBuzzer(Component *owner, short _pin, short _id, short _modbusAddress)
        : NetworkComponent(_modbusAddress, "FeedbackBuzzer", _id, Component::COMPONENT_DEFAULT, owner),
          pin(_pin),
          m_state(this, _id, "BuzzerState"),
          m_mode(this, _id, "BuzzerMode")
    {
        pFlags = E_PersistenceFlags::E_PF_ENABLED;
        lastToggleTime = 0;
        pattern_step = 0;
        _start_time_ms = 0;
        _max_play_time_ms = 0;
    }

    void on()
    {
        setMode(MODE_SOLID);
    }

    void off()
    {
        setMode(MODE_OFF);
    }

    void setMode(E_BuzzerMode newMode, uint32_t max_play_time_ms = FEEDBACK_BUZZER_MAX_PLAY_TIME_MS)
    {
        if (!enabled()) {
            return;
        }
        // If a timed sound is currently playing, do not allow it to be overridden
        // by a new sound. The only exception is explicitly turning it OFF.
        if (millis() - _start_time_ms < _max_play_time_ms) {
            if (newMode != MODE_OFF) {
                return;
            }
        }

        if (m_mode.update(newMode))
        {
            lastToggleTime = millis();
            _start_time_ms = millis();
            pattern_step = 0;

            if (newMode == MODE_OFF)
            {
                _max_play_time_ms = 0;
                if (m_state.update(false))
                {
                    updatePinState();
                }
            }
            else
            {
                if (max_play_time_ms == 0 || max_play_time_ms > FEEDBACK_BUZZER_MAX_PLAY_TIME_MS)
                {
                    _max_play_time_ms = FEEDBACK_BUZZER_MAX_PLAY_TIME_MS;
                }
                else
                {
                    _max_play_time_ms = max_play_time_ms;
                }
                if (m_state.update(true))
                {
                    updatePinState();
                }
            }
        }
    }

    short setup() override
    {
        NetworkComponent::setup();
        pinMode(pin, OUTPUT);
        updatePinState();

        const uint16_t baseAddr = mb_tcp_base_address();

        m_state.initNotify(false, true, NetworkValue_ThresholdMode::DIFFERENCE);
        m_state.initModbus(baseAddr + BUZZER_MB_STATE_OFFSET, 1, this->id, this->slaveId, FN_WRITE_COIL, "Buzzer State", this->name.c_str());
        registerBlock(m_state.getRegisterInfo());

        m_mode.initNotify((ushort)MODE_OFF, 1, NetworkValue_ThresholdMode::DIFFERENCE);
        m_mode.initModbus(baseAddr + BUZZER_MB_MODE_OFFSET, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "Buzzer Mode", this->name.c_str());
        registerBlock(m_mode.getRegisterInfo());

        return E_OK;
    }

    short loop() override
    {
        if (!enabled()) {
            if ((E_BuzzerMode)m_mode.getValue() != MODE_OFF) {
                off(); // Ensure it's off if disabled
            }
            return E_OK;
        }
        Component::loop();
        E_BuzzerMode currentMode = (E_BuzzerMode)m_mode.getValue();

        // If the timed duration has elapsed, turn the buzzer off.
        if (_max_play_time_ms > 0 && millis() - _start_time_ms >= _max_play_time_ms)
        {
            off();
            return E_OK;
        }

        if (currentMode > MODE_SOLID)
        {
            const BuzzerPattern &pattern = patterns[currentMode - MODE_SLOW_BLINK];
            unsigned long currentMillis = millis();
            unsigned long duration = (pattern_step % 2 == 0) ? pattern.on_ms : pattern.off_ms;

            if (currentMillis - lastToggleTime >= duration)
            {
                lastToggleTime = currentMillis;
                if(m_state.update(!m_state.getValue())) {
                    updatePinState();
                }
                pattern_step++;
            }
        }
        return E_OK;
    }

    // --- Pure Virtual Function Implementations ---
    short mb_tcp_write(MB_Registers *reg, short value) override
    {
        short result = NetworkComponent::mb_tcp_write(reg, value);
        if (result != E_NOT_IMPLEMENTED)
        {
            return result;
        }

        uint16_t address = reg->startAddress;
        short offset = address - _baseAddress;

        if (offset == BUZZER_MB_STATE_OFFSET)
        { // Coil write
            setMode(value > 0 ? MODE_SOLID : MODE_OFF);
            return E_OK;
        }
        else if (offset == BUZZER_MB_MODE_OFFSET)
        { // Mode write
            if (value >= 0 && value <= MODE_LONG_BEEP_SHORT_PAUSE)
            {
                setMode((E_BuzzerMode)value);
                return E_OK;
            }
            return E_INVALID_PARAMETER;
        }
        return E_INVALID_PARAMETER;
    }

    short mb_tcp_read(MB_Registers *reg) override
    {
        short result = NetworkComponent::mb_tcp_read(reg);
        if (result != E_NOT_IMPLEMENTED)
        {
            return result;
        }
        uint16_t address = reg->startAddress;
        short offset = address - _baseAddress;
        if (offset == BUZZER_MB_STATE_OFFSET)
        {
            return m_state.getValue() ? 1 : 0;
        }
        else if (offset == BUZZER_MB_MODE_OFFSET)
        {
            return m_mode.getValue();
        }
        return 0;
    }
};

#endif // FEEDBACK_BUZZER_H 
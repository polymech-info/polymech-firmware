#ifndef FEEDBACK_3C_H
#define FEEDBACK_3C_H

// Relay output mode
#define RELAY_MODE_NO 0 // Normally Open: HIGH = ON
#define RELAY_MODE_NC 1 // Normally Closed: LOW = ON

// Select the mode for the Feedback3C component
#define FEEDBACK_3C_RELAY_MODE RELAY_MODE_NO

#include <ArduinoLog.h>
#include <App.h>
#include <enums.h>
#include "config.h"
#include "config-modbus.h"
#include "modbus/NetworkComponent.h"
#include "NetworkValue.h"
#include <array>

#define FEEDBACK3C_MB_COUNT 9

class Bridge;

class Feedback3C : public NetworkComponent<FEEDBACK3C_MB_COUNT>
{
public:
    static const short MAX_LEDS = 3;

    enum E_MB_Offset
    {
        MB_OFS_COIL_LED_0 = E_NVC_USER + 0,
        MB_OFS_COIL_LED_1 = E_NVC_USER + 1,
        MB_OFS_COIL_LED_2 = E_NVC_USER + 2,
        MB_OFS_HR_FREQ_LED_0 = E_NVC_USER + 3,
        MB_OFS_HR_FREQ_LED_1 = E_NVC_USER + 4,
        MB_OFS_HR_FREQ_LED_2 = E_NVC_USER + 5,
        MB_OFS_HR_CMD = E_NVC_USER + 6,
        MB_OFS_HR_MODE = E_NVC_USER + 7,
    };

    enum Mode
    {
        MODE_MANUAL = 0,
        MODE_STARTUP = 1,
        MODE_STANDBY = 2,
        MODE_RUNNING = 3,
        MODE_WAITING = 4,
        MODE_FINISHED = 5,
        MODE_WARNING = 6,
        MODE_WARNING_HOLD = 7,
        MODE_HEATING = 8,
        MODE_ERROR = 9,
        MODE_FATAL = 10,
    };

private:
    Mode _currentMode;
    enum LedState
    {
        LED_OFF,
        LED_ON,
        LED_BLINK
    };

    struct ModeConfig
    {
        LedState led_states[MAX_LEDS];
    };

    static const ModeConfig &getModeConfig(Mode mode)
    {
        static const ModeConfig configs[] = {
            /* MODE_MANUAL */ {{LedState::LED_OFF, LedState::LED_OFF, LedState::LED_OFF}},
            /* MODE_STARTUP */ {{LedState::LED_ON, LedState::LED_ON, LedState::LED_ON}},
            /* MODE_STANDBY */ {{LedState::LED_ON, LedState::LED_ON, LedState::LED_OFF}},
            /* MODE_RUNNING */ {{LedState::LED_OFF, LedState::LED_ON, LedState::LED_ON}},
            /* MODE_WAITING */ {{LedState::LED_OFF, LedState::LED_BLINK, LedState::LED_BLINK}},
            /* MODE_FINISHED*/ {{LedState::LED_ON, LedState::LED_ON, LedState::LED_OFF}},
            /* MODE_WARNING */ {{LedState::LED_OFF, LedState::LED_BLINK, LedState::LED_OFF}},
            /* MODE_WARNING_HOLD */ {{LedState::LED_OFF, LedState::LED_ON, LedState::LED_OFF}},
            /* MODE_HEATING */ {{LedState::LED_OFF, LedState::LED_OFF, LedState::LED_ON}},
            /* MODE_ERROR */ {{LedState::LED_OFF, LedState::LED_OFF, LedState::LED_BLINK}},
            /* MODE_FATAL */ {{LedState::LED_BLINK, LedState::LED_BLINK, LedState::LED_BLINK}},
        };
        if (mode < MODE_MANUAL || mode > MODE_ERROR)
            return configs[MODE_MANUAL];
        return configs[mode];
    }

    void ensure_off_when_disabled()
    {
        if (_currentMode != MODE_MANUAL || m_leds[0]->getValue() || m_leds[1]->getValue() || m_leds[2]->getValue())
        {
            for (int i = 0; i < MAX_LEDS; ++i)
            {
                if (m_leds[i]->update(false))
                    updatePinState(i);
                m_ledFrequencies[i]->update(0);
            }
            m_mode.update(MODE_MANUAL);
            _currentMode = MODE_MANUAL;
        }
    }

    uint8_t getPinState(bool led_on)
    {
#if (FEEDBACK_3C_RELAY_MODE == RELAY_MODE_NO)
        return led_on ? HIGH : LOW;
#else // RELAY_MODE_NC
        return led_on ? LOW : HIGH;
#endif
    }

    void updatePinState(int index)
    {
        digitalWrite(pins[index], getPinState(m_leds[index]->getValue()));
    }

    // for blinking
    unsigned long lastToggleTime[MAX_LEDS];

    NetworkValue<bool> m_led0, m_led1, m_led2;
    NetworkValue<ushort> m_ledFreq0, m_ledFreq1, m_ledFreq2;
    std::array<NetworkValue<bool> *, MAX_LEDS> m_leds;
    std::array<NetworkValue<ushort> *, MAX_LEDS> m_ledFrequencies;
    NetworkValue<Mode> m_mode;
    NetworkValue<ushort> m_command;

public:
    void setMode(Mode newMode)
    {
        if (!enabled())
        {
            ensure_off_when_disabled();
            return;
        }
        if (newMode == _currentMode)
        {
            return;
        }

        _currentMode = newMode;

        m_mode.applyUpdate(newMode);
        if (newMode < MODE_MANUAL || newMode > MODE_ERROR)
        {
            return;
        }

        // 1. Get the configuration for the requested mode.
        const ModeConfig &config = getModeConfig(newMode);

        // Reset all LEDs and frequencies first to ensure a clean state.
        for (int i = 0; i < MAX_LEDS; ++i)
        {
            m_ledFrequencies[i]->applyUpdate(0);
        }

        // 2. Iterate through each LED and apply the state from the fetched ModeConfig.
        for (int i = 0; i < MAX_LEDS; ++i)
        {
            switch (config.led_states[i])
            {
            case LED_OFF:
                if (m_leds[i]->update(false))
                    updatePinState(i);
                break;
            case LED_ON:
                if (m_leds[i]->update(true))
                    updatePinState(i);
                break;
            case LED_BLINK:
                // For blinking, set the initial state and frequency.
                // The loop() function will handle the actual toggling.
                if (m_leds[i]->update(true))
                    updatePinState(i);               // Start with LED on for immediate feedback
                m_ledFrequencies[i]->applyUpdate(1); // Default 1s blink rate
                lastToggleTime[i] = millis();
                break;
            }
        }
    }

    const short pins[MAX_LEDS];

    enum Command
    {
        ALL_OFF = 0,
        ALL_ON = 1,
        ALL_BLINK = 2,
    };

    Feedback3C(Component *owner, short pin0, short pin1, short pin2, short _id, short _modbusAddress)
        : NetworkComponent(_modbusAddress, "Feedback3C", _id, Component::COMPONENT_DEFAULT, owner),
          pins{pin0, pin1, pin2},
          m_led0(this, _id, "LED 0"), m_led1(this, _id, "LED 1"), m_led2(this, _id, "LED 2"),
          m_ledFreq0(this, _id, "LED Freq 0"), m_ledFreq1(this, _id, "LED Freq 1"), m_ledFreq2(this, _id, "LED Freq 2"),
          m_leds{&m_led0, &m_led1, &m_led2},
          m_ledFrequencies{&m_ledFreq0, &m_ledFreq1, &m_ledFreq2},
          m_mode(this, _id, "Mode"),
          m_command(this, _id, "Command")
    {
        pFlags = E_PersistenceFlags::E_PF_ENABLED;
        _currentMode = MODE_STARTUP;
        for (int i = 0; i < MAX_LEDS; ++i)
        {
            lastToggleTime[i] = 0;
        }
        setMode(MODE_STARTUP);
    }

    short setup() override
    {
        NetworkComponent::setup();
        for (int i = 0; i < MAX_LEDS; ++i)
        {
            pinMode(pins[i], OUTPUT);
            updatePinState(i);
        }

        const uint16_t baseAddr = mb_tcp_base_address();

        for (int i = 0; i < MAX_LEDS; ++i)
        {
            m_leds[i]->initNotify(false, true, NetworkValue_ThresholdMode::DIFFERENCE);
            m_leds[i]->initModbus(baseAddr + MB_OFS_COIL_LED_0 + i, 1, this->id, this->slaveId, FN_WRITE_COIL, m_leds[i]->name.c_str(), this->name.c_str());
            registerBlock(m_leds[i]->getRegisterInfo());

            m_ledFrequencies[i]->initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
            m_ledFrequencies[i]->initModbus(baseAddr + MB_OFS_HR_FREQ_LED_0 + i, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, m_ledFrequencies[i]->name.c_str(), this->name.c_str());
            registerBlock(m_ledFrequencies[i]->getRegisterInfo());
        }

        m_mode.initNotify(MODE_STARTUP, (Mode)1, NetworkValue_ThresholdMode::DIFFERENCE);
        m_mode.initModbus(baseAddr + MB_OFS_HR_MODE, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "Mode", this->name.c_str());
        registerBlock(m_mode.getRegisterInfo());

        m_command.initModbus(baseAddr + MB_OFS_HR_CMD, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "Command", this->name.c_str());
        registerBlock(m_command.getRegisterInfo());

        return E_OK;
    }

    short loop() override
    {
        if (!enabled())
        {
            ensure_off_when_disabled();
            return E_OK;
        }
        Component::loop();
        unsigned long currentMillis = millis();
        for (int i = 0; i < MAX_LEDS; ++i)
        {
            // 3. If a blink frequency is set, toggle the LED state periodically.
            if (m_ledFrequencies[i]->getValue() > 0)
            {
                if (currentMillis - lastToggleTime[i] >= m_ledFrequencies[i]->getValue() * 1000)
                {
                    lastToggleTime[i] = currentMillis;
                    bool newState = !m_leds[i]->getValue();
                    if (m_leds[i]->update(newState))
                        updatePinState(i);
                }
            }
        }
        return E_OK;
    }

    uint16_t mb_tcp_base_address() const override
    {
        return _baseAddress;
    }

    short mb_tcp_write(MB_Registers *reg, short value) override
    {
        short result = NetworkComponent::mb_tcp_write(reg, value);
        if (result != E_NOT_IMPLEMENTED)
            return result;

        short offset = reg->startAddress - _baseAddress;

        // Any direct write to an LED coil/freq or a command sets the mode to MANUAL
        if (offset >= MB_OFS_COIL_LED_0 && offset <= MB_OFS_HR_CMD)
        {
            setMode(MODE_MANUAL);
        }

        if (offset >= MB_OFS_COIL_LED_0 && offset <= MB_OFS_COIL_LED_2)
        { // Coil write
            int index = offset - MB_OFS_COIL_LED_0;
            m_ledFrequencies[index]->update(0); // manual override stops blinking
            if (m_leds[index]->update(value > 0))
                updatePinState(index);
            return E_OK;
        }
        else if (offset >= MB_OFS_HR_FREQ_LED_0 && offset <= MB_OFS_HR_FREQ_LED_2)
        { // Freq write
            int index = offset - MB_OFS_HR_FREQ_LED_0;
            m_ledFrequencies[index]->update(value);
            if (m_ledFrequencies[index]->getValue() > 0)
            {
                lastToggleTime[index] = millis();
            }
            return E_OK;
        }
        else if (offset == MB_OFS_HR_CMD)
        { // Command write
            switch (value)
            {
            case ALL_OFF:
                for (int i = 0; i < MAX_LEDS; ++i)
                {
                    m_ledFrequencies[i]->update(0);
                    if (m_leds[i]->update(false))
                        updatePinState(i);
                }
                break;
            case ALL_ON:
                for (int i = 0; i < MAX_LEDS; ++i)
                {
                    m_ledFrequencies[i]->update(0);
                    if (m_leds[i]->update(true))
                        updatePinState(i);
                }
                break;
            case ALL_BLINK:
                for (int i = 0; i < MAX_LEDS; ++i)
                {
                    if (m_ledFrequencies[i]->getValue() == 0)
                        m_ledFrequencies[i]->update(1); // default 1s
                    lastToggleTime[i] = millis();
                }
                break;
            default:
                return E_INVALID_PARAMETER;
            }
            return E_OK;
        }
        else if (offset == MB_OFS_HR_MODE)
        {
            if (value >= MODE_MANUAL && value <= MODE_ERROR)
            {
                Mode newMode = (Mode)value;
                setMode(newMode);
                return E_OK;
            }
            else
            {
                return E_INVALID_PARAMETER;
            }
        }
        return E_INVALID_PARAMETER;
    }

    short mb_tcp_read(MB_Registers *reg) override
    {
        short result = NetworkComponent::mb_tcp_read(reg);
        if (result != E_NOT_IMPLEMENTED)
            return result;

        short offset = reg->startAddress - _baseAddress;
        if (offset >= MB_OFS_COIL_LED_0 && offset <= MB_OFS_COIL_LED_2)
        { // Coil read
            return m_leds[offset - MB_OFS_COIL_LED_0]->getValue() ? 1 : 0;
        }
        else if (offset >= MB_OFS_HR_FREQ_LED_0 && offset <= MB_OFS_HR_FREQ_LED_2)
        { // Freq read
            return m_ledFrequencies[offset - MB_OFS_HR_FREQ_LED_0]->getValue();
        }
        else if (offset == MB_OFS_HR_CMD)
        { // Command read - write only
            return 0;
        }
        else if (offset == MB_OFS_HR_MODE)
        {
            return m_mode.getValue();
        }
        return 0;
    }

    short serial_register(Bridge *bridge) override
    {
        Component::serial_register(bridge);
        return E_OK;
    }
};

#endif
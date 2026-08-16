#ifndef FEEDBACK_3C_H
#define FEEDBACK_3C_H

// Relay output mode
#define RELAY_MODE_NO 0 // Normally Open: HIGH = ON
#define RELAY_MODE_NC 1 // Normally Closed: LOW = ON

// Select the mode for the Feedback3C component
#define FEEDBACK_3C_RELAY_MODE RELAY_MODE_NO

#include <ArduinoLog.h>
#include <App.h>
#include <Component.h>
#include <enums.h>
#include "config.h"
#include <modbus/Modbus.h>
#include <modbus/ModbusTCP.h>
#include "config-modbus.h"

class Bridge;

class Feedback3C : public Component
{
public:
    static const short MAX_LEDS = 3;

    enum E_MB_Offset {
        MB_OFS_COIL_LED_0 = 0,
        MB_OFS_COIL_LED_1 = 1,
        MB_OFS_COIL_LED_2 = 2,
        MB_OFS_HR_FREQ_LED_0 = 3,
        MB_OFS_HR_FREQ_LED_1 = 4,
        MB_OFS_HR_FREQ_LED_2 = 5,
        MB_OFS_HR_CMD = 6,
        MB_OFS_HR_MODE = 7,
        MB_OFS_COUNT = 8,
    };

    enum Mode {
        MODE_MANUAL = 0,
        MODE_STARTUP = 1,
        MODE_STANDBY = 2,
        MODE_RUNNING = 3,
        MODE_WARNING = 4,
        MODE_ERROR = 5,
    };

private:
    enum LedState { LED_OFF, LED_ON, LED_BLINK };

    struct ModeConfig {
        LedState led_states[MAX_LEDS];
    };

    static const ModeConfig& getModeConfig(Mode mode) {
        static const ModeConfig configs[] = {
            /* MODE_MANUAL */  {{LedState::LED_OFF, LedState::LED_OFF, LedState::LED_OFF}},
            /* MODE_STARTUP */ {{LedState::LED_ON,  LedState::LED_ON,  LedState::LED_ON}},
            /* MODE_STANDBY */ {{LedState::LED_ON,  LedState::LED_ON,  LedState::LED_OFF}},
            /* MODE_RUNNING */ {{LedState::LED_OFF, LedState::LED_ON,  LedState::LED_OFF}},
            /* MODE_WARNING */ {{LedState::LED_OFF, LedState::LED_BLINK, LedState::LED_OFF}},
            /* MODE_ERROR */   {{LedState::LED_OFF, LedState::LED_OFF, LedState::LED_BLINK}},
        };
        return configs[mode];
    }

    uint8_t getPinState(bool led_on) {
#if (FEEDBACK_3C_RELAY_MODE == RELAY_MODE_NO)
        return led_on ? HIGH : LOW;
#else // RELAY_MODE_NC
        return led_on ? LOW : HIGH;
#endif
    }

    const short modbusAddress;
    MB_Registers m_modbus_block[MB_OFS_COUNT];
    mutable ModbusBlockView m_modbus_view;

    // for blinking
    unsigned long onOffFrequency[MAX_LEDS]; // in seconds
    unsigned long lastToggleTime[MAX_LEDS];
    Mode currentMode;

    void setLedValue(short index, bool newValue)
    {
        if (index < 0 || index >= MAX_LEDS) return;
        if (value[index] != newValue)
        {
            value[index] = newValue;
            digitalWrite(pins[index], getPinState(newValue));
            notifyStateChange();
        }
    }

public:
    void setMode(Mode newMode) {
        currentMode = newMode;
        if (currentMode == MODE_MANUAL) {
            return; // Don't change LED states when switching to manual
        }

        if (newMode < MODE_STARTUP || newMode > MODE_ERROR) {
            return; // Invalid mode
        }

        const ModeConfig& config = getModeConfig(newMode);

        for (int i = 0; i < MAX_LEDS; ++i) {
            onOffFrequency[i] = 0; // Reset frequency
            switch (config.led_states[i]) {
                case LED_OFF:
                    setLedValue(i, false);
                    break;
                case LED_ON:
                    setLedValue(i, true);
                    break;
                case LED_BLINK:
                    setLedValue(i, true); // Start with LED on
                    onOffFrequency[i] = 1; // Default 1s blink rate
                    lastToggleTime[i] = millis();
                    break;
            }
        }
    }

    const short pins[MAX_LEDS];
    bool value[MAX_LEDS];

    enum Command {
        ALL_OFF = 0,
        ALL_ON = 1,
        ALL_BLINK = 2,
    };

    Feedback3C(Component *owner, short pin0, short pin1, short pin2, short _id, short _modbusAddress)
        : Component("Feedback3C", _id, Component::COMPONENT_DEFAULT, owner),
          pins{pin0, pin1, pin2},
          modbusAddress(_modbusAddress)
    {
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
        for(int i=0; i<MAX_LEDS; ++i) {
            value[i] = false;
            onOffFrequency[i] = 0;
            lastToggleTime[i] = 0;
        }
        setMode(MODE_STARTUP);

        // 3 Coils for LEDs
        m_modbus_block[MB_OFS_COIL_LED_0] = INIT_MODBUS_BLOCK_TCP(this->modbusAddress, MB_OFS_COIL_LED_0, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, "LED 0", "Feedback3C");
        m_modbus_block[MB_OFS_COIL_LED_1] = INIT_MODBUS_BLOCK_TCP(this->modbusAddress, MB_OFS_COIL_LED_1, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, "LED 1", "Feedback3C");
        m_modbus_block[MB_OFS_COIL_LED_2] = INIT_MODBUS_BLOCK_TCP(this->modbusAddress, MB_OFS_COIL_LED_2, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, "LED 2", "Feedback3C");

        // 3 HRs for Blink Frequency
        m_modbus_block[MB_OFS_HR_FREQ_LED_0] = INIT_MODBUS_BLOCK_TCP(this->modbusAddress, MB_OFS_HR_FREQ_LED_0, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "LED 0 Freq", "Feedback3C");
        m_modbus_block[MB_OFS_HR_FREQ_LED_1] = INIT_MODBUS_BLOCK_TCP(this->modbusAddress, MB_OFS_HR_FREQ_LED_1, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "LED 1 Freq", "Feedback3C");
        m_modbus_block[MB_OFS_HR_FREQ_LED_2] = INIT_MODBUS_BLOCK_TCP(this->modbusAddress, MB_OFS_HR_FREQ_LED_2, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "LED 2 Freq", "Feedback3C");

        // 1 HR for Command
        m_modbus_block[MB_OFS_HR_CMD] = INIT_MODBUS_BLOCK_TCP(this->modbusAddress, MB_OFS_HR_CMD, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_WRITE_ONLY, "Command", "Feedback3C");

        m_modbus_block[MB_OFS_HR_MODE] = INIT_MODBUS_BLOCK_TCP(this->modbusAddress, MB_OFS_HR_MODE, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, "Mode", "Feedback3C");

        m_modbus_view.data = m_modbus_block;
        m_modbus_view.count = MB_OFS_COUNT;
    }

    short setup() override {
        Component::setup();
        for(int i=0; i<MAX_LEDS; ++i) {
            pinMode(pins[i], OUTPUT);
            digitalWrite(pins[i], getPinState(value[i]));
        }
        Log.verboseln("Feedback3C::setup - ID %d, Pins: %d,%d,%d, Modbus Addr: %d", id, pins[0], pins[1], pins[2], modbusAddress);
        return E_OK;
    }

    short loop() override {
        Component::loop();
        unsigned long currentMillis = millis();
        for (int i=0; i<MAX_LEDS; ++i) {
            if (onOffFrequency[i] > 0) {
                if (currentMillis - lastToggleTime[i] >= onOffFrequency[i] * 1000) {
                    lastToggleTime[i] = currentMillis;
                    setLedValue(i, !value[i]);
                }
            }
        }
        return E_OK;
    }

    short mb_tcp_write(short address, short networkValue) override {
        short offset = address - modbusAddress;

        // Any direct write to an LED coil/freq or a command sets the mode to MANUAL
        if (offset < MB_OFS_HR_MODE) {
            setMode(MODE_MANUAL);
        }

        if (offset >= MB_OFS_COIL_LED_0 && offset <= MB_OFS_COIL_LED_2) { // Coil write
            onOffFrequency[offset] = 0; // manual override
            setLedValue(offset, networkValue > 0);
            return E_OK;
        } else if (offset >= MB_OFS_HR_FREQ_LED_0 && offset <= MB_OFS_HR_FREQ_LED_2) { // Freq write
            int index = offset - MB_OFS_HR_FREQ_LED_0;
            onOffFrequency[index] = networkValue;
            if (onOffFrequency[index] > 0) {
                lastToggleTime[index] = millis();
            }
            return E_OK;
        } else if (offset == MB_OFS_HR_CMD) { // Command write
            switch(networkValue) {
                case ALL_OFF:
                    for(int i=0; i<MAX_LEDS; ++i) {
                        onOffFrequency[i] = 0;
                        setLedValue(i, false);
                    }
                    break;
                case ALL_ON:
                    for(int i=0; i<MAX_LEDS; ++i) {
                        onOffFrequency[i] = 0;
                        setLedValue(i, true);
                    }
                    break;
                case ALL_BLINK:
                     for(int i=0; i<MAX_LEDS; ++i) {
                        if(onOffFrequency[i] == 0) onOffFrequency[i] = 1; // default 1s
                        lastToggleTime[i] = millis();
                    }
                    break;
                default:
                    return E_INVALID_PARAMETER;
            }
            return E_OK;
        } else if (offset == MB_OFS_HR_MODE) {
            if (networkValue >= MODE_MANUAL && networkValue <= MODE_ERROR) {
                setMode((Mode)networkValue);
                return E_OK;
            } else {
                return E_INVALID_PARAMETER;
            }
        }
        return E_INVALID_PARAMETER;
    }

    short mb_tcp_read(short address) override {
        short offset = address - modbusAddress;
        if (offset >= MB_OFS_COIL_LED_0 && offset <= MB_OFS_COIL_LED_2) { // Coil read
            return value[offset] ? 1 : 0;
        } else if (offset >= MB_OFS_HR_FREQ_LED_0 && offset <= MB_OFS_HR_FREQ_LED_2) { // Freq read
            return onOffFrequency[offset - MB_OFS_HR_FREQ_LED_0];
        } else if (offset == MB_OFS_HR_CMD) { // Command read - write only
            return 0;
        } else if (offset == MB_OFS_HR_MODE) {
            return currentMode;
        }
        return 0;
    }

    short mb_tcp_write(MB_Registers *reg, short networkValue) override { return mb_tcp_write(reg->startAddress, networkValue); }
    short mb_tcp_read(MB_Registers *reg) override { return mb_tcp_read(reg->startAddress); }

    void mb_tcp_register(ModbusTCP *manager) override {
        ModbusBlockView *blocksView = mb_tcp_blocks();
        Component *thiz = const_cast<Feedback3C *>(this);
        for (int i = 0; i < blocksView->count; ++i) {
            MB_Registers info = blocksView->data[i];
            manager->registerModbus(thiz, info);
        }
    }

    ModbusBlockView *mb_tcp_blocks() const override {
        return &m_modbus_view;
    }

    short serial_register(Bridge *bridge) override {
        Component::serial_register(bridge);
        return E_OK;
    }
};

#endif 
#ifndef LED_FEEDBACK_H
#define LED_FEEDBACK_H

#if ENABLE_FEEDBACK_LED_0

#include <ArduinoLog.h>
#include "config.h"        // For LED_UPDATE_INTERVAL_MS potentially
#include <App.h>              // If needed by Component or others
#include <Component.h>
#include <Adafruit_NeoPixel.h> // NeoPixel library
#include <modbus/ModbusTCP.h> // For Modbus interaction base
#include "config-modbus.h"   // For Modbus addresses/constants

// Forward declare if necessary
class Bridge;

// Define max registers needed (Mode + potential future params like Brightness)
#define MAX_LED_FEEDBACK_REGISTERS 3

class LEDFeedback : public Component {
public:
    // Define modes for the LED strip
    enum class LEDMode : ushort {
        OFF = 0,
        FADE_R_B = 1, // Fade Red <-> Blue
        RANGE = 2,    // Display a level 0-100
        TRI_COLOR_BLINK = 3 // Three sections blinking
        // Add more modes later, e.g., SOLID_COLOR, RAINBOW
    };

    // Define Modbus register offsets relative to base address
    enum class LEDRegOffset : ushort {
        MODE = 0,
        LEVEL = 1, // For RANGE mode level (0-100)
        // BRIGHTNESS = 2 // Add later if needed
    };

private:
    // General Configuration & State
    const ushort m_pin;          // NeoPixel data pin
    const ushort m_pixelCount;   // Number of pixels in the strip
    const ushort m_modbusAddr;   // Base Modbus address
    Adafruit_NeoPixel m_strip;   // NeoPixel Object
    LEDMode m_mode = LEDMode::OFF; // Current active mode
    unsigned long m_lastUpdateMs = 0; // Timestamp of the last general LED update

    // Modbus Definitions
    MB_Registers m_modbusBlocks[MAX_LED_FEEDBACK_REGISTERS];
    ushort m_modbusBlockCount = 0;
    ModbusBlockView m_modbusView;

    // --- LEDMode::FADE_R_B ---
    // State
    float m_fadeProgress = 0.0f;      // 0.0 to 1.0 for fade
    bool m_fadingUp = false;          // Direction of fade (false = C1->C2, true = C2->C1)
    // Colors (could be made configurable later)
    const uint32_t m_color1 = Adafruit_NeoPixel::Color(255, 0, 0); // Red
    const uint32_t m_color2 = Adafruit_NeoPixel::Color(0, 0, 255); // Blue
    // Helper
    uint32_t calculateFadeColor() const;
    void handleModeFadeRB();

    // --- LEDMode::RANGE ---
    // State
    ushort m_rangeLevel = 0;       // Level for RANGE mode (0-100)
    // Helper
    void handleModeRange();

    // --- LEDMode::TRI_COLOR_BLINK ---
    // State
    bool m_triColorBlinkStateOn = true; 
    unsigned long m_lastBlinkToggleMs = 0;
    // Helper
    void handleModeTriColorBlink();

    // General Private Helpers
    void handleModeOff();      // Handles LEDMode::OFF

public:
    LEDFeedback(
        Component *owner,
        ushort _pin,
        ushort _pixelCount,
        ushort _id,
        ushort _modbusAddress);

    ~LEDFeedback() override = default; // Default destructor is likely fine

    // Component Lifecycle
    short setup() override;
    short loop() override;
    short info(short val0 = 0, short val1 = 0) override;
    short debug() override { return info(0, 0); }

    // Mode Control (optional direct methods)
    // void setMode(LEDMode newMode);
    // LEDMode getMode() const { return m_mode; }

    // Modbus Interface
    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;
    void mb_tcp_register(ModbusTCP *manager) override;
    ModbusBlockView *mb_tcp_blocks() const override;

    // Serial Interface
    short serial_register(Bridge *bridge) override;

protected:
    void notifyStateChange() override; // Override if mode change notification is useful
};
#endif // ENABLE_FEEDBACK_LED_0
#endif // LED_FEEDBACK_H 
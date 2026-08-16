#if ENABLE_FEEDBACK_LED_0
#include "LEDFeedback.h"
#include <Arduino.h> // For millis(), pinMode potentially (though NeoPixel lib handles pin)
#include <string.h> // For memset

// Define update interval and fade increment locally for now
// These could be moved to config.h if needed elsewhere
const unsigned long LED_UPDATE_INTERVAL_MS = 20; // ms -> ~50 FPS update rate
const float FADE_INCREMENT = 0.01f; // Controls the speed of the fade (smaller = slower)
const unsigned long TRI_COLOR_BLINK_INTERVAL_MS = 500; // Blink interval for TRI_COLOR_BLINK mode

// Define colors for TRI_COLOR_BLINK mode (Traffic Light: Red, Yellow, Green)
const uint32_t TRI_COLOR_SECTION1 = Adafruit_NeoPixel::Color(255, 0, 0);   // Red
const uint32_t TRI_COLOR_SECTION2 = Adafruit_NeoPixel::Color(255, 255, 0); // Yellow
const uint32_t TRI_COLOR_SECTION3 = Adafruit_NeoPixel::Color(0, 255, 0);   // Green

LEDFeedback::LEDFeedback(
    Component *owner,
    ushort _pin,
    ushort _pixelCount,
    ushort _id,
    ushort _modbusAddress) : Component("LEDFeedback", _id, Component::COMPONENT_DEFAULT, owner),
                           m_pin(_pin),
                           m_pixelCount(_pixelCount > 0 ? _pixelCount : 1), // Ensure at least 1 pixel
                           m_modbusAddr(_modbusAddress),
                           // Initialize NeoPixel strip object.
                           // Type is NEO_GRB + NEO_KHZ800, common for WS2812B/SK6812
                           // Adjust if using different pixel types (e.g., NEO_RGB, NEO_KHZ400)
                           m_strip(_pixelCount > 0 ? _pixelCount : 1, _pin, NEO_GRB + NEO_KHZ800),
                           m_mode(LEDMode::OFF), // Default to OFF
                           m_lastUpdateMs(0),
                           m_fadeProgress(0.0f),
                           m_fadingUp(false),
                           m_rangeLevel(0), // Initialize range level
                           m_triColorBlinkStateOn(true), // Default to on state
                           m_lastBlinkToggleMs(0),
                           m_modbusBlockCount(0) // Initialize actual count
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

    if (_pixelCount == 0) {
        Log.warningln("LEDFeedback ID:%d: pixelCount is 0, defaulting to 1.", id);
    }

    // Define Modbus blocks
    memset(m_modbusBlocks, 0, sizeof(m_modbusBlocks)); // Clear array
    int blockIndex = 0;

    // Register 0: Mode Control
    if (blockIndex < MAX_LED_FEEDBACK_REGISTERS) {
        m_modbusBlocks[blockIndex++] = {
            static_cast<ushort>(m_modbusAddr + static_cast<ushort>(LEDRegOffset::MODE)),
            1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_WRITE, // Mode is R/W
            static_cast<ushort>(id), static_cast<ushort>(LEDRegOffset::MODE),
            "LED Mode", "LED"
        };
    }
    // Register 1: Level Control for RANGE mode
    if (blockIndex < MAX_LED_FEEDBACK_REGISTERS) {
        m_modbusBlocks[blockIndex++] = {
            static_cast<ushort>(m_modbusAddr + static_cast<ushort>(LEDRegOffset::LEVEL)),
            1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_WRITE, // Level is R/W
            static_cast<ushort>(id), static_cast<ushort>(LEDRegOffset::LEVEL),
            "LED Level", "LED"
        };
    }
    // Add other registers (e.g., Brightness) here later if needed

    m_modbusBlockCount = blockIndex; // Set the actual number of registers used

    // Set up the view
    m_modbusView.data = m_modbusBlocks;
    m_modbusView.count = m_modbusBlockCount;
}

short LEDFeedback::setup() {
    Component::setup();
    m_strip.begin();           // Initialize NeoPixel library
    m_strip.clear();           // Initialize all pixels to 'off'
    m_strip.setBrightness(50); // Set moderate initial brightness (0-255), adjust as needed
    m_strip.show();            // Send data to pixels
    m_lastUpdateMs = millis(); // Initialize timestamp
    Log.verboseln("LEDFeedback ID:%d setup. Pin:%d, Pixels:%d, Mode:%d, MBAddr:%d",
                  id, m_pin, m_pixelCount, static_cast<ushort>(m_mode), m_modbusAddr);
    return E_OK;
}

void LEDFeedback::handleModeOff() {
    // Optimization: Check if strip is already off? getPixelColor(0) might work.
    // For simplicity, just clear and show.
    m_strip.clear();
    m_strip.show(); 
}

void LEDFeedback::handleModeFadeRB() {
    // Update fade progress
    if (m_fadingUp) {
        m_fadeProgress += FADE_INCREMENT;
        if (m_fadeProgress >= 1.0f) {
            m_fadeProgress = 1.0f;
            m_fadingUp = false; // Change direction
        }
    } else {
        m_fadeProgress -= FADE_INCREMENT;
        if (m_fadeProgress <= 0.0f) {
            m_fadeProgress = 0.0f;
            m_fadingUp = true; // Change direction
        }
    }
    // Calculate color and fill strip
    m_strip.fill(calculateFadeColor());
    m_strip.show(); // Update the strip
}

void LEDFeedback::handleModeRange() {
    m_strip.clear(); // Clear all pixels first

    // Calculate how many pixels to light
    // Ensure m_pixelCount is not zero to avoid division by zero, though constructor ensures it's at least 1.
    ushort pixelsToLight = static_cast<ushort>((m_rangeLevel / 100.0f) * m_pixelCount);
    if (pixelsToLight > m_pixelCount) pixelsToLight = m_pixelCount; // Cap at max

    // Define color for RANGE mode (e.g., Green) - could be configurable
    uint32_t rangeColor = m_strip.Color(0, 255, 0); // Green

    for (ushort i = 0; i < pixelsToLight; ++i) {
        m_strip.setPixelColor(i, rangeColor);
    }
    m_strip.show();
}

void LEDFeedback::handleModeTriColorBlink() {
    unsigned long now = millis(); // Use Component::now if available and preferred

    // Check if it's time to toggle blink state
    if (now - m_lastBlinkToggleMs >= TRI_COLOR_BLINK_INTERVAL_MS) {
        m_triColorBlinkStateOn = !m_triColorBlinkStateOn;
        m_lastBlinkToggleMs = now;
    }

    if (m_triColorBlinkStateOn) {
        ushort pixelsPerSection = m_pixelCount / 3;
        ushort section1End = pixelsPerSection;
        ushort section2End = 2 * pixelsPerSection;

        for (ushort i = 0; i < m_pixelCount; ++i) {
            if (i < section1End) {
                m_strip.setPixelColor(i, TRI_COLOR_SECTION1);
            } else if (i < section2End) {
                m_strip.setPixelColor(i, TRI_COLOR_SECTION2);
            } else {
                m_strip.setPixelColor(i, TRI_COLOR_SECTION3);
            }
        }
    } else {
        m_strip.clear();
    }
    m_strip.show();
}

short LEDFeedback::loop() {
    Component::loop();
    unsigned long now = millis(); // Use Component::now if available and preferred

    // Allow updates slightly more frequently if needed, but base logic on interval
    if (now - m_lastUpdateMs < LED_UPDATE_INTERVAL_MS) {
        return E_OK; // Not time to update yet
    }
    m_lastUpdateMs = now;

    switch (m_mode) {
        case LEDMode::OFF:
            handleModeOff();
            break;

        case LEDMode::FADE_R_B:
            handleModeFadeRB();
            break;

        case LEDMode::RANGE:
            handleModeRange();
            break;

        case LEDMode::TRI_COLOR_BLINK:
            handleModeTriColorBlink();
            break;

        // Add cases for future modes here
        default:
            // Unknown mode, default to OFF
             if (m_mode != LEDMode::OFF) { 
                 Log.warningln("LEDFeedback ID:%d: Unknown mode %d, turning OFF.", id, static_cast<ushort>(m_mode));
                 m_mode = LEDMode::OFF; // Ensure state consistency
             }
             handleModeOff(); // Default to OFF behavior
            break;
    }

    return E_OK;
}

// Helper to calculate intermediate color for fade
uint32_t LEDFeedback::calculateFadeColor() const {
    uint8_t r1 = (m_color1 >> 16) & 0xFF;
    uint8_t g1 = (m_color1 >> 8) & 0xFF;
    uint8_t b1 = m_color1 & 0xFF;
    uint8_t r2 = (m_color2 >> 16) & 0xFF;
    uint8_t g2 = (m_color2 >> 8) & 0xFF;
    uint8_t b2 = m_color2 & 0xFF;

    float progress = m_fadeProgress; // Use current progress
    // Ensure progress stays within [0.0, 1.0] if FADE_INCREMENT logic has edge cases
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    uint8_t r = static_cast<uint8_t>(r1 * (1.0f - progress) + r2 * progress);
    uint8_t g = static_cast<uint8_t>(g1 * (1.0f - progress) + g2 * progress);
    uint8_t b = static_cast<uint8_t>(b1 * (1.0f - progress) + b2 * progress);

    return m_strip.Color(r, g, b);
}


short LEDFeedback::info(short val0, short val1) {
    Log.infoln("LEDFeedback::info - ID: %d", id);
    Log.infoln("  Pin: %d, Pixel Count: %d", m_pin, m_pixelCount);
    Log.infoln("  Modbus Addr: %d, Block Count: %d", m_modbusAddr, m_modbusBlockCount);
    const char* modeStr = "UNKNOWN";
    switch(m_mode) {
        case LEDMode::OFF: modeStr = "OFF"; break;
        case LEDMode::FADE_R_B: modeStr = "FADE_R_B"; break;
        case LEDMode::RANGE: modeStr = "RANGE"; break;
        case LEDMode::TRI_COLOR_BLINK: modeStr = "TRI_COLOR_BLINK"; break;
    }
    Log.infoln("  Current Mode: %d (%s)", static_cast<ushort>(m_mode), modeStr);
    if (m_mode == LEDMode::FADE_R_B) {
        Log.infoln("  Fade Progress: %.2f, Fading Up: %s", m_fadeProgress, m_fadingUp ? "true" : "false");
    }
    if (m_mode == LEDMode::RANGE) {
        Log.infoln("  Range Level: %d", m_rangeLevel);
    }
    // Log Modbus block details if helpful
    // for(ushort i=0; i<m_modbusBlockCount; ++i) { Log.infoln("  MB %d: Addr=%d Name=%s", i, m_modbusBlocks[i].startAddress, m_modbusBlocks[i].name); }
    return E_OK;
}

short LEDFeedback::mb_tcp_write(MB_Registers *reg, short networkValue) {
    if (!reg || m_modbusBlockCount == 0) return E_INVALID_PARAMETER;

    ushort baseAddr = m_modbusAddr;
    ushort regAddr = reg->startAddress;

    if (regAddr == static_cast<ushort>(baseAddr + static_cast<ushort>(LEDRegOffset::MODE))) {
        LEDMode newMode = static_cast<LEDMode>(networkValue);

        // Validate the new mode (add future valid modes to this check)
        if (newMode == LEDMode::OFF || newMode == LEDMode::FADE_R_B || newMode == LEDMode::RANGE || newMode == LEDMode::TRI_COLOR_BLINK) {
             if (newMode != m_mode) {
                Log.verboseln("LEDFeedback ID:%d: Mode change %d -> %d", id, static_cast<ushort>(m_mode), static_cast<ushort>(newMode));
                m_mode = newMode;
                // Reset mode-specific state when mode changes
                m_fadeProgress = 0.0f;
                m_fadingUp = false;
                // Reset blink state for TRI_COLOR_BLINK
                m_triColorBlinkStateOn = true;
                m_lastBlinkToggleMs = millis(); 
                m_lastUpdateMs = millis(); // Force immediate update in new mode
                notifyStateChange(); // Notify app/others if needed
             }
             return E_OK;
        } else {
            Log.warningln("LEDFeedback ID:%d: Invalid mode value written: %d", id, networkValue);
            return MODBUS_ERROR_ILLEGAL_DATA_VALUE; 
        }
    } else if (regAddr == static_cast<ushort>(baseAddr + static_cast<ushort>(LEDRegOffset::LEVEL))) {
        if (networkValue >= 0 && networkValue <= 100) {
            if (static_cast<ushort>(networkValue) != m_rangeLevel) {
                Log.verboseln("LEDFeedback ID:%d: Level change %d -> %d", id, m_rangeLevel, networkValue);
                m_rangeLevel = static_cast<ushort>(networkValue);
                m_lastUpdateMs = millis(); // Force update if mode is RANGE
                notifyStateChange(); 
            }
            return E_OK;
        } else {
            Log.warningln("LEDFeedback ID:%d: Invalid level value written: %d (must be 0-100)", id, networkValue);
            return MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
    }
    // Handle other writable registers here later

    Log.warningln("LEDFeedback ID:%d: Write attempt to unhandled Modbus address %d.", id, regAddr);
    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS; 
}

short LEDFeedback::mb_tcp_read(MB_Registers *reg) {
     if (!reg || m_modbusBlockCount == 0) return 0; // Or error code

    ushort baseAddr = m_modbusAddr;
    ushort regAddr = reg->startAddress;

    if (regAddr == static_cast<ushort>(baseAddr + static_cast<ushort>(LEDRegOffset::MODE))) {
        return static_cast<short>(m_mode);
    }
    if (regAddr == static_cast<ushort>(baseAddr + static_cast<ushort>(LEDRegOffset::LEVEL))) {
        return static_cast<short>(m_rangeLevel);
    }
    // Handle other readable registers here later

    Log.warningln("LEDFeedback ID:%d: Read attempt from unhandled Modbus address %d.", id, regAddr);
    return 0; 
}

void LEDFeedback::mb_tcp_register(ModbusTCP *manager) {
    if (!manager || !hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS) || m_modbusBlockCount == 0) {
        return;
    }
    ModbusBlockView *blocksView = mb_tcp_blocks();
    Component *thiz = const_cast<LEDFeedback *>(this);
    if (blocksView && blocksView->data) {
        for (int i = 0; i < blocksView->count; ++i) {
            MB_Registers info = blocksView->data[i]; 
            manager->registerModbus(thiz, info);
        }
    }
}

ModbusBlockView *LEDFeedback::mb_tcp_blocks() const {
    return const_cast<ModbusBlockView*>(&m_modbusView);
}

short LEDFeedback::serial_register(Bridge *bridge) {
     if (!bridge) return E_INVALID_PARAMETER;
     bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&LEDFeedback::info);
     return E_OK;
}

void LEDFeedback::notifyStateChange() {
    Component::notifyStateChange();
    // Currently called when mode changes via Modbus
} 
#endif // ENABLE_FEEDBACK_LED_0
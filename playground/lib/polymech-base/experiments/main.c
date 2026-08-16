// component_networked_demo.cpp  ---------------------------------------------
#include <cstdint>
#include <cstddef>
#include <array>

#include <type_traits>
#include <ArduinoLog.h>

/* ────────────────────────────────── 0. Build flag → constexpr ──────────── */
#if defined(ENABLE_MODBUS) && ENABLE_MODBUS
    #define kEnableModbus true
#else
    
#endif

/* ────────────────────────────────── 1. Helper: maybe‑base ──────────────── */
struct dummy_base {};

template <typename Base, bool Enable>
using maybe_base_t =
    typename std::conditional<Enable, Base, dummy_base>::type;

/* ────────────────────────────────── 2. Core contracts ──────────────────── */
struct Component {
    virtual void setup() = 0;
    virtual void loop()  = 0;
    virtual ~Component() = default;
};

struct RegisterMap {
    const uint16_t* data;
    std::size_t     size;
};

struct Networked {
    virtual RegisterMap registerMapping() const = 0;
    virtual uint16_t    deviceAddress()   const { return 1; }
    virtual void        onRegisterWrite(uint16_t, uint16_t) {}
    virtual ~Networked() = default;
};

/* ────────────────────────────────── 3. ComponentNetworked ──────────────── */
//  Carries Networked only when kEnableModbus == true
using NetworkBase = maybe_base_t<Networked, kEnableModbus>;

class ComponentNetworked : public Component, public NetworkBase {
    /* purely a glue; still abstract if Networked is really inherited */
};

/* ────────────────────────────────── 4. TemperatureSensorNetworked ─────── */
//  Two specialisations – the compiler picks one at translation time
template <bool Enable>
class TemperatureSensorNetworkedImpl;

/* ----- 4.a  NON‑networked flavour --------------------------------------- */
template <>
class TemperatureSensorNetworkedImpl<false> : public ComponentNetworked
{
public:
    void setup() override { Log.notice(F("[Temp<%d>] setup (lean)\n"), kEnableModbus); }
    void loop()  override { /* Log.verb(F("[Temp<%d>] loop  (lean)\n"), kEnableModbus); */ }

    float readCelsius() const { return 25.0f; }
};

/* ----- 4.b  Network‑capable flavour ------------------------------------- */
template <>
class TemperatureSensorNetworkedImpl<true> : public Component, public Networked
{
    std::array<uint16_t, 2> regs_{ 250 /*25 °C×10*/, 0 };

public:
    /* Component part */
    void setup() override { Log.notice(F("[Temp<%d>] setup (net)\n"), kEnableModbus); }
    void loop()  override { /* Log.verb(F("[Temp<%d>] loop  (net)\n"), kEnableModbus); */ }

    /* Networked part */
    RegisterMap registerMapping() const override {
        return { regs_.data(), regs_.size() };
    }
    uint16_t deviceAddress() const override { return 42; }
    void onRegisterWrite(uint16_t r, uint16_t v) override {
        if (r < regs_.size()) regs_[r] = v;
    }

    float readCelsius() const { return regs_[0] / 10.0f; }
};

/* ----- 4.c  Public alias ------------------------------------------------- */
using TemperatureSensorNetworked =
        TemperatureSensorNetworkedImpl<kEnableModbus>;

/* ────────────────────────────────── 5. Arduino setup/loop ───────────── */

// Global instance of the sensor
TemperatureSensorNetworked dev;

void setup() {
    // Initialize Serial and Logging
    Serial.begin(115200);
    while (!Serial && millis() < 3000) { delay(100); } // Wait for serial port connection

    Log.begin(LOG_LEVEL_VERBOSE, &Serial);
    Log.notice(F("\n--- Logging Initialized ---\n"));
    Log.notice(F("kEnableModbus = %d\n"), kEnableModbus);

    dev.setup();

    // Conditionally compile the network check using the preprocessor
#if kEnableModbus
    Log.notice(F("Network features compiled.\n"));
    // Check if the resolved type actually inherits from Networked
    if (std::is_base_of<Networked, TemperatureSensorNetworked>::value) {
         Log.notice(F("  Device IS Networked. Address: %d, Registers:"), dev.deviceAddress());
         auto map = dev.registerMapping();
         for (std::size_t i = 0; i < map.size; ++i) Log.notice(F(" %d"), map.data[i]);
         Log.notice(F("\n"));
    } else {
         Log.warning(F("  Device IS NOT Networked (type check failed).\n"));
    }
#else
    Log.notice(F("Network features NOT compiled.\n"));
    // Demonstrate accessing non-networked specific method
    Log.notice(F("  Device reading (lean): %.1f C\n"), dev.readCelsius());
#endif
}

void loop() {
    dev.loop();
}
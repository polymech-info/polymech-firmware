// feature_di_component_demo.cpp  -------------------------------------------
//  Build the "lean" variant  : g++ -std=c++20 -O2 -DDEMO_MAIN feature_di_component_demo.cpp -o demo
//  Build the "Modbus" variant: g++ -std=c++20 -O2 -DDEMO_MAIN -DENABLE_MODBUS feature_di_component_demo.cpp -o demo_net
// @link ://chatgpt.com/share/6803c6b7-0468-8001-857f-352a31c8c9bd
// ───────────────────────────────────────────────────────────────────────────

#include <Arduino.h> // Added for Serial
#include <cstdint>
#include <cstddef>
#include <array>
// #include <iostream> // Replaced by Arduino.h
#include <type_traits>
#include <string_view> // For info() return type example
#include <cstring> // For strcmp

/*──────────────────── 1.  Tiny utility -----------------------------------*/
struct RegisterMap {
    const uint16_t* data;
    std::size_t     size;
};

/*──────────────────── 2.  "Feature" concept (any module that has setup/loop/info)*/
// Renamed and updated SFINAE check
template<typename, typename = std::void_t<>>
struct is_valid_feature : std::false_type {};

template<typename F>
struct is_valid_feature<F, std::void_t<
    decltype(std::declval<F&>().setup()),
    decltype(std::declval<F&>().loop()),
    decltype(std::declval<const F&>().info()) // Added const info() check
>> : std::true_type {};

// Updated helper function
template<typename F> constexpr bool is_valid_feature_func() { return is_valid_feature<F>::value; }


/*──────────────────── 3.  Individual feature packs -----------------------*/
// 3a.  Core sensor logic  ────────────────────────────────────────────────
struct CoreFeature {
    static constexpr bool has_network = false;

    void setup() { Serial.println(F("[Core] setup")); } // Use Serial
    void loop () { /*Serial.println(F("[Core] loop"));*/ } // Commented out for less noise
    // Added info method
    void info() const { Serial.println(F("[Core] Info: Basic temperature sensing.")); } // Use Serial

    float readCelsius() const { return 25.0f; }
};

// 3b.  Optional Modbus/Network feature  ──────────────────────────────────
struct ModbusFeature {
    static constexpr bool has_network = true;

    std::array<uint16_t,2> regs{ 250 /*25 °C×10*/, 0 };

    void setup() { Serial.println(F("[Modbus] setup")); } // Use Serial
    void loop () { /*Serial.println(F("[Modbus] loop"));*/ } // Commented out for less noise
    // Added info method
    void info() const { // Use Serial
        Serial.print(F("[Modbus] Info: Modbus TCP/IP enabled. Address: "));
        Serial.println(deviceAddress());
    }


    RegisterMap registerMapping() const { return { regs.data(), regs.size() }; }
    uint16_t    deviceAddress()   const { return 42; }

    void onRegisterWrite(uint16_t r, uint16_t v) {
        if (r < regs.size()) regs[r] = v;
    }
};

// 3c. Default Logger feature ─────────────────────────────────────────────
struct LoggerFeature {
    static constexpr bool has_network = true; // Assuming logger might send logs over network
    void setup() { Serial.println(F("[Logger] setup")); } // Use Serial
    void loop () { /*Serial.println(F("[Logger] loop"));*/ } // Commented out for less noise
    // Added info method
    void info() const { Serial.println(F("[Logger] Info: Basic console logging active.")); } // Use Serial
};


/*──────────────────── 4.  Generic "Device" aggregator ───────────────────*/
// Use static_assert with the updated SFINAE check
// Inherit LoggerFeature by default
template<typename... Fs>
class Device : public LoggerFeature, public Fs...
{
    // Ensure the default and all *additional* base classes Fs satisfy the requirements
    static_assert(is_valid_feature_func<LoggerFeature>(), "Default LoggerFeature must implement setup(), loop(), and info()");
    // Use C++17 fold expression within the static_assert for additional features
    static_assert(sizeof...(Fs) == 0 || (is_valid_feature_func<Fs>() && ...), "All additional features must implement setup(), loop(), and info()");


public:
    void setup() {
        LoggerFeature::setup(); // Explicitly call setup for the default feature
        if constexpr (sizeof...(Fs) > 0) { // Check if there are other features
             (Fs::setup(), ...);           // Call setup for the rest using fold-expression
        }
    }

    void loop () {
        LoggerFeature::loop(); // Explicitly call loop for the default feature
        if constexpr (sizeof...(Fs) > 0) {
             (Fs::loop (), ...);           // Call loop for the rest
        }
    }

    // Added aggregated info method
    void info() const {
        // Call info() for LoggerFeature and all Fs... using a fold expression
        (LoggerFeature::info(), (Fs::info(), ...)); 
    }

    // Include LoggerFeature's network status in the calculation
    static constexpr bool has_network = LoggerFeature::has_network || (false || ... || Fs::has_network);
};

/*──────────────────── 5.  Choose the feature bundle (application layer) ───*/
// LoggerFeature is now implicitly included by Device
#ifdef   ENABLE_MODBUS
using TemperatureSensor = Device<CoreFeature, ModbusFeature>;
#else
using TemperatureSensor = Device<CoreFeature>;
#endif

/*──────────────────── Arduino Entry Points -------------------------------*/
// Instantiate the device globally
TemperatureSensor device;

// Buffer for serial commands
#define SERIAL_CMD_BUFFER_SIZE 64
char serialCmdBuffer[SERIAL_CMD_BUFFER_SIZE];
uint8_t serialCmdBufferIdx = 0;

// Helper function to print commands
void printHelp() {
  Serial.println(F("--- Serial Commands ---"));
  Serial.println(F("1 - Show device info"));
  Serial.println(F("? - Print this help message"));
}

void setup() {
    // Initialize Serial for logging, if needed (optional)
    Serial.begin(115200);
    // Wait for serial port to connect (needed for native USB)
    // On some boards, this delay is needed to allow the Serial Monitor to connect
    // On others (like ESP32), Serial begins immediately. Adjust if needed.
    delay(1000); 
    while (!Serial && millis() < 3000); // Wait up to 3 seconds

    Serial.println(F("\n--- Serial Interface Ready ---"));
    printHelp();

    // <<< Hypothetical serial connection point >>>
    Serial.println(F("--- Device Info ---"));
    device.info(); // Call the aggregated info method
    Serial.println(F("--- Device Setup ---"));
    device.setup(); // Call the aggregated setup
    Serial.println(F("--- Setup Complete ---"));
}

void loop() {
    device.loop();  // Call the aggregated loop

    // --- Handle Serial Input ---
    while (Serial.available() > 0) {
        char receivedChar = Serial.read();

        // Handle backspace
        if (receivedChar == '\b' || receivedChar == 127) {
            if (serialCmdBufferIdx > 0) {
                serialCmdBufferIdx--;
                Serial.print("\b \b"); // Move cursor back, print space, move back again
            }
            continue; // Continue to next character
        }

        // Echo character back (optional, good for interactive use)
        Serial.print(receivedChar);

        // Process command on newline or carriage return
        if (receivedChar == '\n' || receivedChar == '\r') {
            serialCmdBuffer[serialCmdBufferIdx] = '\0'; // Null-terminate the string
            Serial.println(); // Move to next line after command received

            if (serialCmdBufferIdx > 0) { // Process if buffer is not empty
                // --- Process Commands ---
                if (strcmp(serialCmdBuffer, "1") == 0) {
                    Serial.println(F("--- Device Info ---"));
                    device.info();
                } else if (strcmp(serialCmdBuffer, "?") == 0) {
                    printHelp();
                } else {
                    Serial.print(F("Unknown command: '"));
                    Serial.print(serialCmdBuffer);
                    Serial.println(F("'"));
                    printHelp(); // Show help on unknown command
                }
            }
            // Reset buffer index for the next command
            serialCmdBufferIdx = 0;
            serialCmdBuffer[0] = '\0';
        } else if (serialCmdBufferIdx < SERIAL_CMD_BUFFER_SIZE - 1) {
            // Add character to buffer if it's not newline/CR and buffer has space
            serialCmdBuffer[serialCmdBufferIdx++] = receivedChar;
        }
        // Ignore characters if buffer is full until newline/CR is received
    }


    // Add delay or other logic if needed for stability
    // delay(10);
}
// ───────────────────────────────────────────────────────────────────────────

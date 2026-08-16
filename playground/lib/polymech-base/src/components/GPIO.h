#ifndef GPIO_H
#define GPIO_H
// #include <map>
#include <cstdint>
#include <vector>
#include <climits>

#include <ArduinoLog.h>
#include <Component.h>
#include <Bridge.h>
#include <enums.h>
#include <modbus/ModbusTypes.h>
#include <modbus/ModbusTCP.h>
#include <ArduinoJson.h>
// -----------------------------------------------------------------------------
//  GPIO pin number enumeration (unchanged)
// -----------------------------------------------------------------------------
enum E_GPIO_Pin
{
    E_GPIO_0 = 0,
    E_GPIO_1 = 1,
    E_GPIO_2 = 2,
    E_GPIO_3 = 3,
    E_GPIO_4 = 4,
    E_GPIO_5 = 5,
    E_GPIO_6 = 6,
    E_GPIO_7 = 7,
    E_GPIO_8 = 8,
    E_GPIO_9 = 9,
    E_GPIO_10 = 10,
    E_GPIO_11 = 11,
    E_GPIO_12 = 12,
    E_GPIO_13 = 13,
    E_GPIO_14 = 14,
    E_GPIO_15 = 15,
    E_GPIO_16 = 16,
    E_GPIO_17 = 17,
    E_GPIO_18 = 18,
    E_GPIO_19 = 19,
    E_GPIO_20 = 20,
    E_GPIO_21 = 21,
    // Skipping 22‑34 – not available
    E_GPIO_35 = 35,
    E_GPIO_36 = 36,
    E_GPIO_37 = 37,
    E_GPIO_38 = 38,
    E_GPIO_39 = 39,
    E_GPIO_40 = 40,
    E_GPIO_41 = 41,
    E_GPIO_42 = 42,
    E_GPIO_43 = 43,
    E_GPIO_44 = 44,
    E_GPIO_45 = 45,
    E_GPIO_46 = 46,
    E_GPIO_47 = 47,
    E_GPIO_48 = 48
};

// -----------------------------------------------------------------------------
//  MB_GPIO pin types / modes (unchanged)
// -----------------------------------------------------------------------------
enum E_GPIO_Type
{
    E_GPIO_TYPE_UNKNOWN,
    E_GPIO_TYPE_INPUT,
    E_GPIO_TYPE_OUTPUT,
    E_GPIO_TYPE_INPUT_PULLUP,
    E_GPIO_TYPE_INPUT_PULLDOWN,
    E_GPIO_TYPE_OUTPUT_OPEN_DRAIN,
    E_GPIO_TYPE_ANALOG_INPUT, // ADC
    E_GPIO_TYPE_TOUCH         // capacitive touch
};

// -----------------------------------------------------------------------------
//  Configuration for a single managed MB_GPIO pin – inherits Modbus meta‑data and
//  adds timing & caching so we can throttle operations and skip duplicates.
// -----------------------------------------------------------------------------
struct GPIO_PinConfig
{
    // MB_Registers fields copied here
    ushort startAddress = 0xFFFF; // Use 0xFFFF to indicate invalid/not set
    ushort count = 1;             // Assume 1 register per pin
    ushort slaveId = 0;           // Usually 0 for TCP/IP direct
    E_FN_CODE type = E_FN_CODE::FN_NONE;
    E_ModbusAccess access = MB_ACCESS_NONE;
    ushort componentId = 0; // Will be overridden by owner
    const char *name = nullptr;
    const char *group = nullptr;
    ComponentFnPtr writeCallbackFn = nullptr;

    // GPIO specific fields
    E_GPIO_Pin pinNumber = E_GPIO_0;
    E_GPIO_Type pinType = E_GPIO_TYPE_UNKNOWN;

    // Runtime helpers
    uint32_t opIntervalMs = 100;
    int cachedValue = INT_MIN;
    unsigned long lastOpTimestamp = 0;

    // Constructor initializes all members
    GPIO_PinConfig(E_GPIO_Pin pNum = E_GPIO_0,
                   E_GPIO_Type pType = E_GPIO_TYPE_UNKNOWN,
                   ushort addr = 0xFFFF,
                   E_FN_CODE fn = E_FN_CODE::FN_NONE,
                   E_ModbusAccess acc = MB_ACCESS_NONE,
                   uint32_t intervalMs = 100,
                   const char *regName = nullptr,
                   const char *regGroup = nullptr,
                   ComponentFnPtr cb = nullptr)
        : startAddress(addr),
          count(1),
          slaveId(0),
          type(fn),
          access(acc),
          // componentId(0), // Let owner set it
          name(regName),
          group(regGroup),
          writeCallbackFn(cb),
          pinNumber(pNum),
          pinType(pType),
          opIntervalMs(intervalMs),
          cachedValue(INT_MIN),
          lastOpTimestamp(0) // Default initialized
    {
    }
};

// -----------------------------------------------------------------------------
//  Capability bit‑mask (unchanged – table at end)
// -----------------------------------------------------------------------------
// Define capabilities using a bitmask
enum E_GPIO_Capability : uint32_t
{
    CAP_NONE = 0,
    CAP_RTC = 1 << 0,
    CAP_ADC1 = 1 << 1,
    CAP_ADC2 = 1 << 2,
    CAP_TOUCH = 1 << 3,
    CAP_JTAG_TCK = 1 << 4, // GPIO4 (TCK), GPIO39 (MTCK)
    CAP_JTAG_TDI = 1 << 5, // GPIO3 (TDI?), GPIO41 (MTDI)
    CAP_JTAG_TDO = 1 << 6, // GPIO3 (TDO?), GPIO40 (MTDO)
    CAP_JTAG_TMS = 1 << 7, // GPIO42 (MTMS)
    CAP_FSPI_CS0 = 1 << 8,
    CAP_FSPI_CLK = 1 << 9,
    CAP_FSPI_MISO = 1 << 10,   // Q/IO1
    CAP_FSPI_MOSI = 1 << 11,   // D/IO0
    CAP_FSPI_HD = 1 << 12,     // IO3 / GPIO9
    CAP_FSPI_WP = 1 << 13,     // IO2 / GPIO10, GPIO38
    CAP_FSPI_IO4 = 1 << 14,    // GPIO14
    CAP_FSPI_IO5 = 1 << 15,    // GPIO13
    CAP_FSPI_IO6 = 1 << 16,    // GPIO11, GPIO37
    CAP_FSPI_IO7 = 1 << 17,    // GPIO12, GPIO36
    CAP_UART0_TX = 1 << 18,    // GPIO43
    CAP_UART0_RX = 1 << 19,    // GPIO44
    CAP_UART1_TX = 1 << 20,    // GPIO18
    CAP_UART1_RX = 1 << 21,    // GPIO17
    CAP_UART2_TX = 1 << 22,    // GPIO20
    CAP_UART2_RX = 1 << 23,    // GPIO19
    CAP_I2C0_SDA = 1 << 24,    // GPIO8
    CAP_I2C0_SCL = 1 << 25,    // GPIO9
    CAP_CLK_OUT1 = 1 << 26,    // GPIO2, GPIO20, GPIO41
    CAP_CLK_OUT2 = 1 << 27,    // GPIO1, GPIO19, GPIO40
    CAP_CLK_OUT3 = 1 << 28,    // GPIO0, GPIO39
    CAP_USB_D_PLUS = 1 << 29,  // GPIO20
    CAP_USB_D_MINUS = 1 << 30, // GPIO19
    CAP_LED_BUILTIN = 1U << 31 // GPIO38
    // Note: RGB LED (GPIO48), XTAL, Strapping pins are not included in this bitmask
};
// -----------------------------------------------------------------------------
//  MB_GPIO component – manages a *group* of pins
// -----------------------------------------------------------------------------
class MB_GPIO : public Component
{
private:
    // Internal copy of the pin configurations
    std::vector<GPIO_PinConfig> pinConfigs;

    // Internal helper to find config by Modbus address
    const GPIO_PinConfig *findConfigByModbusAddress(ushort address) const
    {
        // Iterate internal vector
        for (const auto &cfg : pinConfigs) // Use direct member access
        {
            if (cfg.startAddress == address)
                return &cfg;
        }
        return nullptr;
    }

    // Internal helper to find *non-const* config (needed for write)
    GPIO_PinConfig *findConfigByModbusAddress(ushort address)
    {
        // Needs to iterate over the internal vector non-constantly
        for (auto &cfg : pinConfigs) // Use direct member access
        {
            if (cfg.startAddress == address)
                return &cfg;
        }
        return nullptr;
    }

    // -------------------------------------------------------------------------
    // Raw read helper
    // -------------------------------------------------------------------------
    int readPinInternal(E_GPIO_Pin pin, E_GPIO_Type type) const
    {
        return (type == E_GPIO_TYPE_ANALOG_INPUT) ? analogRead(static_cast<int>(pin))
                                                  : digitalRead(static_cast<int>(pin));
    }

    // -------------------------------------------------------------------------
    // Raw *and throttled* write helper – skips duplicates & enforces interval.
    // Returns E_OK even when operation skipped (so Modbus sees no error).
    // -------------------------------------------------------------------------
    short writePinInternal(GPIO_PinConfig &cfg, int value)
    {
        if (!(cfg.pinType == E_GPIO_TYPE_OUTPUT || cfg.pinType == E_GPIO_TYPE_OUTPUT_OPEN_DRAIN))
        {
            Log.warningln("MB_GPIO::writePinInternal - Pin %d not output type (%d)", (int)cfg.pinNumber, (int)cfg.pinType);
            return E_INVALID_PARAMETER;
        }

        unsigned long now = millis();
        if (value == cfg.cachedValue)
        {
            // duplicate value → skip silently
            return E_OK;
        }
        if (now - cfg.lastOpTimestamp < cfg.opIntervalMs)
        {
            // too soon → skip
            return E_OK;
        }

        digitalWrite(static_cast<int>(cfg.pinNumber), value ? HIGH : LOW);
        cfg.cachedValue = value;
        cfg.lastOpTimestamp = now;
        return E_OK;
    }

    // -------------------------------------------------------------------------
    //  Modbus block storage
    // -------------------------------------------------------------------------
    MB_Registers *mbRegisterBlocksData = nullptr;
    int mbRegisterBlocksCount = 0;
    ModbusBlockView mbBlockView; // {data,count}

public:
    // static const std::map<E_GPIO_Pin, uint32_t> pinCapabilities;

    // Constructor takes std::vector reference and copies the data
    MB_GPIO(Component *owner,
            short _id,
            const std::vector<GPIO_PinConfig> &configs) // Accept const reference
        : Component("GPIO_Group", _id, Component::COMPONENT_DEFAULT, owner),
          pinConfigs(configs) // Initialize by copying
    {
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
        allocateAndPopulateMbBlocks(); // Uses the internal pinConfigs copy
        Log.verboseln("MB_GPIO %d constructed - Copied %d configs, %d Modbus blocks prepared", id, (int)pinConfigs.size(), mbRegisterBlocksCount);
    }
    ~MB_GPIO() override { delete[] mbRegisterBlocksData; } // Destructor unchanged

    // Component API ------------------------------------------------------------
    short setup() override;
    short loop() override;
    short info(short flags = 0, short val = 0) override;
    short debug() override { return info(); }
    short serial_register(Bridge *bridge) override;
    short load(const JsonObject &config); // Removed override, base Component doesn't have virtual load

    // Modbus TCP ---------------------------------------------------------------
    short mb_tcp_read(short address) override;
    short mb_tcp_read(MB_Registers *reg) override { return reg ? mb_tcp_read(reg->startAddress) : 0; }
    short mb_tcp_write(short address, short networkValue) override;
    short mb_tcp_write(MB_Registers *reg, short networkValue) override { return reg ? mb_tcp_write(reg->startAddress, networkValue) : E_INVALID_PARAMETER; }
    ModbusBlockView *mb_tcp_blocks() const override { return const_cast<ModbusBlockView *>(&mbBlockView); }
    void mb_tcp_register(ModbusTCP *manager) override;

private:
    void populateMbRegisterBlocks();    // Removed component_id parameter
    void allocateAndPopulateMbBlocks(); // New helper combining allocation & population
    short configurePinModeInternal(E_GPIO_Pin pin, E_GPIO_Type type);
};

// -----------------------------------------------------------------------------
//  Helper to manage Modbus block allocation and population
// -----------------------------------------------------------------------------
inline void MB_GPIO::allocateAndPopulateMbBlocks()
{
    // Clear previous allocation if any
    delete[] mbRegisterBlocksData;
    mbRegisterBlocksData = nullptr;
    mbRegisterBlocksCount = 0;
    mbBlockView = {nullptr, 0};

    if (pinConfigs.empty()) { 
        clearNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
        return; 
    }

    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

    int mappedCount = 0;
    for (const auto &cfg : pinConfigs) 
    {
        if (cfg.startAddress != 0xFFFF)
            ++mappedCount;
    }
    if (mappedCount > 0)
    {
        mbRegisterBlocksCount = mappedCount;
        mbRegisterBlocksData = new MB_Registers[mappedCount];
        if (!mbRegisterBlocksData)
        {
            Log.fatalln("MB_GPIO %d: Failed to allocate Modbus blocks", id);
            mbRegisterBlocksCount = 0;
            clearNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
        }
        else
        {
            populateMbRegisterBlocks();
            mbBlockView = {mbRegisterBlocksData, mbRegisterBlocksCount};
        }
    } else {
         clearNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    }
}

// -----------------------------------------------------------------------------
//  populateMbRegisterBlocks – uses internal pinConfigs Vector
// -----------------------------------------------------------------------------
inline void MB_GPIO::populateMbRegisterBlocks()
{
    if (!mbRegisterBlocksData || mbRegisterBlocksCount == 0)
        return;
    int idx = 0;
    for (const auto &cfg : pinConfigs) // Use direct member access
    {
        // Log.infoln("MB_GPIO %d: Populating block %d, Address %u, Count %u, Type %u, Access %u", id, idx, cfg.startAddress, cfg.count, cfg.type, cfg.access);
        if (cfg.startAddress != 0xFFFF && idx < mbRegisterBlocksCount)
        {
            mbRegisterBlocksData[idx] = MB_Registers(
                cfg.startAddress, cfg.count, cfg.type, cfg.access,
                id, // Set the owner MB_GPIO component's ID
                cfg.slaveId, cfg.name, cfg.group, cfg.writeCallbackFn
                // No associatedPin in MB_Registers constructor
            );
            ++idx;
        }
    }
     if (idx != mbRegisterBlocksCount) {
        Log.warningln("MB_GPIO %d: Mismatch in Modbus block count. Expected %d, populated %d.", id, mbRegisterBlocksCount, idx);
    }
    mbRegisterBlocksCount = idx; // Use actual populated count
    mbBlockView.count = idx;
}

// -----------------------------------------------------------------------------
// Load method implementation - Can now modify internal pinConfigs
// -----------------------------------------------------------------------------
inline short MB_GPIO::load(const JsonObject &config)
{
    Log.verboseln("MB_GPIO::load - ID: %d", id);

    JsonArray pinsArray = config["pins"].as<JsonArray>();
    if (!pinsArray)
    {
        Log.warningln("MB_GPIO %d: JSON config missing 'pins' array or it's not an array. Clearing existing config.", id);
        pinConfigs.clear(); // Clear internal vector
    }
    else
    {
        Log.infoln("MB_GPIO %d: Loading %zu pin configurations from JSON...", id, pinsArray.size());
        pinConfigs.clear(); // Clear internal vector before loading new ones
        pinConfigs.reserve(pinsArray.size()); // Reserve capacity

        for (JsonObject pinConfigJson : pinsArray)
        {
            // --- Parse individual pin config ---
            E_GPIO_Pin pinNum = (E_GPIO_Pin)pinConfigJson["pinNumber"].as<int>();  // Required
            E_GPIO_Type pinType = (E_GPIO_Type)pinConfigJson["pinType"].as<int>(); // Required

            ushort addr = pinConfigJson["modbusAddress"] | 0xFFFF; // Use default invalid address
            E_FN_CODE fn = (E_FN_CODE)(pinConfigJson["modbusFunction"] | (int)E_FN_CODE::FN_NONE);
            E_ModbusAccess acc = (E_ModbusAccess)(pinConfigJson["modbusAccess"] | (int)MB_ACCESS_NONE);
            uint32_t interval = pinConfigJson["opIntervalMs"] | 100; // Default interval
            const char *name = pinConfigJson["name"] | nullptr;
            const char *group = pinConfigJson["group"] | nullptr;

            // Basic validation (example)
            if (pinNum < E_GPIO_0 || pinNum > E_GPIO_48)
            {
                Log.errorln("MB_GPIO %d: Invalid pinNumber %d in config.", id, (int)pinNum);
                continue; // Skip this pin config
            }

            pinConfigs.emplace_back(pinNum, pinType, addr, fn, acc, interval, name, group); // Use emplace_back
        }
    }

    allocateAndPopulateMbBlocks(); // Re-create Modbus blocks from the new internal pinConfigs
    setup();                       // Re-apply pin modes based on the new internal pinConfigs

    Log.infoln("MB_GPIO %d: Load complete. %d pins configured, %d Modbus blocks prepared.", id, pinConfigs.size(), mbRegisterBlocksCount);
    return E_OK;
}

// -----------------------------------------------------------------------------
//  setup / loop ---------------------------------------------------------------
// -----------------------------------------------------------------------------
inline short MB_GPIO::setup()
{
    for (const auto &cfg : pinConfigs) // Use direct member access
    {
        configurePinModeInternal(cfg.pinNumber, cfg.pinType);
    }
    return E_OK;
}

inline short MB_GPIO::loop()
{
    Component::loop();
    return E_OK;
}

inline short MB_GPIO::info(short flags, short val)
{
    Log.verboseln("MB_GPIO::info – ID %d, pins %d, NetCaps 0x%04X", id, (int)pinConfigs.size(), nFlags); // Use direct access
    for (size_t i = 0; i < pinConfigs.size(); ++i) // Use direct access
    {
        const auto &cfg = pinConfigs[i]; // Use direct access
        Log.verboseln(" [%d] Pin %d, Type %d, MB %u, Access %d, Intv %u, Last %ld, Cached %d",
                      (int)i, (int)cfg.pinNumber, (int)cfg.pinType, cfg.startAddress, (int)cfg.access,
                      cfg.opIntervalMs, cfg.lastOpTimestamp, cfg.cachedValue);
    }
    for(int i = 0; i < mbRegisterBlocksCount; ++i) {
       Log.verboseln("  - Block %d: Addr=%u, Cnt=%u, Type=%u, Acc=%u, Name=%s", i, mbRegisterBlocksData[i].startAddress, mbRegisterBlocksData[i].count, mbRegisterBlocksData[i].type, mbRegisterBlocksData[i].access, mbRegisterBlocksData[i].name ? mbRegisterBlocksData[i].name : "null");
    }
    return E_OK;
}

inline short MB_GPIO::serial_register(Bridge *bridge)
{
    Component::serial_register(bridge);
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&MB_GPIO::info);
    return E_OK;
}

// -----------------------------------------------------------------------------
//  Modbus read / write ---------------------------------------------------------
// -----------------------------------------------------------------------------
inline short MB_GPIO::mb_tcp_read(short address)
{
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS))
        return E_INVALID_PARAMETER;

    const auto *cfg = findConfigByModbusAddress(address); // Uses internal pinConfigs
    if (!cfg){
         return 0;
    }
    if (cfg->access == MB_ACCESS_READ_ONLY || cfg->access == MB_ACCESS_READ_WRITE)
    {
        return readPinInternal(cfg->pinNumber, cfg->pinType);
    }
    return E_OK;
}

inline short MB_GPIO::mb_tcp_write(short address, short networkValue)
{
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS))
        return E_INVALID_PARAMETER;

    GPIO_PinConfig *cfg = findConfigByModbusAddress(address);
    if (!cfg)
        return E_INVALID_PARAMETER;

    if (cfg->access == MB_ACCESS_WRITE_ONLY || cfg->access == MB_ACCESS_READ_WRITE)
    {
        return writePinInternal(*cfg, networkValue); // Pass the non-const config
    }
    Log.warningln("MB_GPIO %d: MB Write Addr %u (Pin %d) - Access denied (%d)", id, cfg->startAddress, static_cast<int>(cfg->pinNumber), static_cast<int>(cfg->access));
    return E_INVALID_PARAMETER;
}

inline void MB_GPIO::mb_tcp_register(ModbusTCP *manager) const
{
    if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS) || !manager)
        return;
    if (!mbRegisterBlocksData || mbRegisterBlocksCount == 0)
        return;
    Component *self = const_cast<MB_GPIO *>(this);
    for (int i = 0; i < mbRegisterBlocksCount; ++i)
    {
        manager->registerModbus(self, mbRegisterBlocksData[i]);
    }
}

// -----------------------------------------------------------------------------
//  Helper to set pin‑mode ------------------------------------------------------
// -----------------------------------------------------------------------------
inline short MB_GPIO::configurePinModeInternal(E_GPIO_Pin pin, E_GPIO_Type type)
{
    switch (type)
    {
    case E_GPIO_TYPE_INPUT:
        pinMode((int)pin, INPUT);
        return E_OK;
    case E_GPIO_TYPE_OUTPUT:
        pinMode((int)pin, OUTPUT);
        return E_OK;
    case E_GPIO_TYPE_INPUT_PULLUP:
        pinMode((int)pin, INPUT_PULLUP);
        return E_OK;
    case E_GPIO_TYPE_OUTPUT_OPEN_DRAIN:
        pinMode((int)pin, OUTPUT_OPEN_DRAIN);
        return E_OK;
    case E_GPIO_TYPE_INPUT_PULLDOWN:
        return E_NOT_IMPLEMENTED;
    case E_GPIO_TYPE_ANALOG_INPUT:
    case E_GPIO_TYPE_TOUCH:
        return E_OK;
    default:
        Log.errorln("Unknown MB_GPIO type %d for pin %d", (int)type, (int)pin);
        return E_INVALID_PARAMETER;
    }
}
/*
// -----------------------------------------------------------------------------
//  Static map of pin capabilities (unchanged – omitted here for brevity)
// -----------------------------------------------------------------------------
// Define static capabilities map (if still needed)
inline const std::map<E_GPIO_Pin, uint32_t> MB_GPIO::pinCapabilities = {
    {E_GPIO_0, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_CLK_OUT3},
    {E_GPIO_1, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_CLK_OUT2},
    {E_GPIO_2, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_CLK_OUT1},
    {E_GPIO_3, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_JTAG_TDI | CAP_JTAG_TDO}, // Pinout shows TDO, schematic might show TDI
    {E_GPIO_4, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_JTAG_TCK},
    {E_GPIO_5, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_FSPI_CS0},
    {E_GPIO_6, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_FSPI_CLK},
    {E_GPIO_7, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_FSPI_MISO},
    {E_GPIO_8, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_I2C0_SDA},
    {E_GPIO_9, CAP_RTC | CAP_ADC1 | CAP_TOUCH | CAP_I2C0_SCL | CAP_FSPI_HD},
    {E_GPIO_10, CAP_RTC | CAP_ADC2 | CAP_TOUCH | CAP_FSPI_WP},
    {E_GPIO_11, CAP_RTC | CAP_ADC2 | CAP_TOUCH | CAP_FSPI_IO6},
    {E_GPIO_12, CAP_RTC | CAP_ADC2 | CAP_TOUCH | CAP_FSPI_IO7},
    {E_GPIO_13, CAP_RTC | CAP_ADC2 | CAP_TOUCH | CAP_FSPI_IO5},
    {E_GPIO_14, CAP_RTC | CAP_ADC2 | CAP_TOUCH | CAP_FSPI_IO4},
    {E_GPIO_15, CAP_RTC | CAP_ADC2 | CAP_UART0_RX},                                  // TOUCH? No. ADC2_5. XTAL_P. RTS0.
    {E_GPIO_16, CAP_RTC | CAP_ADC2 | CAP_UART0_TX},                                  // TOUCH? No. ADC2_6. XTAL_N. CTS0.
    {E_GPIO_17, CAP_RTC | CAP_ADC2 | CAP_UART1_RX},                                  // ADC2_7? No ADC2_6. RXD1
    {E_GPIO_18, CAP_RTC | CAP_ADC2 | CAP_UART1_TX},                                  // ADC2_7. TXD1
    {E_GPIO_19, CAP_RTC | CAP_ADC2 | CAP_USB_D_MINUS | CAP_CLK_OUT2 | CAP_UART2_RX}, // ADC2_8. RTS1.
    {E_GPIO_20, CAP_RTC | CAP_ADC2 | CAP_USB_D_PLUS | CAP_CLK_OUT1 | CAP_UART2_TX},  // ADC2_9. CTS1.
    {E_GPIO_21, CAP_RTC},
    {E_GPIO_35, CAP_RTC | CAP_ADC1},                // ADC1_7.
    {E_GPIO_36, CAP_RTC | CAP_ADC1 | CAP_FSPI_IO7}, // ADC1_8.
    {E_GPIO_37, CAP_RTC | CAP_ADC1 | CAP_FSPI_IO6}, // ADC1_9.
    {E_GPIO_38, CAP_RTC | CAP_FSPI_WP | CAP_LED_BUILTIN},
    {E_GPIO_39, CAP_RTC | CAP_CLK_OUT3 | CAP_JTAG_TCK}, // MTCK
    {E_GPIO_40, CAP_RTC | CAP_CLK_OUT2 | CAP_JTAG_TDO}, // MTDO
    {E_GPIO_41, CAP_RTC | CAP_CLK_OUT1 | CAP_JTAG_TDI}, // MTDI
    {E_GPIO_42, CAP_RTC | CAP_JTAG_TMS},                // MTMS
    {E_GPIO_43, CAP_UART0_TX},
    {E_GPIO_44, CAP_UART0_RX},
    {E_GPIO_45, CAP_NONE}, // VSPI, Strapping
    {E_GPIO_46, CAP_NONE}, // Strapping
    {E_GPIO_47, CAP_NONE}, // SPICLK_P
    {E_GPIO_48, CAP_NONE}  // SPICLK_N, RGB LED
};
*/
#endif // GPIO_H

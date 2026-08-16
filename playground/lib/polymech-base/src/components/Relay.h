#ifndef RELAY_H
#define RELAY_H

#include <ArduinoLog.h>
#include <App.h>
#include <Component.h>
#include <enums.h>
#include "config.h"
#include <modbus/Modbus.h>
#include <modbus/ModbusTCP.h>

#include "config-modbus.h" // application-specific modbus configuration

class Bridge;
class Relay : public Component
{
private:                       // Keep address private, provide via getModbusInfo
    const short modbusAddress; // Store Modbus address internally
    MB_Registers m_modbus_block[2];
    // m_modbus_view needs to be mutable to be returned as ModbusBlockView* from a const method.
    mutable ModbusBlockView m_modbus_view;

    // for blinking
    unsigned long onOffFrequency; // in seconds
    unsigned long lastToggleTime;

public:
    Relay(
        Component *owner,
        short _pin,
        short _id,
        short _modbusAddress)
        : Component("Relay", _id, Component::COMPONENT_DEFAULT, owner),
          pin(_pin),
          modbusAddress(_modbusAddress),
          value(false),
          onOffFrequency(0),
          lastToggleTime(0)
    {
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

        // Initialize instance-specific Modbus block.
        // The modbusAddress is the actual start address for the Relay's single coil.
        // So, the offset passed to the macro is 0.
        m_modbus_block[0] = INIT_MODBUS_BLOCK_TCP(
            this->modbusAddress,      // Base address for this component's block
            0,                        // Offset for this specific register
            E_FN_CODE::FN_WRITE_COIL, // Function code
            MB_ACCESS_READ_WRITE,     // Access type
            "Relay",                  // Name
            nullptr                   // Group (nullptr if not applicable)
        );

        m_modbus_block[1] = INIT_MODBUS_BLOCK_TCP(
            this->modbusAddress,            // Base address for this component's block
            1,                              // Offset for the frequency register
            E_FN_CODE::FN_WRITE_HOLD_REGISTER,   // Function code
            MB_ACCESS_READ_WRITE,           // Access type
            "Relay On/Off Freq (secs)",     // Name
            nullptr                         // Group (nullptr if not applicable)
        );

        // Initialize the view to point to this instance-specific block
        m_modbus_view.data = m_modbus_block; // Point to the array of blocks
        m_modbus_view.count = 2;
    }

    short info(short flags = 0, short val = 0) override
    {
        Log.verboseln("Relay::info - ID: %d, Pin: %d, Modbus Addr: %d, Value: %d, Freq: %lu, NetCaps: %d",
                      id, pin, modbusAddress, value, onOffFrequency, nFlags);
        return E_OK;
    }

    short debug() override
    {
        return info(0, 0);
    }

    short setup() override
    {
        Component::setup(); // Call base class setup (important if it does network registration)
        pinMode(pin, OUTPUT);
        digitalWrite(pin, value); // Ensure pin state matches initial value
        Log.verboseln("Relay::setup - ID %d, Pin %d, Initial Value: %d, Modbus Addr: %d", id, pin, value, modbusAddress);
        return E_OK;
    }

    short setValue(bool newValue)
    {
        if (value != newValue)
        {
            value = newValue;
            digitalWrite(pin, newValue ? HIGH : LOW);
            notifyStateChange();
        }
        return E_OK;
    }

    short setValueCmd(short arg1, short arg2)
    {
        onOffFrequency = 0; // manual override
        return setValue(arg1 > 0);
    }

    bool getValue() const
    {
        return value;
    }

    /**
     * @brief Handles writes coming from the network (e.g., Modbus write coil/register).
     * @param reg The Modbus register being written to.
     * @param networkValue The value received from the network.
     * @return E_OK if the address matches and the value is set, E_INVALID_PARAMETER otherwise.
     */
    short mb_tcp_write(MB_Registers *reg, short networkValue) override
    {
        return mb_tcp_write(reg->startAddress, networkValue);
    }
    /**
     * @brief Handles writes coming from the network (e.g., Modbus write coil/register).
     * @param address The Modbus address being written to (should match component's address).
     * @param networkValue The value received from the network.
     * @return E_OK if the address matches and the value is set, E_INVALID_PARAMETER otherwise.
     */
    short mb_tcp_write(short address, short networkValue) override
    {
        if (address == modbusAddress) // Use internal member
        {
            onOffFrequency = 0; // Manual control stops blinking
            return setValue(networkValue > 0);
        }
        else if (address == modbusAddress + 1)
        {
            onOffFrequency = networkValue;
            if (onOffFrequency > 0)
            {
                lastToggleTime = millis();
            }
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }

    /**
     * @brief Handles reads requests from the network (e.g., Modbus read coil/register).
     * @param address The Modbus address being read (should match component's address).
     * @return The current state (1 for ON, 0 for OFF) if the address matches, 0 otherwise.
     */
    short mb_tcp_read(short address) override
    {
        if (address == modbusAddress) // Use internal member
        {
            return value ? 1 : 0;
        }
        else if (address == modbusAddress + 1)
        {
            return onOffFrequency;
        }
        return 0; // Default for mismatched addresses
    }

    short mb_tcp_read(MB_Registers *reg) override
    {
        // Log.traceln(F("Relay::mb_tcp_read (Reg Context) - TCP Addr: %d, Type: %d"), reg->startAddress, reg->type);
        return mb_tcp_read(reg->startAddress);
    }

    void mb_tcp_register(ModbusTCP *manager) override
    {
        ModbusBlockView *blocksView = mb_tcp_blocks();
        Component *thiz = const_cast<Relay *>(this);
        for (int i = 0; i < blocksView->count; ++i)
        {
            MB_Registers info = blocksView->data[i];
            manager->registerModbus(thiz, info);
        }
    }

    ModbusBlockView *mb_tcp_blocks() const override
    {
        // Return the instance-specific Modbus block view
        return &m_modbus_view;
    }

    short serial_register(Bridge *bridge) override
    {
        Component::serial_register(bridge);
        bridge->registerMemberFunction(id, this, C_STR("setValue"), (ComponentFnPtr)&Relay::setValueCmd);
        bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&Relay::info);
        return E_OK;
    }

    short loop() override
    {
        Component::loop();
        if (onOffFrequency > 0)
        {
            unsigned long currentMillis = millis();
            if (currentMillis - lastToggleTime >= onOffFrequency * 1000)
            {
                lastToggleTime = currentMillis;
                setValue(!value);
            }
        }
        return E_OK;
    }

    // --- Member Variables ---
    const short pin;
    bool value; 
};

#endif

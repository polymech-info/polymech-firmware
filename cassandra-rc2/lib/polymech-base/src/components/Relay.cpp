#include "components/Relay.h"
#include "modbus/ModbusTypes.h"
#include <Bridge.h>

Relay::Relay(
    Component *owner,
    short _pin,
    short _id,
    short _modbusAddress)
    : NetworkComponent(_modbusAddress, "Relay", _id, Component::COMPONENT_DEFAULT, owner),
      pin(_pin),
      lastToggleTime(0),
      m_state(this, _id, "Relay State"),
      m_frequency(this, _id, "Relay Frequency")
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
}
short Relay::setup()
{
    NetworkComponent::setup();
    pinMode(pin, OUTPUT);
    digitalWrite(pin, m_state.getValue() ? HIGH : LOW);
    
    const uint16_t baseAddr = mb_tcp_base_address();

    m_state.initNotify(false, true, NetworkValue_ThresholdMode::DIFFERENCE);
    m_state.initModbus(baseAddr + MB_OFS_COIL_STATE, 1, this->id, this->slaveId, FN_WRITE_COIL, m_state.name.c_str(), this->name.c_str());
    registerBlock(m_state.getRegisterInfo());

    m_frequency.initNotify(0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_frequency.initModbus(baseAddr + MB_OFS_HR_FREQUENCY, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, m_frequency.name.c_str(), this->name.c_str());
    registerBlock(m_frequency.getRegisterInfo());

    return E_OK;
}

short Relay::setValue(bool newValue)
{
    if (m_state.update(newValue))
    {
        digitalWrite(pin, newValue ? HIGH : LOW);
        notifyStateChange();
    }
    return E_OK;
}

short Relay::setValueCmd(short arg1, short arg2)
{
    m_frequency.update(0); // manual override
    return setValue(arg1 > 0);
}

bool Relay::getValue() const
{
    return m_state.getValue();
}

short Relay::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    short result = NetworkComponent::mb_tcp_write(reg, networkValue);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }

    uint16_t address = reg->startAddress;

    if (address == (_baseAddress + MB_OFS_COIL_STATE)) 
    {
        m_frequency.update(0); // Manual control stops blinking
        return setValue(networkValue > 0);
    }
    else if (address == (_baseAddress + MB_OFS_HR_FREQUENCY))
    {
        m_frequency.update(networkValue);
        if (m_frequency.getValue() > 0)
        {
            lastToggleTime = millis();
        }
        return E_OK;
    }
    return E_INVALID_PARAMETER;
}

short Relay::mb_tcp_read(MB_Registers *reg)
{
    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }

    uint16_t address = reg->startAddress;
    
    if (address == (_baseAddress + MB_OFS_COIL_STATE))
    {
        return m_state.getValue() ? 1 : 0;
    }
    else if (address == (_baseAddress + MB_OFS_HR_FREQUENCY))
    {
        return m_frequency.getValue();
    }
    return 0;
}

short Relay::loop()
{
    Component::loop();
    if (m_frequency.getValue() > 0)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastToggleTime >= m_frequency.getValue() * 1000)
        {
            lastToggleTime = currentMillis;
            setValue(!m_state.getValue());
        }
    }
    return E_OK;
} 
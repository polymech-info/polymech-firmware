#include "components/Solenoid.h"
#include "modbus/ModbusTypes.h"
#include <Bridge.h>
#include <StringUtils.h>


#define MAX_STRING_LENGTH 100

Solenoid::Solenoid(
    Component *owner,
    short _pin,
    short _id,
    short _modbusAddress)
    : NetworkComponent(_modbusAddress, "Solenoid", _id, Component::COMPONENT_DEFAULT, owner),
      pin(_pin),
      lastOnTime(0),
      lastOffTime(millis()),
      lastOnDurationS(0),
      lastOffDurationS(0),
      m_state(this, _id, "State"),
      _activation_count(0)
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

    char buffer[32];
    FORMAT_BUFFER(buffer, "Solenoid-%d", _id);
    this->name = buffer;
}

short Solenoid::setup()
{
    NetworkComponent::setup();
    pinMode(pin, OUTPUT);
    digitalWrite(pin, m_state.getValue() ? HIGH : LOW);
    const uint16_t baseAddr = mb_tcp_base_address();    
    m_state.initNotify(false, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_state.initModbus(baseAddr + MB_OFS_COIL_STATE, 1, this->id, this->slaveId, FN_WRITE_COIL, m_state.name.c_str(), this->name.c_str());
    registerBlock(m_state.getRegisterInfo());

    return E_OK;
}

short Solenoid::setValue(bool newValue)
{
    if(!enabled()) {
        return false;
    }
    digitalWrite(pin, newValue ? HIGH : LOW);    
    if (newValue) { // Turned ON
        lastOnTime = millis();
        lastOffDurationS = (lastOnTime - lastOffTime) / 1000;
        _activation_count++;
    } else { // Turned OFF
        lastOffTime = millis();
        lastOnDurationS = (lastOffTime - lastOnTime) / 1000;
    }
    m_state.update(newValue);
    return E_OK;
}

short Solenoid::setValueCmd(short arg1, short arg2)
{
    return setValue(arg1 > 0);
}

bool Solenoid::getValue() const
{
    return m_state.getValue();
}

uint32_t Solenoid::getActivationCount() const {
    return _activation_count;
}

void Solenoid::resetActivationCount() {
    _activation_count = 0;
}

uint32_t Solenoid::getCurrentOnDurationMs() const
{
    if (m_state.getValue()) {
        return millis() - lastOnTime;
    }
    return 0;
}

short Solenoid::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    short result = NetworkComponent::mb_tcp_write(reg, networkValue);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }
    
    uint16_t address = reg->startAddress;
    if (address == (_baseAddress + MB_OFS_COIL_STATE))
    {
        return setValue(networkValue > 0);
    }
    return E_INVALID_PARAMETER;
}

short Solenoid::mb_tcp_read(MB_Registers *reg)
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
    return 0;
}

short Solenoid::loop()
{
    Component::loop();
    if (!enabled() && m_state.getValue()) {
        setValue(false);
    }
    return E_OK;
} 
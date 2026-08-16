#include "components/Joystick.h"
#include <Bridge.h>

// Define F() macro wrapper for non-AVR architectures if needed
#if defined(ARDUINO_ARCH_AVR)
  #define JOY_L(FSTR)              F(FSTR)
#else
  #define JOY_L(FSTR)              (FSTR)
#endif

Joystick::Joystick(
    Component *owner,
    ushort _pinUp,
    ushort _pinDown,
    ushort _pinLeft,
    ushort _pinRight, 
    ushort _modbusAddress)
    : NetworkComponent(_modbusAddress, "Joystick", COMPONENT_KEY_JOYSTICK_0, COMPONENT_DEFAULT, owner),
      pinUp(_pinUp),
      pinDown(_pinDown),
      pinLeft(_pinLeft),
      pinRight(_pinRight),
      currentPosition(E_POSITION::CENTER),
      lastPosition(E_POSITION::CENTER),
      m_position(this, this->id, "Position"),
      m_mode(this, this->id, "Mode"),
      m_overridePosition(this, this->id, "Override Position")
{
    _baseAddress = _modbusAddress;
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
}

short Joystick::setup() {
    NetworkComponent::setup();
    pinMode(pinUp, INPUT_PULLUP);
    pinMode(pinDown, INPUT_PULLUP);
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);

    const uint16_t baseAddr = mb_tcp_base_address();
    
    m_position.initNotify(E_POSITION::CENTER, (E_POSITION)1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_position.initModbus(baseAddr + POSITION, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, "Position", this->name.c_str());
    registerBlock(m_position.getRegisterInfo());

    m_mode.initNotify(E_MODE::LOCAL, (E_MODE)1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_mode.initModbus(baseAddr + MODE, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "Mode", this->name.c_str());
    registerBlock(m_mode.getRegisterInfo());

    m_overridePosition.initNotify(E_POSITION::CENTER, (E_POSITION)1, NetworkValue_ThresholdMode::DIFFERENCE);
    m_overridePosition.initModbus(baseAddr + OVERRIDE, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, "Override", this->name.c_str());
    registerBlock(m_overridePosition.getRegisterInfo());
    
    return reset();
}

short Joystick::reset() {
    currentPosition = E_POSITION::CENTER;
    lastPosition = E_POSITION::CENTER;
    m_mode.update(E_MODE::LOCAL);
    m_overridePosition.update(E_POSITION::CENTER);
    positionStartMs = 0;
    lastReadMs = 0;
    proposedPosition = E_POSITION::CENTER;
    confirmCount = 0;
    return E_OK;
}

Joystick::E_POSITION Joystick::readPinsPosition() {
    if (digitalRead(pinUp) == LOW) return E_POSITION::UP;
    if (digitalRead(pinDown) == LOW) return E_POSITION::DOWN;
    if (digitalRead(pinLeft) == LOW) return E_POSITION::LEFT;
    if (digitalRead(pinRight) == LOW) return E_POSITION::RIGHT;
    return E_POSITION::CENTER;
}

short Joystick::loop() {
    Component::loop();

    if (!enabled() || m_mode.getValue() == E_MODE::REMOTE) {
        return E_OK;
    }

    if (now - lastReadMs < READ_INTERVAL_MS) {
        return E_OK;
    }
    lastReadMs = now;
    
    E_POSITION pos = readPinsPosition();

    if (useDebouncing) {
        if (pos == proposedPosition) {
            confirmCount++;
        } else {
            confirmCount = 1;
            proposedPosition = pos;
        }
        
        if (confirmCount >= DEBOUNCE_COUNT) {
            if (currentPosition != proposedPosition) {
                lastPosition = currentPosition;
                currentPosition = proposedPosition;
                positionStartMs = now;
                m_position.update(currentPosition);
                notifyStateChange();
            }
        }
    } else {
        if (currentPosition != pos) {
            lastPosition = currentPosition;
            currentPosition = pos;
            positionStartMs = now;
            m_position.update(currentPosition);
            notifyStateChange();
        }
    }

    return E_OK;
}

short Joystick::info(short val0, short val1) {
    Log.traceln(F("Joystick Info: Position=%d, Mode=%d, Override=%d, HoldingTime=%lums"),
                (ushort)getPosition(), (ushort)m_mode.getValue(), (ushort)m_overridePosition.getValue(), getHoldingTime());
    return E_OK;
}

void Joystick::notifyStateChange() {
    Component::notifyStateChange();
}

short Joystick::mb_tcp_write(MB_Registers *reg, short networkValue) {
    short result = NetworkComponent::mb_tcp_write(reg, networkValue);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }

    uint16_t address = reg->startAddress;

    if (address == (_baseAddress + MODE)) {
        if (networkValue >= (ushort)E_MODE::LOCAL && networkValue <= (ushort)E_MODE::REMOTE) {
            setMode((E_MODE)networkValue);
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    } else if (address == (_baseAddress + OVERRIDE)) {
        if (networkValue >= (ushort)E_POSITION::CENTER && networkValue <= (ushort)E_POSITION::UNKNOWN) {
            setOverridePosition((E_POSITION)networkValue);
            return E_OK;
        }
        return E_INVALID_PARAMETER;
    }
    
    return E_INVALID_PARAMETER;
}

short Joystick::mb_tcp_read(MB_Registers *reg) {
    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }

    uint16_t address = reg->startAddress;

    if (address == (_baseAddress + POSITION)) {
        return (ushort)getPosition();
    } else if (address == (_baseAddress + MODE)) {
        return (ushort)getMode();
    } else if (address == (_baseAddress + OVERRIDE)) {
        return (ushort)m_overridePosition.getValue();
    }
    
    return 0;
}

short Joystick::serial_register(Bridge *bridge) {
    Component::serial_register(bridge);
    return E_OK;
} 
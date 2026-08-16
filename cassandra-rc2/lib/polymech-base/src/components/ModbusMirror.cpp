#include "components/ModbusMirror.h"
#include "PHApp.h"
#include <ArduinoLog.h>
#include <modbus/ModbusTypes.h>

#ifdef ENABLE_MODBUS_MIRROR

static ModbusMirror* instance = nullptr; 

void ModbusMirror::onData(ModbusMessage response, uint32_t token) {
    if (instance && !instance->_isConnected) {
        instance->_isConnected = true;
        instance->m_status.update(MBM_STATUS_CONNECTED);
        LS_INFO("ModbusMirror: Connected to server %s:%d.", instance->_serverIP.toString().c_str(), instance->_serverPort);
    }
}

void ModbusMirror::onError(Error error, uint32_t token) {
    ModbusError me(error);
    LS_ERROR("ModbusMirror::onError: %02X - %s, token: %08X", (int)me, (const char *)me, token);
    if (instance) {
        instance->_isConnected = false;
        if (me.operator int() == (int)MB_Error::Timeout) {
            instance->m_status.update(MBM_STATUS_TIMEOUT);
        } else {
            instance->m_status.update(MBM_STATUS_DISCONNECTED);
        }
    }
}

ModbusMirror::ModbusMirror(PHApp* owner, uint16_t id) :
    NetworkComponent(MB_ADDR_MB_MIRROR_START, "ModbusMirror", id, Component::COMPONENT_DEFAULT, owner),
    _serverIP(),
    _serverPort(MODBUS_MIRROR_SERVER_PORT),
    _mbClient(nullptr),
    _isConnected(false),
    _lastConnectionAttempt(0),
    _initialConnectionAttempt(0),
    m_command(this, 0, "Command"),
    m_server_id(this, 1, "ServerID"),
    m_status(this, 2, "Status")
{
    instance = this;
    
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);

    IPAddress ip(MODBUS_MIRROR_SERVER_IP);
    _serverIP = ip;

    _mbClient = new ModbusClientTCPasync(_serverIP, _serverPort);
    
    _mbClient->onDataHandler(&ModbusMirror::onData);
    _mbClient->onErrorHandler(&ModbusMirror::onError);
    _mbClient->setTimeout(10000); 
    _mbClient->setIdleTimeout(60000); 
    m_server_id.update(1);
    m_status.update(MBM_STATUS_DISCONNECTED);
}

ModbusMirror::~ModbusMirror() {
    delete _mbClient;
    instance = nullptr;
}

short ModbusMirror::setup() {
    NetworkComponent::setup();
    _initialConnectionAttempt = 0;     
    const uint16_t baseAddr = mb_tcp_base_address();    
    m_command.initModbus(baseAddr + MB_OFS_COMMAND, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, m_command.name.c_str(), this->name.c_str());
    m_server_id.initModbus(baseAddr + MB_OFS_SERVER_ID, 1, this->id, this->slaveId, FN_WRITE_HOLD_REGISTER, m_server_id.name.c_str(), this->name.c_str());
    m_status.initModbus(baseAddr + MB_OFS_STATUS, 1, this->id, this->slaveId, FN_READ_HOLD_REGISTER, m_status.name.c_str(), this->name.c_str());    
    registerBlock(m_command.getRegisterInfo());
    registerBlock(m_server_id.getRegisterInfo());
    registerBlock(m_status.getRegisterInfo());    
    return E_OK;
}

short ModbusMirror::loop() {
    Component::loop();
    if (!_isConnected) {
        unsigned long now = millis();
        if (_initialConnectionAttempt == 0) {
            _initialConnectionAttempt = now;
        }

        if (now - _initialConnectionAttempt > MODBUS_MIRROR_MAX_RECONNECT_TIME_MS) {
            L_ERROR("ModbusMirror: Max reconnect time exceeded. Giving up.");
            m_status.update(MBM_STATUS_TIMEOUT);
            disable();
            return E_OK;
        }
        
        if (now - _lastConnectionAttempt > MODBUS_MIRROR_RECONNECT_INTERVAL_MS) {
            _lastConnectionAttempt = now;
            L_INFO("ModbusMirror: Attempting to connect to %s:%d...", _serverIP.toString().c_str(), _serverPort);
            m_status.update(MBM_STATUS_CONNECTING);
            
            uint32_t token = millis();
            Error err = _mbClient->addRequest(token, m_server_id.getValue(), READ_HOLD_REGISTER, 0, 1);
            if (err != SUCCESS) {
                ModbusError e(err);
                L_ERROR("ModbusMirror: Error creating request: %02X - %s", (int)e, (const char *)e);
                m_status.update(MBM_STATUS_DISCONNECTED);
            }
        }
    }
    return E_OK;
}

ModbusClientTCPasync* ModbusMirror::getClient() {
    return _mbClient;
}

short ModbusMirror::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    short result = NetworkComponent::mb_tcp_write(reg, networkValue);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }
    
    uint16_t address = reg->startAddress;
    if (address == (_baseAddress + MB_OFS_COMMAND))
    {
        m_command.update(networkValue);
        // Handle command
        return E_OK;
    }
    if (address == (_baseAddress + MB_OFS_SERVER_ID))
    {
        m_server_id.update(networkValue);
        return E_OK;
    }
    return E_INVALID_PARAMETER;
}

short ModbusMirror::mb_tcp_read(MB_Registers *reg)
{
    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED) {
        return result;
    }

    uint16_t address = reg->startAddress;
    if (address == (_baseAddress + MB_OFS_COMMAND))
    {
        return m_command.getValue();
    }
    if (address == (_baseAddress + MB_OFS_SERVER_ID))
    {
        return m_server_id.getValue();
    }
    if (address == (_baseAddress + MB_OFS_STATUS))
    {
        return m_status.getValue();
    }
    return 0;
}


#endif // ENABLE_MODBUS_MIRROR 
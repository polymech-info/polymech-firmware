#ifndef MODBUS_MIRROR_H
#define MODBUS_MIRROR_H

#include "config.h"
#ifdef ENABLE_MODBUS_MIRROR

#include "modbus/NetworkComponent.h"
#include <ModbusClientTCPasync.h>
#include "config-modbus.h"
#include "NetworkValue.h"

#define MODBUS_MIRROR_MB_COUNT 4 // command, server_id, status

class PHApp;

class ModbusMirror : public NetworkComponent<MODBUS_MIRROR_MB_COUNT> {
public:
    enum E_MBM_CMD {
        E_MBM_INFO, // (stub)
        E_MBM_SYNC
    };

    enum E_MBM_Status {
        MBM_STATUS_DISCONNECTED,
        MBM_STATUS_CONNECTING,
        MBM_STATUS_CONNECTED,
        MBM_STATUS_TIMEOUT
    };

    enum E_MB_Offset {
        MB_OFS_COMMAND = E_NVC_USER + 0,
        MB_OFS_SERVER_ID = E_NVC_USER + 1,
        MB_OFS_STATUS = E_NVC_USER + 2
    };

    ModbusMirror(PHApp* owner, uint16_t id);
    ~ModbusMirror() override;

    short setup() override;
    short loop() override;

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;

    ModbusClientTCPasync* getClient();

private:
    friend void onData(ModbusMessage response, uint32_t token);
    friend void onError(Error error, uint32_t token);

    static void onData(ModbusMessage response, uint32_t token);
    static void onError(Error error, uint32_t token);

    IPAddress _serverIP;
    uint16_t _serverPort;
    ModbusClientTCPasync* _mbClient;

    bool _isConnected;
    unsigned long _lastConnectionAttempt;
    unsigned long _initialConnectionAttempt;

    NetworkValue<uint16_t> m_command;
    NetworkValue<uint16_t> m_server_id;
    NetworkValue<uint16_t> m_status;
};

#endif // ENABLE_MODBUS_MIRROR
#endif // MODBUS_MIRROR_H 
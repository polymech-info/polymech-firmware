#ifndef NETWORK_VALUE_BASE_H
#define NETWORK_VALUE_BASE_H

#include "modbus/ModbusTypes.h"

class NetworkValueBase {
public:
    virtual ~NetworkValueBase() = default;
    virtual short setup() = 0;
    virtual MB_Registers getRegisterInfo() const = 0;
    virtual bool ownsAddress(ushort address) const = 0;
    virtual short handleRead(MB_Registers* reg) = 0;
    virtual short handleWrite(MB_Registers* reg, short value) = 0;
};

#endif // NETWORK_VALUE_BASE_H 
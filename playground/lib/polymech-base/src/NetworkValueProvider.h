#ifndef NETWORK_VALUE_PROVIDER_H
#define NETWORK_VALUE_PROVIDER_H

#include <vector>
#include "Component.h"
#include "NetworkValueBase.h"
#include <modbus/ModbusTCP.h>

class ModbusTCP;

/**
 * @class NetworkValueProvider
 * @brief An interface for components that own and manage a collection of NetworkValue instances.
 *
 * This class provides a generic implementation for Modbus TCP registration and handling
 * by iterating over a collection of registered NetworkValue objects. Components that use
 * the NetworkValue system should inherit from this class to gain automatic
 * Modbus functionality.
 */
class NetworkValueProvider : public Component
{
protected:
    std::vector<NetworkValueBase *> _networkValues;
    mutable ModbusTCP* _modbusTCP = nullptr;

public:
    NetworkValueProvider() { _networkValues.reserve(16); } // Default reserve
    NetworkValueProvider(const char* name, ushort id, uint16_t flags, Component* owner, size_t reserveCount = 16)
        : Component(name, id, flags, owner)
        {
            _networkValues.reserve(reserveCount);
        }
    virtual ~NetworkValueProvider() = default;

    /**
     * @brief Adds a NetworkValue instance to the provider's collection.
     * @param nv Pointer to the NetworkValueBase instance.
     */
    void addNetworkValue(NetworkValueBase *nv)
    {
        if (nv)
        {
            _networkValues.push_back(nv);
            Log.infoln("NVP '%s': Added NV. New size: %d, Capacity: %d.", 
                       this->name.c_str(), 
                       _networkValues.size(), 
                       _networkValues.capacity());
        }
    }

    /**
     * @brief Overrides the base setup to also call setup on all registered NetworkValues.
     */
    short setup() override {
        Component::setup();
        for (NetworkValueBase* nv : _networkValues) {
            if (nv) {
                nv->setup();
            }
        }
        return E_OK;
    }

    /**
     * @brief Generic implementation to register all contained NetworkValues with the ModbusTCP manager.
     */
    void mb_tcp_register(ModbusTCP *manager) override
    {
        this->_modbusTCP = manager;
        Log.infoln("NVP '%s': Registering NetworkValues...", this->name.c_str());
        for (NetworkValueBase *nv : _networkValues)
        {
            if (nv != nullptr)
            {
                MB_Registers regInfo = nv->getRegisterInfo();
                if (regInfo.startAddress > 0) {
                    Log.infoln("  > Registering '%s' at addr %d", regInfo.name, regInfo.startAddress);
                    manager->registerModbus(this, regInfo);
                }else{
                    Log.warningln("  > Not registering '%s' at addr %d", regInfo.name, regInfo.startAddress);
                }
            }
        }
    }

    /**
     * @brief Generic implementation to handle Modbus read requests.
     */
    virtual short mb_tcp_read(MB_Registers *reg)
    {
        for (NetworkValueBase *nv : _networkValues)
        {
            if (nv && nv->ownsAddress(reg->startAddress)) {
                return nv->handleRead(reg);
            }else{
                // Log.warningln("  > Not handling read for '%s' at addr %d", nv->getRegisterInfo().name, reg->startAddress);
            }
        }
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }

    /**
     * @brief Generic implementation to handle Modbus write requests.
     */
    virtual short mb_tcp_write(MB_Registers *reg, short value)
    {
        Log.infoln(" mb_tcp_write > Handling write for '%s' at addr %d", reg->name, reg->startAddress);
        for (NetworkValueBase *nv : _networkValues)
        {
            if (nv){
                if (nv->ownsAddress(reg->startAddress)) {
                    Log.infoln(" mb_tcp_write > Handling write for '%s' at addr %d", nv->getRegisterInfo().name, reg->startAddress);
                    return nv->handleWrite(reg, value);
                }else{
                    Log.warningln(" mb_tcp_write > Not handling write for '%s' at addr %d", nv->getRegisterInfo().name, reg->startAddress);
                }
            }else{
                Log.warningln(" mb_tcp_write > Not handling write for '%s' at addr %d", nv->getRegisterInfo().name, reg->startAddress);
            }
        }
        return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
    }
};

#endif // NETWORK_VALUE_PROVIDER_H 
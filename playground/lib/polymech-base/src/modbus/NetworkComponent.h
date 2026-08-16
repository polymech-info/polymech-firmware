#ifndef NETWORK_COMPONENT_H
#define NETWORK_COMPONENT_H

#include "Component.h"
#include "modbus/ModbusTypes.h"
#include <ArduinoLog.h>
#include <utility>
#include <vector>
#include "NetworkValue.h"

#ifndef NETWORKCOMPONENT_ENABLE
#define NETWORKCOMPONENT_ENABLE 1
#endif

/*
 * NetworkComponent<N>
 * -------------------
 * Generic mix-in for components that expose a *fixed* number (N) of Modbus blocks.
 * – Handles storage & view of MB_Registers.
 * – Provides helper to register blocks without manual index maths.
 * – Offers bind() helper that wires an existing NetworkValue<T> to the block.
 */

template <size_t N = 16>
class NetworkComponent : public Component {
protected:
    MB_Registers* _modbusBlocks; // dynamically allocated contiguous array
    ModbusBlockView _modbusBlockView;
    size_t _nextIndex;
    std::vector<NetworkValueBase *> _networkValues;
    mutable ModbusTCP* _modbusTCP = nullptr;


public:
    template <typename... Args>
    NetworkComponent(Args &&... args)
        : Component(std::forward<Args>(args)...),
          _modbusBlocks(new MB_Registers[N]()), // value-initialized
          _modbusBlockView{_modbusBlocks, static_cast<int>(N)},
          _nextIndex(0) {
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    }

    // Destructor to free allocated blocks
    virtual ~NetworkComponent() {
        delete[] _modbusBlocks;
    }

    /**
     * Register a new Modbus block.
     * Returns pointer to internal MB_Registers entry or nullptr if N exceeded.
     */
    MB_Registers *registerBlock(uint16_t startAddress,
                                uint16_t count,
                                E_FN_CODE fn,
                                E_ModbusAccess access,
                                const char *name = nullptr,
                                const char *group = nullptr) {
        if (_nextIndex >= N) {
            Log.errorln(F("NetworkComponent: max blocks (%d) reached for %s"), N, this->name.c_str());
            return nullptr;
        }
        _modbusBlocks[_nextIndex] = MB_Registers(startAddress,
                                                 count,
                                                 fn,
                                                 access,
                                                 this->id,
                                                 this->slaveId,
                                                 name,
                                                 group);
        return &_modbusBlocks[_nextIndex++];
    }

    /**
     * Obtain view for ModbusTCP manager.
     */
    ModbusBlockView *mb_tcp_blocks() const override {
        // cast away constness: manager API isn't const-correct
        return const_cast<ModbusBlockView *>(&_modbusBlockView);
    }

    /**
     * Convenience: wire a NetworkValue instance to the given MB_Registers entry.
     */
    template <typename T>
    void bind(NetworkValue<T> &nv, const MB_Registers &reg) {
        nv.initModbus(reg.startAddress,
                      reg.count,
                      reg.componentId,
                      reg.slaveId,
                      reg.type,
                      reg.name,
                      reg.group);
    }

    void addNetworkValue(NetworkValueBase *nv) {
        if (nv) {
            _networkValues.push_back(nv);
        }
    }

    short info() override {
        Component::info();
        Log.infoln(F("  %d network values:"), _networkValues.size());
        for (const auto& nv : _networkValues) {
            nv->info();
        }
        return E_OK;
    }
};

#define INIT_NETWORK_VALUE(variable, name, ...) \
    variable(this, id, name, [this](NetworkValueBase* nv){ addNetworkValue(nv); }, ##__VA_ARGS__)

#endif // NETWORK_COMPONENT_H
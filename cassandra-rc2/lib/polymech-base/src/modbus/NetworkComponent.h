#ifndef NETWORK_COMPONENT_H
#define NETWORK_COMPONENT_H

#include "Component.h"
#include "modbus/ModbusTypes.h"
#include "net/commons.h"
#include <ArduinoLog.h>
#include <utility>
#include <vector>
#include "NetworkValue.h"
#include "modbus/ModbusTCP.h"

#ifndef NETWORKCOMPONENT_ENABLE
#define NETWORKCOMPONENT_ENABLE 1
#endif

template <size_t N = 20>
class NetworkComponent : public Component
{
protected:
    uint16_t _baseAddress;
    MB_Registers *_modbusBlocks;
    mutable ModbusBlockView _modbusBlockView;
    size_t _nextIndex;
    std::vector<NetworkValueBase *> _networkValues;
    NetworkValue<bool> m_enabled;
    ModbusTCP *modbusTCP;

public:
    template <typename... Args>
    NetworkComponent(uint16_t baseAddress, Args &&...args)
        : Component(std::forward<Args>(args)...),
          _baseAddress(baseAddress),
          _modbusBlocks(new MB_Registers[N]()),
          _modbusBlockView{_modbusBlocks, static_cast<int>(N)},
          _nextIndex(0),
          m_enabled(this, this->id, "Enabled", static_cast<uint8_t>(E_NetworkValueFeatureFlags::E_NVFF_ALL)),
          modbusTCP(nullptr)
    {
        _networkValues.reserve(N);
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
        addNetworkValue(&m_enabled);
    }

    virtual ~NetworkComponent()
    {
        delete[] _modbusBlocks;
    }

    virtual void enable(bool enabled) override
    {
        Component::enable(enabled);
        m_enabled.update(enabled);
    }

    short setup() override
    {
        Component::setup();
        const uint16_t baseAddr = mb_tcp_base_address();
        m_enabled.initModbus(baseAddr + E_NVC_ENABLED, 1, this->id, this->slaveId, FN_WRITE_COIL, "Enabled", this->name.c_str());
        m_enabled.initNotify(true, true, NetworkValue_ThresholdMode::DIFFERENCE);
        registerBlock(m_enabled.getRegisterInfo());
        return E_OK;
    }

    short loop() override
    {
        Component::loop();
        if (!this->enabled())
        {
            return E_OK;
        }
        return loopNetwork();
    }

    virtual short loopNetwork() { return E_OK; }

    MB_Registers *registerBlock(const MB_Registers &reg)
    {
        if (_nextIndex >= N)
        {
            Log.errorln(F("NetworkComponent: max blocks (%d) reached for %s | Address: %d | Count: %d | Type: %d | Name: %s"), N, this->name.c_str(), reg.startAddress, reg.count, reg.type, reg.name);
            return nullptr;
        }
        _modbusBlocks[_nextIndex] = reg;
        return &_modbusBlocks[_nextIndex++];
    }

    ModbusBlockView *mb_tcp_blocks() const override
    {
        _modbusBlockView.count = _nextIndex;
        return const_cast<ModbusBlockView *>(&_modbusBlockView);
    }

    void mb_tcp_register(ModbusTCP *manager) override
    {
        if (!hasNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS))
            return;
        this->modbusTCP = manager;
        ModbusBlockView *blocksView = this->mb_tcp_blocks();
        for (int i = 0; i < blocksView->count; ++i)
        {
            if (blocksView->data[i].startAddress != (ushort)-1)
            {
                manager->registerModbus(this, blocksView->data[i]);
            }
        }
    }

    short onMessage(int id, E_CALLS verb, E_MessageFlags flags, void *user, Component *src) override
    {
        if (verb == E_CALLS::EC_PROTOBUF_UPDATE && user != nullptr)
        {
            return this->owner->onMessage(id, verb, flags, user, src);
        }
        return Component::onMessage(id, verb, flags, user, src);
    }

    void addNetworkValue(NetworkValueBase *nv)
    {
        if (nv)
        {
            _networkValues.push_back(nv);
        }
    }

    // --- Virtual Functions for Derived Classes ---
    uint16_t mb_tcp_base_address() const override { return _baseAddress; }

    short mb_tcp_read(MB_Registers *reg) override
    {
        if (reg->startAddress == (mb_tcp_base_address() + E_NVC_ENABLED))
        {
            // return false;
            // return m_enabled.getValue() ? 1 : 0;
            return this->enabled() ? 1 : 0;
        }
        return E_NOT_IMPLEMENTED;
    }

    short mb_tcp_write(MB_Registers *reg, short value) override
    {
        if (reg->startAddress == (mb_tcp_base_address() + E_NVC_ENABLED))
        {
            this->enable(value == 1);
            m_enabled.update(value == 1);
            Log.verboseln("mb_tcp_write NetworkComponent '%s': Enabled state changed to: %s", this->name.c_str(), m_enabled.getValue() ? "ENABLED" : "DISABLED");
            return E_OK;
        }
        return E_NOT_IMPLEMENTED;
    }
};

#define INIT_NETWORK_VALUE(variable, name, ...) \
    variable(this, id, name, ##__VA_ARGS__)

#define SETUP_NETWORK_VALUE(nv_member, reg_offset_enum, fn_code, desc, ...) \
    do                                                                      \
    {                                                                       \
        (nv_member).initNotify(__VA_ARGS__);                                \
        (nv_member).initModbus(                                             \
            _baseAddress + static_cast<uint16_t>(reg_offset_enum),          \
            1,                                                              \
            this->id,                                                       \
            this->slaveId,                                                  \
            fn_code,                                                        \
            desc,                                                           \
            this->name.c_str());                                            \
        registerBlock((nv_member).getRegisterInfo());                       \
    } while (0)

#endif // NETWORK_COMPONENT_H
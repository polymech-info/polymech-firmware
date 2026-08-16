#ifndef MIXIN_WITH_MODBUS_H
#define MIXIN_WITH_MODBUS_H

#include <ArduinoJson.h>
#include "net/NetworkComponent.h"
#include "modbus/ModbusTCP.h"
#include "modbus/ModbusTypes.h"
#include "net/commons.h"

template <typename Base, size_t N>
class WithModbus : public Base {
protected:
    uint16_t _baseAddress;
    MB_Registers* _modbusBlocks;
    mutable ModbusBlockView _modbusBlockView;
    size_t _nextIndex;
    ModbusTCP* modbusTCP;

public:
    template <typename... Args>
    WithModbus(uint16_t baseAddress, Args&&... args)
        : Base(std::forward<Args>(args)...),
          _baseAddress(baseAddress),
          _modbusBlocks(new MB_Registers[N]()),
          _modbusBlockView{_modbusBlocks, static_cast<int>(N)},
          _nextIndex(0),
          modbusTCP(nullptr)
    {
        this->setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    }

    virtual ~WithModbus() {
        delete[] _modbusBlocks;
    }

    short setup() override {
        Base::setup();
        const uint16_t baseAddr = this->mb_tcp_base_address();
        this->m_enabled.initModbus(baseAddr + E_NVC_ENABLED, 1, this->id, this->slaveId, FN_WRITE_COIL, "Enabled", this->name.c_str());
        this->m_enabled.initNotify(true, true, NetworkValue_ThresholdMode::DIFFERENCE);
        this->registerBlock(this->m_enabled.getRegisterInfo());
        return E_OK;
    }

    MB_Registers* registerBlock(const MB_Registers& reg) {
        if (_nextIndex >= N) {
            Log.errorln(F("WithModbus: max blocks (%d) reached for %s | Address: %d | Count: %d | Type: %d | Name: %s"), N, this->name.c_str(), reg.startAddress, reg.count, reg.type, reg.name);
            return nullptr;
        }
        _modbusBlocks[_nextIndex] = reg;
        return &_modbusBlocks[_nextIndex++];
    }

    ModbusBlockView* mb_tcp_blocks() const override {
        _modbusBlockView.count = _nextIndex;
        return const_cast<ModbusBlockView*>(&_modbusBlockView);
    }

    void mb_tcp_register(ModbusTCP* manager) override {
        this->modbusTCP = manager;
        ModbusBlockView* blocksView = this->mb_tcp_blocks();
        for (int i = 0; i < blocksView->count; ++i) {
            if (blocksView->data[i].startAddress != (ushort)-1) {
                manager->registerModbus(this, blocksView->data[i]);
            }
        }
    }

    uint16_t mb_tcp_base_address() const override { return _baseAddress; }

    short mb_tcp_read(MB_Registers* reg) override {
        if (reg->startAddress == (_baseAddress + E_NVC_ENABLED)) {
            return this->enabled() ? 1 : 0;
        }
        return E_NOT_IMPLEMENTED;
    }

    short mb_tcp_write(MB_Registers* reg, short value) override {
        if (reg->startAddress == (_baseAddress + E_NVC_ENABLED)) {
            this->enable(value != 0);
            this->m_enabled.update(value != 0);
            return E_OK;
        }
        return E_NOT_IMPLEMENTED;
    }
};

// Helper macros can be defined here as they are specific to the Modbus context
#define SETUP_NETWORK_VALUE(nv_member, reg_offset_enum, fn_code, desc, ...) \
    do { \
        (nv_member).initNotify(__VA_ARGS__); \
        (nv_member).initModbus( \
            _baseAddress + static_cast<uint16_t>(reg_offset_enum), \
            1, \
            this->id, \
            this->slaveId, \
            fn_code, \
            desc, \
            this->name.c_str() \
        ); \
        registerBlock((nv_member).getRegisterInfo()); \
    } while(0)

#endif // MIXIN_WITH_MODBUS_H 
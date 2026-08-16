#ifndef MIXIN_WITH_MODBUS_RTU_H
#define MIXIN_WITH_MODBUS_RTU_H

#include <ArduinoJson.h>
#include "net/NetworkComponent.h"
#include "modbus/ModbusRTU.h"
#include "modbus/ModbusTypes.h"
#include "net/commons.h"

template <typename Base, size_t N>
class WithModbusRTU : public Base {
protected:
    uint16_t _baseAddress;
    MB_Registers* _modbusBlocks;
    mutable ModbusBlockView _modbusBlockView;
    size_t _nextIndex;
    ModbusRTU* modbusRTU;

public:
    template <typename... Args>
    WithModbusRTU(uint16_t baseAddress, Args&&... args)
        : Base(std::forward<Args>(args)...),
          _baseAddress(baseAddress),
          _modbusBlocks(new MB_Registers[N]()),
          _modbusBlockView{_modbusBlocks, static_cast<int>(N)},
          _nextIndex(0),
          modbusRTU(nullptr)
    {
        this->setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    }

    virtual ~WithModbusRTU() {
        delete[] _modbusBlocks;
    }

    short setup() override {
        Base::setup();
        const uint16_t baseAddr = this->mb_rtu_base_address();
        this->m_enabled.initModbus(baseAddr + E_NVC_ENABLED, 1, this->id, this->slaveId, FN_WRITE_COIL, "Enabled", this->name.c_str());
        this->m_enabled.initNotify(true, true, NetworkValue_ThresholdMode::DIFFERENCE);
        this->registerBlock(this->m_enabled.getRegisterInfo());
        return E_OK;
    }

    MB_Registers* registerBlock(const MB_Registers& reg) {
        if (_nextIndex >= N) {
            Log.errorln(F("WithModbusRTU: max blocks (%d) reached for %s | Address: %d | Count: %d | Type: %d | Name: %s"), N, this->name.c_str(), reg.startAddress, reg.count, reg.type, reg.name);
            return nullptr;
        }
        _modbusBlocks[_nextIndex] = reg;
        return &_modbusBlocks[_nextIndex++];
    }

    ModbusBlockView* mb_rtu_blocks() const {
        _modbusBlockView.count = _nextIndex;
        return const_cast<ModbusBlockView*>(&_modbusBlockView);
    }

    void mb_rtu_register(ModbusRTU* manager) {
        this->modbusRTU = manager;
    }

    uint16_t mb_rtu_base_address() const { return _baseAddress; }

    short mb_rtu_read(MB_Registers* reg) {
        if (reg->startAddress == (_baseAddress + E_NVC_ENABLED)) {
            return this->enabled() ? 1 : 0;
        }
        return E_NOT_IMPLEMENTED;
    }

    short mb_rtu_write(MB_Registers* reg, short value) {
        if (reg->startAddress == (_baseAddress + E_NVC_ENABLED)) {
            this->enable(value != 0);
            this->m_enabled.update(value != 0);
            return E_OK;
        }
        return E_NOT_IMPLEMENTED;
    }
};

#endif // MIXIN_WITH_MODBUS_RTU_H 
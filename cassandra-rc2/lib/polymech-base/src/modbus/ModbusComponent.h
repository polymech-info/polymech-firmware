#ifndef MODBUS_COMPONENT_H
#define MODBUS_COMPONENT_H

#include "Component.h"
#include "modbus/ModbusTypes.h"
#include <ArduinoLog.h>
#include <utility>
#include <vector>
#include "NetworkValue.h"

template <size_t N>
class ModbusComponent {
public:
    Component* _owner;
    mutable ModbusBlockView _modbusBlockView;
    size_t _nextIndex;
    MB_Registers* _modbusBlocks;

    ModbusComponent()
        : _owner(nullptr),
          _modbusBlockView{nullptr, 0},
          _nextIndex(0),
          _modbusBlocks(nullptr)
    {}

    void init(Component* owner) {
        _owner = owner;
        _modbusBlocks = new MB_Registers[N]();
        _modbusBlockView = {_modbusBlocks, 0};
        _nextIndex = 0;
        if (_owner) {
            _owner->setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
        }
    }

    ~ModbusComponent() {
        delete[] _modbusBlocks;
    }

    MB_Registers *registerBlock(const MB_Registers& reg) {
        if (_nextIndex >= N) {
            if (_owner) {
                Log.errorln(F("ModbusComponent: max blocks (%d) reached for %s"), N, _owner->name.c_str());
            } else {
                Log.errorln(F("ModbusComponent: max blocks (%d) reached"), N);
            }
            return nullptr;
        }
        _modbusBlocks[_nextIndex] = reg;
        return &_modbusBlocks[_nextIndex++];
    }

    ModbusBlockView *mb_tcp_blocks() const {
        const_cast<ModbusBlockView&>(_modbusBlockView).count = _nextIndex;
        return const_cast<ModbusBlockView *>(&_modbusBlockView);
    }
};

#define INIT_MODBUS_NETWORK_VALUE(variable, name, ...) \
    variable(this, this->id, name, ##__VA_ARGS__)

#endif // MODBUS_COMPONENT_H 
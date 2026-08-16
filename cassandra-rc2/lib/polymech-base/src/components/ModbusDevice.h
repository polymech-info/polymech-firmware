#ifndef MODBUS_DEVICE_H
#define MODBUS_DEVICE_H

#include "Component.h"
#include <modbus/ModbusTypes.h>
#include <utility> // For std::forward

/**
 * @class ModbusDevice
 * @brief A template base class (mixin) for components that expose a fixed number of Modbus blocks.
 * 
 * This class handles the boilerplate of creating and managing the _modbusBlocks array
 * and the _modbusBlockView. A derived class should inherit from ModbusDevice<N>,
 * where N is the number of Modbus blocks it will expose.
 * 
 * The derived class is still responsible for:
 * 1. Populating the `_modbusBlocks` array in its constructor.
 * 2. Implementing `mb_tcp_register`, `mb_tcp_read`, and `mb_tcp_write`.
 */
template <size_t N>
class ModbusDevice : public Component {
protected:
    MB_Registers _modbusBlocks[N];
    ModbusBlockView _modbusBlockView;

public:
    // Use a variadic template to perfectly forward all constructor arguments to the Component base.
    // This makes our mixin transparent and flexible.
    template<typename... Args>
    ModbusDevice(Args&&... args) 
        : Component(std::forward<Args>(args)...), 
          _modbusBlockView{_modbusBlocks, N} {
        setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    }

    // Provides the default implementation for returning the block view.
    ModbusBlockView* mb_tcp_blocks() const override {
        // We must cast away const here because the ModbusBlockView struct
        // was not designed with const-correctness in mind.
        return const_cast<ModbusBlockView*>(&_modbusBlockView);
    }
};

#endif // MODBUS_DEVICE_H 
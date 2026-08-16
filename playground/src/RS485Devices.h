#ifndef RS485_DEVICES_H
#define RS485_DEVICES_H
#include "features.h"

#ifdef ENABLE_RS485
class RS485;
/**
 * @brief Utility class to register application-specific RS485 devices.
 */
class RS485Devices {
public:
    /**
     * @brief Registers all required downstream RS485 slave devices with the main RS485 interface.
     *
     * Call this function once during setup after the RS485 component has been initialized.
     *
     * @param rs485Interface A pointer to the initialized RS485 component instance.
     */
    static void registerApplicationDevices(RS485* rs485Interface);
};

#endif // ENABLE_RS485

#endif // RS485_DEVICES_H 
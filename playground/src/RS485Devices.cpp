#include "features.h"

#ifdef ENABLE_RS485

#include <Logger.h>
#include <components/RS485.h>
#include <components/OmronE5.h>
#include <components/SAKO_VFD.h>
#include "RS485Devices.h"
#include "PHApp.h"

void RS485Devices::registerApplicationDevices(RS485 *rs485Interface)
{
    if (!rs485Interface)
    {
        Log.errorln(F("RS485Devices: Cannot register devices, RS485 interface is null!"));
        return;
    }
    Log.infoln(F("RS485Devices: Registering %d application RS485 slaves..."), NUM_OMRON_DEVICES);
    PHApp *phApp = (PHApp *)rs485Interface->owner;

#ifdef ENABLE_OMRON_E5
    for (uint8_t i = 0; i < NUM_OMRON_DEVICES; ++i)
    {
        uint8_t omronSlaveId = OMRON_E5_SLAVE_ID_BASE + i;
        OmronE5 *omronDevice = new OmronE5(rs485Interface, omronSlaveId);
        omronDevice->setup();
        if (!rs485Interface->deviceManager.addDevice(omronDevice))
        {
            Log.errorln(F("RS485Devices: Failed to add OmronE5 Slave %d to manager"), omronSlaveId);
            delete omronDevice;
        }
    }
#endif
#ifdef ENABLE_SAKO_VFD
    phApp->vfd_0->owner = rs485Interface;
    if (!rs485Interface->deviceManager.addDevice(phApp->vfd_0))
    {
        Log.errorln(F("RS485Devices: Failed to add SAKO_VFD Slave %d to manager"), MB_SAKO_VFD_SLAVE_ID);
    }
#endif
    Log.infoln(F("RS485Devices: Finished registering application RS485 slaves."));
}

#endif
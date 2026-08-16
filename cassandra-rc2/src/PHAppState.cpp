#include "PHApp.h"
#include "RuntimeState.h"
#include "config.h"
#include "features.h"

short PHApp::restoreState()
{
    L_INFO("Restoring state");
#ifdef ENABLE_RUNTIME_STATE

    if (!runtimeState || !runtimeState->load(RUNTIME_STATE_FILENAME))
    {
        L_ERROR(F("Failed to load runtime state"));
        return E_INVALID_PARAMETER;
    }

#ifdef ENABLE_MODBUS_TCP
    setSlave(runtimeState->isSlave, false);
#endif

#ifdef ENABLE_PROFILE_TEMPERATURE
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        if (tempProfiles[i])
        {
            const bool _enabled = runtimeState->tempProfiles[i].enabled;
            ((PlotBase *)tempProfiles[i])->enable(_enabled);
        }
        if (runtimeState->tempProfiles[i].running)
        {
            tempProfiles[i]->pause();
            tempProfiles[i]->setElapsedTime(runtimeState->tempProfiles[i].elapsedSec * 1000);
        }
    }
#endif // ENABLE_PROFILE_TEMPERATURE

// Restore Omron PID enabled states
#if defined(ENABLE_OMRON_E5) && defined(ENABLE_RS485)

    RTU_Base *const *devices = rs485->deviceManager.getDevices();
    int numDevices = rs485->deviceManager.getMaxDevices();
    for (int i = 0; i < NUM_OMRON_DEVICES; ++i)
    {
        uint8_t slaveIdToFind = runtimeState->omronPids[i].slaveId;
        for (int j = 0; j < numDevices; ++j)
        {
            if (devices[j] && devices[j]->slaveId == slaveIdToFind)
            {
                OmronE5 *omron = static_cast<OmronE5 *>(devices[j]);
                const bool _enabled = runtimeState->omronPids[i].enabled;
                omron->enable(_enabled);
                break;
            }
        }
    }

    setAllOmronComWrite(runtimeState->allOmronComWrite, false);
    setAllOmronStop(runtimeState->allOmronStop, false);
    setAllOmronComWrite(runtimeState->allOmronComWrite, false);
    L_INFO("Omron Stop state restored: %d", runtimeState->allOmronStop);
    L_INFO("Slave state restored: %d", runtimeState->isSlave);
    L_INFO("Omron Com Write state restored: %d", runtimeState->allOmronComWrite);
#endif
    for (const auto &state : runtimeState->loadedManagedComponents)
    {
        Component *comp = this->byId(state.id);
        if (comp)
        {
            L_INFO("Restoring component %d state: %d : %s", state.id, state.enabled, comp->name.c_str());
            comp->enable(state.enabled);
        }
        else
        {
            L_ERROR("Component %d not found", state.id);
        }
    }
    runtimeState->loadedManagedComponents.clear();

#ifdef ENABLE_MB_GPIO
    if (gpio_0)
    {
        for (const auto &state : runtimeState->gpioPins)
        {
            short result = gpio_0->setPinTypeByNumber(state.pin, state.type, false);
            if (result != E_OK)
            {
                L_WARN("Failed to restore GPIO pin %u type %u: %d", state.pin, state.type, result);
            }
        }
        L_INFO("GPIO runtime pin types restored: %d", runtimeState->gpioPins.size());
    }
#endif
    return E_OK;
#else
    return E_OK;
#endif // ENABLE_RUNTIME_STATE
}

void PHApp::updateRuntimeState()
{
#ifdef ENABLE_RUNTIME_STATE
    if (!runtimeState)
        return;

    // Update temperature profiles runtime state
#ifdef ENABLE_PROFILE_TEMPERATURE
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        if (tempProfiles[i])
        {
            runtimeState->tempProfiles[i].running = tempProfiles[i]->isRunning();
            runtimeState->tempProfiles[i].enabled = tempProfiles[i]->enabled();
            runtimeState->tempProfiles[i].elapsedSec = tempProfiles[i]->getElapsedMs() / 1000;
        }
    }
#endif

    // Update Omron PIDs runtime state
#if defined(ENABLE_OMRON_E5) && defined(ENABLE_RS485)
    uint32_t totalWhSum = 0;
    RTU_Base *const *devices = rs485->deviceManager.getDevices();
    int numDevices = rs485->deviceManager.getMaxDevices();
    int omronStateIndex = 0;
    for (int i = 0; i < numDevices && omronStateIndex < NUM_OMRON_DEVICES; ++i)
    {
        if (devices[i] && devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
        {
            OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
            runtimeState->omronPids[omronStateIndex].slaveId = omron->slaveId;
            runtimeState->omronPids[omronStateIndex].enabled = omron->enabled();
            // L_INFO("updateRuntimeState - Omron PID %d (Slave %d) enabled: %d", omronStateIndex, omron->slaveId, omron->enabled());
            totalWhSum += omron->getTotalWh();
            omronStateIndex++;
        }
    }
    runtimeState->totalKwh = totalWhSum / 1000;
#endif

    // Update coil states
    runtimeState->isSlave = _is_slave;
    runtimeState->allOmronStop = _all_omron_stop;
    runtimeState->allOmronComWrite = _all_omron_com_write;

#ifdef ENABLE_MB_GPIO
    runtimeState->gpioPins.clear();
    if (gpio_0)
    {
        const size_t count = gpio_0->pinConfigCount();
        runtimeState->gpioPins.reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            uint8_t pin = 0;
            uint8_t type = 0;
            if (gpio_0->getPinTypeAt(i, pin, type))
            {
                GPIORuntime state;
                state.pin = pin;
                state.type = type;
                runtimeState->gpioPins.push_back(state);
            }
        }
    }
#endif

#endif
    runtimeState->save(RUNTIME_STATE_FILENAME);
}

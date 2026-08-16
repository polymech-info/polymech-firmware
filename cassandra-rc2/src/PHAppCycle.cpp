#include "PHApp.h"

#include <profiles/PlotBase.h>
#include <profiles/TemperatureProfile.h>
#include <components/Feedback3C.h>
#include <components/OperatorSwitch.h>
#include <profiles/PlotBase.h>
#include <components/OmronE5.h>
#include <profiles/IPlungerEvents.h>
#include <components/Plunger.h>
#include <enums.h>

void PHApp::loopCycle()
{
    // --- Application State Handling ---
    // Check for critical app errors first. This logic takes precedence.
    static bool slaveToggleReady = true;
    short appState = getAppState(0);
    bool hasError = (appState >= APP_STATE::ERROR || getLastError() != E_OK);

    if (hasError)
    {
#if defined(ENABLE_PROFILE_TEMPERATURE)
        for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
        {
            if (tempProfiles[i] != nullptr)
            {
                PlotStatus status = tempProfiles[i]->getCurrentStatus();
                if (status == PlotStatus::RUNNING || status == PlotStatus::PAUSED)
                {
                    tempProfiles[i]->stop();
                }
            }
        }
#endif

#if defined(ENABLE_PROFILE_PRESSURE)
        for (int i = 0; i < PROFILE_PRESSURE_COUNT; ++i)
        {
            if (pressureProfiles[i] != nullptr)
            {
                PlotStatus status = pressureProfiles[i]->getCurrentStatus();
                if (status == PlotStatus::RUNNING || status == PlotStatus::PAUSED)
                {
                    pressureProfiles[i]->stop();
                }
            }
        }
#endif
        // Abort further cycle logic if the app is in an error state.
        return;
    }

    if (operatorSwitch_0 && operatorSwitch_0->getState() == OperatorSwitch::State::IDLE)
    {
        slaveToggleReady = true;
    }

    if (_is_slave)
    {
#if defined(ENABLE_OPERATOR_SWITCH)
        if (operatorSwitch_0)
        {
            OperatorSwitch::State currentState = operatorSwitch_0->getState();
            if (currentState != this->lastOperatorSwitchState)
            {
                if (currentState == OperatorSwitch::State::BOTH_HELD && slaveToggleReady)
                {
                    onSlaveModeChanged(false);
                    updateRuntimeState();
                    L_INFO("Slave Mode Disabled via BOTH_HELD");
                    slaveToggleReady = false;
                }
                this->lastOperatorSwitchState = currentState;
            }
        }
#endif
    }

    if (_is_slave)
    {
#if defined(ENABLE_PROFILE_TEMPERATURE)
        for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
        {
            if (tempProfiles[i] != nullptr)
            {
                PlotStatus status = tempProfiles[i]->getCurrentStatus();
                if (status == PlotStatus::RUNNING || status == PlotStatus::PAUSED)
                {
                    tempProfiles[i]->stop();
                }
            }
        }
#endif

#if defined(ENABLE_PROFILE_PRESSURE)
        for (int i = 0; i < PROFILE_PRESSURE_COUNT; ++i)
        {
            if (pressureProfiles[i] != nullptr)
            {
                PlotStatus status = pressureProfiles[i]->getCurrentStatus();
                if (status == PlotStatus::RUNNING || status == PlotStatus::PAUSED)
                {
                    pressureProfiles[i]->stop();
                }
            }
        }
#endif

#if defined(ENABLE_FEEDBACK_3C)
        if (feedback3C_0)
        {
            feedback3C_0->setMode(Feedback3C::Mode::MODE_WARNING_HOLD);
        }
#endif
        return;
    }

#if defined(ENABLE_PROFILE_TEMPERATURE) && defined(ENABLE_FEEDBACK_3C)
    bool anyProfilePaused = false;
    bool anyProfileRunning = false;
    bool anyProfileFinished = false;
    for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
    {
        if (tempProfiles[i] == nullptr)
        {
            continue;
        }

        switch (tempProfiles[i]->getCurrentStatus())
        {
        case PlotStatus::PAUSED:
            anyProfilePaused = true;
            anyProfileRunning = true;
            break;
        case PlotStatus::RUNNING:
            anyProfileRunning = true;
            break;
        case PlotStatus::FINISHED:
            anyProfileFinished = true;
            break;
        default:
            break;
        }
    }

#endif

#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5) && defined(ENABLE_FEEDBACK_3C) && defined(ENABLE_PROFILE_TEMPERATURE)
    if (!anyProfileRunning)
    {
        bool anyOmronHeating = false;
        RTU_Base *const *devices = rs485->deviceManager.getDevices();
        int numDevices = rs485->deviceManager.getMaxDevices();
        for (int i = 0; i < numDevices; ++i)
        {
            if (devices[i] != nullptr && devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
            {
                OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
                if (omron && omron->isHeating())
                {
                    anyOmronHeating = true;
                    break;
                }
            }
        }
        if (anyOmronHeating)
        {
            feedback3C_0->setMode(Feedback3C::Mode::MODE_HEATING);
        }
        else
        {
            feedback3C_0->setMode(Feedback3C::Mode::MODE_STANDBY);
        }
    }
#endif

#if defined(ENABLE_OPERATOR_SWITCH) && defined(ENABLE_PROFILE_TEMPERATURE)
    auto onCycleStart = [&]()
    {
        // Resume paused profiles first
        bool resumedProfile = false;
        for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
        {
            if (tempProfiles[i] != nullptr && tempProfiles[i]->getCurrentStatus() == PlotStatus::PAUSED)
            {
                tempProfiles[i]->resume();
                resumedProfile = true;
            }
        }

        // If no profiles were resumed, start any that are idle and enabled.
        if (!resumedProfile)
        {
            for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
            {
                if (tempProfiles[i] != nullptr && tempProfiles[i]->enabled() && tempProfiles[i]->getCurrentStatus() == PlotStatus::IDLE)
                {
                    tempProfiles[i]->start();
                }
            }
        }

        /*

        */
    };

    auto onCycleStop = [&]()
    {
        for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
        {
            if (tempProfiles[i] != nullptr)
            {
                tempProfiles[i]->stop();
            }
        }
#if defined(ENABLE_PROFILE_PRESSURE)
        for (int i = 0; i < PROFILE_PRESSURE_COUNT; ++i)
        {
            if (pressureProfiles[i] != nullptr)
            {
                pressureProfiles[i]->stop();
            }
        }
#endif
#if defined(ENABLE_PROFILE_SIGNAL_PLOT)
        for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; ++i)
        {
            if (signalPlots[i] != nullptr)
            {
                signalPlots[i]->stop();
            }
        }
#endif
#if defined(ENABLE_PRESS_CYLINDER)
        if (pressCylinder_0)
        {
            pressCylinder_0->m_targetSP.update(0);
            pressCylinder_0->stop();
        }
#endif
    };

    OperatorSwitch::State currentState = operatorSwitch_0->getState();
    if (currentState != this->lastOperatorSwitchState)
    {
        switch (currentState)
        {
        case OperatorSwitch::State::CYCLE_HELD:
#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)
            if (rs485)
            {
                RTU_Base *const *devices = rs485->deviceManager.getDevices();
                int numDevices = rs485->deviceManager.getMaxDevices();
                for (int i = 0; i < numDevices; ++i)
                {
                    if (devices[i] != nullptr && devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
                    {
                        OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
                        if (omron)
                        {
                            omron->run();
                        }
                    }
                }
            }
#endif
            break;
        case OperatorSwitch::State::STOP_HELD:
        {
            onCycleStop();
#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)
            if (rs485)
            {
                RTU_Base *const *devices = rs485->deviceManager.getDevices();
                int numDevices = rs485->deviceManager.getMaxDevices();
                for (int i = 0; i < numDevices; ++i)
                {
                    if (devices[i] != nullptr && devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
                    {
                        OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
                        if (omron)
                        {
                            omron->setSP(0);
                        }
                    }
                }
            }
#endif
            break;
        }
        case OperatorSwitch::State::BOTH_HELD:
            if (slaveToggleReady)
            {
                onSlaveModeChanged(true);
                updateRuntimeState();
                L_INFO("Slave Mode Enabled via BOTH_HELD");
                slaveToggleReady = false;
            }

            // Priority 1: Stop all temperature profiles
            for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
            {
                if (tempProfiles[i] != nullptr)
                {
                    tempProfiles[i]->stop();
                }
            }

            // Stop all signal plots
#if defined(ENABLE_PROFILE_SIGNAL_PLOT)
            for (int i = 0; i < PROFILE_SIGNAL_PLOT_COUNT; ++i)
            {
                if (signalPlots[i] != nullptr)
                {
                    signalPlots[i]->stop();
                }
            }
#endif

            // Set feedback to startup mode
#if defined(ENABLE_FEEDBACK_3C)
            if (feedback3C_0)
            {
                feedback3C_0->setMode(Feedback3C::Mode::MODE_STARTUP);
            }
#endif

// Priority 2: Stop all Omron controllers and set SP to 0
#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)
            if (rs485)
            {
                RTU_Base *const *devices = rs485->deviceManager.getDevices();
                int numDevices = rs485->deviceManager.getMaxDevices();
                for (int i = 0; i < numDevices; ++i)
                {
                    if (devices[i] != nullptr && devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
                    {
                        OmronE5 *omron = static_cast<OmronE5 *>(devices[i]);
                        if (omron)
                        {
                            omron->stop();
                            omron->setSP(0);
                        }
                    }
                }
            }
#endif

            break;
        case OperatorSwitch::State::STOP_PRESSED:
        {
            onCycleStop();
            break;
        }
        case OperatorSwitch::State::CYCLE_PRESSED:
        {
            bool any_running = false;
            bool any_paused = false;
            for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
            {
                if (tempProfiles[i] != nullptr && tempProfiles[i]->enabled())
                {
                    PlotStatus status = tempProfiles[i]->getCurrentStatus();
                    if (status == PlotStatus::RUNNING)
                    {
                        any_running = true;
                    }
                    else if (status == PlotStatus::PAUSED)
                    {
                        any_paused = true;
                    }
                }
            }

            if (any_running)
            {
                // Priority 1: If anything is running, pause everything that's running.
                for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
                {
                    if (tempProfiles[i] != nullptr && tempProfiles[i]->enabled() && tempProfiles[i]->getCurrentStatus() == PlotStatus::RUNNING)
                    {
                        tempProfiles[i]->pause();
                    }
                }
            }
            else if (any_paused)
            {
                // Priority 2: If nothing is running but something is paused, resume everything that's paused.
                for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
                {
                    if (tempProfiles[i] != nullptr && tempProfiles[i]->enabled() && tempProfiles[i]->getCurrentStatus() == PlotStatus::PAUSED)
                    {
                        tempProfiles[i]->resume();
                    }
                }
            }
            else
            {
                // Priority 3: If nothing is running or paused, start everything that's ready.
                for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
                {
                    if (tempProfiles[i] != nullptr && tempProfiles[i]->enabled())
                    {
                        PlotStatus status = tempProfiles[i]->getCurrentStatus();
                        if (status == PlotStatus::IDLE || status == PlotStatus::STOPPED || status == PlotStatus::FINISHED)
                        {
                            tempProfiles[i]->start();
                        }
                    }
                }
            }
        }
        break;
        default:
            // No action for other states
            break;
        }
    }
    this->lastOperatorSwitchState = currentState;
#endif
#if defined(ENABLE_FEEDBACK_BUZZER) && defined(ENABLE_PRESS_CYLINDER)
    if (feedbackBuzzer_0 && pressCylinder_0)
    {
        /*
        if(pressCylinder_0->isOverloaded()) {
            feedbackBuzzer_0->setMode(FeedbackBuzzer::E_BuzzerMode::MODE_LONG_BEEP_SHORT_PAUSE, 2000);
        } else {
            feedbackBuzzer_0->setMode(FeedbackBuzzer::E_BuzzerMode::MODE_OFF);
        }
        */
    }
#endif
}

#ifdef ENABLE_PLUNGER
void PHApp::onPlungerEvent(Plunger *instance, PlungerEvent event, int16_t val)
{
    if (!instance)
        return;

    switch (event)
    {
    case PlungerEvent::STATE_CHANGED:
        Log.traceln("Plunger state changed to: %d", val);
        break;

    case PlungerEvent::JAM_DETECTED:
        Log.warningln("Plunger jam detected");
#if defined(ENABLE_FEEDBACK_3C)
        if (feedback3C_0)
        {
            feedback3C_0->setMode(Feedback3C::Mode::MODE_ERROR);
        }
#endif
#if defined(ENABLE_FEEDBACK_BUZZER)
        if (feedbackBuzzer_0)
        {
            feedbackBuzzer_0->setMode(FeedbackBuzzer::E_BuzzerMode::MODE_LONG_BEEP_SHORT_PAUSE, 3000);
        }
#endif
        break;

    case PlungerEvent::JAM_CLEARED:
        Log.infoln("Plunger jam cleared");
#if defined(ENABLE_FEEDBACK_3C)
        if (feedback3C_0)
        {
            feedback3C_0->setMode(Feedback3C::Mode::MODE_STANDBY);
        }
#endif
#if defined(ENABLE_FEEDBACK_BUZZER)
        if (feedbackBuzzer_0)
        {
            feedbackBuzzer_0->setMode(FeedbackBuzzer::E_BuzzerMode::MODE_OFF);
        }
#endif
        break;

    case PlungerEvent::OPERATION_STARTED:
        Log.traceln("Plunger operation started");
        break;

    case PlungerEvent::OPERATION_COMPLETED:
        Log.traceln("Plunger operation completed");
        break;

    case PlungerEvent::HOMING_STARTED:
        Log.infoln("Plunger homing started");
        break;

    case PlungerEvent::HOMING_COMPLETED:
        Log.infoln("Plunger homing completed");
        break;

    case PlungerEvent::PLUNGING_STARTED:
        Log.infoln("Plunger plunging started");
        break;

    case PlungerEvent::PLUNGING_COMPLETED:
        Log.infoln("Plunger plunging completed");
        break;

    case PlungerEvent::FILL_STARTED:
        Log.infoln("Plunger fill sequence started");
        break;

    case PlungerEvent::FILL_COMPLETED:
        Log.infoln("Plunger fill sequence completed");
        break;

    case PlungerEvent::RECORD_STARTED:
        Log.infoln("Plunger recording started");
        break;

    case PlungerEvent::RECORD_COMPLETED:
        Log.infoln("Plunger recording completed, duration: %d ms", val);
        break;

    case PlungerEvent::REPLAY_STARTED:
        Log.infoln("Plunger replay started");
        break;

    case PlungerEvent::REPLAY_COMPLETED:
        Log.infoln("Plunger replay completed");
        break;

    case PlungerEvent::POST_FLOW_STARTED:
        Log.infoln("Plunger post-flow started");
        break;

    case PlungerEvent::POST_FLOW_COMPLETED:
        Log.infoln("Plunger post-flow completed");
        break;

    case PlungerEvent::AUTO_MODE_ENABLED:
        Log.infoln("Plunger auto mode enabled");
        break;

    case PlungerEvent::AUTO_MODE_DISABLED:
        Log.infoln("Plunger auto mode disabled");
        break;

    default:
        Log.traceln("Unknown plunger event: %d", static_cast<int>(event));
        break;
    }
}
#endif

void PHApp::onSlaveModeChanged(bool enableSlave)
{
    if (_is_slave == enableSlave)
    {
        return;
    }
    _is_slave = enableSlave;
    L_INFO("onSlaveModeChanged: %d", enableSlave);

#if defined(ENABLE_RS485) && defined(ENABLE_OMRON_E5)
    if (rs485)
    {
        RTU_Base *const *devices = rs485->deviceManager.getDevices();
        int numDevices = rs485->deviceManager.getMaxDevices();
        for (int i = 0; i < numDevices; ++i)
        {
            if (devices[i] && devices[i]->type == COMPONENT_TYPE::COMPONENT_TYPE_PID)
            {
                devices[i]->enable(!enableSlave);
            }
        }
    }
#endif

#ifdef ENABLE_LOADCELL_0
    if (loadCell_0)
        loadCell_0->enable(!enableSlave);
#endif
#ifdef ENABLE_LOADCELL_1
    if (loadCell_1)
        loadCell_1->enable(!enableSlave);
#endif

    updateRuntimeState();
}
#include "PHApp.h"
#include "profiles/TemperatureProfile.h"
#include "components/Feedback3C.h"
#include "components/OperatorSwitch.h"
#include "profiles/PlotBase.h"

void PHApp::loopCycle()
{
    // --- Application State Handling ---
    // Check for critical app errors first. This logic takes precedence.
    short appState = getAppState(0);
    bool hasError = (appState >= APP_STATE::ERROR || getLastError() != E_OK);

    if (hasError)
    {
#if defined(ENABLE_FEEDBACK_3C)        
    feedback3C_0->setMode(Feedback3C::Mode::MODE_ERROR);
#endif

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
        // Abort further cycle logic if the app is in an error state.
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
    if (anyProfilePaused)
    {
        feedback3C_0->setMode(Feedback3C::Mode::MODE_WARNING);
    }
    else if (anyProfileRunning)
    {
        feedback3C_0->setMode(Feedback3C::Mode::MODE_RUNNING);
    }
    else if (anyProfileFinished)
    {
        feedback3C_0->setMode(Feedback3C::Mode::MODE_STANDBY);
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
                Log.infoln("Operator switch CYCLE_HELD detected. Resuming profile %d.", i);
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
                    Log.infoln("Operator switch CYCLE_HELD detected. Starting profile %d.", i);
                    tempProfiles[i]->start();
                }
            }
        }
    };

    auto onCycleStop = [&]()
    {
        // Pause any running profiles
        for (int i = 0; i < PROFILE_TEMPERATURE_COUNT; ++i)
        {
            if (tempProfiles[i] != nullptr && tempProfiles[i]->getCurrentStatus() == PlotStatus::RUNNING)
            {
                Log.infoln("Operator switch STOP_HELD detected. Pausing profile %d.", i);
                tempProfiles[i]->pause();
            }
        }
    };

    switch (operatorSwitch_0->getState())
    {
    case OperatorSwitch::State::CYCLE_HELD:
        onCycleStart();
        break;
    case OperatorSwitch::State::STOP_HELD:
        onCycleStop();
        break;
    default:
        // No action for other states
        break;
    }
#endif
}
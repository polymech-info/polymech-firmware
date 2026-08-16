#include "PressCylinder.h"

#ifdef ENABLE_PRESS_CYLINDER

#include <ArduinoLog.h>
#include <LittleFS.h>
#include <modbus/ModbusTypes.h>
#include <Bridge.h>
#include "config.h"
#include "features.h"
#include "StringUtils.h"
#include "xmath.h"
#include "macros.h"

#include <components/Loadcell.h>
#include <components/Solenoid.h>
#include <components/Joystick.h>

constexpr PressCylinder::E_Mode PressCylinder::DEFAULT_AUTO_MODE_STOPPED;
constexpr PressCylinder::E_Mode PressCylinder::DEFAULT_AUTO_MODE_NORMAL;
constexpr PressCylinder::E_Mode PressCylinder::DEFAULT_AUTO_MODE_INTERLOCKED;

// #define ENABLE_MIN_LOAD_CHECK

void PressCylinderSettings::fromJSON(const JsonVariantConst &doc)
{
    interlocked = doc["interlocked"] | true;
    mode = doc["mode"] | (uint8_t)PressCylinder::MODE_AUTO_DETECT;
    homing_threshold = doc["homing_threshold"] | PRESS_CYLINDER_DEFAULT_HOMING_THRESHOLD;
    pressing_threshold = doc["pressing_threshold"] | PRESS_CYLINDER_DEFAULT_PRESSING_THRESHOLD;
    highload_threshold = doc["highload_threshold"] | PRESS_CYLINDER_DEFAULT_HIGHLOAD_THRESHOLD;
    maxload_threshold = doc["maxload_threshold"] | PRESS_CYLINDER_DEFAULT_MAXLOAD_THRESHOLD;
    maxload_threshold = clamp(maxload_threshold, (uint32_t)1, (uint32_t)PRESS_CYLINDER_MAX_LOAD_CLAMP);
    sp_deadband_percent = doc["sp_deadband_percent"] | PRESS_CYLINDER_DEFAULT_SP_DEADBAND_PERCENT;
    min_load = doc["min_load"] | PRESS_CYLINDER_DEFAULT_MIN_LOAD;
    balance_interval = doc["balance_interval"] | BALANCE_INTERVAL;
    balance_min_pv_diff = doc["balance_min_pv_diff"] | BALANCE_MIN_PV_DIFF;
    balance_max_pv_diff = doc["balance_max_pv_diff"] | BALANCE_MAX_PV_DIFF;
    max_stall_activations = doc["max_stall_activations"] | PRESS_CYLINDER_MAX_STALL_ACTIVATIONS;
}

void PressCylinderSettings::toJSON(JsonVariant doc)
{
    doc["homing_threshold"] = homing_threshold;
    doc["pressing_threshold"] = pressing_threshold;
    doc["highload_threshold"] = highload_threshold;
    doc["maxload_threshold"] = maxload_threshold;
    doc["sp_deadband_percent"] = sp_deadband_percent;
    doc["min_load"] = min_load;
    doc["balance_interval"] = balance_interval;
    doc["balance_min_pv_diff"] = balance_min_pv_diff;
    doc["balance_max_pv_diff"] = balance_max_pv_diff;
    doc["max_stall_activations"] = max_stall_activations;
    doc["interlocked"] = interlocked;
    doc["mode"] = mode;
}

void PressCylinder::_applyDefaultSettings()
{
    _settings.homing_threshold = PRESS_CYLINDER_DEFAULT_HOMING_THRESHOLD;
    _settings.pressing_threshold = PRESS_CYLINDER_DEFAULT_PRESSING_THRESHOLD;
    _settings.highload_threshold = PRESS_CYLINDER_DEFAULT_HIGHLOAD_THRESHOLD;
    _settings.maxload_threshold = PRESS_CYLINDER_DEFAULT_MAXLOAD_THRESHOLD;
    _settings.sp_deadband_percent = PRESS_CYLINDER_DEFAULT_SP_DEADBAND_PERCENT;
    _settings.min_load = PRESS_CYLINDER_DEFAULT_MIN_LOAD;
    _settings.balance_interval = BALANCE_INTERVAL;
    _settings.balance_min_pv_diff = BALANCE_MIN_PV_DIFF;
    _settings.balance_max_pv_diff = BALANCE_MAX_PV_DIFF;
    _settings.max_stall_activations = PRESS_CYLINDER_MAX_STALL_ACTIVATIONS;
    _settings.max_stall_activations = PRESS_CYLINDER_MAX_STALL_ACTIVATIONS;
    _settings.interlocked = true;
    _settings.mode = MODE_AUTO_DETECT;
}

PressCylinder::PressCylinder(Component *owner, short id, short mbAddress, Joystick *joystick, PushButton *pushButton)
    : NetworkComponent(mbAddress, "PressCylinder", id, COMPONENT_DEFAULT, owner),
      m_targetSP(this, id, "Target SP"),
      m_mode(this, id, "Mode(0:NONE,1:MANUAL,2:AUTO,3:MANUAL_MULTI,4:AUTO_MULTI,5:AUTO_MULTI_BALANCED,6:REMOTE,7:AUTO_DETECT)"),
      m_state(this, id, "State(0:IDLE,1:HOMING,2:PRESSING,3:HIGHLOAD,4:MAXLOAD,5:ERROR)"),
      m_error_code(this, id, "Error(0:OK,1:MINLOAD,2:OVERLOAD,3:BALANCE_MAX_DIFF,4:LOADCELL,5:MAX_TIME,6:STALLED,7:AUTO_TIMEOUT)"),
      m_cflags(this, id, "CFlags(1:MINLOAD,2:MAX_TIME,4:STALLED,8:BALANCE,16:LOADCELL,32:MULTI_TIMEOUT,64:DBL_CLK)"),
      m_outputMode(this, id, "OutputMode(1:HOLD)"),
      m_cmd(this, id, "Cmd(1:INFO,2:RESET)"),
      m_interlocked(this, id, "Interlocked"),
      _joystick(joystick),
      _pushButton(pushButton),
      _last_error_code(E_PC_OK),
      _last_balance_time(0),
      _auto_interlocked_start_time(0),
      _balanced_mode_active_cylinder(0),
      _previous_mode(MODE_AUTO_DETECT),
      _hold_sp(0)
{
    m_cflags.update(E_PC_OP_ALL);
    _applyDefaultSettings();
    for (uint8_t i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        _loadcells[i] = nullptr;
        _solenoids[i] = nullptr;
        _pv_names[i].format("PV %d", i);
        m_pvs[i] = new PressCylinderValue(this, id, _pv_names[i].c_str());
        _pvs_raw_previous[i] = 0;
    }
}

short PressCylinder::addLoadcell(Loadcell *loadcell)
{
    for (uint8_t i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (!_loadcells[i])
        {
            _loadcells[i] = loadcell;
            return E_OK;
        }
    }
    L_ERROR("Cannot add more loadcells to PressCylinder");
    return E_INVALID_PARAMETER;
}

short PressCylinder::addSolenoid(Solenoid *solenoid)
{
    for (uint8_t i = 0; i < PRESS_CYLINDER_MAX_SOLENOIDS; ++i)
    {
        if (!_solenoids[i])
        {
            _solenoids[i] = solenoid;
            return E_OK;
        }
    }
    L_ERROR("Cannot add more solenoids to PressCylinder");
    return E_INVALID_PARAMETER;
}

short PressCylinder::setup()
{
    NetworkComponent::setup();

    for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (m_pvs[i])
        {
            SETUP_NETWORK_VALUE((*m_pvs[i]), (MB_OFS_HR_PVS + i), FN_READ_HOLD_REGISTER, _pv_names[i].c_str(), 0, 2, NetworkValue_ThresholdMode::DIFFERENCE);
        }
    }
    SETUP_NETWORK_VALUE(m_targetSP, MB_OFS_HR_TARGET_SP, FN_WRITE_HOLD_REGISTER, "Target SP", 0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    SETUP_NETWORK_VALUE(m_mode, MB_OFS_HR_MODE, FN_WRITE_HOLD_REGISTER, "Mode(0:NONE,1:MANUAL,2:AUTO,3:MANUAL_MULTI,4:AUTO_MULTI,5:AUTO_MULTI_BALANCED,6:REMOTE,7:AUTO_DETECT)", MODE_AUTO_DETECT, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    SETUP_NETWORK_VALUE(m_state, MB_OFS_HR_STATE, FN_READ_HOLD_REGISTER, "State(0:IDLE,1:MAXLOAD,2:ERROR)", STATE_IDLE, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    SETUP_NETWORK_VALUE(m_error_code, MB_OFS_HR_ERROR_CODE, FN_READ_HOLD_REGISTER, "Error(0:OK,1:MINLOAD,2:OVERLOAD,3:BALANCE_MAX_DIFF,4:LOADCELL,5:MAX_TIME,6:STALLED,7:AUTO_TIMEOUT)", E_PC_OK, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    SETUP_NETWORK_VALUE(m_cflags, MB_OFS_HR_CFLAGS, FN_WRITE_HOLD_REGISTER, "CFlags(1:MINLOAD,2:MAX_TIME,4:STALLED,8:BALANCE,16:LOADCELL,32:MULTI_TIMEOUT)", E_PC_OP_ALL, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    SETUP_NETWORK_VALUE(m_outputMode, MB_OFS_HR_OUTPUT_MODE, FN_WRITE_HOLD_REGISTER, "OutputMode(1:HOLD)", E_PC_OM_NONE, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    SETUP_NETWORK_VALUE(m_cmd, MB_OFS_HR_CMD, FN_WRITE_HOLD_REGISTER, "Cmd(1:INFO,2:RESET)", 0, 1, NetworkValue_ThresholdMode::DIFFERENCE);
    SETUP_NETWORK_VALUE(m_interlocked, MB_OFS_C_INTERLOCKED, FN_WRITE_COIL, "Interlocked", true, 1, NetworkValue_ThresholdMode::DIFFERENCE);

    if (!_loadcells[0])
    {
        L_ERROR("PressCylinder setup failed: No Loadcell provided");
        return E_INVALID_PARAMETER;
    }
    if (!_solenoids[0])
    {
        L_ERROR("PressCylinder setup failed: No Solenoid provided");
        return E_INVALID_PARAMETER;
    }
    loadSettings();
    m_interlocked.update(_settings.interlocked);
    m_mode.update(_settings.mode);
    SOLENOIDS_OFF();
    return E_OK;
}

bool PressCylinder::pvsOk()
{
    uint8_t validCnt = 0;
    for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (_loadcells[i] && _loadcells[i]->enabled())
        {
            if (_loadcells[i]->getWeight(_pvs_raw[i]))
            {
                ++validCnt;
            }
            else
            {
                return false; // sensor error
            }
        }
    }
    return validCnt > 0;
}

const char *PressCylinder::getErrorString(int error_code)
{
    switch ((E_PC_ErrorCode)error_code)
    {
    case E_PC_OK:
        return "OK";
    case E_PC_MINLOAD:
        return "MINLOAD";
    case E_PC_OVERLOAD:
        return "OVERLOAD";
    case E_PC_BALANCE_MAX_DIFF:
        return "BALANCE_MAX_DIFF";
    case E_PC_LOADCELL_ERROR:
        return "LOADCELL_ERROR";
    case E_PC_MAX_TIME:
        return "MAX_TIME";
    case E_PC_STALLED:
        return "STALLED";
    case E_PC_AUTO_TIMEOUT:
        return "AUTO_TIMEOUT";
    default:
        return "UNKNOWN";
    }
}

void PressCylinder::onError(int error_code)
{
    // Log full state for debug
    info(0, 0);

    SOLENOIDS_OFF();
    m_state.update(STATE_ERROR);
    m_error_code.update(error_code);
    _last_error_code = (E_PC_ErrorCode)error_code;
    m_mode.update(DEFAULT_AUTO_MODE_STOPPED);
    JsonDocument doc;
    doc["type"] = "error";
    doc["source"] = this->name;
    doc["code"] = error_code;
    doc["msg"] = getErrorString(error_code);

    if (this->owner)
    {
        this->owner->onMessage(this->id, EC_DISPATCH, E_MessageFlags::E_MF_NEW, &doc, this);
    }
}

/**
 * @brief Handles control logic for single-cylinder operational modes.
 *
 * This function is responsible for modes that only operate on the first loadcell/solenoid pair (`_loadcells[0]`, `_solenoids[0]`).
 *
 * @param mode The operational mode to execute (e.g. MODE_AUTO, MODE_MANUAL).
 * @param target_sp The target setpoint to control against.
 * @return short E_OK on success, or an error code.
 */
short PressCylinder::loopSingle(E_Mode mode, uint32_t target_sp)
{
    uint32_t pv_raw = _pvs_raw[0];

    // MIN_LOAD safety check - prevent any activation below minimum load
    if ((m_cflags.getValue() & E_PC_OP_CHECK_MINLOAD) && pv_raw < _settings.min_load)
    {
        SOLENOID_OFF(0);
        m_state.update(STATE_ERROR);
        m_error_code.update(E_PC_MINLOAD);
        return E_PC_MINLOAD;
    }

    switch (mode)
    {
    case MODE_MANUAL:
    case MODE_AUTO:
    {
        uint32_t target_val = (target_sp > 0) ? ((_settings.maxload_threshold * target_sp) / 100) : 0;
        if (pv_raw < target_val)
        {
            SOLENOID_ON(0);
        }
        else
        {
            SOLENOID_OFF(0);
        }
    }
    break;
    default:
        break;
    }
    return E_OK;
}

/**
 * @brief Handles control logic for multi-cylinder operational modes.
 *
 * @param mode The operational mode to execute.
 * @param target_sp The target setpoint to control against.
 * @return short E_OK on success, or an error code.
 */
short PressCylinder::loopMulti(E_Mode mode, uint32_t target_sp)
{
    if (target_sp == 0)
    {
        SOLENOIDS_OFF();
        return E_OK;
    }

    auto is_lc_active = [&](int i)
    {
        return _loadcells[i] && _loadcells[i]->enabled();
    };

    // MIN_LOAD safety check - prevent any activation below minimum load
    if (m_cflags.getValue() & E_PC_OP_CHECK_MINLOAD)
    {
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
        {
            if (is_lc_active(i) && _pvs_raw[i] < _settings.min_load)
            {
                SOLENOIDS_OFF();
                m_state.update(STATE_ERROR);
                m_error_code.update(E_PC_MINLOAD);
                return E_PC_MINLOAD;
            }
        }
    }

    auto is_unbalanced = [&](uint16_t diff_threshold)
    {
        uint16_t min_pv = -1, max_pv = 0;
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
        {
            if (is_lc_active(i))
            {
                if (_pvs_raw[i] < min_pv)
                    min_pv = _pvs_raw[i];
                if (_pvs_raw[i] > max_pv)
                    max_pv = _pvs_raw[i];
            }
        }
        return (max_pv - min_pv) > diff_threshold;
    };

    auto has_stalled = [&](uint32_t last_pv)
    {
        if (!(m_cflags.getValue() & E_PC_OP_CHECK_STALLED))
            return false;

        uint32_t total_activations = 0;
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
        {
            if (_solenoids[i])
                total_activations += _solenoids[i]->getActivationCount();
        }

        if (total_activations < _settings.max_stall_activations)
            return false; // Not enough data

        uint32_t min_pv_stalled = UINT32_MAX;
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
        {
            if (_solenoids[i] && _solenoids[i]->getActivationCount() > _settings.max_stall_activations)
            {
                if (is_lc_active(i) && _pvs_raw[i] < last_pv + _settings.balance_min_pv_diff)
                {
                    return true;
                }
            }
        }
        return false;
    };

    uint32_t target_sp_raw = (_settings.maxload_threshold * target_sp) / 100;
    uint32_t target_with_deadband = target_sp_raw * (100 + _settings.sp_deadband_percent) / 100;

    auto canPress = [&](uint32_t pv_raw)
    {
        return pv_raw < target_sp_raw;
    };

    auto shouldRelease = [&](uint32_t pv_raw)
    {
        return pv_raw >= target_with_deadband;
    };

    auto manage_solenoid = [&](int i)
    {
        if (is_lc_active(i) && _solenoids[i])
        {
            if (_solenoids[i]->getValue())
            { // Solenoid is ON
                if (shouldRelease(_pvs_raw[i]))
                {
                    SOLENOID_OFF(i);
                }
            }
            else
            { // Solenoid is OFF
                if (canPress(_pvs_raw[i]))
                {
                    SOLENOID_ON(i);
                }
            }
        }
        else if (_solenoids[i])
        {
            SOLENOID_OFF(i);
        }
    };

    /** @brief Safety Check: Fatal Pressure Imbalance. Halts the system if the pressure difference between cylinders is dangerously high. */
    if (m_interlocked.getValue() && (m_cflags.getValue() & E_PC_OP_CHECK_BALANCE) && is_unbalanced(_settings.balance_max_pv_diff))
    {
        _last_error_code = E_PC_BALANCE_MAX_DIFF;
        L_ERROR("Fatal balance error! PV difference > %d. System halted. Operator reset required.", _settings.balance_max_pv_diff);
        onError(_last_error_code);
        return _last_error_code;
    }

    switch (mode)
    {
    case MODE_MANUAL_MULTI:
    case MODE_AUTO_MULTI:
    {
        if (m_interlocked.getValue() && is_unbalanced(_settings.balance_min_pv_diff))
        {
            L_WARN("PV difference too high in AUTO_MULTI, aborting.");
            onError(E_PC_BALANCE_MAX_DIFF);
            break;
        }
        uint32_t min_pv = UINT32_MAX;
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
        {
            if (is_lc_active(i) && _pvs_raw[i] < min_pv)
                min_pv = _pvs_raw[i];
        }
        if (has_stalled(min_pv))
        {
            onError(E_PC_STALLED);
            break;
        }
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; i++)
        {
            manage_solenoid(i);
        }
    }
    break;
    case MODE_AUTO_MULTI_BALANCED:
    {
        uint32_t min_pv = UINT32_MAX;
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
        {
            if (is_lc_active(i) && _pvs_raw[i] < min_pv)
                min_pv = _pvs_raw[i];
        }
        if (has_stalled(min_pv))
        {
            L_WARN("No progress in AUTO_MULTI_BALANCED, aborting.");
            onError(E_PC_STALLED);
            break;
        }

        // Check if cylinders are in balance using deadband
        bool cylinders_balanced = !is_unbalanced(PRESS_CYLINDER_DEFAULT_SP_DEADBAND_RAW);

        if (!cylinders_balanced)
        {
            // Out of balance: press on cylinder with lowest PV (most urgent)
            uint32_t lowest_pv = UINT32_MAX;
            int most_urgent_cylinder = 0;
            for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
            {
                if (is_lc_active(i) && _pvs_raw[i] < lowest_pv)
                {
                    lowest_pv = _pvs_raw[i];
                    most_urgent_cylinder = i;
                }
            }
            if (_balanced_mode_active_cylinder != most_urgent_cylinder)
            {
                L_INFO("AUTO_MULTI_BALANCED: Switching to most urgent cylinder %d (PV=%d)", most_urgent_cylinder, lowest_pv);
                SOLENOID_OFF(_balanced_mode_active_cylinder);
                _balanced_mode_active_cylinder = most_urgent_cylinder;
            }
            manage_solenoid(_balanced_mode_active_cylinder);
        }
        else
        {
            // Cylinders are balanced: use periodic pressing
            if (m_interlocked.getValue() && is_unbalanced(_settings.balance_min_pv_diff))
            {
                if (millis() - _last_balance_time > _settings.balance_interval)
                {
                    L_INFO("AUTO_MULTI_BALANCED: Balanced cylinders, periodic switch from %d to %d",
                           _balanced_mode_active_cylinder, 1 - _balanced_mode_active_cylinder);
                    _last_balance_time = millis();
                    _balanced_mode_active_cylinder = 1 - _balanced_mode_active_cylinder;
                    SOLENOID_OFF(1 - _balanced_mode_active_cylinder);
                }
            }
            else
            {
                _last_balance_time = millis();
            }
            manage_solenoid(_balanced_mode_active_cylinder);
        }
    }
    break;
    default:
        break;
    }
    return E_OK;
}

/**
 * @brief Main loop function for the PressCylinder component.
 *
 * This function orchestrates the overall control logic of the PressCylinder.
 * It handles mode switching, safety checks, and calls the appropriate loopSingle or loopMulti functions.
 *
 * @return short E_OK on success, or an error code.
 */

void PressCylinder::onJoystickUp()
{
    // Deprecated: Logic moved to loop() with virtual target SP
}

short PressCylinder::loop()
{
    Component::loop();

    if (millis() - _last_loop_time < PRESSCYLINDER_INTERVAL)
    {
        return E_OK;
    }
    _last_loop_time = millis();

    if (_joystick && _joystick->getPosition() == Joystick::E_POSITION::DOWN)
    {
        E_Mode current_mode = (E_Mode)m_mode.getValue();
        if (current_mode == MODE_AUTO || current_mode == MODE_AUTO_MULTI || current_mode == MODE_AUTO_MULTI_BALANCED || current_mode == MODE_AUTO_DETECT)
        {
            m_targetSP.update(0);
        }
        SOLENOIDS_OFF();
        m_mode.update(DEFAULT_AUTO_MODE_STOPPED);
        m_state.update(STATE_IDLE);
        return E_OK;
    }

    E_Mode current_mode = (E_Mode)m_mode.getValue();
    if (current_mode != _previous_mode)
    {
        /*
        if ((current_mode == MODE_AUTO || current_mode == MODE_AUTO_MULTI || current_mode == MODE_AUTO_MULTI_BALANCED) && m_targetSP.getValue() == 0)
        {
            SOLENOIDS_OFF();
            m_mode.update(MODE_MANUAL);
            m_state.update(STATE_IDLE);
            _previous_mode = MODE_MANUAL;
            return E_OK;
        }
        */
        if (current_mode == MODE_AUTO || current_mode == MODE_AUTO_MULTI || current_mode == MODE_AUTO_MULTI_BALANCED || current_mode == MODE_AUTO_DETECT)
        {
            if (m_interlocked.getValue())
            {
                _auto_interlocked_start_time = millis();
                for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
                {
                    _pvs_raw_previous[i] = _pvs_raw[i];
                }
                L_INFO("Auto mode started while interlocked, timeout timer started");
            }
        }
        else
        {
            _auto_interlocked_start_time = 0;
        }
        _previous_mode = current_mode;
    }

    /** @brief Safety timeout for the solenoid. */
    if (m_cflags.getValue() & E_PC_OP_CHECK_MAX_TIME)
    {
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; i++)
        {
            if (_solenoids[i] && _solenoids[i]->getCurrentOnDurationMs() > PRESS_MAX_PRESS_TIME)
            {
                L_WARN("Solenoid %d on for too long (%ums), turning off all solenoids for safety.", i, PRESS_MAX_PRESS_TIME);
                onError(E_PC_MAX_TIME);
                break;
            }
        }
    }

    /** @brief Auto mode timeout when interlocked. */
    if (current_mode == MODE_AUTO && m_interlocked.getValue() && _auto_interlocked_start_time > 0)
    {
        if (millis() - _auto_interlocked_start_time > PRESS_AUTO_TIMEOUT)
        {
            L_WARN("Auto mode interlocked timeout (%ums), aborting.", PRESS_AUTO_TIMEOUT);
            onError(E_PC_AUTO_TIMEOUT);
            return E_OK;
        }
    }

    /** @brief Multi auto mode timeout when interlocked (only if flag enabled). */
    if ((m_cflags.getValue() & E_PC_OP_CHECK_MULTI_TIMEOUT) &&
        (current_mode == MODE_AUTO_MULTI || current_mode == MODE_AUTO_MULTI_BALANCED) &&
        m_interlocked.getValue() && _auto_interlocked_start_time > 0)
    {
        if (millis() - _auto_interlocked_start_time > PRESS_AUTO_TIMEOUT)
        {
            L_WARN("Multi auto mode interlocked timeout (%ums), aborting.", PRESS_AUTO_TIMEOUT);
            onError(E_PC_AUTO_TIMEOUT);
            return E_OK;
        }
    }

    /** @brief 1. Handle disabled state */
    if (!enabled())
    {
        SOLENOIDS_OFF();
        m_state.update(STATE_IDLE);
        return E_OK;
    }

    if (m_cflags.getValue() & E_PC_OP_CHECK_LOADCELL)
    {
        uint8_t healthy_count = 0;
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; i++)
        {
            if (_loadcells[i] && _loadcells[i]->enabled())
            {
                if (_loadcells[i]->lastErrorCode != 0)
                {
                    m_interlocked.update(false);
                    L_ERROR("Loadcell %d failed with error code %d", i, _loadcells[i]->lastErrorCode);
                    return E_OK;
                }
                healthy_count++;
            }
        }

        if (!m_interlocked.getValue() && healthy_count == PRESS_CYLINDER_MAX_PAIRS)
        {
            m_interlocked.update(true);
        }
    }

    bool pv_ok = pvsOk();

    for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (_loadcells[i] && _loadcells[i]->enabled() && _pvs_raw[i] <= INT32_MAX)
        {
            int32_t pv_signed = (int32_t)_pvs_raw[i];
            if (pv_signed >= 0 && _pvs_raw[i] >= _settings.maxload_threshold)
            {
                if (current_mode == MODE_AUTO_MULTI_BALANCED)
                {
                    uint32_t balanced_threshold = (_settings.maxload_threshold * PRESS_CYLINDER_DEFAULT_OVERLOAD) / 100;
                    if (_pvs_raw[i] >= balanced_threshold)
                    {
                        onError(E_PC_OVERLOAD);
                        return E_OK;
                    }
                }
                else
                {
                    onError(E_PC_OVERLOAD);
                    return E_OK;
                }
            }
        }
    }

    _last_error_code = E_PC_OK;
    m_state.update(STATE_IDLE);

    if (!pv_ok && (m_cflags.getValue() & E_PC_OP_CHECK_MINLOAD))
    {
        _last_error_code = E_PC_MINLOAD;
    }
    m_error_code.update(_last_error_code);

    if (_last_error_code != E_PC_OK)
    {
        m_state.update(STATE_ERROR);
        SOLENOIDS_OFF();
        return E_OK;
    }

    for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (m_pvs[i])
        {
            m_pvs[i]->update(_loadcells[i] && _loadcells[i]->enabled() && _pvs_raw[i] <= INT32_MAX ? (_pvs_raw[i] * 100) / _settings.maxload_threshold : 0);
        }
    }

    /** @brief Advance timeout timer if significant PV progress detected during auto interlocked mode */
    if ((current_mode == MODE_AUTO || current_mode == MODE_AUTO_MULTI || current_mode == MODE_AUTO_MULTI_BALANCED || current_mode == MODE_AUTO_DETECT) &&
        m_interlocked.getValue() && _auto_interlocked_start_time > 0)
    {
        bool should_advance_timeout = false;

        // Check for significant PV changes
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
        {
            if (_loadcells[i] && _loadcells[i]->enabled())
            {
                uint32_t pv_diff = (_pvs_raw[i] > _pvs_raw_previous[i]) ? (_pvs_raw[i] - _pvs_raw_previous[i]) : (_pvs_raw_previous[i] - _pvs_raw[i]);
                if (pv_diff > _settings.balance_min_pv_diff)
                {
                    should_advance_timeout = true;
                    L_INFO("Significant PV change detected (cylinder %d: %d->%d, diff=%d), advancing timeout timer",
                           i, _pvs_raw_previous[i], _pvs_raw[i], pv_diff);
                    break;
                }
            }
        }

        // For MODE_AUTO_MULTI_BALANCED: advance timeout if cylinders are balanced and near target
        if (!should_advance_timeout && current_mode == MODE_AUTO_MULTI_BALANCED)
        {
            uint32_t target_sp_raw = (_settings.maxload_threshold * m_targetSP.getValue()) / 100;
            uint16_t min_pv = -1, max_pv = 0;
            bool near_target = true;

            for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
            {
                if (_loadcells[i] && _loadcells[i]->enabled())
                {
                    if (_pvs_raw[i] < min_pv)
                        min_pv = _pvs_raw[i];
                    if (_pvs_raw[i] > max_pv)
                        max_pv = _pvs_raw[i];
                    // Check if this cylinder is reasonably close to target (within 2x deadband)
                    if (_pvs_raw[i] < (target_sp_raw - (2 * _settings.sp_deadband_percent * target_sp_raw / 100)))
                    {
                        near_target = false;
                    }
                }
            }

            bool cylinders_balanced = (max_pv - min_pv) <= PRESS_CYLINDER_DEFAULT_SP_DEADBAND_RAW;

            if (cylinders_balanced && near_target)
            {
                should_advance_timeout = true;
            }
        }

        if (should_advance_timeout)
        {
            _auto_interlocked_start_time = millis();
        }
    }

    /** @brief Update previous PV values for next cycle */
    for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (_loadcells[i] && _loadcells[i]->enabled())
        {
            _pvs_raw_previous[i] = _pvs_raw[i];
        }
    }

    // Unified Control Logic: Determine Effective Mode and Target
    E_Mode base_mode = (E_Mode)m_mode.getValue();
    E_Mode effective_mode = base_mode;
    uint32_t effective_sp = m_targetSP.getValue();

    if (base_mode == MODE_AUTO_DETECT)
    {
        if (m_interlocked.getValue())
        {
            effective_mode = DEFAULT_AUTO_MODE_INTERLOCKED;
        }
        else
        {
            effective_mode = DEFAULT_AUTO_MODE_NORMAL;
        }
    }

    bool manual_input_active = false;

    // Check Inputs (Joystick only - PushButton is a modifier)
    if (_joystick && _joystick->getPosition() == Joystick::E_POSITION::UP)
    {
        manual_input_active = true;
        effective_sp = MANUAL_VIRTUAL_TARGET_PERCENT;

        // Check PushButton modifier
        bool button_pressed = _pushButton && _pushButton->enabled() && (_pushButton->getState() == PushButton::State::PRESSED || _pushButton->getState() == PushButton::State::HELD);

        if (button_pressed)
        {
            // Joystick UP + Button Pressed -> Force Manual Single Cylinder
            effective_mode = MODE_MANUAL;
        }
        else
        {
            // Joystick UP + Button Released -> Smart Auto / Interlock Dependent
            // Only apply "Smart Auto" if we are currently in a Manual mode (not overriding an existing Auto run)
            if (effective_mode == MODE_MANUAL || effective_mode == MODE_MANUAL_MULTI)
            {
                if (m_interlocked.getValue())
                {
                    effective_mode = DEFAULT_AUTO_MODE_INTERLOCKED; // e.g. AUTO_MULTI_BALANCED
                }
                else
                {
                    effective_mode = DEFAULT_AUTO_MODE_NORMAL; // e.g. AUTO
                }
            }
        }
    }

    if (manual_input_active || current_mode == MODE_AUTO || current_mode == MODE_AUTO_MULTI || current_mode == MODE_AUTO_MULTI_BALANCED || current_mode == MODE_AUTO_DETECT)
    {
        if (effective_mode == MODE_MANUAL_MULTI || effective_mode == MODE_AUTO_MULTI || effective_mode == MODE_AUTO_MULTI_BALANCED)
        {

            loopMulti(effective_mode, effective_sp);
        }
        else
        {

            loopSingle(effective_mode, effective_sp);
        }
    }
    else
    {
        // Idle / Stop (Manual input released and not in Auto mode)
        if (current_mode == MODE_MANUAL || current_mode == MODE_MANUAL_MULTI)
        {
            SOLENOIDS_OFF();
        }
        else
        {
            // For other modes, we might need to be careful not to turn off if they have their own logic,
            // but loopSingle/Multi covers most. REMOTE case is separate.
            // Existing logic had a 'loopSingle()' fallback for 'else'.
            // Here we explicitly turn off if standard manual/auto logic isn't running.
            SOLENOIDS_OFF();
        }
    }

    // Final safety override for manual modes
    // Final safety override for manual modes - REMOVED (Handled by Unified Logic above)

    return E_OK;
}

short PressCylinder::mb_tcp_write(MB_Registers *reg, short networkValue)
{
    if (!reg)
        return E_INVALID_PARAMETER;

    short result = NetworkComponent::mb_tcp_write(reg, networkValue);
    if (result != E_NOT_IMPLEMENTED)
    {
        // The base class handles enabled/disabled, and loop() will enforce safety.
        if (reg->startAddress == (mb_tcp_base_address() + E_NVC_ENABLED) && networkValue == 0)
        {
            SOLENOIDS_OFF();
        }
        return result;
    }

    uint16_t address = reg->startAddress;

    if (address == (mb_tcp_base_address() + MB_OFS_HR_TARGET_SP))
    {
        m_targetSP.update(networkValue);
        return E_OK;
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_MODE))
    {
        int sp = m_targetSP.getValue();
        if (m_mode.getValue() != (E_Mode)networkValue)
        {
            reset();
        }
        m_mode.update((E_Mode)networkValue);
        _settings.mode = (uint8_t)networkValue;
        saveSettings();
        if (sp > 0)
        {
            m_targetSP.update(sp);
        }
        return E_OK;
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_CMD))
    {
        E_Command cmd = (E_Command)networkValue;
        switch (cmd)
        {
        case CMD_INFO:
            info(0, 0);
            return E_OK;
        case CMD_RESET:
            reset();
            return E_OK;
        default:
            return E_INVALID_PARAMETER;
        }
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_CFLAGS))
    {
        m_cflags.update(networkValue);
        return E_OK;
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_OUTPUT_MODE))
    {
        uint16_t old_flags = m_outputMode.getValue();
        uint16_t new_flags = networkValue;

        // Check for HOLD flag rising edge
        if (!TEST_MASK(old_flags, E_PC_OM_HOLD) && TEST_MASK(new_flags, E_PC_OM_HOLD))
        {
            // Calculate SP from current load
            uint32_t max_pv = 0;
            for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
            {
                if (_loadcells[i] && _loadcells[i]->enabled() && _pvs_raw[i] > max_pv)
                {
                    max_pv = _pvs_raw[i];
                }
            }
            // Convert raw load to SP (0-100 scale based on maxload_threshold)
            if (_settings.maxload_threshold > 0)
            {
                _hold_sp = (max_pv * 100) / _settings.maxload_threshold;
                // Clamp to 100
                if (_hold_sp > 100)
                    _hold_sp = 100;

                m_targetSP.update(_hold_sp);
                L_INFO("HOLD activated: Captured SP=%d from PV=%d", _hold_sp, max_pv);
            }
        }

        m_outputMode.update(new_flags);
        return E_OK;
    }
    if (address == (mb_tcp_base_address() + MB_OFS_C_INTERLOCKED))
    {
        bool was_interlocked = m_interlocked.getValue();
        m_interlocked.update(networkValue);
        _settings.interlocked = m_interlocked.getValue();
        saveSettings();

        E_Mode current_mode = (E_Mode)m_mode.getValue();
        if ((current_mode == MODE_AUTO || current_mode == MODE_AUTO_MULTI || current_mode == MODE_AUTO_MULTI_BALANCED || current_mode == MODE_AUTO_DETECT))
        {
            if (networkValue && !was_interlocked)
            {
                _auto_interlocked_start_time = millis();
                for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
                {
                    _pvs_raw_previous[i] = _pvs_raw[i];
                }
                L_INFO("Interlocked activated during auto mode, timeout timer started");
            }
            else if (!networkValue && was_interlocked)
            {
                _auto_interlocked_start_time = 0;
                L_INFO("Interlocked deactivated, timeout timer reset");
            }
        }
        return E_OK;
    }
    return E_INVALID_PARAMETER;
}

short PressCylinder::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
        return 0;

    short result = NetworkComponent::mb_tcp_read(reg);
    if (result != E_NOT_IMPLEMENTED)
    {
        return result;
    }

    uint16_t address = reg->startAddress;

    if (address == (mb_tcp_base_address() + MB_OFS_HR_TARGET_SP))
    {
        return m_targetSP.getValue();
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_MODE))
    {
        return m_mode.getValue();
    }
    if (address >= (_baseAddress + MB_OFS_HR_PVS) && address < (_baseAddress + MB_OFS_HR_PVS + PRESS_CYLINDER_MAX_PAIRS))
    {
        int index = address - (_baseAddress + MB_OFS_HR_PVS);
        if (m_pvs[index])
        {
            return m_pvs[index]->getValue();
        }
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_STATE))
    {
        return m_state.getValue();
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_ERROR_CODE))
    {
        return m_error_code.getValue();
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_CFLAGS))
    {
        return m_cflags.getValue();
    }
    if (address == (mb_tcp_base_address() + MB_OFS_C_INTERLOCKED))
    {
        return m_interlocked.getValue();
    }
    if (address == (mb_tcp_base_address() + MB_OFS_HR_CMD))
    {
        return 0; // Write-only register
    }
    return 0;
}

bool PressCylinder::loadSettings(const char *filename)
{
    return true;
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
        L_WARN("Failed to open settings file: %s. Using default values.", filename);
        _applyDefaultSettings();
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        L_ERROR("Failed to parse settings file: %s", error.c_str());
        _applyDefaultSettings();
        return false;
    }

    _settings.fromJSON(doc.as<JsonVariant>());
    L_INFO("Successfully loaded settings from %s", filename);
    return true;
}

bool PressCylinder::saveSettings(const char *filename)
{
    File file = LittleFS.open(filename, "w");
    if (!file)
    {
        L_ERROR("Failed to create settings file: %s", filename);
        return false;
    }

    JsonDocument doc;
    _settings.toJSON(doc.to<JsonVariant>());

    if (serializeJson(doc, file) == 0)
    {
        file.close();
        L_ERROR("Failed to write to settings file: %s", filename);
        return false;
    }

    file.close();
    L_INFO("Successfully saved settings to %s", filename);
    return true;
}

short PressCylinder::stop()
{
    SOLENOIDS_OFF();
    _last_error_code = E_PC_OK;
    m_error_code.update(_last_error_code);
    m_state.update(STATE_IDLE);
    m_mode.update(DEFAULT_AUTO_MODE_STOPPED);
    for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (_solenoids[i])
        {
            _solenoids[i]->resetActivationCount();
        }
    }
    L_INFO("PressCylinder stopped");
    return E_OK;
}

short PressCylinder::reset()
{
    L_INFO("PressCylinder reset");
    stop();
    _auto_interlocked_start_time = 0;
    for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        _pvs_raw_previous[i] = 0;
    }
    return E_OK;
}

/*
short PressCylinder::onError(short errorCode, const char* errorMessage) {
    L_ERROR("PressCylinder error: %d - %s", errorCode, errorMessage);
    return E_OK;
}
*/

short PressCylinder::info(short val0, short val1)
{
    L_INFO("PressCylinder info");
    bool has_loadcells = false;
    for (uint8_t i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (_loadcells[i])
        {
            has_loadcells = true;
            uint32_t weight = 0;
            _loadcells[i]->getWeight(weight);
            L_INFO("Loadcell %d PV: %d, Local PV: %d", i, weight, m_pvs[i] ? m_pvs[i]->getValue() : -1);
        }
    }
    if (!has_loadcells)
    {
        L_INFO("Loadcell: null");
    }

    bool has_solenoids = false;
    for (uint8_t i = 0; i < PRESS_CYLINDER_MAX_SOLENOIDS; ++i)
    {
        if (_solenoids[i])
        {
            has_solenoids = true;
            L_INFO("Solenoid %d: %s", i, _solenoids[i]->getValue() ? "ON" : "OFF");
        }
    }
    if (!has_solenoids)
    {
        L_INFO("Solenoid: null");
    }
    if (_joystick)
    {
        L_INFO("Joystick: %d", _joystick->getPosition()); // TODO: add joystick position to info
    }
    else
    {
        L_INFO("Joystick: null");
    }
    L_INFO("PressCylinder: Mode=%d, State=%d, Error=%d, Interlocked=%d", m_mode.getValue(), m_state.getValue(), _last_error_code, m_interlocked.getValue());
    if (m_mode.getValue() == MODE_AUTO_MULTI_BALANCED)
    {
        uint16_t min_pv = -1, max_pv = 0;
        for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
        {
            if (_loadcells[i])
            {
                uint32_t weight = 0;
                if (_loadcells[i]->getWeight(weight))
                {
                    if (weight < min_pv)
                        min_pv = weight;
                    if (weight > max_pv)
                        max_pv = weight;
                }
            }
        }
        bool cylinders_balanced = (max_pv - min_pv) <= PRESS_CYLINDER_DEFAULT_SP_DEADBAND_RAW;
        L_INFO("Balanced Mode: Active cylinder=%d, Status=%s (diff=%d)",
               _balanced_mode_active_cylinder,
               cylinders_balanced ? "Periodic" : "Urgent",
               max_pv - min_pv);
    }
    L_INFO("Settings: TargetSP=%d, Homing=%d, Pressing=%d, Highload=%d, Maxload=%d, MinLoad=%d",
           m_targetSP.getValue(),
           _settings.homing_threshold,
           _settings.pressing_threshold,
           _settings.highload_threshold,
           _settings.maxload_threshold,
           _settings.min_load);
    L_INFO("Balance Settings: Interval=%d, MinDiff=%d, MaxDiff=%d, StallActivations=%d",
           _settings.balance_interval,
           _settings.balance_min_pv_diff,
           _settings.balance_max_pv_diff,
           _settings.max_stall_activations);
    if (_auto_interlocked_start_time > 0)
    {
        unsigned long elapsed = millis() - _auto_interlocked_start_time;
        L_INFO("Auto Mode Interlocked: Elapsed=%ums, Timeout=%ums, Remaining=%ums",
               elapsed, PRESS_AUTO_TIMEOUT, PRESS_AUTO_TIMEOUT - elapsed);
        L_INFO("Previous PV values: %d, %d (for progress tracking)",
               _pvs_raw_previous[0], _pvs_raw_previous[1]);
    }
    if (_loadcells[0])
    {
        L_INFO("Loadcell 0: %d", _loadcells[0]->info());
    }
    if (_loadcells[1])
    {
        L_INFO("Loadcell 1: %d", _loadcells[1]->info());
    }
    return E_OK;
}

bool PressCylinder::isOverloaded()
{
    for (int i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i)
    {
        if (_loadcells[i] && _pvs_raw[i] <= INT32_MAX)
        {
            int32_t pv_signed = (int32_t)_pvs_raw[i];
            if (pv_signed >= 0 && _pvs_raw[i] >= _settings.maxload_threshold)
            {
                return true;
            }
        }
    }
    return false;
}

#endif // ENABLE_PRESS_CYLINDER

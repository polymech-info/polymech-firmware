#ifndef PRESS_CYLINDER_H
#define PRESS_CYLINDER_H

#include "config.h"
#include <ArduinoJson.h>

#ifdef ENABLE_PRESS_CYLINDER

#include <modbus/NetworkComponent.h>
#include <NetworkValue.h>
#include "xmath.h"
#include "StringUtils.h"
#include "components/Joystick.h"

class Loadcell;
class Solenoid;
class PushButton;

// Default pressure thresholds for states
#define PRESS_CYLINDER_DEFAULT_HOMING_THRESHOLD 5
#define PRESS_CYLINDER_DEFAULT_PRESSING_THRESHOLD 20
#define PRESS_CYLINDER_DEFAULT_MAXLOAD_THRESHOLD 3200
#define PRESS_CYLINDER_DEFAULT_HIGHLOAD_THRESHOLD 3000
#define PRESS_CYLINDER_DEFAULT_OVERLOAD 120
#define PRESS_CYLINDER_DEFAULT_SP_DEADBAND_PERCENT 5
#define PRESS_CYLINDER_DEFAULT_SP_DEADBAND_RAW 20
#define PRESS_CYLINDER_DEFAULT_MIN_LOAD 10

#define PRESS_CYLINDER_DEFAULT_NO_LOAD 140
#define PRESS_CYLINDER_DEFAULT_NO_LOAD_SWITCH_INERVAL 1500

#define PRESS_CYLINDER_MAX_LOAD_CLAMP 4000

#define PRESS_MAX_PRESS_TIME 35000
#define PRESS_AUTO_TIMEOUT 35000
#define PRESS_CYLINDER_AUTO_MODE_HOLD_DURATION_MS 1500
#define BALANCE_INTERVAL 20
#define BALANCE_MIN_PV_DIFF 50
#define BALANCE_MAX_PV_DIFF 250

#define PRESSCYLINDER_INTERVAL 20
#define PRESS_CYLINDER_MAX_STALL_ACTIVATIONS 4
#define PRESS_CYLINDER_MB_COUNT 12
#define PRESS_CYLINDER_MAX_PAIRS 2
#define PRESS_CYLINDER_MAX_LOADCELLS PRESS_CYLINDER_MAX_PAIRS
#define PRESS_CYLINDER_MAX_SOLENOIDS PRESS_CYLINDER_MAX_PAIRS

#define SOLENOIDS_ON()                                     \
    for (uint8_t i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i) \
        if (_solenoids[i])                                 \
    _solenoids[i]->setValue(true)
#define SOLENOIDS_OFF()                                    \
    for (uint8_t i = 0; i < PRESS_CYLINDER_MAX_PAIRS; ++i) \
        if (_solenoids[i])                                 \
    _solenoids[i]->setValue(false)

#define SOLENOID_ON(idx)                                   \
    if (idx < PRESS_CYLINDER_MAX_PAIRS && _solenoids[idx]) \
    _solenoids[idx]->setValue(true)
#define SOLENOID_OFF(idx)                                  \
    if (idx < PRESS_CYLINDER_MAX_PAIRS && _solenoids[idx]) \
    _solenoids[idx]->setValue(false)

using PressCylinderValue = NetworkValue<uint16_t>;

struct PressCylinderSettings
{
    uint32_t homing_threshold;
    uint32_t pressing_threshold;
    uint32_t highload_threshold;
    uint32_t maxload_threshold;
    uint8_t sp_deadband_percent;
    uint32_t min_load;
    uint32_t balance_interval;
    uint32_t balance_min_pv_diff;
    uint32_t balance_max_pv_diff;
    uint8_t max_stall_activations;
    bool interlocked;
    uint8_t mode;

    void fromJSON(const JsonVariantConst &doc);
    void toJSON(JsonVariant doc);
};

class PressCylinder : public NetworkComponent<PRESS_CYLINDER_MB_COUNT>
{
public:
    enum E_State
    {
        STATE_IDLE,
        STATE_MAXLOAD,
        STATE_ERROR
    };
    enum E_PC_ErrorCode
    {
        E_PC_OK = 0,
        E_PC_MINLOAD,
        E_PC_OVERLOAD,
        E_PC_BALANCE_MAX_DIFF,
        E_PC_LOADCELL_ERROR,
        E_PC_MAX_TIME,
        E_PC_STALLED,
        E_PC_AUTO_TIMEOUT
    };
    enum E_Command
    {
        CMD_INFO = 1,
        CMD_RESET
    };
    enum E_Mode
    {
        MODE_NONE = 0,
        MODE_MANUAL,
        MODE_AUTO,
        MODE_MANUAL_MULTI,
        MODE_AUTO_MULTI,
        MODE_AUTO_MULTI_BALANCED,
        MODE_REMOTE,
        MODE_AUTO_DETECT
    };
    enum E_PC_OpFlags
    {
        E_PC_OP_NONE = 0,
        E_PC_OP_CHECK_MINLOAD = 1 << 0,
        E_PC_OP_CHECK_MAX_TIME = 1 << 1,
        E_PC_OP_CHECK_STALLED = 1 << 2,
        E_PC_OP_CHECK_BALANCE = 1 << 3,
        E_PC_OP_CHECK_LOADCELL = 1 << 4,
        E_PC_OP_CHECK_MULTI_TIMEOUT = 1 << 5,
        // E_PC_OP_ENABLE_DOUBLE_CLICK = 1 << 6, // Removed
        // E_PC_OP_ALL = E_PC_OP_CHECK_MAX_TIME | E_PC_OP_CHECK_STALLED | E_PC_OP_CHECK_BALANCE | E_PC_OP_CHECK_LOADCELL,
        E_PC_OP_ALL = E_PC_OP_CHECK_MAX_TIME
    };
    enum E_PC_OutputMode
    {
        E_PC_OM_NONE = 0,
        E_PC_OM_HOLD = 1 << 0
    };
    enum E_MB_Offset
    {
        MB_OFS_HR_PVS = E_NVC_USER + 0,
        MB_OFS_HR_TARGET_SP = E_NVC_USER + PRESS_CYLINDER_MAX_PAIRS,
        MB_OFS_HR_MODE,
        MB_OFS_HR_STATE,
        MB_OFS_HR_CMD,
        MB_OFS_HR_ERROR_CODE,
        MB_OFS_HR_CFLAGS,
        MB_OFS_HR_OUTPUT_MODE,
        MB_OFS_C_INTERLOCKED
    };

private:
    E_PC_ErrorCode _last_error_code;
    unsigned long _last_loop_time;
    Loadcell *_loadcells[PRESS_CYLINDER_MAX_PAIRS];
    Solenoid *_solenoids[PRESS_CYLINDER_MAX_PAIRS];
    Joystick *_joystick;
    PushButton *_pushButton;
    unsigned long _last_balance_time;
    unsigned long _auto_interlocked_start_time;
    uint32_t _pvs_raw_previous[PRESS_CYLINDER_MAX_PAIRS];
    uint8_t _balanced_mode_active_cylinder;
    XString<10> _pv_names[PRESS_CYLINDER_MAX_PAIRS];
    uint32_t _pvs_raw[PRESS_CYLINDER_MAX_PAIRS];
    bool _pvs_valid[PRESS_CYLINDER_MAX_PAIRS];
    E_Mode _previous_mode;
    uint32_t _hold_sp;

public:
    PressCylinderValue *m_pvs[PRESS_CYLINDER_MAX_PAIRS];
    PressCylinderValue m_targetSP;
    PressCylinderValue m_mode;
    PressCylinderValue m_state;
    PressCylinderValue m_error_code;
    PressCylinderValue m_cflags;
    PressCylinderValue m_outputMode;
    PressCylinderValue m_cmd;
    NetworkValue<bool> m_interlocked;
    PressCylinderSettings _settings;

    PressCylinder(Component *owner, short id, short mbAddress, Joystick *joystick, PushButton *pushButton);

    short addLoadcell(Loadcell *loadcell);
    short addSolenoid(Solenoid *solenoid);

    short setup() override;
    short loop() override;

    void onJoystickUp();

    static constexpr E_Mode DEFAULT_AUTO_MODE_STOPPED = MODE_AUTO_DETECT;
    static constexpr E_Mode DEFAULT_AUTO_MODE_NORMAL = MODE_AUTO;
    static constexpr E_Mode DEFAULT_AUTO_MODE_INTERLOCKED = MODE_AUTO_MULTI_BALANCED;
    static constexpr uint8_t MANUAL_VIRTUAL_TARGET_PERCENT = 70;

    short mb_tcp_write(MB_Registers *reg, short networkValue) override;
    short mb_tcp_read(MB_Registers *reg) override;

    bool loadSettings(const char *filename = "/press-cylinder.json");
    bool saveSettings(const char *filename = "/press-cylinder.json");

    short stop() override;
    short reset() override;

    bool isOverloaded();

    short info(short val0, short val1);

private:
    void _applyDefaultSettings();
    bool pvsOk();
    const char *getErrorString(int error_code);
    void onError(int error_code);
    short loopSingle(E_Mode mode, uint32_t targetVal);
    short loopMulti(E_Mode mode, uint32_t targetVal);
};

#endif // ENABLE_PRESS_CYLINDER
#endif // PRESS_CYLINDER_H
#ifndef MODBUSH_H
#define MODBUSH_H

#include "../macros.h"

#ifndef LOW_WORD
#define LOW_WORD(lw) ((uint16_t)(((uint32_t)(lw)) & 0xFFFF))
#endif
#ifndef HIGH_WORD
#define HIGH_WORD(hw) ((uint16_t)((((uint32_t)(hw)) >> 16) & 0xFFFF))
#endif

/**
 * @file Modbus.h
 * @brief Modbus communication header for ESP-32 industrial applications
 * @author Polymech Development Team
 * @revision doxygen comments and structure
 * @date 2024
 */

#include <inttypes.h>
#include "ModbusTypes.h"

/**
 * @defgroup ModbusMacros Modbus Macros
 * @brief Macros for initializing Modbus communication blocks
 * @details These macros provide a standardized way to initialize Modbus block structures
 * for both TCP and standard interfaces, automating address calculations and parameter setting.
 * @ingroup ModbusCommunication
 * @{
 */

/**
 * @brief Initializes Modbus block with TCP base address
 * @param tcpBaseAddr TCP base address for calculation
 * @param offset_enum Enum value representing address offset
 * @param fn_code Modbus function code
 * @param access Access flags for read/write permissions
 * @param desc Description string for documentation
 * @param group Group identifier for organization
 * @ingroup ModbusMacros
 */
#define INIT_MODBUS_BLOCK_TCP(tcpBaseAddr, offset_enum, fn_code, access, desc, group) \
    { \
        static_cast<ushort>(tcpBaseAddr + static_cast<ushort>(offset_enum)), \
        1, \
        fn_code, \
        access, \
        this->id, \
        this->slaveId, \
        desc, \
        group \
    }

/**
 * @brief Initializes standard Modbus block
 * @note Requires tcpBaseAddr variable in scope
 * @param offset_enum Enum value representing address offset
 * @param fn_code Modbus function code
 * @param access Access flags for read/write permissions
 * @param desc Description string for documentation
 * @param group Group identifier for organization
 * @ingroup ModbusMacros
 */
#define INIT_MODBUS_BLOCK(offset_enum, fn_code, access, desc, group) \
    { \
        static_cast<ushort>(tcpBaseAddr + static_cast<ushort>(offset_enum)), \
        1, \
        fn_code, \
        access, \
        this->id, \
        this->slaveId, \
        desc, \
        group \
    }


/**
 * @brief Initializes standard Modbus block
 * @note Requires tcpBaseAddr variable in scope
 * @param offset_enum Enum value representing address offset
 * @param fn_code Modbus function code
 * @param desc Description string for documentation
 * @param group Group identifier for organization
 * @ingroup ModbusMacros
 */
#define MB_BLOCK(offset_enum, fn_code, desc, group) \
    { \
        static_cast<ushort>(tcpBaseAddr + static_cast<ushort>(offset_enum)), \
        1, \
        fn_code, \
        MB_ACCESS_READ_WRITE, \
        this->id, \
        this->slaveId, \
        desc, \
        group \
    }

/** @} */

/**
 * @defgroup ModbusValueWrapperMacros Modbus Value Wrapper Macros
 * @brief Macros for initializing Modbus blocks and associated ValueWrappers together.
 * @ingroup ModbusCommunication
 * @{
 */

#define _GET_OVERLOADED_MACRO(M, ...) _OVR(M, _VA_NUM_ARGS(__VA_ARGS__)) (__VA_ARGS__)
#define _OVR(M, N) M##N
#define _VA_NUM_ARGS(...) _VA_NUM_ARGS_IMPL(__VA_ARGS__, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define _VA_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, N, ...) N

/**
 * @brief Initializes a ValueWrapper and its corresponding Modbus block in one go.
 *
 * This macro should be called from within a component's constructor body.
 * It initializes a `ValueWrapper` member and a corresponding `MB_Registers` entry
 * in the `_modbusBlocks` member array.
 *
 * @param vw_member The `ValueWrapper<T>` member variable to initialize.
 * @param vw_type The data type `T` for the `ValueWrapper`.
 * @param mb_reg_offset_enum The Modbus register offset (from an enum).
 * @param mb_fn_code The Modbus function code for both notification and the register.
 * @param mb_desc A description for the Modbus register.
 * @param mb_group A group name for the Modbus register.
 * @param vw_initial_val The initial value for the `ValueWrapper`.
 * @param vw_threshold_val The threshold value for triggering notifications.
 * @param vw_threshold_mode The threshold comparison mode.
 * @param vw_post_notify_cb An optional callback to run after a notification is sent.
 */
#define MB_REG_EX(...) CAT(MB_REG_EX_, NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define MB_REG_EX_10( \
    vw_member, \
    vw_type, \
    mb_reg_offset_enum, \
    mb_fn_code, \
    mb_desc, \
    mb_group, \
    vw_initial_val, \
    vw_threshold_val, \
    vw_threshold_mode, \
    vw_post_notify_cb \
) \
do { \
    (vw_member) = ValueWrapper<vw_type>( \
        this->owner, \
        this->id, \
        [this]() -> uint16_t { return this->mb_tcp_base_address(); }, \
        static_cast<uint16_t>(mb_reg_offset_enum), \
        mb_fn_code, \
        static_cast<uint8_t>(this->slaveId), \
        vw_initial_val, \
        vw_threshold_val, \
        vw_threshold_mode, \
        vw_post_notify_cb \
    ); \
    (_modbusBlocks)[static_cast<uint16_t>(mb_reg_offset_enum)] = MB_Registers( \
        this->mb_tcp_base_address() + static_cast<uint16_t>(mb_reg_offset_enum), \
        1, \
        mb_fn_code, \
        MB_ACCESS_READ_WRITE, \
        this->id, \
        static_cast<uint16_t>(this->slaveId), \
        mb_desc, \
        mb_group \
    ); \
} while(0)

#define MB_REG_EX_9( \
    vw_member, \
    vw_type, \
    mb_reg_offset_enum, \
    mb_fn_code, \
    mb_desc, \
    mb_group, \
    vw_initial_val, \
    vw_threshold_val, \
    vw_threshold_mode \
) \
MB_REG_EX_10( \
    vw_member, \
    vw_type, \
    mb_reg_offset_enum, \
    mb_fn_code, \
    mb_desc, \
    mb_group, \
    vw_initial_val, \
    vw_threshold_val, \
    vw_threshold_mode, \
    nullptr \
)

/*
// Expanded reference for:
MB_REG_EX(
    _statusWrapper, PlotStatus,
    TemperatureProfileRegisterOffset::STATUS, E_FN_CODE::FN_READ_HOLD_REGISTER,
    "TProf Status", "TempProfile_2001_Slot_0",
    PlotStatus::IDLE, PlotStatus::RUNNING, ValueWrapper<PlotStatus>::ThresholdMode::DIFFERENCE,
    nullptr
);

// Expands to:
do {
    _statusWrapper = ValueWrapper<PlotStatus>(
        this->owner,
        this->id,
        [this]() -> uint16_t { return this->mb_tcp_base_address(); },
        static_cast<uint16_t>(TemperatureProfileRegisterOffset::STATUS),
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        static_cast<uint8_t>(this->slaveId),
        PlotStatus::IDLE,
        PlotStatus::RUNNING,
        ValueWrapper<PlotStatus>::ThresholdMode::DIFFERENCE,
        nullptr
    );
    _modbusBlocks[static_cast<uint16_t>(TemperatureProfileRegisterOffset::STATUS)] = MB_Registers(
        this->mb_tcp_base_address() + static_cast<uint16_t>(TemperatureProfileRegisterOffset::STATUS),
        1,
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        MB_ACCESS_READ_WRITE,
        this->id,
        static_cast<uint16_t>(this->slaveId),
        "TProf Status",
        "TempProfile_2001_Slot_0"
    );
} while(0);
*/

/**
 * @brief Initializes a ValueWrapper and its corresponding Modbus block using the
 * simplified INIT_COMPONENT_VALUE_WRAPPER.
 *
 * This version assumes notifications are always enabled and the slave ID is 1.
 * It offers a simpler parameter list for common use cases.
 */
#define MB_REG( \
    vw_member, \
    vw_type, \
    mb_reg_offset_enum, \
    mb_fn_code, \
    mb_desc, \
    mb_group, \
    vw_initial_val, \
    vw_threshold_val, \
    vw_threshold_mode, \
    vw_post_notify_cb \
) \
do { \
    (vw_member) = INIT_COMPONENT_VALUE_WRAPPER( \
        vw_type, \
        mb_reg_offset_enum, \
        mb_fn_code, \
        vw_initial_val, \
        vw_threshold_val, \
        vw_threshold_mode, \
        vw_post_notify_cb \
    ); \
    (_modbusBlocks)[static_cast<uint16_t>(mb_reg_offset_enum)] = MB_Registers( \
        this->mb_tcp_base_address() + static_cast<uint16_t>(mb_reg_offset_enum), \
        1, \
        mb_fn_code, \
        MB_ACCESS_READ_WRITE, \
        this->id, \
        static_cast<uint16_t>(this->slaveId), \
        mb_desc, \
        mb_group \
    ); \
} while(0)

/*
// Expanded reference for:
MB_REG(
    _statusWrapper, PlotStatus,
    TemperatureProfileRegisterOffset::STATUS, E_FN_CODE::FN_READ_HOLD_REGISTER,
    "TProf Status", "TempProfile_2001_Slot_0",
    PlotStatus::IDLE, PlotStatus::RUNNING, ValueWrapper<PlotStatus>::ThresholdMode::DIFFERENCE,
    nullptr
);

// Expands to:
do {
    _statusWrapper = ValueWrapper<PlotStatus>(
        this->owner,
        this->id,
        [this]() -> uint16_t { return this->mb_tcp_base_address(); },
        static_cast<uint16_t>(TemperatureProfileRegisterOffset::STATUS),
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        static_cast<uint8_t>(this->slaveId),
        PlotStatus::IDLE,
        PlotStatus::RUNNING,
        ValueWrapper<PlotStatus>::ThresholdMode::DIFFERENCE,
        nullptr
    );
    _modbusBlocks[static_cast<uint16_t>(TemperatureProfileRegisterOffset::STATUS)] = MB_Registers(
        this->mb_tcp_base_address() + static_cast<uint16_t>(TemperatureProfileRegisterOffset::STATUS),
        1,
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        MB_ACCESS_READ_WRITE,
        this->id,
        static_cast<uint16_t>(this->slaveId),
        "TProf Status",
        "TempProfile_2001_Slot_0"
    );
} while(0);
*/


/**
 * @brief Initializes a ValueWrapper and Modbus block with auto-stringified description.
 * 
 * @param vw_member The ValueWrapper<T> member variable to initialize.
 * @param vw_type The data type `T` for the ValueWrapper.
 * @param enum_type The enum type (e.g., TemperatureProfileRegisterOffset).
 * @param enum_member The enum member (e.g., STATUS).
 * @param fn_code The Modbus function code.
 * @param prefix A string prefix for the auto-generated description.
 * @param group A group name for the Modbus register.
 */
#define MB_REG_EX_A(vw_member, vw_type, enum_type, enum_member, fn_code, prefix, group, ...) \
    MB_REG_EX(vw_member, vw_type, enum_type::enum_member, fn_code, prefix " " #enum_member, group, __VA_ARGS__)

/**
 * @brief Declares a ValueWrapper member and an inline initializer function for it.
 * 
 * @param vw_member The ValueWrapper<T> member variable to declare.
 * @param vw_type The data type `T` for the ValueWrapper.
 * @param enum_type The enum type for the register offset (e.g., TemperatureProfileRegisterOffset).
 * @param enum_member The enum member for the register offset (e.g., STATUS).
 * @param fn_code The Modbus function code.
 * @param prefix A string prefix for the auto-generated description.
 * @param ... The rest of the arguments for the ValueWrapper constructor.
 */
#define MB_WVAR_H(vw_member, vw_type, enum_type, enum_member, fn_code, prefix, ...) \
    ValueWrapper<vw_type> vw_member; \
    inline void init_##vw_member() { \
        const char* group = this->name.c_str(); \
        const uint16_t tcpBaseAddr = this->mb_tcp_base_address(); \
        MB_REG_EX_A(vw_member, vw_type, enum_type, enum_member, fn_code, prefix, group, __VA_ARGS__); \
    }

#endif

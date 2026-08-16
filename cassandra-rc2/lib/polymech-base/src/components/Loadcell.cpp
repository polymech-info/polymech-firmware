#include "config.h"
#include <Logger.h>
#include <modbus/ModbusTypes.h>
#include <modbus/Modbus.h>
#include "RS485.h"
#include "NetworkValue.h"
#include "Loadcell.h"
//
// DO NOT ENABLE DEBUG_LOADCELL_MB IN PRODUCTION
// #define DEBUG_LOADCELL_MB
// RTU Register Addresses from documentation
#define LOADCELL_RTU_REAL_TIME_WEIGHT_LOW 0
#define LOADCELL_RTU_REAL_TIME_WEIGHT_HIGH 1
#define LOADCELL_RTU_REAL_TIME_VOLTAGE_LOW 2
#define LOADCELL_RTU_REAL_TIME_VOLTAGE_HIGH 3
#define LOADCELL_RTU_ZERO_VOLTAGE_LOW 4
#define LOADCELL_RTU_ZERO_VOLTAGE_HIGH 5

#define LOADCELL_READ_BLOCK_START_ADDR 0
#define LOADCELL_READ_BLOCK_REG_COUNT 5

// #define DEBUG_LOADCELL_MB

Loadcell::Loadcell(Component *owner, uint8_t slaveId, millis_t readInterval, short component_id)
    : RTU_Base(owner, slaveId, component_id),
      _readInterval(readInterval),
      _weightLow(0), _weightHigh(0),
      _voltageLow(0), _voltageHigh(0),
      _zeroVoltageLow(0), _zeroVoltageHigh(0),
      _lastUpdateTime(0),
      _timeoutReported(false),
      INIT_MODBUS_NETWORK_VALUE(_weightWrapper, "PV", 0, 1, NetworkValue_ThresholdMode::DIFFERENCE, nullptr),
      INIT_MODBUS_NETWORK_VALUE(_voltageWrapper, "Voltage", 0, 1, NetworkValue_ThresholdMode::DIFFERENCE, nullptr),
      INIT_MODBUS_NETWORK_VALUE(_zeroVoltageWrapper, "Zero Voltage", 0, 1, NetworkValue_ThresholdMode::DIFFERENCE, nullptr)
{
    _modbusHelper.init(this);
    pFlags = E_PersistenceFlags::E_PF_ENABLED;
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    type = COMPONENT_TYPE::COMPONENT_TYPE_SENSOR;
    name = "Loadcell[" + String(slaveId) + "]";
    const uint16_t tcpBaseAddr = mb_tcp_base_address();

    _weightWrapper.initModbus(tcpBaseAddr + (ushort)E_LoadcellTcpOffset::PV, 1, this->id, this->slaveId, E_FN_CODE::FN_READ_HOLD_REGISTER, "PV", name.c_str());
    _voltageWrapper.initModbus(tcpBaseAddr + (ushort)E_LoadcellTcpOffset::VOLTAGE, 1, this->id, this->slaveId, E_FN_CODE::FN_READ_HOLD_REGISTER, "Voltage", name.c_str());
    _zeroVoltageWrapper.initModbus(tcpBaseAddr + (ushort)E_LoadcellTcpOffset::ZERO_VOLTAGE, 1, this->id, this->slaveId, E_FN_CODE::FN_READ_HOLD_REGISTER, "Zero Voltage", name.c_str());

    _modbusHelper.registerBlock(_weightWrapper.getRegisterInfo());
    _modbusHelper.registerBlock(_voltageWrapper.getRegisterInfo());
    _modbusHelper.registerBlock(_zeroVoltageWrapper.getRegisterInfo());
    _modbusHelper.registerBlock(MB_Registers(tcpBaseAddr + (ushort)E_LoadcellTcpOffset::ENABLED, 1, E_FN_CODE::FN_WRITE_COIL, MB_ACCESS_READ_WRITE, this->id, this->slaveId, "Enabled", name.c_str()));
    _modbusHelper.registerBlock(MB_Registers(tcpBaseAddr + (ushort)E_LoadcellTcpOffset::MODE, 1, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, this->id, this->slaveId, "Mode", name.c_str()));
    _modbusHelper.registerBlock(MB_Registers(tcpBaseAddr + (ushort)E_LoadcellTcpOffset::COMMAND, 1, E_FN_CODE::FN_WRITE_HOLD_REGISTER, MB_ACCESS_READ_WRITE, this->id, this->slaveId, "Command", name.c_str()));
}

uint16_t Loadcell::mb_tcp_base_address() const
{
    return MB_HREG_LOADCELL_MB_START + (this->slaveId - LOADCELL_SLAVE_ID_0) * LOADCELL_TCP_BLOCK_COUNT;
}

uint16_t Loadcell::mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const
{
    switch (rtuAddress)
    {
    case LOADCELL_RTU_REAL_TIME_WEIGHT_LOW:
        return (uint16_t)E_LoadcellTcpOffset::PV;
    case LOADCELL_RTU_REAL_TIME_WEIGHT_HIGH:
        return (uint16_t)E_LoadcellTcpOffset::PV;
    case LOADCELL_RTU_REAL_TIME_VOLTAGE_LOW:
        return (uint16_t)E_LoadcellTcpOffset::VOLTAGE;
    case LOADCELL_RTU_REAL_TIME_VOLTAGE_HIGH:
        return (uint16_t)E_LoadcellTcpOffset::VOLTAGE;
    case LOADCELL_RTU_ZERO_VOLTAGE_LOW:
        return (uint16_t)E_LoadcellTcpOffset::ZERO_VOLTAGE;
    case LOADCELL_RTU_ZERO_VOLTAGE_HIGH:
        return (uint16_t)E_LoadcellTcpOffset::ZERO_VOLTAGE;
    }
    return 0;
}

short Loadcell::setup()
{
    ModbusReadBlock *block = addMandatoryReadBlock(
        LOADCELL_READ_BLOCK_START_ADDR,
        LOADCELL_READ_BLOCK_REG_COUNT,
        E_FN_CODE::FN_READ_HOLD_REGISTER,
        _readInterval);

    if (!block)
    {
        L_ERROR(F("Loadcell[%d]: Failed to add mandatory read block!"), slaveId);
        return E_INVALID_PARAMETERS;
    }
    return E_OK;
}

short Loadcell::loop()
{
    if (!enabled())
    {
        return E_OK;
    }

    if (millis() - _lastUpdateTime > LOADCELL_TIMEOUT_MS)
    {
        if (!_timeoutReported)
        {
            onError((ushort)MB_Error::Timeout, "Timeout (Stalled)");
            _timeoutReported = true;
        }
    }

    return E_OK;
}

short Loadcell::info()
{
    uint32_t rt_val, volt_val, zero_volt_val;
    bool rt_ok = getWeight(rt_val);
    bool volt_ok = getVoltage(volt_val);
    bool zero_volt_ok = getZeroVoltage(zero_volt_val);
    L_INFO(F("--- Loadcell[%d] Info ---"), slaveId);
    L_INFO(F("  State: %s, Mode: %d"), getStateString(), _mode);
    L_INFO(F("  Last Error: %d"), getLastErrorCode());
    L_INFO(F("  Real-time Net Weight: %s (%lu) raw: H=%d L=%d"), rt_ok ? "OK" : "Error/Missing", rt_ok ? rt_val : 0, _weightHigh, _weightLow);
    L_INFO(F("  Real-time Voltage: %s (%lu) raw: H=%u L=%u"), volt_ok ? "OK" : "Error/Missing", volt_ok ? volt_val : 0, _voltageHigh, _voltageLow);
    L_INFO(F("  Zero Voltage: %s (%lu) raw: H=%u L=%u"), zero_volt_ok ? "OK" : "Error/Missing", zero_volt_ok ? zero_volt_val : 0, _zeroVoltageHigh, _zeroVoltageLow);
    L_INFO(F("--- End Loadcell[%d] Info --- "), slaveId);
    return 0;
}

bool Loadcell::onRegisterUpdate(uint16_t address, uint16_t newValue)
{
    bool updated = false;
#ifdef DEBUG_LOADCELL_MB
    L_INFO("Loadcell[%d]: onRegisterUpdate: %d, %d", slaveId, address, newValue);
#endif

    switch (address)
    {
    case LOADCELL_RTU_REAL_TIME_WEIGHT_LOW:
        _weightLow = newValue;
        _lastUpdateTime = millis();
        updated = true;
        break;
    case LOADCELL_RTU_REAL_TIME_WEIGHT_HIGH:
        _weightHigh = newValue;
        _lastUpdateTime = millis();
        {
            uint32_t current_weight = (static_cast<uint32_t>(_weightHigh) << 16) | _weightLow;
            int32_t weight_signed = static_cast<int32_t>(current_weight);

            // Interpret high values as disconnected
            if (weight_signed > (int32_t)LOADCELL_WEIGHT_MAX_VALUE)
            {
                if (!_timeoutReported)
                {
                    onError((ushort)MB_Error::Timeout, "Disconnected (High Value)");
                    _timeoutReported = true;
                }
                // Do not update _lastUpdateTime or _weightWrapper
            }
            else
            {
                _timeoutReported = false;
                if (_weightWrapper.update(current_weight, E_PRIORITY::E_PRIORITY_HIGHEST))
                    updated = true;
            }
        }
        break;
    case LOADCELL_RTU_REAL_TIME_VOLTAGE_LOW:
        _voltageLow = newValue;
        updated = true;
        break;
    case LOADCELL_RTU_REAL_TIME_VOLTAGE_HIGH:
        _voltageHigh = newValue;
        if (_voltageWrapper.update(
                (static_cast<uint32_t>(_voltageHigh) << 16) | _voltageLow))
            updated = true;
        break;
    case LOADCELL_RTU_ZERO_VOLTAGE_LOW:
        _zeroVoltageLow = newValue;
        updated = true;
        break;
    case LOADCELL_RTU_ZERO_VOLTAGE_HIGH:
        _zeroVoltageHigh = newValue;
        if (_zeroVoltageWrapper.update(
                (static_cast<uint32_t>(_zeroVoltageHigh) << 16) | _zeroVoltageLow))
            updated = true;
        break;
    }
    if (RTU_Base::onRegisterUpdate(address, newValue))
        updated = true;

    return updated;
}

bool Loadcell::getWeight(uint32_t &value) const
{
    if (millis() - _lastUpdateTime > LOADCELL_TIMEOUT_MS)
        return false;

    value = (static_cast<uint32_t>(_weightHigh) << 16) | _weightLow;

    // Interpret as signed 32-bit
    int32_t value_signed = static_cast<int32_t>(value);

    // Disconnect check: > MAX_VALUE (6000)
    // We allow negative values (dead zone)
    if (value_signed > (int32_t)LOADCELL_WEIGHT_MAX_VALUE)
        return false;

    return true;
}

bool Loadcell::getVoltage(uint32_t &value) const
{
    value = (static_cast<uint32_t>(_voltageHigh) << 16) | _voltageLow;
    return true;
}

bool Loadcell::getZeroVoltage(uint32_t &value) const
{
    value = (static_cast<uint32_t>(_zeroVoltageHigh) << 16) | _zeroVoltageLow;
    return true;
}

ModbusBlockView *Loadcell::mb_tcp_blocks() const
{
    return _modbusHelper.mb_tcp_blocks();
}

short Loadcell::mb_tcp_read(MB_Registers *reg)
{
    if (!reg)
        return (short)MB_Error::ServerDeviceFailure;

    const uint16_t instanceBaseAddr = this->mb_tcp_base_address();
    short offset = reg->startAddress - instanceBaseAddr;

    if (offset < 1)
        return (short)MB_Error::IllegalDataAddress;

    E_LoadcellTcpOffset regOffset = static_cast<E_LoadcellTcpOffset>(offset);
    uint16_t value = 0;

    switch (regOffset)
    {
    case E_LoadcellTcpOffset::PV:
        value = _weightWrapper.getValue();
        break;
    case E_LoadcellTcpOffset::VOLTAGE:
        value = _voltageWrapper.getValue();
        break;
    case E_LoadcellTcpOffset::ZERO_VOLTAGE:
        value = _zeroVoltageWrapper.getValue();
        break;
    case E_LoadcellTcpOffset::ENABLED:
        value = enabled() ? 1 : 0;
        break;
    case E_LoadcellTcpOffset::MODE:
        value = _mode;
        break;
    case E_LoadcellTcpOffset::COMMAND:
        value = 0; // write-only
        break;
    default:
        return (short)MB_Error::IllegalDataAddress;
    }

    return value;
}

short Loadcell::mb_tcp_write(MB_Registers *reg, short value)
{
    if (!reg)
        return (short)MB_Error::ServerDeviceFailure;

    const uint16_t tcpBaseAddr = this->mb_tcp_base_address();
    short offset = reg->startAddress - tcpBaseAddr;
    E_LoadcellTcpOffset regOffset = static_cast<E_LoadcellTcpOffset>(offset);

    switch (regOffset)
    {
    case E_LoadcellTcpOffset::PV:
    case E_LoadcellTcpOffset::VOLTAGE:
    case E_LoadcellTcpOffset::ZERO_VOLTAGE:
        return (short)MB_Error::IllegalDataAddress; // Read-only
    case E_LoadcellTcpOffset::ENABLED:
        value ? enable() : disable();
        break;
    default:
        if (enabled())
        {
            switch (regOffset)
            {
            case E_LoadcellTcpOffset::MODE:
                _mode = value;
                break;
            case E_LoadcellTcpOffset::COMMAND:
            {
                E_LoadcellCommand cmd = (E_LoadcellCommand)value;
                switch (cmd)
                {
                case E_LoadcellCommand::INFO:
                    this->info();
                    break;
                case E_LoadcellCommand::RESET:
                    L_INFO("Loadcell[%d]: Command RESET", this->slaveId);
                    // TODO: reset component state
                    break;
                case E_LoadcellCommand::TARE:
                    L_INFO("Loadcell[%d]: Command TARE", this->slaveId);
                    // TODO: implement tare functionality
                    break;
                default:
                    return (short)MB_Error::IllegalDataValue;
                }
                break;
            }
            default:
                return (short)MB_Error::IllegalDataAddress;
            }
        }
    }
    return (short)MB_Error::Success;
}

void Loadcell::onError(ushort errorCode, const char *errorMessage)
{
    RTU_Base::onError(errorCode, errorMessage);
    switch (errorCode)
    {
    case (ushort)MB_Error::Timeout:
    {
        L_ERROR("Loadcell error : %d - Timeout : Not connected. SlaveId: %d", errorCode, slaveId);
        return;
    }
    }
}

bool Loadcell::isHighPriority(const ModbusOperation &op) const
{
    if (op.type == E_FN_CODE::FN_READ_HOLD_REGISTER &&
        op.address == LOADCELL_READ_BLOCK_START_ADDR &&
        op.quantity == LOADCELL_READ_BLOCK_REG_COUNT)
    {
        return true;
    }
    return RTU_Base::isHighPriority(op);
}
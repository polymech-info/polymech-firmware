#ifndef LOADCELL_H
#define LOADCELL_H

// RS485 - Loadcell amplifier : https://qlsensor.com/products/load-cell-amplifier-weighing-sensor-transmitter-output-0-5v-0-10v-0-20ma

#include "config.h"

#include <Component.h>
#include <modbus/ModbusRTU.h>
#include <modbus/ModbusTypes.h>
#include <modbus/ModbusComponent.h>
#include <NetworkValue.h>
#include "xstatistics.h"

using LoadcellValue = NetworkValue<uint32_t>;

enum class E_LoadcellTcpOffset : ushort
{
    PV = 1,
    VOLTAGE = 2,
    ZERO_VOLTAGE = 3,
    ENABLED = 4,
    MODE = 5,
    COMMAND = 6,

};

enum class E_LoadcellCommand : uint16_t
{
    RESERVED = 0,
    INFO = 1,
    RESET = 2,
    TARE = 3,
};

constexpr int LOADCELL_TCP_BLOCK_COUNT = 6;
constexpr int LOADCELL_READ_INTERVAL = 50;
constexpr uint32_t LOADCELL_WEIGHT_UNDERFLOW_THRESHOLD = 64000;
constexpr uint32_t LOADCELL_WEIGHT_MAX_VALUE = 6000;

class Loadcell : public RTU_Base
{
public:
    Loadcell(Component *owner, uint8_t slaveId, millis_t readInterval = 50, short component_id = COMPONENT_KEY_LOADCELL_0);
    virtual ~Loadcell() = default;

    virtual short setup() override;
    virtual short loop() override;
    virtual short info() override;

    virtual bool onRegisterUpdate(uint16_t address, uint16_t newValue) override;

    bool getWeight(uint32_t &value) const;
    bool getVoltage(uint32_t &value) const;
    bool getZeroVoltage(uint32_t &value) const;

    virtual ModbusBlockView *mb_tcp_blocks() const override;
    virtual short mb_tcp_read(MB_Registers *reg) override;
    virtual short mb_tcp_write(MB_Registers *reg, short value) override;

    uint16_t mb_tcp_base_address() const override;
    uint16_t mb_tcp_offset_for_rtu_address(uint16_t rtuAddress) const override;

    virtual void onError(ushort errorCode, const char *errorMessage);

    virtual bool isHighPriority(const ModbusOperation &op) const override;

private:
    millis_t _readInterval;
    uint16_t _mode = 0;

    ModbusComponent<LOADCELL_TCP_BLOCK_COUNT> _modbusHelper;

    LoadcellValue _weightWrapper;
    LoadcellValue _voltageWrapper;
    LoadcellValue _zeroVoltageWrapper;
    // These are needed to reconstruct 32-bit values from 16-bit RTU registers
    uint16_t _weightLow, _weightHigh;
    uint16_t _voltageLow, _voltageHigh;
    uint16_t _zeroVoltageLow, _zeroVoltageHigh;
    millis_t _lastUpdateTime;
    bool _timeoutReported;
};

#define LOADCELL_TIMEOUT_MS 2500

#endif // LOADCELL_H
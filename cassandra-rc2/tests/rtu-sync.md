INFO: RTU_Base::setOutputRegisterValue(slaveId: 13, address: 0x2601, value: 30)
INFO: RTU_Base::triggerRTUWrite(slaveId: 13)
INFO: RTU_Base::write(slaveId: 13) - Iterating 2 output registers.
INFO: [OmronE5[13]:633] RTU_Base::write(slaveId: 13) - Processing register index 0: address 0x2601, value 30
INFO: RegisterState::writeToDevice(slaveId: 13, address: 0x2601, value: 30, force: false)
INFO: [ModbusRTU:100] ModbusRTU::writeRegister(slaveId: 13, address: 0x2601, value: 30, force: false)
INFO: [ModbusRTU:100] ModbusRTU::writeRegister - Value has changed or force=true. Queuing write.
INFO: [OmronE5[13]:633] RTU_ index 1: address 0x0000, value 0
INFO: RegisterState::writeToDevice(slaveId: 13, address: 0x0000, value: 0, force: true)
INFO: [ModbusRTU:100] ModbusRTU::writeRegister(slaveId: 13, address: 0x0000, value: 0, force: true)
INFO: [ModbusRTU:100] ModbusRTU::wforce=true. Queuing write.

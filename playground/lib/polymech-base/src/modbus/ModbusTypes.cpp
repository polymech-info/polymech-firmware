#include <ArduinoLog.h>
#include <modbus/ModbusTypes.h>
#include <modbus/ModbusRTU.h>
#include <components/RS485.h>

bool RTU_Base::triggerRTUWrite()
{
  RS485 *rs485 = (RS485 *)owner;
  if (rs485)
  {
    write(rs485->modbus);
    return true;
  }
  Log.errorln("Device %d: Failed to cast owner to RS485*! Cannot trigger RTU write.", slaveId);
  return false;
}

MB_Error RegisterState::readFromDevice(ModbusRTU &manager, uint8_t slaveId)
{
  switch (type)
  {
  case E_FN_CODE::FN_READ_INPUT_REGISTER:
  case E_FN_CODE::FN_READ_HOLD_REGISTER:
    return manager.readRegister(slaveId, address);
  case E_FN_CODE::FN_READ_COIL:
  case E_FN_CODE::FN_READ_DISCR_INPUT:
    return manager.readCoil(slaveId, address);
  }
  return MB_Error::UndefinedError;
}

MB_Error RegisterState::writeToDevice(ModbusRTU &manager, uint8_t slaveId, bool forceWrite)
{
  switch (type)
  {
  case E_FN_CODE::FN_WRITE_HOLD_REGISTER:
    return manager.writeRegister(slaveId, address, value, forceWrite);
  case E_FN_CODE::FN_WRITE_COIL:
    return manager.writeCoil(slaveId, address, getBoolValue());
  case E_FN_CODE::FN_READ_INPUT_REGISTER:
  case E_FN_CODE::FN_READ_DISCR_INPUT:
  case E_FN_CODE::FN_READ_HOLD_REGISTER:
  case E_FN_CODE::FN_READ_COIL:
    return MB_Error::IllegalDataAddress;
  default:
    Log.warningln("Attempted writeToDevice with unhandled/read-only type: %X", (int)type);
    return MB_Error::IllegalFunction;
  }
  return MB_Error::UndefinedError;
}

void RegisterState::printState(ModbusRTU &manager, uint8_t slaveId)
{
  switch (type)
  {
  case E_FN_CODE::FN_READ_INPUT_REGISTER:
    Log.noticeln("Input Register %d: %d (Priority: %d, %s)",
                 address, value, priority,
                 manager.isRegisterSynchronized(slaveId, address) ? "Synchronized" : "Not synchronized");
    break;

  case E_FN_CODE::FN_READ_HOLD_REGISTER:
    Log.noticeln("Holding Register %d: %d (Priority: %d, %s)",
                 address, value, priority,
                 manager.isRegisterSynchronized(slaveId, address) ? "Synchronized" : "Not synchronized");
    break;

  case E_FN_CODE::FN_READ_COIL:
    Log.noticeln("Coil %d: %s (Priority: %d, %s)",
                 address, getBoolValue() ? "ON" : "OFF", priority,
                 manager.isCoilSynchronized(slaveId, address) ? "Synchronized" : "Not synchronized");
    break;

  case E_FN_CODE::FN_READ_DISCR_INPUT:
    Log.noticeln("Discrete Input %d: %s (Priority: %d, %s)",
                 address, getBoolValue() ? "ON" : "OFF", priority,
                 manager.isCoilSynchronized(slaveId, address) ? "Synchronized" : "Not synchronized");
    break;

  default:
    Log.noticeln("Register Address %d: Value %d (Type: %X, Priority: %d, %s)",
                 address, value, (int)type, priority,
                 manager.isRegisterSynchronized(slaveId, address) ? "Synchronized" : "Not synchronized");
    break;
  }
}

DuplicateOperationFilter::DuplicateOperationFilter(ModbusRTU *rtu)
    : modbusRTU(rtu), operationTimeout(OPERATION_TIMEOUT)
{
  if (modbusRTU == nullptr)
  {
    Log.errorln("DuplicateOperationFilter created with null ModbusRTU pointer!");
  }
}

DuplicateOperationFilter::~DuplicateOperationFilter()
{
  // Nothing to clean up
}

bool DuplicateOperationFilter::filter(const ModbusOperation &op)
{
  // Check if this operation is already pending in the ModbusRTU queues
  // Use the stored modbusRTU pointer
  if (modbusRTU && modbusRTU->isOperationAlreadyPending(op))
  {
    // Log.traceln("Filter: Dropping duplicate operation (slave: %d, type: %d, address: %d)", op.slaveId, op.type, op.address);
    return false; // Filter out the operation
  }

  return true; // Let the operation through
}

RateLimitFilter::RateLimitFilter(unsigned long minIntervalMs)
    : minInterval(minIntervalMs), lastOperationTime(0) {}

bool RateLimitFilter::filter(const ModbusOperation &op)
{
  unsigned long now = millis();

  // Check if enough time has passed since the last operation
  if (now - lastOperationTime < minInterval)
  {
    Log.traceln("Filter: Rate limiting operation (slave: %d, type: %d, address: %d)",
                op.slaveId, op.type, op.address);
    return false; // Filter out the operation
  }

  // Update the last operation time
  lastOperationTime = now;
  return true; // Let the operation through
}

PriorityFilter::PriorityFilter() {}

bool PriorityFilter::filter(const ModbusOperation &op)
{
  return true;
}

bool PriorityFilter::adjustPriority(ModbusOperation &op)
{
  if (op.type == E_FN_CODE::FN_WRITE_COIL ||
      op.type == E_FN_CODE::FN_WRITE_HOLD_REGISTER ||
      op.type == E_FN_CODE::FN_WRITE_MULT_COILS ||
      op.type == E_FN_CODE::FN_WRITE_MULT_REGISTERS)
  {
    // Write operations are considered high priority
    // The actual priority value might be set elsewhere or this could return a priority level.
    // For now, returning true indicates it meets the high-priority condition.
    return true;
  }
  return false;
}

void RTU_Base::updateState(ModbusRTU &manager)
{
  if (state != UNINITIALIZED && lastResponseTime > 0 && millis() - lastResponseTime > responseTimeout)
  {
    if (state != ERROR)
    {
      errorCount++;
      setState(ERROR);
    }
    return; // If timeout occurred, don't process further state changes here
  }
  bool hasPendingOps = manager.hasPendingOperations(slaveId);
  switch (state)
  {
  case UNINITIALIZED:
    break;
  case INITIALIZING:
    if (!hasPendingOps)
    {
      setState(IDLE);
    }
    break;
  case IDLE:
    if (hasPendingOps)
    {
      setState(RUNNING);
    }
    break;
  case RUNNING:
    if (!hasPendingOps)
    {
      setState(IDLE);
    }
    break;
  case ERROR:
    break;
  }
}

bool RTU_Base::initialize(ModbusRTU &manager)
{
  if (state == UNINITIALIZED)
  {
    setState(INITIALIZING);
    write(manager);
    return true; // Return true indicating initialization was started
  }
  else
  {
    return false; // Return false as it wasn't in UNINITIALIZED state
  }
}

void RTU_Base::read(ModbusRTU &manager)
{
  if (state == RUNNING)
  {
    return;
  }
  unsigned long currentTime = millis();
  bool readQueued = false;
  int numBlocks = mandatoryReadBlocks.size();
  for (int i = 0; i < numBlocks; ++i)
  {
    ModbusReadBlock &block = mandatoryReadBlocks[i];

    if (!block.isUsed())
    {
      continue;
    }

    if (currentTime - block.lastReadTime >= block.readInterval)
    {
      MB_Error err = MB_Error::Success;
      switch (block.type)
      {
      case E_FN_CODE::FN_READ_HOLD_REGISTER:
        err = manager.readHoldingRegisters(slaveId, block.startAddress, block.count);
        break;
      case E_FN_CODE::FN_READ_INPUT_REGISTER:
        err = manager.readInputRegisters(slaveId, block.startAddress, block.count);
        break;
      case E_FN_CODE::FN_READ_COIL:
        err = manager.readCoils(slaveId, block.startAddress, block.count);
        break;
      case E_FN_CODE::FN_READ_DISCR_INPUT:
        err = manager.readDiscreteInputs(slaveId, block.startAddress, block.count);
        break;
      default:
        Log.warningln("Device %d: Mandatory read block has invalid type (%X). Skipping.", slaveId, (int)block.type);
        err = MB_Error::IllegalFunction; // Mark as error to prevent state change if this was the only block
        break;
      }

      if (err == MB_Error::Success)
      {
        readQueued = true;
        block.lastReadTime = currentTime;
      }
      else
      {
        if (err != MB_Error::OpNotReady)
        {
          Log.warningln("Device %d: Read queue is full. Skipping read for block - Start: %d, Count: %d.",
                        slaveId, block.startAddress, block.count);
        }
      }
    }
  }
  if (readQueued && state == IDLE)
  {
    setState(RUNNING);
  }
}

void RTU_Base::write(ModbusRTU &manager)
{
  if (state == RUNNING)
  {
    // Log.infoln("Device %d: Write operation already in progress. Skipping.", slaveId);
    // return;
  }
  bool writeQueued = false;
  for (int i = 0; i < registerCount; i++)
  {
    if (registers[i] != nullptr)
    {
      if (registers[i]->type == E_FN_CODE::FN_WRITE_HOLD_REGISTER ||
          registers[i]->type == E_FN_CODE::FN_WRITE_COIL)
      {
        MB_Error err = registers[i]->writeToDevice(manager, slaveId, registers[i]->priority == PRIORITY_HIGHEST);
        if (err == MB_Error::Success)
        {
          // Log.traceln("Device %d: Write operation queued for register %d (addr %d) | Value: %d.", slaveId, i, registers[i]->address, registers[i]->value);
          writeQueued = true;
        }
        else
        {
          Log.warningln("Device %d: Failed to queue write for register %d (addr %d). Error: %u",
                        slaveId, i, registers[i]->address, (unsigned int)err);
        }
      }
    }
    else
    {
      Log.errorln("Device %d: Register %d (addr %d) is not writable.", slaveId, i, registers[i]->address);
    }
  }
  if (writeQueued && state == IDLE)
  {
    setState(RUNNING);
  }
}

void RTU_Base::printState()
{
  for (int i = 0; i < registerCount; i++)
  {
    if (registers[i] != nullptr)
    {
      Log.traceln("Register %d: Type %d, Address %d, Value %d", i, registers[i]->type, registers[i]->address, registers[i]->value);
    }
  }
}

void RTU_Base::reset()
{
  Log.noticeln("Device %d: Resetting state...", slaveId);
  setState(UNINITIALIZED);
  lastResponseTime = 0;
  errorCount = 0;
  lastSyncTime = 0;
}

const char *RTU_Base::getStateString() const
{
  switch (state)
  {
  case UNINITIALIZED:
    return "Uninitialized";
  case INITIALIZING:
    return "Initializing";
  case IDLE:
    return "Idle";
  case RUNNING:
    return "Running";
  case ERROR:
    return "Error";
  default:
    return "Unknown";
  }
}

void RTU_Base::setState(E_DeviceState newState)
{
  if (state != newState)
  {
    E_DeviceState oldState = state;
    const char *oldStateStr = getStateString();
    state = newState;
    if ((oldState == ERROR) && (newState == IDLE || newState == RUNNING))
    {
      if (errorCount > 0)
      {
        errorCount = 0;
      }
    }
  }
}

void RTU_Base::handleResponseReceived()
{
  lastResponseTime = millis();
  if (state == ERROR)
  {
    setState(IDLE);
  }
  else if (state == IDLE || state == RUNNING || state == INITIALIZING)
  {
    // Log.traceln("Device %d received response in state %s.", slaveId, getStateString());
    // Log.traceln("Device %d received response in state %s. = %d", slaveId, getStateString());
    // printState();
  }
}

const RegisterState *RTU_Base::getRegisterByAddress(uint16_t address) const
{
  for (int i = 0; i < registerCount; i++)
  {
    if (registers[i] != nullptr && registers[i]->address == address)
    {
      return registers[i];
    }
  }
  return nullptr;
}

void RTU_Base::setOutputRegisterValue(uint16_t address, uint16_t value)
{
  bool found = false;
  for (int i = 0; i < registerCount; i++)
  {
    if (registers[i] != nullptr && registers[i]->address == address)
    {
      if (registers[i]->type == E_FN_CODE::FN_WRITE_HOLD_REGISTER || registers[i]->type == E_FN_CODE::FN_WRITE_COIL)
      {
        registers[i]->value = value;
        found = true;
        break;
      }
      else
      {
        Log.warningln("Device %d: Register with address %d found, but cannot set value - not a Holding/Coil type (%d).",
                      slaveId, address, registers[i]->type);
        found = true;
        break;
      }
    }
  }
  if (!found)
  {
    Log.warningln("Device %d: Writable register with address %d not found for setting value.", slaveId, address);
  }
  else
  {
    triggerRTUWrite();
  }
}

ModbusReadBlock *RTU_Base::addMandatoryReadBlock(uint16_t startAddress, uint16_t count, E_FN_CODE type, unsigned long interval)
{
  if (count == 0)
  {
    Log.warningln("Device %d: Cannot add mandatory read block with count 0.", slaveId);
    return nullptr;
  }
  if (type != E_FN_CODE::FN_READ_HOLD_REGISTER &&
      type != E_FN_CODE::FN_READ_INPUT_REGISTER &&
      type != E_FN_CODE::FN_READ_COIL &&
      type != E_FN_CODE::FN_READ_DISCR_INPUT)
  {
    Log.warningln("Device %d: Invalid type (%X) for mandatory read block.", slaveId, (int)type);
    return nullptr;
  }
  if (mandatoryReadBlocks.full())
  {
    Log.errorln("Device %d: Cannot add mandatory read block - maximum (%d) reached.", slaveId, MAX_READ_BLOCKS);
    return nullptr;
  }
  ModbusReadBlock newBlock(startAddress, count, type, interval);
  mandatoryReadBlocks.push_back(newBlock);
  Log.traceln("Device %d: Added mandatory read block - Start: %d, Count: %d, Type: %X, Interval: %lu ms",
              slaveId, startAddress, count, (int)type, interval);

  newBlock.setUsed(true);
  return &mandatoryReadBlocks.back();
}
void RTU_Base::onError(ushort errorCode, const char *errorMessage)
{
  lastErrorCode = errorCode;
}
void RTU_Base::onRegisterUpdate(uint16_t address, uint16_t newValue)
{
  bool updated = false;
  for (int i = 0; i < registerCount; ++i)
  {
    if (registers[i] != nullptr && registers[i]->address == address)
    {
      if (registers[i]->value != newValue)
      {
        registers[i]->value = newValue;
      }
      updated = true;
      break;
    }
  }
}
const char *modbusErrorToString(MB_Error error)
{
  switch (error)
  {
  // Standard Modbus Exceptions
  case MB_Error::Success:
    return "Success";
  case MB_Error::IllegalFunction:
    return "Illegal Function";
  case MB_Error::IllegalDataAddress:
    return "Illegal Data Address";
  case MB_Error::IllegalDataValue:
    return "Illegal Data Value";
  case MB_Error::ServerDeviceFailure:
    return "Server Device Failure";
  case MB_Error::Acknowledge:
    return "Acknowledge";
  case MB_Error::ServerDeviceBusy:
    return "Server Device Busy";
  case MB_Error::NegativeAcknowledge:
    return "Negative Acknowledge";
  case MB_Error::MemoryParityError:
    return "Memory Parity Error";
  case MB_Error::GatewayPathUnavailable:
    return "Gateway Path Unavailable";
  case MB_Error::GatewayTargetNoResp:
    return "Gateway Target Device Failed to Respond";

  // Internal Operation/Queue Errors
  case MB_Error::OpNotReady:
    return "Operation Not Ready";
  case MB_Error::OpQueueFull:
    return "ModbusRTU Operation Queue Full";
  case MB_Error::OpClientQueueFull:
    return "eModbus Client Queue Full";
  case MB_Error::OpExecutionFailed:
    return "Operation Execution Failed";
  case MB_Error::OpInvalidParameter:
    return "Invalid Parameter";
  case MB_Error::OpRetrying:
    return "Operation Retrying";
  case MB_Error::OpMaxRetriesExceeded:
    return "Max Retries Exceeded";

  // eModbus Specific Communication Errors
  case MB_Error::Timeout:
    return "Timeout";
  case MB_Error::InvalidServer:
    return "Invalid Server Response";
  case MB_Error::CrcError:
    return "CRC Error"; // RTU
  case MB_Error::FcMismatch:
    return "Function Code Mismatch";
  case MB_Error::ServerIdMismatch:
    return "Server ID Mismatch";
  case MB_Error::PacketLengthError:
    return "Packet Length Error";
  case MB_Error::ParameterCountError:
    return "Parameter Count Error";
  case MB_Error::ParameterLimitError:
    return "Parameter Limit Error";
  case MB_Error::RequestQueueFull:
    return "eModbus Request Queue Full"; // Same as OpClientQueueFull? Check enum vals
  case MB_Error::IllegalIpOrPort:
    return "Illegal IP or Port"; // TCP
  case MB_Error::IpConnectionFailed:
    return "IP Connection Failed"; // TCP
  case MB_Error::TcpHeadMismatch:
    return "TCP Header Mismatch"; // TCP
  case MB_Error::EmptyMessage:
    return "Empty Message Received";
  case MB_Error::AsciiFrameError:
    return "ASCII Frame Error"; // ASCII
  case MB_Error::AsciiCrcError:
    return "ASCII LRC Error"; // ASCII
  case MB_Error::AsciiInvalidChar:
    return "ASCII Invalid Character"; // ASCII
  case MB_Error::BroadcastError:
    return "Broadcast Error";

  case MB_Error::UndefinedError:
  default:
    return "Undefined or Unknown Error";
  }
}
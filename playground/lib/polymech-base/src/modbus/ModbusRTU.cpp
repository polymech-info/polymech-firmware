#include "ModbusRTU.h"
#include <ArduinoLog.h>
#include "ModbusTypes.h"
#include "config-modbus.h"

class RTU_Device;

ModbusRTU *ModbusRTU::instance = nullptr;
static uint32_t nextTokenId = 1;
Manager *Manager::instance = nullptr;

ModbusRTU::ModbusRTU(int8_t redePin, uint16_t queueSize,
                     unsigned long baseMinIntervalMs,
                     OnRegisterChangeCallback onRegisterChange,
                     OnWriteCallback onWrite,
                     OnErrorCallback onError)
    : client(nullptr),
      ready(false),
      rePin(redePin),
      maxQueueSize(queueSize),
      serial(nullptr),
      baudRate(MB_RTU_BAUDRATE),
      firstFilter(nullptr),
      errorCount(0),
      successCount(0),
      lastOperationTime(0),
      minOperationInterval(baseMinIntervalMs),
      operationCount(0),
      responseCallback(nullptr),
      onRegisterChangeCallback(onRegisterChange),
      onWriteCallback(onWrite),
      onErrorCallback(onError),
      initState(INIT_NOT_STARTED),
      initStartTime(0),
      lastProcessTime(0)
#ifdef ENABLE_ADAPTIVE_TIMEOUT
      // Initialize adaptive interval state only if enabled
      ,
      baseMinInterval(baseMinIntervalMs),
      maxMinInterval(1000), intervalIncreaseStep(50),
      intervalDecreaseThreshold(10), intervalDecreaseStep(10),
      consecutiveSuccessCount(0)
#endif
{
    for (int i = 0; i < MAX_MODBUS_SLAVES; i++)
    {
        slaveData[i] = SlaveData();
    }
    addFilter(new DuplicateOperationFilter(this));
    addFilter(new OperationLifecycleFilter());
    // addFilter(new RateLimitFilter(10));
    instance = this;

    // Initialize device error state
    for (int i = 0; i < MAX_MODBUS_SLAVES; i++) {
        deviceErrorState.lastBackoffIncreaseTime[i] = 0;
        deviceErrorState.currentBackoffInterval[i] = MB_OFFLINE_BACKOFF_INITIAL;
    }
}

ModbusRTU::~ModbusRTU()
{
    end();
    clearFilters();
    instance = nullptr;
}

MB_Error ModbusRTU::begin(HardwareSerial &serialPort, uint32_t baudrate)
{
    if (client != nullptr)
    {
        end();
    }
    // Store references
    serial = &serialPort;
    baudRate = baudrate;
    client = new ModbusClientRTU(rePin, maxQueueSize);
    if (!client)
    {
        Log.error("Failed to create ModbusClientRTU instance" CR);
        initState = INIT_FAILED;
        return MB_Error::OpExecutionFailed;
    }

    // Set up callbacks
    client->onDataHandler(&ModbusRTU::staticDataHandler);
    client->onErrorHandler(&ModbusRTU::staticErrorHandler);
    client->setTimeout(MODBUS_SERIAL_TIMEOUT);

    // Initialize the serial port if it's not already
    if (!serialPort)
    {
        RTUutils::prepareHardwareSerial(serialPort);
        serialPort.begin(baudrate, MODBUS_SERIAL_MODE);
        // Non-blocking initialization
        initState = INIT_SERIAL_STARTED;
        initStartTime = millis();
    }
    else
    {
        initState = INIT_SERIAL_STARTED;
        initStartTime = millis() - DELAY_SERIAL_INIT;
    }
    Log.infoln(F("ModbusRTU: Initialized with baud rate %d"), baudrate);
    return MB_Error::Success;
}

void ModbusRTU::end()
{
    if (client)
    {
        client->end();
        delete client;
        client = nullptr;
    }
    ready = false;
}

MB_Error ModbusRTU::process()
{
    unsigned long currentTime = millis();

    if (currentTime - lastProcessTime < MB_RTU_PROCESS_INTERVAL_MS)
    {
        return MB_Error::Success; // Not time yet, return success (nothing to do)
    }
    lastProcessTime = currentTime; // Update last execution time

    // First handle initialization if not complete
    if (initState != INIT_READY && initState != INIT_FAILED)
    {
        processInitialization();
        return initState == INIT_FAILED ? MB_Error::OpNotReady : MB_Error::Success;
    }

    if (!ready || !client)
    {
        return MB_Error::OpNotReady;
    }

    // Handle any pending operations
    MB_Error result = handlePendingOperations();
    if (result != MB_Error::Success && result != MB_Error::OpNotReady)
    {
        // Log non-fatal errors at verbose level
        Log.verboseln("Operation processing error: %d", static_cast<int>(result));
        // Continue processing even if there was an error with one operation
    }

    // Re-validate pending operations through filters
    // This ensures expired operations are removed
    // We should ONLY apply lifecycle filters here, not duplication checks
    for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
    {
        if (TEST(operationQueue[i].flags, OP_USED_BIT))
        {
            // Find the OperationLifecycleFilter in the chain
            ModbusOperationFilter *filter = firstFilter;
            bool dropOperation = false;
            while (filter)
            {
                // Only apply lifecycle filter here
                if (filter->getType() == FILTER_LIFECYCLE)
                {
                    if (!filter->filter(operationQueue[i]))
                    { // Call filter() method directly
                        dropOperation = true;
                        // Log the drop reason using the lifecycle filter's logic
                        // Note: The filter itself logs details in its filter() method if TRACE is enabled
                        break; // No need to check other filters if dropped
                    }
                }
                filter = filter->getNext();
            }

            if (dropOperation)
            {
                // Operation should be dropped due to lifecycle filter (timeout/retries)
                CBI(operationQueue[i].flags, OP_USED_BIT); // Clear the USED flag
                if (operationCount > 0)
                    operationCount--;
                // Mark as failed if it timed out or exceeded retries
                if (operationQueue[i].status != MB_SUCCESS)
                { // Avoid marking successful ops as failed
                    operationQueue[i].status = MB_FAILED;
                    errorCount++; // Count timeout/retry failure as an error
                }
            }
        }
    }

    // Return the error from the most recent operation, or success if everything was fine
    return result;
}

// Handle the non-blocking initialization states
void ModbusRTU::processInitialization()
{
    unsigned long now = millis();
    unsigned long elapsed = now - initStartTime;

    switch (initState)
    {
    case INIT_SERIAL_STARTED:
        if (elapsed >= DELAY_SERIAL_INIT)
        {
            // Serial init time has passed, start client
            if (serial && client)
            {
                // Start the Modbus client
                client->begin(*serial);
                initState = INIT_CLIENT_STARTED;
                initStartTime = now;
            }
            else
            {
                Log.error("Serial or client is null during initialization" CR);
                initState = INIT_FAILED;
            }
        }
        break;

    case INIT_CLIENT_STARTED:
        if (elapsed >= DELAY_CLIENT_INIT)
        {
            // Client init time has passed, mark as ready
            initState = INIT_READY;
            ready = true;
            Log.notice("ModbusRTU initialized successfully (non-blocking)" CR);
        }
        break;

    case INIT_READY:
    case INIT_FAILED:
    case INIT_NOT_STARTED:
        // Nothing to do in these states
        break;
    }
}

// Reset the client
MB_Error ModbusRTU::reset(bool fullReset)
{
    Log.notice("Resetting ModbusRTU..." CR);
    if (fullReset)
    {
        // End the client
        if (client)
        {
            client->end();
            delete client;
            client = nullptr;
        }

        // Reset state tracking using CBI
        for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
        {
            CBI(operationQueue[i].flags, OP_USED_BIT);
        }
        operationCount = 0;

        // Create a new client
        client = new ModbusClientRTU(rePin, maxQueueSize);
        if (!client)
        {
            Log.error("Failed to create ModbusClientRTU instance during reset" CR);
            ready = false;
            initState = INIT_FAILED;
            return MB_Error::OpExecutionFailed;
        }

        // Set up callbacks
        client->onDataHandler(&ModbusRTU::staticDataHandler);
        client->onErrorHandler(&ModbusRTU::staticErrorHandler);
        client->setTimeout(MODBUS_SERIAL_TIMEOUT);

        // Reset the serial port
        if (serial)
        {
            serial->end();

            // Start non-blocking init
            RTUutils::prepareHardwareSerial(*serial);
            serial->begin(baudRate, MODBUS_SERIAL_MODE);
            initState = INIT_SERIAL_STARTED;
            initStartTime = millis();
            ready = false;
        }
        else
        {
            Log.error("Serial port reference is null during reset" CR);
            ready = false;
            initState = INIT_FAILED;
            return MB_Error::OpNotReady;
        }
    }
    else
    {
        // Just cleanup the queue
        if (client)
        {
            client->clearQueue();
        }

        // Reset state tracking using CBI
        for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
        {
            CBI(operationQueue[i].flags, OP_USED_BIT);
        }
        operationCount = 0;

        // Keep existing client and serial, just mark as ready
        ready = (client != nullptr && serial != nullptr);

        if (ready)
        {
            initState = INIT_READY;
        }
        else
        {
            initState = INIT_FAILED;
            return MB_Error::OpNotReady;
        }
    }

    Log.notice("ModbusRTU reset initiated" CR);
    return MB_Error::Success;
}

// Coil operations
MB_Error ModbusRTU::readCoil(uint8_t slaveId, uint16_t address)
{
    ModbusOperation op(E_FN_CODE::FN_READ_COIL, slaveId, address);
    return queueOperation(op);
}

MB_Error ModbusRTU::writeCoil(uint8_t slaveId, uint16_t address, bool value)
{
    bool currentValue;
    if (getCoilValue(slaveId, address, currentValue) && currentValue == value)
    {
        return MB_Error::Success; // Value is already correct, no need to write
    }

    ModbusOperation op(E_FN_CODE::FN_WRITE_COIL, slaveId, address, value ? COIL_ON : COIL_OFF);
    updateCoilValue(slaveId, address, value, false);
    return queueOperation(op, true);
}

bool ModbusRTU::getCoilValue(uint8_t slaveId, uint16_t address, bool &value) const
{
    const SlaveData *slave = _getSlaveData(slaveId);
    if (!slave)
    {
        return false;
    }

    for (int i = 0; i < slave->coilCount; i++)
    {
        if (TEST(slave->coils[i].flags, VALUE_USED_BIT) && slave->coils[i].address == address)
        {
            value = (slave->coils[i].value != 0);
            return true;
        }
    }
    return false;
}

bool ModbusRTU::isCoilSynchronized(uint8_t slaveId, uint16_t address) const
{
    const SlaveData *slave = _getSlaveData(slaveId);
    if (!slave)
    {
        return false;
    }

    for (int i = 0; i < slave->coilCount; i++)
    {
        if (TEST(slave->coils[i].flags, VALUE_USED_BIT) && slave->coils[i].address == address)
        {
            return TEST(slave->coils[i].flags, VALUE_SYNCHRONIZED_BIT);
        }
    }
    return false;
}

MB_Error ModbusRTU::readRegister(uint8_t slaveId, uint16_t address)
{
    ModbusOperation op(E_FN_CODE::FN_READ_HOLD_REGISTER, slaveId, address);
    return queueOperation(op);
}

MB_Error ModbusRTU::writeRegister(uint8_t slaveId, uint16_t address, uint16_t value, bool force)
{
    uint16_t currentValue;
    if (!force && getRegisterValue(slaveId, address, currentValue) && currentValue == value)
    {
        // Log.infoln("Device %d: Register %d is already set to %d. Skipping write.", slaveId, address, value);
        return MB_Error::Success;
    }    
    ModbusOperation op(E_FN_CODE::FN_WRITE_HOLD_REGISTER, slaveId, address, value);
    updateRegisterValue(slaveId, address, value, false);
    return queueOperation(op, true);
}

MB_Error ModbusRTU::writeMultipleRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t quantity, uint16_t value)
{
    ModbusOperation op(E_FN_CODE::FN_R_W_MULT_REGISTERS, slaveId, startAddress, value, quantity);
    for (uint16_t i = 0; i < quantity; i++)
    {
        updateRegisterValue(slaveId, startAddress + i, value, false);
    }
    return queueOperation(op, true);
}

bool ModbusRTU::getRegisterValue(uint8_t slaveId, uint16_t address, uint16_t &value) const
{
    const SlaveData *slave = _getSlaveData(slaveId);
    if (!slave)
    {
        return false;
    }

    for (int i = 0; i < slave->registerCount; i++)
    {
        if (TEST(slave->registers[i].flags, VALUE_USED_BIT) && slave->registers[i].address == address)
        {
            value = slave->registers[i].value;
            return true;
        }
    }
    return false;
}

bool ModbusRTU::isRegisterSynchronized(uint8_t slaveId, uint16_t address) const
{
    const SlaveData *slave = _getSlaveData(slaveId);
    if (!slave)
    {
        return false;
    }

    for (int i = 0; i < slave->registerCount; i++)
    {
        if (TEST(slave->registers[i].flags, VALUE_USED_BIT) && slave->registers[i].address == address)
        {
            return TEST(slave->registers[i].flags, VALUE_SYNCHRONIZED_BIT);
        }
    }
    return false;
}

float ModbusRTU::getSuccessRate() const
{
    if (successCount + errorCount == 0)
    {
        return 0.0f;
    }
    return (float)successCount / (float)(successCount + errorCount) * 100.0f;
}

void ModbusRTU::clearStats()
{
    errorCount = 0;
    successCount = 0;
}

void ModbusRTU::printStatus() const
{
    Log.notice("=== ModbusRTU Status ===" CR);
    Log.notice("Ready: %s" CR, ready ? "Yes" : "No");
    Log.notice("Success rate: %.1f%%" CR, getSuccessRate());
    Log.notice("Successful operations: %u" CR, successCount);
    Log.notice("Failed operations: %u" CR, errorCount);

    // Count high priority operations using TEST
    uint8_t highPriorityCount = 0;
    for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
    {
        if (TEST(operationQueue[i].flags, OP_USED_BIT) && TEST(operationQueue[i].flags, OP_HIGH_PRIORITY_BIT))
        {
            highPriorityCount++;
        }
    }

    Log.notice("Total pending operations: %u" CR, operationCount);
    Log.notice("High priority operations: %u" CR, highPriorityCount);

    uint32_t coilCount = 0;
    uint32_t regCount = 0;

    for (int s = 0; s < MAX_MODBUS_SLAVES; s++)
    {
        const SlaveData *slave = _getSlaveData(s + 1); // Use getter (slaveId is 1-based)
        if (!slave)
            continue;
        for (int i = 0; i < MAX_ADDRESSES_PER_SLAVE; i++)
        {
            if (TEST(slave->coils[i].flags, VALUE_USED_BIT))
                coilCount++;
            if (TEST(slave->registers[i].flags, VALUE_USED_BIT))
                regCount++;
        }
    }

    Log.notice("Tracked coils: %u" CR, coilCount);
    Log.notice("Tracked registers: %u" CR, regCount);
    Log.notice("===========================" CR);
}

MB_Error ModbusRTU::handlePendingOperations()
{
    if (!ready || !client)
    {
        return MB_Error::OpNotReady;
    }

    // Check if it's time to send another operation
    if (!hasTimeToNextOperation())
    {
        return MB_Error::Success; // No operations were processed, which is normal
    }

    // First look for high priority operations using TEST
    for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
    {
        if (TEST(operationQueue[i].flags, OP_USED_BIT) &&          
            TEST(operationQueue[i].flags, OP_HIGH_PRIORITY_BIT) && 
            !TEST(operationQueue[i].flags, OP_IN_PROGRESS_BIT))    
        {
            //Log.traceln("Selecting high priority operation: token=%u, slaveId=%u, address=%u, type=%d",
            //            operationQueue[i].token, operationQueue[i].slaveId, operationQueue[i].address, operationQueue[i].type);
            return executeOperation(operationQueue[i]);
        }
    }

    // Then look for regular priority operations using TEST
    for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
    {
        if (TEST(operationQueue[i].flags, OP_USED_BIT) &&           
            !TEST(operationQueue[i].flags, OP_HIGH_PRIORITY_BIT) && 
            !TEST(operationQueue[i].flags, OP_IN_PROGRESS_BIT))     
        {
            // Log.traceln("Selecting normal priority operation: token=%u, slaveId=%u, address=%u, type=%d",
            //            operationQueue[i].token, operationQueue[i].slaveId, operationQueue[i].address, operationQueue[i].type);
            return executeOperation(operationQueue[i]);
        }
    }

    return MB_Error::Success; // No operations were found to process
}

MB_Error ModbusRTU::executeOperation(ModbusOperation &op)
{
    if (!ready || !client)
    {
        return MB_Error::OpNotReady;
    }

    // Mark as in progress using SBI and OP_IN_PROGRESS_BIT
    SBI(op.flags, OP_IN_PROGRESS_BIT);

    // Generate a unique token for this operation using the counter
    unsigned long oldToken = op.token; // Keep track of old one for logging if needed
    op.token = nextTokenId++;
    if (nextTokenId == 0)
        nextTokenId = 1;
    op.timestamp = millis();
    lastOperationTime = millis();

    // Notify filters that operation is being executed
    ModbusOperationFilter *filter = firstFilter;
    while (filter)
    {
        filter->notifyOperationExecuted(op);
        filter = filter->getNext();
    }

    // Execute the operation based on type
    Error err = Modbus::SUCCESS;

    // Extract values based on type
    // Determine if it's a coil or register based on the original operation type
    bool isCoil = (op.type == E_FN_CODE::FN_READ_COIL);
    // Check for discrete input as well, as they are read using the same mechanism internally
    bool isDiscreteInput = (op.type == E_FN_CODE::FN_READ_DISCR_INPUT);

    switch (op.type)
    {
    case E_FN_CODE::FN_READ_COIL:
    case E_FN_CODE::FN_READ_DISCR_INPUT: // Grouped Discrete Input here
        err = client->addRequest(op.token, op.slaveId, op.type, op.address, op.quantity);
        break;

    case E_FN_CODE::FN_READ_HOLD_REGISTER:
    case E_FN_CODE::FN_READ_INPUT_REGISTER:
        err = client->addRequest(op.token, op.slaveId, op.type, op.address, op.quantity);
        break;

    case E_FN_CODE::FN_WRITE_COIL:
        err = client->addRequest(op.token, op.slaveId, WRITE_COIL, op.address, op.value);
        break;

    case E_FN_CODE::FN_WRITE_HOLD_REGISTER:
        err = client->addRequest(op.token, op.slaveId, WRITE_HOLD_REGISTER, op.address, op.value);
        break;

    case E_FN_CODE::FN_R_W_MULT_REGISTERS:
        static uint8_t msgData[128]; // Large enough for typical operations (max ~60 registers)

        if (op.quantity > 0 && op.quantity <= 60)
        { // Can hold more registers now
            uint8_t msgLength = 7 + (op.quantity * 2);

            // Ensure we don't exceed buffer size
            if (msgLength <= sizeof(msgData))
            {
                // Fill in the message directly to the static buffer
                msgData[0] = op.slaveId;
                msgData[1] = 0x10; // Function code 16
                msgData[2] = highByte(op.address);
                msgData[3] = lowByte(op.address);
                msgData[4] = highByte(op.quantity);
                msgData[5] = lowByte(op.quantity);
                msgData[6] = op.quantity * 2; // Byte count

                // Set all registers to the same value, but working directly with bytes
                uint8_t highVal = highByte(op.value);
                uint8_t lowVal = lowByte(op.value);

                for (int i = 0; i < op.quantity; i++)
                {
                    msgData[7 + (i * 2)] = highVal;
                    msgData[8 + (i * 2)] = lowVal;
                }

                err = client->addBroadcastMessage(msgData, msgLength);
            }
            else
            {
                Log.error("Message too large for buffer" CR);
                err = Modbus::MEMORY_PARITY_ERROR;
            }
        }
        else
        {
            Log.error("Invalid quantity for multiple registers write" CR);
            err = Modbus::ILLEGAL_DATA_VALUE;
        }
        break;
    }
    if (err != Modbus::SUCCESS)
    {
        // Log.error("Error adding request: %d for token %u", (int)err, op.token);
        // Clear IN_PROGRESS flag using CBI and OP_IN_PROGRESS_BIT
        CBI(op.flags, OP_IN_PROGRESS_BIT);
        // Only retry on specific communication errors, not queue full
        if (err == Modbus::TIMEOUT || err == Modbus::CRC_ERROR)
        {
            op.retries++;
            op.status = MB_RETRYING;

            if (op.retries < MAX_RETRIES)
            {
                // Operation remains in our queue (ModbusRTU::operationQueue)
                // because addRequest failed. It will be picked up again by handlePendingOperations.
                // No need to re-queue explicitly. Just update status and retry count.
                Log.noticeln("Communication error for token %u, will retry later (retry %d)", op.token, op.retries);
                return MB_Error::OpRetrying; // Indicate a retry will happen eventually
            }
            else
            {
                // Max retries exceeded for communication errors
                Log.error("Max retries exceeded for communication error, giving up on operation (token %u)", op.token);
                op.status = MB_FAILED;
                CBI(op.flags, OP_USED_BIT); // Clear USED flag using CBI
                if (operationCount > 0)
                    operationCount--;
                errorCount++; // Count this as a final error
                return MB_Error::OpMaxRetriesExceeded;
            }
        }
        else if (err == static_cast<Error>(MB_Error::RequestQueueFull)) { 
            uint8_t slaveIdx = op.slaveId - 1; // Convert to 0-based index
            uint32_t now = millis();
            
            if (now - deviceErrorState.lastBackoffIncreaseTime[slaveIdx] > deviceErrorState.currentBackoffInterval[slaveIdx]) {
                deviceErrorState.lastBackoffIncreaseTime[slaveIdx] = now;
                
                if (deviceErrorState.currentBackoffInterval[slaveIdx] < MB_OFFLINE_BACKOFF_MAX) {
                    deviceErrorState.currentBackoffInterval[slaveIdx] *= MB_OFFLINE_BACKOFF_MULTIPLIER;
                    if (deviceErrorState.currentBackoffInterval[slaveIdx] > MB_OFFLINE_BACKOFF_MAX) {
                        deviceErrorState.currentBackoffInterval[slaveIdx] = MB_OFFLINE_BACKOFF_MAX;
                    }
                }
                
                Log.traceln("Device %u queue full. Setting backoff to %lu ms", 
                            op.slaveId, deviceErrorState.currentBackoffInterval[slaveIdx]);
            }
            
            // Add random jitter to the backoff time
            long jitter = (random(-10, 11) / 100.0) * deviceErrorState.currentBackoffInterval[slaveIdx]; // +/- 10%
            lastOperationTime = now + deviceErrorState.currentBackoffInterval[slaveIdx] + jitter;
            return MB_Error::OpClientQueueFull;
        }
        else
        {
            // Other unrecoverable errors (e.g., ILLEGAL_DATA_ADDRESS)
            Log.error("Unrecoverable error %d adding request, giving up on operation (token %u)", (int)err, op.token);
            op.status = MB_FAILED;
            CBI(op.flags, OP_USED_BIT); // Clear USED flag using CBI
            if (operationCount > 0)
                operationCount--;
            errorCount++;                       // Count this as a final error
            return MB_Error::OpExecutionFailed; // Generic execution failure
        }
    }
    else
    {
        // Success: client->addRequest() accepted the operation.
        // Operation remains in the queue (OP_USED_BIT is set)
        // AND OP_IN_PROGRESS_BIT is set.
        return MB_Error::Success; // Indicate successful submission to eModbus
    }
    // Need to return appropriate error code from the if/else if branches
    if (err == Modbus::TIMEOUT || err == Modbus::CRC_ERROR)
    {
        Log.traceln("Communication error for token %u, will retry later (retry %d) (error %d)", op.token, op.retries, (int)err);
        return (op.retries < MAX_RETRIES) ? MB_Error::OpRetrying : MB_Error::OpMaxRetriesExceeded;
    }
    else if (err == static_cast<Error>(MB_Error::OpClientQueueFull))
    {
        return MB_Error::OpClientQueueFull;
    }
    else
    {
        return MB_Error::OpExecutionFailed; // Should have been caught above
    }
}

MB_Error ModbusRTU::queueOperation(ModbusOperation op, bool highPriority)
{
    if (!ready)
    {
        return MB_Error::OpNotReady;
    }
    if (highPriority)
        SBI(op.flags, OP_HIGH_PRIORITY_BIT); 
    else
        CBI(op.flags, OP_HIGH_PRIORITY_BIT); 

    if (firstFilter && !firstFilter->process(op))
    {
        return MB_Error::Success;
    }
    bool added = false;
    for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
    {
        if (!TEST(operationQueue[i].flags, OP_USED_BIT))
        { 
            operationQueue[i] = op;
            SBI(operationQueue[i].flags, OP_USED_BIT); 
            operationCount++;
            added = true;
            break;
        }
    }

    if (!added)
    {
        Log.errorln("ModbusRTU::queueOperation - Queue full");
        return MB_Error::OpQueueFull;
    }

    return MB_Error::Success;
}

bool ModbusRTU::isOperationAlreadyPending(const ModbusOperation &op) const
{
    const uint8_t MAX_RESULTS = 16;
    ModbusOperation *potentialMatches[MAX_RESULTS];
    uint8_t numFound = getOperations(potentialMatches, MAX_RESULTS,
                                     op.slaveId,
                                     (1 << OP_USED_BIT),
                                     nullptr,
                                     const_cast<E_FN_CODE *>(&op.type));

    for (uint8_t i = 0; i < numFound; ++i)
    {
        const ModbusOperation *existingOp = potentialMatches[i];

        // Check address
        if (existingOp->address != op.address)
        {
            continue;
        }

        switch (op.type)
        {
        case E_FN_CODE::FN_WRITE_COIL:
        case E_FN_CODE::FN_WRITE_HOLD_REGISTER:
            if (existingOp->value == op.value)
            {
                return true; // Found duplicate
            }
            break;
        case E_FN_CODE::FN_R_W_MULT_REGISTERS:
            if (existingOp->quantity == op.quantity && existingOp->value == op.value)
            {
                return true; // Found duplicate
            }
            break;
        case E_FN_CODE::FN_READ_COIL:
        case E_FN_CODE::FN_READ_HOLD_REGISTER:
            return true; 
        default:
            return false;
        }
    }
    return false;
}

// Helper methods for finding and creating entries
ModbusValueEntry *ModbusRTU::findCoilEntry(uint8_t slaveId, uint16_t address)
{
    SlaveData *slave = _getSlaveData(slaveId);
    if (!slave)
        return nullptr;

    for (int i = 0; i < MAX_ADDRESSES_PER_SLAVE; i++)
    {
        if (TEST(slave->coils[i].flags, VALUE_USED_BIT) && slave->coils[i].address == address)
        {
            return &slave->coils[i];
        }
    }
    return nullptr;
}

ModbusValueEntry *ModbusRTU::findRegisterEntry(uint8_t slaveId, uint16_t address)
{
    SlaveData *slave = _getSlaveData(slaveId);
    if (!slave)
        return nullptr;

    for (int i = 0; i < MAX_ADDRESSES_PER_SLAVE; i++)
    {
        if (TEST(slave->registers[i].flags, VALUE_USED_BIT) && slave->registers[i].address == address)
        {
            return &slave->registers[i];
        }
    }
    return nullptr;
}

ModbusValueEntry *ModbusRTU::createCoilEntry(uint8_t slaveId, uint16_t address)
{
    SlaveData *slave = _getSlaveData(slaveId);
    if (!slave)
        return nullptr;

    for (int i = 0; i < MAX_ADDRESSES_PER_SLAVE; i++)
    {
        if (!TEST(slave->coils[i].flags, VALUE_USED_BIT))
        {
            slave->coils[i].address = address;
            SBI(slave->coils[i].flags, VALUE_USED_BIT);
            slave->coils[i].value = 0;
            CBI(slave->coils[i].flags, VALUE_SYNCHRONIZED_BIT);
            slave->coils[i].lastUpdate = millis();
            slave->coilCount++;
            return &slave->coils[i];
        }
    }

    Log.error("Failed to create coil entry - array full" CR);
    return nullptr;
}

ModbusValueEntry *ModbusRTU::createRegisterEntry(uint8_t slaveId, uint16_t address)
{
    SlaveData *slave = _getSlaveData(slaveId);
    if (!slave)
        return nullptr;

    for (int i = 0; i < MAX_ADDRESSES_PER_SLAVE; i++)
    {
        if (!TEST(slave->registers[i].flags, VALUE_USED_BIT))
        {
            slave->registers[i].address = address;
            SBI(slave->registers[i].flags, VALUE_USED_BIT);
            slave->registers[i].value = 0;
            CBI(slave->registers[i].flags, VALUE_SYNCHRONIZED_BIT);
            slave->registers[i].lastUpdate = millis();
            slave->registerCount++;
            return &slave->registers[i];
        }
    }

    Log.error("Failed to create register entry - array full" CR);
    return nullptr;
}

void ModbusRTU::updateCoilValue(uint8_t slaveId, uint16_t address, bool value, bool synchronized)
{
    updateValue<bool, ModbusValueEntry>(slaveId, address, value, synchronized,
                                        &ModbusRTU::findCoilEntry, &ModbusRTU::createCoilEntry);
}

void ModbusRTU::updateRegisterValue(uint8_t slaveId, uint16_t address, uint16_t value, bool synchronized)
{
    updateValue<uint16_t, ModbusValueEntry>(slaveId, address, value, synchronized, &ModbusRTU::findRegisterEntry, &ModbusRTU::createRegisterEntry);
}

void ModbusRTU::onDataReceived(ModbusMessage message, uint32_t token)
{
    ModbusOperation *operation = findOperationByToken(token);
    if (operation == nullptr)
    {
        Log.warningln("Unknown token %u received. Active ops: %d, Total queued ops: %d",
                      token, 0, operationCount);

        // Debug - print active operations details if in TRACE log level
        if (Log.getLevel() <= LOG_LEVEL_TRACE)
        {
            Log.traceln("Examining operation queue for token %u:", token);
            int displayCount = 0;
            for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
            {
                if (TEST(operationQueue[i].flags, OP_USED_BIT))
                {
                    Log.traceln("  [%d]: token=%u, address=%u, function=%d, slaveId=%u",
                                i, operationQueue[i].token,
                                operationQueue[i].address,
                                operationQueue[i].type,
                                operationQueue[i].slaveId);
                    displayCount++;
                    if (displayCount >= 5)
                        break; // Limit to 5 entries to avoid log flooding
                }
            }
        }

        // Get current time to check for timeouts
        unsigned long now = millis();

        // Check if this could be an old token from a timed-out operation
        if (now - token > 10000)
        { // If token is more than 10 seconds old
            Log.traceln("Token %u appears to be very old (over 10 seconds), likely a timed out operation", token);
        }

        return;
    }
    successCount++;
    operation->status = MB_SUCCESS;

#ifdef ENABLE_ADAPTIVE_TIMEOUT
    // ---- ADAPTIVE INTERVAL DECREASE LOGIC ----
    consecutiveSuccessCount++;
    if (consecutiveSuccessCount >= intervalDecreaseThreshold && minOperationInterval > baseMinInterval)
    {
        unsigned long oldInterval = minOperationInterval;
        if (minOperationInterval <= baseMinInterval + intervalDecreaseStep)
        {
            minOperationInterval = baseMinInterval; // Prevent going below base
        }
        else
        {
            minOperationInterval -= intervalDecreaseStep;
        }
        Log.verboseln("Adaptive interval decreased: %lu -> %lu ms (%u consecutive successes)",
                      oldInterval, minOperationInterval, consecutiveSuccessCount);
        consecutiveSuccessCount = 0; // Reset count after decrease
    }
    // -----------------------------------------
#endif

    // Process based on operation type
    switch (operation->type)
    {
    case E_FN_CODE::FN_READ_COIL:
    case E_FN_CODE::FN_READ_HOLD_REGISTER:
    case E_FN_CODE::FN_READ_INPUT_REGISTER:
    case E_FN_CODE::FN_READ_DISCR_INPUT:
        // Delegate processing of read data to the helper function
        _processReadResponse(*operation, message);
        break;

    case E_FN_CODE::FN_WRITE_COIL:
        // Update synchronization status for the written coil
        updateCoilValue(operation->slaveId, operation->address, operation->value == COIL_ON, true);
        onWriteCallback(*operation);
        break;

    case E_FN_CODE::FN_WRITE_HOLD_REGISTER:
        // Update synchronization status for the written register
        updateRegisterValue(operation->slaveId, operation->address, operation->value, true);
        onWriteCallback(*operation);
        break;

    case E_FN_CODE::FN_R_W_MULT_REGISTERS:
        // Update synchronization status for all written registers
        for (uint16_t i = 0; i < operation->quantity; ++i)
        {
            updateRegisterValue(operation->slaveId, operation->address + i, operation->value, true);
        }
        // Call write callback (once for the whole operation)
        onWriteCallback(*operation);
        break;

    default:
        // Handle unknown or unexpected operation types if necessary
        Log.warningln("onDataReceived: Unhandled operation type %X for token %u", (int)operation->type, token);
        break;
    }

    // Call the general response callback if registered
    if (responseCallback)
    {
        responseCallback(operation->slaveId);
    }

    // Notify filters that the operation is complete
    ModbusOperationFilter *filter = firstFilter;
    while (filter)
    {
        filter->notifyOperationCompleted(*operation);
        filter = filter->getNext();
    }

    // Mark operation as complete by clearing the USED and IN_PROGRESS flags using CBI and OP_*_BIT
    CBI(operation->flags, OP_USED_BIT);        
    CBI(operation->flags, OP_IN_PROGRESS_BIT); 
    if (operationCount > 0)
        operationCount--;

    // Log.traceln("Operation completed successfully for token %u", token);
}

void ModbusRTU::staticDataHandler(ModbusMessage message, uint32_t token)
{
    if (instance)
    {
        instance->onDataReceived(message, token);
    }
}

void ModbusRTU::staticErrorHandler(Error error, uint32_t token)
{
    if (instance)
    {
        instance->onErrorReceived(error, token);
    }
}

bool ModbusRTU::hasPendingOperations(uint8_t slaveId) const
{
    for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
    {
        if (TEST(operationQueue[i].flags, OP_USED_BIT) && 
            (slaveId == 0 || operationQueue[i].slaveId == slaveId))
        { 
            return true;
        }
    }

    return false;
}

uint8_t ModbusRTU::getOperations(ModbusOperation *results[], uint8_t maxResults, uint8_t slaveId, uint8_t required_flags_mask, E_MB_OpStatus *status, E_FN_CODE *type) const
{
    uint8_t count = 0;
    for (int i = 0; i < MAX_PENDING_OPERATIONS && count < maxResults; ++i)
    {
        const ModbusOperation &op = operationQueue[i];
        if (!TEST(op.flags, OP_USED_BIT))
        {
            continue;
        }

        if (slaveId != 0 && op.slaveId != slaveId)
        {
            continue;
        }

        if (required_flags_mask != 0 && (op.flags & required_flags_mask) != required_flags_mask)
        {
            continue;
        }

        if (status != nullptr && op.status != *status)
        {
            continue;
        }

        if (type != nullptr && op.type != *type)
        {
            continue;
        }

        results[count++] = const_cast<ModbusOperation *>(&op);
    }

    return count;
}

void ModbusRTU::setResponseCallback(ResponseCallback callback)
{
    responseCallback = callback;
}

void ModbusRTU::addFilter(ModbusOperationFilter *filter)
{
    if (!filter)
        return;

    if (!firstFilter)
    {
        firstFilter = filter;
    }
    else
    {
        // Find the last filter in the chain
        ModbusOperationFilter *current = firstFilter;
        while (current->getNext())
        {
            current = current->getNext();
        }
        current->setNext(filter);
    }
}

void ModbusRTU::clearFilters()
{
    firstFilter = nullptr;
}

void ModbusRTU::setOnRegisterChangeCallback(OnRegisterChangeCallback callback)
{
    onRegisterChangeCallback = callback ? callback : emptyRegisterChangeCallback;
}

void ModbusRTU::setOnWriteCallback(OnWriteCallback callback)
{
    onWriteCallback = callback ? callback : emptyWriteCallback;
}

void ModbusRTU::setOnErrorCallback(OnErrorCallback callback)
{
    onErrorCallback = callback ? callback : emptyErrorCallback;
}

ModbusOperation *ModbusRTU::findOperationByToken(uint32_t token)
{
    for (int i = 0; i < MAX_PENDING_OPERATIONS; i++)
    {
        if (TEST(operationQueue[i].flags, OP_USED_BIT) && operationQueue[i].token == token)
        {
            return &operationQueue[i];
        }
    }
    return nullptr;
}

bool ModbusRTU::hasTimeToNextOperation() const
{
    return millis() - lastOperationTime >= minOperationInterval;
}

void ModbusRTU::onErrorReceived(Error error, uint32_t token)
{
    ModbusOperation *operation = findOperationByToken(token);
    if (operation == nullptr)
    {
        MB_Error mbError = static_cast<MB_Error>(error);
        // Log.errorln("Callback Error: Operation not found for token %u (error %u: %s)", token, (int)error, modbusErrorToString(mbError));
        return;
    }

    errorCount++;
    operation->status = MB_FAILED;

#ifdef ENABLE_ADAPTIVE_TIMEOUT
    MB_Error mbErrorCheck = static_cast<MB_Error>(error);
    if (mbErrorCheck == MB_Error::Timeout || mbErrorCheck == MB_Error::CrcError || 
        mbErrorCheck == MB_Error::RequestQueueFull) {
        consecutiveSuccessCount = 0; 
        unsigned long increaseAmount = (mbErrorCheck == MB_Error::RequestQueueFull) ? 
            intervalIncreaseStep * 2 : intervalIncreaseStep;
            
        if (minOperationInterval < maxMinInterval) {
            unsigned long oldInterval = minOperationInterval;
            minOperationInterval += increaseAmount;
            if (minOperationInterval > maxMinInterval) {
                minOperationInterval = maxMinInterval; 
            }
            Log.warningln("Adaptive interval increased due to error %s: %lu -> %lu ms", 
                         modbusErrorToString(mbErrorCheck), oldInterval, minOperationInterval);
        }
    }
#endif

    // Handle device-specific backoff for timeouts
    if (error == Modbus::TIMEOUT) {
        uint8_t slaveIdx = operation->slaveId - 1;        
        uint32_t now = millis();
        if (now - deviceErrorState.lastBackoffIncreaseTime[slaveIdx] > deviceErrorState.currentBackoffInterval[slaveIdx]) {
            deviceErrorState.lastBackoffIncreaseTime[slaveIdx] = now;

            if (deviceErrorState.currentBackoffInterval[slaveIdx] < MB_OFFLINE_BACKOFF_MAX) {
                deviceErrorState.currentBackoffInterval[slaveIdx] *= MB_OFFLINE_BACKOFF_MULTIPLIER;
                if (deviceErrorState.currentBackoffInterval[slaveIdx] > MB_OFFLINE_BACKOFF_MAX) {
                    deviceErrorState.currentBackoffInterval[slaveIdx] = MB_OFFLINE_BACKOFF_MAX;
                }
            }
        }
        long jitter = (random(-10, 11) / 100.0) * deviceErrorState.currentBackoffInterval[slaveIdx]; // +/- 10%
        lastOperationTime = now + deviceErrorState.currentBackoffInterval[slaveIdx] + jitter;
    }

    MB_Error mbError = static_cast<MB_Error>(error);
    const char *errorString = modbusErrorToString(mbError);
    if (mbError != MB_Error::Timeout && MB_PRINT_ERRORS) {
        Log.errorln("Modbus Error: Token=%u, Error=%d (%s)", token, (int)error, errorString);
    }
    onErrorCallback(*operation, (int)error, errorString);
    CBI(operation->flags, OP_USED_BIT);
    CBI(operation->flags, OP_IN_PROGRESS_BIT);
    if (operationCount > 0)
        operationCount--;
}

#ifdef ENABLE_ADAPTIVE_TIMEOUT
// Configuration for Adaptive Minimum Operation Interval
void ModbusRTU::setAdaptiveInterval(unsigned long baseInterval, unsigned long maxInterval,
                                    unsigned long increaseStep, uint16_t decreaseThreshold, unsigned long decreaseStep)
{
    baseMinInterval = baseInterval;
    maxMinInterval = maxInterval;
    intervalIncreaseStep = increaseStep;
    intervalDecreaseThreshold = decreaseThreshold;
    intervalDecreaseStep = decreaseStep;

    // Reset current interval to base when configuring
    minOperationInterval = baseMinInterval;
    consecutiveSuccessCount = 0; // Reset success count

    Log.noticeln("Adaptive interval configured: Base=%lums, Max=%lums, Incr=%lums, DecrThr=%u, Decr=%lums",
                 baseMinInterval, maxMinInterval, intervalIncreaseStep, intervalDecreaseThreshold, intervalDecreaseStep);
}
#endif // ENABLE_ADAPTIVE_TIMEOUT

#ifdef DEBUG_INTERVAL_CONTROL
// --- DEBUG ONLY --- Force setting the current min interval
void ModbusRTU::_forceSetMinInterval(unsigned long newInterval)
{
    minOperationInterval = newInterval;
    // Don't reset consecutive success count here, let adaptive logic handle it
    Log.warningln("DEBUG: Forced minInterval to %lu ms", minOperationInterval);
}
#endif // DEBUG_INTERVAL_CONTROL

void ModbusRTU::printSlaveData(uint8_t slaveId) const
{
    Log.notice("=== Modbus Slave Data Cache ===" CR);
    uint8_t startSlave = (slaveId == 0) ? 1 : slaveId;
    uint8_t endSlave = (slaveId == 0) ? MAX_MODBUS_SLAVES : slaveId;
    if (slaveId != 0 && (slaveId < 1 || slaveId > MAX_MODBUS_SLAVES))
    {
        Log.warningln("Invalid slave ID requested: %d", slaveId);
        return;
    }

    bool dataFound = false;
    for (uint8_t s = startSlave; s <= endSlave; ++s)
    {
        const SlaveData *slave = _getSlaveData(s);
        if (!slave) continue;

        bool slaveHeaderPrinted = false;
        for (int i = 0; i < MAX_ADDRESSES_PER_SLAVE; ++i)
        {
            if (TEST(slave->coils[i].flags, VALUE_USED_BIT))
            {
                if (!slaveHeaderPrinted) {
                    Log.notice("-- Slave %d --" CR, s);
                    slaveHeaderPrinted = true;
                    dataFound = true;
                }
                Log.notice("  Coil   [%04X]: %s (Sync: %s, Updated: %lums ago)" CR,
                           slave->coils[i].address,
                           slave->coils[i].value ? "ON" : "OFF",
                           TEST(slave->coils[i].flags, VALUE_SYNCHRONIZED_BIT) ? "Yes" : "No",
                           (millis() - slave->coils[i].lastUpdate));
            }
        }

        // Print Registers
        for (int i = 0; i < MAX_ADDRESSES_PER_SLAVE; ++i)
        {
             if (TEST(slave->registers[i].flags, VALUE_USED_BIT))
             {
                if (!slaveHeaderPrinted) {
                    Log.notice("-- Slave %d --" CR, s);
                    slaveHeaderPrinted = true;
                    dataFound = true;
                }
                Log.notice("  Reg    [%04X]: %u (0x%04X) (Sync: %s, Updated: %lums ago)" CR,
                           slave->registers[i].address,
                           slave->registers[i].value, slave->registers[i].value, // Print dec and hex
                           TEST(slave->registers[i].flags, VALUE_SYNCHRONIZED_BIT) ? "Yes" : "No",
                           (millis() - slave->registers[i].lastUpdate));
             }
        }
    }
     if (!dataFound && slaveId != 0) {
         Log.notice("No cached data found for slave %d.", slaveId);
     } else if (!dataFound && slaveId == 0) {
         Log.notice("No cached data found for any slaves.");
     }

    Log.notice("=============================" CR);
}

void ModbusRTU::printQueue() const
{
    Log.notice("=== Modbus Operation Queue ===" CR);
    Log.notice("Total pending operations: %u / %d" CR, operationCount, MAX_PENDING_OPERATIONS);

    if (operationCount == 0)
    {
        Log.notice("Queue is empty." CR);
        Log.notice("===========================" CR);
        return;
    }

    int printedCount = 0;
    for (int i = 0; i < MAX_PENDING_OPERATIONS; ++i)
    {
        if (TEST(operationQueue[i].flags, OP_USED_BIT))
        {
            const ModbusOperation &op = operationQueue[i];
            unsigned long age = millis() - op.timestamp;

            Log.notice("  [%d] Token:%u Slave:%u Addr:%04X Type:0x%02X Value:%u Qty:%u Prio:%s Status:%d Retries:%d Age:%lums InProg:%s" CR,
                       i,
                       op.token,
                       op.slaveId,
                       op.address,
                       (int)op.type,
                       op.value,
                       op.quantity,
                       TEST(op.flags, OP_HIGH_PRIORITY_BIT) ? "High" : "Norm",
                       (int)op.status,
                       op.retries,
                       age,
                       TEST(op.flags, OP_IN_PROGRESS_BIT) ? "Yes" : "No"
                      );
            printedCount++;
        }
    }
     if (printedCount != operationCount) {
         Log.warningln("Printed count (%d) does not match internal operation count (%u)!", printedCount, operationCount);
     }

    Log.notice("===========================" CR);
}

void ModbusRTU::_updateSlaveData(uint8_t slaveId, uint16_t address, uint16_t value, bool isCoil)
{
    if (isCoil)
    {
        // The 'value' here is 0 or 1 for coil OFF/ON
        updateCoilValue(slaveId, address, (value != 0), true); // Mark as synchronized
    }
    else
    {
        updateRegisterValue(slaveId, address, value, true); // Mark as synchronized
    }
}

const SlaveData *ModbusRTU::_getSlaveData(uint8_t slaveId) const
{
    if (slaveId == 0 || slaveId > MAX_MODBUS_SLAVES)
    {
        Log.warningln("Invalid slave ID %d requested in _getSlaveData", slaveId);
        return nullptr;
    }
    return &(_slaveData[slaveId - 1]);
}

SlaveData *ModbusRTU::_getSlaveData(uint8_t slaveId)
{
    if (slaveId == 0 || slaveId > MAX_MODBUS_SLAVES)
    {
        Log.warningln("Invalid slave ID %d requested in _getSlaveData", slaveId);
        return nullptr;
    }
    return &(_slaveData[slaveId - 1]);
}

void ModbusRTU::clearQueue()
{
    operationCount = 0;
    // Clear the USED flag for all operations in the array using CBI
    for (int i = 0; i < MAX_PENDING_OPERATIONS; ++i)
    {
        CBI(operationQueue[i].flags, OP_USED_BIT);
    }
    // Clear slave data cache using the new clear() method
    for (int i = 0; i < MAX_MODBUS_SLAVES; ++i)
    {
        _slaveData[i].clear(); // Use the clear() method
    }
    Log.noticeln("Modbus operation queue and slave data cache cleared.");
}

void ModbusRTU::_processReadResponse(const ModbusOperation &op, ModbusMessage &response)
{
    uint8_t slaveId = op.slaveId;
    uint16_t startAddress = op.address;
    uint16_t quantity = op.quantity;
    bool isCoil = (op.type == E_FN_CODE::FN_READ_COIL);

    // Get the data pointer and count from the response
    const uint8_t *rawData = response.data(); // Pointer to the start of the full PDU
    uint16_t messageSize = response.size();   // Get total size for bounds checking

    // Ensure the message is long enough to contain SlaveID, FC, and ByteCount
    if (messageSize < 3)
    {
        Log.warningln("Read response PDU too short (%u bytes) for slave %d, addr %d.",
                      messageSize, slaveId, startAddress);
        return; // Cannot process
    }

    uint8_t byteCount = rawData[2];       // Byte count is the 3rd byte (index 2)
    const uint8_t *dataPtr = &rawData[3]; // Actual data starts at the 4th byte (index 3)

    // Basic validation: Check if reported byteCount matches expected size based on quantity
    uint16_t expectedByteCount = isCoil ? (quantity + 7) / 8 : quantity * 2;
    if (byteCount != expectedByteCount)
    {
        Log.warningln("Read response byte count mismatch for slave %d, addr %d. Expected %d bytes for %d %s, got %d reported in PDU.",
                      slaveId, startAddress,
                      expectedByteCount,
                      quantity, (isCoil ? "coils" : "registers"), byteCount);
        // Decide how to handle: Proceed cautiously? Abort? For now, let's proceed but log.
    }

    // Additional validation: Check if the actual data size matches the reported byteCount
    // The actual data size is messageSize - (SlaveID + FC + ByteCount + CRC=2) = messageSize - 5
    // However, the eModbus ModbusMessage might already exclude CRC. Let's assume response.data() stops before CRC.
    // Check if messageSize is at least SlaveID + FC + ByteCount + DataBytes
    if (messageSize < (uint16_t)(3 + byteCount))
    {
        Log.warningln("Read response message size (%u) is smaller than implied by header (3 + byteCount=%u) for slave %d, addr %d.",
                      messageSize, 3 + byteCount, slaveId, startAddress);
        return; // Cannot safely process
    }

    // Log.verboseln("Processing read response for slave %d, addr %d, quantity %d (%s). Reported Bytes: %d",
    //               slaveId, startAddress, quantity, (isCoil ? "Coil" : "Register"), byteCount);

    // Update slave data cache
    SlaveData *slave = _getSlaveData(slaveId); // Use non-const getter
    if (!slave)
        return;

    for (uint16_t i = 0; i < quantity; ++i)
    {
        uint16_t currentAddress = startAddress + i;
        uint16_t value = 0;

        if (isCoil)
        {
            // Extract coil value (bit) from the correct byte
            uint8_t byteIndex = i / 8;
            uint8_t bitIndex = i % 8;
            if (byteIndex < byteCount)
            {                                                            // Check bounds against the *reported* byteCount
                value = (dataPtr[byteIndex] >> bitIndex) & 0x01 ? 1 : 0; // 1 for ON, 0 for OFF
            }
            else
            {
                Log.warningln("Byte index %d out of bounds (byteCount %d) for coil %d (addr %d)",
                              byteIndex, byteCount, i, currentAddress);
                continue; // Skip this coil
            }
        }
        else
        {
            // Extract register value (2 bytes, Big Endian)
            uint8_t highByteIndex = i * 2;
            uint8_t lowByteIndex = highByteIndex + 1;
            if (lowByteIndex < byteCount)
            { // Check bounds against the *reported* byteCount
                value = (dataPtr[highByteIndex] << 8) | dataPtr[lowByteIndex];
            }
            else
            {
                Log.warningln("Byte index %d/%d out of bounds (byteCount %d) for register %d (addr %d)",
                              highByteIndex, lowByteIndex, byteCount, i, currentAddress);
                continue; // Skip this register
            }
        }
        // Update the specific entry using the helper function
        _updateSlaveData(slaveId, currentAddress, value, isCoil);
    }
}

MB_Error ModbusRTU::readHoldingRegisters(uint8_t slaveId, uint16_t address, uint16_t quantity)
{
    if (quantity == 0)
        return MB_Error::IllegalDataValue;
    ModbusOperation op(E_FN_CODE::FN_READ_HOLD_REGISTER, slaveId, address, 0, quantity);
    return queueOperation(op);
}

MB_Error ModbusRTU::readInputRegisters(uint8_t slaveId, uint16_t address, uint16_t quantity)
{
    if (quantity == 0)
        return MB_Error::IllegalDataValue;
    ModbusOperation op(E_FN_CODE::FN_READ_INPUT_REGISTER, slaveId, address, 0, quantity);
    return queueOperation(op);
}

MB_Error ModbusRTU::readCoils(uint8_t slaveId, uint16_t address, uint16_t quantity)
{
    if (quantity == 0)
        return MB_Error::IllegalDataValue;
    ModbusOperation op(E_FN_CODE::FN_READ_COIL, slaveId, address, 0, quantity);
    return queueOperation(op);
}

MB_Error ModbusRTU::readDiscreteInputs(uint8_t slaveId, uint16_t address, uint16_t quantity)
{
    if (quantity == 0)
        return MB_Error::IllegalDataValue;
    // Note: There might not be a specific E_FN_CODE for discrete inputs in your enum,
    // assuming FN_READ_DISCR_INPUT exists and is correct. Adjust if necessary.
    ModbusOperation op(E_FN_CODE::FN_READ_DISCR_INPUT, slaveId, address, 0, quantity);
    return queueOperation(op);
}

void Manager::staticOnRegisterChange(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue)
{
    if (instance != nullptr)
    {
        instance->handleRegisterChange(op, oldValue, newValue);
    }
    else
    {
        Log.warningln("ModbusDeviceManager::staticOnRegisterChange called, but no instance set!");
    }
}

void Manager::handleRegisterChange(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue)
{
    RTU_Base *device = getDeviceById(op.slaveId);
    if (device)
    {
        // Call the virtual method on the specific device instance
        // Log.traceln("Manager routing register update for slave %d, addr %d, value %d", op.slaveId, op.address, newValue);
        device->onRegisterUpdate(op.address, newValue);
    }
    else
    {
        Log.traceln("Manager received register change for unmanaged slaveId: %d", op.slaveId);
    }
}

const ModbusOperation* ModbusRTU::getOperationQueue() const {
    return operationQueue;
}

uint8_t ModbusRTU::getOperationCount() const {
    return operationCount;
}

const SlaveData* ModbusRTU::getSlaveData() const {
    return _slaveData;
}

uint8_t ModbusRTU::getMaxSlaves() const {
    return MAX_MODBUS_SLAVES;
}

// Add method to reset offline state for a device
void ModbusRTU::resetDeviceOfflineState(uint8_t slaveId) {
    if (slaveId > 0 && slaveId <= MAX_MODBUS_SLAVES) {
        uint8_t idx = slaveId - 1;
        deviceErrorState.lastBackoffIncreaseTime[idx] = 0;
        deviceErrorState.currentBackoffInterval[idx] = MB_OFFLINE_BACKOFF_INITIAL;
    }
}

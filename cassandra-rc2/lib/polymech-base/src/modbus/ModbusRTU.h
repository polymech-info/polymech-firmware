#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <Arduino.h>
#include <ModbusClientRTU.h>
#include <HardwareSerial.h>
#include <ArduinoLog.h>
#include "ModbusTypes.h"
#include "../Component.h"

// --- Feature Flags ---
// Uncomment to enable adaptive minimum operation interval
// #define ENABLE_ADAPTIVE_TIMEOUT
// Uncomment to allow external forcing of min interval for debugging
// #define DEBUG_INTERVAL_CONTROL

enum E_InitState
{
  INIT_NOT_STARTED,
  INIT_SERIAL_STARTED,
  INIT_CLIENT_STARTED,
  INIT_READY,
  INIT_FAILED
};

class ModbusRTU : public Component
{
public:
  // Constructor (takes base interval regardless of adaptive feature)
  ModbusRTU(Component *owner, uint8_t redePin = -1, uint8_t queueSize = 32,
            unsigned long baseMinIntervalMs = 10,
            OnRegisterChangeCallback onRegisterChange = emptyRegisterChangeCallback,
            OnWriteCallback onWrite = emptyWriteCallback,
            OnErrorCallback onError = emptyErrorCallback);

  // Destructor
  ~ModbusRTU();

  // Initialization
  MB_Error begin(HardwareSerial &serial, uint16_t baudrate = 9600);
  void end();
  bool isReady() const { return ready && initState == INIT_READY; }

  // Main processing loop (call this in your main loop)
  MB_Error process();

  // Reset the client (for error recovery)
  MB_Error reset(bool fullReset = false);

  // Coil operations
  MB_Error readCoil(uint8_t slaveId, uint16_t address);
  MB_Error writeCoil(uint8_t slaveId, uint16_t address, bool value);
  bool getCoilValue(uint8_t slaveId, uint16_t address, bool &value) const;
  bool isCoilSynchronized(uint8_t slaveId, uint16_t address) const;

  // Register operations
  MB_Error readRegister(uint8_t slaveId, uint16_t address);
  MB_Error writeRegister(uint8_t slaveId, uint16_t address, uint16_t value, bool force = false);
  MB_Error writeMultipleRegisters(uint8_t slaveId, uint16_t startAddress, uint16_t quantity, uint16_t value);
  bool getRegisterValue(uint8_t slaveId, uint16_t address, uint16_t &value) const;
  bool isRegisterSynchronized(uint8_t slaveId, uint16_t address) const;

  // New Multi-Register Read operations
  MB_Error readHoldingRegisters(uint8_t slaveId, uint16_t address, uint16_t quantity);
  MB_Error readInputRegisters(uint8_t slaveId, uint16_t address, uint16_t quantity);
  MB_Error readCoils(uint8_t slaveId, uint16_t address, uint16_t quantity);
  MB_Error readDiscreteInputs(uint8_t slaveId, uint16_t address, uint16_t quantity);

  // Status information
  bool hasErrors() const { return errorCount > 0; }
  uint32_t getErrorCount() const { return errorCount; }
  uint32_t getSuccessCount() const { return successCount; }
  float getSuccessRate() const;
  void clearStats();

  // New detailed stats getters
  uint32_t getTimeoutCount() const;
  uint32_t getCrcErrorCount() const;
  uint32_t getOtherErrorCount() const;
  uint32_t getTotalPacketsSent() const;

  // Debug output
  void printStatus() const;
  void printSlaveData(uint8_t slaveId = 0) const;
  void printSlaves() const;
  void printQueue() const;
  void printBackoffQueue() const;

  // Check if there are pending operations for a specific slave ID
  // If slaveId is 0, check for any pending operations
  bool hasPendingOperations(uint8_t slaveId = 0) const;
  bool isDeviceBusy(uint8_t slaveId) const;

  // Get operations matching specific criteria
  // Fills the provided 'results' array (up to 'maxResults') with matching operation pointers
  // Returns the number of operations found.
  uint8_t getOperations(ModbusOperation *results[], uint8_t maxResults, uint8_t slaveId = 0, uint8_t flags = 0, E_MB_OpStatus *status = nullptr, E_FN_CODE *type = nullptr) const;

  // Set callback for when a response is received (can be used for custom logic)
  void setResponseCallback(ResponseCallback callback);

  // Callback setters
  void setOnRegisterChangeCallback(OnRegisterChangeCallback callback);
  void setOnWriteCallback(OnWriteCallback callback);
  void setOnErrorCallback(OnErrorCallback callback);

  // Filter chain management
  void addFilter(ModbusOperationFilter *filter);
  void clearFilters();
  bool isOperationAlreadyPending(const ModbusOperation &op) const;

#ifdef ENABLE_ADAPTIVE_TIMEOUT
  // Configuration for Adaptive Minimum Operation Interval (Only available if enabled)
  void setAdaptiveInterval(unsigned long baseInterval = 50,
                           unsigned long maxInterval = 1000,
                           unsigned long increaseStep = 50,
                           uint16_t decreaseThreshold = 10,
                           unsigned long decreaseStep = 10);

  // Get current minimum interval (can vary if adaptive is enabled)
  unsigned long getCurrentMinInterval() const { return minOperationInterval; }
#else
  // Get current minimum interval (fixed if adaptive is disabled)
  unsigned long getCurrentMinInterval() const { return minOperationInterval; }
#endif

#ifdef DEBUG_INTERVAL_CONTROL
  // --- DEBUG ONLY --- Force setting the current min interval
  void _forceSetMinInterval(unsigned long newInterval);
#endif

  // Check if a specific coil for a slave is marked as synchronized
  // bool isCoilSynchronized(uint8_t slaveId, uint16_t address) const; // Removed duplicate

  // Clear the operation queue and optionally the slave data cache
  void clearQueue();

  // Getters for internal data (use with caution)
  const ModbusOperation *getOperationQueue() const;
  uint8_t getOperationCount() const;
  const SlaveData *getSlaveData() const;
  uint8_t getMaxSlaves() const;

  void resetDeviceOfflineState(uint8_t slaveId);
  bool isDeviceReady(uint8_t slaveId) const;
  void setOperationTimeout(uint32_t timeout) { operationTimeout = timeout; }

private:
  // Member variables
  ModbusClientRTU *client;
  bool ready;
  int8_t rePin;
  uint16_t maxQueueSize;
  HardwareSerial *serial;
  uint32_t baudRate;
  uint32_t operationTimeout;

  // Notification callbacks
  OnRegisterChangeCallback onRegisterChangeCallback;
  OnWriteCallback onWriteCallback;
  OnErrorCallback onErrorCallback;
  ResponseCallback responseCallback;

  // Filter chain
  ModbusOperationFilter *firstFilter;

  // State tracking with fixed arrays
  ModbusOperation operationQueue[MAX_PENDING_OPERATIONS]; // Single array for all operations
  uint8_t operationCount;                                 // Count of operations in the queue

  // Initialization state tracking
  E_InitState initState;
  unsigned long initStartTime;

  // Statistics & Timing
  uint16_t errorCount;
  uint16_t successCount;
  uint16_t timeoutCount;
  uint16_t crcErrorCount;
  uint16_t otherErrorCount;
  uint16_t packetsSentCount;
  unsigned long lastOperationTime;
  unsigned long minOperationInterval; // Min time between operations (Initialized to base)
  unsigned long lastProcessTime;      // Track last time process() ran fully

#ifdef ENABLE_ADAPTIVE_TIMEOUT
  // Adaptive Interval State (only compiled if feature enabled)
  unsigned long baseMinInterval; // Stores the configured base
  unsigned long maxMinInterval;
  unsigned long intervalIncreaseStep;
  uint16_t intervalDecreaseThreshold;
  unsigned long intervalDecreaseStep;
  uint16_t consecutiveSuccessCount;
#endif

  // Internal methods
  MB_Error handlePendingOperations();
  MB_Error executeOperation(ModbusOperation &op);
  MB_Error queueOperation(ModbusOperation op, bool highPriority = false);

  // Handle non-blocking initialization
  void processInitialization();

  // Template method to update values with common functionality
  template <typename ValueType, typename EntryType>
  void updateValue(uint8_t slaveId, uint16_t address, ValueType value,
                   bool synchronized, EntryType *(ModbusRTU::*findEntry)(uint8_t, uint16_t),
                   EntryType *(ModbusRTU::*createEntry)(uint8_t, uint16_t));

  // Specialized update methods that use the template
  void updateCoilValue(uint8_t slaveId, uint16_t address, bool value, bool synchronized = true);
  void updateRegisterValue(uint8_t slaveId, uint16_t address, uint16_t value, bool synchronized = true);

  void onDataReceived(ModbusMessage message, uint32_t token);
  void onErrorReceived(Error error, uint32_t token);
  ModbusOperation *findOperationByToken(uint32_t token);
  bool hasTimeToNextOperation() const;

  // Helper methods for fixed arrays
  ModbusValueEntry *findCoilEntry(uint8_t slaveId, uint16_t address);
  ModbusValueEntry *findRegisterEntry(uint8_t slaveId, uint16_t address);
  ModbusValueEntry *createCoilEntry(uint8_t slaveId, uint16_t address);
  ModbusValueEntry *createRegisterEntry(uint8_t slaveId, uint16_t address);

  // Static callback handlers (need to be static to work with eModbus)
  static void staticDataHandler(ModbusMessage message, uint32_t token);
  static void staticErrorHandler(Error error, uint32_t token);
  static ModbusRTU *instance; // Singleton instance for callbacks

  // Callback for response handling
  // ResponseCallback responseCallback; // Removed duplicate

  // Data structure to hold cached register/coil values for each slave
  SlaveData _slaveData[MAX_MODBUS_SLAVES];

  // Private helper methods
  void _processReadResponse(const ModbusOperation &op, ModbusMessage &response);
  void _processWriteResponse(const ModbusOperation &op, const ModbusMessage &response);
  void _handleSuccess(const ModbusOperation &op, const ModbusMessage &response);
  void _handleTimeout(ModbusOperation &op);
  void _handleError(ModbusOperation &op, Error error);
  void _queueOperation(const ModbusOperation &op);
  bool _addToQueue(const ModbusOperation &op);
  bool _findAndRemoveOperation(uint32_t token, ModbusOperation *removedOp = nullptr);
  ModbusOperation *_findOperationByToken(uint32_t token);
  void _updateSlaveData(uint8_t slaveId, uint16_t address, uint16_t value, bool isCoil);

  // Getter for slave data with boundary checks (const version)
  const SlaveData *_getSlaveData(uint8_t slaveId) const;
  // Getter for slave data with boundary checks (non-const version)
  SlaveData *_getSlaveData(uint8_t slaveId);

  // Device error state tracking for backoff
  struct
  {
    uint32_t currentBackoffInterval[MAX_MODBUS_SLAVES];
  } deviceErrorState;

  unsigned long deviceNextReadyTime[MAX_MODBUS_SLAVES];
};

template <typename ValueType, typename EntryType>
void ModbusRTU::updateValue(uint8_t slaveId, uint16_t address, ValueType value,
                            bool synchronized, EntryType *(ModbusRTU::*findEntry)(uint8_t, uint16_t),
                            EntryType *(ModbusRTU::*createEntry)(uint8_t, uint16_t))
{

  EntryType *entry = (this->*findEntry)(slaveId, address);
  if (entry)
  {
    if (entry->value != value)
    {
      E_FN_CODE opType = (findEntry == &ModbusRTU::findCoilEntry) ? E_FN_CODE::FN_READ_COIL : E_FN_CODE::FN_READ_HOLD_REGISTER;
      ModbusOperation op(opType, slaveId, address, value);
      onRegisterChangeCallback(op, entry->value, value);
    }
    entry->value = value;
    SET_BIT_TO(entry->flags, OP_HIGH_PRIORITY_BIT, synchronized);
    entry->lastUpdate = millis();
  }
  else
  {
    entry = (this->*createEntry)(slaveId, address);
    if (entry)
    {
      // Create a temporary ModbusOperation for the callback
      E_FN_CODE opType = (createEntry == &ModbusRTU::createCoilEntry) ? E_FN_CODE::FN_READ_COIL : E_FN_CODE::FN_READ_HOLD_REGISTER;
      ModbusOperation op(opType, slaveId, address, value);
      // Call the register change callback for new entry
      onRegisterChangeCallback(op, 0, value);
      entry->value = value;
      SET_BIT_TO(entry->flags, OP_HIGH_PRIORITY_BIT, synchronized);
      entry->lastUpdate = millis();
    }
  }
}

class Manager
{
public:
  Manager()
  {
    // Initialize all slots to nullptr
    for (int i = 0; i < MAX_MODBUS_SLAVES; i++)
    {
      devices[i] = nullptr;
    }
  }

  ~Manager()
  {
    // Clean up all devices
    removeAllDevices();
  }
  void handleResponse(uint8_t slaveId)
  {
    RTU_Base *device = getDeviceById(slaveId);
    if (device)
    {
      device->handleResponseReceived();
    }
    else
    {
      Log.warningln("Received response for unknown device ID: %d", slaveId);
    }
  }
  void handleError(const ModbusOperation &op, int errorCode, const char *errorMessage)
  {
    RTU_Base *device = getDeviceById(op.slaveId);
    if (device)
    {
      device->onError(errorCode, errorMessage);
    }
  }
  static void staticOnError(const ModbusOperation &op, int errorCode, const char *errorMessage)
  {
    if (instance != nullptr)
    {
      instance->handleError(op, errorCode, errorMessage);
    }
  }
  static void responseCallback(uint8_t slaveId)
  {
    // Call the instance method if the global instance pointer is set
    if (instance != nullptr)
    {
      instance->handleResponse(slaveId);
    }
  }

  void setAsGlobalInstance()
  {
    instance = this;
  }

  // Add a device to the manager
  bool addDevice(RTU_Base *device)
  {
    if (!device)
    {
      Log.errorln("Cannot add null device");
      return false;
    }

    uint8_t slaveId = device->slaveId;

    // Check if a device with this ID already exists
    if (devices[slaveId - 1] != nullptr)
    {
      Log.warningln("Device with ID %d already exists, replacing", slaveId);
      delete devices[slaveId - 1];
    }

    // Store the device
    devices[slaveId - 1] = device;
    return true;
  }

  bool removeDevice(uint8_t slaveId)
  {
    if (slaveId == 0 || slaveId > MAX_MODBUS_SLAVES)
    {
      Log.errorln("Invalid slave ID: %d", slaveId);
      return false;
    }

    if (devices[slaveId - 1] == nullptr)
    {
      Log.warningln("No device found with ID %d", slaveId);
      return false;
    }

    // Delete and clear the slot
    delete devices[slaveId - 1];
    devices[slaveId - 1] = nullptr;
    Log.noticeln("Removed device with ID %d", slaveId);
    return true;
  }

  RTU_Base *getDeviceById(uint8_t slaveId)
  {
    if (slaveId == 0 || slaveId > MAX_MODBUS_SLAVES)
    {
      return nullptr;
    }
    return devices[slaveId - 1];
  }

  void processDevices(ModbusRTU &manager)
  {
    for (int i = 0; i < MAX_MODBUS_SLAVES; i++)
    {
      if (devices[i] != nullptr)
      {
        devices[i]->loop();
        // devices[i]->write(manager);
        devices[i]->read(manager);
        devices[i]->updateState(manager);
      }
    }
  }
  // Remove all devices
  void removeAllDevices()
  {
    for (int i = 0; i < MAX_MODBUS_SLAVES; i++)
    {
      if (devices[i] != nullptr)
      {
        delete devices[i];
        devices[i] = nullptr;
      }
    }
  }

  void initializeDevices(ModbusRTU &manager)
  {
    for (int i = 0; i < MAX_MODBUS_SLAVES; i++)
    {
      if (devices[i] != nullptr)
      {
        devices[i]->initialize(manager);
      }
    }
  }

  int getActiveDeviceCount()
  {
    int activeCount = 0;
    for (int i = 0; i < MAX_MODBUS_SLAVES; i++)
    {
      if (devices[i] != nullptr)
      {
        activeCount++;
      }
    }
    return activeCount;
  }

  // Print status of all devices
  void printDeviceStatuses(ModbusRTU &manager)
  {
    Log.noticeln("--- Device Manager Status ---");
    int activeCount = 0;
    for (int i = 0; i < MAX_MODBUS_SLAVES; i++)
    {
      if (devices[i] != nullptr)
      {
        activeCount++;
        Log.noticeln("Slave ID %d: %s (Errors: %d)",
                     devices[i]->slaveId,
                     devices[i]->getStateString(),
                     devices[i]->errorCount);
      }
    }
    Log.noticeln("Active devices: %d/%d", activeCount, MAX_MODBUS_SLAVES);
    Log.noticeln("----------------------------");
  }

  // --- Register Change Callback Handling ---
  // Static wrapper for the callback (needed for ModbusRTU)
  static void staticOnRegisterChange(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue);

  // Member function to handle the actual logic
  void handleRegisterChange(const ModbusOperation &op, uint16_t oldValue, uint16_t newValue);

  // --- New Getters for Iteration ---
  /**
   * @brief Gets a pointer to the internal array of device pointers.
   * Use getMaxDevices() to know the size of the array.
   * @return Pointer to the first element of the devices array.
   */
  RTU_Base *const *getDevices() const { return devices; }

  /**
   * @brief Gets the maximum number of devices the manager can hold.
   * @return The size of the internal devices array.
   */
  int getMaxDevices() const { return MAX_MODBUS_SLAVES; }

  static Manager *getInstance() { return instance; }

private:
  RTU_Base *devices[MAX_MODBUS_SLAVES];
  static Manager *instance;
};

#endif // MODBUS_RTU_H
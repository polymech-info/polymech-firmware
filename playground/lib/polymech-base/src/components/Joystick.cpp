#include "Joystick.h"
#include <Arduino.h>
#include <string.h>

// Define F() macro wrapper for non-AVR architectures if needed
#if defined(ARDUINO_ARCH_AVR)
  #define JOY_L(FSTR)              F(FSTR)
#else
  #define JOY_L(FSTR)              (FSTR)
#endif

// Constructor
Joystick::Joystick(Component *owner,
                   ushort _pinUp,
                   ushort _pinDown,
                   ushort _pinLeft,
                   ushort _pinRight,
                   ushort _modbusAddress)
    : Component("Joystick", 100, Component::COMPONENT_DEFAULT, owner),
      pinUp            (_pinUp),
      pinDown          (_pinDown),
      pinLeft          (_pinLeft),
      pinRight         (_pinRight),
      modbusAddr       (_modbusAddress),
      currentPosition  (E_POSITION::CENTER),
      lastPosition     (E_POSITION::CENTER),
      mode             (E_MODE::LOCAL),
      overridePosition (E_POSITION::CENTER),
      positionStartMs  (0),
      lastReadMs       (0),
      proposedPosition (E_POSITION::CENTER),
      confirmCount     (0),
      useDebouncing    (true),
      modbusBlockCount (0)
{
    setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
    
    memset(modbusBlocks, 0, sizeof(modbusBlocks));
    const char *joyGroup = "Joystick - 4P";
    
    // Position register
    modbusBlocks[0] = {
        static_cast<ushort>(modbusAddr + static_cast<ushort>(E_REGISTER::POSITION)),
        1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_ONLY,
        static_cast<ushort>(id), static_cast<ushort>(E_REGISTER::POSITION),
        "Position", joyGroup };
        
    // Mode register (read/write)
    modbusBlocks[1] = {
        static_cast<ushort>(modbusAddr + static_cast<ushort>(E_REGISTER::MODE)),
        1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_WRITE,
        static_cast<ushort>(id), static_cast<ushort>(E_REGISTER::MODE),
        "Mode", joyGroup };
        
    // Override register (read/write)
    modbusBlocks[2] = {
        static_cast<ushort>(modbusAddr + static_cast<ushort>(E_REGISTER::OVERRIDE)),
        1, E_FN_CODE::FN_READ_HOLD_REGISTER, MB_ACCESS_READ_WRITE,
        static_cast<ushort>(id), static_cast<ushort>(E_REGISTER::OVERRIDE),
        "Override", joyGroup };
    
    modbusBlockCount = 3;
    modbusView.data = modbusBlocks;
    modbusView.count = modbusBlockCount;
}

// Initialize component
short Joystick::setup() {
    Component::setup();
    
    // Configure pins as inputs with pull-up resistors
    pinMode(pinUp, INPUT_PULLUP);
    pinMode(pinDown, INPUT_PULLUP);
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);
    
    return reset();
}

// Reset component state
short Joystick::reset() {
    // Reset state
    currentPosition = readPinsPosition();
    lastPosition = currentPosition;
    proposedPosition = currentPosition;
    confirmCount = DEBOUNCE_COUNT;
    positionStartMs = now;
    
    return E_OK;
}

// Read joystick position from GPIO pins
Joystick::E_POSITION Joystick::readPinsPosition() {
    // Note: Pull-up means LOW when pressed/active
    bool up = !digitalRead(pinUp);
    bool down = !digitalRead(pinDown);
    bool left = !digitalRead(pinLeft);
    bool right = !digitalRead(pinRight);
    
    // Detect invalid combinations (opposing directions pressed simultaneously)
    if ((up && down) || (left && right)) {
        Log.verboseln(JOY_L("Joystick ID:%d: Invalid input detected U:%d D:%d L:%d R:%d"), 
                     id, up, down, left, right);
        return E_POSITION::UNKNOWN;
    }
    
    // Determine joystick position based on pin states
    // Simple 4-way logic
    if (up) return E_POSITION::UP;
    if (down) return E_POSITION::DOWN;
    if (left) return E_POSITION::LEFT;
    if (right) return E_POSITION::RIGHT;
    
    // No buttons pressed - CENTER position
    return E_POSITION::CENTER;
}

// Main loop
short Joystick::loop() {
    Component::loop();
    
    // Check if it's time to read pins
    if (now - lastReadMs < READ_INTERVAL_MS) return E_OK;
    lastReadMs = now;
    
    // Only process input changes if in LOCAL mode
    if (mode == E_MODE::LOCAL) {
        E_POSITION candidatePosition = readPinsPosition();
        
        // Apply debouncing if enabled
        if (useDebouncing) {
            if (candidatePosition == proposedPosition) {
                if (confirmCount < DEBOUNCE_COUNT) ++confirmCount;
            } else {
                proposedPosition = candidatePosition;
                confirmCount = 1;
            }
            
            // Update position only when debouncing confirms it
            if (confirmCount >= DEBOUNCE_COUNT && proposedPosition != currentPosition) {
                lastPosition = currentPosition;
                currentPosition = proposedPosition;
                positionStartMs = now;
                Log.verboseln(JOY_L("Joystick ID:%d: Position %d → %d (debounced)"),
                             id, static_cast<int>(lastPosition), static_cast<int>(currentPosition));
                notifyStateChange();
            }
        } else {
            // Direct position update without debouncing
            if (candidatePosition != currentPosition) {
                lastPosition = currentPosition;
                currentPosition = candidatePosition;
                positionStartMs = now;
                Log.verboseln(JOY_L("Joystick ID:%d: Position %d → %d (direct)"),
                             id, static_cast<int>(lastPosition), static_cast<int>(currentPosition));
                notifyStateChange();
            }
        }
    }
    
    return E_OK;
}

// Info display
short Joystick::info(short, short) {
    Log.infoln(JOY_L("Joystick::info - ID:%d"), id);
    Log.infoln(JOY_L("  Pins - Up:%d Down:%d Left:%d Right:%d"), 
               pinUp, pinDown, pinLeft, pinRight);
    Log.infoln(JOY_L("  Mode: %s, Debouncing: %s"), 
               mode == E_MODE::LOCAL ? "LOCAL" : "REMOTE", 
               useDebouncing ? "ENABLED" : "DISABLED");
    Log.infoln(JOY_L("  Current Position: %d, Last Position: %d"), 
               static_cast<int>(currentPosition), static_cast<int>(lastPosition));
    Log.infoln(JOY_L("  Position Holding Time: %lu ms"), getHoldingTime());    
    if (mode == E_MODE::REMOTE) {
        Log.infoln(JOY_L("  Override Position: %d"), static_cast<int>(overridePosition));
    }
    return E_OK;
}

// Notify of state changes
void Joystick::notifyStateChange() {
    Component::notifyStateChange();
}

// Modbus write implementation
short Joystick::mb_tcp_write(MB_Registers *reg, short networkValue) {
    ushort addr = reg->startAddress;
    
    // Handle MODE register
    if (addr == modbusAddr + static_cast<ushort>(E_REGISTER::MODE)) {
        if (networkValue < 0 || networkValue > 1) {
            Log.warningln(JOY_L("Joystick ID:%d: Invalid mode value: %d"), id, networkValue);
            return MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        setMode(static_cast<E_MODE>(networkValue));
        Log.verboseln(JOY_L("Joystick ID:%d: Mode set to %s"), 
                      id, networkValue == 0 ? "LOCAL" : "REMOTE");
        return E_OK;
    }
    
    // Handle OVERRIDE register
    if (addr == modbusAddr + static_cast<ushort>(E_REGISTER::OVERRIDE)) {
        if (networkValue < 0 || networkValue > static_cast<int>(E_POSITION::UNKNOWN)) {
            Log.warningln(JOY_L("Joystick ID:%d: Invalid override position: %d"), id, networkValue);
            return MODBUS_ERROR_ILLEGAL_DATA_VALUE;
        }
        setOverridePosition(static_cast<E_POSITION>(networkValue));
        Log.verboseln(JOY_L("Joystick ID:%d: Override position set to %d"), id, networkValue);
        return E_OK;
    }
    
    // Handle POSITION register (read-only)
    if (addr == modbusAddr + static_cast<ushort>(E_REGISTER::POSITION)) {
        Log.warningln(JOY_L("Joystick ID:%d: Write to read-only address %d"), id, addr);
        return MODBUS_ERROR_ILLEGAL_FUNCTION;
    }
    
    Log.warningln(JOY_L("Joystick ID:%d: Write to unknown address %d"), id, addr);
    return MODBUS_ERROR_ILLEGAL_DATA_ADDRESS;
}

// Modbus read implementation
short Joystick::mb_tcp_read(MB_Registers *reg) {
    ushort addr = reg->startAddress;
    
    // Current position (accounting for mode)
    if (addr == modbusAddr + static_cast<ushort>(E_REGISTER::POSITION)) {
        return static_cast<short>(getValue());
    }
    
    // Mode (LOCAL/REMOTE)
    if (addr == modbusAddr + static_cast<ushort>(E_REGISTER::MODE)) {
        return static_cast<short>(mode);
    }
    
    // Override position value
    if (addr == modbusAddr + static_cast<ushort>(E_REGISTER::OVERRIDE)) {
        return static_cast<short>(overridePosition);
    }
    
    Log.warningln(JOY_L("Joystick ID:%d: Read from unknown address %d"), id, addr);
    return 0;
}

// Register with ModbusTCP manager
void Joystick::mb_tcp_register(ModbusTCP *mgr) {
    auto *blocks = mb_tcp_blocks();
    if (!blocks || !blocks->data) return;
    Component *thiz = const_cast<Joystick*>(this);
    for (ushort i = 0; i < blocks->count; ++i) mgr->registerModbus(thiz, blocks->data[i]);
}

// Return Modbus blocks
ModbusBlockView *Joystick::mb_tcp_blocks() const {
    return const_cast<ModbusBlockView*>(&modbusView);
}

// Register with serial bridge
short Joystick::serial_register(Bridge *bridge) {
    bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&Joystick::info);
    return E_OK;
} 
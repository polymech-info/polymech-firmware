#ifndef POS3_ANALOG_H
#define POS3_ANALOG_H

#include <ArduinoLog.h>
#include "config.h"
#include <App.h>
#include <Component.h>
#include <xmath.h>
#include <modbus/ModbusTCP.h>
#include "config-modbus.h"
#include "enums.h"

// Added POTControlMode enum definition here
enum class POTControlMode : uint8_t
{
    LOCAL  = 0,
    REMOTE = 1
};

class Bridge;

class Pos3Analog : public Component
{
private:
   const short modbusAddress;
   POTControlMode controlMode;
   int remoteValue;

   // Instance-specific storage for Modbus block definitions
   MB_Registers m_modbus_blocks[3];
   // m_modbus_view needs to be mutable to be returned as ModbusBlockView* from a const method.
   mutable ModbusBlockView m_modbus_view;

public:
   enum E_POS3_DIRECTION
   {
      UP = 1,
      MIDDLE = 0,
      DOWN = 2,
      INVALID = 10
   };

   Pos3Analog(
       Component *owner,
       short _upPin, short _downPin,
       short _id,
       short _modbusAddress) : Component("Pos3Analog", _id, Component::COMPONENT_DEFAULT, owner),
                              upPin(_upPin),
                              downPin(_downPin),
                              modbusAddress(_modbusAddress),
                              value(MIDDLE)
   {
      setNetCapability(OBJECT_NET_CAPS::E_NCAPS_MODBUS);
      controlMode = POTControlMode::LOCAL;
      remoteValue = E_POS3_DIRECTION::MIDDLE;

      // Initialize instance-specific Modbus blocks by direct struct initialization
      // This matches the 8-field structure from the original static initialization.

      // Block for "3PosAnalog Value"
      m_modbus_blocks[0] = {
          static_cast<uint16_t>(this->modbusAddress + 0), // startAddress
          1,                                              // count
          E_FN_CODE::FN_READ_HOLD_REGISTER,               // type
          MB_ACCESS_READ_ONLY,                            // access
          static_cast<uint16_t>(this->id),                // field 5 (was component id)
          0,                                              // field 6 (was offset 0)
          "3PosAnalog Value",                             // name
          "Pos3"                                          // group
      };

      // Block for "3PosAnalog Mode"
      m_modbus_blocks[1] = {
          static_cast<uint16_t>(this->modbusAddress + 1), // startAddress
          1,                                              // count
          E_FN_CODE::FN_READ_HOLD_REGISTER,               // type
          MB_ACCESS_READ_WRITE,                           // access
          static_cast<uint16_t>(this->id),                // field 5 (was component id)
          1,                                              // field 6 (was offset 1)
          "3PosAnalog Mode (0=L,1=R)",                    // name
          "Pos3"                                          // group
      };

      // Block for "3PosAnalog Remote Value"
      m_modbus_blocks[2] = {
          static_cast<uint16_t>(this->modbusAddress + 2), // startAddress
          1,                                              // count
          E_FN_CODE::FN_READ_HOLD_REGISTER,               // type
          MB_ACCESS_READ_WRITE,                           // access
          static_cast<uint16_t>(this->id),                // field 5 (was component id)
          2,                                              // field 6 (was offset 2)
          "3PosAnalog Remote Value",                      // name
          "Pos3"                                          // group
      };

      // Initialize the view to point to these instance-specific blocks
      m_modbus_view.data = m_modbus_blocks;
      m_modbus_view.count = sizeof(m_modbus_blocks) / sizeof(m_modbus_blocks[0]);
   }

   short setup() override
   {
      Component::setup();
      pinMode(upPin, INPUT);
      pinMode(downPin, INPUT);
      loop();
      return E_OK;
   }

   short info(short val0 = 0, short val1 = 0) override
   {
      Log.verboseln("3PosAnalog::info - ID: %d, UpPin: %d, DownPin: %d, Modbus Addr: %d, Value: %d, NetCaps: %d, Mode: %d, RemoteVal: %d",
                    id, upPin, downPin, modbusAddress, value, nFlags,
                    static_cast<uint8_t>(controlMode), remoteValue);
      return E_OK;
   }
   short debug() override
   {
      return info(0, 0);
   }

   short loop() override
   {
      Component::loop();
      if (now - last < ANALOG_SWITCH_READ_INTERVAL)
      {
         return E_OK;
      }
      last = now;
      int newValue = value;
      int readValue = E_POS3_DIRECTION::MIDDLE;

      if (controlMode == POTControlMode::LOCAL)
      {
         readValue = read();
         if (readValue != value)
         {
             newValue = readValue;
         }
      }
      else
      {
         if (remoteValue != value) {
            newValue = remoteValue;
         }
      }

      if (newValue != value)
      {
         value = newValue;
         notifyStateChange();
      }
      return E_OK;
   }

   int getValue() const
   {
      return value;
   }

   short mb_tcp_write(MB_Registers *reg, short networkValue) override
   {
       uint16_t addr = reg->startAddress;
       bool changed = false;

       if (addr == modbusAddress + 1)
       {
           POTControlMode newMode = (networkValue == 0) ? POTControlMode::LOCAL : POTControlMode::REMOTE;
           if (newMode != controlMode)
           {
               Log.verboseln("3PosAnalog::mb_write - ID:%d Mode change %d -> %d", id, static_cast<uint8_t>(controlMode), static_cast<uint8_t>(newMode));
               controlMode = newMode;
               changed = true;
           }
       }
       else if (addr == modbusAddress + 2)
       {
           int clampedValue = networkValue;
           if (clampedValue != E_POS3_DIRECTION::MIDDLE &&
               clampedValue != E_POS3_DIRECTION::UP &&
               clampedValue != E_POS3_DIRECTION::DOWN) {
                Log.warningln("3PosAnalog::mb_write - ID:%d Invalid remote value %d, clamping to MIDDLE(0)", id, networkValue);
               clampedValue = E_POS3_DIRECTION::MIDDLE;
           }

           if (clampedValue != remoteValue)
           {
               Log.verboseln("3PosAnalog::mb_write - ID:%d Remote value change %d -> %d", id, remoteValue, clampedValue);
               remoteValue = clampedValue;
                if (controlMode == POTControlMode::REMOTE) {
                    changed = true;
                }
           }
       }
       else if (addr == modbusAddress)
       {
            return MODBUS_ERROR_ILLEGAL_FUNCTION;
       }
       else
       {
            return E_INVALID_PARAMETER;
       }

       if (changed && controlMode == POTControlMode::REMOTE)
       {
            if (value != remoteValue)
            {
                value = remoteValue;
                notifyStateChange();
            }
        }

       return E_OK;
   }

   short mb_tcp_read(MB_Registers *reg) override
   {
      uint16_t addr = reg->startAddress;
      if (addr == modbusAddress) {
          return value;
      }
      else if (addr == modbusAddress + 1) {
          return static_cast<uint16_t>(controlMode);
      }
      else if (addr == modbusAddress + 2) {
          return remoteValue;
      }
      return 0;
   }

   void mb_tcp_register(ModbusTCP *manager) override
   {
      ModbusBlockView *blocksView = mb_tcp_blocks();
      Component *thiz = const_cast<Pos3Analog *>(this);
      for (int i = 0; i < blocksView->count; ++i)
      {
         MB_Registers info = blocksView->data[i];
         manager->registerModbus(thiz, info);
      }
   }

   ModbusBlockView *mb_tcp_blocks() const override
   {
      // Return the instance-specific Modbus block view
      return &m_modbus_view;
   }

   short serial_register(Bridge *bridge) override
   {
      bridge->registerMemberFunction(id, this, C_STR("info"), (ComponentFnPtr)&Pos3Analog::info);
      return E_OK;
   }

   int value;
   const short upPin;
   const short downPin;

protected:
   void notifyStateChange() override
   {
      Component::notifyStateChange();
   }
   unsigned long last = 0;

private:
   int read()
   {
      bool up = RANGE(analogRead(upPin), ANALOG_INPUT_MIN_THRESHOLD_0, 1000);
      bool down = RANGE(analogRead(downPin), ANALOG_INPUT_MIN_THRESHOLD_0, 1000);
      int newDirection = E_POS3_DIRECTION::MIDDLE;
      if (up && !down)
      {
         newDirection = E_POS3_DIRECTION::DOWN;
      }
      else if (down && !up)
      {
         newDirection = E_POS3_DIRECTION::UP;
      }
      else if (up && down)
      {
         newDirection = E_POS3_DIRECTION::INVALID;
      }
      return newDirection;
   }
};

#endif

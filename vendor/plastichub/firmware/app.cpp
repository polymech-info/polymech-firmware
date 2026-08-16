#include <Vector.h>
#include <Streaming.h>
#include <Arduino.h>

#include "app.h"
#include "features.h"

#include <MemoryFree.h>

static Addon *addonsArray[10];

short App::ok()
{
    return E_OK;
}

App::App() : Addon("APP", APP, 1 << STATE),
#ifdef HAS_DIRECTION_SWITCH
             dirSwitch(new DirectionSwitch()),
#endif
#ifdef HAS_TC
             mTC(new TemperatureController(modbusBridge)),
#endif
#ifdef NB_CONTROLLINO_RELAYS
             cRelays(new CRelays(modbusBridge, CRELAY_START, NB_CONTROLLINO_RELAYS)),
#endif
#ifdef ENCLOSURE_SENSOR
             enclosureSensor(new EnclosureSensor()),
#endif
#ifdef HAS_VFD
             vfd(new VFD()),
#endif
#ifdef MOTOR_HAS_TEMPERTURE
             mHeat(new MotorTemperature()),
#endif
#ifdef HAS_MOTOR_IR_SENSOR
             mSpeed(new MotorSpeed()),
#endif
#ifdef MOTOR_LOAD_PIN
             mLoad(new MotorLoad(MOTOR_LOAD_PIN)),
#endif
#ifdef HAS_SERIAL
             serialBridge(new PPSerial(Serial)),
#endif
#ifdef HAS_OP_MODE_SWITCH
#ifdef OP_MODE_ANALOG
             opModeSwitch(new OperationModeSwitch(OP_MODE_1_PIN, 120, 60, 30)),
#else
             opModeSwitch(new OperationModeSwitch(OP_MODE_1_PIN, OP_MODE_2_PIN, OP_MODE_3_PIN)),
#endif
#endif
#ifdef HAS_MODBUS_BRIDGE
             modbusBridge(new ModbusBridge()),
#endif
#ifdef HAS_OMRON_VFD_MODBUS
             omronVFD(new OmronVFD(modbusBridge, OMRON_MX2_SLAVE_ID)),
#endif
#ifdef OMRON_PID_SLAVE_START
             pids(new OmronPID(modbusBridge, OMRON_PID_SLAVE_START)),
#endif
             shredState(0)
{
    //#if defined(MODBUS_BRIDGE) && defined(HAS_VFD)
    //    vfd->modbus = modbusBridge;
    //#endif
    /// modbusBridge->debug(&Serial);
}

#ifdef HAS_STATES
String App::state()
{
    const int capacity = JSON_OBJECT_SIZE(6);
    StaticJsonDocument<capacity> doc;
    doc["0"] = id;
    doc["1"] = _state;
    doc["2"] = shredState;
    doc["3"] = overloaded;
    doc["4"] = _error;
    doc["5"] = freeMemory();
    return doc.as<String>();
}
#endif

short App::getAppState(short val)
{
    return _state;
}
void (*resetFunction)(void) = 0; // Self reset (to be used with watchdog)

short App::setAppState(short newState)
{
}

void printMem()
{
    Serial.print("mem: ");
    Serial.print(freeMemory());
    Serial.println('--');
}
short App::setup()
{
    Serial.begin(DEBUG_BAUD_RATE);
    Serial.println("Booting Firmware ...................... ");
    addons.setStorage(addonsArray);
    setup_addons();
    printMem();
    digitalWrite(STATUS_POWER_PIN, HIGH);

    Serial.print(VERSION);
    Serial.print("\n SUPPORT :");
    Serial.print(SUPPORT);
    Serial.print("\n | BUILD: ");
    Serial.print(BUILD);
    Serial.print("\n | FIRMWARE SOURCE: ");
    Serial.println(FW_SRC);

    /*
    powerSwitch->on(0);
    powerSwitch->on(1);
    delay(4000);
    dFC.initFC();
    */

#ifdef MEARSURE_PERFORMANCE
    printPerfTS = 0;
    addonLoopTime = 0;
    bridgeLoopTime = 0;
#endif
    debugTS = 0;
    comTS = 0;
    loopTS = 0;
    shredState = 0;
    overloaded = 0;
    _state = 0;
    _cstate = E_CS_OK;
    /*
    timer.every(5000, [](App *app) -> void {
        printMem();
    },
                this);
                */
}

void App::loop_service()
{
#ifdef HAS_POWER
    powerSwitch->on(POWER_PRIMARY);
#endif
    // _loop_motor_manual();
}
void App::_loop_motor_manual()
{

#if defined(HAS_DIRECTION_SWITCH) && defined(HAS_VFD)
    uchar sw = this->dirSwitch->loop();
    if (sw == 2)
    {
        this->vfd->fwd(true);
    }
    else if (sw == 1)
    {
        this->vfd->rev(true);
    }
    else
    {
        this->vfd->stop();
    }
#endif
}

void App::loop_normal()
{
}
void App::debug_mode_loop()
{
    uchar s = addons.size();
    for (uchar i = 0; i < s; i++)
    {
        Addon *addon = addons[i];
        if (addon->hasFlag(LOOP))
        {
            addon->loop();
        }
    }
}

short App::loop()
{
    loop_addons();
    loop_com();
    timer.tick();
    now = millis();
    short error = ok();
    if (error)
    {
        _error = error;
        return;
    }

#ifdef HAS_OP_MODE_SWITCH
    short op = opModeSwitch->value();
    switch (op)
    {
    case OP_DEBUG:
    {
#ifdef HAS_POWER
        powerSwitch->on(POWER_PRIMARY);
        powerSwitch->on(POWER_SECONDARY);
#endif
        break;
    }

    case OP_NORMAL:
    {
#ifdef HAS_POWER
        // powerSwitch->on(POWER_PRIMARY);
        // powerSwitch->on(POWER_SECONDARY);
#endif
        loop_normal();
        debug();
        break;
    }
    case OP_NONE:
    {
#ifdef HAS_POWER
        // powerSwitch->off(POWER_PRIMARY);
#endif
        // vfd->stop();
        // plunger->stop();
        // loopShred();
        break;
    }
    case OP_SERVICE:
    {
        // loop_normal();
        // powerSwitch->on(POWER_PRIMARY);
        // powerSwitch->on(POWER_SECONDARY);
        // vfd->rev(true);

        break;
    }
    }
#endif
}

void App::loop_com()
{
    if (millis() - comTS > 300)
    {
#if defined(HAS_BRIDGE) && defined(HAS_SERIAL)
        PPSerial::Message *msg = serialBridge->read();
        if (msg)
        {
            switch (msg->verb)
            {

            case Bridge::EC_METHOD:
            {
                char *strings[3];
                char *ptr = NULL;
                byte index = 0;

                ptr = strtok(msg->payload, ":");

                while (ptr != NULL && index < 4)
                {
                    strings[index] = ptr;
                    index++;
                    ptr = strtok(NULL, ":");
                }

                int id = atoi(strings[0]);
                char *_method = strings[1];

                SKeyVal *method = VSL::instance()->hasMethod(id, _method);
                if (method)
                {
                    int arg = atoi(strings[2]);
                    Addon *addon = (Addon *)method->instance;
                    AddonFnPtr ptr = method->mPtr;
                    short ret = (addon->*ptr)(arg);

                    if (TEST(msg->flags, Bridge::STATE))
                    {
#ifdef HAS_STATES
                        this->appState(0);
#endif
                    }
                    else if (TEST(msg->flags, Bridge::RECEIPT))
                    {
#ifdef BRIDGE_HAS_RESPONSE
                        const char *response = Bridge::CreateResponse(msg->id, 0, ret);
                        Serial.write(response);
#endif
                    }
                    if (TEST(msg->flags, Bridge::DEBUG))
                    {
                        // Serial.println("Called command");
                    }
                }
                else
                {
                    VSL::instance()->debug();
                    if (TEST(msg->flags, Bridge::DEBUG))
                    {
                        /*
                        Serial.print("Incoming message, cant find class & method ");
                        Serial.print(_class);
                        Serial.print(":");
                        Serial.print(_method);
                        Serial.print("\n");
                        */
                    }
                }
                break;
            }
            }
            msg->payload = NULL;
        }
#endif
        comTS = millis();
    }
}

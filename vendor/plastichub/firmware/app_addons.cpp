#include <Vector.h>
#include <Streaming.h>
#include <Arduino.h>
#include "app.h"
#include "config.h"
#include "types.h"
#include "common/macros.h"
#include "Addon.h"
#include "features.h"

short App::setFlag(ushort addonId, ushort flag)
{
    Addon *addon = byId(addonId);
    if (addon)
    {
        addon->setFlag(flag);
        return E_OK;
    }
    return ERROR_WARNING;
}

ushort App::numByFlag(ushort flag)
{
    uchar s = addons.size();
    uchar l = 0;
    String out = "";
    for (uchar i = 0; i < s; i++)
    {
        Addon *addon = addons[i];
        if (!!(addon->hasFlag(flag)))
        {
            l++;
        }
    }
    return l;
}

#ifdef HAS_STATES
short App::appState(short nop)
{
    uchar s = addons.size();
    uchar si = 0;
    String out = "";
    uchar l = numByFlag(STATE);
    for (uchar i = 0; i < s; i++)
    {
        Addon *addon = addons[i];
        if (!!(addon->hasFlag(STATE)))
        {
            si++;
            out += addon->state();
            if (si < l)
            {
                out += ",";
            }
        }
    }
    const char *response = Bridge::CreateResponse(STATE_RESPONSE_CODE, 0, out.c_str());
    Serial.write(response);
}
#endif
short App::debug()
{
#ifndef MEARSURE_PERFORMANCE
    if (millis() - debugTS > DEBUG_INTERVAL)
    {
        uchar s = addons.size();
        uchar nb = 0;
        for (uchar i = 0; i < s; i++)
        {
            Addon *addon = addons[i];
            if (addon->hasFlag(DEBUG))
            {
                if ((addon->debug)(&Serial))
                {
                    Serial.println("\n");
                    nb++;
                }
            }
        }
        if (nb)
        {
            Serial.println("\n");
        }
        debugTS = millis();
    }
#endif
}

short App::info()
{
    uchar s = addons.size();
    for (uchar i = 0; i < s; i++)
    {
        Addon *addon = addons[i];
        if (addon->hasFlag(INFO))
        {
            addon->info(&Serial);
            Serial << "\n";
        }
    }
}

Addon *App::byId(short id)
{
    uchar s = addons.size();
    for (uchar i = 0; i < s; i++)
    {
        Addon *addon = addons[i];
        if (addon->id == id)
        {
            return addon;
        }
    }
    return NULL;
}

void App::setup_addons()
{

#ifdef HAS_POWER
    addons.push_back((Addon *)powerSwitch);
#endif

#ifdef HAS_DIRECTION_SWITCH
    addons.push_back((Addon *)dirSwitch);
#endif

#ifdef ENCLOSURE_SENSOR
    addons.push_back((Addon *)enclosureSensor);
#endif

#ifdef MOTOR_OVERLOAD_PIN
    addons.push_back((Addon *)mOverload);
#endif

#ifdef MOTOR_IDLE_PIN
    addons.push_back((Addon *)mIdle);
#endif

#ifdef HAS_OP_MODE_SWITCH
    addons.push_back((Addon *)opModeSwitch);
#endif
#ifdef HAS_SERIAL
    addons.push_back((Addon *)serialBridge);
#endif

#ifdef MOTOR_LOAD_PIN
    addons.push_back((Addon *)mLoad);
#endif

#if ENABLED(MOTOR_HAS_TEMPERTURE)
    addons.push_back((Addon *)mHeat);
#endif

#if ENABLED(HAS_MOTOR_IR_SENSOR)
    addons.push_back((Addon *)mSpeed);
#endif

#ifdef HAS_VFD
    addons.push_back((Addon *)vfd);
#endif

#ifdef HAS_MODBUS_BRIDGE
    addons.push_back((Addon *)modbusBridge);
#endif

#ifdef HAS_TC
    addons.push_back((Addon *)mTC);
#endif

#ifdef OMRON_PID_SLAVE_START
    addons.push_back((Addon *)pids);
#endif

#ifdef HAS_OMRON_VFD_MODBUS
    addons.push_back((Addon *)omronVFD);
    omronVFD->owner = this;
#endif

#ifdef NB_CONTROLLINO_RELAYS
    addons.push_back((Addon *)cRelays);
#endif
    
    addons.push_back((Addon *)this);
    uchar s = addons.size();
    for (uchar i = 0; i < s; i++)
    {
        Addon *addon = addons[i];
        if (addon->hasFlag(SETUP))
        {
            addon->setup();
        }
    }
#ifdef HAS_BRIDGE
    REGISTER_CLASS_MEMBER_FN(POWER, powerSwitch, "on", (AddonFnPtr)&Power::on, short);
    REGISTER_CLASS_MEMBER_FN(POWER, powerSwitch, "off", (AddonFnPtr)&Power::off, short);
    REGISTER_CLASS_MEMBER_FN(VFD_CONTROL, vfd, "fwd", (AddonFnPtr)&VFD::fwd, short);
    REGISTER_CLASS_MEMBER_FN(VFD_CONTROL, vfd, "rev", (AddonFnPtr)&VFD::rev, short);
    REGISTER_CLASS_MEMBER_FN(VFD_CONTROL, vfd, "stop", (AddonFnPtr)&VFD::stop, short);

    // REGISTER_CLASS_MEMBER_FN(APP, this, "shred", (AddonFnPtr)&App::shred, short);
    REGISTER_CLASS_MEMBER_FN(APP, this, "setOverload", (AddonFnPtr)&App::setOverload, short);

    REGISTER_CLASS_MEMBER_FN(APP, this, "setAppState", (AddonFnPtr)&App::setAppState, short);
    REGISTER_CLASS_MEMBER_FN(APP, this, "getAppState", (AddonFnPtr)&App::getAppState, short);

#ifdef HAS_MODBUS_BRIDGE
    REGISTER_CLASS_MEMBER_FN(ModbusBridge, modbusBridge, "setFn", (AddonFnPtr)&ModbusBridge::setFn, short);
    REGISTER_CLASS_MEMBER_FN(ModbusBridge, modbusBridge, "setAddr", (AddonFnPtr)&ModbusBridge::setAddr, short);
    REGISTER_CLASS_MEMBER_FN(ModbusBridge, modbusBridge, "setNb", (AddonFnPtr)&ModbusBridge::setNb, short);
#endif

#ifdef HAS_STATES
    REGISTER_CLASS_MEMBER_FN(APP, this, "appState", (AddonFnPtr)&App::appState, short);
#endif

#endif
}
void App::loop_addons()
{

#ifdef MEARSURE_PERFORMANCE
    millis_t now = millis();
#endif

    uchar s = addons.size();
    for (uchar i = 0; i < s; i++)
    {
        Addon *addon = addons[i];
        if (addon->hasFlag(LOOP))
        {
            addon->now = millis();
            addon->loop();
        }
    }

#ifdef MEARSURE_PERFORMANCE
    addonLoopTime = millis() - now;
    if (millis() - printPerfTS > 3000)
    {
        printPerfTS = now;
        Serial << SPACE("Addon loop time") << addonLoopTime << "\n";
    }
#endif
    debug();
}

#include <Vector.h>
#include <Arduino.h>

#include "App.h"
#include "Bridge.h"
#include "error_codes.h"
#include "enums.h"
#include "constants.h"

#include <esp_heap_caps.h>

#define ENABLE_COMPONENT_STATS


App::App() : Component("APP", COMPONENT_KEY_APP, Component::COMPONENT_DEFAULT),
             debugTS(0),
             loopTS(0),
             _lastHeapCheck(0),
             _heapFragmentationPercent(0.0f)             
{
    DEBUG_INTERVAL = DEFAULT_DEBUG_INTERVAL;
    components.reserve(MAX_COMPONENTS);
}

float App::getHeapFragmentation() const
{
    return _heapFragmentationPercent;
}

////////////////////////////////////////////////
//
//  Component related functions
//
short App::onRun()
{
    short s = components.size();
    for (short i = 0; i < s; i++)
    {
        Component *component = components[i];
        if (!component)
        {
            Log.errorln(F("App::onRun - Found NULL component at index %d"), i);
            continue;
        }
        if (!component->owner)
        {
            component->owner = this;
        }
        component->onRun();
    }
    return E_OK;
}

short App::setup()
{
    short s = components.size();
    for (short i = 0; i < s; i++)
    {
        Component *component = components[i];
        if (!component)
        {
            Log.errorln(F("App::setup - Found NULL component at index %d"), i);
            continue;
        }
        if (!component->owner)
        {
            component->owner = this;
        }
        if (component->hasFlag(OBJECT_RUN_FLAGS::E_OF_SETUP))
        {
            component->setup();
        }
        else
        {
            Log.verboseln(F("App::setup - Skipping setup() for component (no flag): ID=%d, Name=%s"), component->id, component->name.c_str());
        }
    }
    return E_OK;
}

short App::registerComponents(Bridge *bridge)
{
    #ifdef ENABLE_SERIAL_BRIDGE
    short s = components.size();
    for (short i = 0; i < s; i++)
    {
        Component *component = components[i];
        if (component->hasFlag(OBJECT_RUN_FLAGS::E_OF_DISABLED))
        {
            continue;
        }
        component->serial_register(bridge);
    }
    #endif
    return E_OK;
}

short App::loop()
{
    Component::loop();
    now = millis();
    short s = components.size();
    for (short i = 0; i < s; i++)
    {
        Component *component = components[i];
        if (component->hasFlag(OBJECT_RUN_FLAGS::E_OF_LOOP) &&
            component->enabled())
        {

            component->_loop_start_time_us = micros();
            component->now = millis();
            component->loop();
            component->_loop_duration_us = micros() - component->_loop_start_time_us;
        }
    }

    if (now - _lastHeapCheck > 150)
    {
        _heapFragmentationPercent = (1.0 - (float)heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) / ESP.getFreeHeap()) * 100.0;
        _lastHeapCheck = now;
    }
    
    debug();
    return E_OK;
}

short App::numByFlag(ushort flag)
{
    short s = components.size();
    short l = 0;
    for (short i = 0; i < s; i++)
    {
        Component *component = components[i];
        if (!!(component->hasFlag(flag)))
        {
            l++;
        }
    }
    return l;
}

short App::setDebugParams(short val0, short val1)
{
    DEBUG_INTERVAL = val0;
    return E_OK;
}

short App::debug()
{
    if (millis() - debugTS < DEBUG_INTERVAL)
    {
        return E_OK;
    }

    debugTS = millis();
    short s = components.size();
    for (short i = 0; i < s; i++)
    {
        Component *component = components[i];
        if (component->hasFlag(OBJECT_RUN_FLAGS::E_OF_DEBUG))
        {
            component->debug();
        }
    }
    return E_OK;
}

short App::info()
{
    short s = components.size();
    for (short i = 0; i < s; i++)
    {
        Component *component = components[i];
        if (component->hasFlag(OBJECT_RUN_FLAGS::E_OF_INFO))
        {
            component->info();
        }
    }
    return E_OK;
}

Component *App::byId(ushort id)
{
    short s = components.size();
    for (short i = 0; i < s; i++)
    {
        Component *component = components[i];
        if (component->id == id)
        {
            return component;
        }
    }
    return NULL;
}

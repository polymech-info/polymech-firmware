#include <Vector.h>
#include <Arduino.h>

#include "App.h"
#include "Bridge.h"
#include "error_codes.h"
#include "enums.h"
#include "constants.h"
#include "config.h"

static Component *componentsArray[MAX_COMPONENTS];

App::App() : Component("APP", COMPONENT_KEY_APP, Component::COMPONENT_DEFAULT)
{
    DEBUG_INTERVAL = DEFAULT_DEBUG_INTERVAL;
    components.setStorage(componentsArray);
    debugTS = 0;
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
        if (component->hasFlag(OBJECT_RUN_FLAGS::E_OF_LOOP))
        {
            component->now = now;
            component->loop();
        }
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

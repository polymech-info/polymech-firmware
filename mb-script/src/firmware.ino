#ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_LEVEL_SILENT
#endif

#include <Arduino.h>

#include <ArduinoLog.h>
#include <Component.h>
#include <App.h>
#include <CommandMessage.h>
#include <Bridge.h>
#include <SerialMessage.h>

#include "config.h"
#include "macros.h"
#include "xtypes.h"
#include "StringUtils.h"


#include "PHApp.h"
PHApp testApp;
void setup()
{
  testApp.setup();
}

void loop()
{   
  testApp.loop();
}
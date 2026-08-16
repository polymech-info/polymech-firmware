#include <Vector.h>
#include <Streaming.h>
#include <Arduino.h>
#include "app.h"
#include "features.h"

#ifdef HAS_EXTRUDE_DEBUG
#define EXTRUDE_DEBUG(A) Serial.println(A);
#else
#define EXTRUDE_DEBUG(A)
#endif

short App::extrude(short value)
{
}

ushort App::loopExtrude()
{
}

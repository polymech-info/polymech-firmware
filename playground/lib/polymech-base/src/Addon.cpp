#include "Addon.h"
#include <Streaming.h>
#include <Vector.h>
#include <Arduino.h>

Addon *byId(Addons addons, uchar id)
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
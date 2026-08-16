#include "Addon.h"
#include <Streaming.h>
#include <Vector.h>
#include <Arduino.h>

bool Addon::hasFlag(uchar flag)
{
    return TEST(flags, flag);
}
void Addon::setFlag(uchar flag)
{
    flags = SBI(flags, flag);
}
void Addon::clearFlag(uchar flag)
{
    CBI(flags, flag);
}
short Addon::debug(Stream *stream)
{
}
short Addon::info(Stream *stream)
{
}
void Addon::enable()
{
    this->clearFlag(DISABLED);
}
void Addon::disable()
{
    this->setFlag(DISABLED);
}
bool Addon::enabled()
{
    return this->hasFlag(DISABLED);
}

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
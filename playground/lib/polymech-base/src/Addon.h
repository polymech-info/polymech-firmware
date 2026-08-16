#ifndef ADDON_H
#define ADDON_H

#include <WString.h>
#include <Vector.h>

#include "enums.h"
#include "error_codes.h"
#include "macros.h"
#include "Component.h"

// back compat
class Addon : public Component
{

public:
    static const int ADDON_FLAGS_DEFAULT = 1 << OBJECT_RUN_FLAGS::E_OF_LOOP | 1 << OBJECT_RUN_FLAGS::E_OF_INFO | 1 << OBJECT_RUN_FLAGS::E_OF_SETUP;

    /*
    Addon(
        String _name,
        short _id) : Component(_name, _id),
                     name(_name),
                     id(_id),
                     now(0),
                     flags(COMPONENT_DEFAULT)
    {
    }

    Addon(
        String _name,
        short _id,
        short _flags) : Component(_name, _id, _flags),
                        name(_name),
                        id(_id),
                        flags(_flags)
    {
    }

    Addon(
        String _name,
        short _id,
        short _flags,
        Addon *_owner) : Component(_name, _id, _flags, _owner),
                         name(_name),
                         id(_id),
                         flags(_flags),
                         owner(_owner)
    {
    }
    */
};

typedef Vector<Addon *> Addons;
Addon *byId(Addons addons, uchar id);
typedef short (Addon::*AddonFnPtr)(short);

#endif
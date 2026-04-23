#pragma once

#include "editor/EntityType.h"

struct EntityRef
{
    EntityType type = EntityType::None;
    int id = -1;

    bool IsValid() const
    {
        return type != EntityType::None && id >= 0;
    }
};

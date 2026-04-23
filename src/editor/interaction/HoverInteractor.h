#pragma once

#include "editor/EntityRef.h"
#include "model/ProjectDocument.h"
#include "raylib.h"

struct HoveredEntities
{
    Node* node = nullptr;
    Beam* beam = nullptr;
    Dimension* dimension = nullptr;
    EntityRef entity{};
};

namespace HoverInteractor
{
HoveredEntities ResolveHoveredEntities(
    ProjectDocument& document,
    const Camera2D& camera,
    Vector2 mouseWorldPosition,
    Vector2 mouseScreenPosition);
}


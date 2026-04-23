#pragma once

#include "model/ProjectDocument.h"
#include "raylib.h"

namespace BeamSnapResolver
{
bool TryGetBeamIntersectionSnap(
    const ProjectDocument& document,
    const Camera2D& camera,
    Vector2 mouseScreenPosition,
    Vector2& snappedPosition);
bool TryGetBeamInteriorSnap(
    const ProjectDocument& document,
    const Camera2D& camera,
    Vector2 mouseWorldPosition,
    Vector2 mouseScreenPosition,
    Vector2& snappedPosition,
    int& referenceBeamId);
}

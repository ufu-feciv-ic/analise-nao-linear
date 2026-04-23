#pragma once

#include "model/ProjectDocument.h"
#include "raylib.h"

namespace HitTester
{
Node* FindNodeAtWorldPosition(ProjectDocument& document, const Camera2D& camera, Vector2 worldPosition);
Beam* FindBeamAtWorldPosition(ProjectDocument& document, const Camera2D& camera, Vector2 worldPosition);
Dimension* FindDimensionAtScreenPosition(ProjectDocument& document, const Camera2D& camera, Vector2 screenPosition);
}

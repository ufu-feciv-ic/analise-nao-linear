#pragma once

#include "editor/EditorState.h"
#include "model/ProjectDocument.h"
#include "raylib.h"

namespace SnapResolver
{
Vector2 RoundPositionToGrid(Vector2 position, float spacing);
Vector2 GetCurrentPlacementWorldPosition(
    const EditorState& state,
    Vector2 mouseWorldPosition,
    Vector2 snappedWorldPosition);
Point2D GetCurrentTransformTargetWorldPosition(
    const EditorState& state,
    Vector2 mouseWorldPosition,
    Vector2 snappedWorldPosition,
    Node* hoveredNode,
    bool allowSelectedNodeSnap);
}

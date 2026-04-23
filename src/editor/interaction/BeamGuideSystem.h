#pragma once

#include "editor/EditorState.h"
#include "raylib.h"

namespace BeamGuideSystem
{
bool ShouldAutoCreateGuideForNewNodes(const EditorState& state);
void EnsureGuideAtPosition(EditorState& state, Vector2 position, double currentTime);
void ResetGuideHoverState(EditorState& state);
void ToggleBeamGuideAtPosition(EditorState& state, Vector2 position, double currentTime);
void UpdateActiveBeamGuides(EditorState& state, Vector2 mouseWorldPosition, float cameraZoom);
}

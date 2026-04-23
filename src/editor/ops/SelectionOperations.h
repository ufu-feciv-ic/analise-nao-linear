#pragma once

#include "editor/EditorState.h"
#include "editor/SelectionMode.h"
#include "model/ProjectDocument.h"
#include "raylib.h"

namespace SelectionOperations
{
void ApplyNodeSelection(EditorState& state, int nodeId, SelectionMode selectionMode);
void ApplyBeamSelection(EditorState& state, int beamId, SelectionMode selectionMode);
void ApplyDimensionSelection(EditorState& state, int dimensionId, SelectionMode selectionMode);

void CompleteSelectionBox(
    const ProjectDocument& document,
    EditorState& state,
    const Camera2D& camera,
    Vector2 mouseWorldPosition,
    SelectionMode selectionMode);

bool FinalizeNodeBoxSelection(
    const ProjectDocument& document,
    EditorState& state,
    const Camera2D& camera,
    Vector2 mouseWorldPosition);

bool FinalizeBeamBoxSelection(
    const ProjectDocument& document,
    EditorState& state,
    const Camera2D& camera,
    Vector2 mouseWorldPosition);
}

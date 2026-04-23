#pragma once

#include "editor/state/SelectionState.h"
#include "model/ProjectDocument.h"
#include "raylib.h"

namespace SelectionInteractor
{
Rectangle GetSelectionRectangle(const Camera2D& camera, const SelectionState& selectionState);
bool IsRightToLeftBoxSelection(const Camera2D& camera, const SelectionState& selectionState);
bool IsBeamInsideSelectionRectangle(
    const ProjectDocument& document,
    const Camera2D& camera,
    const Beam& beam,
    Rectangle selectionRect);
bool DoesBeamCrossSelectionRectangle(
    const ProjectDocument& document,
    const Camera2D& camera,
    const Beam& beam,
    Rectangle selectionRect);
bool DoesDimensionCrossSelectionRectangle(
    const ProjectDocument& document,
    const Camera2D& camera,
    const Dimension& dimension,
    Rectangle selectionRect);
}

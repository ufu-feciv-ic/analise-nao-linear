#pragma once

#include "editor/EditorState.h"
#include "model/ProjectDocument.h"
#include "raylib.h"
#include <vector>

namespace DimensionOperations
{
bool TryAddDimensionBetweenNodes(
    ProjectDocument& document,
    int startNodeId,
    int endNodeId,
    DimensionType type,
    LengthUnit lengthUnit,
    DimensionOffsetMode offsetMode,
    double offsetPixels,
    double offsetWorld);

bool UpdateDimensionPreviewFromMouse(
    const ProjectDocument& document,
    const Camera2D& camera,
    Vector2 previewScreenPosition,
    int startNodeId,
    int endNodeId,
    DimensionType& type,
    double& offsetPixels,
    double& offsetWorld);

bool TryAlignDimensionToReference(
    const ProjectDocument& document,
    const Camera2D& camera,
    int startNodeId,
    int endNodeId,
    const Dimension& referenceDimension,
    DimensionType& type,
    double& offsetPixels,
    double& offsetWorld);

bool IsNodeCollinearWithDimensionChain(
    const ProjectDocument& document,
    const EditorState& state,
    int candidateNodeId);

bool TryBeginDimensionCreationFromBeam(
    const ProjectDocument& document,
    EditorState& state,
    const Camera2D& camera,
    const Beam& beam);

bool TryGetChainedDimensionNodeFromBeam(
    const ProjectDocument& document,
    const EditorState& state,
    const Beam& beam,
    Vector2 mouseWorldPosition,
    int& candidateNodeId);

bool ApplyDimensionMoveOffsets(
    ProjectDocument& document,
    const std::vector<int>& movingDimensionIds,
    DimensionType type,
    DimensionOffsetMode offsetMode,
    double offsetPixels,
    double offsetWorld);

bool BeginDimensionMoveSelection(
    ProjectDocument& document,
    EditorState& state,
    int dimensionId);
}

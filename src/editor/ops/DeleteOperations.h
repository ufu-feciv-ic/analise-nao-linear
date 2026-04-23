#pragma once

#include "editor/EditorState.h"
#include "model/ProjectDocument.h"

namespace DeleteOperations
{
bool DeleteSelection(ProjectDocument& document, EditorState& state);
bool DeleteSelectedNodes(ProjectDocument& document, EditorState& state);
bool DeleteSelectedBeams(ProjectDocument& document, EditorState& state);
bool DeleteSelectedDimensions(ProjectDocument& document, EditorState& state);

bool DeleteNodeById(ProjectDocument& document, EditorState& state, int nodeId);
bool DeleteBeamById(ProjectDocument& document, EditorState& state, int beamId);
bool DeleteDimensionById(ProjectDocument& document, EditorState& state, int dimensionId);

void PurgeInvalidSelection(const ProjectDocument& document, EditorState& state);
}

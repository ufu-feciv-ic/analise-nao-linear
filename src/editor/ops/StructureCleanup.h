#pragma once

#include <functional>

#include "editor/EditorState.h"
#include "model/ProjectDocument.h"

namespace StructureCleanup
{
using AssignDistributedLoadAlongBeamPathFn =
    std::function<void(int originalStartNodeId, int originalEndNodeId, const DistributedLoadValue& loadValue)>;

void MergeCoincidentNodes(ProjectDocument& document, EditorState& state);
void RebuildBeamsAfterNodeChanges(
    ProjectDocument& document,
    EditorState& state,
    const AssignDistributedLoadAlongBeamPathFn& assignDistributedLoadAlongBeamPath);
void CleanupDimensionsAfterNodeChanges(ProjectDocument& document, EditorState& state);
}

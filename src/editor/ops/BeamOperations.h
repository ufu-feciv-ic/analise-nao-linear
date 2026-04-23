#pragma once

#include <cstdint>
#include <functional>
#include <unordered_set>
#include <vector>

#include "editor/EditorState.h"
#include "model/ProjectDocument.h"

namespace BeamOperations
{
using AssignDistributedLoadAlongBeamPathFn =
    std::function<void(int originalStartNodeId, int originalEndNodeId, const DistributedLoadValue& loadValue)>;

bool ResolveBeamDirection(const ProjectDocument& document, int& startNodeId, int& endNodeId);
std::vector<int> CollectNodeIdsOnSegment(const ProjectDocument& document, int startNodeId, int endNodeId);
int AddBeamBetweenNodesWithProperties(
    ProjectDocument& document,
    int startNodeId,
    int endNodeId,
    int materialId,
    int sectionId);
bool EnsureBeamPathBetweenNodes(
    ProjectDocument& document,
    int startNodeId,
    int endNodeId,
    int materialId,
    int sectionId);
bool AppendBeamPathBetweenNodesFast(
    ProjectDocument& document,
    int startNodeId,
    int endNodeId,
    int materialId,
    int sectionId,
    std::unordered_set<std::uint64_t>& existingBeamKeys);
bool SplitBeamsAtNode(
    ProjectDocument& document,
    EditorState& state,
    int nodeId,
    const AssignDistributedLoadAlongBeamPathFn& assignDistributedLoadAlongBeamPath);
}

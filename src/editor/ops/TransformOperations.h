#pragma once

#include <cstdint>
#include <functional>
#include <unordered_set>
#include <vector>

#include "editor/Selection.h"
#include "model/DistributedLoad.h"
#include "model/ProjectDocument.h"

namespace TransformOperations
{
using SplitBeamAtNodeCallback = std::function<bool(int nodeId)>;
using AppendBeamPathCallback = std::function<bool(
    int startNodeId,
    int endNodeId,
    int materialId,
    int sectionId,
    std::unordered_set<std::uint64_t>& existingBeamKeys)>;
using AssignDistributedLoadCallback = std::function<void(
    int originalStartNodeId,
    int originalEndNodeId,
    const DistributedLoadValue& loadValue)>;
using NormalizeStructureCallback = std::function<void()>;

std::vector<int> CollectTransformNodeIds(const ProjectDocument& document, const Selection& selection);
bool HasTransformSelection(const Selection& selection);
void ApplyMoveSelection(
    ProjectDocument& document,
    const Selection& selection,
    Point2D basePointWorld,
    Point2D targetPointWorld,
    const NormalizeStructureCallback& normalizeStructureAfterMove);
void ApplyCopySelection(
    ProjectDocument& document,
    const Selection& selection,
    Point2D basePointWorld,
    Point2D targetPointWorld,
    const SplitBeamAtNodeCallback& splitBeamAtNode,
    const AppendBeamPathCallback& appendBeamPathBetweenNodesFast,
    const AssignDistributedLoadCallback& assignDistributedLoadAlongBeamPath);
bool ApplyMirrorSelection(
    ProjectDocument& document,
    const Selection& selection,
    Point2D axisStartPointWorld,
    Point2D axisEndPointWorld,
    bool mirrorKeepsOriginal,
    const SplitBeamAtNodeCallback& splitBeamAtNode,
    const AppendBeamPathCallback& appendBeamPathBetweenNodesFast,
    const AssignDistributedLoadCallback& assignDistributedLoadAlongBeamPath,
    const NormalizeStructureCallback& normalizeStructureAfterMove);
}

#include "editor/ops/StructureCleanup.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include "editor/ops/BeamOperations.h"

namespace
{
    constexpr double geometryTolerance = 0.00001;

    void MergeNodeAttributes(Node& target, const Node& source)
    {
        if (target.support == SupportType::None && source.support != SupportType::None)
        {
            target.support = source.support;
        }

        if (std::abs(target.load.fx) <= geometryTolerance && std::abs(source.load.fx) > geometryTolerance)
        {
            target.load.fx = source.load.fx;
        }

        if (std::abs(target.load.fy) <= geometryTolerance && std::abs(source.load.fy) > geometryTolerance)
        {
            target.load.fy = source.load.fy;
        }

        if (std::abs(target.load.mz) <= geometryTolerance && std::abs(source.load.mz) > geometryTolerance)
        {
            target.load.mz = source.load.mz;
        }
    }

}

namespace StructureCleanup
{
void MergeCoincidentNodes(ProjectDocument& document, EditorState& state)
{
    document.InvalidateLookupIndices();
    std::unordered_map<int, int> survivorByNodeId;
    survivorByNodeId.reserve(document.nodes.size());

    for (std::size_t i = 0; i < document.nodes.size(); ++i)
    {
        const Node& candidateNode = document.nodes[i];
        int survivorId = candidateNode.id;

        for (std::size_t j = 0; j < i; ++j)
        {
            const Node& survivorNode = document.nodes[j];
            if (std::abs(candidateNode.position.x - survivorNode.position.x) <= geometryTolerance &&
                std::abs(candidateNode.position.y - survivorNode.position.y) <= geometryTolerance)
            {
                survivorId = survivorNode.id;
                break;
            }
        }

        survivorByNodeId[candidateNode.id] = survivorId;
    }

    bool mergedAnyNode = false;

    for (Node& node : document.nodes)
    {
        const int survivorId = survivorByNodeId[node.id];
        if (survivorId == node.id)
        {
            continue;
        }

        Node* survivorNode = document.FindNodeById(survivorId);
        if (survivorNode != nullptr)
        {
            MergeNodeAttributes(*survivorNode, node);
        }

        mergedAnyNode = true;
    }

    if (!mergedAnyNode)
    {
        return;
    }

    for (Beam& beam : document.beams)
    {
        beam.startNodeId = survivorByNodeId[beam.startNodeId];
        beam.endNodeId = survivorByNodeId[beam.endNodeId];
    }

    for (int& nodeId : state.selectionState.selection.nodeIds)
    {
        nodeId = survivorByNodeId[nodeId];
    }

    for (Dimension& dimension : document.dimensions)
    {
        dimension.startNodeId = survivorByNodeId[dimension.startNodeId];
        dimension.endNodeId = survivorByNodeId[dimension.endNodeId];
    }

    if (state.dimensionTool.dimensionStartNodeId >= 0)
    {
        state.dimensionTool.dimensionStartNodeId = survivorByNodeId[state.dimensionTool.dimensionStartNodeId];
    }

    if (state.dimensionTool.dimensionEndNodeId >= 0)
    {
        state.dimensionTool.dimensionEndNodeId = survivorByNodeId[state.dimensionTool.dimensionEndNodeId];
    }

    if (state.dimensionTool.dimensionChainReferenceNodeId >= 0)
    {
        state.dimensionTool.dimensionChainReferenceNodeId = survivorByNodeId[state.dimensionTool.dimensionChainReferenceNodeId];
    }

    document.nodes.erase(
        std::remove_if(
            document.nodes.begin(),
            document.nodes.end(),
            [&](const Node& node)
            {
                return survivorByNodeId[node.id] != node.id;
            }),
        document.nodes.end());

    document.InvalidateLookupIndices();

    std::sort(state.selectionState.selection.nodeIds.begin(), state.selectionState.selection.nodeIds.end());
    state.selectionState.selection.nodeIds.erase(
        std::unique(
            state.selectionState.selection.nodeIds.begin(),
            state.selectionState.selection.nodeIds.end()),
        state.selectionState.selection.nodeIds.end());
}

void RebuildBeamsAfterNodeChanges(
    ProjectDocument& document,
    EditorState& state,
    const AssignDistributedLoadAlongBeamPathFn& assignDistributedLoadAlongBeamPath)
{
    document.InvalidateLookupIndices();
    const std::vector<Beam> originalBeams = document.beams;
    const std::vector<BeamDistributedLoad> originalDistributedLoads = document.distributedLoads;
    const std::vector<int> originallySelectedBeamIds = state.selectionState.selection.beamIds;
    std::unordered_set<std::uint64_t> existingBeamKeys;
    existingBeamKeys.reserve(originalBeams.size() * 2);

    document.beams.clear();
    document.distributedLoads.clear();
    state.selectionState.selection.ClearBeams();

    for (const Beam& beam : originalBeams)
    {
        BeamOperations::AppendBeamPathBetweenNodesFast(
            document,
            beam.startNodeId,
            beam.endNodeId,
            beam.materialId,
            beam.sectionId,
            existingBeamKeys);
    }

    for (const BeamDistributedLoad& distributedLoad : originalDistributedLoads)
    {
        const Beam* originalBeam = nullptr;
        for (const Beam& beam : originalBeams)
        {
            if (beam.id == distributedLoad.beamId)
            {
                originalBeam = &beam;
                break;
            }
        }

        if (originalBeam == nullptr)
        {
            continue;
        }

        assignDistributedLoadAlongBeamPath(
            originalBeam->startNodeId,
            originalBeam->endNodeId,
            distributedLoad.value);
    }

    for (const Beam& beam : originalBeams)
    {
        const bool wasSelected =
            std::find(
                originallySelectedBeamIds.begin(),
                originallySelectedBeamIds.end(),
                beam.id) != originallySelectedBeamIds.end();
        if (!wasSelected)
        {
            continue;
        }

        const std::vector<int> nodeIdsOnSegment =
            BeamOperations::CollectNodeIdsOnSegment(document, beam.startNodeId, beam.endNodeId);
        for (std::size_t i = 1; i < nodeIdsOnSegment.size(); ++i)
        {
            const Beam* rebuiltBeam = document.FindBeamBetweenNodes(
                nodeIdsOnSegment[i - 1],
                nodeIdsOnSegment[i]);
            if (rebuiltBeam != nullptr)
            {
                state.selectionState.selection.AddBeam(rebuiltBeam->id);
            }
        }
    }

    state.hover.ClearEntity();
    document.InvalidateLookupIndices();
}

void CleanupDimensionsAfterNodeChanges(ProjectDocument& document, EditorState& state)
{
    document.dimensions.erase(
        std::remove_if(
            document.dimensions.begin(),
            document.dimensions.end(),
            [](const Dimension& dimension)
            {
                return dimension.startNodeId == dimension.endNodeId;
            }),
        document.dimensions.end());

    document.InvalidateLookupIndices();

    if (state.dimensionTool.dimensionStartNodeId == state.dimensionTool.dimensionEndNodeId)
    {
        state.dimensionTool.dimensionEndNodeId = -1;
    }
}
}


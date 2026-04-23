#include "editor/ops/DeleteOperations.h"

#include <algorithm>

namespace DeleteOperations
{
bool DeleteSelection(ProjectDocument& document, EditorState& state)
{
    const std::vector<int> beamIds = state.selectionState.selection.beamIds;
    const std::vector<int> nodeIds = state.selectionState.selection.nodeIds;
    const std::vector<int> dimensionIds = state.selectionState.selection.dimensionIds;

    bool removed = false;

    if (!beamIds.empty())
    {
        removed = document.RemoveBeams(beamIds) || removed;
    }

    if (!dimensionIds.empty())
    {
        removed = document.RemoveDimensions(dimensionIds) || removed;
    }

    if (!nodeIds.empty())
    {
        removed = document.RemoveNodes(nodeIds) || removed;
    }

    if (!removed)
    {
        return false;
    }

    state.selectionState.selection.Clear();
    state.hover.ClearEntity();
    state.beamTool.isCreatingBeam = false;
    state.beamTool.beamStartNodeId = -1;
    return true;
}

bool DeleteSelectedNodes(ProjectDocument& document, EditorState& state)
{
    const std::vector<int> nodeIds = state.selectionState.selection.nodeIds;
    if (!document.RemoveNodes(nodeIds))
    {
        return false;
    }

    for (int nodeId : nodeIds)
    {
        state.selectionState.selection.RemoveNode(nodeId);
    }

    PurgeInvalidSelection(document, state);
    state.hover.ClearEntity();
    state.beamTool.isCreatingBeam = false;
    state.beamTool.beamStartNodeId = -1;
    return true;
}

bool DeleteSelectedBeams(ProjectDocument& document, EditorState& state)
{
    const std::vector<int> beamIds = state.selectionState.selection.beamIds;
    if (!document.RemoveBeams(beamIds))
    {
        return false;
    }

    for (int beamId : beamIds)
    {
        state.selectionState.selection.RemoveBeam(beamId);
    }

    PurgeInvalidSelection(document, state);
    state.hover.ClearEntity();
    return true;
}

bool DeleteSelectedDimensions(ProjectDocument& document, EditorState& state)
{
    const std::vector<int> dimensionIds = state.selectionState.selection.dimensionIds;
    if (!document.RemoveDimensions(dimensionIds))
    {
        return false;
    }

    for (int dimensionId : dimensionIds)
    {
        state.selectionState.selection.RemoveDimension(dimensionId);
    }

    PurgeInvalidSelection(document, state);
    state.hover.ClearEntity();
    return true;
}

bool DeleteNodeById(ProjectDocument& document, EditorState& state, int nodeId)
{
    if (!document.RemoveNode(nodeId))
    {
        return false;
    }

    state.selectionState.selection.RemoveNode(nodeId);
    PurgeInvalidSelection(document, state);
    state.hover.ClearEntity();

    if (state.beamTool.beamStartNodeId == nodeId)
    {
        state.beamTool.isCreatingBeam = false;
        state.beamTool.beamStartNodeId = -1;
    }

    return true;
}

bool DeleteBeamById(ProjectDocument& document, EditorState& state, int beamId)
{
    if (!document.RemoveBeam(beamId))
    {
        return false;
    }

    state.selectionState.selection.RemoveBeam(beamId);
    PurgeInvalidSelection(document, state);
    state.hover.ClearEntity();
    return true;
}

bool DeleteDimensionById(ProjectDocument& document, EditorState& state, int dimensionId)
{
    if (!document.RemoveDimension(dimensionId))
    {
        return false;
    }

    state.selectionState.selection.RemoveDimension(dimensionId);
    PurgeInvalidSelection(document, state);
    state.hover.ClearEntity();

    if (state.dimensionTool.movingDimensionId == dimensionId)
    {
        state.dimensionTool.dimensionMoveStep = DimensionToolState::DimensionMoveStep::None;
        state.dimensionTool.movingDimensionId = -1;
        state.dimensionTool.movingDimensionIds.clear();
    }
    else
    {
        state.dimensionTool.movingDimensionIds.erase(
            std::remove(
                state.dimensionTool.movingDimensionIds.begin(),
                state.dimensionTool.movingDimensionIds.end(),
                dimensionId),
            state.dimensionTool.movingDimensionIds.end());
    }

    return true;
}

void PurgeInvalidSelection(const ProjectDocument& document, EditorState& state)
{
    state.selectionState.selection.nodeIds.erase(
        std::remove_if(
            state.selectionState.selection.nodeIds.begin(),
            state.selectionState.selection.nodeIds.end(),
            [&](int id)
            {
                return document.FindNodeById(id) == nullptr;
            }),
        state.selectionState.selection.nodeIds.end());

    state.selectionState.selection.beamIds.erase(
        std::remove_if(
            state.selectionState.selection.beamIds.begin(),
            state.selectionState.selection.beamIds.end(),
            [&](int id)
            {
                return document.FindBeamById(id) == nullptr;
            }),
        state.selectionState.selection.beamIds.end());

    state.selectionState.selection.dimensionIds.erase(
        std::remove_if(
            state.selectionState.selection.dimensionIds.begin(),
            state.selectionState.selection.dimensionIds.end(),
            [&](int id)
            {
                return document.FindDimensionById(id) == nullptr;
            }),
        state.selectionState.selection.dimensionIds.end());

    state.dimensionTool.movingDimensionIds.erase(
        std::remove_if(
            state.dimensionTool.movingDimensionIds.begin(),
            state.dimensionTool.movingDimensionIds.end(),
            [&](int id)
            {
                return document.FindDimensionById(id) == nullptr;
            }),
        state.dimensionTool.movingDimensionIds.end());
}
}


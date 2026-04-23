#include "editor/Editor.h"
#include "editor/ops/TransformOperations.h"

#include <utility>

std::vector<int> Editor::CollectTransformNodeIds() const
{
    return TransformOperations::CollectTransformNodeIds(document, state.selectionState.selection);
}

bool Editor::HasTransformSelection() const
{
    return TransformOperations::HasTransformSelection(state.selectionState.selection);
}

bool Editor::IsTransformTool(EditorTool tool) const
{
    return tool == EditorTool::MoveNode ||
           tool == EditorTool::CopySelection ||
           tool == EditorTool::MirrorSelection;
}

void Editor::ActivateTransformTool(EditorTool tool)
{
    if (!IsTransformTool(tool))
    {
        return;
    }

    if (tool == EditorTool::MoveNode && state.activeTool == EditorTool::CopySelection)
    {
        state.ClearSelection();
    }

    state.lastSelectedTool = tool;
    state.activeTool = tool;
    if (HasTransformSelection())
    {
        BeginMoveSelection();
    }
    else
    {
        state.transformTool.moveSelectionStep = TransformToolState::MoveSelectionStep::AwaitSelectionConfirm;
    }
}

void Editor::ApplyTransformTool(EditorTool tool, Point2D targetPointWorld)
{
    switch (tool)
    {
    case EditorTool::MoveNode:
        ApplyMoveSelection(targetPointWorld);
        break;

    case EditorTool::CopySelection:
        ApplyCopySelection(targetPointWorld);
        break;

    case EditorTool::MirrorSelection:
        ApplyMirrorSelection(targetPointWorld);
        break;

    default:
        break;
    }
}

void Editor::BeginMoveSelection()
{
    state.transformTool.moveSelectionStep = TransformToolState::MoveSelectionStep::AwaitBasePoint;
}

void Editor::ApplyMoveSelection(Point2D targetPointWorld)
{
    ProjectDocument beforeDocument = document;
    TransformOperations::ApplyMoveSelection(
        document,
        state.selectionState.selection,
        state.transformTool.moveBasePointWorld,
        targetPointWorld,
        [&]()
        {
            NormalizeStructureAfterMove();
        });
    RecordDocumentChange(std::move(beforeDocument), true);
    state.ClearSelection();
    state.ResetMoveSelection();
    state.activeTool = EditorTool::Select;
}

void Editor::ApplyCopySelection(Point2D targetPointWorld)
{
    ProjectDocument beforeDocument = document;
    TransformOperations::ApplyCopySelection(
        document,
        state.selectionState.selection,
        state.transformTool.moveBasePointWorld,
        targetPointWorld,
        [&](int nodeId)
        {
            return SplitBeamsAtNode(nodeId);
        },
        [&](int startNodeId,
            int endNodeId,
            int materialId,
            int sectionId,
            std::unordered_set<std::uint64_t>& existingBeamKeys)
        {
            return AppendBeamPathBetweenNodesFast(
                startNodeId,
                endNodeId,
                materialId,
                sectionId,
                existingBeamKeys);
        },
        [&](int originalStartNodeId, int originalEndNodeId, const DistributedLoadValue& loadValue)
        {
            AssignDistributedLoadAlongBeamPath(originalStartNodeId, originalEndNodeId, loadValue);
        });
    RecordDocumentChange(std::move(beforeDocument), true);
}

void Editor::ApplyMirrorSelection(Point2D axisEndPointWorld)
{
    ProjectDocument beforeDocument = document;
    if (!TransformOperations::ApplyMirrorSelection(
            document,
            state.selectionState.selection,
            state.transformTool.moveBasePointWorld,
            axisEndPointWorld,
            state.transformTool.mirrorKeepsOriginal,
            [&](int nodeId)
            {
                return SplitBeamsAtNode(nodeId);
            },
            [&](int startNodeId,
                int endNodeId,
                int materialId,
                int sectionId,
                std::unordered_set<std::uint64_t>& existingBeamKeys)
            {
                return AppendBeamPathBetweenNodesFast(
                    startNodeId,
                    endNodeId,
                    materialId,
                    sectionId,
                    existingBeamKeys);
            },
            [&](int originalStartNodeId, int originalEndNodeId, const DistributedLoadValue& loadValue)
            {
                AssignDistributedLoadAlongBeamPath(originalStartNodeId, originalEndNodeId, loadValue);
            },
            [&]()
            {
                NormalizeStructureAfterMove();
            }))
    {
        return;
    }
    RecordDocumentChange(std::move(beforeDocument), true);
    state.ResetMoveSelection();
    state.activeTool = EditorTool::Select;
}


#include "editor/Editor.h"
#include "editor/interaction/SelectionInteractor.h"
#include "editor/ops/SelectionOperations.h"
#include "editor/ops/SupportLoadOperations.h"

SelectionMode Editor::GetSelectionMode() const
{
    const bool isSubtractiveSelection = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if (isSubtractiveSelection)
    {
        return SelectionMode::Remove;
    }

    const bool isAdditiveSelection = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    if (isAdditiveSelection)
    {
        return SelectionMode::Add;
    }

    return SelectionMode::Replace;
}

void Editor::ApplySelectionClick(
    Node* hoveredNode,
    Beam* hoveredBeam,
    Dimension* hoveredDimension,
    SelectionMode selectionMode)
{
    if (hoveredNode != nullptr)
    {
        SelectionOperations::ApplyNodeSelection(state, hoveredNode->id, selectionMode);
        state.selectionState.isBoxSelecting = false;
        return;
    }

    if (hoveredBeam != nullptr)
    {
        SelectionOperations::ApplyBeamSelection(state, hoveredBeam->id, selectionMode);
        state.selectionState.isBoxSelecting = false;
        return;
    }

    if (hoveredDimension != nullptr)
    {
        SelectionOperations::ApplyDimensionSelection(state, hoveredDimension->id, selectionMode);
        state.selectionState.isBoxSelecting = false;
        return;
    }

    BeginBoxSelection();
}

void Editor::HandleSelectionBox(SelectionMode selectionMode)
{
    if (state.selectionState.isBoxSelecting && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        UpdateBoxSelection();
    }

    if (state.selectionState.isBoxSelecting && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        EndBoxSelection(selectionMode);
    }
}

bool Editor::IsRightToLeftBoxSelection() const
{
    return SelectionInteractor::IsRightToLeftBoxSelection(camera, state.selectionState);
}

bool Editor::IsBeamInsideSelectionRectangle(const Beam& beam, Rectangle selectionRect) const
{
    return SelectionInteractor::IsBeamInsideSelectionRectangle(document, camera, beam, selectionRect);
}

bool Editor::DoesBeamCrossSelectionRectangle(const Beam& beam, Rectangle selectionRect) const
{
    return SelectionInteractor::DoesBeamCrossSelectionRectangle(document, camera, beam, selectionRect);
}

void Editor::BeginBoxSelection()
{
    state.selectionState.isBoxSelecting = true;
    state.selectionState.boxSelectStartWorld = mouseWorldPosition;
    state.selectionState.boxSelectEndWorld = mouseWorldPosition;
}

void Editor::UpdateBoxSelection()
{
    state.selectionState.boxSelectEndWorld = mouseWorldPosition;
}

void Editor::EndBoxSelection(SelectionMode selectionMode)
{
    SelectionOperations::CompleteSelectionBox(
        document,
        state,
        camera,
        mouseWorldPosition,
        selectionMode);
}

bool Editor::FinalizeNodeBoxSelection()
{
    return SelectionOperations::FinalizeNodeBoxSelection(
        document,
        state,
        camera,
        mouseWorldPosition);
}

void Editor::HandleBoxSelectionReleaseWithoutWorldInput(SelectionMode selectionMode)
{
    if (!state.selectionState.isBoxSelecting || !IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    switch (state.activeTool)
    {
    case EditorTool::Select:
    case EditorTool::MoveNode:
    case EditorTool::CopySelection:
    case EditorTool::MirrorSelection:
        EndBoxSelection(selectionMode);
        break;

    case EditorTool::SetSupportNone:
    case EditorTool::SetSupportX:
    case EditorTool::SetSupportY:
    case EditorTool::SetSupportXY:
    case EditorTool::SetSupportFixed:
        ApplySupportTypeInSelectionRectangle(
            SupportLoadOperations::GetSupportTypeForTool(state.activeTool));
        if (state.selectionState.selection.HasNodes())
        {
            state.activeTool = EditorTool::Select;
        }
        break;

    case EditorTool::AddPointLoad:
        ApplyPointLoadInSelectionRectangle(state.loadTool.pendingNodalLoad);
        if (state.selectionState.selection.HasNodes())
        {
            state.activeTool = EditorTool::Select;
        }
        break;

    case EditorTool::RemovePointLoad:
        ClearPointLoadInSelectionRectangle();
        if (state.selectionState.selection.HasNodes())
        {
            state.activeTool = EditorTool::Select;
        }
        break;

    case EditorTool::AddDistributedLoad:
        ApplyDistributedLoadInSelectionRectangle(state.loadTool.pendingDistributedLoad);
        if (state.selectionState.selection.HasBeams())
        {
            state.activeTool = EditorTool::Select;
        }
        break;

    case EditorTool::RemoveDistributedLoad:
        ClearDistributedLoadInSelectionRectangle();
        if (state.selectionState.selection.HasBeams())
        {
            state.activeTool = EditorTool::Select;
        }
        break;

    case EditorTool::RemoveNode:
        if (DeleteNodesInSelectionRectangle())
        {
            state.activeTool = EditorTool::Select;
        }
        break;

    case EditorTool::RemoveBeam:
        if (DeleteBeamsInSelectionRectangle())
        {
            state.activeTool = EditorTool::Select;
        }
        break;

    default:
        state.selectionState.isBoxSelecting = false;
        break;
    }
}

bool Editor::FinalizeBeamBoxSelection()
{
    return SelectionOperations::FinalizeBeamBoxSelection(
        document,
        state,
        camera,
        mouseWorldPosition);
}

bool Editor::DeleteNodesInSelectionRectangle()
{
    if (!FinalizeNodeBoxSelection())
    {
        return false;
    }

    const std::size_t nodeCountBefore = document.nodes.size();
    DeleteSelectedNodes();
    return document.nodes.size() != nodeCountBefore;
}

bool Editor::DeleteBeamsInSelectionRectangle()
{
    if (!FinalizeBeamBoxSelection())
    {
        return false;
    }

    const std::size_t beamCountBefore = document.beams.size();
    DeleteSelectedBeams();
    return document.beams.size() != beamCountBefore;
}

Rectangle Editor::GetSelectionRectangle() const
{
    return SelectionInteractor::GetSelectionRectangle(camera, state.selectionState);
}

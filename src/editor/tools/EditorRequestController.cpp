#include "editor/tools/EditorRequestController.h"

#include "editor/Editor.h"
#include "editor/ops/SupportLoadOperations.h"

#include <utility>

namespace
{
NodalLoad MakeZeroNodalLoad()
{
    return NodalLoad{0.0, 0.0, 0.0};
}

DistributedLoadValue MakeZeroDistributedLoad()
{
    return DistributedLoadValue{};
}
} // namespace

void EditorRequestController::Apply(Editor& editor, EditorRequest request) const
{
    if (request != EditorRequest::None && request != EditorRequest::ActivateAddBeamTool)
    {
        editor.CancelBeamCreationOperation();
    }

    if (request != EditorRequest::None && request != EditorRequest::ActivateAddDimensionTool)
    {
        editor.CancelDimensionCreationOperation();
    }

    if (request != EditorRequest::None && request != EditorRequest::ActivateMoveDimensionTool)
    {
        editor.CancelDimensionMoveOperation();
    }

    auto activateTool = [&](EditorTool tool)
    {
        editor.state.lastSelectedTool = tool;
        editor.state.activeTool = tool;
        editor.state.ResetMoveSelection();
    };

    auto applySupportOnSelection = [&](EditorTool tool)
    {
        activateTool(tool);
        if (!editor.state.selectionState.selection.HasNodes())
        {
            return;
        }

        ProjectDocument beforeDocument = editor.document;
        if (SupportLoadOperations::ApplySupportTypeToSelection(
                editor.document,
                editor.state,
                SupportLoadOperations::GetSupportTypeForTool(tool)))
        {
            editor.RecordDocumentChange(std::move(beforeDocument), true);
        }
        editor.state.activeTool = EditorTool::Select;
    };

    auto applyPointLoadOnSelection = [&](EditorTool tool, const NodalLoad& load)
    {
        activateTool(tool);
        if (!editor.state.selectionState.selection.HasNodes())
        {
            return;
        }

        ProjectDocument beforeDocument = editor.document;
        if (SupportLoadOperations::ApplyPointLoadToSelection(editor.document, editor.state, load))
        {
            editor.RecordDocumentChange(std::move(beforeDocument), true);
        }
        editor.state.activeTool = EditorTool::Select;
    };

    auto applyDistributedLoadOnSelection = [&](EditorTool tool, const DistributedLoadValue& load)
    {
        activateTool(tool);
        if (!editor.state.selectionState.selection.HasBeams())
        {
            return;
        }

        ProjectDocument beforeDocument = editor.document;
        if (SupportLoadOperations::ApplyDistributedLoadToSelection(editor.document, editor.state, load))
        {
            editor.RecordDocumentChange(std::move(beforeDocument), true);
        }
        editor.state.activeTool = EditorTool::Select;
    };

    switch (request)
    {
    case EditorRequest::ActivateAddNodeTool:
        activateTool(EditorTool::AddNode);
        break;

    case EditorRequest::ActivateMoveNodeTool:
        editor.ActivateTransformTool(EditorTool::MoveNode);
        break;

    case EditorRequest::ActivateCopySelectionTool:
        editor.ActivateTransformTool(EditorTool::CopySelection);
        break;

    case EditorRequest::ActivateMirrorSelectionTool:
        editor.ActivateTransformTool(EditorTool::MirrorSelection);
        break;

    case EditorRequest::InvokeNodeRemoveAction:
        editor.state.ResetMoveSelection();
        if (editor.state.selectionState.selection.HasNodes())
        {
            editor.DeleteSelectedNodes();
            editor.state.activeTool = EditorTool::Select;
        }
        else
        {
            editor.state.lastSelectedTool = EditorTool::RemoveNode;
            editor.state.activeTool = EditorTool::RemoveNode;
        }
        break;

    case EditorRequest::ActivateAddBeamTool:
        activateTool(EditorTool::AddBeam);
        break;

    case EditorRequest::ActivateAddDimensionTool:
        activateTool(EditorTool::AddDimension);
        editor.CancelDimensionCreationOperation();
        editor.state.dimensionTool.dimensionLengthUnit = editor.state.dimensionTool.newDimensionLengthUnit;
        editor.state.dimensionTool.dimensionOffsetMode = editor.state.dimensionTool.newDimensionOffsetMode;
        editor.state.dimensionTool.dimensionCreationStep = DimensionToolState::DimensionCreationStep::AwaitFirstNode;
        break;

    case EditorRequest::ActivateMoveDimensionTool:
        activateTool(EditorTool::MoveDimension);
        editor.CancelDimensionMoveOperation();
        editor.state.dimensionTool.dimensionMoveStep = DimensionToolState::DimensionMoveStep::AwaitDimensionSelection;
        break;

    case EditorRequest::InvokeBeamRemoveAction:
        editor.state.ResetMoveSelection();
        if (editor.state.selectionState.selection.HasBeams())
        {
            editor.DeleteSelectedBeams();
            editor.state.activeTool = EditorTool::Select;
        }
        else
        {
            editor.state.lastSelectedTool = EditorTool::RemoveBeam;
            editor.state.activeTool = EditorTool::RemoveBeam;
        }
        break;

    case EditorRequest::SetSupportNone:
        applySupportOnSelection(EditorTool::SetSupportNone);
        break;

    case EditorRequest::SetSupportX:
        applySupportOnSelection(EditorTool::SetSupportX);
        break;

    case EditorRequest::SetSupportY:
        applySupportOnSelection(EditorTool::SetSupportY);
        break;

    case EditorRequest::SetSupportXY:
        applySupportOnSelection(EditorTool::SetSupportXY);
        break;

    case EditorRequest::SetSupportFixed:
        applySupportOnSelection(EditorTool::SetSupportFixed);
        break;

    case EditorRequest::ActivateAddPointLoadTool:
        applyPointLoadOnSelection(EditorTool::AddPointLoad, editor.state.loadTool.pendingNodalLoad);
        break;

    case EditorRequest::ActivateRemovePointLoadTool:
        applyPointLoadOnSelection(EditorTool::RemovePointLoad, MakeZeroNodalLoad());
        break;

    case EditorRequest::ActivateAddDistributedLoadTool:
        applyDistributedLoadOnSelection(
            EditorTool::AddDistributedLoad,
            editor.state.loadTool.pendingDistributedLoad);
        break;

    case EditorRequest::ActivateRemoveDistributedLoadTool:
        applyDistributedLoadOnSelection(EditorTool::RemoveDistributedLoad, MakeZeroDistributedLoad());
        break;

    case EditorRequest::DeleteSelection:
        editor.state.ResetMoveSelection();
        editor.DeleteSelection();
        break;

    case EditorRequest::None:
    default:
        break;
    }
}

EditorRequest EditorRequestController::GetActivationRequestForTool(EditorTool tool) const
{
    switch (tool)
    {
    case EditorTool::AddNode:
        return EditorRequest::ActivateAddNodeTool;
    case EditorTool::MoveNode:
        return EditorRequest::ActivateMoveNodeTool;
    case EditorTool::CopySelection:
        return EditorRequest::ActivateCopySelectionTool;
    case EditorTool::MirrorSelection:
        return EditorRequest::ActivateMirrorSelectionTool;
    case EditorTool::RemoveNode:
        return EditorRequest::InvokeNodeRemoveAction;
    case EditorTool::AddBeam:
        return EditorRequest::ActivateAddBeamTool;
    case EditorTool::RemoveBeam:
        return EditorRequest::InvokeBeamRemoveAction;
    case EditorTool::AddDimension:
        return EditorRequest::ActivateAddDimensionTool;
    case EditorTool::MoveDimension:
        return EditorRequest::ActivateMoveDimensionTool;
    case EditorTool::SetSupportNone:
        return EditorRequest::SetSupportNone;
    case EditorTool::SetSupportX:
        return EditorRequest::SetSupportX;
    case EditorTool::SetSupportY:
        return EditorRequest::SetSupportY;
    case EditorTool::SetSupportXY:
        return EditorRequest::SetSupportXY;
    case EditorTool::SetSupportFixed:
        return EditorRequest::SetSupportFixed;
    case EditorTool::AddPointLoad:
        return EditorRequest::ActivateAddPointLoadTool;
    case EditorTool::RemovePointLoad:
        return EditorRequest::ActivateRemovePointLoadTool;
    case EditorTool::AddDistributedLoad:
        return EditorRequest::ActivateAddDistributedLoadTool;
    case EditorTool::RemoveDistributedLoad:
        return EditorRequest::ActivateRemoveDistributedLoadTool;
    case EditorTool::Select:
    default:
        return EditorRequest::None;
    }
}

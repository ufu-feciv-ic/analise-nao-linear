#include "editor/tools/SupportToolController.h"

#include "editor/Editor.h"
#include "editor/ops/SupportLoadOperations.h"

#include <utility>

bool SupportToolController::Handles(EditorTool tool) const
{
    switch (tool)
    {
    case EditorTool::SetSupportNone:
    case EditorTool::SetSupportX:
    case EditorTool::SetSupportY:
    case EditorTool::SetSupportXY:
    case EditorTool::SetSupportFixed:
        return true;
    default:
        return false;
    }
}

void SupportToolController::Update(EditorContext& context)
{
    if (!Handles(context.state.activeTool))
    {
        return;
    }

    const SupportType supportType = SupportLoadOperations::GetSupportTypeForTool(context.state.activeTool);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (context.hoveredNode != nullptr)
        {
            ProjectDocument beforeDocument = context.document;
            const bool changed =
                SupportLoadOperations::ApplySupportTypeToNode(context.document, context.hoveredNode->id, supportType);
            context.state.hover.SetHoveredNode(context.hoveredNode->id);
            if (changed)
            {
                context.editor.RecordDocumentChange(std::move(beforeDocument), true);
            }
            context.state.activeTool = EditorTool::Select;
            return;
        }

        context.editor.BeginBoxSelection();
    }

    if (context.state.selectionState.isBoxSelecting && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        context.editor.UpdateBoxSelection();
    }

    if (context.state.selectionState.isBoxSelecting && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        context.editor.ApplySupportTypeInSelectionRectangle(supportType);
        if (context.state.selectionState.selection.HasNodes())
        {
            context.state.activeTool = EditorTool::Select;
        }
    }
}

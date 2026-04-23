#include "editor/tools/PointLoadToolController.h"

#include "editor/Editor.h"
#include "editor/ops/SupportLoadOperations.h"

#include <utility>

bool PointLoadToolController::Handles(EditorTool tool) const
{
    return tool == EditorTool::AddPointLoad || tool == EditorTool::RemovePointLoad;
}

void PointLoadToolController::Update(EditorContext& context)
{
    if (!Handles(context.state.activeTool))
    {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (context.hoveredNode != nullptr)
        {
            ProjectDocument beforeDocument = context.document;
            const bool changed =
                context.state.activeTool == EditorTool::AddPointLoad
                    ? SupportLoadOperations::ApplyPointLoadToNode(
                          context.document,
                          context.hoveredNode->id,
                          context.state.loadTool.pendingNodalLoad)
                    : SupportLoadOperations::ApplyPointLoadToNode(
                          context.document,
                          context.hoveredNode->id,
                          NodalLoad{0.0, 0.0, 0.0});

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
        if (context.state.activeTool == EditorTool::AddPointLoad)
        {
            context.editor.ApplyPointLoadInSelectionRectangle(context.state.loadTool.pendingNodalLoad);
            if (context.state.selectionState.selection.HasNodes())
            {
                context.state.activeTool = EditorTool::Select;
            }
        }
        else
        {
            context.editor.ClearPointLoadInSelectionRectangle();
            if (context.state.selectionState.selection.HasNodes())
            {
                context.state.activeTool = EditorTool::Select;
            }
        }
    }
}

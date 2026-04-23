#include "editor/tools/DistributedLoadToolController.h"

#include "editor/Editor.h"
#include "editor/ops/SupportLoadOperations.h"

#include <utility>

bool DistributedLoadToolController::Handles(EditorTool tool) const
{
    return tool == EditorTool::AddDistributedLoad || tool == EditorTool::RemoveDistributedLoad;
}

void DistributedLoadToolController::Update(EditorContext& context)
{
    if (!Handles(context.state.activeTool))
    {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (context.hoveredBeam != nullptr)
        {
            ProjectDocument beforeDocument = context.document;
            const bool changed =
                context.state.activeTool == EditorTool::AddDistributedLoad
                    ? SupportLoadOperations::ApplyDistributedLoadToBeam(
                          context.document,
                          context.hoveredBeam->id,
                          context.state.loadTool.pendingDistributedLoad)
                    : SupportLoadOperations::ApplyDistributedLoadToBeam(
                          context.document,
                          context.hoveredBeam->id,
                          DistributedLoadValue{});

            context.state.hover.SetHoveredBeam(context.hoveredBeam->id);
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
        if (context.state.activeTool == EditorTool::AddDistributedLoad)
        {
            context.editor.ApplyDistributedLoadInSelectionRectangle(context.state.loadTool.pendingDistributedLoad);
            if (context.state.selectionState.selection.HasBeams())
            {
                context.state.activeTool = EditorTool::Select;
            }
        }
        else
        {
            context.editor.ClearDistributedLoadInSelectionRectangle();
            if (context.state.selectionState.selection.HasBeams())
            {
                context.state.activeTool = EditorTool::Select;
            }
        }
    }
}

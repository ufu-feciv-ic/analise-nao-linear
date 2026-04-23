#include "editor/tools/SelectToolController.h"

#include "editor/Editor.h"

bool SelectToolController::Handles(EditorTool tool) const
{
    return tool == EditorTool::Select;
}

void SelectToolController::Update(EditorContext& context)
{
    context.editor.HandleSelectionBox(context.selectionMode);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        context.editor.ApplySelectionClick(
            context.hoveredNode,
            context.hoveredBeam,
            context.hoveredDimension,
            context.selectionMode);
    }
}

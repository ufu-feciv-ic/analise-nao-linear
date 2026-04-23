#include "editor/tools/DeleteToolController.h"

#include "editor/Editor.h"

bool DeleteToolController::Handles(EditorTool tool) const
{
    return tool == EditorTool::RemoveNode || tool == EditorTool::RemoveBeam;
}

void DeleteToolController::Update(EditorContext& context)
{
    if (!Handles(context.state.activeTool))
    {
        return;
    }

    if (context.state.activeTool == EditorTool::RemoveNode)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (context.hoveredNode != nullptr)
            {
                if (context.editor.DeleteNodeById(context.hoveredNode->id))
                {
                    context.state.selectionState.isBoxSelecting = false;
                    context.state.activeTool = EditorTool::Select;
                }
            }
            else
            {
                context.editor.BeginBoxSelection();
            }
        }

        if (context.state.selectionState.isBoxSelecting && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            context.editor.UpdateBoxSelection();
        }

        if (context.state.selectionState.isBoxSelecting && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (context.editor.DeleteNodesInSelectionRectangle())
            {
                context.state.activeTool = EditorTool::Select;
            }
        }

        return;
    }

    if (context.state.activeTool == EditorTool::RemoveBeam)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (context.hoveredBeam != nullptr)
            {
                if (context.editor.DeleteBeamById(context.hoveredBeam->id))
                {
                    context.state.selectionState.isBoxSelecting = false;
                    context.state.activeTool = EditorTool::Select;
                }
            }
            else
            {
                context.editor.BeginBoxSelection();
            }
        }

        if (context.state.selectionState.isBoxSelecting && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            context.editor.UpdateBoxSelection();
        }

        if (context.state.selectionState.isBoxSelecting && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (context.editor.DeleteBeamsInSelectionRectangle())
            {
                context.state.activeTool = EditorTool::Select;
            }
        }
    }
}

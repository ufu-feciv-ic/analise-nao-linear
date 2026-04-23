#include "editor/tools/TransformToolController.h"

#include "editor/Editor.h"

bool TransformToolController::Handles(EditorTool tool) const
{
    return tool == EditorTool::MoveNode ||
           tool == EditorTool::CopySelection ||
           tool == EditorTool::MirrorSelection;
}

void TransformToolController::Update(EditorContext& context)
{
    const EditorTool tool = context.state.activeTool;
    if (!Handles(tool))
    {
        return;
    }

    const bool allowSelectedNodeSnapForTarget =
        tool == EditorTool::CopySelection ||
        tool == EditorTool::MirrorSelection;

    if (context.state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitSelectionConfirm)
    {
        context.editor.HandleSelectionBox(context.selectionMode);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            context.editor.ApplySelectionClick(context.hoveredNode, nullptr, nullptr, context.selectionMode);
        }

        if (context.editor.HasTransformSelection() && context.editor.IsMoveSelectionConfirmPressed())
        {
            context.editor.BeginMoveSelection();
        }

        return;
    }

    if (context.state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitBasePoint)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Vector2 guideTargetPoint{0.0f, 0.0f};
            if (context.hoveredNode == nullptr && context.editor.TryResolveTransformTargetFromGuides(guideTargetPoint))
            {
                if (!context.state.beamTool.isBeamDistanceInputOpen)
                {
                    context.state.transformTool.moveBasePointWorld = Point2D{
                        static_cast<double>(guideTargetPoint.x),
                        static_cast<double>(guideTargetPoint.y)};
                    context.state.transformTool.moveSelectionStep = TransformToolState::MoveSelectionStep::AwaitTargetPoint;
                }
            }
            else
            {
                context.state.transformTool.moveBasePointWorld =
                    context.editor.GetCurrentTransformTargetWorldPosition(context.hoveredNode, true);
                context.state.transformTool.moveSelectionStep = TransformToolState::MoveSelectionStep::AwaitTargetPoint;
            }
        }

        return;
    }

    if (context.state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitTargetPoint)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Vector2 guideTargetPoint{0.0f, 0.0f};
            if (context.hoveredNode == nullptr && context.editor.TryResolveTransformTargetFromGuides(guideTargetPoint))
            {
                if (!context.state.beamTool.isBeamDistanceInputOpen)
                {
                    context.editor.ApplyTransformTool(
                        tool,
                        Point2D{
                            static_cast<double>(guideTargetPoint.x),
                            static_cast<double>(guideTargetPoint.y)});
                }
            }
            else
            {
                context.editor.ApplyTransformTool(
                    tool,
                    context.editor.GetCurrentTransformTargetWorldPosition(
                        context.hoveredNode,
                        allowSelectedNodeSnapForTarget));
            }
        }

        return;
    }

    if (context.editor.HasTransformSelection())
    {
        context.editor.BeginMoveSelection();
    }
    else
    {
        context.state.transformTool.moveSelectionStep = TransformToolState::MoveSelectionStep::AwaitSelectionConfirm;
    }
}


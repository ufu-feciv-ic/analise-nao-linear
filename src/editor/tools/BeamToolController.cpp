#include "editor/tools/BeamToolController.h"

#include "editor/Editor.h"

bool BeamToolController::Handles(EditorTool tool) const
{
    return tool == EditorTool::AddBeam;
}

void BeamToolController::Update(EditorContext& context)
{
    if (!Handles(context.state.activeTool))
    {
        return;
    }

    const Vector2 targetPosition = context.editor.GetCurrentPlacementWorldPosition();

    if (!context.state.beamTool.isCreatingBeam)
    {
        context.state.beamTool.beamPreviewPointWorld = targetPosition;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Node* startNode = context.hoveredNode;
            if (startNode == nullptr)
            {
                startNode = context.editor.ResolveNodePlacementFromGuides(targetPosition);
            }

            if (startNode == nullptr && !context.state.beamTool.isBeamDistanceInputOpen)
            {
                startNode = context.editor.GetOrCreateNodeAtWorldPosition(targetPosition);
            }

            if (startNode != nullptr)
            {
                context.editor.BeginBeamCreationFromNode(startNode);
            }
        }

        return;
    }

    Node* startNode = context.document.FindNodeById(context.state.beamTool.beamStartNodeId);
    if (startNode == nullptr)
    {
        context.state.beamTool.isCreatingBeam = false;
        context.state.beamTool.beamStartNodeId = -1;
        return;
    }

    if (!context.state.beamTool.isBeamDistanceInputOpen && IsKeyPressed(KEY_SPACE))
    {
        context.state.beamTool.isCreatingBeam = false;
        context.state.beamTool.beamStartNodeId = -1;
        context.state.beamTool.beamPreviewPointWorld = targetPosition;
        if (context.hoveredNode != nullptr)
        {
            context.state.hover.SetHoveredNode(context.hoveredNode->id);
        }
        else
        {
            context.state.hover.ClearEntity();
        }
        return;
    }

    if (context.hoveredNode != nullptr && context.hoveredNode->id != startNode->id)
    {
        context.state.beamTool.beamPreviewPointWorld = Vector2{
            static_cast<float>(context.hoveredNode->position.x),
            static_cast<float>(context.hoveredNode->position.y)};
    }
    else
    {
        context.state.beamTool.beamPreviewPointWorld = targetPosition;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    Node* endNode = context.hoveredNode;
    bool createdEndNodeForBeam = false;
    if (endNode == nullptr || endNode->id == startNode->id)
    {
        const int nextNodeIdBefore = context.document.nextNodeId;
        endNode = context.editor.ResolveNodePlacementFromGuides(targetPosition);
        if (endNode == nullptr && !context.state.beamTool.isBeamDistanceInputOpen)
        {
            endNode = context.editor.GetOrCreateNodeAtWorldPosition(targetPosition);
        }
        createdEndNodeForBeam =
            endNode != nullptr &&
            context.document.nextNodeId != nextNodeIdBefore &&
            endNode->id == nextNodeIdBefore;
    }

    if (!context.editor.CompleteBeamCreationToNode(endNode) && createdEndNodeForBeam)
    {
        context.editor.DeleteNodeById(endNode->id);
    }
}


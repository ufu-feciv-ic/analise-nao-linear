#include "editor/tools/AddNodeToolController.h"

#include "editor/Editor.h"

#include <utility>

bool AddNodeToolController::Handles(EditorTool tool) const
{
    return tool == EditorTool::AddNode;
}

void AddNodeToolController::Update(EditorContext& context)
{
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    const Vector2 targetPosition = context.editor.GetCurrentPlacementWorldPosition();
    ProjectDocument beforeDocument = context.document;
    Node* createdNode = context.editor.ResolveAddNodePlacement(context.hoveredNode, targetPosition);
    const bool changed =
        context.document.nextNodeId != beforeDocument.nextNodeId ||
        context.document.nextBeamId != beforeDocument.nextBeamId ||
        context.document.nextDistributedLoadId != beforeDocument.nextDistributedLoadId ||
        context.document.nodes.size() != beforeDocument.nodes.size() ||
        context.document.beams.size() != beforeDocument.beams.size() ||
        context.document.distributedLoads.size() != beforeDocument.distributedLoads.size();
    if (changed)
    {
        context.editor.RecordDocumentChange(std::move(beforeDocument), true);
    }
    if (createdNode != nullptr)
    {
        context.state.hover.SetHoveredNode(createdNode->id);
    }
}

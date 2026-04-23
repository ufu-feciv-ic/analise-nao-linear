#include "editor/ops/NodeOperations.h"

#include <cmath>

namespace NodeOperations
{
bool IsCoincidentWithWorldPosition(
    const Node& node,
    Vector2 worldPosition,
    double tolerance)
{
    const double dx = node.position.x - static_cast<double>(worldPosition.x);
    const double dy = node.position.y - static_cast<double>(worldPosition.y);
    return std::abs(dx) <= tolerance && std::abs(dy) <= tolerance;
}

Node* FindNodeByExactWorldPosition(
    ProjectDocument& document,
    Vector2 worldPosition,
    double tolerance)
{
    for (Node& node : document.nodes)
    {
        if (IsCoincidentWithWorldPosition(node, worldPosition, tolerance))
        {
            return &node;
        }
    }

    return nullptr;
}

Node* CreateNodeAtWorldPosition(ProjectDocument& document, Vector2 worldPosition)
{
    Node node;
    node.id = document.nextNodeId++;
    node.position.x = static_cast<double>(worldPosition.x);
    node.position.y = static_cast<double>(worldPosition.y);
    return document.AppendNode(node);
}

Node* GetOrCreateNodeAtWorldPosition(
    ProjectDocument& document,
    Vector2 worldPosition,
    double tolerance)
{
    if (Node* existingNode = FindNodeByExactWorldPosition(document, worldPosition, tolerance))
    {
        return existingNode;
    }

    return CreateNodeAtWorldPosition(document, worldPosition);
}
}

#pragma once

#include "model/ProjectDocument.h"
#include "raylib.h"

namespace NodeOperations
{
constexpr double kDefaultNodeCoincidenceTolerance = 0.00001;

bool IsCoincidentWithWorldPosition(
    const Node& node,
    Vector2 worldPosition,
    double tolerance = kDefaultNodeCoincidenceTolerance);

Node* FindNodeByExactWorldPosition(
    ProjectDocument& document,
    Vector2 worldPosition,
    double tolerance = kDefaultNodeCoincidenceTolerance);

Node* CreateNodeAtWorldPosition(ProjectDocument& document, Vector2 worldPosition);

Node* GetOrCreateNodeAtWorldPosition(
    ProjectDocument& document,
    Vector2 worldPosition,
    double tolerance = kDefaultNodeCoincidenceTolerance);
}

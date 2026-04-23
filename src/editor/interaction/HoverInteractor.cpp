#include "editor/interaction/HoverInteractor.h"

#include "editor/interaction/HitTester.h"

namespace HoverInteractor
{
HoveredEntities ResolveHoveredEntities(
    ProjectDocument& document,
    const Camera2D& camera,
    Vector2 mouseWorldPosition,
    Vector2 mouseScreenPosition)
{
    HoveredEntities hovered{};

    hovered.node = HitTester::FindNodeAtWorldPosition(document, camera, mouseWorldPosition);
    hovered.beam = HitTester::FindBeamAtWorldPosition(document, camera, mouseWorldPosition);
    hovered.dimension = HitTester::FindDimensionAtScreenPosition(document, camera, mouseScreenPosition);

    if (hovered.node != nullptr || hovered.beam != nullptr)
    {
        hovered.dimension = nullptr;
    }

    if (hovered.node != nullptr)
    {
        hovered.entity.type = EntityType::Node;
        hovered.entity.id = hovered.node->id;
    }
    else if (hovered.beam != nullptr)
    {
        hovered.entity.type = EntityType::Beam;
        hovered.entity.id = hovered.beam->id;
    }
    else if (hovered.dimension != nullptr)
    {
        hovered.entity.type = EntityType::Dimension;
        hovered.entity.id = hovered.dimension->id;
    }

    return hovered;
}
}


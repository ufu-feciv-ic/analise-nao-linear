#pragma once

#include "editor/EntityRef.h"

struct HoverState
{
    EntityRef entity;
    void ClearEntity()
    {
        entity = EntityRef{};
    }

    void SetHoveredNode(int nodeId)
    {
        entity = EntityRef{EntityType::Node, nodeId};
    }

    void SetHoveredBeam(int beamId)
    {
        entity = EntityRef{EntityType::Beam, beamId};
    }

    void SetHoveredDimension(int dimensionId)
    {
        entity = EntityRef{EntityType::Dimension, dimensionId};
    }

    int HoveredNodeId() const
    {
        return entity.type == EntityType::Node ? entity.id : -1;
    }

    int HoveredBeamId() const
    {
        return entity.type == EntityType::Beam ? entity.id : -1;
    }

    int HoveredDimensionId() const
    {
        return entity.type == EntityType::Dimension ? entity.id : -1;
    }

    int beamGuideHoverNodeId = -1;
    double beamGuideHoverStartTime = 0.0;
    bool beamGuideCreatedOnCurrentHover = false;
};

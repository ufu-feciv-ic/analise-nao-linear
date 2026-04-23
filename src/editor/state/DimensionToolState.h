#pragma once

#include <vector>

#include "model/Dimension.h"
#include "raylib.h"

struct DimensionToolState
{
    enum class DimensionCreationStep
    {
        None,
        AwaitFirstNode,
        AwaitSecondNode,
        AwaitOffsetPoint,
        AwaitChainedNode
    };

    enum class DimensionMoveStep
    {
        None,
        AwaitDimensionSelection,
        AwaitOffsetPoint
    };

    LengthUnit newDimensionLengthUnit = LengthUnit::Meter;
    DimensionCreationStep dimensionCreationStep = DimensionCreationStep::None;
    int dimensionStartNodeId = -1;
    int dimensionEndNodeId = -1;
    int dimensionChainReferenceNodeId = -1;
    LengthUnit dimensionLengthUnit = LengthUnit::Meter;
    DimensionOffsetMode newDimensionOffsetMode = DimensionOffsetMode::WorldUnits;
    DimensionType dimensionType = DimensionType::Aligned;
    DimensionOffsetMode dimensionOffsetMode = DimensionOffsetMode::WorldUnits;
    double dimensionOffsetPixels = 32.0;
    double dimensionOffsetWorld = 0.32;
    Vector2 dimensionPreviewPointWorld{0.0f, 0.0f};
    Vector2 dimensionPreviewScreenPosition{0.0f, 0.0f};
    DimensionMoveStep dimensionMoveStep = DimensionMoveStep::None;
    int movingDimensionId = -1;
    std::vector<int> movingDimensionIds;
};

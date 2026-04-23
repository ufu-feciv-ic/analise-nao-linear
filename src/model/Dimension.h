#pragma once

#include "model/DisplayUnits.h"

enum class DimensionType
{
    Aligned,
    Horizontal,
    Vertical
};

enum class DimensionOffsetMode
{
    ScreenPixels,
    WorldUnits
};

class Dimension
{
public:
    int id = -1;
    int startNodeId = -1;
    int endNodeId = -1;
    DimensionType type = DimensionType::Aligned;
    LengthUnit lengthUnit = LengthUnit::Meter;
    DimensionOffsetMode offsetMode = DimensionOffsetMode::ScreenPixels;
    double offsetPixels = 32.0;
    double offsetWorld = 0.0;

public:
    Dimension() = default;

    Dimension(
        int newId,
        int newStartNodeId,
        int newEndNodeId,
        DimensionType newType,
        LengthUnit newLengthUnit,
        DimensionOffsetMode newOffsetMode,
        double newOffsetPixels,
        double newOffsetWorld)
        : id(newId),
          startNodeId(newStartNodeId),
          endNodeId(newEndNodeId),
          type(newType),
          lengthUnit(newLengthUnit),
          offsetMode(newOffsetMode),
          offsetPixels(newOffsetPixels),
          offsetWorld(newOffsetWorld)
    {
    }
};

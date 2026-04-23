#pragma once

#include <vector>

#include "model/Node.h"
#include "raylib.h"

struct BeamToolState
{
    struct BeamGuide
    {
        Vector2 position{0.0f, 0.0f};
        double creationTime = 0.0;
        bool activeX = false;
        bool activeY = false;
    };

    enum class DistanceInputMode
    {
        GuideAxis,
        BeamAlongSegment
    };

    bool isCreatingBeam = false;
    int beamStartNodeId = -1;
    Vector2 beamPreviewPointWorld{0.0f, 0.0f};
    std::vector<BeamGuide> beamGuides;
    int activeBeamGuideXIndex = -1;
    int activeBeamGuideYIndex = -1;

    bool isBeamDistanceInputOpen = false;
    DistanceInputMode beamDistanceMode = DistanceInputMode::GuideAxis;
    bool beamDistanceLocksX = false;
    Vector2 beamDistanceScreenPosition{0.0f, 0.0f};
    Point2D beamDistanceClickWorld{0.0, 0.0};
    Point2D beamDistanceGuideWorld{0.0, 0.0};
    Point2D beamDistanceSegmentStartWorld{0.0, 0.0};
    Point2D beamDistanceSegmentEndWorld{0.0, 0.0};
    Point2D beamDistanceCreatedNodeWorld{0.0, 0.0};
};

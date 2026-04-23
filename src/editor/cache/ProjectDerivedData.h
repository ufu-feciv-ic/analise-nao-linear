#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "model/ProjectDocument.h"

enum NodeOccupiedDirectionFlags
{
    NodeOccupiedDirectionNone = 0,
    NodeOccupiedDirectionUp = 1 << 0,
    NodeOccupiedDirectionDown = 1 << 1,
    NodeOccupiedDirectionLeft = 1 << 2,
    NodeOccupiedDirectionRight = 1 << 3
};

class ProjectDerivedData
{
public:
    struct NodeConnectivity
    {
        Point2D averageConnectedBeamDirection;
        Point2D averageConnectedBeamAxis;
        bool hasAverageConnectedBeamDirection = false;
        bool hasAverageConnectedBeamAxis = false;
        bool hasPreferredConnectedBeamDirection = false;
        double preferredConnectedBeamDirectionStrength = 0.0;
        double averageConnectedBeamAxisStrength = 0.0;
        unsigned char occupiedDirectionMask = NodeOccupiedDirectionNone;
    };

    struct BeamGroup
    {
        struct ShadowSegment
        {
            Point2D start;
            Point2D end;
        };

        std::vector<int> beamIds;
        std::vector<int> nodeIds;
        std::vector<Point2D> shadowNodePositions;
        std::vector<ShadowSegment> shadowSegments;
        double shadowBaseY = 0.0;
    };

    std::unordered_map<int, NodeConnectivity> nodeConnectivityById;
    std::vector<BeamGroup> beamGroups;
    std::unordered_map<int, std::size_t> beamGroupIndexByBeamId;

public:
    void Clear();
    void Rebuild(const ProjectDocument& document);
    const NodeConnectivity* FindNodeConnectivityById(int nodeId) const;
    const BeamGroup* FindBeamGroupByBeamId(int beamId) const;
    double GetShadowBaseYForBeam(const ProjectDocument& document, int beamId) const;
};

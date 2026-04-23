#include "editor/cache/ProjectDerivedData.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

void ProjectDerivedData::Clear()
{
    nodeConnectivityById.clear();
    beamGroups.clear();
    beamGroupIndexByBeamId.clear();
}

void ProjectDerivedData::Rebuild(const ProjectDocument& document)
{
    Clear();

    struct DirectionAccumulator
    {
        double sumX = 0.0;
        double sumY = 0.0;
        double axisSumX = 0.0;
        double axisSumY = 0.0;
        double fallbackDirectionX = 0.0;
        double fallbackDirectionY = 0.0;
        bool hasFallbackDirection = false;
        int connectedBeamCount = 0;
    };

    constexpr double preferredDirectionStrengthThreshold = 0.55;
    std::vector<DirectionAccumulator> accumulators(document.nodes.size());
    nodeConnectivityById.reserve(document.nodes.size());

    for (const Node& node : document.nodes)
    {
        nodeConnectivityById.emplace(node.id, NodeConnectivity{});
    }

    auto markOccupiedDirection = [this](int nodeId, double directionX, double directionY)
    {
        auto it = nodeConnectivityById.find(nodeId);
        if (it == nodeConnectivityById.end())
        {
            return;
        }

        NodeConnectivity& connectivity = it->second;
        const double absX = std::abs(directionX);
        const double absY = std::abs(directionY);
        if (absX <= 1.0e-9 && absY <= 1.0e-9)
        {
            return;
        }

        const double angleDegrees = std::atan2(absY, absX) * 180.0 / 3.14159265358979323846;
        const bool occupiesHorizontal = angleDegrees <= 30.0 || (angleDegrees > 30.0 && angleDegrees < 60.0);
        const bool occupiesVertical = angleDegrees >= 60.0 || (angleDegrees > 30.0 && angleDegrees < 60.0);

        if (occupiesHorizontal)
        {
            if (directionX > 1.0e-9)
            {
                connectivity.occupiedDirectionMask |= NodeOccupiedDirectionRight;
            }
            else if (directionX < -1.0e-9)
            {
                connectivity.occupiedDirectionMask |= NodeOccupiedDirectionLeft;
            }
        }

        if (occupiesVertical)
        {
            if (directionY > 1.0e-9)
            {
                connectivity.occupiedDirectionMask |= NodeOccupiedDirectionDown;
            }
            else if (directionY < -1.0e-9)
            {
                connectivity.occupiedDirectionMask |= NodeOccupiedDirectionUp;
            }
        }
    };

    for (const Beam& beam : document.beams)
    {
        const Node* startNode = document.FindNodeById(beam.startNodeId);
        const Node* endNode = document.FindNodeById(beam.endNodeId);
        if (startNode == nullptr || endNode == nullptr)
        {
            continue;
        }

        const std::size_t startIndex = static_cast<std::size_t>(startNode - document.nodes.data());
        const std::size_t endIndex = static_cast<std::size_t>(endNode - document.nodes.data());
        const double dx = endNode->position.x - startNode->position.x;
        const double dy = endNode->position.y - startNode->position.y;
        const double length = std::sqrt(dx * dx + dy * dy);
        if (length <= 1.0e-9)
        {
            continue;
        }

        DirectionAccumulator& startAccumulator = accumulators[startIndex];
        startAccumulator.sumX += dx / length;
        startAccumulator.sumY += dy / length;
        double canonicalAxisX = dx / length;
        double canonicalAxisY = dy / length;
        if (canonicalAxisX < -1.0e-9 ||
            (std::abs(canonicalAxisX) <= 1.0e-9 && canonicalAxisY < 0.0))
        {
            canonicalAxisX = -canonicalAxisX;
            canonicalAxisY = -canonicalAxisY;
        }
        startAccumulator.axisSumX += canonicalAxisX;
        startAccumulator.axisSumY += canonicalAxisY;
        startAccumulator.connectedBeamCount += 1;
        if (!startAccumulator.hasFallbackDirection)
        {
            startAccumulator.hasFallbackDirection = true;
            startAccumulator.fallbackDirectionX = dx / length;
            startAccumulator.fallbackDirectionY = dy / length;
        }
        markOccupiedDirection(beam.startNodeId, dx, dy);

        DirectionAccumulator& endAccumulator = accumulators[endIndex];
        endAccumulator.sumX -= dx / length;
        endAccumulator.sumY -= dy / length;
        endAccumulator.axisSumX += canonicalAxisX;
        endAccumulator.axisSumY += canonicalAxisY;
        endAccumulator.connectedBeamCount += 1;
        if (!endAccumulator.hasFallbackDirection)
        {
            endAccumulator.hasFallbackDirection = true;
            endAccumulator.fallbackDirectionX = -dx / length;
            endAccumulator.fallbackDirectionY = -dy / length;
        }
        markOccupiedDirection(beam.endNodeId, -dx, -dy);
    }

    for (std::size_t i = 0; i < document.nodes.size(); ++i)
    {
        const Node& node = document.nodes[i];
        auto connectivityIt = nodeConnectivityById.find(node.id);
        if (connectivityIt == nodeConnectivityById.end())
        {
            continue;
        }

        NodeConnectivity& connectivity = connectivityIt->second;
        const DirectionAccumulator& accumulator = accumulators[i];
        const double magnitude =
            std::sqrt(accumulator.sumX * accumulator.sumX + accumulator.sumY * accumulator.sumY);
        const double axisMagnitude =
            std::sqrt(accumulator.axisSumX * accumulator.axisSumX + accumulator.axisSumY * accumulator.axisSumY);
        const double strength =
            accumulator.connectedBeamCount > 0
                ? magnitude / static_cast<double>(accumulator.connectedBeamCount)
                : 0.0;
        const double axisStrength =
            accumulator.connectedBeamCount > 0
                ? axisMagnitude / static_cast<double>(accumulator.connectedBeamCount)
                : 0.0;
        connectivity.preferredConnectedBeamDirectionStrength = strength;
        connectivity.averageConnectedBeamAxisStrength = axisStrength;

        if (magnitude > 1.0e-9)
        {
            connectivity.averageConnectedBeamDirection = Point2D(
                accumulator.sumX / magnitude,
                accumulator.sumY / magnitude);
            connectivity.hasAverageConnectedBeamDirection = true;
            connectivity.hasPreferredConnectedBeamDirection = strength >= preferredDirectionStrengthThreshold;
        }
        else if (accumulator.hasFallbackDirection)
        {
            connectivity.averageConnectedBeamDirection = Point2D(
                accumulator.fallbackDirectionX,
                accumulator.fallbackDirectionY);
            connectivity.hasAverageConnectedBeamDirection = true;
            connectivity.hasPreferredConnectedBeamDirection = false;
        }

        if (axisMagnitude > 1.0e-9)
        {
            connectivity.averageConnectedBeamAxis = Point2D(
                accumulator.axisSumX / axisMagnitude,
                accumulator.axisSumY / axisMagnitude);
            connectivity.hasAverageConnectedBeamAxis = true;
        }
    }

    if (document.beams.empty())
    {
        return;
    }

    std::unordered_map<int, std::vector<int>> beamIdsByNodeId;
    beamIdsByNodeId.reserve(document.nodes.size());

    for (const Beam& beam : document.beams)
    {
        beamIdsByNodeId[beam.startNodeId].push_back(beam.id);
        beamIdsByNodeId[beam.endNodeId].push_back(beam.id);
    }

    std::unordered_set<int> visitedBeamIds;
    visitedBeamIds.reserve(document.beams.size());

    for (const Beam& seedBeam : document.beams)
    {
        if (!visitedBeamIds.insert(seedBeam.id).second)
        {
            continue;
        }

        BeamGroup group;
        std::unordered_set<int> groupNodeIds;
        groupNodeIds.reserve(8);
        std::vector<int> pendingBeamIds;
        pendingBeamIds.push_back(seedBeam.id);
        double lowestY = std::numeric_limits<double>::lowest();

        while (!pendingBeamIds.empty())
        {
            const int beamId = pendingBeamIds.back();
            pendingBeamIds.pop_back();

            const Beam* beam = document.FindBeamById(beamId);
            if (beam == nullptr)
            {
                continue;
            }

            group.beamIds.push_back(beam->id);

            const int nodeIds[] = {beam->startNodeId, beam->endNodeId};
            for (int nodeId : nodeIds)
            {
                if (groupNodeIds.insert(nodeId).second)
                {
                    group.nodeIds.push_back(nodeId);

                    const Node* node = document.FindNodeById(nodeId);
                    if (node != nullptr)
                    {
                        lowestY = std::max(lowestY, node->position.y);
                    }
                }

                const auto adjacentIt = beamIdsByNodeId.find(nodeId);
                if (adjacentIt == beamIdsByNodeId.end())
                {
                    continue;
                }

                for (int adjacentBeamId : adjacentIt->second)
                {
                    if (visitedBeamIds.insert(adjacentBeamId).second)
                    {
                        pendingBeamIds.push_back(adjacentBeamId);
                    }
                }
            }
        }

        if (lowestY == std::numeric_limits<double>::lowest())
        {
            lowestY = 0.0;
        }

        group.shadowBaseY = lowestY;
        group.shadowNodePositions.reserve(group.nodeIds.size());
        group.shadowSegments.reserve(group.beamIds.size());

        std::unordered_map<int, Point2D> shadowPositionByNodeId;
        shadowPositionByNodeId.reserve(group.nodeIds.size());
        for (int nodeId : group.nodeIds)
        {
            const Node* node = document.FindNodeById(nodeId);
            if (node == nullptr)
            {
                continue;
            }

            const double dy = node->position.y - group.shadowBaseY;
            const Point2D shadowPosition{
                node->position.x + dy * 0.3,
                node->position.y - dy * 0.8};
            group.shadowNodePositions.push_back(shadowPosition);
            shadowPositionByNodeId.emplace(nodeId, shadowPosition);
        }

        for (int beamId : group.beamIds)
        {
            const Beam* beam = document.FindBeamById(beamId);
            if (beam == nullptr)
            {
                continue;
            }

            const auto startIt = shadowPositionByNodeId.find(beam->startNodeId);
            const auto endIt = shadowPositionByNodeId.find(beam->endNodeId);
            if (startIt == shadowPositionByNodeId.end() || endIt == shadowPositionByNodeId.end())
            {
                continue;
            }

            group.shadowSegments.push_back(ProjectDerivedData::BeamGroup::ShadowSegment{
                startIt->second,
                endIt->second});
        }

        const std::size_t groupIndex = beamGroups.size();
        for (int beamId : group.beamIds)
        {
            beamGroupIndexByBeamId[beamId] = groupIndex;
        }

        beamGroups.push_back(std::move(group));
    }
}

const ProjectDerivedData::NodeConnectivity* ProjectDerivedData::FindNodeConnectivityById(int nodeId) const
{
    const auto it = nodeConnectivityById.find(nodeId);
    if (it == nodeConnectivityById.end())
    {
        return nullptr;
    }

    return &it->second;
}

const ProjectDerivedData::BeamGroup* ProjectDerivedData::FindBeamGroupByBeamId(int beamId) const
{
    const auto it = beamGroupIndexByBeamId.find(beamId);
    if (it == beamGroupIndexByBeamId.end())
    {
        return nullptr;
    }

    return &beamGroups.at(it->second);
}

double ProjectDerivedData::GetShadowBaseYForBeam(const ProjectDocument& document, int beamId) const
{
    const BeamGroup* group = FindBeamGroupByBeamId(beamId);
    if (group != nullptr)
    {
        return group->shadowBaseY;
    }

    const Beam* beam = document.FindBeamById(beamId);
    if (beam == nullptr)
    {
        return 0.0;
    }

    const Node* startNode = document.FindNodeById(beam->startNodeId);
    const Node* endNode = document.FindNodeById(beam->endNodeId);
    if (startNode == nullptr || endNode == nullptr)
    {
        return 0.0;
    }

    return std::max(startNode->position.y, endNode->position.y);
}

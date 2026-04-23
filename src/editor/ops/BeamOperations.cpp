#include "editor/ops/BeamOperations.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "editor/ops/DeleteOperations.h"

namespace
{
    constexpr double geometryTolerance = 0.00001;

    Vector2 Subtract(Vector2 a, Vector2 b)
    {
        return Vector2{a.x - b.x, a.y - b.y};
    }

    double DotProduct(Vector2 a, Vector2 b)
    {
        return static_cast<double>(a.x) * static_cast<double>(b.x) +
               static_cast<double>(a.y) * static_cast<double>(b.y);
    }

    double CrossProduct(Vector2 a, Vector2 b)
    {
        return static_cast<double>(a.x) * static_cast<double>(b.y) -
               static_cast<double>(a.y) * static_cast<double>(b.x);
    }

    double LengthSquared(Vector2 vector)
    {
        return DotProduct(vector, vector);
    }

    double ProjectPointParameter(Vector2 point, Vector2 start, Vector2 end)
    {
        const Vector2 segment = Subtract(end, start);
        const double segmentLengthSquared = LengthSquared(segment);
        if (segmentLengthSquared <= geometryTolerance)
        {
            return 0.0;
        }

        return DotProduct(Subtract(point, start), segment) / segmentLengthSquared;
    }

    bool IsPointOnSegment(Vector2 point, Vector2 start, Vector2 end, double tolerance = geometryTolerance)
    {
        const Vector2 segment = Subtract(end, start);
        const Vector2 toPoint = Subtract(point, start);
        const double segmentLengthSquared = LengthSquared(segment);
        if (segmentLengthSquared <= tolerance)
        {
            return LengthSquared(toPoint) <= tolerance;
        }

        const double cross = std::abs(CrossProduct(segment, toPoint));
        const double length = sqrt(segmentLengthSquared);
        if (cross > tolerance * length)
        {
            return false;
        }

        const double projection = DotProduct(toPoint, segment);
        return projection >= -tolerance &&
               projection <= segmentLengthSquared + tolerance;
    }

    bool IsPointInsideSegment(Vector2 point, Vector2 start, Vector2 end, double tolerance = geometryTolerance)
    {
        if (!IsPointOnSegment(point, start, end, tolerance))
        {
            return false;
        }

        const double parameter = ProjectPointParameter(point, start, end);
        return parameter > tolerance && parameter < 1.0 - tolerance;
    }

    std::uint64_t MakeBeamSegmentKey(int startNodeId, int endNodeId)
    {
        if (startNodeId > endNodeId)
        {
            std::swap(startNodeId, endNodeId);
        }

        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(startNodeId)) << 32) |
               static_cast<std::uint32_t>(endNodeId);
    }
}

namespace BeamOperations
{
bool ResolveBeamDirection(const ProjectDocument& document, int& startNodeId, int& endNodeId)
{
    const Node* startNode = document.FindNodeById(startNodeId);
    const Node* endNode = document.FindNodeById(endNodeId);
    if (startNode == nullptr || endNode == nullptr || startNodeId == endNodeId)
    {
        return false;
    }

    const double dx = endNode->position.x - startNode->position.x;
    constexpr double tolerance = 1.0e-9;

    const bool isVertical = std::abs(dx) < tolerance;
    const bool shouldSwap =
        isVertical
            ? (startNode->position.y < endNode->position.y)
            : (startNode->position.x > endNode->position.x);

    if (shouldSwap)
    {
        std::swap(startNodeId, endNodeId);
    }

    return true;
}

std::vector<int> CollectNodeIdsOnSegment(const ProjectDocument& document, int startNodeId, int endNodeId)
{
    const Node* startNode = document.FindNodeById(startNodeId);
    const Node* endNode = document.FindNodeById(endNodeId);
    if (startNode == nullptr || endNode == nullptr)
    {
        return {};
    }

    const Vector2 startPosition = Vector2{
        static_cast<float>(startNode->position.x),
        static_cast<float>(startNode->position.y)};
    const Vector2 endPosition = Vector2{
        static_cast<float>(endNode->position.x),
        static_cast<float>(endNode->position.y)};

    std::vector<std::pair<double, int>> nodesWithParameters;
    nodesWithParameters.reserve(document.nodes.size());

    for (const Node& node : document.nodes)
    {
        const Vector2 nodePosition = Vector2{
            static_cast<float>(node.position.x),
            static_cast<float>(node.position.y)};
        if (!IsPointOnSegment(nodePosition, startPosition, endPosition))
        {
            continue;
        }

        nodesWithParameters.emplace_back(
            ProjectPointParameter(nodePosition, startPosition, endPosition),
            node.id);
    }

    std::sort(
        nodesWithParameters.begin(),
        nodesWithParameters.end(),
        [](const std::pair<double, int>& left, const std::pair<double, int>& right)
        {
            if (std::abs(left.first - right.first) <= geometryTolerance)
            {
                return left.second < right.second;
            }

            return left.first < right.first;
        });

    std::vector<int> nodeIds;
    nodeIds.reserve(nodesWithParameters.size());
    for (const std::pair<double, int>& nodeWithParameter : nodesWithParameters)
    {
        nodeIds.push_back(nodeWithParameter.second);
    }

    return nodeIds;
}

int AddBeamBetweenNodesWithProperties(
    ProjectDocument& document,
    int startNodeId,
    int endNodeId,
    int materialId,
    int sectionId)
{
    if (materialId < 0 || sectionId < 0)
    {
        return -1;
    }

    if (!ResolveBeamDirection(document, startNodeId, endNodeId))
    {
        return -1;
    }

    return document.AddBeam(startNodeId, endNodeId, materialId, sectionId);
}

bool EnsureBeamPathBetweenNodes(
    ProjectDocument& document,
    int startNodeId,
    int endNodeId,
    int materialId,
    int sectionId)
{
    if (!ResolveBeamDirection(document, startNodeId, endNodeId))
    {
        return false;
    }

    const std::vector<int> nodeIdsOnSegment = CollectNodeIdsOnSegment(document, startNodeId, endNodeId);
    if (nodeIdsOnSegment.size() < 2)
    {
        return false;
    }

    bool handled = false;

    for (std::size_t i = 1; i < nodeIdsOnSegment.size(); ++i)
    {
        const int previousNodeId = nodeIdsOnSegment[i - 1];
        const int currentNodeId = nodeIdsOnSegment[i];
        if (previousNodeId == currentNodeId)
        {
            continue;
        }

        const Node* previousNode = document.FindNodeById(previousNodeId);
        const Node* currentNode = document.FindNodeById(currentNodeId);
        if (previousNode == nullptr || currentNode == nullptr)
        {
            continue;
        }

        const Vector2 previousPosition = Vector2{
            static_cast<float>(previousNode->position.x),
            static_cast<float>(previousNode->position.y)};
        const Vector2 currentPosition = Vector2{
            static_cast<float>(currentNode->position.x),
            static_cast<float>(currentNode->position.y)};
        if (std::abs(previousPosition.x - currentPosition.x) <= geometryTolerance &&
            std::abs(previousPosition.y - currentPosition.y) <= geometryTolerance)
        {
            continue;
        }

        if (document.HasBeamBetweenNodes(previousNodeId, currentNodeId))
        {
            handled = true;
            continue;
        }

        if (AddBeamBetweenNodesWithProperties(document, previousNodeId, currentNodeId, materialId, sectionId) >= 0)
        {
            handled = true;
        }
    }

    return handled;
}

bool AppendBeamPathBetweenNodesFast(
    ProjectDocument& document,
    int startNodeId,
    int endNodeId,
    int materialId,
    int sectionId,
    std::unordered_set<std::uint64_t>& existingBeamKeys)
{
    if (materialId < 0 || sectionId < 0 || !ResolveBeamDirection(document, startNodeId, endNodeId))
    {
        return false;
    }

    const std::vector<int> nodeIdsOnSegment = CollectNodeIdsOnSegment(document, startNodeId, endNodeId);
    if (nodeIdsOnSegment.size() < 2)
    {
        return false;
    }

    bool handled = false;

    for (std::size_t i = 1; i < nodeIdsOnSegment.size(); ++i)
    {
        int previousNodeId = nodeIdsOnSegment[i - 1];
        int currentNodeId = nodeIdsOnSegment[i];
        if (previousNodeId == currentNodeId || !ResolveBeamDirection(document, previousNodeId, currentNodeId))
        {
            continue;
        }

        const Node* previousNode = document.FindNodeById(previousNodeId);
        const Node* currentNode = document.FindNodeById(currentNodeId);
        if (previousNode == nullptr || currentNode == nullptr)
        {
            continue;
        }

        const Vector2 previousPosition = Vector2{
            static_cast<float>(previousNode->position.x),
            static_cast<float>(previousNode->position.y)};
        const Vector2 currentPosition = Vector2{
            static_cast<float>(currentNode->position.x),
            static_cast<float>(currentNode->position.y)};
        if (std::abs(previousPosition.x - currentPosition.x) <= geometryTolerance &&
            std::abs(previousPosition.y - currentPosition.y) <= geometryTolerance)
        {
            continue;
        }

        const std::uint64_t segmentKey = MakeBeamSegmentKey(previousNodeId, currentNodeId);
        if (!existingBeamKeys.insert(segmentKey).second)
        {
            handled = true;
            continue;
        }

        document.AppendBeam(Beam(
            document.nextBeamId++,
            previousNodeId,
            currentNodeId,
            materialId,
            sectionId));
        handled = true;
    }

    return handled;
}

bool SplitBeamsAtNode(
    ProjectDocument& document,
    EditorState& state,
    int nodeId,
    const AssignDistributedLoadAlongBeamPathFn& assignDistributedLoadAlongBeamPath)
{
    document.InvalidateLookupIndices();
    const Node* splitNode = document.FindNodeById(nodeId);
    if (splitNode == nullptr)
    {
        return false;
    }

    const Vector2 splitPosition = Vector2{
        static_cast<float>(splitNode->position.x),
        static_cast<float>(splitNode->position.y)};

    std::vector<Beam> beamsToSplit;
    for (const Beam& beam : document.beams)
    {
        if (beam.startNodeId == nodeId || beam.endNodeId == nodeId)
        {
            continue;
        }

        const Node* startNode = document.FindNodeById(beam.startNodeId);
        const Node* endNode = document.FindNodeById(beam.endNodeId);
        if (startNode == nullptr || endNode == nullptr)
        {
            continue;
        }

        const Vector2 startPosition = Vector2{
            static_cast<float>(startNode->position.x),
            static_cast<float>(startNode->position.y)};
        const Vector2 endPosition = Vector2{
            static_cast<float>(endNode->position.x),
            static_cast<float>(endNode->position.y)};

        if (IsPointInsideSegment(splitPosition, startPosition, endPosition))
        {
            beamsToSplit.push_back(beam);
        }
    }

    bool anySplit = false;

    for (const Beam& beam : beamsToSplit)
    {
        if (document.FindBeamById(beam.id) == nullptr)
        {
            continue;
        }

        const bool wasSelected = state.selectionState.selection.IsBeamSelected(beam.id);
        const BeamDistributedLoad* distributedLoad = document.FindDistributedLoadByBeamId(beam.id);
        const bool hadDistributedLoad = distributedLoad != nullptr;
        DistributedLoadValue distributedLoadValue;
        if (distributedLoad != nullptr)
        {
            distributedLoadValue = distributedLoad->value;
        }
        if (!document.RemoveBeam(beam.id))
        {
            continue;
        }

        state.selectionState.selection.RemoveBeam(beam.id);

        const int firstBeamId = AddBeamBetweenNodesWithProperties(
            document,
            beam.startNodeId,
            nodeId,
            beam.materialId,
            beam.sectionId);
        const int secondBeamId = AddBeamBetweenNodesWithProperties(
            document,
            nodeId,
            beam.endNodeId,
            beam.materialId,
            beam.sectionId);

        if (hadDistributedLoad && firstBeamId >= 0 && secondBeamId >= 0)
        {
            assignDistributedLoadAlongBeamPath(
                beam.startNodeId,
                beam.endNodeId,
                distributedLoadValue);
        }

        if (wasSelected)
        {
            if (firstBeamId >= 0)
            {
                state.selectionState.selection.AddBeam(firstBeamId);
            }

            if (secondBeamId >= 0)
            {
                state.selectionState.selection.AddBeam(secondBeamId);
            }
        }

        anySplit = true;
    }

    if (anySplit)
    {
        DeleteOperations::PurgeInvalidSelection(document, state);
        state.hover.ClearEntity();
    }

    document.InvalidateLookupIndices();

    return anySplit;
}
}

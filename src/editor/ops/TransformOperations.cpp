#include "editor/ops/TransformOperations.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace
{
constexpr double kGeometryTolerance = 0.00001;

struct PositionKey
{
    long long x = 0;
    long long y = 0;

    bool operator==(const PositionKey& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct PositionKeyHash
{
    std::size_t operator()(const PositionKey& key) const
    {
        const std::size_t xHash = std::hash<long long>{}(key.x);
        const std::size_t yHash = std::hash<long long>{}(key.y);
        return xHash ^ (yHash + 0x9e3779b9u + (xHash << 6) + (xHash >> 2));
    }
};

PositionKey MakePositionKey(const Point2D& position)
{
    return PositionKey{
        llround(position.x / kGeometryTolerance),
        llround(position.y / kGeometryTolerance)};
}

bool TryReflectPointAcrossLine(
    const Point2D& point,
    const Point2D& lineStart,
    const Point2D& lineEnd,
    Point2D& reflectedPoint)
{
    const long double lineDirectionX =
        static_cast<long double>(lineEnd.x) - static_cast<long double>(lineStart.x);
    const long double lineDirectionY =
        static_cast<long double>(lineEnd.y) - static_cast<long double>(lineStart.y);
    const long double lineLengthSquared =
        lineDirectionX * lineDirectionX + lineDirectionY * lineDirectionY;
    if (lineLengthSquared <= kGeometryTolerance)
    {
        return false;
    }

    const long double toPointX =
        static_cast<long double>(point.x) - static_cast<long double>(lineStart.x);
    const long double toPointY =
        static_cast<long double>(point.y) - static_cast<long double>(lineStart.y);
    const long double projectionParameter =
        (toPointX * lineDirectionX + toPointY * lineDirectionY) / lineLengthSquared;
    const long double projectionX =
        static_cast<long double>(lineStart.x) + lineDirectionX * projectionParameter;
    const long double projectionY =
        static_cast<long double>(lineStart.y) + lineDirectionY * projectionParameter;

    reflectedPoint = Point2D{
        static_cast<double>(2.0L * projectionX - static_cast<long double>(point.x)),
        static_cast<double>(2.0L * projectionY - static_cast<long double>(point.y))};
    return true;
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

void MergeNodeAttributes(Node& target, const Node& source)
{
    if (target.support == SupportType::None && source.support != SupportType::None)
    {
        target.support = source.support;
    }

    if (std::abs(target.load.fx) <= kGeometryTolerance && std::abs(source.load.fx) > kGeometryTolerance)
    {
        target.load.fx = source.load.fx;
    }

    if (std::abs(target.load.fy) <= kGeometryTolerance && std::abs(source.load.fy) > kGeometryTolerance)
    {
        target.load.fy = source.load.fy;
    }

    if (std::abs(target.load.mz) <= kGeometryTolerance && std::abs(source.load.mz) > kGeometryTolerance)
    {
        target.load.mz = source.load.mz;
    }
}
}

namespace TransformOperations
{
std::vector<int> CollectTransformNodeIds(const ProjectDocument& document, const Selection& selection)
{
    std::vector<int> nodeIds = selection.nodeIds;

    for (int beamId : selection.beamIds)
    {
        const Beam* beam = document.FindBeamById(beamId);
        if (beam == nullptr)
        {
            continue;
        }

        nodeIds.push_back(beam->startNodeId);
        nodeIds.push_back(beam->endNodeId);
    }

    std::sort(nodeIds.begin(), nodeIds.end());
    nodeIds.erase(std::unique(nodeIds.begin(), nodeIds.end()), nodeIds.end());
    return nodeIds;
}

bool HasTransformSelection(const Selection& selection)
{
    return !selection.nodeIds.empty() || !selection.beamIds.empty();
}

void ApplyMoveSelection(
    ProjectDocument& document,
    const Selection& selection,
    Point2D basePointWorld,
    Point2D targetPointWorld,
    const NormalizeStructureCallback& normalizeStructureAfterMove)
{
    const Point2D delta = Point2D{
        targetPointWorld.x - basePointWorld.x,
        targetPointWorld.y - basePointWorld.y};

    for (int nodeId : CollectTransformNodeIds(document, selection))
    {
        Node* node = document.FindNodeById(nodeId);
        if (node == nullptr)
        {
            continue;
        }

        node->position.x += delta.x;
        node->position.y += delta.y;
    }

    normalizeStructureAfterMove();
}

void ApplyCopySelection(
    ProjectDocument& document,
    const Selection& selection,
    Point2D basePointWorld,
    Point2D targetPointWorld,
    const SplitBeamAtNodeCallback& splitBeamAtNode,
    const AppendBeamPathCallback& appendBeamPathBetweenNodesFast,
    const AssignDistributedLoadCallback& assignDistributedLoadAlongBeamPath)
{
    const Point2D delta = Point2D{
        targetPointWorld.x - basePointWorld.x,
        targetPointWorld.y - basePointWorld.y};

    const std::vector<int> sourceNodeIds = CollectTransformNodeIds(document, selection);
    std::unordered_map<PositionKey, int, PositionKeyHash> nodeIdByPosition;
    nodeIdByPosition.reserve(document.nodes.size() + sourceNodeIds.size());
    for (const Node& node : document.nodes)
    {
        nodeIdByPosition[MakePositionKey(node.position)] = node.id;
    }

    std::unordered_map<int, int> copiedNodeIdBySourceId;
    copiedNodeIdBySourceId.reserve(sourceNodeIds.size());
    std::vector<int> newlyCreatedNodeIds;
    newlyCreatedNodeIds.reserve(sourceNodeIds.size());

    for (int sourceNodeId : sourceNodeIds)
    {
        const Node* sourceNode = document.FindNodeById(sourceNodeId);
        if (sourceNode == nullptr)
        {
            continue;
        }

        const Point2D copiedPosition = Point2D{
            sourceNode->position.x + delta.x,
            sourceNode->position.y + delta.y};
        const PositionKey positionKey = MakePositionKey(copiedPosition);
        const auto existingNodeIt = nodeIdByPosition.find(positionKey);
        if (existingNodeIt != nodeIdByPosition.end())
        {
            Node* copiedNode = document.FindNodeById(existingNodeIt->second);
            if (copiedNode == nullptr)
            {
                continue;
            }

            MergeNodeAttributes(*copiedNode, *sourceNode);
            copiedNodeIdBySourceId[sourceNodeId] = copiedNode->id;
            continue;
        }

        Node copiedNode;
        copiedNode.id = document.nextNodeId++;
        copiedNode.position = copiedPosition;
        MergeNodeAttributes(copiedNode, *sourceNode);

        Node* createdNode = document.AppendNode(copiedNode);
        if (createdNode == nullptr)
        {
            continue;
        }

        copiedNodeIdBySourceId[sourceNodeId] = createdNode->id;
        nodeIdByPosition.emplace(positionKey, createdNode->id);
        newlyCreatedNodeIds.push_back(createdNode->id);
    }

    if (!newlyCreatedNodeIds.empty())
    {
        if (!document.beams.empty())
        {
            for (int newNodeId : newlyCreatedNodeIds)
            {
                splitBeamAtNode(newNodeId);
            }
        }
    }

    std::unordered_set<std::uint64_t> existingBeamKeys;
    existingBeamKeys.reserve(document.beams.size() * 2 + selection.beamIds.size());
    for (const Beam& beam : document.beams)
    {
        existingBeamKeys.insert(MakeBeamSegmentKey(beam.startNodeId, beam.endNodeId));
    }

    bool addedAnyBeam = false;
    for (int beamId : selection.beamIds)
    {
        const Beam* beam = document.FindBeamById(beamId);
        if (beam == nullptr)
        {
            continue;
        }

        const auto startIt = copiedNodeIdBySourceId.find(beam->startNodeId);
        const auto endIt = copiedNodeIdBySourceId.find(beam->endNodeId);
        if (startIt == copiedNodeIdBySourceId.end() || endIt == copiedNodeIdBySourceId.end())
        {
            continue;
        }

        addedAnyBeam =
            appendBeamPathBetweenNodesFast(
                startIt->second,
                endIt->second,
                beam->materialId,
                beam->sectionId,
                existingBeamKeys) || addedAnyBeam;
    }

    if (addedAnyBeam)
    {
        for (int beamId : selection.beamIds)
        {
            const Beam* sourceBeam = document.FindBeamById(beamId);
            const BeamDistributedLoad* sourceLoad = document.FindDistributedLoadByBeamId(beamId);
            if (sourceBeam == nullptr || sourceLoad == nullptr)
            {
                continue;
            }

            const auto startIt = copiedNodeIdBySourceId.find(sourceBeam->startNodeId);
            const auto endIt = copiedNodeIdBySourceId.find(sourceBeam->endNodeId);
            if (startIt == copiedNodeIdBySourceId.end() || endIt == copiedNodeIdBySourceId.end())
            {
                continue;
            }

            assignDistributedLoadAlongBeamPath(
                startIt->second,
                endIt->second,
                sourceLoad->value);
        }
    }
}

bool ApplyMirrorSelection(
    ProjectDocument& document,
    const Selection& selection,
    Point2D axisStartPointWorld,
    Point2D axisEndPointWorld,
    bool mirrorKeepsOriginal,
    const SplitBeamAtNodeCallback& splitBeamAtNode,
    const AppendBeamPathCallback& appendBeamPathBetweenNodesFast,
    const AssignDistributedLoadCallback& assignDistributedLoadAlongBeamPath,
    const NormalizeStructureCallback& normalizeStructureAfterMove)
{
    Point2D reflectedPosition{0.0, 0.0};
    if (!TryReflectPointAcrossLine(
            axisStartPointWorld,
            axisStartPointWorld,
            axisEndPointWorld,
            reflectedPosition))
    {
        return false;
    }

    if (mirrorKeepsOriginal)
    {
        const std::vector<int> sourceNodeIds = CollectTransformNodeIds(document, selection);
        std::unordered_map<PositionKey, int, PositionKeyHash> nodeIdByPosition;
        nodeIdByPosition.reserve(document.nodes.size() + sourceNodeIds.size());
        for (const Node& node : document.nodes)
        {
            nodeIdByPosition[MakePositionKey(node.position)] = node.id;
        }

        std::unordered_map<int, int> mirroredNodeIdBySourceId;
        mirroredNodeIdBySourceId.reserve(sourceNodeIds.size());
        std::vector<int> newlyCreatedNodeIds;
        newlyCreatedNodeIds.reserve(sourceNodeIds.size());

        for (int sourceNodeId : sourceNodeIds)
        {
            const Node* sourceNode = document.FindNodeById(sourceNodeId);
            if (sourceNode == nullptr)
            {
                continue;
            }

            const Point2D currentPosition = sourceNode->position;
            if (!TryReflectPointAcrossLine(
                    currentPosition,
                    axisStartPointWorld,
                    axisEndPointWorld,
                    reflectedPosition))
            {
                continue;
            }

            const PositionKey positionKey = MakePositionKey(reflectedPosition);
            const auto existingNodeIt = nodeIdByPosition.find(positionKey);
            if (existingNodeIt != nodeIdByPosition.end())
            {
                Node* mirroredNode = document.FindNodeById(existingNodeIt->second);
                if (mirroredNode == nullptr)
                {
                    continue;
                }

                MergeNodeAttributes(*mirroredNode, *sourceNode);
                mirroredNodeIdBySourceId[sourceNodeId] = mirroredNode->id;
                continue;
            }

            Node newMirroredNode;
            newMirroredNode.id = document.nextNodeId++;
            newMirroredNode.position = reflectedPosition;
            MergeNodeAttributes(newMirroredNode, *sourceNode);

            Node* createdNode = document.AppendNode(newMirroredNode);
            if (createdNode == nullptr)
            {
                continue;
            }

            mirroredNodeIdBySourceId[sourceNodeId] = createdNode->id;
            newlyCreatedNodeIds.push_back(createdNode->id);
            nodeIdByPosition.emplace(positionKey, createdNode->id);
        }

        if (!newlyCreatedNodeIds.empty())
        {
            if (!document.beams.empty())
            {
                for (int newNodeId : newlyCreatedNodeIds)
                {
                    splitBeamAtNode(newNodeId);
                }
            }
        }

        std::unordered_set<std::uint64_t> existingBeamKeys;
        existingBeamKeys.reserve(document.beams.size() * 2 + selection.beamIds.size());
        for (const Beam& beam : document.beams)
        {
            existingBeamKeys.insert(MakeBeamSegmentKey(beam.startNodeId, beam.endNodeId));
        }

        bool addedAnyBeam = false;
        for (int beamId : selection.beamIds)
        {
            const Beam* beam = document.FindBeamById(beamId);
            if (beam == nullptr)
            {
                continue;
            }

            const auto startIt = mirroredNodeIdBySourceId.find(beam->startNodeId);
            const auto endIt = mirroredNodeIdBySourceId.find(beam->endNodeId);
            if (startIt == mirroredNodeIdBySourceId.end() || endIt == mirroredNodeIdBySourceId.end())
            {
                continue;
            }

            addedAnyBeam =
                appendBeamPathBetweenNodesFast(
                    startIt->second,
                    endIt->second,
                    beam->materialId,
                    beam->sectionId,
                    existingBeamKeys) || addedAnyBeam;
        }

        if (addedAnyBeam)
        {
            for (int beamId : selection.beamIds)
            {
                const Beam* sourceBeam = document.FindBeamById(beamId);
                const BeamDistributedLoad* sourceLoad = document.FindDistributedLoadByBeamId(beamId);
                if (sourceBeam == nullptr || sourceLoad == nullptr)
                {
                    continue;
                }

                const auto startIt = mirroredNodeIdBySourceId.find(sourceBeam->startNodeId);
                const auto endIt = mirroredNodeIdBySourceId.find(sourceBeam->endNodeId);
                if (startIt == mirroredNodeIdBySourceId.end() || endIt == mirroredNodeIdBySourceId.end())
                {
                    continue;
                }

                assignDistributedLoadAlongBeamPath(
                    startIt->second,
                    endIt->second,
                    sourceLoad->value);
            }
        }

        return true;
    }

    const std::vector<int> targetNodeIds = CollectTransformNodeIds(document, selection);

    for (int nodeId : targetNodeIds)
    {
        Node* node = document.FindNodeById(nodeId);
        if (node == nullptr)
        {
            continue;
        }

        const Point2D currentPosition = node->position;
        if (!TryReflectPointAcrossLine(
                currentPosition,
                axisStartPointWorld,
                axisEndPointWorld,
                reflectedPosition))
        {
            continue;
        }

        node->position = reflectedPosition;
    }

    normalizeStructureAfterMove();
    return true;
}
}

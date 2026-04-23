#include "editor/ops/SupportLoadOperations.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

#include "editor/ops/BeamOperations.h"

namespace
{
    constexpr double geometryTolerance = 0.00001;
    constexpr double distributedLoadTolerance = 1.0e-9;

    Vector2 Subtract(Vector2 a, Vector2 b)
    {
        return Vector2{a.x - b.x, a.y - b.y};
    }

    double DotProduct(Vector2 a, Vector2 b)
    {
        return static_cast<double>(a.x) * static_cast<double>(b.x) +
               static_cast<double>(a.y) * static_cast<double>(b.y);
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

    bool HasDistributedLoadComponent(double value)
    {
        return std::abs(value) > distributedLoadTolerance;
    }

    bool HasDistributedLoadValue(const DistributedLoadValue& load)
    {
        return HasDistributedLoadComponent(load.qxStart) ||
               HasDistributedLoadComponent(load.qyStart) ||
               HasDistributedLoadComponent(load.qxEnd) ||
               HasDistributedLoadComponent(load.qyEnd);
    }

    DistributedLoadValue BuildDistributedLoadSegment(
        const DistributedLoadValue& load,
        double startParameter,
        double endParameter)
    {
        startParameter = std::clamp(startParameter, 0.0, 1.0);
        endParameter = std::clamp(endParameter, 0.0, 1.0);

        return DistributedLoadValue{
            load.qxStart + (load.qxEnd - load.qxStart) * startParameter,
            load.qyStart + (load.qyEnd - load.qyStart) * startParameter,
            load.qxStart + (load.qxEnd - load.qxStart) * endParameter,
            load.qyStart + (load.qyEnd - load.qyStart) * endParameter};
    }

    DistributedLoadValue ReverseDistributedLoad(const DistributedLoadValue& load)
    {
        return DistributedLoadValue{
            load.qxEnd,
            load.qyEnd,
            load.qxStart,
            load.qyStart};
    }
}

namespace SupportLoadOperations
{
    SupportType GetSupportTypeForTool(EditorTool tool)
    {
        switch (tool)
        {
        case EditorTool::SetSupportX:
            return SupportType::RestrainedX;
        case EditorTool::SetSupportY:
            return SupportType::RestrainedY;
        case EditorTool::SetSupportXY:
            return SupportType::RestrainedXY;
        case EditorTool::SetSupportFixed:
            return SupportType::Fixed;
        case EditorTool::SetSupportNone:
        default:
            return SupportType::None;
        }
    }

    bool ApplySupportTypeToSelection(
        ProjectDocument& document,
        const EditorState& state,
        SupportType supportType)
    {
        bool changed = false;
        for (int nodeId : state.selectionState.selection.nodeIds)
        {
            Node* node = document.FindNodeById(nodeId);
            if (node != nullptr && node->support != supportType)
            {
                node->support = supportType;
                changed = true;
            }
        }

        return changed;
    }

    bool ApplySupportTypeToNode(
        ProjectDocument& document,
        int nodeId,
        SupportType supportType)
    {
        Node* node = document.FindNodeById(nodeId);
        if (node == nullptr || node->support == supportType)
        {
            return false;
        }

        node->support = supportType;
        return true;
    }

    bool ApplyPointLoadToNode(
        ProjectDocument& document,
        int nodeId,
        const NodalLoad& load)
    {
        Node* node = document.FindNodeById(nodeId);
        if (node == nullptr)
        {
            return false;
        }

        if (node->load.fx == load.fx &&
            node->load.fy == load.fy &&
            node->load.mz == load.mz)
        {
            return false;
        }

        node->load = load;
        return true;
    }

    bool ApplyPointLoadToSelection(
        ProjectDocument& document,
        const EditorState& state,
        const NodalLoad& load)
    {
        bool changed = false;
        for (int nodeId : state.selectionState.selection.nodeIds)
        {
            Node* node = document.FindNodeById(nodeId);
            if (node == nullptr)
            {
                continue;
            }

            if (node->load.fx != load.fx ||
                node->load.fy != load.fy ||
                node->load.mz != load.mz)
            {
                node->load = load;
                changed = true;
            }
        }

        return changed;
    }

    bool ApplyDistributedLoadToBeam(
        ProjectDocument& document,
        int beamId,
        const DistributedLoadValue& load)
    {
        if (document.FindBeamById(beamId) == nullptr)
        {
            return false;
        }

        const bool removeLoad = !HasDistributedLoadValue(load);
        BeamDistributedLoad* existingLoad = document.FindDistributedLoadByBeamId(beamId);

        if (removeLoad)
        {
            if (existingLoad == nullptr)
            {
                return false;
            }

            const std::size_t oldSize = document.distributedLoads.size();
            document.distributedLoads.erase(
                std::remove_if(
                    document.distributedLoads.begin(),
                    document.distributedLoads.end(),
                    [&](const BeamDistributedLoad& distributedLoad)
                    {
                        return distributedLoad.beamId == beamId;
                    }),
                document.distributedLoads.end());
            document.InvalidateLookupIndices();
            return document.distributedLoads.size() != oldSize;
        }

        if (existingLoad != nullptr)
        {
            if (existingLoad->value.qxStart == load.qxStart &&
                existingLoad->value.qyStart == load.qyStart &&
                existingLoad->value.qxEnd == load.qxEnd &&
                existingLoad->value.qyEnd == load.qyEnd)
            {
                return false;
            }

            existingLoad->value = load;
            return true;
        }

        document.AppendDistributedLoad(BeamDistributedLoad(document.nextDistributedLoadId++, beamId, load));
        return true;
    }

    bool ApplyDistributedLoadToSelection(
        ProjectDocument& document,
        const EditorState& state,
        const DistributedLoadValue& load)
    {
        const bool removeLoad = !HasDistributedLoadValue(load);
        const std::vector<int> selectedBeamIds = state.selectionState.selection.beamIds;
        if (selectedBeamIds.empty())
        {
            return false;
        }

        bool changed = false;
        if (removeLoad)
        {
            const std::unordered_set<int> selectedBeamIdSet(selectedBeamIds.begin(), selectedBeamIds.end());
            const std::size_t oldSize = document.distributedLoads.size();
            document.distributedLoads.erase(
                std::remove_if(
                    document.distributedLoads.begin(),
                    document.distributedLoads.end(),
                    [&selectedBeamIdSet](const BeamDistributedLoad& distributedLoad)
                    {
                        return selectedBeamIdSet.find(distributedLoad.beamId) != selectedBeamIdSet.end();
                    }),
                document.distributedLoads.end());
            document.InvalidateLookupIndices();

            return document.distributedLoads.size() != oldSize;
        }

        std::unordered_map<int, std::size_t> loadIndexByBeamId;
        loadIndexByBeamId.reserve(document.distributedLoads.size());
        for (std::size_t i = 0; i < document.distributedLoads.size(); ++i)
        {
            loadIndexByBeamId[document.distributedLoads[i].beamId] = i;
        }

        for (int beamId : selectedBeamIds)
        {
            if (document.FindBeamById(beamId) == nullptr)
            {
                continue;
            }

            const auto existingLoadIt = loadIndexByBeamId.find(beamId);
            if (existingLoadIt != loadIndexByBeamId.end())
            {
                BeamDistributedLoad& distributedLoad = document.distributedLoads[existingLoadIt->second];
                if (distributedLoad.value.qxStart != load.qxStart ||
                    distributedLoad.value.qyStart != load.qyStart ||
                    distributedLoad.value.qxEnd != load.qxEnd ||
                    distributedLoad.value.qyEnd != load.qyEnd)
                {
                    distributedLoad.value = load;
                    changed = true;
                }
                continue;
            }

            document.AppendDistributedLoad(BeamDistributedLoad(document.nextDistributedLoadId++, beamId, load));
            loadIndexByBeamId[beamId] = document.distributedLoads.size() - 1;
            changed = true;
        }

        return changed;
    }

    void AssignDistributedLoadAlongBeamPath(
        ProjectDocument& document,
        int originalStartNodeId,
        int originalEndNodeId,
        const DistributedLoadValue& loadValue)
    {
        if (!HasDistributedLoadValue(loadValue))
        {
            return;
        }

        const Node* originalStartNode = document.FindNodeById(originalStartNodeId);
        const Node* originalEndNode = document.FindNodeById(originalEndNodeId);
        if (originalStartNode == nullptr || originalEndNode == nullptr)
        {
            return;
        }

        const Vector2 originalStartPosition = Vector2{
            static_cast<float>(originalStartNode->position.x),
            static_cast<float>(originalStartNode->position.y)};
        const Vector2 originalEndPosition = Vector2{
            static_cast<float>(originalEndNode->position.x),
            static_cast<float>(originalEndNode->position.y)};

        const std::vector<int> nodeIdsOnSegment =
            BeamOperations::CollectNodeIdsOnSegment(document, originalStartNodeId, originalEndNodeId);
        if (nodeIdsOnSegment.size() < 2)
        {
            return;
        }

        std::unordered_map<int, std::size_t> loadIndexByBeamId;
        loadIndexByBeamId.reserve(document.distributedLoads.size());
        for (std::size_t i = 0; i < document.distributedLoads.size(); ++i)
        {
            loadIndexByBeamId[document.distributedLoads[i].beamId] = i;
        }

        for (std::size_t i = 1; i < nodeIdsOnSegment.size(); ++i)
        {
            const int pathStartNodeId = nodeIdsOnSegment[i - 1];
            const int pathEndNodeId = nodeIdsOnSegment[i];
            const Node* pathStartNode = document.FindNodeById(pathStartNodeId);
            const Node* pathEndNode = document.FindNodeById(pathEndNodeId);
            if (pathStartNode == nullptr || pathEndNode == nullptr)
            {
                continue;
            }

            const Vector2 pathStartPosition = Vector2{
                static_cast<float>(pathStartNode->position.x),
                static_cast<float>(pathStartNode->position.y)};
            const Vector2 pathEndPosition = Vector2{
                static_cast<float>(pathEndNode->position.x),
                static_cast<float>(pathEndNode->position.y)};
            const double startParameter = std::clamp(
                ProjectPointParameter(pathStartPosition, originalStartPosition, originalEndPosition),
                0.0,
                1.0);
            const double endParameter = std::clamp(
                ProjectPointParameter(pathEndPosition, originalStartPosition, originalEndPosition),
                0.0,
                1.0);

            DistributedLoadValue segmentLoad = BuildDistributedLoadSegment(
                loadValue,
                startParameter,
                endParameter);
            if (!HasDistributedLoadValue(segmentLoad))
            {
                continue;
            }

            Beam* rebuiltBeam = document.FindBeamBetweenNodes(pathStartNodeId, pathEndNodeId);
            if (rebuiltBeam == nullptr)
            {
                continue;
            }

            if (rebuiltBeam->startNodeId != pathStartNodeId || rebuiltBeam->endNodeId != pathEndNodeId)
            {
                segmentLoad = ReverseDistributedLoad(segmentLoad);
            }

            const auto existingLoadIt = loadIndexByBeamId.find(rebuiltBeam->id);
            if (existingLoadIt != loadIndexByBeamId.end())
            {
                document.distributedLoads[existingLoadIt->second].value = segmentLoad;
                continue;
            }

            document.AppendDistributedLoad(BeamDistributedLoad(
                document.nextDistributedLoadId++,
                rebuiltBeam->id,
                segmentLoad));
            loadIndexByBeamId[rebuiltBeam->id] = document.distributedLoads.size() - 1;
        }
    }
}

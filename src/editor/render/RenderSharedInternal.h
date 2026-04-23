#pragma once

namespace
{
    std::vector<int> MakeSortedUniqueIdList(const std::vector<int>& ids)
    {
        std::vector<int> sortedIds = ids;
        std::sort(sortedIds.begin(), sortedIds.end());
        sortedIds.erase(std::unique(sortedIds.begin(), sortedIds.end()), sortedIds.end());
        return sortedIds;
    }

    bool ContainsSortedId(const std::vector<int>& ids, int id)
    {
        return std::binary_search(ids.begin(), ids.end(), id);
    }

    Vector2 ToVector2(const Point2D& point)
    {
        return Vector2{
            static_cast<float>(point.x),
            static_cast<float>(point.y)};
    }

    double GetDimensionOffsetPixelsForDisplay(const Dimension& dimension, float cameraZoom)
    {
        if (dimension.offsetMode == DimensionOffsetMode::WorldUnits)
        {
            return dimension.offsetWorld * static_cast<double>(cameraZoom);
        }

        return dimension.offsetPixels;
    }

    std::vector<int> CollectPreviewTransformNodeIds(
        const ProjectDocument& document,
        const EditorState& state)
    {
        std::vector<int> nodeIds = state.selectionState.selection.nodeIds;

        for (int beamId : state.selectionState.selection.beamIds)
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

    bool ContainsNodeId(const std::vector<int>& nodeIds, int nodeId)
    {
        return ContainsSortedId(nodeIds, nodeId);
    }

    bool TryReflectPointAcrossLine(Vector2 point, Vector2 lineStart, Vector2 lineEnd, Vector2& reflectedPoint)
    {
        const Vector2 lineDirection = Vector2{
            lineEnd.x - lineStart.x,
            lineEnd.y - lineStart.y};
        const float lineLengthSquared =
            lineDirection.x * lineDirection.x +
            lineDirection.y * lineDirection.y;
        if (lineLengthSquared <= 1.0e-6f)
        {
            return false;
        }

        const Vector2 toPoint = Vector2{
            point.x - lineStart.x,
            point.y - lineStart.y};
        const float projectionParameter =
            (toPoint.x * lineDirection.x + toPoint.y * lineDirection.y) / lineLengthSquared;
        const Vector2 projection = Vector2{
            lineStart.x + lineDirection.x * projectionParameter,
            lineStart.y + lineDirection.y * projectionParameter};

        reflectedPoint = Vector2{
            2.0f * projection.x - point.x,
            2.0f * projection.y - point.y};
        return true;
    }
}

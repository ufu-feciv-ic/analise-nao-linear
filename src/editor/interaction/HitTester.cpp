#include "editor/interaction/HitTester.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kNodeHoverRadiusPixels = 6.0f;
constexpr float kBeamHitPaddingPixels = 7.0f;
constexpr float kDimensionHitPaddingPixels = 8.0f;

float Clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

float DistancePointToSegmentSquared(Vector2 point, Vector2 start, Vector2 end)
{
    const float dx = end.x - start.x;
    const float dy = end.y - start.y;
    const float lengthSquared = dx * dx + dy * dy;

    if (lengthSquared <= 1.0e-6f)
    {
        const float px = point.x - start.x;
        const float py = point.y - start.y;
        return px * px + py * py;
    }

    const float t = Clamp01(((point.x - start.x) * dx + (point.y - start.y) * dy) / lengthSquared);
    const Vector2 projection = Vector2{start.x + dx * t, start.y + dy * t};
    const float px = point.x - projection.x;
    const float py = point.y - projection.y;
    return px * px + py * py;
}

float DistancePointToSegmentScreen(Vector2 point, Vector2 start, Vector2 end)
{
    return sqrtf(DistancePointToSegmentSquared(point, start, end));
}

Vector2 Subtract(Vector2 a, Vector2 b)
{
    return Vector2{a.x - b.x, a.y - b.y};
}

double GetDimensionOffsetPixelsForDisplay(
    DimensionOffsetMode offsetMode,
    double offsetPixels,
    double offsetWorld,
    float cameraZoom)
{
    if (offsetMode == DimensionOffsetMode::WorldUnits)
    {
        return offsetWorld * static_cast<double>(cameraZoom);
    }

    return offsetPixels;
}
}

namespace HitTester
{
Node* FindNodeAtWorldPosition(ProjectDocument& document, const Camera2D& camera, Vector2 worldPosition)
{
    const float maxDistanceSquared =
        (kNodeHoverRadiusPixels / camera.zoom) * (kNodeHoverRadiusPixels / camera.zoom);

    Node* closestNode = nullptr;
    float closestDistanceSquared = maxDistanceSquared;

    for (Node& node : document.nodes)
    {
        const float dx = static_cast<float>(node.position.x) - worldPosition.x;
        const float dy = static_cast<float>(node.position.y) - worldPosition.y;
        const float distanceSquared = dx * dx + dy * dy;

        if (distanceSquared <= closestDistanceSquared)
        {
            closestDistanceSquared = distanceSquared;
            closestNode = &node;
        }
    }

    return closestNode;
}

Beam* FindBeamAtWorldPosition(ProjectDocument& document, const Camera2D& camera, Vector2 worldPosition)
{
    const Vector2 mouseScreen = GetWorldToScreen2D(worldPosition, camera);

    Beam* closestBeam = nullptr;
    float closestDistanceSquared = kBeamHitPaddingPixels * kBeamHitPaddingPixels;

    for (Beam& beam : document.beams)
    {
        const Node* startNode = document.FindNodeById(beam.startNodeId);
        const Node* endNode = document.FindNodeById(beam.endNodeId);
        if (startNode == nullptr || endNode == nullptr)
        {
            continue;
        }

        const Vector2 startScreen = GetWorldToScreen2D(
            Vector2{static_cast<float>(startNode->position.x), static_cast<float>(startNode->position.y)},
            camera);
        const Vector2 endScreen = GetWorldToScreen2D(
            Vector2{static_cast<float>(endNode->position.x), static_cast<float>(endNode->position.y)},
            camera);

        const float segmentDx = endScreen.x - startScreen.x;
        const float segmentDy = endScreen.y - startScreen.y;
        const float segmentLengthSquared = segmentDx * segmentDx + segmentDy * segmentDy;
        if (segmentLengthSquared <= 1.0e-6f)
        {
            continue;
        }

        const float segmentLength = sqrtf(segmentLengthSquared);
        if (segmentLength <= kNodeHoverRadiusPixels * 2.0f)
        {
            continue;
        }

        const float projectionT =
            ((mouseScreen.x - startScreen.x) * segmentDx + (mouseScreen.y - startScreen.y) * segmentDy) /
            segmentLengthSquared;
        const float endpointExclusionT = kNodeHoverRadiusPixels / segmentLength;
        if (projectionT <= endpointExclusionT || projectionT >= 1.0f - endpointExclusionT)
        {
            continue;
        }

        const float distanceSquared = DistancePointToSegmentSquared(mouseScreen, startScreen, endScreen);
        if (distanceSquared <= closestDistanceSquared)
        {
            closestDistanceSquared = distanceSquared;
            closestBeam = &beam;
        }
    }

    return closestBeam;
}

Dimension* FindDimensionAtScreenPosition(ProjectDocument& document, const Camera2D& camera, Vector2 screenPosition)
{
    Dimension* closestDimension = nullptr;
    float closestDistance = kDimensionHitPaddingPixels;

    for (Dimension& dimension : document.dimensions)
    {
        const Node* startNode = document.FindNodeById(dimension.startNodeId);
        const Node* endNode = document.FindNodeById(dimension.endNodeId);
        if (startNode == nullptr || endNode == nullptr)
        {
            continue;
        }

        const Vector2 startScreen = GetWorldToScreen2D(
            Vector2{static_cast<float>(startNode->position.x), static_cast<float>(startNode->position.y)},
            camera);
        const Vector2 endScreen = GetWorldToScreen2D(
            Vector2{static_cast<float>(endNode->position.x), static_cast<float>(endNode->position.y)},
            camera);

        const double displayOffsetPixels = GetDimensionOffsetPixelsForDisplay(
            dimension.offsetMode,
            dimension.offsetPixels,
            dimension.offsetWorld,
            camera.zoom);

        Vector2 lineStart{};
        Vector2 lineEnd{};
        Vector2 extensionStartEnd{};
        Vector2 extensionEndEnd{};

        switch (dimension.type)
        {
        case DimensionType::Horizontal:
        {
            const float anchorY =
                (displayOffsetPixels < 0.0 ? std::min(startScreen.y, endScreen.y) : std::max(startScreen.y, endScreen.y)) +
                static_cast<float>(displayOffsetPixels);
            const float overshootY = anchorY + (displayOffsetPixels < 0.0 ? -10.0f : 10.0f);
            lineStart = Vector2{startScreen.x, anchorY};
            lineEnd = Vector2{endScreen.x, anchorY};
            extensionStartEnd = Vector2{startScreen.x, overshootY};
            extensionEndEnd = Vector2{endScreen.x, overshootY};
            break;
        }

        case DimensionType::Vertical:
        {
            const float anchorX =
                (displayOffsetPixels < 0.0 ? std::min(startScreen.x, endScreen.x) : std::max(startScreen.x, endScreen.x)) +
                static_cast<float>(displayOffsetPixels);
            const float overshootX = anchorX + (displayOffsetPixels < 0.0 ? -10.0f : 10.0f);
            lineStart = Vector2{anchorX, startScreen.y};
            lineEnd = Vector2{anchorX, endScreen.y};
            extensionStartEnd = Vector2{overshootX, startScreen.y};
            extensionEndEnd = Vector2{overshootX, endScreen.y};
            break;
        }

        case DimensionType::Aligned:
        default:
        {
            const Vector2 segment = Subtract(endScreen, startScreen);
            const float length = sqrtf(segment.x * segment.x + segment.y * segment.y);
            if (length <= 1.0e-6f)
            {
                continue;
            }

            const Vector2 normal = Vector2{-segment.y / length, segment.x / length};
            const float signedOffsetPixels = static_cast<float>(displayOffsetPixels);
            const Vector2 offsetVector = Vector2{normal.x * signedOffsetPixels, normal.y * signedOffsetPixels};
            const Vector2 overshootVector = Vector2{
                normal.x * 10.0f * (signedOffsetPixels < 0.0f ? -1.0f : 1.0f),
                normal.y * 10.0f * (signedOffsetPixels < 0.0f ? -1.0f : 1.0f)};

            lineStart = Vector2{startScreen.x + offsetVector.x, startScreen.y + offsetVector.y};
            lineEnd = Vector2{endScreen.x + offsetVector.x, endScreen.y + offsetVector.y};
            extensionStartEnd = Vector2{lineStart.x + overshootVector.x, lineStart.y + overshootVector.y};
            extensionEndEnd = Vector2{lineEnd.x + overshootVector.x, lineEnd.y + overshootVector.y};
            break;
        }
        }

        const float distance = std::min({
            DistancePointToSegmentScreen(screenPosition, lineStart, lineEnd),
            DistancePointToSegmentScreen(screenPosition, startScreen, extensionStartEnd),
            DistancePointToSegmentScreen(screenPosition, endScreen, extensionEndEnd)});
        if (distance <= closestDistance)
        {
            closestDistance = distance;
            closestDimension = &dimension;
        }
    }

    return closestDimension;
}
}

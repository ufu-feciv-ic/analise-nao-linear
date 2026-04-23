#pragma once

namespace
{
    struct SupportStyle
    {
        Color outlineColor{};
        Color fillColor{};
        Color groundColor{};
    };

    void DrawTriangleOutline(Vector2 a, Vector2 b, Vector2 c, float thickness, Color color)
    {
        DrawLineEx(a, b, thickness, color);
        DrawLineEx(b, c, thickness, color);
        DrawLineEx(c, a, thickness, color);
    }

    void DrawSupportTriangle(
        Vector2 apex,
        Vector2 baseLeft,
        Vector2 baseRight,
        float thickness,
        Color fillColor,
        Color outlineColor)
    {
        DrawFilledTriangleSafe(apex, baseLeft, baseRight, fillColor);
        DrawTriangleOutline(apex, baseLeft, baseRight, thickness, outlineColor);
        DrawLineEx(apex, baseRight, thickness, Color{170, 170, 170, 255});
    }

    void DrawHatchedGroundAligned(
        Vector2 baseStart,
        Vector2 baseEnd,
        Vector2 outwardNormal,
        float hatchLength,
        float spacing,
        float baseThickness,
        float hatchThickness,
        Color color)
    {
        DrawLineEx(baseStart, baseEnd, baseThickness, color);

        const Vector2 baseVector = Vector2{baseEnd.x - baseStart.x, baseEnd.y - baseStart.y};
        const Vector2 baseDirection = NormalizeVectorSafe(baseVector);
        const Vector2 normalDirection = NormalizeVectorSafe(outwardNormal);
        const float baseLength = sqrtf(baseVector.x * baseVector.x + baseVector.y * baseVector.y);
        if (baseLength <= 1.0e-6f || (normalDirection.x == 0.0f && normalDirection.y == 0.0f))
        {
            return;
        }

        for (float nominalEndDistance = 0.0f; nominalEndDistance <= baseLength + hatchLength; nominalEndDistance += spacing)
        {
            const float tMin = std::max(0.0f, (nominalEndDistance - baseLength) / hatchLength);
            const float tMax = std::min(1.0f, nominalEndDistance / hatchLength);
            if (tMax - tMin <= 0.01f)
            {
                continue;
            }

            auto pointAt = [&](float t)
            {
                const float localX = nominalEndDistance - t * hatchLength;
                return Vector2{
                    baseStart.x + baseDirection.x * localX + normalDirection.x * t * hatchLength,
                    baseStart.y + baseDirection.y * localX + normalDirection.y * t * hatchLength};
            };

            DrawLineEx(pointAt(tMax), pointAt(tMin), hatchThickness, color);
        }
    }

    const ProjectDerivedData::NodeConnectivity& GetNodeConnectivity(
        const ProjectDerivedData& derivedData,
        int nodeId)
    {
        static const ProjectDerivedData::NodeConnectivity kDefaultConnectivity{};
        const ProjectDerivedData::NodeConnectivity* connectivity = derivedData.FindNodeConnectivityById(nodeId);
        return connectivity != nullptr ? *connectivity : kDefaultConnectivity;
    }

    Vector2 ChooseSecondKindSupportDirection(const ProjectDerivedData::NodeConnectivity& connectivity)
    {
        const unsigned char occupiedMask = connectivity.occupiedDirectionMask;
        if ((occupiedMask & NodeOccupiedDirectionDown) == 0)
        {
            return Vector2{0.0f, 1.0f};
        }

        if ((occupiedMask & NodeOccupiedDirectionUp) == 0)
        {
            return Vector2{0.0f, -1.0f};
        }

        if ((occupiedMask & NodeOccupiedDirectionLeft) == 0)
        {
            return Vector2{-1.0f, 0.0f};
        }

        if ((occupiedMask & NodeOccupiedDirectionRight) == 0)
        {
            return Vector2{1.0f, 0.0f};
        }

        if (connectivity.hasAverageConnectedBeamAxis)
        {
            Vector2 orthogonalDirection = NormalizeVectorSafe(
                Vector2{
                    static_cast<float>(-connectivity.averageConnectedBeamAxis.y),
                    static_cast<float>(connectivity.averageConnectedBeamAxis.x)});
            if (orthogonalDirection.y < 0.0f ||
                (std::abs(orthogonalDirection.y) <= 1.0e-6f && orthogonalDirection.x > 0.0f))
            {
                orthogonalDirection.x = -orthogonalDirection.x;
                orthogonalDirection.y = -orthogonalDirection.y;
            }

            return orthogonalDirection;
        }

        return Vector2{0.0f, 1.0f};
    }

    Vector2 ChooseVerticalRollerSupportDirection(const ProjectDerivedData::NodeConnectivity& connectivity)
    {
        const unsigned char occupiedMask = connectivity.occupiedDirectionMask;
        if ((occupiedMask & NodeOccupiedDirectionDown) == 0)
        {
            return Vector2{0.0f, 1.0f};
        }

        if ((occupiedMask & NodeOccupiedDirectionUp) == 0)
        {
            return Vector2{0.0f, -1.0f};
        }

        return Vector2{0.0f, 1.0f};
    }

    Vector2 ChooseHorizontalRollerSupportDirection(const ProjectDerivedData::NodeConnectivity& connectivity)
    {
        const unsigned char occupiedMask = connectivity.occupiedDirectionMask;
        if ((occupiedMask & NodeOccupiedDirectionLeft) == 0)
        {
            return Vector2{-1.0f, 0.0f};
        }

        if ((occupiedMask & NodeOccupiedDirectionRight) == 0)
        {
            return Vector2{1.0f, 0.0f};
        }

        return Vector2{-1.0f, 0.0f};
    }
}


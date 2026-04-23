#pragma once

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include "editor/EditorRenderer.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>

#include "editor/render/RenderMathInternal.h"
#include "editor/render/LoadRendererInternal.h"
#include "editor/render/DimensionRendererInternal.h"
#include "editor/render/RenderSharedInternal.h"
#include "editor/render/StructureRendererInternal.h"
#include "editor/render/SupportRendererInternal.h"

namespace
{
    constexpr int kSmallNodeCircleSegments = 16;
    constexpr int kShadowCircleSegments = 8;

    void DrawSmallNodeCircle(Vector2 center, float radius, Color color)
    {
        DrawCircleSector(center, radius, 0.0f, 360.0f, kSmallNodeCircleSegments, color);
    }

    void DrawSmallNodeCircleOutline(Vector2 center, float radius, float thickness, Color color)
    {
        if (radius <= 0.0f)
        {
            return;
        }

        const float angleStep = 2.0f * PI / static_cast<float>(kSmallNodeCircleSegments);
        for (int segmentIndex = 0; segmentIndex < kSmallNodeCircleSegments; ++segmentIndex)
        {
            const float startAngle = angleStep * static_cast<float>(segmentIndex);
            const float endAngle = angleStep * static_cast<float>(segmentIndex + 1);
            const Vector2 startPoint = Vector2{
                center.x + cosf(startAngle) * radius,
                center.y + sinf(startAngle) * radius};
            const Vector2 endPoint = Vector2{
                center.x + cosf(endAngle) * radius,
                center.y + sinf(endAngle) * radius};
            DrawLineEx(startPoint, endPoint, thickness, color);
        }
    }

    void DrawShadowCircle(Vector2 center, float radius, Color color)
    {
        DrawCircleSector(center, radius, 0.0f, 360.0f, kShadowCircleSegments, color);
    }

    void DrawGridLayer(
        const Vector2& worldTopLeft,
        const Vector2& worldBottomRight,
        float zoomLevel,
        float spacing,
        float alpha)
    {
        if (alpha <= 0.001f || spacing <= 0.0f)
        {
            return;
        }

        const float safeZoom = (zoomLevel > 0.0001f) ? zoomLevel : 0.0001f;

        const float secondarySpacing = spacing;
        const float primarySpacing = spacing * 5.0f;

        const Color secondaryColor = WithScaledAlpha(Color{0, 0, 0, 20}, alpha);
        const Color primaryColor = WithScaledAlpha(Color{0, 0, 0, 40}, alpha);

        const float secondaryThickness = 1.0f / safeZoom;
        const float primaryThickness = 2.0f / safeZoom;

        const int startSecondaryX = static_cast<int>(floorf(worldTopLeft.x / secondarySpacing));
        const int endSecondaryX = static_cast<int>(ceilf(worldBottomRight.x / secondarySpacing));
        const int startSecondaryY = static_cast<int>(floorf(worldTopLeft.y / secondarySpacing));
        const int endSecondaryY = static_cast<int>(ceilf(worldBottomRight.y / secondarySpacing));

        for (int i = startSecondaryX; i <= endSecondaryX; ++i)
        {
            if (i % 5 == 0)
            {
                continue;
            }

            const float x = static_cast<float>(i) * secondarySpacing;

            DrawLineEx(
                Vector2{x, worldTopLeft.y},
                Vector2{x, worldBottomRight.y},
                secondaryThickness,
                secondaryColor);
        }

        for (int i = startSecondaryY; i <= endSecondaryY; ++i)
        {
            if (i % 5 == 0)
            {
                continue;
            }

            const float y = static_cast<float>(i) * secondarySpacing;

            DrawLineEx(
                Vector2{worldTopLeft.x, y},
                Vector2{worldBottomRight.x, y},
                secondaryThickness,
                secondaryColor);
        }

        const int startPrimaryX = static_cast<int>(floorf(worldTopLeft.x / primarySpacing));
        const int endPrimaryX = static_cast<int>(ceilf(worldBottomRight.x / primarySpacing));
        const int startPrimaryY = static_cast<int>(floorf(worldTopLeft.y / primarySpacing));
        const int endPrimaryY = static_cast<int>(ceilf(worldBottomRight.y / primarySpacing));

        for (int i = startPrimaryX; i <= endPrimaryX; ++i)
        {
            const float x = static_cast<float>(i) * primarySpacing;

            DrawLineEx(
                Vector2{x, worldTopLeft.y},
                Vector2{x, worldBottomRight.y},
                primaryThickness,
                primaryColor);
        }

        for (int i = startPrimaryY; i <= endPrimaryY; ++i)
        {
            const float y = static_cast<float>(i) * primarySpacing;

            DrawLineEx(
                Vector2{worldTopLeft.x, y},
                Vector2{worldBottomRight.x, y},
                primaryThickness,
                primaryColor);
        }
    }
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#pragma once

#include <string>

#include "utils/UnitConversion.h"

namespace
{
    constexpr float dimensionGapPixels = 18.0f;
    constexpr float dimensionNodeClearancePixels = 10.0f;
    constexpr float dimensionExtensionOvershootPixels = 10.0f;
    constexpr float dimensionArrowSizePixels = 10.0f;
    constexpr float dimensionTextOffsetPixels = 12.0f;

    struct ScreenDimensionGeometry
    {
        Vector2 startExtensionStart{};
        Vector2 startExtensionEnd{};
        Vector2 endExtensionStart{};
        Vector2 endExtensionEnd{};
        Vector2 lineStart{};
        Vector2 lineEnd{};
        Vector2 startArrowTip{};
        Vector2 startArrowLeft{};
        Vector2 startArrowRight{};
        Vector2 endArrowTip{};
        Vector2 endArrowLeft{};
        Vector2 endArrowRight{};
        Vector2 labelScreen{};
        float lineLengthPixels = 0.0f;
        float textRotationDegrees = 0.0f;
        double measuredLengthWorld = 0.0;
    };

    struct DimensionLabelJob
    {
        Vector2 screenCenter{};
        std::string text;
        Vector2 textSize{};
        float rotationDegrees = 0.0f;
        Color textColor{};
    };

    double GetMeasuredLengthWorld(Vector2 startWorld, Vector2 endWorld, DimensionType type)
    {
        switch (type)
        {
        case DimensionType::Horizontal:
            return std::abs(static_cast<double>(endWorld.x - startWorld.x));
        case DimensionType::Vertical:
            return std::abs(static_cast<double>(endWorld.y - startWorld.y));
        case DimensionType::Aligned:
        default:
            return std::sqrt(
                static_cast<double>(endWorld.x - startWorld.x) * static_cast<double>(endWorld.x - startWorld.x) +
                static_cast<double>(endWorld.y - startWorld.y) * static_cast<double>(endWorld.y - startWorld.y));
        }
    }

    constexpr float kDimensionFadeOutThresholdRatio = 0.55f;
    constexpr float kDimensionFadeInThresholdRatio = 0.72f;

    std::string FormatDisplayLength(double valueInMeters, LengthUnit unit)
    {
        return FormatDisplayScalar(UnitConversion::LengthToDisplay(valueInMeters, unit)) +
               " " +
               UnitConversion::GetLengthUnitLabel(unit);
    }

    bool TryBuildScreenDimensionGeometry(
        Vector2 startWorld,
        Vector2 endWorld,
        Vector2 startScreen,
        Vector2 endScreen,
        DimensionType type,
        double offsetPixels,
        ScreenDimensionGeometry& geometry)
    {
        const float clampedOffsetPixels =
            static_cast<float>(offsetPixels >= 0.0
                ? std::max(offsetPixels, static_cast<double>(dimensionGapPixels))
                : std::min(offsetPixels, static_cast<double>(-dimensionGapPixels)));

        Vector2 outsideNormal{0.0f, 0.0f};
        switch (type)
        {
        case DimensionType::Horizontal:
        {
            const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
            const float anchorY =
                (sideSign < 0.0f ? std::min(startScreen.y, endScreen.y) : std::max(startScreen.y, endScreen.y)) +
                clampedOffsetPixels;
            const float overshootY =
                anchorY +
                sideSign * dimensionExtensionOvershootPixels;
            const float extensionStartYStart = startScreen.y + sideSign * dimensionNodeClearancePixels;
            const float extensionStartYEnd = endScreen.y + sideSign * dimensionNodeClearancePixels;

            geometry.startExtensionStart = Vector2{startScreen.x, extensionStartYStart};
            geometry.startExtensionEnd = Vector2{startScreen.x, overshootY};
            geometry.endExtensionStart = Vector2{endScreen.x, extensionStartYEnd};
            geometry.endExtensionEnd = Vector2{endScreen.x, overshootY};
            geometry.lineStart = Vector2{startScreen.x, anchorY};
            geometry.lineEnd = Vector2{endScreen.x, anchorY};
            outsideNormal = Vector2{0.0f, sideSign};
            break;
        }

        case DimensionType::Vertical:
        {
            const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
            const float anchorX =
                (sideSign < 0.0f ? std::min(startScreen.x, endScreen.x) : std::max(startScreen.x, endScreen.x)) +
                clampedOffsetPixels;
            const float overshootX =
                anchorX +
                sideSign * dimensionExtensionOvershootPixels;
            const float extensionStartXStart = startScreen.x + sideSign * dimensionNodeClearancePixels;
            const float extensionStartXEnd = endScreen.x + sideSign * dimensionNodeClearancePixels;

            geometry.startExtensionStart = Vector2{extensionStartXStart, startScreen.y};
            geometry.startExtensionEnd = Vector2{overshootX, startScreen.y};
            geometry.endExtensionStart = Vector2{extensionStartXEnd, endScreen.y};
            geometry.endExtensionEnd = Vector2{overshootX, endScreen.y};
            geometry.lineStart = Vector2{anchorX, startScreen.y};
            geometry.lineEnd = Vector2{anchorX, endScreen.y};
            outsideNormal = Vector2{sideSign, 0.0f};
            break;
        }

        case DimensionType::Aligned:
        default:
        {
            const Vector2 screenSegment = Vector2{endScreen.x - startScreen.x, endScreen.y - startScreen.y};
            const Vector2 lineDirection = NormalizeVector(screenSegment);
            if (VectorLength(lineDirection) <= 1.0e-6f)
            {
                return false;
            }

            const Vector2 rawNormal = Vector2{-lineDirection.y, lineDirection.x};
            const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
            outsideNormal = Vector2{
                rawNormal.x * sideSign,
                rawNormal.y * sideSign};
            const Vector2 offsetVector = Vector2{
                outsideNormal.x * fabsf(clampedOffsetPixels),
                outsideNormal.y * fabsf(clampedOffsetPixels)};
            const Vector2 clearanceVector = Vector2{
                outsideNormal.x * dimensionNodeClearancePixels,
                outsideNormal.y * dimensionNodeClearancePixels};
            const Vector2 overshootVector = Vector2{
                outsideNormal.x * dimensionExtensionOvershootPixels,
                outsideNormal.y * dimensionExtensionOvershootPixels};

            geometry.startExtensionStart = Vector2{startScreen.x + clearanceVector.x, startScreen.y + clearanceVector.y};
            geometry.endExtensionStart = Vector2{endScreen.x + clearanceVector.x, endScreen.y + clearanceVector.y};
            geometry.lineStart = Vector2{startScreen.x + offsetVector.x, startScreen.y + offsetVector.y};
            geometry.lineEnd = Vector2{endScreen.x + offsetVector.x, endScreen.y + offsetVector.y};
            geometry.startExtensionEnd = Vector2{geometry.lineStart.x + overshootVector.x, geometry.lineStart.y + overshootVector.y};
            geometry.endExtensionEnd = Vector2{geometry.lineEnd.x + overshootVector.x, geometry.lineEnd.y + overshootVector.y};
            break;
        }
        }

        const Vector2 lineVector = Vector2{geometry.lineEnd.x - geometry.lineStart.x, geometry.lineEnd.y - geometry.lineStart.y};
        const Vector2 lineDirection = NormalizeVector(lineVector);
        const float lineLength = VectorLength(lineVector);
        if (lineLength <= 1.0e-6f)
        {
            return false;
        }

        const Vector2 lineNormal = Vector2{-lineDirection.y, lineDirection.x};
        const float arrowSize = std::min(dimensionArrowSizePixels, lineLength * 0.25f);
        const float arrowHalfWidth = arrowSize * 0.35f;

        geometry.startArrowTip = geometry.lineStart;
        const Vector2 startArrowBase = Vector2{
            geometry.lineStart.x + lineDirection.x * arrowSize,
            geometry.lineStart.y + lineDirection.y * arrowSize};
        geometry.startArrowLeft = Vector2{
            startArrowBase.x + lineNormal.x * arrowHalfWidth,
            startArrowBase.y + lineNormal.y * arrowHalfWidth};
        geometry.startArrowRight = Vector2{
            startArrowBase.x - lineNormal.x * arrowHalfWidth,
            startArrowBase.y - lineNormal.y * arrowHalfWidth};

        geometry.endArrowTip = geometry.lineEnd;
        const Vector2 endArrowBase = Vector2{
            geometry.lineEnd.x - lineDirection.x * arrowSize,
            geometry.lineEnd.y - lineDirection.y * arrowSize};
        geometry.endArrowLeft = Vector2{
            endArrowBase.x + lineNormal.x * arrowHalfWidth,
            endArrowBase.y + lineNormal.y * arrowHalfWidth};
        geometry.endArrowRight = Vector2{
            endArrowBase.x - lineNormal.x * arrowHalfWidth,
            endArrowBase.y - lineNormal.y * arrowHalfWidth};

        const Vector2 lineMidpoint = Vector2{
            (geometry.lineStart.x + geometry.lineEnd.x) * 0.5f,
            (geometry.lineStart.y + geometry.lineEnd.y) * 0.5f};
        geometry.labelScreen = Vector2{
            lineMidpoint.x + outsideNormal.x * dimensionTextOffsetPixels,
            lineMidpoint.y + outsideNormal.y * dimensionTextOffsetPixels};
        geometry.lineLengthPixels = lineLength;
        geometry.textRotationDegrees = NormalizeReadableAngle(atan2f(lineDirection.y, lineDirection.x) * RAD2DEG);
        geometry.measuredLengthWorld = GetMeasuredLengthWorld(startWorld, endWorld, type);
        return true;
    }
}

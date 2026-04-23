#pragma once

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "editor/cache/ProjectDerivedData.h"
#include "model/Node.h"
#include "raylib.h"
#include "utils/UnitConversion.h"

namespace
{
    enum class RotatedTextCorner
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    bool HasNodalLoadComponent(double value)
    {
        return std::abs(value) > 1.0e-9;
    }

    bool HasVisibleNodalLoad(const NodalLoad& load)
    {
        return HasNodalLoadComponent(load.fx) ||
               HasNodalLoadComponent(load.fy) ||
               HasNodalLoadComponent(load.mz);
    }

    std::string FormatDisplayScalar(double value)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;

        std::string text = stream.str();
        const std::size_t decimalSeparator = text.find('.');
        if (decimalSeparator != std::string::npos)
        {
            while (!text.empty() && text.back() == '0')
            {
                text.pop_back();
            }

            if (!text.empty() && text.back() == '.')
            {
                text.pop_back();
            }
        }

        if (text == "-0")
        {
            text = "0";
        }

        return text;
    }

    std::string FormatDisplayForce(double valueInNewtons, ForceUnit unit)
    {
        return FormatDisplayScalar(std::abs(UnitConversion::ForceToDisplay(valueInNewtons, unit))) +
               " " +
               UnitConversion::GetForceUnitLabel(unit);
    }

    std::string FormatDisplayMoment(double valueInNewtonMeters, MomentUnit unit)
    {
        return FormatDisplayScalar(std::abs(UnitConversion::MomentToDisplay(valueInNewtonMeters, unit))) +
               " " +
               UnitConversion::GetMomentUnitLabel(unit);
    }

    std::string FormatDisplayDistributedLoad(double valueInNewtonsPerMeter, DistributedLoadUnit unit)
    {
        return FormatDisplayScalar(std::abs(UnitConversion::DistributedLoadToDisplay(valueInNewtonsPerMeter, unit))) +
               " " +
               UnitConversion::GetDistributedLoadUnitLabel(unit);
    }

    void DrawLoadArrow(
        Vector2 start,
        Vector2 end,
        float thickness,
        Color color,
        float headWidth,
        float minimumTailLengthFactor = 2.0f)
    {
        float headLength = thickness * 5.0f;
        const float minimumTailLength = thickness * minimumTailLengthFactor;
        const float dx = end.x - start.x;
        const float dy = end.y - start.y;
        const float length = sqrtf(dx * dx + dy * dy);
        if (length <= 1.0e-6f)
        {
            return;
        }

        if (length < headLength + minimumTailLength)
        {
            const float maximumHeadLength = std::max(0.0f, length - minimumTailLength);
            const float headScale =
                (headLength > 1.0e-6f)
                    ? Clamp01(maximumHeadLength / headLength)
                    : 0.0f;
            headWidth *= headScale;
            headLength = maximumHeadLength;
        }

        const float angle = atan2f(end.y - start.y, end.x - start.x);
        const Vector2 shortenedEnd = Vector2{
            end.x - cosf(angle) * headLength,
            end.y - sinf(angle) * headLength};

        DrawLineEx(start, shortenedEnd, thickness, color);

        Vector2 headPointA = Vector2{end.x - headLength, end.y - headWidth};
        Vector2 headPointB = Vector2{end.x - headLength, end.y + headWidth};
        headPointA = RotatePointAround(headPointA, end, angle);
        headPointB = RotatePointAround(headPointB, end, angle);
        DrawTriangle(headPointA, headPointB, end, color);
    }

    Vector2 ComputeLoadLabelPosition(
        Vector2 tail,
        Vector2 tip,
        Vector2 textSize,
        float alongOffset,
        float normalOffset,
        bool useNormalBasedHorizontalAnchor = false)
    {
        const Vector2 direction = NormalizeVectorSafe(Vector2{tip.x - tail.x, tip.y - tail.y});
        Vector2 normal = Vector2{-direction.y, direction.x};
        if (normal.y > 0.0f || (std::abs(normal.y) <= 1.0e-4f && normal.x < 0.0f))
        {
            normal.x = -normal.x;
            normal.y = -normal.y;
        }

        const float verticalHalfSize = textSize.y * 0.3f;
        const Vector2 anchorPosition = Vector2{
            tail.x + direction.x * alongOffset + normal.x * normalOffset,
            tail.y + direction.y * alongOffset + normal.y * (normalOffset + verticalHalfSize)};

        bool anchorByRightEdge = false;
        if (useNormalBasedHorizontalAnchor)
        {
            anchorByRightEdge =
                normal.x < -1.0e-4f ||
                (std::abs(normal.x) <= 1.0e-4f && direction.x > 0.0f);
        }
        else
        {
            anchorByRightEdge =
                direction.x > 0.0f && std::abs(direction.y) <= 1.0e-4f;
        }

        return Vector2{
            anchorByRightEdge ? (anchorPosition.x - textSize.x) : anchorPosition.x,
            anchorPosition.y - textSize.y * 0.5f};
    }

    Vector2 ComputeRotatedTextCenterFromCorner(
        Vector2 cornerPoint,
        Vector2 textSize,
        float rotationDegrees,
        RotatedTextCorner corner)
    {
        const float angleRadians = rotationDegrees * DEG2RAD;
        const Vector2 xAxis = Vector2{cosf(angleRadians), sinf(angleRadians)};
        const Vector2 yAxis = Vector2{-sinf(angleRadians), cosf(angleRadians)};
        const float halfWidth = textSize.x * 0.5f;
        const float halfHeight = textSize.y * 0.5f;

        switch (corner)
        {
        case RotatedTextCorner::TopLeft:
            return Vector2{
                cornerPoint.x + xAxis.x * halfWidth + yAxis.x * halfHeight,
                cornerPoint.y + xAxis.y * halfWidth + yAxis.y * halfHeight};
        case RotatedTextCorner::TopRight:
            return Vector2{
                cornerPoint.x - xAxis.x * halfWidth + yAxis.x * halfHeight,
                cornerPoint.y - xAxis.y * halfWidth + yAxis.y * halfHeight};
        case RotatedTextCorner::BottomLeft:
            return Vector2{
                cornerPoint.x + xAxis.x * halfWidth - yAxis.x * halfHeight,
                cornerPoint.y + xAxis.y * halfWidth - yAxis.y * halfHeight};
        case RotatedTextCorner::BottomRight:
        default:
            return Vector2{
                cornerPoint.x - xAxis.x * halfWidth - yAxis.x * halfHeight,
                cornerPoint.y - xAxis.y * halfWidth - yAxis.y * halfHeight};
        }
    }

    Vector2 ComputeDistributedLoadLabelCenter(
        Vector2 referencePoint,
        Vector2 outwardDirection,
        float textHeight,
        float gapPixels)
    {
        const Vector2 outward = NormalizeVectorSafe(outwardDirection);
        return Vector2{
            referencePoint.x + outward.x * (gapPixels + textHeight * 0.5f),
            referencePoint.y + outward.y * (gapPixels + textHeight * 0.5f)};
    }

    float ComputeMomentLabelAdditionalOffset(
        Vector2 textSize,
        Vector2 momentDirection,
        float momentRadius)
    {
        const Vector2 direction = NormalizeVectorSafe(momentDirection);
        if (direction.x == 0.0f && direction.y == 0.0f)
        {
            return 0.0f;
        }

        const float halfWidth = textSize.x * 0.5f;
        const float halfHeight = textSize.y * 0.5f;
        const Vector2 initialLabelCenter = Vector2{
            direction.x * momentRadius,
            direction.y * momentRadius};

        constexpr float directionEpsilon = 1.0e-4f;
        Vector2 innerPoint = initialLabelCenter;

        if (std::abs(direction.x) <= directionEpsilon)
        {
            innerPoint.y += (direction.y >= 0.0f) ? -halfHeight : halfHeight;
        }
        else if (std::abs(direction.y) <= directionEpsilon)
        {
            innerPoint.x += (direction.x >= 0.0f) ? -halfWidth : halfWidth;
        }
        else
        {
            innerPoint.x += (direction.x >= 0.0f) ? -halfWidth : halfWidth;
            innerPoint.y += (direction.y >= 0.0f) ? -halfHeight : halfHeight;
        }

        const float projectionAlongDirection =
            innerPoint.x * direction.x + innerPoint.y * direction.y;
        const float distanceSquared =
            innerPoint.x * innerPoint.x + innerPoint.y * innerPoint.y;
        const float circleRadiusSquared = momentRadius * momentRadius;
        const float discriminant =
            projectionAlongDirection * projectionAlongDirection -
            (distanceSquared - circleRadiusSquared);
        if (discriminant <= 0.0f)
        {
            return 0.0f;
        }

        const float additionalOffset =
            -projectionAlongDirection + sqrtf(discriminant);
        return std::max(0.0f, additionalOffset);
    }

    Vector2 GetTextPositionSizeWithoutGlyphPadding(const Font& font, float fontSize, Vector2 measuredTextSize)
    {
        const float scaleFactor = (font.baseSize > 0) ? (fontSize / static_cast<float>(font.baseSize)) : 1.0f;
        const float verticalPadding = 2.0f * static_cast<float>(font.glyphPadding) * scaleFactor;

        return Vector2{
            measuredTextSize.x,
            std::max(0.0f, measuredTextSize.y - verticalPadding)};
    }

    void DrawMomentLoad(
        float radius,
        float pixelsPerSegment,
        bool anticlockwise,
        float centerX,
        float centerY,
        float angleStartDegrees,
        float angleEndDegrees,
        float thickness,
        float headWidth,
        Color color)
    {
        if (radius <= 1.0e-6f)
        {
            return;
        }

        float headLength = thickness * 5.0f;
        float headAngle = headLength / radius;
        const float thetaStart = angleStartDegrees * DEG2RAD;
        const float thetaEnd = angleEndDegrees * DEG2RAD;

        float totalSweep = thetaEnd - thetaStart;
        if (anticlockwise)
        {
            while (totalSweep > 0.0f)
            {
                totalSweep -= 2.0f * PI;
            }
        }
        else
        {
            while (totalSweep < 0.0f)
            {
                totalSweep += 2.0f * PI;
            }
        }

        const float totalSweepMagnitude = std::abs(totalSweep);
        if (totalSweepMagnitude < headAngle)
        {
            headAngle = totalSweepMagnitude;
        }

        if (totalSweepMagnitude * radius < headLength)
        {
            headWidth = headWidth * totalSweepMagnitude * radius / headLength;
            headLength = totalSweepMagnitude * radius;
        }

        const float sweepSign = anticlockwise ? -1.0f : 1.0f;
        const float visibleSweepMagnitude = std::max(0.0f, totalSweepMagnitude - headAngle);
        const float visibleSweep = sweepSign * visibleSweepMagnitude;
        const float visibleArcLengthPixels = visibleSweepMagnitude * radius;
        int segmentCount = 6;
        if (pixelsPerSegment > 1.0e-6f)
        {
            segmentCount = std::max(6, static_cast<int>(visibleArcLengthPixels / pixelsPerSegment));
        }

        const float halfThickness = thickness * 0.5f;
        const float innerRadius = std::max(0.0f, radius - halfThickness);
        const float outerRadius = radius + halfThickness;
        std::vector<Vector2> stripPoints;
        stripPoints.reserve(static_cast<std::size_t>((segmentCount + 1) * 2));

        for (int segmentIndex = 0; segmentIndex <= segmentCount; ++segmentIndex)
        {
            const float t = static_cast<float>(segmentIndex) / static_cast<float>(segmentCount);
            const float theta = thetaStart + visibleSweep * t;
            const float cosTheta = cosf(theta);
            const float sinTheta = sinf(theta);

            if (anticlockwise)
            {
                stripPoints.push_back(Vector2{
                    centerX + innerRadius * cosTheta,
                    centerY + innerRadius * sinTheta});
                stripPoints.push_back(Vector2{
                    centerX + outerRadius * cosTheta,
                    centerY + outerRadius * sinTheta});
            }
            else
            {
                stripPoints.push_back(Vector2{
                    centerX + outerRadius * cosTheta,
                    centerY + outerRadius * sinTheta});
                stripPoints.push_back(Vector2{
                    centerX + innerRadius * cosTheta,
                    centerY + innerRadius * sinTheta});
            }
        }

        if (!stripPoints.empty())
        {
            DrawTriangleStrip(stripPoints.data(), static_cast<int>(stripPoints.size()), color);
        }

        const Vector2 arcEndPoint = Vector2{
            centerX + radius * cosf(thetaEnd),
            centerY + radius * sinf(thetaEnd)};

        Vector2 arrowTip = Vector2{0.0f, 0.0f};
        Vector2 arrowPointA = Vector2{arrowTip.x - headLength, arrowTip.y - headWidth};
        Vector2 arrowPointB = Vector2{arrowTip.x - headLength, arrowTip.y + headWidth};

        const float headMidAngle = thetaEnd - sweepSign * headAngle * 0.5f;
        const Vector2 tangentDirection = anticlockwise
            ? Vector2{sinf(headMidAngle), -cosf(headMidAngle)}
            : Vector2{-sinf(headMidAngle), cosf(headMidAngle)};
        const float arrowRotation = atan2f(tangentDirection.y, tangentDirection.x);

        arrowPointA = RotatePointAround(arrowPointA, arrowTip, arrowRotation);
        arrowPointB = RotatePointAround(arrowPointB, arrowTip, arrowRotation);

        arrowTip = Vector2{arrowTip.x + arcEndPoint.x, arrowTip.y + arcEndPoint.y};
        arrowPointA = Vector2{arrowPointA.x + arcEndPoint.x, arrowPointA.y + arcEndPoint.y};
        arrowPointB = Vector2{arrowPointB.x + arcEndPoint.x, arrowPointB.y + arcEndPoint.y};
        DrawTriangle(arrowTip, arrowPointA, arrowPointB, color);
    }

    Vector2 ChooseHorizontalForcePlacementDirection(
        const ProjectDerivedData::NodeConnectivity& connectivity,
        float loadDirectionX)
    {
        const unsigned char occupiedMask = connectivity.occupiedDirectionMask;
        const bool preferLeft = loadDirectionX > 0.0f;

        if (preferLeft)
        {
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

        if ((occupiedMask & NodeOccupiedDirectionRight) == 0)
        {
            return Vector2{1.0f, 0.0f};
        }

        if ((occupiedMask & NodeOccupiedDirectionLeft) == 0)
        {
            return Vector2{-1.0f, 0.0f};
        }

        return Vector2{1.0f, 0.0f};
    }

    Vector2 ChooseVerticalForcePlacementDirection(
        const ProjectDerivedData::NodeConnectivity& connectivity,
        float loadDirectionY)
    {
        const unsigned char occupiedMask = connectivity.occupiedDirectionMask;
        const bool preferDown = loadDirectionY < 0.0f;

        if (preferDown)
        {
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

        if ((occupiedMask & NodeOccupiedDirectionUp) == 0)
        {
            return Vector2{0.0f, -1.0f};
        }

        if ((occupiedMask & NodeOccupiedDirectionDown) == 0)
        {
            return Vector2{0.0f, 1.0f};
        }

        return Vector2{0.0f, -1.0f};
    }

    float ComputeOccupiedDirectionScore(
        const ProjectDerivedData::NodeConnectivity& connectivity,
        Vector2 direction)
    {
        direction = NormalizeVectorSafe(direction);
        const unsigned char occupiedMask = connectivity.occupiedDirectionMask;
        float score = 0.0f;

        if (direction.x > 1.0e-4f && (occupiedMask & NodeOccupiedDirectionRight) != 0)
        {
            score += std::abs(direction.x);
        }
        else if (direction.x < -1.0e-4f && (occupiedMask & NodeOccupiedDirectionLeft) != 0)
        {
            score += std::abs(direction.x);
        }

        if (direction.y > 1.0e-4f && (occupiedMask & NodeOccupiedDirectionDown) != 0)
        {
            score += std::abs(direction.y);
        }
        else if (direction.y < -1.0e-4f && (occupiedMask & NodeOccupiedDirectionUp) != 0)
        {
            score += std::abs(direction.y);
        }

        return score;
    }

    Vector2 ChooseMomentArcDirection(const ProjectDerivedData::NodeConnectivity& connectivity)
    {
        constexpr double kMinimumReliableAxisStrength = 0.9;

        if (connectivity.hasAverageConnectedBeamDirection &&
            connectivity.preferredConnectedBeamDirectionStrength > 1.0e-4)
        {
            Vector2 preferredDirection = NormalizeVectorSafe(
                Vector2{
                    static_cast<float>(-connectivity.averageConnectedBeamDirection.x),
                    static_cast<float>(-connectivity.averageConnectedBeamDirection.y)});
            if (preferredDirection.x != 0.0f || preferredDirection.y != 0.0f)
            {
                return preferredDirection;
            }
        }

        if (connectivity.hasAverageConnectedBeamAxis &&
            connectivity.averageConnectedBeamAxisStrength >= kMinimumReliableAxisStrength)
        {
            const Vector2 axisDirection = NormalizeVectorSafe(
                Vector2{
                    static_cast<float>(connectivity.averageConnectedBeamAxis.x),
                    static_cast<float>(connectivity.averageConnectedBeamAxis.y)});

            const Vector2 candidateA = NormalizeVectorSafe(
                Vector2{-axisDirection.y, axisDirection.x});
            const Vector2 candidateB = Vector2{-candidateA.x, -candidateA.y};

            const float scoreA = ComputeOccupiedDirectionScore(connectivity, candidateA);
            const float scoreB = ComputeOccupiedDirectionScore(connectivity, candidateB);

            if (std::abs(scoreA - scoreB) <= 1.0e-4f)
            {
                if (candidateA.y < candidateB.y - 1.0e-4f)
                {
                    return candidateA;
                }

                if (candidateB.y < candidateA.y - 1.0e-4f)
                {
                    return candidateB;
                }

                return candidateA.x >= candidateB.x ? candidateA : candidateB;
            }

            return scoreA < scoreB ? candidateA : candidateB;
        }

        const Vector2 candidates[] = {
            Vector2{0.0f, -1.0f},
            Vector2{1.0f, 0.0f},
            Vector2{-1.0f, 0.0f},
            Vector2{0.0f, 1.0f}};

        float bestScore = std::numeric_limits<float>::max();
        Vector2 bestDirection = Vector2{0.0f, -1.0f};

        for (const Vector2& candidate : candidates)
        {
            const float score = ComputeOccupiedDirectionScore(connectivity, candidate);
            if (score < bestScore - 1.0e-4f)
            {
                bestScore = score;
                bestDirection = candidate;
            }
        }

        return bestDirection;
    }

    void ComputeAnchoredPointLoadSegment(
        Vector2 screenCenter,
        Vector2 placementDirection,
        Vector2 loadDirection,
        float anchorOffsetPixels,
        float loadLengthPixels,
        Vector2& outTail,
        Vector2& outTip)
    {
        const Vector2 anchorPoint = Vector2{
            screenCenter.x + placementDirection.x * anchorOffsetPixels,
            screenCenter.y + placementDirection.y * anchorOffsetPixels};
        const bool startsAtNode =
            placementDirection.x * loadDirection.x + placementDirection.y * loadDirection.y > 0.0f;

        if (startsAtNode)
        {
            outTail = anchorPoint;
            outTip = Vector2{
                anchorPoint.x + loadDirection.x * loadLengthPixels,
                anchorPoint.y + loadDirection.y * loadLengthPixels};
        }
        else
        {
            outTip = anchorPoint;
            outTail = Vector2{
                anchorPoint.x - loadDirection.x * loadLengthPixels,
                anchorPoint.y - loadDirection.y * loadLengthPixels};
        }
    }

    void ComputeLoadLabelReferenceSegment(
        Vector2 nodeAnchor,
        Vector2 directionTowardNode,
        float referenceLengthPixels,
        Vector2& outReferenceTail,
        Vector2& outReferenceTip)
    {
        outReferenceTail = Vector2{
            nodeAnchor.x - directionTowardNode.x * referenceLengthPixels,
            nodeAnchor.y - directionTowardNode.y * referenceLengthPixels};
        outReferenceTip = nodeAnchor;
    }
}

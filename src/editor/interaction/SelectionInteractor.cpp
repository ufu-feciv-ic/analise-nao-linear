#include "editor/interaction/SelectionInteractor.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "utils/UnitConversion.h"

namespace
{
constexpr float kDimensionGapPixels = 18.0f;
constexpr float kDimensionTextOffsetPixels = 12.0f;
constexpr float kDimensionTextFontSize = 18.0f;

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

std::string FormatDisplayScalar(double value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}

std::string FormatDisplayLength(double valueInMeters, LengthUnit unit)
{
    return FormatDisplayScalar(UnitConversion::LengthToDisplay(valueInMeters, unit)) +
           " " +
           UnitConversion::GetLengthUnitLabel(unit);
}

bool TryBuildDimensionScreenGeometry(
    Vector2 startScreen,
    Vector2 endScreen,
    DimensionType type,
    double offsetPixels,
    Vector2& lineStart,
    Vector2& lineEnd,
    Vector2& labelCenter)
{
    const float clampedOffsetPixels =
        static_cast<float>(offsetPixels >= 0.0
                               ? std::max(offsetPixels, static_cast<double>(kDimensionGapPixels))
                               : std::min(offsetPixels, static_cast<double>(-kDimensionGapPixels)));

    Vector2 outsideNormal{0.0f, 0.0f};
    switch (type)
    {
    case DimensionType::Horizontal:
    {
        const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
        const float anchorY =
            (sideSign < 0.0f ? std::min(startScreen.y, endScreen.y) : std::max(startScreen.y, endScreen.y)) +
            clampedOffsetPixels;
        lineStart = Vector2{startScreen.x, anchorY};
        lineEnd = Vector2{endScreen.x, anchorY};
        outsideNormal = Vector2{0.0f, sideSign};
        break;
    }

    case DimensionType::Vertical:
    {
        const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
        const float anchorX =
            (sideSign < 0.0f ? std::min(startScreen.x, endScreen.x) : std::max(startScreen.x, endScreen.x)) +
            clampedOffsetPixels;
        lineStart = Vector2{anchorX, startScreen.y};
        lineEnd = Vector2{anchorX, endScreen.y};
        outsideNormal = Vector2{sideSign, 0.0f};
        break;
    }

    case DimensionType::Aligned:
    default:
    {
        const Vector2 screenSegment = Vector2{endScreen.x - startScreen.x, endScreen.y - startScreen.y};
        const float segmentLength = std::sqrt(screenSegment.x * screenSegment.x + screenSegment.y * screenSegment.y);
        if (segmentLength <= 1.0e-6f)
        {
            return false;
        }

        const Vector2 lineDirection = Vector2{screenSegment.x / segmentLength, screenSegment.y / segmentLength};
        const Vector2 rawNormal = Vector2{-lineDirection.y, lineDirection.x};
        const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
        outsideNormal = Vector2{rawNormal.x * sideSign, rawNormal.y * sideSign};
        const Vector2 offsetVector = Vector2{
            outsideNormal.x * std::fabs(clampedOffsetPixels),
            outsideNormal.y * std::fabs(clampedOffsetPixels)};

        lineStart = Vector2{startScreen.x + offsetVector.x, startScreen.y + offsetVector.y};
        lineEnd = Vector2{endScreen.x + offsetVector.x, endScreen.y + offsetVector.y};
        break;
    }
    }

    labelCenter = Vector2{
        (lineStart.x + lineEnd.x) * 0.5f + outsideNormal.x * kDimensionTextOffsetPixels,
        (lineStart.y + lineEnd.y) * 0.5f + outsideNormal.y * kDimensionTextOffsetPixels};
    return true;
}

bool DoesLineCrossRectangle(Vector2 a, Vector2 b, Rectangle rect)
{
    if (CheckCollisionPointRec(a, rect) || CheckCollisionPointRec(b, rect))
    {
        return true;
    }

    Vector2 collisionPoint{};
    const Vector2 topLeft = Vector2{rect.x, rect.y};
    const Vector2 topRight = Vector2{rect.x + rect.width, rect.y};
    const Vector2 bottomRight = Vector2{rect.x + rect.width, rect.y + rect.height};
    const Vector2 bottomLeft = Vector2{rect.x, rect.y + rect.height};

    return CheckCollisionLines(a, b, topLeft, topRight, &collisionPoint) ||
           CheckCollisionLines(a, b, topRight, bottomRight, &collisionPoint) ||
           CheckCollisionLines(a, b, bottomRight, bottomLeft, &collisionPoint) ||
           CheckCollisionLines(a, b, bottomLeft, topLeft, &collisionPoint);
}
}

namespace SelectionInteractor
{
Rectangle GetSelectionRectangle(const Camera2D& camera, const SelectionState& selectionState)
{
    const Vector2 startScreen = GetWorldToScreen2D(selectionState.boxSelectStartWorld, camera);
    const Vector2 endScreen = GetWorldToScreen2D(selectionState.boxSelectEndWorld, camera);

    const float x = fminf(startScreen.x, endScreen.x);
    const float y = fminf(startScreen.y, endScreen.y);
    const float width = fabsf(endScreen.x - startScreen.x);
    const float height = fabsf(endScreen.y - startScreen.y);

    return Rectangle{x, y, width, height};
}

bool IsRightToLeftBoxSelection(const Camera2D& camera, const SelectionState& selectionState)
{
    const Vector2 startScreen = GetWorldToScreen2D(selectionState.boxSelectStartWorld, camera);
    const Vector2 endScreen = GetWorldToScreen2D(selectionState.boxSelectEndWorld, camera);
    return endScreen.x < startScreen.x;
}

bool IsBeamInsideSelectionRectangle(
    const ProjectDocument& document,
    const Camera2D& camera,
    const Beam& beam,
    Rectangle selectionRect)
{
    const Node* startNode = document.FindNodeById(beam.startNodeId);
    const Node* endNode = document.FindNodeById(beam.endNodeId);
    if (startNode == nullptr || endNode == nullptr)
    {
        return false;
    }

    const Vector2 startScreen = GetWorldToScreen2D(
        Vector2{static_cast<float>(startNode->position.x), static_cast<float>(startNode->position.y)},
        camera);
    const Vector2 endScreen = GetWorldToScreen2D(
        Vector2{static_cast<float>(endNode->position.x), static_cast<float>(endNode->position.y)},
        camera);

    return CheckCollisionPointRec(startScreen, selectionRect) &&
           CheckCollisionPointRec(endScreen, selectionRect);
}

bool DoesBeamCrossSelectionRectangle(
    const ProjectDocument& document,
    const Camera2D& camera,
    const Beam& beam,
    Rectangle selectionRect)
{
    const Node* startNode = document.FindNodeById(beam.startNodeId);
    const Node* endNode = document.FindNodeById(beam.endNodeId);
    if (startNode == nullptr || endNode == nullptr)
    {
        return false;
    }

    const Vector2 startScreen = GetWorldToScreen2D(
        Vector2{static_cast<float>(startNode->position.x), static_cast<float>(startNode->position.y)},
        camera);
    const Vector2 endScreen = GetWorldToScreen2D(
        Vector2{static_cast<float>(endNode->position.x), static_cast<float>(endNode->position.y)},
        camera);

    if (CheckCollisionPointRec(startScreen, selectionRect) ||
        CheckCollisionPointRec(endScreen, selectionRect))
    {
        return true;
    }

    Vector2 collisionPoint{};
    const Vector2 topLeft = Vector2{selectionRect.x, selectionRect.y};
    const Vector2 topRight = Vector2{selectionRect.x + selectionRect.width, selectionRect.y};
    const Vector2 bottomRight = Vector2{selectionRect.x + selectionRect.width, selectionRect.y + selectionRect.height};
    const Vector2 bottomLeft = Vector2{selectionRect.x, selectionRect.y + selectionRect.height};

    return CheckCollisionLines(startScreen, endScreen, topLeft, topRight, &collisionPoint) ||
           CheckCollisionLines(startScreen, endScreen, topRight, bottomRight, &collisionPoint) ||
           CheckCollisionLines(startScreen, endScreen, bottomRight, bottomLeft, &collisionPoint) ||
           CheckCollisionLines(startScreen, endScreen, bottomLeft, topLeft, &collisionPoint);
}

bool DoesDimensionCrossSelectionRectangle(
    const ProjectDocument& document,
    const Camera2D& camera,
    const Dimension& dimension,
    Rectangle selectionRect)
{
    const Node* startNode = document.FindNodeById(dimension.startNodeId);
    const Node* endNode = document.FindNodeById(dimension.endNodeId);
    if (startNode == nullptr || endNode == nullptr)
    {
        return false;
    }

    const Vector2 startWorld = Vector2{
        static_cast<float>(startNode->position.x),
        static_cast<float>(startNode->position.y)};
    const Vector2 endWorld = Vector2{
        static_cast<float>(endNode->position.x),
        static_cast<float>(endNode->position.y)};
    const Vector2 startScreen = GetWorldToScreen2D(startWorld, camera);
    const Vector2 endScreen = GetWorldToScreen2D(endWorld, camera);

    Vector2 lineStart{};
    Vector2 lineEnd{};
    Vector2 labelCenter{};
    const bool hasGeometry = TryBuildDimensionScreenGeometry(
        startScreen,
        endScreen,
        dimension.type,
        GetDimensionOffsetPixelsForDisplay(
            dimension.offsetMode,
            dimension.offsetPixels,
            dimension.offsetWorld,
            camera.zoom),
        lineStart,
        lineEnd,
        labelCenter);
    if (!hasGeometry)
    {
        return false;
    }

    if (DoesLineCrossRectangle(lineStart, lineEnd, selectionRect))
    {
        return true;
    }

    const Vector2 lineDirection = Vector2{lineEnd.x - lineStart.x, lineEnd.y - lineStart.y};
    const float lineLength = std::sqrt(lineDirection.x * lineDirection.x + lineDirection.y * lineDirection.y);
    if (lineLength <= 1.0e-6f)
    {
        return CheckCollisionPointRec(labelCenter, selectionRect);
    }

    const Vector2 tangent = Vector2{lineDirection.x / lineLength, lineDirection.y / lineLength};
    const Vector2 normal = Vector2{-tangent.y, tangent.x};
    const std::string labelText = FormatDisplayLength(
        std::sqrt(
            static_cast<double>(endWorld.x - startWorld.x) * static_cast<double>(endWorld.x - startWorld.x) +
            static_cast<double>(endWorld.y - startWorld.y) * static_cast<double>(endWorld.y - startWorld.y)),
        dimension.lengthUnit);
    const Vector2 labelSize = MeasureTextEx(GetFontDefault(), labelText.c_str(), kDimensionTextFontSize, 0.0f);
    const Vector2 halfTangent = Vector2{tangent.x * labelSize.x * 0.5f, tangent.y * labelSize.x * 0.5f};
    const Vector2 halfNormal = Vector2{normal.x * labelSize.y * 0.5f, normal.y * labelSize.y * 0.5f};

    const Vector2 textCorners[4] = {
        Vector2{labelCenter.x - halfTangent.x - halfNormal.x, labelCenter.y - halfTangent.y - halfNormal.y},
        Vector2{labelCenter.x + halfTangent.x - halfNormal.x, labelCenter.y + halfTangent.y - halfNormal.y},
        Vector2{labelCenter.x + halfTangent.x + halfNormal.x, labelCenter.y + halfTangent.y + halfNormal.y},
        Vector2{labelCenter.x - halfTangent.x + halfNormal.x, labelCenter.y - halfTangent.y + halfNormal.y}};

    for (const Vector2 corner : textCorners)
    {
        if (CheckCollisionPointRec(corner, selectionRect))
        {
            return true;
        }
    }

    return DoesLineCrossRectangle(textCorners[0], textCorners[1], selectionRect) ||
           DoesLineCrossRectangle(textCorners[1], textCorners[2], selectionRect) ||
           DoesLineCrossRectangle(textCorners[2], textCorners[3], selectionRect) ||
           DoesLineCrossRectangle(textCorners[3], textCorners[0], selectionRect);
}
}

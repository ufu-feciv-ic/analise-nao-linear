#include "editor/ops/DimensionOperations.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
constexpr double kGeometryTolerance = 0.00001;

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

double GetDimensionOffsetWorldFromPixels(double offsetPixels, float cameraZoom)
{
    if (cameraZoom <= 0.0f)
    {
        return 0.0;
    }

    return offsetPixels / static_cast<double>(cameraZoom);
}

bool TryComputeSignedDimensionOffset(
    Vector2 startScreen,
    Vector2 endScreen,
    Vector2 mouseScreenPosition,
    DimensionType dimensionType,
    double& signedOffsetPixels)
{
    constexpr double minimumGapPixels = 18.0;

    const Vector2 midpoint = Vector2{
        (startScreen.x + endScreen.x) * 0.5f,
        (startScreen.y + endScreen.y) * 0.5f};

    switch (dimensionType)
    {
    case DimensionType::Horizontal:
    {
        const double sign = mouseScreenPosition.y < midpoint.y ? -1.0 : 1.0;
        const double referenceY =
            sign < 0.0 ? std::min(startScreen.y, endScreen.y) : std::max(startScreen.y, endScreen.y);
        signedOffsetPixels = static_cast<double>(mouseScreenPosition.y) - referenceY;
        if (std::abs(signedOffsetPixels) < minimumGapPixels)
        {
            signedOffsetPixels = sign * minimumGapPixels;
        }
        return true;
    }

    case DimensionType::Vertical:
    {
        const double sign = mouseScreenPosition.x < midpoint.x ? -1.0 : 1.0;
        const double referenceX =
            sign < 0.0 ? std::min(startScreen.x, endScreen.x) : std::max(startScreen.x, endScreen.x);
        signedOffsetPixels = static_cast<double>(mouseScreenPosition.x) - referenceX;
        if (std::abs(signedOffsetPixels) < minimumGapPixels)
        {
            signedOffsetPixels = sign * minimumGapPixels;
        }
        return true;
    }

    case DimensionType::Aligned:
    default:
    {
        const Vector2 segment = Subtract(endScreen, startScreen);
        const double segmentLengthSquared = LengthSquared(segment);
        if (segmentLengthSquared <= kGeometryTolerance)
        {
            return false;
        }

        const double segmentLength = sqrt(segmentLengthSquared);
        const Vector2 normal = Vector2{
            -segment.y / static_cast<float>(segmentLength),
            segment.x / static_cast<float>(segmentLength)};
        signedOffsetPixels = DotProduct(Subtract(mouseScreenPosition, midpoint), normal);
        if (std::abs(signedOffsetPixels) < minimumGapPixels)
        {
            signedOffsetPixels = (signedOffsetPixels < 0.0 ? -1.0 : 1.0) * minimumGapPixels;
        }
        return true;
    }
    }
}

DimensionType DetermineDimensionTypeFromMouse(
    Vector2 startScreen,
    Vector2 endScreen,
    Vector2 mouseScreenPosition)
{
    constexpr float axisLockThresholdPixels = 1.0f;

    const Vector2 segment = Subtract(endScreen, startScreen);
    if (fabsf(segment.y) <= axisLockThresholdPixels)
    {
        return DimensionType::Horizontal;
    }

    if (fabsf(segment.x) <= axisLockThresholdPixels)
    {
        return DimensionType::Vertical;
    }

    const Vector2 midpoint = Vector2{
        (startScreen.x + endScreen.x) * 0.5f,
        (startScreen.y + endScreen.y) * 0.5f};
    Vector2 toMouse = Subtract(mouseScreenPosition, midpoint);
    const double toMouseLengthSquared = LengthSquared(toMouse);
    if (toMouseLengthSquared <= kGeometryTolerance)
    {
        return DimensionType::Aligned;
    }

    const double toMouseLength = sqrt(toMouseLengthSquared);
    toMouse.x /= static_cast<float>(toMouseLength);
    toMouse.y /= static_cast<float>(toMouseLength);

    const double segmentLengthSquared = LengthSquared(segment);
    Vector2 alignedNormal{0.0f, -1.0f};
    if (segmentLengthSquared > kGeometryTolerance)
    {
        const double segmentLength = sqrt(segmentLengthSquared);
        alignedNormal = Vector2{
            -segment.y / static_cast<float>(segmentLength),
            segment.x / static_cast<float>(segmentLength)};
    }

    const double horizontalScore = std::abs(DotProduct(toMouse, Vector2{0.0f, 1.0f}));
    const double verticalScore = std::abs(DotProduct(toMouse, Vector2{1.0f, 0.0f}));
    const double alignedScore = std::abs(DotProduct(toMouse, alignedNormal));

    if (horizontalScore >= verticalScore && horizontalScore >= alignedScore)
    {
        return DimensionType::Horizontal;
    }

    if (verticalScore >= horizontalScore && verticalScore >= alignedScore)
    {
        return DimensionType::Vertical;
    }

    return DimensionType::Aligned;
}

bool ArePointsCollinear(Vector2 a, Vector2 b, Vector2 c)
{
    const Vector2 ab = Subtract(b, a);
    const Vector2 ac = Subtract(c, a);
    const double areaTwice = std::abs(CrossProduct(ab, ac));
    const double length = sqrt(std::max(LengthSquared(ab), LengthSquared(ac)));
    if (length <= kGeometryTolerance)
    {
        return false;
    }

    return areaTwice <= kGeometryTolerance * length;
}

constexpr float dimensionGapPixelsForSelection = 18.0f;
constexpr float dimensionNodeClearancePixelsForSelection = 10.0f;
constexpr float dimensionExtensionOvershootPixelsForSelection = 10.0f;

struct ScreenDimensionSegments
{
    Vector2 startExtensionStart{};
    Vector2 startExtensionEnd{};
    Vector2 endExtensionStart{};
    Vector2 endExtensionEnd{};
    Vector2 lineStart{};
    Vector2 lineEnd{};
};

bool TryBuildDimensionScreenSegments(
    Vector2 startScreen,
    Vector2 endScreen,
    DimensionType type,
    double offsetPixels,
    ScreenDimensionSegments& segments)
{
    const float clampedOffsetPixels =
        static_cast<float>(offsetPixels >= 0.0
                               ? std::max(offsetPixels, static_cast<double>(dimensionGapPixelsForSelection))
                               : std::min(offsetPixels, static_cast<double>(-dimensionGapPixelsForSelection)));

    switch (type)
    {
    case DimensionType::Horizontal:
    {
        const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
        const float anchorY =
            (sideSign < 0.0f ? std::min(startScreen.y, endScreen.y) : std::max(startScreen.y, endScreen.y)) +
            clampedOffsetPixels;
        const float overshootY = anchorY + sideSign * dimensionExtensionOvershootPixelsForSelection;

        segments.startExtensionStart = Vector2{
            startScreen.x,
            startScreen.y + sideSign * dimensionNodeClearancePixelsForSelection};
        segments.startExtensionEnd = Vector2{startScreen.x, overshootY};
        segments.endExtensionStart = Vector2{
            endScreen.x,
            endScreen.y + sideSign * dimensionNodeClearancePixelsForSelection};
        segments.endExtensionEnd = Vector2{endScreen.x, overshootY};
        segments.lineStart = Vector2{startScreen.x, anchorY};
        segments.lineEnd = Vector2{endScreen.x, anchorY};
        return true;
    }

    case DimensionType::Vertical:
    {
        const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
        const float anchorX =
            (sideSign < 0.0f ? std::min(startScreen.x, endScreen.x) : std::max(startScreen.x, endScreen.x)) +
            clampedOffsetPixels;
        const float overshootX = anchorX + sideSign * dimensionExtensionOvershootPixelsForSelection;

        segments.startExtensionStart = Vector2{
            startScreen.x + sideSign * dimensionNodeClearancePixelsForSelection,
            startScreen.y};
        segments.startExtensionEnd = Vector2{overshootX, startScreen.y};
        segments.endExtensionStart = Vector2{
            endScreen.x + sideSign * dimensionNodeClearancePixelsForSelection,
            endScreen.y};
        segments.endExtensionEnd = Vector2{overshootX, endScreen.y};
        segments.lineStart = Vector2{anchorX, startScreen.y};
        segments.lineEnd = Vector2{anchorX, endScreen.y};
        return true;
    }

    case DimensionType::Aligned:
    default:
    {
        const Vector2 screenSegment = Vector2{endScreen.x - startScreen.x, endScreen.y - startScreen.y};
        const float length = sqrtf(screenSegment.x * screenSegment.x + screenSegment.y * screenSegment.y);
        if (length <= 1.0e-6f)
        {
            return false;
        }

        const float sideSign = clampedOffsetPixels < 0.0f ? -1.0f : 1.0f;
        const Vector2 lineDirection = Vector2{screenSegment.x / length, screenSegment.y / length};
        const Vector2 outsideNormal = Vector2{-lineDirection.y * sideSign, lineDirection.x * sideSign};
        const Vector2 offsetVector = Vector2{
            outsideNormal.x * fabsf(clampedOffsetPixels),
            outsideNormal.y * fabsf(clampedOffsetPixels)};
        const Vector2 clearanceVector = Vector2{
            outsideNormal.x * dimensionNodeClearancePixelsForSelection,
            outsideNormal.y * dimensionNodeClearancePixelsForSelection};
        const Vector2 overshootVector = Vector2{
            outsideNormal.x * dimensionExtensionOvershootPixelsForSelection,
            outsideNormal.y * dimensionExtensionOvershootPixelsForSelection};

        segments.startExtensionStart = Vector2{startScreen.x + clearanceVector.x, startScreen.y + clearanceVector.y};
        segments.endExtensionStart = Vector2{endScreen.x + clearanceVector.x, endScreen.y + clearanceVector.y};
        segments.lineStart = Vector2{startScreen.x + offsetVector.x, startScreen.y + offsetVector.y};
        segments.lineEnd = Vector2{endScreen.x + offsetVector.x, endScreen.y + offsetVector.y};
        segments.startExtensionEnd = Vector2{segments.lineStart.x + overshootVector.x, segments.lineStart.y + overshootVector.y};
        segments.endExtensionEnd = Vector2{segments.lineEnd.x + overshootVector.x, segments.lineEnd.y + overshootVector.y};
        return true;
    }
    }
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

} // namespace

namespace DimensionOperations
{
bool TryAddDimensionBetweenNodes(
    ProjectDocument& document,
    int startNodeId,
    int endNodeId,
    DimensionType type,
    LengthUnit lengthUnit,
    DimensionOffsetMode offsetMode,
    double offsetPixels,
    double offsetWorld)
{
    const Node* startNode = document.FindNodeById(startNodeId);
    const Node* endNode = document.FindNodeById(endNodeId);
    if (startNode == nullptr || endNode == nullptr || startNodeId == endNodeId)
    {
        return false;
    }

    if (std::abs(offsetPixels) <= kGeometryTolerance)
    {
        return false;
    }

    return document.AddDimension(
               startNodeId,
               endNodeId,
               type,
               lengthUnit,
               offsetMode,
               offsetPixels,
               offsetWorld) >= 0;
}

bool UpdateDimensionPreviewFromMouse(
    const ProjectDocument& document,
    const Camera2D& camera,
    Vector2 previewScreenPosition,
    int startNodeId,
    int endNodeId,
    DimensionType& type,
    double& offsetPixels,
    double& offsetWorld)
{
    const Node* startNode = document.FindNodeById(startNodeId);
    const Node* endNode = document.FindNodeById(endNodeId);
    if (startNode == nullptr || endNode == nullptr || startNodeId == endNodeId)
    {
        return false;
    }

    const Vector2 startScreen = GetWorldToScreen2D(
        Vector2{
            static_cast<float>(startNode->position.x),
            static_cast<float>(startNode->position.y)},
        camera);
    const Vector2 endScreen = GetWorldToScreen2D(
        Vector2{
            static_cast<float>(endNode->position.x),
            static_cast<float>(endNode->position.y)},
        camera);

    type = DetermineDimensionTypeFromMouse(startScreen, endScreen, previewScreenPosition);
    const bool computed = TryComputeSignedDimensionOffset(
        startScreen,
        endScreen,
        previewScreenPosition,
        type,
        offsetPixels);
    if (computed)
    {
        offsetWorld = GetDimensionOffsetWorldFromPixels(offsetPixels, camera.zoom);
    }

    return computed;
}

bool TryAlignDimensionToReference(
    const ProjectDocument& document,
    const Camera2D& camera,
    int startNodeId,
    int endNodeId,
    const Dimension& referenceDimension,
    DimensionType& type,
    double& offsetPixels,
    double& offsetWorld)
{
    const Node* startNode = document.FindNodeById(startNodeId);
    const Node* endNode = document.FindNodeById(endNodeId);
    const Node* referenceStartNode = document.FindNodeById(referenceDimension.startNodeId);
    const Node* referenceEndNode = document.FindNodeById(referenceDimension.endNodeId);
    if (startNode == nullptr || endNode == nullptr ||
        referenceStartNode == nullptr || referenceEndNode == nullptr ||
        startNodeId == endNodeId)
    {
        return false;
    }

    const Vector2 startScreen = GetWorldToScreen2D(
        Vector2{
            static_cast<float>(startNode->position.x),
            static_cast<float>(startNode->position.y)},
        camera);
    const Vector2 endScreen = GetWorldToScreen2D(
        Vector2{
            static_cast<float>(endNode->position.x),
            static_cast<float>(endNode->position.y)},
        camera);
    const Vector2 referenceStartScreen = GetWorldToScreen2D(
        Vector2{
            static_cast<float>(referenceStartNode->position.x),
            static_cast<float>(referenceStartNode->position.y)},
        camera);
    const Vector2 referenceEndScreen = GetWorldToScreen2D(
        Vector2{
            static_cast<float>(referenceEndNode->position.x),
            static_cast<float>(referenceEndNode->position.y)},
        camera);

    ScreenDimensionSegments referenceSegments;
    if (!TryBuildDimensionScreenSegments(
            referenceStartScreen,
            referenceEndScreen,
            referenceDimension.type,
            GetDimensionOffsetPixelsForDisplay(
                referenceDimension.offsetMode,
                referenceDimension.offsetPixels,
                referenceDimension.offsetWorld,
                camera.zoom),
            referenceSegments))
    {
        return false;
    }

    type = referenceDimension.type;

    switch (type)
    {
    case DimensionType::Horizontal:
    {
        const float referenceY = referenceSegments.lineStart.y;
        const float minY = std::min(startScreen.y, endScreen.y);
        const float maxY = std::max(startScreen.y, endScreen.y);
        const float midpointY = (minY + maxY) * 0.5f;
        offsetPixels = static_cast<double>(
            referenceY <= midpointY
                ? referenceY - minY
                : referenceY - maxY);
        break;
    }

    case DimensionType::Vertical:
    {
        const float referenceX = referenceSegments.lineStart.x;
        const float minX = std::min(startScreen.x, endScreen.x);
        const float maxX = std::max(startScreen.x, endScreen.x);
        const float midpointX = (minX + maxX) * 0.5f;
        offsetPixels = static_cast<double>(
            referenceX <= midpointX
                ? referenceX - minX
                : referenceX - maxX);
        break;
    }

    case DimensionType::Aligned:
    default:
    {
        const Vector2 targetSegment = Vector2{endScreen.x - startScreen.x, endScreen.y - startScreen.y};
        const float targetLength = sqrtf(targetSegment.x * targetSegment.x + targetSegment.y * targetSegment.y);
        if (targetLength <= 1.0e-6f)
        {
            return false;
        }

        const Vector2 targetDirection = Vector2{targetSegment.x / targetLength, targetSegment.y / targetLength};
        const Vector2 targetNormal = Vector2{-targetDirection.y, targetDirection.x};
        const Vector2 targetMidpoint = Vector2{
            (startScreen.x + endScreen.x) * 0.5f,
            (startScreen.y + endScreen.y) * 0.5f};
        const Vector2 referencePoint = referenceSegments.lineStart;
        offsetPixels = DotProduct(
            Vector2{referencePoint.x - targetMidpoint.x, referencePoint.y - targetMidpoint.y},
            targetNormal);
        break;
    }
    }

    offsetWorld = GetDimensionOffsetWorldFromPixels(offsetPixels, camera.zoom);
    return true;
}

bool IsNodeCollinearWithDimensionChain(
    const ProjectDocument& document,
    const EditorState& state,
    int candidateNodeId)
{
    if (state.dimensionTool.dimensionChainReferenceNodeId < 0 || state.dimensionTool.dimensionStartNodeId < 0 || candidateNodeId < 0)
    {
        return false;
    }

    const Node* firstNode = document.FindNodeById(state.dimensionTool.dimensionChainReferenceNodeId);
    const Node* secondNode = document.FindNodeById(state.dimensionTool.dimensionStartNodeId);
    const Node* candidateNode = document.FindNodeById(candidateNodeId);
    if (firstNode == nullptr || secondNode == nullptr || candidateNode == nullptr)
    {
        return false;
    }

    return ArePointsCollinear(
        Vector2{
            static_cast<float>(firstNode->position.x),
            static_cast<float>(firstNode->position.y)},
        Vector2{
            static_cast<float>(secondNode->position.x),
            static_cast<float>(secondNode->position.y)},
        Vector2{
            static_cast<float>(candidateNode->position.x),
            static_cast<float>(candidateNode->position.y)});
}

bool TryBeginDimensionCreationFromBeam(
    const ProjectDocument& document,
    EditorState& state,
    const Camera2D& camera,
    const Beam& beam)
{
    if (beam.startNodeId < 0 || beam.endNodeId < 0 || beam.startNodeId == beam.endNodeId)
    {
        return false;
    }

    const Node* startNode = document.FindNodeById(beam.startNodeId);
    const Node* endNode = document.FindNodeById(beam.endNodeId);
    if (startNode == nullptr || endNode == nullptr)
    {
        return false;
    }

    state.dimensionTool.dimensionStartNodeId = beam.startNodeId;
    state.dimensionTool.dimensionEndNodeId = beam.endNodeId;
    state.dimensionTool.dimensionChainReferenceNodeId = -1;
    state.dimensionTool.dimensionLengthUnit = state.dimensionTool.newDimensionLengthUnit;
    state.dimensionTool.dimensionType = DimensionType::Aligned;
    state.dimensionTool.dimensionOffsetMode = state.dimensionTool.newDimensionOffsetMode;
    state.dimensionTool.dimensionOffsetPixels = 32.0;
    state.dimensionTool.dimensionOffsetWorld = GetDimensionOffsetWorldFromPixels(state.dimensionTool.dimensionOffsetPixels, camera.zoom);
    state.dimensionTool.dimensionCreationStep = DimensionToolState::DimensionCreationStep::AwaitOffsetPoint;
    return true;
}

bool TryGetChainedDimensionNodeFromBeam(
    const ProjectDocument& document,
    const EditorState& state,
    const Beam& beam,
    Vector2 mouseWorldPosition,
    int& candidateNodeId)
{
    candidateNodeId = -1;
    if (state.dimensionTool.dimensionStartNodeId < 0)
    {
        return false;
    }

    auto candidateDistanceSquared = [&](int nodeId)
    {
        const Node* node = document.FindNodeById(nodeId);
        if (node == nullptr)
        {
            return std::numeric_limits<float>::max();
        }

        const float dx = static_cast<float>(node->position.x) - mouseWorldPosition.x;
        const float dy = static_cast<float>(node->position.y) - mouseWorldPosition.y;
        return dx * dx + dy * dy;
    };

    auto isValidCandidate = [&](int nodeId)
    {
        return nodeId >= 0 &&
               nodeId != state.dimensionTool.dimensionStartNodeId &&
               nodeId != state.dimensionTool.dimensionChainReferenceNodeId &&
               IsNodeCollinearWithDimensionChain(document, state, nodeId);
    };

    const bool startIsCandidate = isValidCandidate(beam.startNodeId);
    const bool endIsCandidate = isValidCandidate(beam.endNodeId);
    if (!startIsCandidate && !endIsCandidate)
    {
        return false;
    }

    if (startIsCandidate && !endIsCandidate)
    {
        candidateNodeId = beam.startNodeId;
        return true;
    }

    if (!startIsCandidate && endIsCandidate)
    {
        candidateNodeId = beam.endNodeId;
        return true;
    }

    // Both endpoints are valid candidates: pick the one closest to the mouse
    candidateNodeId =
        candidateDistanceSquared(beam.startNodeId) <= candidateDistanceSquared(beam.endNodeId)
            ? beam.startNodeId
            : beam.endNodeId;
    return true;
}

bool ApplyDimensionMoveOffsets(
    ProjectDocument& document,
    const std::vector<int>& movingDimensionIds,
    DimensionType type,
    DimensionOffsetMode offsetMode,
    double offsetPixels,
    double offsetWorld)
{
    bool changed = false;
    for (int dimensionId : movingDimensionIds)
    {
        Dimension* movingDimension = document.FindDimensionById(dimensionId);
        if (movingDimension == nullptr)
        {
            continue;
        }

        movingDimension->type = type;
        movingDimension->offsetMode = offsetMode;
        movingDimension->offsetPixels = offsetPixels;
        movingDimension->offsetWorld = offsetWorld;
        changed = true;
    }

    return changed;
}

bool BeginDimensionMoveSelection(
    ProjectDocument& document,
    EditorState& state,
    int dimensionId)
{
    Dimension* hoveredDimension = document.FindDimensionById(dimensionId);
    if (hoveredDimension == nullptr)
    {
        return false;
    }

    if (!state.selectionState.selection.IsDimensionSelected(hoveredDimension->id))
    {
        state.selectionState.selection.SelectSingleDimension(hoveredDimension->id);
    }

    state.dimensionTool.movingDimensionIds =
        state.selectionState.selection.HasDimensions()
            ? state.selectionState.selection.dimensionIds
            : std::vector<int>{hoveredDimension->id};
    state.dimensionTool.dimensionMoveStep = DimensionToolState::DimensionMoveStep::AwaitOffsetPoint;
    state.dimensionTool.movingDimensionId = hoveredDimension->id;
    state.dimensionTool.dimensionLengthUnit = hoveredDimension->lengthUnit;
    state.dimensionTool.dimensionType = hoveredDimension->type;
    state.dimensionTool.dimensionOffsetMode = hoveredDimension->offsetMode;
    state.dimensionTool.dimensionOffsetPixels = hoveredDimension->offsetPixels;
    state.dimensionTool.dimensionOffsetWorld = hoveredDimension->offsetWorld;
    return true;
}
}

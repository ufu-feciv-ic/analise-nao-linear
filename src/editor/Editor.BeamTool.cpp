#include "editor/Editor.h"
#include "editor/interaction/BeamGuideSystem.h"
#include "editor/interaction/BeamSnapResolver.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
    constexpr double geometryTolerance = 0.00001;

    Point2D ToPoint2D(Vector2 position)
    {
        return Point2D{
            static_cast<double>(position.x),
            static_cast<double>(position.y)};
    }

    Vector2 ToVector2(const Point2D& position)
    {
        return Vector2{
            static_cast<float>(position.x),
            static_cast<float>(position.y)};
    }

    float Clamp01(float value)
    {
        if (value < 0.0f) return 0.0f;
        if (value > 1.0f) return 1.0f;
        return value;
    }

    float EaseInOutQuad(float value)
    {
        value = Clamp01(value);
        return value < 0.5f
            ? 2.0f * value * value
            : -1.0f + (4.0f - 2.0f * value) * value;
    }

    void DrawSelector(Vector2 center, const Camera2D& camera, Color color)
    {
        constexpr float cursorSize = 25.0f;
        constexpr float cursorGap = 7.0f;
        constexpr float cursorWidth = 3.0f;

        Vector2 v1 = {(center.x - (cursorSize / 2.0f) / camera.zoom), (center.y + (cursorSize / 2.0f) / camera.zoom)};
        Vector2 v2 = {(center.x - (cursorGap / 2.0f) / camera.zoom), (center.y + (cursorSize / 2.0f) / camera.zoom)};
        Vector2 v3 = {(center.x - (cursorGap / 2.0f) / camera.zoom), (center.y + (cursorSize / 2.0f - cursorWidth) / camera.zoom)};
        Vector2 v4 = {(center.x - (cursorSize / 2.0f - cursorWidth) / camera.zoom), (center.y + (cursorSize / 2.0f - cursorWidth) / camera.zoom)};
        Vector2 v5 = {(center.x - (cursorSize / 2.0f - cursorWidth) / camera.zoom), (center.y + (cursorGap / 2.0f) / camera.zoom)};
        Vector2 v6 = {(center.x - (cursorSize / 2.0f) / camera.zoom), (center.y + (cursorGap / 2.0f) / camera.zoom)};

        DrawTriangle(v1, v2, v3, color);
        DrawTriangle(v1, v3, v4, color);
        DrawTriangle(v1, v4, v5, color);
        DrawTriangle(v1, v5, v6, color);

        v1 = {(center.x + (cursorSize / 2.0f) / camera.zoom), (center.y + (cursorSize / 2.0f) / camera.zoom)};
        v2 = {(center.x + (cursorGap / 2.0f) / camera.zoom), (center.y + (cursorSize / 2.0f) / camera.zoom)};
        v3 = {(center.x + (cursorGap / 2.0f) / camera.zoom), (center.y + (cursorSize / 2.0f - cursorWidth) / camera.zoom)};
        v4 = {(center.x + (cursorSize / 2.0f - cursorWidth) / camera.zoom), (center.y + (cursorSize / 2.0f - cursorWidth) / camera.zoom)};
        v5 = {(center.x + (cursorSize / 2.0f - cursorWidth) / camera.zoom), (center.y + (cursorGap / 2.0f) / camera.zoom)};
        v6 = {(center.x + (cursorSize / 2.0f) / camera.zoom), (center.y + (cursorGap / 2.0f) / camera.zoom)};

        DrawTriangle(v3, v2, v1, color);
        DrawTriangle(v4, v3, v1, color);
        DrawTriangle(v5, v4, v1, color);
        DrawTriangle(v6, v5, v1, color);

        v1 = {(center.x - (cursorSize / 2.0f) / camera.zoom), (center.y - (cursorSize / 2.0f) / camera.zoom)};
        v2 = {(center.x - (cursorGap / 2.0f) / camera.zoom), (center.y - (cursorSize / 2.0f) / camera.zoom)};
        v3 = {(center.x - (cursorGap / 2.0f) / camera.zoom), (center.y - (cursorSize / 2.0f - cursorWidth) / camera.zoom)};
        v4 = {(center.x - (cursorSize / 2.0f - cursorWidth) / camera.zoom), (center.y - (cursorSize / 2.0f - cursorWidth) / camera.zoom)};
        v5 = {(center.x - (cursorSize / 2.0f - cursorWidth) / camera.zoom), (center.y - (cursorGap / 2.0f) / camera.zoom)};
        v6 = {(center.x - (cursorSize / 2.0f) / camera.zoom), (center.y - (cursorGap / 2.0f) / camera.zoom)};

        DrawTriangle(v3, v2, v1, color);
        DrawTriangle(v4, v3, v1, color);
        DrawTriangle(v5, v4, v1, color);
        DrawTriangle(v6, v5, v1, color);

        v1 = {(center.x + (cursorSize / 2.0f) / camera.zoom), (center.y - (cursorSize / 2.0f) / camera.zoom)};
        v2 = {(center.x + (cursorGap / 2.0f) / camera.zoom), (center.y - (cursorSize / 2.0f) / camera.zoom)};
        v3 = {(center.x + (cursorGap / 2.0f) / camera.zoom), (center.y - (cursorSize / 2.0f - cursorWidth) / camera.zoom)};
        v4 = {(center.x + (cursorSize / 2.0f - cursorWidth) / camera.zoom), (center.y - (cursorSize / 2.0f - cursorWidth) / camera.zoom)};
        v5 = {(center.x + (cursorSize / 2.0f - cursorWidth) / camera.zoom), (center.y - (cursorGap / 2.0f) / camera.zoom)};
        v6 = {(center.x + (cursorSize / 2.0f) / camera.zoom), (center.y - (cursorGap / 2.0f) / camera.zoom)};

        DrawTriangle(v1, v2, v3, color);
        DrawTriangle(v1, v3, v4, color);
        DrawTriangle(v1, v4, v5, color);
        DrawTriangle(v1, v5, v6, color);
    }
}

void Editor::BeginBeamCreationFromNode(Node* startNode)
{
    if (startNode == nullptr)
    {
        return;
    }

    state.beamTool.isCreatingBeam = true;
    state.beamTool.beamStartNodeId = startNode->id;
    state.beamTool.beamPreviewPointWorld = Vector2{
        static_cast<float>(startNode->position.x),
        static_cast<float>(startNode->position.y)};
    state.hover.SetHoveredNode(startNode->id);
}

bool Editor::CompleteBeamCreationToNode(Node* endNode)
{
    if (endNode == nullptr || !state.beamTool.isCreatingBeam)
    {
        return false;
    }

    Node* startNode = document.FindNodeById(state.beamTool.beamStartNodeId);
    if (startNode == nullptr || endNode->id == startNode->id)
    {
        return false;
    }

    ProjectDocument beforeDocument = document;
    if (!TryAddBeamBetweenNodes(startNode->id, endNode->id))
    {
        return false;
    }
    RecordDocumentChange(std::move(beforeDocument), true);

    state.beamTool.beamStartNodeId = endNode->id;
    state.beamTool.beamPreviewPointWorld = Vector2{
        static_cast<float>(endNode->position.x),
        static_cast<float>(endNode->position.y)};
    state.hover.SetHoveredNode(endNode->id);
    return true;
}

bool Editor::ConsumeBeamDistanceInputResult(
    bool beamDistanceConfirmed,
    bool beamDistanceCancelled,
    Point2D beamDistanceCreatedNodeWorld)
{
    if (!beamDistanceConfirmed && !beamDistanceCancelled)
    {
        return false;
    }

    if (beamDistanceConfirmed)
    {
        if (IsTransformTool(state.activeTool))
        {
            if (state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitBasePoint)
            {
                state.transformTool.moveBasePointWorld = beamDistanceCreatedNodeWorld;
                state.transformTool.moveSelectionStep = TransformToolState::MoveSelectionStep::AwaitTargetPoint;
                return true;
            }

            if (state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitTargetPoint)
            {
                ApplyTransformTool(state.activeTool, beamDistanceCreatedNodeWorld);
                return true;
            }
        }

        const int nextNodeIdBefore = document.nextNodeId;
        Node* createdNode = GetOrCreateNodeAtWorldPosition(ToVector2(beamDistanceCreatedNodeWorld));
        if (createdNode != nullptr)
        {
            if (state.activeTool == EditorTool::AddBeam)
            {
                if (state.beamTool.isCreatingBeam)
                {
                    const bool createdNodeForBeam =
                        document.nextNodeId != nextNodeIdBefore &&
                        createdNode->id == nextNodeIdBefore;
                    if (!CompleteBeamCreationToNode(createdNode) && createdNodeForBeam)
                    {
                        DeleteNodeById(createdNode->id);
                    }
                }
                else
                {
                    BeginBeamCreationFromNode(createdNode);
                }
            }

            state.hover.SetHoveredNode(createdNode->id);
        }
    }

    return true;
}

bool Editor::ShouldAutoCreateGuideForNewNodes() const
{
    return BeamGuideSystem::ShouldAutoCreateGuideForNewNodes(state);
}

void Editor::EnsureGuideAtPosition(Vector2 position)
{
    BeamGuideSystem::EnsureGuideAtPosition(state, position, GetTime());
}

void Editor::ResetGuideHoverState()
{
    BeamGuideSystem::ResetGuideHoverState(state);
}

void Editor::UpdateBeamGuides(Node* hoveredNode)
{
    if (!state.view.showGuides)
    {
        ResetGuideHoverState();
        return;
    }

    UpdateActiveBeamGuides();

    if (!IsGuideInteractionToolActive())
    {
        ResetGuideHoverState();
        return;
    }

    if (hoveredNode == nullptr)
    {
        ResetGuideHoverState();
        return;
    }

    if (state.hover.beamGuideHoverNodeId != hoveredNode->id)
    {
        state.hover.beamGuideHoverNodeId = hoveredNode->id;
        state.hover.beamGuideHoverStartTime = GetTime();
        state.hover.beamGuideCreatedOnCurrentHover = false;
    }
    else if (!state.hover.beamGuideCreatedOnCurrentHover &&
             (GetTime() - state.hover.beamGuideHoverStartTime) >= 1.0)
    {
        ToggleBeamGuideAtPosition(
            Vector2{
                static_cast<float>(hoveredNode->position.x),
                static_cast<float>(hoveredNode->position.y)});
        state.hover.beamGuideCreatedOnCurrentHover = true;
    }
}

void Editor::ToggleBeamGuideAtPosition(Vector2 position)
{
    BeamGuideSystem::ToggleBeamGuideAtPosition(state, position, GetTime());
}

void Editor::UpdateActiveBeamGuides()
{
    BeamGuideSystem::UpdateActiveBeamGuides(state, mouseWorldPosition, camera.zoom);
}

bool Editor::TryResolveTransformTargetFromGuides(Vector2& targetPointWorld)
{
    const bool hasGuideX = state.beamTool.activeBeamGuideXIndex >= 0;
    const bool hasGuideY = state.beamTool.activeBeamGuideYIndex >= 0;

    if (hasGuideX && hasGuideY)
    {
        targetPointWorld = Vector2{
            state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideXIndex)].position.x,
            state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideYIndex)].position.y};
        return true;
    }

    if (hasGuideX)
    {
        OpenBeamDistanceInputFromGuide(
            state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideXIndex)].position,
            mouseWorldPosition,
            true);
        return true;
    }

    if (hasGuideY)
    {
        OpenBeamDistanceInputFromGuide(
            state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideYIndex)].position,
            mouseWorldPosition,
            false);
        return true;
    }

    return false;
}

Node* Editor::ResolveAddNodePlacement(Node* hoveredNode, Vector2 targetPosition)
{
    if (hoveredNode != nullptr)
    {
        return hoveredNode;
    }

    if (Node* guideNode = ResolveNodePlacementFromGuides(targetPosition))
    {
        return guideNode;
    }

    if (state.beamTool.isBeamDistanceInputOpen)
    {
        return nullptr;
    }

    Vector2 beamIntersectionSnap{0.0f, 0.0f};
    if (TryGetBeamIntersectionSnap(beamIntersectionSnap))
    {
        return GetOrCreateNodeAtWorldPosition(beamIntersectionSnap);
    }

    if (state.view.snapToGrid)
    {
        Vector2 beamInteriorSnap{0.0f, 0.0f};
        int referenceBeamId = -1;
        if (TryGetBeamInteriorSnap(beamInteriorSnap, referenceBeamId))
        {
            if (FindNodeByExactWorldPosition(beamInteriorSnap) != nullptr)
            {
                return GetOrCreateNodeAtWorldPosition(beamInteriorSnap);
            }

            const Beam* referenceBeam = document.FindBeamById(referenceBeamId);
            if (referenceBeam != nullptr)
            {
                OpenBeamDistanceInputFromBeam(*referenceBeam, beamInteriorSnap);
                return nullptr;
            }
        }
    }

    return GetOrCreateNodeAtWorldPosition(targetPosition);
}

Node* Editor::ResolveNodePlacementFromGuides(Vector2 targetPosition)
{
    (void)targetPosition;

    const bool hasGuideX = state.beamTool.activeBeamGuideXIndex >= 0;
    const bool hasGuideY = state.beamTool.activeBeamGuideYIndex >= 0;

    if (hasGuideX && hasGuideY)
    {
        const Vector2 guideIntersection = Vector2{
            state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideXIndex)].position.x,
            state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideYIndex)].position.y};
        return GetOrCreateNodeAtWorldPosition(guideIntersection);
    }

    if (hasGuideX)
    {
        OpenBeamDistanceInputFromGuide(
            state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideXIndex)].position,
            mouseWorldPosition,
            true);
        return nullptr;
    }

    if (hasGuideY)
    {
        OpenBeamDistanceInputFromGuide(
            state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideYIndex)].position,
            mouseWorldPosition,
            false);
        return nullptr;
    }

    return nullptr;
}

void Editor::OpenBeamDistanceInputFromGuide(Vector2 guidePosition, Vector2 clickWorldPosition, bool locksX)
{
    state.beamTool.isBeamDistanceInputOpen = true;
    state.beamTool.beamDistanceMode = BeamToolState::DistanceInputMode::GuideAxis;
    state.beamTool.beamDistanceLocksX = locksX;
    state.beamTool.beamDistanceGuideWorld = ToPoint2D(guidePosition);
    state.beamTool.beamDistanceClickWorld = ToPoint2D(clickWorldPosition);
    state.beamTool.beamDistanceSegmentStartWorld = Point2D{0.0, 0.0};
    state.beamTool.beamDistanceSegmentEndWorld = Point2D{0.0, 0.0};
    state.beamTool.beamDistanceScreenPosition = mouseScreenPosition;
}

void Editor::OpenBeamDistanceInputFromBeam(const Beam& beam, Vector2 clickWorldPosition)
{
    const Node* startNode = document.FindNodeById(beam.startNodeId);
    const Node* endNode = document.FindNodeById(beam.endNodeId);
    if (startNode == nullptr || endNode == nullptr)
    {
        return;
    }

    state.beamTool.isBeamDistanceInputOpen = true;
    state.beamTool.beamDistanceMode = BeamToolState::DistanceInputMode::BeamAlongSegment;
    state.beamTool.beamDistanceLocksX = false;
    state.beamTool.beamDistanceClickWorld = ToPoint2D(clickWorldPosition);
    state.beamTool.beamDistanceGuideWorld = Point2D{0.0, 0.0};
    state.beamTool.beamDistanceSegmentStartWorld = startNode->position;
    state.beamTool.beamDistanceSegmentEndWorld = endNode->position;
    state.beamTool.beamDistanceScreenPosition = mouseScreenPosition;
}

bool Editor::TryGetBeamIntersectionSnap(Vector2& snappedPosition) const
{
    return BeamSnapResolver::TryGetBeamIntersectionSnap(document, camera, mouseScreenPosition, snappedPosition);
}

bool Editor::TryGetBeamInteriorSnap(Vector2& snappedPosition, int& referenceBeamId) const
{
    return BeamSnapResolver::TryGetBeamInteriorSnap(
        document,
        camera,
        mouseWorldPosition,
        mouseScreenPosition,
        snappedPosition,
        referenceBeamId);
}

void Editor::DrawBeamGuides() const
{
    if (!state.view.showGuides)
    {
        return;
    }

    const Vector2 worldTopLeft = GetScreenToWorld2D(Vector2{0.0f, 0.0f}, camera);
    const Vector2 worldBottomRight = GetScreenToWorld2D(
        Vector2{static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())},
        camera);

    BeginMode2D(camera);

    const double currentTime = GetTime();

    for (const BeamToolState::BeamGuide& guide : state.beamTool.beamGuides)
    {
        const float elapsed = static_cast<float>(currentTime - guide.creationTime);
        float alpha = elapsed / 0.2f;
        alpha = Clamp01(alpha);
        alpha = EaseInOutQuad(alpha);

        const unsigned char alphaIntX = static_cast<unsigned char>(
            (guide.activeX ? 1.0f : 0.2f) * alpha * 255.0f);
        const unsigned char alphaIntY = static_cast<unsigned char>(
            (guide.activeY ? 1.0f : 0.2f) * alpha * 255.0f);

        const Color verticalColor = Color{70, 145, 255, alphaIntX};
        const Color horizontalColor = Color{70, 145, 255, alphaIntY};
        const float thickness = 10.0f + (2.0f - 10.0f) * alpha;

        const float left = guide.position.x + alpha * (worldTopLeft.x - guide.position.x);
        const float right = guide.position.x + alpha * (worldBottomRight.x - guide.position.x);
        const float top = guide.position.y + alpha * (worldTopLeft.y - guide.position.y);
        const float bottom = guide.position.y + alpha * (worldBottomRight.y - guide.position.y);

        DrawLineEx(
            Vector2{guide.position.x, top},
            Vector2{guide.position.x, bottom},
            thickness / camera.zoom,
            verticalColor);

        DrawLineEx(
            Vector2{left, guide.position.y},
            Vector2{right, guide.position.y},
            thickness / camera.zoom,
            horizontalColor);
    }

    EndMode2D();
}

void Editor::DrawActiveBeamGuideOriginSelectors() const
{
    if (!state.view.showGuides || !IsGuideInteractionToolActive())
    {
        return;
    }

    BeginMode2D(camera);

    if (state.beamTool.activeBeamGuideXIndex >= 0)
    {
        const Vector2 center = state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideXIndex)].position;
        DrawSelector(center, camera, Color{70, 145, 255, 255});
    }

    if (state.beamTool.activeBeamGuideYIndex >= 0)
    {
        const Vector2 center = state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideYIndex)].position;
        DrawSelector(center, camera, Color{70, 145, 255, 255});
    }

    EndMode2D();
}

void Editor::DrawBeamStartIndicator() const
{
    if (!state.beamTool.isCreatingBeam || state.beamTool.beamStartNodeId < 0)
    {
        return;
    }

    const Node* startNode = document.FindNodeById(state.beamTool.beamStartNodeId);
    if (startNode == nullptr)
    {
        return;
    }

    const Vector2 center = Vector2{
        static_cast<float>(startNode->position.x),
        static_cast<float>(startNode->position.y)};

    const Color indicatorColor = Color{215, 215, 215, 255};

    BeginMode2D(camera);
    DrawSelector(center, camera, indicatorColor);
    EndMode2D();
}

void Editor::DrawBeamDistanceReferenceIndicator() const
{
    if (!state.beamTool.isBeamDistanceInputOpen ||
        state.beamTool.beamDistanceMode != BeamToolState::DistanceInputMode::BeamAlongSegment)
    {
        return;
    }

    const Vector2 center = ToVector2(state.beamTool.beamDistanceSegmentStartWorld);

    BeginMode2D(camera);
    DrawSelector(center, camera, Color{70, 145, 255, 255});
    EndMode2D();
}

void Editor::DrawBeamGuideHoverIndicator() const
{
    if (!state.view.showGuides ||
        !IsGuideInteractionToolActive() ||
        state.hover.beamGuideHoverNodeId < 0 ||
        state.hover.beamGuideCreatedOnCurrentHover)
    {
        return;
    }

    const Node* hoveredNode = document.FindNodeById(state.hover.beamGuideHoverNodeId);
    if (hoveredNode == nullptr)
    {
        return;
    }

    const float elapsed = static_cast<float>(GetTime() - state.hover.beamGuideHoverStartTime);
    const float alpha = Clamp01((elapsed / 1.0f) * 2.0f);
    const unsigned char alphaByte = static_cast<unsigned char>(alpha * 255.0f);

    const Vector2 center = Vector2{
        static_cast<float>(hoveredNode->position.x),
        static_cast<float>(hoveredNode->position.y)};

    BeginMode2D(camera);
    DrawSelector(center, camera, Color{170, 170, 170, alphaByte});
    EndMode2D();
}


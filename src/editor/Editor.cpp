#include "editor/Editor.h"

#include "editor/ops/PropertySelectionOperations.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "editor/interaction/HoverInteractor.h"
#include "editor/interaction/SnapResolver.h"
#include "editor/ops/BeamOperations.h"
#include "editor/ops/NodeOperations.h"
#include "editor/ops/StructureCleanup.h"
#include "editor/EditorContext.h"

namespace
{
    constexpr float viewportTopToolbarHeight = 150.0f;
    constexpr float viewportLeftPanelWidth = 73.0f;
    constexpr float nodeHoverRadiusPixels = 6.0f;
    void DrawDashedLine(Vector2 start, Vector2 end, float thickness, Color color)
    {
        constexpr float dashLength = 8.0f;
        constexpr float gapLength = 5.0f;

        const float deltaX = end.x - start.x;
        const float deltaY = end.y - start.y;
        const float totalLength = sqrtf(deltaX * deltaX + deltaY * deltaY);
        if (totalLength <= 1.0e-6f)
        {
            return;
        }

        const float directionX = deltaX / totalLength;
        const float directionY = deltaY / totalLength;
        float currentLength = 0.0f;

        while (currentLength < totalLength)
        {
            const float dashEnd = fminf(currentLength + dashLength, totalLength);
            const Vector2 dashStartPoint = Vector2{
                start.x + directionX * currentLength,
                start.y + directionY * currentLength};
            const Vector2 dashEndPoint = Vector2{
                start.x + directionX * dashEnd,
                start.y + directionY * dashEnd};
            DrawLineEx(dashStartPoint, dashEndPoint, thickness, color);
            currentLength = dashEnd + gapLength;
        }
    }

    void DrawDashedRectangleOutline(Rectangle rect, float thickness, Color color)
    {
        const Vector2 topLeft = Vector2{rect.x, rect.y};
        const Vector2 topRight = Vector2{rect.x + rect.width, rect.y};
        const Vector2 bottomRight = Vector2{rect.x + rect.width, rect.y + rect.height};
        const Vector2 bottomLeft = Vector2{rect.x, rect.y + rect.height};

        DrawDashedLine(topLeft, topRight, thickness, color);
        DrawDashedLine(topRight, bottomRight, thickness, color);
        DrawDashedLine(bottomRight, bottomLeft, thickness, color);
        DrawDashedLine(bottomLeft, topLeft, thickness, color);
    }

    Rectangle AlignRectangleOutlineToPixelGrid(Rectangle rect, float thickness)
    {
        const float halfThickness = thickness * 0.5f;

        const float left = floorf(rect.x) + halfThickness;
        const float top = floorf(rect.y) + halfThickness;
        const float right = floorf(rect.x + rect.width) + halfThickness;
        const float bottom = floorf(rect.y + rect.height) + halfThickness;

        return Rectangle{
            left,
            top,
            fmaxf(0.0f, right - left),
            fmaxf(0.0f, bottom - top)};
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
Editor::Editor() = default;

void Editor::SetViewportOverlayFont(Font font)
{
    viewportOverlayFont = font;
    hasViewportOverlayFont = font.texture.id != 0;
}

void Editor::SetViewportOverlayTitleFont(Font font)
{
    viewportOverlayTitleFont = font;
    hasViewportOverlayTitleFont = font.texture.id != 0;
}

void Editor::SetDimensionFont(Font font)
{
    renderer.SetDimensionFont(font);
}

void Editor::Initialize()
{
    cameraController.Initialize(camera);
    InitializeDemoScene();
    undoHistory.clear();
    redoHistory.clear();
    MarkDerivedDataDirty();
}

void Editor::Update(
    const FrameRequests& requests,
    bool mouseWorldInputAllowed,
    bool keyboardShortcutsAllowed,
    bool beamDistanceConfirmed,
    bool beamDistanceCancelled,
    Point2D beamDistanceCreatedNodeWorld)
{
    cameraController.Update(camera, mouseWorldInputAllowed);
    UpdateWorldPositions();
    const SelectionMode selectionMode = GetSelectionMode();

    const HoveredEntities hoveredEntities = HoverInteractor::ResolveHoveredEntities(
        document,
        camera,
        mouseWorldPosition,
        mouseScreenPosition);

    Node* hoveredNode = hoveredEntities.node;
    Beam* hoveredBeam = hoveredEntities.beam;
    Dimension* hoveredDimension = hoveredEntities.dimension;
    state.hover.entity = hoveredEntities.entity;
    UpdateInteractionPreviews(hoveredNode);

    editorRequestController.Apply(*this, requests.editor);
    UpdateTransformPreviewState(hoveredNode);

    if (keyboardShortcutsAllowed)
    {
        HandleKeyboardShortcuts();
    }

    if (keyboardShortcutsAllowed && IsKeyPressed(KEY_DELETE))
    {
        DeleteSelection();
    }

    if (ConsumeBeamDistanceInputResult(
            beamDistanceConfirmed,
            beamDistanceCancelled,
            beamDistanceCreatedNodeWorld))
    {
        return;
    }

    if (!mouseWorldInputAllowed)
    {
        HandleBoxSelectionReleaseWithoutWorldInput(selectionMode);
        return;
    }

    if (state.beamTool.isBeamDistanceInputOpen)
    {
        return;
    }

    HandleActiveTool(hoveredNode, hoveredBeam, hoveredDimension, selectionMode);
}

void Editor::UpdateInteractionPreviews(Node* hoveredNode)
{
    if (!state.beamTool.isBeamDistanceInputOpen)
    {
        state.beamTool.beamPreviewPointWorld = GetCurrentPlacementWorldPosition();
        UpdateBeamGuides(hoveredNode);
    }
}

void Editor::UpdateTransformPreviewState(Node* hoveredNode)
{
    const bool allowSelectedNodeSnapForTarget =
        state.activeTool == EditorTool::CopySelection ||
        state.activeTool == EditorTool::MirrorSelection;

    state.transformTool.moveSnapNodeId = -1;
    if (state.view.snapToGrid &&
        (state.activeTool == EditorTool::MoveNode ||
         state.activeTool == EditorTool::CopySelection ||
         state.activeTool == EditorTool::MirrorSelection) &&
        (state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitBasePoint ||
         state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitTargetPoint) &&
        hoveredNode != nullptr &&
        (state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitBasePoint ||
         allowSelectedNodeSnapForTarget ||
         !state.selectionState.selection.IsNodeSelected(hoveredNode->id)))
    {
        state.transformTool.moveSnapNodeId = hoveredNode->id;
    }

    state.transformTool.movePreviewPointWorld = GetCurrentTransformTargetWorldPosition(
        hoveredNode,
        state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitBasePoint ||
            allowSelectedNodeSnapForTarget);
}

void Editor::HandleActiveTool(
    Node* hoveredNode,
    Beam* hoveredBeam,
    Dimension* hoveredDimension,
    SelectionMode selectionMode)
{
    EditorContext context{
        *this,
        document,
        state,
        camera,
        mouseScreenPosition,
        mouseWorldPosition,
        snappedWorldPosition,
        true,
        true,
        hoveredNode,
        hoveredBeam,
        hoveredDimension,
        selectionMode};
    toolDispatcher.Dispatch(
        context,
        selectToolController,
        addNodeToolController,
        transformToolController,
        beamToolController,
        dimensionToolController,
        supportToolController,
        pointLoadToolController,
        distributedLoadToolController,
        deleteToolController);
}

Node* Editor::FindNodeByExactWorldPosition(Vector2 worldPosition)
{
    return NodeOperations::FindNodeByExactWorldPosition(document, worldPosition);
}

Node* Editor::GetOrCreateNodeAtWorldPosition(Vector2 worldPosition, bool allowAutoGuideCreation)
{
    const bool nodeAlreadyExisted = FindNodeByExactWorldPosition(worldPosition) != nullptr;
    Node* createdNode = NodeOperations::GetOrCreateNodeAtWorldPosition(document, worldPosition);
    if (createdNode == nullptr)
    {
        return nullptr;
    }

    if (!nodeAlreadyExisted)
    {
        SplitBeamsAtNode(createdNode->id);
    }

    if (!nodeAlreadyExisted && allowAutoGuideCreation && ShouldAutoCreateGuideForNewNodes())
    {
        EnsureGuideAtPosition(
            Vector2{
                static_cast<float>(createdNode->position.x),
                static_cast<float>(createdNode->position.y)});
    }

    return createdNode;
}

bool Editor::SplitBeamsAtNode(int nodeId)
{
    return BeamOperations::SplitBeamsAtNode(
        document,
        state,
        nodeId,
        [&](int originalStartNodeId, int originalEndNodeId, const DistributedLoadValue& loadValue)
        {
            AssignDistributedLoadAlongBeamPath(originalStartNodeId, originalEndNodeId, loadValue);
        });
}

bool Editor::TryAddBeamBetweenNodes(int startNodeId, int endNodeId)
{
    state.currentMaterialId = PropertySelectionOperations::ResolveCurrentMaterialId(
        document,
        state.currentMaterialId);
    state.currentSectionId = PropertySelectionOperations::ResolveCurrentSectionId(
        document,
        state.currentSectionId);

    if (state.currentMaterialId < 0 || state.currentSectionId < 0)
    {
        return false;
    }

    return EnsureBeamPathBetweenNodes(
        startNodeId,
        endNodeId,
        state.currentMaterialId,
        state.currentSectionId);
}

int Editor::AddBeamBetweenNodesWithProperties(int startNodeId, int endNodeId, int materialId, int sectionId)
{
    return BeamOperations::AddBeamBetweenNodesWithProperties(
        document,
        startNodeId,
        endNodeId,
        materialId,
        sectionId);
}

bool Editor::EnsureBeamPathBetweenNodes(int startNodeId, int endNodeId, int materialId, int sectionId)
{
    return BeamOperations::EnsureBeamPathBetweenNodes(
        document,
        startNodeId,
        endNodeId,
        materialId,
        sectionId);
}

std::vector<int> Editor::CollectNodeIdsOnSegment(int startNodeId, int endNodeId) const
{
    return BeamOperations::CollectNodeIdsOnSegment(document, startNodeId, endNodeId);
}

bool Editor::ResolveBeamDirection(int& startNodeId, int& endNodeId) const
{
    return BeamOperations::ResolveBeamDirection(document, startNodeId, endNodeId);
}

Vector2 Editor::GetCurrentPlacementWorldPosition() const
{
    return SnapResolver::GetCurrentPlacementWorldPosition(state, mouseWorldPosition, snappedWorldPosition);
}

bool Editor::IsGuideInteractionToolActive() const
{
    return state.activeTool == EditorTool::AddBeam ||
           state.activeTool == EditorTool::AddNode ||
           ((state.activeTool == EditorTool::MoveNode ||
             state.activeTool == EditorTool::CopySelection ||
             state.activeTool == EditorTool::MirrorSelection) &&
            (state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitBasePoint ||
             state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitTargetPoint));
}

bool Editor::IsMoveSelectionConfirmPressed() const
{
    return IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER);
}

Point2D Editor::GetCurrentTransformTargetWorldPosition(Node* hoveredNode, bool allowSelectedNodeSnap) const
{
    return SnapResolver::GetCurrentTransformTargetWorldPosition(
        state,
        mouseWorldPosition,
        snappedWorldPosition,
        hoveredNode,
        allowSelectedNodeSnap);
}

bool Editor::AppendBeamPathBetweenNodesFast(
    int startNodeId,
    int endNodeId,
    int materialId,
    int sectionId,
    std::unordered_set<std::uint64_t>& existingBeamKeys)
{
    return BeamOperations::AppendBeamPathBetweenNodesFast(
        document,
        startNodeId,
        endNodeId,
        materialId,
        sectionId,
        existingBeamKeys);
}

void Editor::CancelBeamCreationOperation()
{
    state.beamTool.isCreatingBeam = false;
    state.beamTool.beamStartNodeId = -1;
    state.beamTool.beamPreviewPointWorld = Vector2{0.0f, 0.0f};
    state.beamTool.isBeamDistanceInputOpen = false;
    state.beamTool.beamDistanceMode = BeamToolState::DistanceInputMode::GuideAxis;
    state.beamTool.beamDistanceLocksX = false;
    state.beamTool.beamDistanceScreenPosition = Vector2{0.0f, 0.0f};
    state.beamTool.beamDistanceClickWorld = Point2D{0.0, 0.0};
    state.beamTool.beamDistanceGuideWorld = Point2D{0.0, 0.0};
    state.beamTool.beamDistanceSegmentStartWorld = Point2D{0.0, 0.0};
    state.beamTool.beamDistanceSegmentEndWorld = Point2D{0.0, 0.0};
}

void Editor::MergeCoincidentNodes()
{
    StructureCleanup::MergeCoincidentNodes(document, state);
}

void Editor::RebuildBeamsAfterNodeChanges()
{
    StructureCleanup::RebuildBeamsAfterNodeChanges(
        document,
        state,
        [&](int originalStartNodeId, int originalEndNodeId, const DistributedLoadValue& loadValue)
        {
            AssignDistributedLoadAlongBeamPath(originalStartNodeId, originalEndNodeId, loadValue);
        });
    PurgeInvalidSelection();
}

void Editor::NormalizeStructureAfterMove()
{
    MergeCoincidentNodes();
    RebuildBeamsAfterNodeChanges();
    CleanupDimensionsAfterNodeChanges();
    PurgeInvalidSelection();
    state.hover.ClearEntity();
}

void Editor::CleanupDimensionsAfterNodeChanges()
{
    StructureCleanup::CleanupDimensionsAfterNodeChanges(document, state);
}

void Editor::Render()
{
    RefreshDerivedDataIfNeeded();
    renderer.Render(document, derivedData, state, camera, cameraController.GetTargetZoom());
    DrawBeamGuides();
    DrawBeamGuideHoverIndicator();
    DrawBeamStartIndicator();
    DrawDimensionToolIndicators();
    DrawActiveBeamGuideOriginSelectors();
    DrawBeamDistanceReferenceIndicator();
    DrawMoveSnapIndicator();

    if (state.selectionState.isBoxSelecting)
    {
        const Rectangle rect = GetSelectionRectangle();
        const bool crossingSelection = IsRightToLeftBoxSelection();
        const SelectionMode selectionMode = GetSelectionMode();
        const Color fillColor =
            selectionMode == SelectionMode::Remove
                ? Color{232, 140, 54, 40}
                : Color{80, 140, 255, 35};
        const Color outlineColor =
            selectionMode == SelectionMode::Remove
                ? Color{232, 140, 54, 200}
                : Color{80, 140, 255, 180};

        DrawRectangleRec(rect, fillColor);
        if (crossingSelection)
        {
            DrawDashedRectangleOutline(
                AlignRectangleOutlineToPixelGrid(rect, 1.0f),
                1.0f,
                outlineColor);
        }
        else
        {
            DrawRectangleLinesEx(rect, 1.0f, outlineColor);
        }
    }

    DrawViewportStatusOverlay();
    DrawGridScaleOverlay();
}


void Editor::MarkDerivedDataDirty()
{
    derivedDataDirty = true;
}

void Editor::RefreshDerivedDataIfNeeded()
{
    if (!derivedDataDirty)
    {
        return;
    }

    derivedData.Rebuild(document);
    derivedDataDirty = false;
}

void Editor::DrawMoveSnapIndicator() const
{
    if (state.transformTool.moveSnapNodeId < 0)
    {
        return;
    }

    const Node* snapNode = document.FindNodeById(state.transformTool.moveSnapNodeId);
    if (snapNode == nullptr)
    {
        return;
    }

    const Vector2 center = Vector2{
        static_cast<float>(snapNode->position.x),
        static_cast<float>(snapNode->position.y)};

    BeginMode2D(camera);
    DrawSelector(center, camera, Color{70, 145, 255, 255});
    EndMode2D();
}

bool Editor::HandleEscape()
{
    if (state.beamTool.isBeamDistanceInputOpen)
    {
        state.beamTool.isBeamDistanceInputOpen = false;
        return true;
    }

    if (state.HasActiveToolOrOperation())
    {
        state.CancelCurrentTool();
        return true;
    }

    if (!state.beamTool.beamGuides.empty())
    {
        state.beamTool.beamGuides.clear();
        state.hover.beamGuideHoverNodeId = -1;
        state.hover.beamGuideHoverStartTime = 0.0;
        state.hover.beamGuideCreatedOnCurrentHover = false;
        state.beamTool.activeBeamGuideXIndex = -1;
        state.beamTool.activeBeamGuideYIndex = -1;
        return true;
    }

    if (state.HasSelection())
    {
        state.ClearSelection();
        return true;
    }

    return false;
}

void Editor::UpdateWorldPositions()
{
    mouseScreenPosition = GetMousePosition();
    mouseWorldPosition = GetScreenToWorld2D(mouseScreenPosition, camera);

    const float spacing = CalculateGridSpacing(100.0f, camera.zoom);
    snappedWorldPosition = RoundPositionToGrid(mouseWorldPosition, spacing);
}

Vector2 Editor::RoundPositionToGrid(Vector2 position, float spacing) const
{
    return SnapResolver::RoundPositionToGrid(position, spacing);
}

float Editor::CalculateGridSpacing(float baseSpacing, float zoomLevel) const
{
    const float rawSpacing = baseSpacing / zoomLevel;
    const float exponent = floorf(log10f(rawSpacing));
    float normalizedSpacing = powf(10.0f, exponent);

    if (rawSpacing / normalizedSpacing < 1.0f)
    {
        normalizedSpacing /= 10.0f;
    }

    return normalizedSpacing;
}



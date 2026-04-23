#include "editor/Editor.h"

#include <cmath>

namespace
{
    double GetDimensionOffsetWorldFromPixels(double offsetPixels, float cameraZoom)
    {
        if (cameraZoom <= 0.0f)
        {
            return 0.0;
        }

        return offsetPixels / static_cast<double>(cameraZoom);
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

void Editor::DrawDimensionToolIndicators() const
{
    if (state.activeTool != EditorTool::AddDimension)
    {
        return;
    }

    const Node* startNode = document.FindNodeById(state.dimensionTool.dimensionStartNodeId);
    const Node* endNode = document.FindNodeById(state.dimensionTool.dimensionEndNodeId);
    if (startNode == nullptr && endNode == nullptr)
    {
        return;
    }

    BeginMode2D(camera);

    if (startNode != nullptr)
    {
        const Vector2 center = Vector2{
            static_cast<float>(startNode->position.x),
            static_cast<float>(startNode->position.y)};
        DrawSelector(center, camera, Color{215, 215, 215, 255});
    }

    if (endNode != nullptr &&
        (state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitOffsetPoint ||
         state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitChainedNode))
    {
        const Vector2 center = Vector2{
            static_cast<float>(endNode->position.x),
            static_cast<float>(endNode->position.y)};
        DrawSelector(center, camera, Color{70, 145, 255, 255});
    }

    EndMode2D();
}

void Editor::CancelDimensionCreationOperation()
{
    state.dimensionTool.dimensionCreationStep = DimensionToolState::DimensionCreationStep::None;
    state.dimensionTool.dimensionStartNodeId = -1;
    state.dimensionTool.dimensionEndNodeId = -1;
    state.dimensionTool.dimensionChainReferenceNodeId = -1;
    state.dimensionTool.dimensionLengthUnit = state.dimensionTool.newDimensionLengthUnit;
    state.dimensionTool.dimensionType = DimensionType::Aligned;
    state.dimensionTool.dimensionOffsetMode = state.dimensionTool.newDimensionOffsetMode;
    state.dimensionTool.dimensionOffsetPixels = 32.0;
    state.dimensionTool.dimensionOffsetWorld = GetDimensionOffsetWorldFromPixels(state.dimensionTool.dimensionOffsetPixels, camera.zoom);
    state.dimensionTool.dimensionPreviewPointWorld = Vector2{0.0f, 0.0f};
    state.dimensionTool.dimensionPreviewScreenPosition = Vector2{0.0f, 0.0f};
}

void Editor::CancelDimensionMoveOperation()
{
    state.dimensionTool.dimensionMoveStep = DimensionToolState::DimensionMoveStep::None;
    state.dimensionTool.movingDimensionId = -1;
    state.dimensionTool.movingDimensionIds.clear();
}


#include "editor/render/EditorRendererInternal.h"

void EditorRenderer::RenderDimensionPass(
    const ProjectDocument& document,
    const EditorState& state,
    const Camera2D& camera) const
{
    if (!state.view.showDimensions)
    {
        return;
    }

    DrawDimensions(document, state, camera);
}

void EditorRenderer::DrawDimensions(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const
{
    constexpr float lineThickness = 1.8f;
    const Color dimensionColor = Color{90, 90, 90, 255};
    const Color hoveredColor = Color{130, 160, 205, 255};
    const Color selectedColor = Color{70, 145, 255, 255};
    const Color previewColor = Color{70, 145, 255, 255};
    const float deltaTime = GetFrameTime();
    std::vector<DimensionLabelJob> labels;
    labels.reserve(document.dimensions.size() + 2);
    const Font labelFont = m_hasDimensionFont ? m_dimensionFont : GetFontDefault();
    constexpr float labelFontSize = 18.0f;
    std::unordered_map<int, bool> activeDimensionIds;
    activeDimensionIds.reserve(document.dimensions.size());
    const std::vector<int> selectedDimensionIds = MakeSortedUniqueIdList(state.selectionState.selection.dimensionIds);
    std::unordered_map<std::string, Vector2> textMeasureCache;
    textMeasureCache.reserve(document.dimensions.size() + 4);
    auto getTextSize = [&](const std::string& text) -> Vector2
    {
        auto [it, inserted] = textMeasureCache.emplace(text, Vector2{});
        if (inserted)
        {
            it->second = MeasureTextEx(labelFont, text.c_str(), labelFontSize, 0.0f);
        }

        return it->second;
    };

    auto drawDimensionGeometry = [&](const ScreenDimensionGeometry& geometry, Color color)
    {
        DrawLineEx(geometry.startExtensionStart, geometry.startExtensionEnd, lineThickness, color);
        DrawLineEx(geometry.endExtensionStart, geometry.endExtensionEnd, lineThickness, color);
        DrawLineEx(geometry.lineStart, geometry.lineEnd, lineThickness, color);
        DrawFilledTriangleSafe(geometry.startArrowTip, geometry.startArrowLeft, geometry.startArrowRight, color);
        DrawFilledTriangleSafe(geometry.endArrowTip, geometry.endArrowLeft, geometry.endArrowRight, color);
    };

    auto enqueueDimension = [&](
                              const ScreenDimensionGeometry& geometry,
                              Color baseColor,
                              LengthUnit lengthUnit,
                              int dimensionId,
                              bool useTimedFade)
    {
        const std::string text = FormatDisplayLength(geometry.measuredLengthWorld, lengthUnit);
        const Vector2 textSize = getTextSize(text);
        float alpha = 1.0f;

        if (useTimedFade && state.view.enableDimensionTextFade)
        {
            activeDimensionIds[dimensionId] = true;
            auto [it, inserted] = m_dimensionTextFadeById.emplace(dimensionId, 1.0f);
            float& storedAlpha = it->second;
            if (inserted)
            {
                storedAlpha = 1.0f;
            }

            const float fadeOutThreshold = textSize.x * kDimensionFadeOutThresholdRatio;
            const float fadeInThreshold = textSize.x * kDimensionFadeInThresholdRatio;
            const float step = deltaTime / kDimensionFadeDuration;
            if (geometry.lineLengthPixels < fadeOutThreshold)
            {
                storedAlpha = std::max(0.0f, storedAlpha - step);
            }
            else if (geometry.lineLengthPixels > fadeInThreshold)
            {
                storedAlpha = std::min(1.0f, storedAlpha + step);
            }

            alpha = storedAlpha;
        }
        else if (useTimedFade)
        {
            m_dimensionTextFadeById[dimensionId] = 1.0f;
        }

        if (alpha <= 0.01f)
        {
            return;
        }

        const Color color = WithScaledAlpha(baseColor, alpha);
        drawDimensionGeometry(geometry, color);
        labels.push_back(DimensionLabelJob{
            geometry.labelScreen,
            text,
            textSize,
            geometry.textRotationDegrees,
            color});
    };

    auto choosePreviewOffsetPixels = [&](Vector2 startWorld, Vector2 endWorld) -> double
    {
        const Vector2 startScreen = GetWorldToScreen2D(startWorld, camera);
        const Vector2 endScreen = GetWorldToScreen2D(endWorld, camera);

        if (state.beamTool.beamDistanceMode == BeamToolState::DistanceInputMode::BeamAlongSegment)
        {
            Vector2 segmentDirection = Vector2{
                endScreen.x - startScreen.x,
                endScreen.y - startScreen.y};
            const float segmentLength = VectorLength(segmentDirection);
            if (segmentLength <= 1.0e-6f)
            {
                return 32.0;
            }

            segmentDirection.x /= segmentLength;
            segmentDirection.y /= segmentLength;

            Vector2 normal = Vector2{-segmentDirection.y, segmentDirection.x};
            if (normal.y > 0.0f || (std::abs(normal.y) <= 1.0e-6f && normal.x < 0.0f))
            {
                normal.x = -normal.x;
                normal.y = -normal.y;
            }

            const Vector2 rawNormal = Vector2{-segmentDirection.y, segmentDirection.x};
            const float sign =
                rawNormal.x * normal.x + rawNormal.y * normal.y >= 0.0f ? 1.0f : -1.0f;
            return 150.0 * static_cast<double>(sign);
        }

        const Vector2 referenceScreen = GetWorldToScreen2D(ToVector2(state.beamTool.beamDistanceGuideWorld), camera);
        if (state.beamTool.beamDistanceLocksX)
        {
            return state.beamTool.beamDistanceScreenPosition.x >= referenceScreen.x ? 150.0 : -150.0;
        }

        return state.beamTool.beamDistanceScreenPosition.y >= referenceScreen.y ? 150.0 : -150.0;
    };

    for (const Dimension& dimension : document.dimensions)
    {
        const Node* startNode = document.FindNodeById(dimension.startNodeId);
        const Node* endNode = document.FindNodeById(dimension.endNodeId);
        if (startNode == nullptr || endNode == nullptr)
        {
            continue;
        }

        const Vector2 start = Vector2{
            static_cast<float>(startNode->position.x),
            static_cast<float>(startNode->position.y)};
        const Vector2 end = Vector2{
            static_cast<float>(endNode->position.x),
            static_cast<float>(endNode->position.y)};
        const Vector2 startScreen = GetWorldToScreen2D(start, camera);
        const Vector2 endScreen = GetWorldToScreen2D(end, camera);

        ScreenDimensionGeometry geometry;
        if (!TryBuildScreenDimensionGeometry(
                start,
                end,
                startScreen,
                endScreen,
                dimension.type,
                GetDimensionOffsetPixelsForDisplay(dimension, camera.zoom),
                geometry))
        {
            continue;
        }

        const bool isSelected = ContainsSortedId(selectedDimensionIds, dimension.id);
        const bool isHovered =
            state.hover.entity.type == EntityType::Dimension &&
            state.hover.entity.id == dimension.id;
        const Color color = isSelected ? selectedColor : (isHovered ? hoveredColor : dimensionColor);
        enqueueDimension(geometry, color, dimension.lengthUnit, dimension.id, true);
    }

    if (state.activeTool == EditorTool::AddDimension &&
        state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitOffsetPoint)
    {
        const Node* startNode = document.FindNodeById(state.dimensionTool.dimensionStartNodeId);
        const Node* endNode = document.FindNodeById(state.dimensionTool.dimensionEndNodeId);
        if (startNode != nullptr && endNode != nullptr)
        {
            const Vector2 start = Vector2{
                static_cast<float>(startNode->position.x),
                static_cast<float>(startNode->position.y)};
            const Vector2 end = Vector2{
                static_cast<float>(endNode->position.x),
                static_cast<float>(endNode->position.y)};
            ScreenDimensionGeometry previewGeometry;
            if (TryBuildScreenDimensionGeometry(
                    start,
                    end,
                    GetWorldToScreen2D(start, camera),
                    GetWorldToScreen2D(end, camera),
                    state.dimensionTool.dimensionType,
                    state.dimensionTool.dimensionOffsetMode == DimensionOffsetMode::WorldUnits
                        ? state.dimensionTool.dimensionOffsetWorld * static_cast<double>(camera.zoom)
                        : state.dimensionTool.dimensionOffsetPixels,
                    previewGeometry))
            {
                enqueueDimension(previewGeometry, previewColor, state.dimensionTool.dimensionLengthUnit, -1, false);
            }
        }
    }

    if (state.activeTool == EditorTool::AddDimension &&
        state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitChainedNode)
    {
        const Node* startNode = document.FindNodeById(state.dimensionTool.dimensionStartNodeId);
        const Node* endNode = document.FindNodeById(state.dimensionTool.dimensionEndNodeId);
        if (startNode != nullptr && endNode != nullptr)
        {
            const Vector2 start = Vector2{
                static_cast<float>(startNode->position.x),
                static_cast<float>(startNode->position.y)};
            const Vector2 end = Vector2{
                static_cast<float>(endNode->position.x),
                static_cast<float>(endNode->position.y)};
            ScreenDimensionGeometry previewGeometry;
            if (TryBuildScreenDimensionGeometry(
                    start,
                    end,
                    GetWorldToScreen2D(start, camera),
                    GetWorldToScreen2D(end, camera),
                    state.dimensionTool.dimensionType,
                    state.dimensionTool.dimensionOffsetMode == DimensionOffsetMode::WorldUnits
                        ? state.dimensionTool.dimensionOffsetWorld * static_cast<double>(camera.zoom)
                        : state.dimensionTool.dimensionOffsetPixels,
                    previewGeometry))
            {
                enqueueDimension(previewGeometry, previewColor, state.dimensionTool.dimensionLengthUnit, -1, false);
            }
        }
    }

    if (state.activeTool == EditorTool::MoveDimension &&
        state.dimensionTool.dimensionMoveStep == DimensionToolState::DimensionMoveStep::AwaitOffsetPoint)
    {
        std::vector<int> previewDimensionIds = state.dimensionTool.movingDimensionIds;
        if (previewDimensionIds.empty() && state.dimensionTool.movingDimensionId >= 0)
        {
            previewDimensionIds.push_back(state.dimensionTool.movingDimensionId);
        }

        for (int dimensionId : previewDimensionIds)
        {
            const Dimension* movingDimension = document.FindDimensionById(dimensionId);
            if (movingDimension == nullptr)
            {
                continue;
            }

            const Node* startNode = document.FindNodeById(movingDimension->startNodeId);
            const Node* endNode = document.FindNodeById(movingDimension->endNodeId);
            if (startNode == nullptr || endNode == nullptr)
            {
                continue;
            }

            const Vector2 start = Vector2{
                static_cast<float>(startNode->position.x),
                static_cast<float>(startNode->position.y)};
            const Vector2 end = Vector2{
                static_cast<float>(endNode->position.x),
                static_cast<float>(endNode->position.y)};
            ScreenDimensionGeometry previewGeometry;
            if (TryBuildScreenDimensionGeometry(
                    start,
                    end,
                    GetWorldToScreen2D(start, camera),
                    GetWorldToScreen2D(end, camera),
                    state.dimensionTool.dimensionType,
                    state.dimensionTool.dimensionOffsetMode == DimensionOffsetMode::WorldUnits
                        ? state.dimensionTool.dimensionOffsetWorld * static_cast<double>(camera.zoom)
                        : state.dimensionTool.dimensionOffsetPixels,
                    previewGeometry))
            {
                enqueueDimension(previewGeometry, previewColor, movingDimension->lengthUnit, -1, false);
            }
        }
    }

    if (state.beamTool.isBeamDistanceInputOpen)
    {
        const Vector2 start =
            state.beamTool.beamDistanceMode == BeamToolState::DistanceInputMode::BeamAlongSegment
                ? ToVector2(state.beamTool.beamDistanceSegmentStartWorld)
                : ToVector2(state.beamTool.beamDistanceGuideWorld);
        const Vector2 end = ToVector2(state.beamTool.beamDistanceCreatedNodeWorld);
        const Vector2 startScreen = GetWorldToScreen2D(start, camera);
        const Vector2 endScreen = GetWorldToScreen2D(end, camera);

        const DimensionType previewType =
            state.beamTool.beamDistanceMode == BeamToolState::DistanceInputMode::BeamAlongSegment
                ? DimensionType::Aligned
                : (state.beamTool.beamDistanceLocksX ? DimensionType::Vertical : DimensionType::Horizontal);
        ScreenDimensionGeometry previewGeometry;
        if (TryBuildScreenDimensionGeometry(
                start,
                end,
                startScreen,
                endScreen,
                previewType,
                choosePreviewOffsetPixels(start, end),
                previewGeometry))
        {
            enqueueDimension(previewGeometry, previewColor, LengthUnit::Meter, -1, false);
        }
    }

    for (auto it = m_dimensionTextFadeById.begin(); it != m_dimensionTextFadeById.end();)
    {
        if (activeDimensionIds.find(it->first) == activeDimensionIds.end())
        {
            it = m_dimensionTextFadeById.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (const DimensionLabelJob& label : labels)
    {
        DrawTextPro(
            labelFont,
            label.text.c_str(),
            label.screenCenter,
            Vector2{label.textSize.x * 0.5f, label.textSize.y * 0.5f},
            label.rotationDegrees,
            labelFontSize,
            0.0f,
            label.textColor);
    }
}

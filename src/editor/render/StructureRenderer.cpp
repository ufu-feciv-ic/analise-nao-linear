#include "editor/render/EditorRendererInternal.h"

void EditorRenderer::RenderStructurePass(
    const ProjectDocument& document,
    const EditorState& state,
    const Camera2D& camera) const
{
    if (!state.view.showNodes && state.view.showBeams)
    {
        DrawNodes(document, state, camera);
    }

    if (state.view.showBeams)
    {
        DrawBeams(document, state, camera);
    }

    if (state.view.showNodes)
    {
        DrawNodes(document, state, camera);
    }

    DrawNodeHighlights(document, state, camera);
}

void EditorRenderer::DrawGrid(const Camera2D& camera)
{
    const Vector2 worldTopLeft = GetScreenToWorld2D(Vector2{0.0f, 0.0f}, camera);
    const Vector2 worldBottomRight = GetScreenToWorld2D(
        Vector2{static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())},
        camera);

    BeginMode2D(camera);

    if (m_gridTransitionActive)
    {
        float t = m_gridTransitionTime / kGridFadeDuration;
        t = SmoothStep(t);

        const float outgoingAlpha = 1.0f - t;
        const float incomingAlpha = kIncomingMinAlpha + (1.0f - kIncomingMinAlpha) * t;

        DrawGridLayer(
            worldTopLeft,
            worldBottomRight,
            camera.zoom,
            m_previousGridSpacing,
            outgoingAlpha);

        DrawGridLayer(
            worldTopLeft,
            worldBottomRight,
            camera.zoom,
            m_currentGridSpacing,
            incomingAlpha);
    }
    else
    {
        DrawGridLayer(
            worldTopLeft,
            worldBottomRight,
            camera.zoom,
            m_currentGridSpacing,
            1.0f);
    }

    EndMode2D();
}

void EditorRenderer::DrawNodes(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const
{
    constexpr float nodeRadius = 6.0f;
    constexpr float hiddenNodeRadius = 4.0f;

    const Color nodeColor = Color{70, 70, 70, 255};
    const Color hoveredNodeColor = Color{190, 208, 232, 255};
    const Color selectedNodeColor = Color{178, 205, 240, 255};
    const Color hiddenNodeColor = Color{146, 146, 146, 255};
    const std::vector<int> selectedNodeIds = MakeSortedUniqueIdList(state.selectionState.selection.nodeIds);

    BeginMode2D(camera);

    for (const Node& node : document.nodes)
    {
        const Vector2 center = Vector2{
            static_cast<float>(node.position.x),
            static_cast<float>(node.position.y)};

        const bool isSelected = ContainsSortedId(selectedNodeIds, node.id);
        const bool isHovered =
            state.hover.entity.type == EntityType::Node &&
            state.hover.entity.id == node.id;
        const float activeRadius = state.view.showNodes ? nodeRadius : hiddenNodeRadius;

        const Color fillColor =
            isSelected ? selectedNodeColor :
            (isHovered ? hoveredNodeColor :
                (state.view.showNodes ? nodeColor : hiddenNodeColor));
        DrawSmallNodeCircle(center, activeRadius / camera.zoom, fillColor);
    }

    EndMode2D();
}

void EditorRenderer::DrawNodeHighlights(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const
{
    if (!state.view.showNodes && !state.view.showBeams)
    {
        return;
    }

    if (state.hover.entity.type != EntityType::Node && !state.selectionState.selection.HasNodes())
    {
        return;
    }

    constexpr float nodeRadius = 6.0f;
    constexpr float hiddenNodeRadius = 4.0f;
    const Color selectedOutlineColor = Color{70, 145, 255, 255};
    const Color hoveredOutlineColor = Color{120, 150, 195, 255};
    const std::vector<int> selectedNodeIds = MakeSortedUniqueIdList(state.selectionState.selection.nodeIds);

    BeginMode2D(camera);

    auto drawNodeOutline = [&](Vector2 center, float radius, Color color)
    {
        const float outlineThickness = 1.0f / camera.zoom;
        DrawSmallNodeCircleOutline(center, radius, outlineThickness, color);
        DrawSmallNodeCircleOutline(center, radius + outlineThickness, outlineThickness, color);
    };

    for (const Node& node : document.nodes)
    {
        const bool isSelected = ContainsSortedId(selectedNodeIds, node.id);
        const bool isHovered =
            state.hover.entity.type == EntityType::Node &&
            state.hover.entity.id == node.id;
        if (!isSelected && !isHovered)
        {
            continue;
        }

        const Vector2 center = Vector2{
            static_cast<float>(node.position.x),
            static_cast<float>(node.position.y)};
        const float activeRadius = state.view.showNodes ? nodeRadius : hiddenNodeRadius;

        if (isSelected)
        {
            drawNodeOutline(center, activeRadius / camera.zoom, selectedOutlineColor);
        }
        else
        {
            drawNodeOutline(center, activeRadius / camera.zoom, hoveredOutlineColor);
        }
    }

    EndMode2D();
}

void EditorRenderer::DrawShadows(
    const ProjectDocument& document,
    const ProjectDerivedData& derivedData,
    const Camera2D& camera) const
{
    if (document.beams.empty() || document.nodes.empty() || derivedData.beamGroups.empty())
    {
        return;
    }

    constexpr float shadowThickness = 8.4f;
    const float shadowRadius = (shadowThickness * 0.5f) / camera.zoom;
    const float cullMargin = shadowThickness / std::max(camera.zoom, 0.0001f) + 1.0f;
    const Color shadowColor = Color{240, 240, 240, 255};
    const Vector2 worldTopLeft = GetScreenToWorld2D(Vector2{0.0f, 0.0f}, camera);
    const Vector2 worldBottomRight = GetScreenToWorld2D(
        Vector2{static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())},
        camera);
    const float minVisibleX = std::min(worldTopLeft.x, worldBottomRight.x) - cullMargin;
    const float maxVisibleX = std::max(worldTopLeft.x, worldBottomRight.x) + cullMargin;
    const float minVisibleY = std::min(worldTopLeft.y, worldBottomRight.y) - cullMargin;
    const float maxVisibleY = std::max(worldTopLeft.y, worldBottomRight.y) + cullMargin;

    BeginMode2D(camera);

    for (const ProjectDerivedData::BeamGroup& group : derivedData.beamGroups)
    {
        if (group.shadowSegments.empty() && group.shadowNodePositions.empty())
        {
            continue;
        }

        for (const ProjectDerivedData::BeamGroup::ShadowSegment& segment : group.shadowSegments)
        {
            const Vector2 shadowStart = ToVector2(segment.start);
            const Vector2 shadowEnd = ToVector2(segment.end);
            const float beamMinX = std::min(shadowStart.x, shadowEnd.x);
            const float beamMaxX = std::max(shadowStart.x, shadowEnd.x);
            const float beamMinY = std::min(shadowStart.y, shadowEnd.y);
            const float beamMaxY = std::max(shadowStart.y, shadowEnd.y);
            if (beamMaxX < minVisibleX || beamMinX > maxVisibleX ||
                beamMaxY < minVisibleY || beamMinY > maxVisibleY)
            {
                continue;
            }

            DrawLineEx(shadowStart, shadowEnd, shadowThickness / camera.zoom, shadowColor);
        }

        for (const Point2D& shadowNodePosition : group.shadowNodePositions)
        {
            const Vector2 shadowPosition = ToVector2(shadowNodePosition);

            if (shadowPosition.x < minVisibleX || shadowPosition.x > maxVisibleX ||
                shadowPosition.y < minVisibleY || shadowPosition.y > maxVisibleY)
            {
                continue;
            }

            DrawShadowCircle(shadowPosition, shadowRadius, shadowColor);
        }
    }

    EndMode2D();
}

void EditorRenderer::DrawBeams(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const
{
    struct BeamRenderSegment
    {
        int beamId = -1;
        int startNodeId = -1;
        int endNodeId = -1;
        int materialId = -1;
        int sectionId = -1;
        Vector2 start;
        Vector2 end;
        Color fillColor;
        Color lowerEdgeColor;
        Color upperEdgeColor;
    };

    constexpr float beamThickness = 6.0f;
    constexpr float lowerBeamEdgeThickness = 4.0f;
    constexpr float upperBeamEdgeThickness = 4.0f;

    const Color beamColor = Color{146, 146, 146, 255};
    const Color beamLowerEdgeColor = Color{115, 115, 115, 255};
    const Color beamUpperEdgeColor = Color{215, 215, 215, 255};
    const Color hoveredBeamColor = Color{190, 208, 232, 255};
    const Color hoveredBeamLowerEdgeColor = Color{120, 150, 195, 255};
    const Color hoveredBeamUpperEdgeColor = Color{232, 240, 250, 255};
    const Color selectedBeamColor = Color{178, 205, 240, 255};
    const Color selectedBeamLowerEdgeColor = Color{70, 145, 255, 255};
    const Color selectedBeamUpperEdgeColor = Color{224, 236, 252, 255};
    const Color beamLabelColor = Color{80, 80, 80, 255};
    const float cullMargin =
        (beamThickness + std::max(lowerBeamEdgeThickness, upperBeamEdgeThickness)) / std::max(camera.zoom, 0.0001f) + 1.0f;
    const Vector2 worldTopLeft = GetScreenToWorld2D(Vector2{0.0f, 0.0f}, camera);
    const Vector2 worldBottomRight = GetScreenToWorld2D(
        Vector2{static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())},
        camera);
    const float minVisibleX = std::min(worldTopLeft.x, worldBottomRight.x) - cullMargin;
    const float maxVisibleX = std::max(worldTopLeft.x, worldBottomRight.x) + cullMargin;
    const float minVisibleY = std::min(worldTopLeft.y, worldBottomRight.y) - cullMargin;
    const float maxVisibleY = std::max(worldTopLeft.y, worldBottomRight.y) + cullMargin;
    std::vector<BeamRenderSegment> segments;
    segments.reserve(document.beams.size());
    std::vector<BeamTextLabelJob> labelJobs;
    labelJobs.reserve(document.beams.size() * 2);
    std::unordered_map<int, bool> activeMaterialLabelIds;
    std::unordered_map<int, bool> activeSectionLabelIds;
    activeMaterialLabelIds.reserve(document.beams.size());
    activeSectionLabelIds.reserve(document.beams.size());
    const std::vector<int> selectedBeamIds = MakeSortedUniqueIdList(state.selectionState.selection.beamIds);
    const Font labelFont = m_hasDimensionFont ? m_dimensionFont : GetFontDefault();
    constexpr float labelFontSize = 16.0f;
    constexpr float labelOffsetPixels = 13.0f;
    const float deltaTime = GetFrameTime();
    struct CachedTextMeasure
    {
        Vector2 size{};
    };
    std::unordered_map<std::string, CachedTextMeasure> textMeasureCache;
    textMeasureCache.reserve((state.view.showBeamMaterials || state.view.showBeamSections) ? 32 : 0);
    auto getTextMeasure = [&](const std::string& text) -> const CachedTextMeasure&
    {
        auto [it, inserted] = textMeasureCache.emplace(text, CachedTextMeasure{});
        if (inserted)
        {
            it->second.size = MeasureTextEx(labelFont, text.c_str(), labelFontSize, 0.0f);
        }

        return it->second;
    };

    BeginMode2D(camera);

    for (const Beam& beam : document.beams)
    {
        const Node* startNode = document.FindNodeById(beam.startNodeId);
        const Node* endNode = document.FindNodeById(beam.endNodeId);
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
        const float beamMinX = std::min(start.x, end.x);
        const float beamMaxX = std::max(start.x, end.x);
        const float beamMinY = std::min(start.y, end.y);
        const float beamMaxY = std::max(start.y, end.y);
        if (beamMaxX < minVisibleX || beamMinX > maxVisibleX ||
            beamMaxY < minVisibleY || beamMinY > maxVisibleY)
        {
            continue;
        }

        const bool isSelected = ContainsSortedId(selectedBeamIds, beam.id);
        const bool isHovered =
            state.hover.entity.type == EntityType::Beam &&
            state.hover.entity.id == beam.id;
        const Color fillColor =
            isSelected ? selectedBeamColor : (isHovered ? hoveredBeamColor : beamColor);
        const Color lowerEdgeColor =
            isSelected ? selectedBeamLowerEdgeColor : (isHovered ? hoveredBeamLowerEdgeColor : beamLowerEdgeColor);
        const Color upperEdgeColor =
            isSelected ? selectedBeamUpperEdgeColor : (isHovered ? hoveredBeamUpperEdgeColor : beamUpperEdgeColor);
        segments.push_back(BeamRenderSegment{
            beam.id,
            beam.startNodeId,
            beam.endNodeId,
            beam.materialId,
            beam.sectionId,
            start,
            end,
            fillColor,
            lowerEdgeColor,
            upperEdgeColor});
    }

    for (const BeamRenderSegment& segment : segments)
    {
        const Vector2 direction = NormalizeVector(Vector2{
            segment.end.x - segment.start.x,
            segment.end.y - segment.start.y});
        const Vector2 normal = Vector2{-direction.y, direction.x};
        const float edgeOffset = (beamThickness * 0.5f) / camera.zoom;

        const Vector2 upperStart = Vector2{
            segment.start.x + normal.x * edgeOffset,
            segment.start.y + normal.y * edgeOffset};
        const Vector2 upperEnd = Vector2{
            segment.end.x + normal.x * edgeOffset,
            segment.end.y + normal.y * edgeOffset};
        const Vector2 lowerStart = Vector2{
            segment.start.x - normal.x * edgeOffset,
            segment.start.y - normal.y * edgeOffset};
        const Vector2 lowerEnd = Vector2{
            segment.end.x - normal.x * edgeOffset,
            segment.end.y - normal.y * edgeOffset};

        DrawLineEx(lowerStart, lowerEnd, upperBeamEdgeThickness / camera.zoom, segment.upperEdgeColor);
        DrawLineEx(upperStart, upperEnd, lowerBeamEdgeThickness / camera.zoom, segment.lowerEdgeColor);
    }

    for (const BeamRenderSegment& segment : segments)
    {
        DrawLineEx(segment.start, segment.end, beamThickness / camera.zoom, segment.fillColor);
    }

    EndMode2D();

    auto enqueueBeamLabel = [&](int beamId, bool isMaterialLabel, Vector2 screenCenter, const std::string& text, float rotationDegrees, float segmentLengthPixels)
    {
        if (text.empty())
        {
            return;
        }

        const Vector2 textSize = getTextMeasure(text).size;
        std::unordered_map<int, float>& fadeMap = isMaterialLabel ? m_beamMaterialTextFadeById : m_beamSectionTextFadeById;
        std::unordered_map<int, bool>& activeIds = isMaterialLabel ? activeMaterialLabelIds : activeSectionLabelIds;
        activeIds[beamId] = true;

        auto [it, inserted] = fadeMap.emplace(beamId, 1.0f);
        float& storedAlpha = it->second;
        if (inserted)
        {
            storedAlpha = 1.0f;
        }

        const float fadeOutThreshold = textSize.x * kBeamLabelFadeOutThresholdRatio;
        const float fadeInThreshold = textSize.x * kBeamLabelFadeInThresholdRatio;
        const float step = deltaTime / kDimensionFadeDuration;
        if (segmentLengthPixels < fadeOutThreshold)
        {
            storedAlpha = std::max(0.0f, storedAlpha - step);
        }
        else if (segmentLengthPixels > fadeInThreshold)
        {
            storedAlpha = std::min(1.0f, storedAlpha + step);
        }

        if (storedAlpha <= 0.01f)
        {
            return;
        }

        labelJobs.push_back(BeamTextLabelJob{
            beamId,
            isMaterialLabel,
            screenCenter,
            text,
            textSize,
            rotationDegrees,
            segmentLengthPixels,
            WithScaledAlpha(beamLabelColor, storedAlpha)});
    };

    if (state.view.showBeamMaterials || state.view.showBeamSections)
    {
        for (const BeamRenderSegment& segment : segments)
        {
            const Vector2 startScreen = GetWorldToScreen2D(segment.start, camera);
            const Vector2 endScreen = GetWorldToScreen2D(segment.end, camera);
            const Vector2 screenDelta = Vector2{endScreen.x - startScreen.x, endScreen.y - startScreen.y};
            const float segmentLengthPixels = VectorLength(screenDelta);
            if (segmentLengthPixels <= 1.0e-6f)
            {
                continue;
            }

            const Vector2 direction = NormalizeVector(screenDelta);
            const Vector2 normal = Vector2{-direction.y, direction.x};
            const Vector2 midPoint = Vector2{
                (startScreen.x + endScreen.x) * 0.5f,
                (startScreen.y + endScreen.y) * 0.5f};
            const float rotationDegrees = NormalizeReadableAngle(atan2f(direction.y, direction.x) * RAD2DEG);

            if (state.view.showBeamMaterials)
            {
                const StructuralMaterial* material = document.FindMaterialById(segment.materialId);
                if (material != nullptr)
                {
                    enqueueBeamLabel(
                        segment.beamId,
                        true,
                        Vector2{midPoint.x - normal.x * labelOffsetPixels, midPoint.y - normal.y * labelOffsetPixels},
                        material->name,
                        rotationDegrees,
                        segmentLengthPixels);
                }
            }

            if (state.view.showBeamSections)
            {
                const Section* section = document.FindSectionById(segment.sectionId);
                if (section != nullptr)
                {
                    enqueueBeamLabel(
                        segment.beamId,
                        false,
                        Vector2{midPoint.x + normal.x * labelOffsetPixels, midPoint.y + normal.y * labelOffsetPixels},
                        section->name,
                        rotationDegrees,
                        segmentLengthPixels);
                }
            }
        }
    }

    auto pruneBeamLabelFadeMap = [](std::unordered_map<int, float>& fadeMap, const std::unordered_map<int, bool>& activeIds)
    {
        for (auto it = fadeMap.begin(); it != fadeMap.end();)
        {
            if (activeIds.find(it->first) == activeIds.end())
            {
                it = fadeMap.erase(it);
            }
            else
            {
                ++it;
            }
        }
    };

    pruneBeamLabelFadeMap(m_beamMaterialTextFadeById, activeMaterialLabelIds);
    pruneBeamLabelFadeMap(m_beamSectionTextFadeById, activeSectionLabelIds);

    for (const BeamTextLabelJob& label : labelJobs)
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

void EditorRenderer::DrawBeamPreview(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const
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

    constexpr float beamThickness = 6.5f;
    constexpr float beamOutlineThickness = 9.0f;

    const Vector2 start = Vector2{
        static_cast<float>(startNode->position.x),
        static_cast<float>(startNode->position.y)};

    const Color previewFillColor = Color{150, 150, 150, 220};
    const Color previewOutlineColor = Color{85, 85, 85, 220};

    BeginMode2D(camera);

    DrawCircleV(
        state.beamTool.beamPreviewPointWorld,
        (beamOutlineThickness * 0.5f) / camera.zoom,
        previewOutlineColor);
    DrawLineEx(start, state.beamTool.beamPreviewPointWorld, beamOutlineThickness / camera.zoom, previewOutlineColor);
    DrawCircleV(
        state.beamTool.beamPreviewPointWorld,
        (beamThickness * 0.5f) / camera.zoom,
        previewFillColor);
    DrawLineEx(start, state.beamTool.beamPreviewPointWorld, beamThickness / camera.zoom, previewFillColor);

    EndMode2D();
}

void EditorRenderer::DrawMovePreview(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const
{
    struct BeamPreviewSegment
    {
        Vector2 start;
        Vector2 end;
    };

    if (state.transformTool.moveSelectionStep != TransformToolState::MoveSelectionStep::AwaitTargetPoint)
    {
        return;
    }

    constexpr float crossHalfSize = 10.0f;
    constexpr float lineThickness = 2.0f;
    constexpr float beamThickness = 6.5f;
    constexpr float beamOutlineThickness = 9.0f;
    constexpr float nodeRadius = 6.0f;

    const Vector2 basePoint = ToVector2(state.transformTool.moveBasePointWorld);
    const Vector2 previewPoint = ToVector2(state.transformTool.movePreviewPointWorld);
    const Vector2 delta = Vector2{
        previewPoint.x - basePoint.x,
        previewPoint.y - basePoint.y};
    const std::vector<int> movedNodeIds = CollectPreviewTransformNodeIds(document, state);
    if (movedNodeIds.empty())
    {
        return;
    }
    const bool isCopyPreview = state.activeTool == EditorTool::CopySelection;
    const bool isMirrorPreview = state.activeTool == EditorTool::MirrorSelection;
    const bool previewCreatesDetachedCopy =
        isCopyPreview || (isMirrorPreview && state.transformTool.mirrorKeepsOriginal);
    const std::vector<int> selectedBeamIds =
        previewCreatesDetachedCopy ? MakeSortedUniqueIdList(state.selectionState.selection.beamIds) : std::vector<int>{};
    Vector2 reflectedPoint{};
    const bool canMirrorPreview =
        !isMirrorPreview ||
        TryReflectPointAcrossLine(basePoint, basePoint, previewPoint, reflectedPoint);

    const Color markerColor = Color{70, 145, 255, 255};
    const Color lineColor = Color{70, 145, 255, 220};
    const Color previewBeamOutlineColor = Color{70, 145, 255, 255};
    const Color previewBeamFillColor = Color{156, 206, 255, 255};
    const Color previewNodeColor = Color{70, 145, 255, 255};
    std::vector<BeamPreviewSegment> previewSegments;
    previewSegments.reserve(document.beams.size());

    BeginMode2D(camera);

    for (const Beam& beam : document.beams)
    {
        if (previewCreatesDetachedCopy && !ContainsSortedId(selectedBeamIds, beam.id))
        {
            continue;
        }

        const Node* startNode = document.FindNodeById(beam.startNodeId);
        const Node* endNode = document.FindNodeById(beam.endNodeId);
        if (startNode == nullptr || endNode == nullptr)
        {
            continue;
        }

        const bool movesStartNode = ContainsNodeId(movedNodeIds, startNode->id);
        const bool movesEndNode = ContainsNodeId(movedNodeIds, endNode->id);
        if (!movesStartNode && !movesEndNode)
        {
            continue;
        }

        Vector2 start = Vector2{
            static_cast<float>(startNode->position.x),
            static_cast<float>(startNode->position.y)};
        Vector2 end = Vector2{
            static_cast<float>(endNode->position.x),
            static_cast<float>(endNode->position.y)};

        if (isMirrorPreview)
        {
            if (!canMirrorPreview)
            {
                continue;
            }

            if (movesStartNode)
            {
                TryReflectPointAcrossLine(start, basePoint, previewPoint, start);
            }

            if (movesEndNode)
            {
                TryReflectPointAcrossLine(end, basePoint, previewPoint, end);
            }
        }
        else
        {
            start.x += movesStartNode ? delta.x : 0.0f;
            start.y += movesStartNode ? delta.y : 0.0f;
            end.x += movesEndNode ? delta.x : 0.0f;
            end.y += movesEndNode ? delta.y : 0.0f;
        }

        previewSegments.push_back(BeamPreviewSegment{start, end});
    }

    for (const BeamPreviewSegment& segment : previewSegments)
    {
        DrawLineEx(segment.start, segment.end, beamOutlineThickness / camera.zoom, previewBeamOutlineColor);
    }

    for (const BeamPreviewSegment& segment : previewSegments)
    {
        DrawLineEx(segment.start, segment.end, beamThickness / camera.zoom, previewBeamFillColor);
    }

    for (int nodeId : movedNodeIds)
    {
        const Node* node = document.FindNodeById(nodeId);
        if (node == nullptr)
        {
            continue;
        }

        Vector2 center = Vector2{
            static_cast<float>(node->position.x),
            static_cast<float>(node->position.y)};
        if (isMirrorPreview)
        {
            if (!canMirrorPreview)
            {
                continue;
            }

            if (!TryReflectPointAcrossLine(center, basePoint, previewPoint, center))
            {
                continue;
            }
        }
        else
        {
            center.x += delta.x;
            center.y += delta.y;
        }
        DrawSmallNodeCircle(center, nodeRadius / camera.zoom, previewNodeColor);
    }

    DrawLineEx(basePoint, previewPoint, lineThickness / camera.zoom, lineColor);

    DrawLineEx(
        Vector2{basePoint.x - crossHalfSize / camera.zoom, basePoint.y},
        Vector2{basePoint.x + crossHalfSize / camera.zoom, basePoint.y},
        lineThickness / camera.zoom,
        markerColor);

    DrawLineEx(
        Vector2{basePoint.x, basePoint.y - crossHalfSize / camera.zoom},
        Vector2{basePoint.x, basePoint.y + crossHalfSize / camera.zoom},
        lineThickness / camera.zoom,
        markerColor);

    if (isMirrorPreview)
    {
        DrawLineEx(
            Vector2{previewPoint.x - crossHalfSize / camera.zoom, previewPoint.y},
            Vector2{previewPoint.x + crossHalfSize / camera.zoom, previewPoint.y},
            lineThickness / camera.zoom,
            markerColor);

        DrawLineEx(
            Vector2{previewPoint.x, previewPoint.y - crossHalfSize / camera.zoom},
            Vector2{previewPoint.x, previewPoint.y + crossHalfSize / camera.zoom},
            lineThickness / camera.zoom,
            markerColor);
    }

    EndMode2D();
}

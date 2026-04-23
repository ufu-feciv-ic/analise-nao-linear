#include "editor/render/EditorRendererInternal.h"

void EditorRenderer::DrawPointLoads(
    const ProjectDocument& document,
    const ProjectDerivedData& derivedData,
    const EditorState& state,
    const Camera2D& camera) const
{
    if (!state.view.showPointLoads)
    {
        return;
    }

    constexpr float cullMargin = 56.0f;
    constexpr float lineThickness = 3.0f;
    constexpr float forceArrowHeadWidth = 5.0f;
    constexpr float momentArrowHeadWidth = 5.0f;
    constexpr float momentOpeningDegrees = 120.0f;
    constexpr float momentPixelsPerSegment = 4.0f;
    constexpr float textFontSize = 20.0f;
    constexpr float textSpacing = 0.0f;
    constexpr float nodeClearancePixels = 3.0f;
    constexpr float minimumArrowLengthPixels = 14.4f;
    constexpr float minimumMomentRadiusPixels = 12.0f;
    constexpr float loadLabelAlongOffset = 4.0f;
    constexpr float loadLabelNormalOffset = 8.0f;
    const float minimumLabelReferenceLengthPixels = lineThickness * 5.0f * 1.5f;
    const Color loadColor = Color{210, 15, 15, 255};
    const Color textColor = Color{20, 20, 20, 255};
    const Font textFont = m_hasDimensionFont ? m_dimensionFont : GetFontDefault();
    const float forceMaxPixels = std::max(1.0f, state.view.pointLoadForceMaxPixels);
    const float momentMaxPixels = std::max(1.0f, state.view.pointLoadMomentMaxPixels);
    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    struct CachedTextMeasure
    {
        Vector2 size{};
        Vector2 positionSize{};
    };
    std::unordered_map<std::string, CachedTextMeasure> textMeasureCache;
    textMeasureCache.reserve(document.nodes.size() * 2);
    auto getTextMeasure = [&](const std::string& text) -> const CachedTextMeasure&
    {
        auto [it, inserted] = textMeasureCache.emplace(text, CachedTextMeasure{});
        if (inserted)
        {
            it->second.size = MeasureTextEx(textFont, text.c_str(), textFontSize, textSpacing);
            it->second.positionSize =
                GetTextPositionSizeWithoutGlyphPadding(textFont, textFontSize, it->second.size);
        }

        return it->second;
    };

    double maxForceForScale = 0.0;
    double maxMomentForScale = 0.0;
    for (const Node& node : document.nodes)
    {
        if (state.view.showPointLoadResultant)
        {
            maxForceForScale = std::max(
                maxForceForScale,
                hypot(node.load.fx, node.load.fy));
        }
        else
        {
            maxForceForScale = std::max(
                maxForceForScale,
                std::max(std::abs(node.load.fx), std::abs(node.load.fy)));
        }

        maxMomentForScale = std::max(maxMomentForScale, std::abs(node.load.mz));
    }

    if (forceMaxPixels <= 0.0f && momentMaxPixels <= 0.0f)
    {
        return;
    }

    for (const Node& node : document.nodes)
    {
        if (!HasVisibleNodalLoad(node.load))
        {
            continue;
        }

        const ProjectDerivedData::NodeConnectivity& connectivity = GetNodeConnectivity(derivedData, node.id);

        const Vector2 screenCenter = GetWorldToScreen2D(
            Vector2{
                static_cast<float>(node.position.x),
                static_cast<float>(node.position.y)},
            camera);

        if (screenCenter.x < -cullMargin ||
            screenCenter.x > screenWidth + cullMargin ||
            screenCenter.y < -cullMargin ||
            screenCenter.y > screenHeight + cullMargin)
        {
            continue;
        }

        if (state.view.showPointLoadResultant)
        {
            const double resultantMagnitude = hypot(node.load.fx, node.load.fy);
            if (resultantMagnitude > 1.0e-9 && maxForceForScale > 1.0e-9 && forceMaxPixels > 0.0f)
            {
                const float scaledForceLength =
                    static_cast<float>(resultantMagnitude / maxForceForScale) * forceMaxPixels;
                const float forceLength = std::max(minimumArrowLengthPixels, scaledForceLength);
                const std::string label = FormatDisplayForce(resultantMagnitude, document.displayUnits.force);
                const CachedTextMeasure& labelMeasure = getTextMeasure(label);
                const Vector2 labelSize = labelMeasure.size;
                const float labelReferenceLength =
                    std::max(forceLength, minimumLabelReferenceLengthPixels);
                const bool isPureHorizontal = std::abs(node.load.fy) <= 1.0e-9;
                const bool isPureVertical = std::abs(node.load.fx) <= 1.0e-9;
                Vector2 tail{};
                Vector2 tip{};
                Vector2 labelTail{};
                Vector2 labelTip{};
                bool useNormalBasedHorizontalAnchor = true;

                if (isPureHorizontal)
                {
                    const float directionSign = node.load.fx >= 0.0 ? 1.0f : -1.0f;
                    const Vector2 direction = Vector2{directionSign, 0.0f};
                    const Vector2 placementDirection =
                        ChooseHorizontalForcePlacementDirection(connectivity, direction.x);
                    const bool startsAtNode =
                        placementDirection.x * direction.x + placementDirection.y * direction.y > 0.0f;
                    ComputeAnchoredPointLoadSegment(
                        screenCenter,
                        placementDirection,
                        direction,
                        nodeClearancePixels,
                        forceLength,
                        tail,
                        tip);

                    const Vector2 nodeAnchor = startsAtNode ? tail : tip;
                    const Vector2 directionTowardNode = startsAtNode
                        ? Vector2{-direction.x, -direction.y}
                        : direction;
                    ComputeLoadLabelReferenceSegment(
                        nodeAnchor,
                        directionTowardNode,
                        labelReferenceLength,
                        labelTail,
                        labelTip);
                    useNormalBasedHorizontalAnchor = false;
                }
                else if (isPureVertical)
                {
                    const float directionSign = node.load.fy >= 0.0 ? -1.0f : 1.0f;
                    const Vector2 direction = Vector2{0.0f, directionSign};
                    const Vector2 placementDirection =
                        ChooseVerticalForcePlacementDirection(connectivity, direction.y);
                    const bool startsAtNode =
                        placementDirection.x * direction.x + placementDirection.y * direction.y > 0.0f;
                    ComputeAnchoredPointLoadSegment(
                        screenCenter,
                        placementDirection,
                        direction,
                        nodeClearancePixels,
                        forceLength,
                        tail,
                        tip);

                    const Vector2 nodeAnchor = startsAtNode ? tail : tip;
                    const Vector2 directionTowardNode = startsAtNode
                        ? Vector2{-direction.x, -direction.y}
                        : direction;
                    ComputeLoadLabelReferenceSegment(
                        nodeAnchor,
                        directionTowardNode,
                        labelReferenceLength,
                        labelTail,
                        labelTip);
                    useNormalBasedHorizontalAnchor = false;
                }
                else
                {
                    const Vector2 screenDirection = NormalizeVectorSafe(
                        Vector2{
                            static_cast<float>(node.load.fx),
                            static_cast<float>(-node.load.fy)});
                    tip = Vector2{
                        screenCenter.x - screenDirection.x * nodeClearancePixels,
                        screenCenter.y - screenDirection.y * nodeClearancePixels};
                    tail = Vector2{
                        tip.x - screenDirection.x * forceLength,
                        tip.y - screenDirection.y * forceLength};
                    labelTail = Vector2{
                        tip.x - screenDirection.x * labelReferenceLength,
                        tip.y - screenDirection.y * labelReferenceLength};
                    labelTip = tip;
                }

                DrawLoadArrow(tail, tip, lineThickness, loadColor, forceArrowHeadWidth);

                const Vector2 labelPosition = ComputeLoadLabelPosition(
                    labelTail,
                    labelTip,
                    labelSize,
                    loadLabelAlongOffset,
                    loadLabelNormalOffset,
                    useNormalBasedHorizontalAnchor);
                if (!state.view.suppressPointLoadText)
                {
                    DrawTextEx(
                        textFont,
                        label.c_str(),
                        labelPosition,
                        textFontSize,
                        textSpacing,
                        textColor);
                }
            }
        }
        else
        {
            if (HasNodalLoadComponent(node.load.fx) && maxForceForScale > 1.0e-9 && forceMaxPixels > 0.0f)
            {
                const float directionSign = node.load.fx >= 0.0 ? 1.0f : -1.0f;
                const Vector2 direction = Vector2{directionSign, 0.0f};
                const Vector2 placementDirection =
                    ChooseHorizontalForcePlacementDirection(connectivity, direction.x);
                const float scaledForceLength =
                    static_cast<float>(std::abs(node.load.fx) / maxForceForScale) * forceMaxPixels;
                const float forceLength = std::max(minimumArrowLengthPixels, scaledForceLength);
                Vector2 tail{};
                Vector2 tip{};
                const bool startsAtNode =
                    placementDirection.x * direction.x + placementDirection.y * direction.y > 0.0f;
                ComputeAnchoredPointLoadSegment(
                    screenCenter,
                    placementDirection,
                    direction,
                    nodeClearancePixels,
                    forceLength,
                    tail,
                    tip);

                DrawLoadArrow(tail, tip, lineThickness, loadColor, forceArrowHeadWidth);

                const std::string label = FormatDisplayForce(node.load.fx, document.displayUnits.force);
                const Vector2 labelSize = getTextMeasure(label).size;
                const float labelReferenceLength =
                    std::max(forceLength, minimumLabelReferenceLengthPixels);
                const Vector2 nodeAnchor = startsAtNode ? tail : tip;
                const Vector2 directionTowardNode = startsAtNode
                    ? Vector2{-direction.x, -direction.y}
                    : direction;
                Vector2 labelTail{};
                Vector2 labelTip{};
                ComputeLoadLabelReferenceSegment(
                    nodeAnchor,
                    directionTowardNode,
                    labelReferenceLength,
                    labelTail,
                    labelTip);
                if (!state.view.suppressPointLoadText)
                {
                    DrawTextEx(
                        textFont,
                        label.c_str(),
                        ComputeLoadLabelPosition(
                            labelTail,
                            labelTip,
                            labelSize,
                            loadLabelAlongOffset,
                            loadLabelNormalOffset),
                        textFontSize,
                        textSpacing,
                        textColor);
                }
            }

            if (HasNodalLoadComponent(node.load.fy) && maxForceForScale > 1.0e-9 && forceMaxPixels > 0.0f)
            {
                const float directionSign = node.load.fy >= 0.0 ? -1.0f : 1.0f;
                const Vector2 direction = Vector2{0.0f, directionSign};
                const Vector2 placementDirection =
                    ChooseVerticalForcePlacementDirection(connectivity, direction.y);
                const float scaledForceLength =
                    static_cast<float>(std::abs(node.load.fy) / maxForceForScale) * forceMaxPixels;
                const float forceLength = std::max(minimumArrowLengthPixels, scaledForceLength);
                Vector2 tail{};
                Vector2 tip{};
                const bool startsAtNode =
                    placementDirection.x * direction.x + placementDirection.y * direction.y > 0.0f;
                ComputeAnchoredPointLoadSegment(
                    screenCenter,
                    placementDirection,
                    direction,
                    nodeClearancePixels,
                    forceLength,
                    tail,
                    tip);

                DrawLoadArrow(tail, tip, lineThickness, loadColor, forceArrowHeadWidth);

                const std::string label = FormatDisplayForce(node.load.fy, document.displayUnits.force);
                const Vector2 labelSize = getTextMeasure(label).size;
                const float labelReferenceLength =
                    std::max(forceLength, minimumLabelReferenceLengthPixels);
                const Vector2 nodeAnchor = startsAtNode ? tail : tip;
                const Vector2 directionTowardNode = startsAtNode
                    ? Vector2{-direction.x, -direction.y}
                    : direction;
                Vector2 labelTail{};
                Vector2 labelTip{};
                ComputeLoadLabelReferenceSegment(
                    nodeAnchor,
                    directionTowardNode,
                    labelReferenceLength,
                    labelTail,
                    labelTip);
                if (!state.view.suppressPointLoadText)
                {
                    DrawTextEx(
                        textFont,
                        label.c_str(),
                        ComputeLoadLabelPosition(
                            labelTail,
                            labelTip,
                            labelSize,
                            loadLabelAlongOffset,
                            loadLabelNormalOffset),
                        textFontSize,
                        textSpacing,
                        textColor);
                }
            }
        }

        if (HasNodalLoadComponent(node.load.mz) && maxMomentForScale > 1.0e-9 && momentMaxPixels > 0.0f)
        {
            const float scaledMomentRadius =
                static_cast<float>(std::abs(node.load.mz) / maxMomentForScale) * momentMaxPixels * 0.5f;
            const float momentRadius = std::max(minimumMomentRadiusPixels, scaledMomentRadius);
            const float halfOpening = momentOpeningDegrees * 0.5f;
            const bool isPositiveMoment = node.load.mz >= 0.0;
            const Vector2 momentDirection = ChooseMomentArcDirection(connectivity);
            const float centerAngleDegrees = atan2f(momentDirection.y, momentDirection.x) * RAD2DEG;
            DrawMomentLoad(
                momentRadius,
                momentPixelsPerSegment,
                isPositiveMoment,
                screenCenter.x,
                screenCenter.y,
                isPositiveMoment ? (centerAngleDegrees + halfOpening) : (centerAngleDegrees - halfOpening),
                isPositiveMoment ? (centerAngleDegrees - halfOpening) : (centerAngleDegrees + halfOpening),
                lineThickness,
                momentArrowHeadWidth,
                loadColor);

            const std::string label = FormatDisplayMoment(node.load.mz, document.displayUnits.moment);
            const CachedTextMeasure& labelMeasure = getTextMeasure(label);
            const Vector2 labelSize = labelMeasure.size;
            const Vector2 labelPositionSize = labelMeasure.positionSize;
            const float labelAdditionalOffset =
                ComputeMomentLabelAdditionalOffset(labelPositionSize, momentDirection, momentRadius) + 8.0f;
            const Vector2 labelCenter = Vector2{
                screenCenter.x + momentDirection.x * (momentRadius + labelAdditionalOffset),
                screenCenter.y + momentDirection.y * (momentRadius + labelAdditionalOffset)};
            if (!state.view.suppressPointLoadText)
            {
                DrawTextEx(
                    textFont,
                    label.c_str(),
                    Vector2{
                        labelCenter.x - labelSize.x * 0.5f,
                        labelCenter.y - labelSize.y * 0.5f},
                    textFontSize,
                    textSpacing,
                    textColor);
            }
        }
    }
}

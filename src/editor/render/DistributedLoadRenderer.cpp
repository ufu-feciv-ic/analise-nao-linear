#include "editor/render/EditorRendererInternal.h"

void EditorRenderer::DrawDistributedLoads(
    const ProjectDocument& document,
    const EditorState& state,
    const Camera2D& camera,
    float zoomTarget) const
{
    if (!state.view.showPointLoads || document.distributedLoads.empty())
    {
        return;
    }

    constexpr float cullMargin = 72.0f;
    constexpr float beamHalfThicknessPixels = 3.0f;
    constexpr float lineThickness = 3.0f;
    constexpr float topLineThickness = 2.6f;
    constexpr float arrowHeadWidth = 5.0f;
    constexpr float textFontSize = 20.0f;
    constexpr float textSpacing = 0.0f;
    constexpr float arrowsPerSegmentSpacingPixels = 56.0f;
    constexpr float labelGapPixels = 5.0f;
    constexpr float inclinedEndpointAlongOffsetPixels = 10.0f;
    constexpr float distributedLoadTextFadeOutThresholdRatio = 1.0f;
    constexpr float distributedLoadTextFadeInThresholdRatio = 1.12f;

    const Color loadColor = Color{210, 15, 15, 255};
    const Color fillColor = Color{210, 15, 15, 52};
    const Color textColor = Color{20, 20, 20, 255};
    const Font textFont = m_hasDimensionFont ? m_dimensionFont : GetFontDefault();
    const float distributedMaxPixels = std::max(1.0f, state.view.distributedLoadMaxPixels);
    const float deltaTime = GetFrameTime();
    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    const bool suppressLoadText = state.view.suppressPointLoadText;
    std::unordered_map<std::uint64_t, bool> activeArrowFadeKeys;
    std::unordered_map<std::uint64_t, bool> activeTextFadeKeys;
    activeArrowFadeKeys.reserve(m_distributedLoadArrowFadeByKey.size() + document.distributedLoads.size() * 4);
    activeTextFadeKeys.reserve(m_distributedLoadTextFadeByKey.size() + document.distributedLoads.size() * 4);
    struct CachedTextMeasure
    {
        Vector2 size{};
        Vector2 positionSize{};
    };
    std::unordered_map<std::string, CachedTextMeasure> textMeasureCache;
    textMeasureCache.reserve(document.distributedLoads.size() * 4);
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

    if (suppressLoadText && !m_distributedLoadTextFadeByKey.empty())
    {
        m_distributedLoadTextFadeByKey.clear();
    }

    if (!m_distributedLoadZoomInitialized)
    {
        m_distributedLoadZoomInitialized = true;
        m_lastDistributedLoadZoom = zoomTarget;
        m_distributedLoadZoomIdleTime = kDistributedLoadArrowZoomSettleDelay;
    }
    else if (std::abs(zoomTarget - m_lastDistributedLoadZoom) > kDistributedLoadArrowZoomChangeInterval)
    {
        m_lastDistributedLoadZoom = zoomTarget;
        m_distributedLoadZoomIdleTime = 0.0f;
    }
    else
    {
        m_distributedLoadZoomIdleTime += deltaTime;
    }

    const bool allowArrowCountUpdates =
        m_distributedLoadZoomIdleTime >= kDistributedLoadArrowZoomSettleDelay;

    double maxDistributedMagnitudeForScale = 0.0;
    for (const BeamDistributedLoad& distributedLoad : document.distributedLoads)
    {
        if (state.view.showDistributedLoadResultant)
        {
            maxDistributedMagnitudeForScale = std::max(
                maxDistributedMagnitudeForScale,
                std::max(
                    std::hypot(distributedLoad.value.qxStart, distributedLoad.value.qyStart),
                    std::hypot(distributedLoad.value.qxEnd, distributedLoad.value.qyEnd)));
        }
        else
        {
            maxDistributedMagnitudeForScale = std::max(
                maxDistributedMagnitudeForScale,
                std::max({
                    std::abs(distributedLoad.value.qxStart),
                    std::abs(distributedLoad.value.qxEnd),
                    std::abs(distributedLoad.value.qyStart),
                    std::abs(distributedLoad.value.qyEnd)}));
        }
    }

    if (maxDistributedMagnitudeForScale <= 1.0e-9)
    {
        return;
    }

    auto lerpVector = [](Vector2 start, Vector2 end, double t)
    {
        const float blend = static_cast<float>(std::clamp(t, 0.0, 1.0));
        return Vector2{
            start.x + (end.x - start.x) * blend,
            start.y + (end.y - start.y) * blend};
    };

    auto scaledArrowLength = [&](double value)
    {
        if (std::abs(value) <= 1.0e-9)
        {
            return 0.0f;
        }

        const float scaledLength =
            static_cast<float>(std::abs(value) / maxDistributedMagnitudeForScale) * distributedMaxPixels;
        return scaledLength;
    };

    auto chooseDistributedStartCorner = [&](Vector2 textDirection, Vector2 offsetDirection)
    {
        const Vector2 textNormal = Vector2{-textDirection.y, textDirection.x};
        return (textNormal.x * offsetDirection.x + textNormal.y * offsetDirection.y) >= 0.0f
                   ? RotatedTextCorner::TopLeft
                   : RotatedTextCorner::BottomLeft;
    };

    auto chooseDistributedEndCorner = [&](Vector2 textDirection, Vector2 offsetDirection)
    {
        const Vector2 textNormal = Vector2{-textDirection.y, textDirection.x};
        return (textNormal.x * offsetDirection.x + textNormal.y * offsetDirection.y) >= 0.0f
                   ? RotatedTextCorner::TopRight
                   : RotatedTextCorner::BottomRight;
    };

    auto makeDistributedArrowFadeKey = [](int loadId, int channelId, double tStart, double tEnd)
    {
        const auto quantize = [](double t) -> std::uint64_t
        {
            const double clamped = std::clamp(t, 0.0, 1.0);
            return static_cast<std::uint64_t>(std::llround(clamped * 4096.0));
        };

        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(loadId)) << 32) |
               (static_cast<std::uint64_t>(channelId & 0xFF) << 24) |
               (quantize(tStart) << 12) |
               quantize(tEnd);
    };

    struct DistributedArrowFadeResolved
    {
        int previousCount = 2;
        int currentCount = 2;
        float previousAlpha = 0.0f;
        float currentAlpha = 1.0f;
    };

    auto resolveDistributedArrowFade = [&](std::uint64_t key, int targetCount)
    {
        activeArrowFadeKeys[key] = true;

        auto [it, inserted] = m_distributedLoadArrowFadeByKey.emplace(
            key,
            DistributedLoadArrowFadeState{targetCount, targetCount, targetCount, 1.0f, false});
        DistributedLoadArrowFadeState& fadeState = it->second;
        if (inserted)
        {
            return DistributedArrowFadeResolved{
                fadeState.previousCount,
                fadeState.currentCount,
                0.0f,
                1.0f};
        }

        fadeState.pendingCount = targetCount;

        if (!allowArrowCountUpdates)
        {
            if (fadeState.isTransitioning)
            {
                const int lockedCount =
                    fadeState.transitionAlpha >= 0.5f ? fadeState.currentCount : fadeState.previousCount;
                fadeState.previousCount = lockedCount;
                fadeState.currentCount = lockedCount;
                fadeState.pendingCount = targetCount;
                fadeState.transitionAlpha = 1.0f;
                fadeState.isTransitioning = false;
            }

            return DistributedArrowFadeResolved{
                fadeState.previousCount,
                fadeState.currentCount,
                0.0f,
                1.0f};
        }

        if (fadeState.pendingCount != fadeState.currentCount)
        {
            const int anchorCount =
                (fadeState.isTransitioning && fadeState.transitionAlpha < 0.5f)
                    ? fadeState.previousCount
                    : fadeState.currentCount;
            fadeState.previousCount = anchorCount;
            fadeState.currentCount = fadeState.pendingCount;
            fadeState.transitionAlpha = 0.0f;
            fadeState.isTransitioning = fadeState.previousCount != fadeState.currentCount;
            if (!fadeState.isTransitioning)
            {
                fadeState.transitionAlpha = 1.0f;
            }
        }

        if (fadeState.isTransitioning)
        {
            const float step = deltaTime / kDistributedLoadArrowFadeDuration;
            fadeState.transitionAlpha = std::min(1.0f, fadeState.transitionAlpha + step);
            if (fadeState.transitionAlpha >= 0.999f)
            {
                fadeState.transitionAlpha = 1.0f;
                fadeState.previousCount = fadeState.currentCount;
                fadeState.isTransitioning = false;
            }
        }

        return DistributedArrowFadeResolved{
            fadeState.previousCount,
            fadeState.currentCount,
            fadeState.isTransitioning ? (1.0f - fadeState.transitionAlpha) : 0.0f,
            fadeState.isTransitioning ? fadeState.transitionAlpha : 1.0f};
    };

    auto resolveDistributedTextFade = [&](std::uint64_t key, float availableLength, float requiredLength)
    {
        activeTextFadeKeys[key] = true;

        auto [it, inserted] = m_distributedLoadTextFadeByKey.emplace(key, 1.0f);
        float& storedAlpha = it->second;
        if (inserted)
        {
            storedAlpha = 1.0f;
        }

        if (requiredLength <= 1.0e-3f)
        {
            storedAlpha = 1.0f;
            return 1.0f;
        }

        const float fadeOutThreshold = requiredLength * distributedLoadTextFadeOutThresholdRatio;
        const float fadeInThreshold = requiredLength * distributedLoadTextFadeInThresholdRatio;
        const float step = deltaTime / kDimensionFadeDuration;
        if (availableLength < fadeOutThreshold)
        {
            storedAlpha = std::max(0.0f, storedAlpha - step);
        }
        else if (availableLength > fadeInThreshold)
        {
            storedAlpha = std::min(1.0f, storedAlpha + step);
        }

        return storedAlpha;
    };

    auto drawDistributedResultant = [&](int loadId,
                                        Vector2 beamStartScreen,
                                        Vector2 beamEndScreen,
                                        Vector2 targetBeamStartScreen,
                                        Vector2 targetBeamEndScreen,
                                        Vector2 screenVectorStart,
                                        Vector2 screenVectorEnd)
    {
        auto tryComputeZeroFactor = [&](Vector2 startVector, Vector2 endVector, double& zeroFactor) -> bool
        {
            const double startMagnitude = VectorLength(startVector);
            const double endMagnitude = VectorLength(endVector);
            if (startMagnitude <= 1.0e-9 || endMagnitude <= 1.0e-9)
            {
                return false;
            }

            const double cross = static_cast<double>(startVector.x) * static_cast<double>(endVector.y) -
                                 static_cast<double>(startVector.y) * static_cast<double>(endVector.x);
            const double dot = static_cast<double>(startVector.x) * static_cast<double>(endVector.x) +
                               static_cast<double>(startVector.y) * static_cast<double>(endVector.y);
            if (std::abs(cross) > startMagnitude * endMagnitude * 1.0e-6 || dot >= 0.0)
            {
                return false;
            }

            const double deltaX = static_cast<double>(endVector.x - startVector.x);
            const double deltaY = static_cast<double>(endVector.y - startVector.y);
            if (std::abs(deltaX) >= std::abs(deltaY) && std::abs(deltaX) > 1.0e-9)
            {
                zeroFactor = -static_cast<double>(startVector.x) / deltaX;
            }
            else if (std::abs(deltaY) > 1.0e-9)
            {
                zeroFactor = -static_cast<double>(startVector.y) / deltaY;
            }
            else
            {
                return false;
            }

            return zeroFactor > 1.0e-6 && zeroFactor < 1.0 - 1.0e-6;
        };

        std::function<void(double, double, Vector2, Vector2)> drawRange =
            [&](double tStart, double tEnd, Vector2 vectorStart, Vector2 vectorEnd)
        {
            const double subMagnitudeStart = VectorLength(vectorStart);
            const double subMagnitudeEnd = VectorLength(vectorEnd);
            if (subMagnitudeStart <= 1.0e-9 && subMagnitudeEnd <= 1.0e-9)
            {
                return;
            }

            double zeroFactor = 0.0;
            if (tryComputeZeroFactor(vectorStart, vectorEnd, zeroFactor))
            {
                const double tMid = tStart + (tEnd - tStart) * zeroFactor;
                drawRange(tStart, tMid, vectorStart, Vector2{0.0f, 0.0f});
                drawRange(tMid, tEnd, Vector2{0.0f, 0.0f}, vectorEnd);
                return;
            }

            const Vector2 subStartScreen = lerpVector(beamStartScreen, beamEndScreen, tStart);
            const Vector2 subEndScreen = lerpVector(beamStartScreen, beamEndScreen, tEnd);
            const float subLength = VectorLength(Vector2{
                subEndScreen.x - subStartScreen.x,
                subEndScreen.y - subStartScreen.y});
            if (subLength <= 1.0e-6f)
            {
                return;
            }

            const Vector2 segmentDirection =
                subMagnitudeStart > 1.0e-9 ? NormalizeVectorSafe(vectorStart) : NormalizeVectorSafe(vectorEnd);
            const Vector2 startDirection =
                subMagnitudeStart > 1.0e-9 ? NormalizeVectorSafe(vectorStart) : segmentDirection;
            const Vector2 endDirection =
                subMagnitudeEnd > 1.0e-9 ? NormalizeVectorSafe(vectorEnd) : segmentDirection;
            const float startLength = scaledArrowLength(subMagnitudeStart);
            const float endLength = scaledArrowLength(subMagnitudeEnd);
            const Vector2 tipStart = Vector2{
                subStartScreen.x - startDirection.x * beamHalfThicknessPixels,
                subStartScreen.y - startDirection.y * beamHalfThicknessPixels};
            const Vector2 tipEnd = Vector2{
                subEndScreen.x - endDirection.x * beamHalfThicknessPixels,
                subEndScreen.y - endDirection.y * beamHalfThicknessPixels};
            const Vector2 tailStart = Vector2{
                tipStart.x - startDirection.x * startLength,
                tipStart.y - startDirection.y * startLength};
            const Vector2 tailEnd = Vector2{
                tipEnd.x - endDirection.x * endLength,
                tipEnd.y - endDirection.y * endLength};

            if (state.view.showDistributedLoadArea)
            {
                DrawFilledTriangleSafe(tailStart, tailEnd, tipEnd, fillColor);
                DrawFilledTriangleSafe(tailStart, tipEnd, tipStart, fillColor);
            }

            DrawLineEx(tailStart, tailEnd, topLineThickness, loadColor);

            Vector2 labelLine = Vector2{tailEnd.x - tailStart.x, tailEnd.y - tailStart.y};
            if (VectorLength(labelLine) <= 1.0e-6f)
            {
                labelLine = Vector2{subEndScreen.x - subStartScreen.x, subEndScreen.y - subStartScreen.y};
            }
            const float labelRotationDegrees =
                NormalizeReadableAngle(atan2f(labelLine.y, labelLine.x) * RAD2DEG);
            const float labelRotationRadians = labelRotationDegrees * DEG2RAD;
            const Vector2 textDirection = Vector2{cosf(labelRotationRadians), sinf(labelRotationRadians)};
            const Vector2 labelNormal = Vector2{-textDirection.y, textDirection.x};
            const Vector2 centerDirection =
                NormalizeVectorSafe(Vector2{tailEnd.x - tailStart.x, tailEnd.y - tailStart.y});
            const Vector2 outwardStart = NormalizeVectorSafe(Vector2{
                tailStart.x - tipStart.x,
                tailStart.y - tipStart.y});
            const Vector2 outwardEnd = NormalizeVectorSafe(Vector2{
                tailEnd.x - tipEnd.x,
                tailEnd.y - tipEnd.y});
            auto orientToLoadSide = [&](Vector2 baseNormal, Vector2 loadDirection)
            {
                if ((baseNormal.x * loadDirection.x + baseNormal.y * loadDirection.y) < 0.0f)
                {
                    baseNormal.x = -baseNormal.x;
                    baseNormal.y = -baseNormal.y;
                }
                return baseNormal;
            };
            const Vector2 startOffsetDirection = orientToLoadSide(labelNormal, outwardStart);
            const Vector2 endOffsetDirection = orientToLoadSide(labelNormal, outwardEnd);
            const bool useInclinedEndpointLayout =
                std::abs(textDirection.x) > 1.0e-3f && std::abs(textDirection.y) > 1.0e-3f &&
                VectorLength(centerDirection) > 1.0e-6f;
            const Vector2 alongTextTowardCenter =
                (textDirection.x * centerDirection.x + textDirection.y * centerDirection.y) >= 0.0f
                    ? textDirection
                    : Vector2{-textDirection.x, -textDirection.y};

            const Vector2 targetSubStartScreen = lerpVector(targetBeamStartScreen, targetBeamEndScreen, tStart);
            const Vector2 targetSubEndScreen = lerpVector(targetBeamStartScreen, targetBeamEndScreen, tEnd);
            const float targetSubLength = VectorLength(Vector2{
                targetSubEndScreen.x - targetSubStartScreen.x,
                targetSubEndScreen.y - targetSubStartScreen.y});
            const int targetArrowCount =
                std::max(2, static_cast<int>(ceilf(targetSubLength / arrowsPerSegmentSpacingPixels)) + 1);
            const DistributedArrowFadeResolved arrowFade =
                resolveDistributedArrowFade(makeDistributedArrowFadeKey(loadId, 0, tStart, tEnd), targetArrowCount);

            auto drawResultantArrowSet = [&](int arrowCount, float alpha)
            {
                if (alpha <= 0.001f)
                {
                    return;
                }

                const Color arrowColor = WithScaledAlpha(loadColor, alpha);
                for (int arrowIndex = 0; arrowIndex < arrowCount; ++arrowIndex)
                {
                    const double arrowT =
                        arrowCount > 1
                            ? static_cast<double>(arrowIndex) / static_cast<double>(arrowCount - 1)
                            : 0.0;
                    const double beamParameter = tStart + (tEnd - tStart) * arrowT;
                    const Vector2 vectorAtArrow = lerpVector(vectorStart, vectorEnd, arrowT);
                    const float arrowMagnitude = VectorLength(vectorAtArrow);
                    if (arrowMagnitude <= 1.0e-6f)
                    {
                        continue;
                    }

                    const Vector2 arrowDirection = NormalizeVectorSafe(vectorAtArrow);
                    const float arrowLength = scaledArrowLength(arrowMagnitude);
                    const Vector2 beamPoint = lerpVector(beamStartScreen, beamEndScreen, beamParameter);
                    const Vector2 tip = Vector2{
                        beamPoint.x - arrowDirection.x * beamHalfThicknessPixels,
                        beamPoint.y - arrowDirection.y * beamHalfThicknessPixels};
                    const Vector2 tail = Vector2{
                        tip.x - arrowDirection.x * arrowLength,
                        tip.y - arrowDirection.y * arrowLength};
                    DrawLoadArrow(tail, tip, lineThickness, arrowColor, arrowHeadWidth);
                }
            };

            if (arrowFade.previousAlpha > 0.001f)
            {
                drawResultantArrowSet(arrowFade.previousCount, arrowFade.previousAlpha);
            }
            drawResultantArrowSet(arrowFade.currentCount, arrowFade.currentAlpha);

            if (suppressLoadText)
            {
                return;
            }

            const bool isConstant = std::abs(subMagnitudeStart - subMagnitudeEnd) <= 1.0e-9;
            if (isConstant)
            {
                const std::string label =
                    FormatDisplayDistributedLoad(subMagnitudeStart, document.displayUnits.distributedLoad);
                const CachedTextMeasure& labelMeasure = getTextMeasure(label);
                const Vector2 labelSize = labelMeasure.size;
                const Vector2 labelPositionSize = labelMeasure.positionSize;
                const float labelAlpha = resolveDistributedTextFade(
                    makeDistributedArrowFadeKey(loadId, 10, tStart, tEnd),
                    subLength,
                    labelPositionSize.x);
                if (labelAlpha <= 0.01f)
                {
                    return;
                }
                const Vector2 midTail = Vector2{
                    (tailStart.x + tailEnd.x) * 0.5f,
                    (tailStart.y + tailEnd.y) * 0.5f};
                const Vector2 midTip = Vector2{
                    (tipStart.x + tipEnd.x) * 0.5f,
                    (tipStart.y + tipEnd.y) * 0.5f};
                const Vector2 labelCenter = ComputeDistributedLoadLabelCenter(
                    midTail,
                    Vector2{midTail.x - midTip.x, midTail.y - midTip.y},
                    labelPositionSize.y,
                    labelGapPixels);
                DrawTextPro(
                    textFont,
                    label.c_str(),
                    labelCenter,
                    Vector2{labelSize.x * 0.5f, labelSize.y * 0.5f},
                    labelRotationDegrees,
                    textFontSize,
                    textSpacing,
                    WithScaledAlpha(textColor, labelAlpha));
                return;
            }

            const bool hasStartLabel = subMagnitudeStart > 1.0e-9;
            const bool hasEndLabel = subMagnitudeEnd > 1.0e-9;
            float combinedLabelWidth = 0.0f;
            if (hasStartLabel)
            {
                const std::string startLabel =
                    FormatDisplayDistributedLoad(subMagnitudeStart, document.displayUnits.distributedLoad);
                const CachedTextMeasure& startLabelMeasure = getTextMeasure(startLabel);
                const Vector2 startLabelPositionSize = startLabelMeasure.positionSize;
                combinedLabelWidth += startLabelPositionSize.x;
            }
            if (hasEndLabel)
            {
                const std::string endLabel =
                    FormatDisplayDistributedLoad(subMagnitudeEnd, document.displayUnits.distributedLoad);
                const CachedTextMeasure& endLabelMeasure = getTextMeasure(endLabel);
                const Vector2 endLabelPositionSize = endLabelMeasure.positionSize;
                combinedLabelWidth += endLabelPositionSize.x;
            }
            const float endpointLabelAlpha = resolveDistributedTextFade(
                makeDistributedArrowFadeKey(loadId, 11, tStart, tEnd),
                subLength,
                combinedLabelWidth);

            if (hasStartLabel && endpointLabelAlpha > 0.01f)
            {
                const std::string startLabel =
                    FormatDisplayDistributedLoad(subMagnitudeStart, document.displayUnits.distributedLoad);
                const CachedTextMeasure& startLabelMeasure = getTextMeasure(startLabel);
                const Vector2 startLabelSize = startLabelMeasure.size;
                const Vector2 startLabelPositionSize = startLabelMeasure.positionSize;
                Vector2 startLabelCenter = ComputeDistributedLoadLabelCenter(
                    tailStart,
                    Vector2{tailStart.x - tipStart.x, tailStart.y - tipStart.y},
                    startLabelPositionSize.y,
                    labelGapPixels);
                if (useInclinedEndpointLayout)
                {
                    const Vector2 startNormalStepEnd = Vector2{
                        tailStart.x + startOffsetDirection.x * labelGapPixels,
                        tailStart.y + startOffsetDirection.y * labelGapPixels};
                    const Vector2 startAnchor = Vector2{
                        startNormalStepEnd.x +
                            alongTextTowardCenter.x * inclinedEndpointAlongOffsetPixels,
                        startNormalStepEnd.y +
                            alongTextTowardCenter.y * inclinedEndpointAlongOffsetPixels};
                    startLabelCenter = ComputeRotatedTextCenterFromCorner(
                        startAnchor,
                        startLabelPositionSize,
                        labelRotationDegrees,
                        chooseDistributedStartCorner(textDirection, startOffsetDirection));
                }
                DrawTextPro(
                    textFont,
                    startLabel.c_str(),
                    startLabelCenter,
                    Vector2{startLabelSize.x * 0.5f, startLabelSize.y * 0.5f},
                    labelRotationDegrees,
                    textFontSize,
                    textSpacing,
                    WithScaledAlpha(textColor, endpointLabelAlpha));
            }

            if (hasEndLabel && endpointLabelAlpha > 0.01f)
            {
                const std::string endLabel =
                    FormatDisplayDistributedLoad(subMagnitudeEnd, document.displayUnits.distributedLoad);
                const CachedTextMeasure& endLabelMeasure = getTextMeasure(endLabel);
                const Vector2 endLabelSize = endLabelMeasure.size;
                const Vector2 endLabelPositionSize = endLabelMeasure.positionSize;
                Vector2 endLabelCenter = ComputeDistributedLoadLabelCenter(
                    tailEnd,
                    Vector2{tailEnd.x - tipEnd.x, tailEnd.y - tipEnd.y},
                    endLabelPositionSize.y,
                    labelGapPixels);
                if (useInclinedEndpointLayout)
                {
                    const Vector2 endNormalStepEnd = Vector2{
                        tailEnd.x + endOffsetDirection.x * labelGapPixels,
                        tailEnd.y + endOffsetDirection.y * labelGapPixels};
                    const Vector2 endAnchor = Vector2{
                        endNormalStepEnd.x -
                            alongTextTowardCenter.x * inclinedEndpointAlongOffsetPixels,
                        endNormalStepEnd.y -
                            alongTextTowardCenter.y * inclinedEndpointAlongOffsetPixels};
                    endLabelCenter = ComputeRotatedTextCenterFromCorner(
                        endAnchor,
                        endLabelPositionSize,
                        labelRotationDegrees,
                        chooseDistributedEndCorner(textDirection, endOffsetDirection));
                }
                DrawTextPro(
                    textFont,
                    endLabel.c_str(),
                    endLabelCenter,
                    Vector2{endLabelSize.x * 0.5f, endLabelSize.y * 0.5f},
                    labelRotationDegrees,
                    textFontSize,
                    textSpacing,
                    WithScaledAlpha(textColor, endpointLabelAlpha));
            }
        };

        drawRange(0.0, 1.0, screenVectorStart, screenVectorEnd);
    };

    auto drawDistributedComponent = [&](int loadId,
                                        int channelId,
                                        Vector2 beamStartScreen,
                                        Vector2 beamEndScreen,
                                        Vector2 targetBeamStartScreen,
                                        Vector2 targetBeamEndScreen,
                                        double componentStart,
                                        double componentEnd,
                                        Vector2 positiveDirection)
    {
        std::function<void(double, double, double, double)> drawRange =
            [&](double tStart, double tEnd, double valueStart, double valueEnd)
        {
            if (std::abs(valueStart) <= 1.0e-9 && std::abs(valueEnd) <= 1.0e-9)
            {
                return;
            }

            const bool startPositive = valueStart > 1.0e-9;
            const bool endPositive = valueEnd > 1.0e-9;
            const bool startNegative = valueStart < -1.0e-9;
            const bool endNegative = valueEnd < -1.0e-9;
            if ((startPositive && endNegative) || (startNegative && endPositive))
            {
                const double zeroFactor = std::abs(valueStart) / (std::abs(valueStart) + std::abs(valueEnd));
                const double tMid = tStart + (tEnd - tStart) * zeroFactor;
                drawRange(tStart, tMid, valueStart, 0.0);
                drawRange(tMid, tEnd, 0.0, valueEnd);
                return;
            }

            const Vector2 subStartScreen = lerpVector(beamStartScreen, beamEndScreen, tStart);
            const Vector2 subEndScreen = lerpVector(beamStartScreen, beamEndScreen, tEnd);
            const float subLength = VectorLength(Vector2{
                subEndScreen.x - subStartScreen.x,
                subEndScreen.y - subStartScreen.y});
            if (subLength <= 1.0e-6f)
            {
                return;
            }

            const int segmentSign =
                valueStart > 1.0e-9 ? 1 : valueStart < -1.0e-9 ? -1 : valueEnd > 1.0e-9 ? 1 : -1;
            const int startSign =
                valueStart > 1.0e-9 ? 1 : valueStart < -1.0e-9 ? -1 : segmentSign;
            const int endSign =
                valueEnd > 1.0e-9 ? 1 : valueEnd < -1.0e-9 ? -1 : segmentSign;
            const Vector2 startDirection =
                startSign > 0 ? positiveDirection : Vector2{-positiveDirection.x, -positiveDirection.y};
            const Vector2 endDirection =
                endSign > 0 ? positiveDirection : Vector2{-positiveDirection.x, -positiveDirection.y};
            const float startLength = scaledArrowLength(valueStart);
            const float endLength = scaledArrowLength(valueEnd);
            const Vector2 tipStart = Vector2{
                subStartScreen.x - startDirection.x * beamHalfThicknessPixels,
                subStartScreen.y - startDirection.y * beamHalfThicknessPixels};
            const Vector2 tipEnd = Vector2{
                subEndScreen.x - endDirection.x * beamHalfThicknessPixels,
                subEndScreen.y - endDirection.y * beamHalfThicknessPixels};
            const Vector2 tailStart = Vector2{
                tipStart.x - startDirection.x * startLength,
                tipStart.y - startDirection.y * startLength};
            const Vector2 tailEnd = Vector2{
                tipEnd.x - endDirection.x * endLength,
                tipEnd.y - endDirection.y * endLength};

            if (state.view.showDistributedLoadArea)
            {
                DrawFilledTriangleSafe(tailStart, tailEnd, tipEnd, fillColor);
                DrawFilledTriangleSafe(tailStart, tipEnd, tipStart, fillColor);
            }

            DrawLineEx(tailStart, tailEnd, topLineThickness, loadColor);

            Vector2 labelLine = Vector2{tailEnd.x - tailStart.x, tailEnd.y - tailStart.y};
            if (VectorLength(labelLine) <= 1.0e-6f)
            {
                labelLine = Vector2{subEndScreen.x - subStartScreen.x, subEndScreen.y - subStartScreen.y};
            }
            const float labelRotationDegrees =
                NormalizeReadableAngle(atan2f(labelLine.y, labelLine.x) * RAD2DEG);
            const float labelRotationRadians = labelRotationDegrees * DEG2RAD;
            const Vector2 textDirection = Vector2{cosf(labelRotationRadians), sinf(labelRotationRadians)};
            const Vector2 labelNormal = Vector2{-textDirection.y, textDirection.x};
            const Vector2 centerDirection =
                NormalizeVectorSafe(Vector2{tailEnd.x - tailStart.x, tailEnd.y - tailStart.y});
            const Vector2 outwardStart = NormalizeVectorSafe(Vector2{
                tailStart.x - tipStart.x,
                tailStart.y - tipStart.y});
            const Vector2 outwardEnd = NormalizeVectorSafe(Vector2{
                tailEnd.x - tipEnd.x,
                tailEnd.y - tipEnd.y});
            auto orientToLoadSide = [&](Vector2 baseNormal, Vector2 loadDirection)
            {
                if ((baseNormal.x * loadDirection.x + baseNormal.y * loadDirection.y) < 0.0f)
                {
                    baseNormal.x = -baseNormal.x;
                    baseNormal.y = -baseNormal.y;
                }
                return baseNormal;
            };
            const Vector2 startOffsetDirection = orientToLoadSide(labelNormal, outwardStart);
            const Vector2 endOffsetDirection = orientToLoadSide(labelNormal, outwardEnd);
            const bool useInclinedEndpointLayout =
                std::abs(textDirection.x) > 1.0e-3f && std::abs(textDirection.y) > 1.0e-3f &&
                VectorLength(centerDirection) > 1.0e-6f;
            const Vector2 alongTextTowardCenter =
                (textDirection.x * centerDirection.x + textDirection.y * centerDirection.y) >= 0.0f
                    ? textDirection
                    : Vector2{-textDirection.x, -textDirection.y};

            const Vector2 targetSubStartScreen = lerpVector(targetBeamStartScreen, targetBeamEndScreen, tStart);
            const Vector2 targetSubEndScreen = lerpVector(targetBeamStartScreen, targetBeamEndScreen, tEnd);
            const float targetSubLength = VectorLength(Vector2{
                targetSubEndScreen.x - targetSubStartScreen.x,
                targetSubEndScreen.y - targetSubStartScreen.y});
            const int targetArrowCount =
                std::max(2, static_cast<int>(ceilf(targetSubLength / arrowsPerSegmentSpacingPixels)) + 1);
            const DistributedArrowFadeResolved arrowFade = resolveDistributedArrowFade(
                makeDistributedArrowFadeKey(loadId, channelId, tStart, tEnd),
                targetArrowCount);

            auto drawComponentArrowSet = [&](int arrowCount, float alpha)
            {
                if (alpha <= 0.001f)
                {
                    return;
                }

                const Color arrowColor = WithScaledAlpha(loadColor, alpha);
                for (int arrowIndex = 0; arrowIndex < arrowCount; ++arrowIndex)
                {
                    const double arrowT =
                        arrowCount > 1
                            ? static_cast<double>(arrowIndex) / static_cast<double>(arrowCount - 1)
                            : 0.0;
                    const double beamParameter = tStart + (tEnd - tStart) * arrowT;
                    const double arrowValue = valueStart + (valueEnd - valueStart) * arrowT;
                    if (std::abs(arrowValue) <= 1.0e-9)
                    {
                        continue;
                    }

                    const Vector2 arrowDirection =
                        arrowValue >= 0.0 ? positiveDirection : Vector2{-positiveDirection.x, -positiveDirection.y};
                    const float arrowLength = scaledArrowLength(arrowValue);
                    const Vector2 beamPoint = lerpVector(beamStartScreen, beamEndScreen, beamParameter);
                    const Vector2 tip = Vector2{
                        beamPoint.x - arrowDirection.x * beamHalfThicknessPixels,
                        beamPoint.y - arrowDirection.y * beamHalfThicknessPixels};
                    const Vector2 tail = Vector2{
                        tip.x - arrowDirection.x * arrowLength,
                        tip.y - arrowDirection.y * arrowLength};
                    DrawLoadArrow(tail, tip, lineThickness, arrowColor, arrowHeadWidth);
                }
            };

            if (arrowFade.previousAlpha > 0.001f)
            {
                drawComponentArrowSet(arrowFade.previousCount, arrowFade.previousAlpha);
            }
            drawComponentArrowSet(arrowFade.currentCount, arrowFade.currentAlpha);

            if (suppressLoadText)
            {
                return;
            }

            const bool isConstant = std::abs(valueStart - valueEnd) <= 1.0e-9;
            if (isConstant)
            {
                const std::string label = FormatDisplayDistributedLoad(valueStart, document.displayUnits.distributedLoad);
                const CachedTextMeasure& labelMeasure = getTextMeasure(label);
                const Vector2 labelSize = labelMeasure.size;
                const Vector2 labelPositionSize = labelMeasure.positionSize;
                const float labelAlpha = resolveDistributedTextFade(
                    makeDistributedArrowFadeKey(loadId, 20 + channelId, tStart, tEnd),
                    subLength,
                    labelPositionSize.x);
                if (labelAlpha <= 0.01f)
                {
                    return;
                }
                const Vector2 midTail = Vector2{
                    (tailStart.x + tailEnd.x) * 0.5f,
                    (tailStart.y + tailEnd.y) * 0.5f};
                const Vector2 midTip = Vector2{
                    (tipStart.x + tipEnd.x) * 0.5f,
                    (tipStart.y + tipEnd.y) * 0.5f};
                const Vector2 labelCenter = ComputeDistributedLoadLabelCenter(
                    midTail,
                    Vector2{midTail.x - midTip.x, midTail.y - midTip.y},
                    labelPositionSize.y,
                    labelGapPixels);
                DrawTextPro(
                    textFont,
                    label.c_str(),
                    labelCenter,
                    Vector2{labelSize.x * 0.5f, labelSize.y * 0.5f},
                    labelRotationDegrees,
                    textFontSize,
                    textSpacing,
                    WithScaledAlpha(textColor, labelAlpha));
                return;
            }

            const bool hasStartLabel = std::abs(valueStart) > 1.0e-9;
            const bool hasEndLabel = std::abs(valueEnd) > 1.0e-9;
            float combinedLabelWidth = 0.0f;
            if (hasStartLabel)
            {
                const std::string startLabel =
                    FormatDisplayDistributedLoad(valueStart, document.displayUnits.distributedLoad);
                const CachedTextMeasure& startLabelMeasure = getTextMeasure(startLabel);
                const Vector2 startLabelPositionSize = startLabelMeasure.positionSize;
                combinedLabelWidth += startLabelPositionSize.x;
            }
            if (hasEndLabel)
            {
                const std::string endLabel =
                    FormatDisplayDistributedLoad(valueEnd, document.displayUnits.distributedLoad);
                const CachedTextMeasure& endLabelMeasure = getTextMeasure(endLabel);
                const Vector2 endLabelPositionSize = endLabelMeasure.positionSize;
                combinedLabelWidth += endLabelPositionSize.x;
            }
            const float endpointLabelAlpha = resolveDistributedTextFade(
                makeDistributedArrowFadeKey(loadId, 30 + channelId, tStart, tEnd),
                subLength,
                combinedLabelWidth);

            if (hasStartLabel && endpointLabelAlpha > 0.01f)
            {
                const std::string startLabel =
                    FormatDisplayDistributedLoad(valueStart, document.displayUnits.distributedLoad);
                const CachedTextMeasure& startLabelMeasure = getTextMeasure(startLabel);
                const Vector2 startLabelSize = startLabelMeasure.size;
                const Vector2 startLabelPositionSize = startLabelMeasure.positionSize;
                Vector2 startLabelCenter = ComputeDistributedLoadLabelCenter(
                    tailStart,
                    Vector2{tailStart.x - tipStart.x, tailStart.y - tipStart.y},
                    startLabelPositionSize.y,
                    labelGapPixels);
                if (useInclinedEndpointLayout)
                {
                    const Vector2 startNormalStepEnd = Vector2{
                        tailStart.x + startOffsetDirection.x * labelGapPixels,
                        tailStart.y + startOffsetDirection.y * labelGapPixels};
                    const Vector2 startAnchor = Vector2{
                        startNormalStepEnd.x +
                            alongTextTowardCenter.x * inclinedEndpointAlongOffsetPixels,
                        startNormalStepEnd.y +
                            alongTextTowardCenter.y * inclinedEndpointAlongOffsetPixels};
                    startLabelCenter = ComputeRotatedTextCenterFromCorner(
                        startAnchor,
                        startLabelPositionSize,
                        labelRotationDegrees,
                        chooseDistributedStartCorner(textDirection, startOffsetDirection));
                }
                DrawTextPro(
                    textFont,
                    startLabel.c_str(),
                    startLabelCenter,
                    Vector2{startLabelSize.x * 0.5f, startLabelSize.y * 0.5f},
                    labelRotationDegrees,
                    textFontSize,
                    textSpacing,
                    WithScaledAlpha(textColor, endpointLabelAlpha));
            }

            if (hasEndLabel && endpointLabelAlpha > 0.01f)
            {
                const std::string endLabel =
                    FormatDisplayDistributedLoad(valueEnd, document.displayUnits.distributedLoad);
                const CachedTextMeasure& endLabelMeasure = getTextMeasure(endLabel);
                const Vector2 endLabelSize = endLabelMeasure.size;
                const Vector2 endLabelPositionSize = endLabelMeasure.positionSize;
                Vector2 endLabelCenter = ComputeDistributedLoadLabelCenter(
                    tailEnd,
                    Vector2{tailEnd.x - tipEnd.x, tailEnd.y - tipEnd.y},
                    endLabelPositionSize.y,
                    labelGapPixels);
                if (useInclinedEndpointLayout)
                {
                    const Vector2 endNormalStepEnd = Vector2{
                        tailEnd.x + endOffsetDirection.x * labelGapPixels,
                        tailEnd.y + endOffsetDirection.y * labelGapPixels};
                    const Vector2 endAnchor = Vector2{
                        endNormalStepEnd.x -
                            alongTextTowardCenter.x * inclinedEndpointAlongOffsetPixels,
                        endNormalStepEnd.y -
                            alongTextTowardCenter.y * inclinedEndpointAlongOffsetPixels};
                    endLabelCenter = ComputeRotatedTextCenterFromCorner(
                        endAnchor,
                        endLabelPositionSize,
                        labelRotationDegrees,
                        chooseDistributedEndCorner(textDirection, endOffsetDirection));
                }
                DrawTextPro(
                    textFont,
                    endLabel.c_str(),
                    endLabelCenter,
                    Vector2{endLabelSize.x * 0.5f, endLabelSize.y * 0.5f},
                    labelRotationDegrees,
                    textFontSize,
                    textSpacing,
                    WithScaledAlpha(textColor, endpointLabelAlpha));
            }
        };

        drawRange(0.0, 1.0, componentStart, componentEnd);
    };

    for (const BeamDistributedLoad& distributedLoad : document.distributedLoads)
    {
        const Beam* beam = document.FindBeamById(distributedLoad.beamId);
        if (beam == nullptr)
        {
            continue;
        }

        const Node* startNode = document.FindNodeById(beam->startNodeId);
        const Node* endNode = document.FindNodeById(beam->endNodeId);
        if (startNode == nullptr || endNode == nullptr)
        {
            continue;
        }

        Camera2D targetCamera = camera;
        targetCamera.zoom = zoomTarget;
        const Vector2 beamStartScreen = GetWorldToScreen2D(
            Vector2{
                static_cast<float>(startNode->position.x),
                static_cast<float>(startNode->position.y)},
            camera);
        const Vector2 beamEndScreen = GetWorldToScreen2D(
            Vector2{
                static_cast<float>(endNode->position.x),
                static_cast<float>(endNode->position.y)},
            camera);
        const Vector2 targetBeamStartScreen = GetWorldToScreen2D(
            Vector2{
                static_cast<float>(startNode->position.x),
                static_cast<float>(startNode->position.y)},
            targetCamera);
        const Vector2 targetBeamEndScreen = GetWorldToScreen2D(
            Vector2{
                static_cast<float>(endNode->position.x),
                static_cast<float>(endNode->position.y)},
            targetCamera);

        const float minX = std::min(beamStartScreen.x, beamEndScreen.x);
        const float maxX = std::max(beamStartScreen.x, beamEndScreen.x);
        const float minY = std::min(beamStartScreen.y, beamEndScreen.y);
        const float maxY = std::max(beamStartScreen.y, beamEndScreen.y);
        if (maxX < -cullMargin ||
            minX > screenWidth + cullMargin ||
            maxY < -cullMargin ||
            minY > screenHeight + cullMargin)
        {
            continue;
        }

        if (state.view.showDistributedLoadResultant)
        {
            const Vector2 screenVectorStart = Vector2{
                static_cast<float>(distributedLoad.value.qxStart),
                static_cast<float>(-distributedLoad.value.qyStart)};
            const Vector2 screenVectorEnd = Vector2{
                static_cast<float>(distributedLoad.value.qxEnd),
                static_cast<float>(-distributedLoad.value.qyEnd)};
            drawDistributedResultant(
                distributedLoad.id,
                beamStartScreen,
                beamEndScreen,
                targetBeamStartScreen,
                targetBeamEndScreen,
                screenVectorStart,
                screenVectorEnd);
        }
        else
        {
            if (std::abs(distributedLoad.value.qxStart) > 1.0e-9 ||
                std::abs(distributedLoad.value.qxEnd) > 1.0e-9)
            {
                drawDistributedComponent(
                    distributedLoad.id,
                    1,
                    beamStartScreen,
                    beamEndScreen,
                    targetBeamStartScreen,
                    targetBeamEndScreen,
                    distributedLoad.value.qxStart,
                    distributedLoad.value.qxEnd,
                    Vector2{1.0f, 0.0f});
            }

            if (std::abs(distributedLoad.value.qyStart) > 1.0e-9 ||
                std::abs(distributedLoad.value.qyEnd) > 1.0e-9)
            {
                drawDistributedComponent(
                    distributedLoad.id,
                    2,
                    beamStartScreen,
                    beamEndScreen,
                    targetBeamStartScreen,
                    targetBeamEndScreen,
                    distributedLoad.value.qyStart,
                    distributedLoad.value.qyEnd,
                    Vector2{0.0f, -1.0f});
            }
        }
    }

    for (auto it = m_distributedLoadArrowFadeByKey.begin(); it != m_distributedLoadArrowFadeByKey.end();)
    {
        if (activeArrowFadeKeys.find(it->first) == activeArrowFadeKeys.end())
        {
            it = m_distributedLoadArrowFadeByKey.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto it = m_distributedLoadTextFadeByKey.begin(); it != m_distributedLoadTextFadeByKey.end();)
    {
        if (activeTextFadeKeys.find(it->first) == activeTextFadeKeys.end())
        {
            it = m_distributedLoadTextFadeByKey.erase(it);
        }
        else
        {
            ++it;
        }
    }
}


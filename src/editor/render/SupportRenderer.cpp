#include "editor/render/EditorRendererInternal.h"

void EditorRenderer::DrawSupports(
    const ProjectDocument& document,
    const ProjectDerivedData& derivedData,
    const EditorState& state,
    const Camera2D& camera) const
{
    constexpr float lineThickness = 1.8f;
    constexpr float beamHalfThicknessPixels = 3.25f;
    constexpr float triangleHalfWidth = 10.0f;
    constexpr float triangleHeight = triangleHalfWidth * 1.7320508f;
    constexpr float rollerRadius = 3.0f;
    constexpr float hatchSpacing = 5.0f;
    constexpr float hatchLength = 6.0f;
    constexpr float groundHalfWidth = 17.0f;
    constexpr float cullMargin = 36.0f;
    constexpr float fixedGroundThickness = 3.0f;
    constexpr float hatchThickness = 1.35f;

    const SupportStyle defaultStyle{
        Color{76, 76, 76, 255},
        Color{202, 202, 202, 255},
        Color{112, 112, 112, 255}};
    const SupportStyle hoveredStyle{
        Color{120, 150, 195, 255},
        Color{190, 208, 232, 255},
        Color{120, 150, 195, 255}};
    const SupportStyle selectedStyle{
        Color{70, 145, 255, 255},
        Color{178, 205, 240, 255},
        Color{70, 145, 255, 255}};
    const float screenWidth = static_cast<float>(GetScreenWidth());
    const float screenHeight = static_cast<float>(GetScreenHeight());
    const std::vector<int> selectedNodeIds = MakeSortedUniqueIdList(state.selectionState.selection.nodeIds);

    auto drawRollerCircle = [&](Vector2 center, const SupportStyle& style)
    {
        const Color rollerOuterFill = Color{176, 176, 176, 255};
        const Color rollerInnerFill = style.fillColor;
        const Color rollerHighlight = Color{248, 248, 248, 190};
        DrawCircleSector(center, rollerRadius + 0.55f, 0.0f, 360.0f, kShadowCircleSegments, style.outlineColor);
        DrawCircleSector(center, std::max(1.0f, rollerRadius - 0.15f), 0.0f, 360.0f, kShadowCircleSegments, rollerOuterFill);
        DrawCircleSector(
            center,
            std::max(1.0f, rollerRadius - 0.85f),
            0.0f,
            360.0f,
            kShadowCircleSegments,
            rollerInnerFill);
        DrawCircleSector(
            Vector2{center.x - 0.45f, center.y - 0.45f},
            std::max(0.8f, rollerRadius - 1.65f),
            0.0f,
            360.0f,
            kShadowCircleSegments,
            rollerHighlight);
    };

    for (const Node& node : document.nodes)
    {
        if (node.support == SupportType::None)
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

        const bool isSelected = ContainsSortedId(selectedNodeIds, node.id);
        const bool isHovered =
            state.hover.entity.type == EntityType::Node &&
            state.hover.entity.id == node.id;
        const SupportStyle style = isSelected ? selectedStyle : (isHovered ? hoveredStyle : defaultStyle);

        switch (node.support)
        {
        case SupportType::RestrainedY:
        {
            const Vector2 supportDirection = ChooseVerticalRollerSupportDirection(connectivity);
            const Vector2 supportTangent = Vector2{-supportDirection.y, supportDirection.x};
            const Vector2 apex = Vector2{
                screenCenter.x + supportDirection.x * beamHalfThicknessPixels,
                screenCenter.y + supportDirection.y * beamHalfThicknessPixels};
            const Vector2 baseCenter = Vector2{
                apex.x + supportDirection.x * triangleHeight,
                apex.y + supportDirection.y * triangleHeight};
            const Vector2 baseLeft = Vector2{
                baseCenter.x - supportTangent.x * triangleHalfWidth,
                baseCenter.y - supportTangent.y * triangleHalfWidth};
            const Vector2 baseRight = Vector2{
                baseCenter.x + supportTangent.x * triangleHalfWidth,
                baseCenter.y + supportTangent.y * triangleHalfWidth};
            const Vector2 rollerCenterBase = Vector2{
                baseCenter.x + supportDirection.x * rollerRadius,
                baseCenter.y + supportDirection.y * rollerRadius};
            const Vector2 leftRoller = Vector2{
                rollerCenterBase.x - supportTangent.x * triangleHalfWidth * 0.45f,
                rollerCenterBase.y - supportTangent.y * triangleHalfWidth * 0.45f};
            const Vector2 rightRoller = Vector2{
                rollerCenterBase.x + supportTangent.x * triangleHalfWidth * 0.45f,
                rollerCenterBase.y + supportTangent.y * triangleHalfWidth * 0.45f};
            const Vector2 groundCenter = Vector2{
                baseCenter.x + supportDirection.x * rollerRadius * 2.0f,
                baseCenter.y + supportDirection.y * rollerRadius * 2.0f};
            const Vector2 groundStart = Vector2{
                groundCenter.x - supportTangent.x * groundHalfWidth,
                groundCenter.y - supportTangent.y * groundHalfWidth};
            const Vector2 groundEnd = Vector2{
                groundCenter.x + supportTangent.x * groundHalfWidth,
                groundCenter.y + supportTangent.y * groundHalfWidth};

            drawRollerCircle(leftRoller, style);
            drawRollerCircle(rightRoller, style);
            DrawSupportTriangle(
                apex,
                baseLeft,
                baseRight,
                lineThickness,
                style.fillColor,
                style.groundColor);
            DrawHatchedGroundAligned(
                groundEnd,
                groundStart,
                supportDirection,
                hatchLength,
                hatchSpacing,
                lineThickness,
                hatchThickness,
                style.groundColor);
            break;
        }

        case SupportType::RestrainedXY:
        {
            const Vector2 supportDirection = ChooseSecondKindSupportDirection(connectivity);
            const Vector2 supportTangent = Vector2{-supportDirection.y, supportDirection.x};
            const Vector2 apex = Vector2{
                screenCenter.x + supportDirection.x * beamHalfThicknessPixels,
                screenCenter.y + supportDirection.y * beamHalfThicknessPixels};
            const Vector2 baseCenter = Vector2{
                apex.x + supportDirection.x * triangleHeight,
                apex.y + supportDirection.y * triangleHeight};
            const Vector2 baseLeft = Vector2{
                baseCenter.x - supportTangent.x * triangleHalfWidth,
                baseCenter.y - supportTangent.y * triangleHalfWidth};
            const Vector2 baseRight = Vector2{
                baseCenter.x + supportTangent.x * triangleHalfWidth,
                baseCenter.y + supportTangent.y * triangleHalfWidth};
            const Vector2 groundStart = Vector2{
                baseCenter.x - supportTangent.x * groundHalfWidth,
                baseCenter.y - supportTangent.y * groundHalfWidth};
            const Vector2 groundEnd = Vector2{
                baseCenter.x + supportTangent.x * groundHalfWidth,
                baseCenter.y + supportTangent.y * groundHalfWidth};

            DrawSupportTriangle(
                apex,
                baseLeft,
                baseRight,
                lineThickness,
                style.fillColor,
                style.groundColor);
            DrawHatchedGroundAligned(
                groundEnd,
                groundStart,
                supportDirection,
                hatchLength,
                hatchSpacing,
                lineThickness,
                hatchThickness,
                style.groundColor);
            break;
        }

        case SupportType::RestrainedX:
        {
            const Vector2 supportDirection = ChooseHorizontalRollerSupportDirection(connectivity);
            const Vector2 supportTangent = Vector2{-supportDirection.y, supportDirection.x};
            const Vector2 apex = Vector2{
                screenCenter.x + supportDirection.x * beamHalfThicknessPixels,
                screenCenter.y + supportDirection.y * beamHalfThicknessPixels};
            const Vector2 baseCenter = Vector2{
                apex.x + supportDirection.x * triangleHeight,
                apex.y + supportDirection.y * triangleHeight};
            const Vector2 baseLeft = Vector2{
                baseCenter.x - supportTangent.x * triangleHalfWidth,
                baseCenter.y - supportTangent.y * triangleHalfWidth};
            const Vector2 baseRight = Vector2{
                baseCenter.x + supportTangent.x * triangleHalfWidth,
                baseCenter.y + supportTangent.y * triangleHalfWidth};
            const Vector2 rollerCenterBase = Vector2{
                baseCenter.x + supportDirection.x * rollerRadius,
                baseCenter.y + supportDirection.y * rollerRadius};
            const Vector2 leftRoller = Vector2{
                rollerCenterBase.x - supportTangent.x * triangleHalfWidth * 0.45f,
                rollerCenterBase.y - supportTangent.y * triangleHalfWidth * 0.45f};
            const Vector2 rightRoller = Vector2{
                rollerCenterBase.x + supportTangent.x * triangleHalfWidth * 0.45f,
                rollerCenterBase.y + supportTangent.y * triangleHalfWidth * 0.45f};
            const Vector2 groundCenter = Vector2{
                baseCenter.x + supportDirection.x * rollerRadius * 2.0f,
                baseCenter.y + supportDirection.y * rollerRadius * 2.0f};
            const Vector2 groundStart = Vector2{
                groundCenter.x - supportTangent.x * groundHalfWidth,
                groundCenter.y - supportTangent.y * groundHalfWidth};
            const Vector2 groundEnd = Vector2{
                groundCenter.x + supportTangent.x * groundHalfWidth,
                groundCenter.y + supportTangent.y * groundHalfWidth};

            drawRollerCircle(leftRoller, style);
            drawRollerCircle(rightRoller, style);
            DrawSupportTriangle(
                apex,
                baseLeft,
                baseRight,
                lineThickness,
                style.fillColor,
                style.groundColor);
            DrawHatchedGroundAligned(
                groundEnd,
                groundStart,
                supportDirection,
                hatchLength,
                hatchSpacing,
                lineThickness,
                hatchThickness,
                style.groundColor);
            break;
        }

        case SupportType::Fixed:
        {
            if (!connectivity.hasPreferredConnectedBeamDirection)
            {
                const float supportBlockHalfExtent = groundHalfWidth * 0.75f;
                const Rectangle supportBlock = Rectangle{
                    screenCenter.x - supportBlockHalfExtent,
                    screenCenter.y - supportBlockHalfExtent,
                    supportBlockHalfExtent * 2.0f,
                    supportBlockHalfExtent * 2.0f};

                DrawRectangleRec(supportBlock, style.fillColor);

                DrawHatchedGroundAligned(
                    Vector2{supportBlock.x, supportBlock.y},
                    Vector2{supportBlock.x + supportBlock.width, supportBlock.y},
                    Vector2{0.0f, -1.0f},
                    hatchLength,
                    hatchSpacing,
                    fixedGroundThickness,
                    hatchThickness,
                    style.groundColor);
                DrawHatchedGroundAligned(
                    Vector2{supportBlock.x + supportBlock.width, supportBlock.y},
                    Vector2{supportBlock.x + supportBlock.width, supportBlock.y + supportBlock.height},
                    Vector2{1.0f, 0.0f},
                    hatchLength,
                    hatchSpacing,
                    fixedGroundThickness,
                    hatchThickness,
                    style.groundColor);
                DrawHatchedGroundAligned(
                    Vector2{supportBlock.x + supportBlock.width, supportBlock.y + supportBlock.height},
                    Vector2{supportBlock.x, supportBlock.y + supportBlock.height},
                    Vector2{0.0f, 1.0f},
                    hatchLength,
                    hatchSpacing,
                    fixedGroundThickness,
                    hatchThickness,
                    style.groundColor);
                DrawHatchedGroundAligned(
                    Vector2{supportBlock.x, supportBlock.y + supportBlock.height},
                    Vector2{supportBlock.x, supportBlock.y},
                    Vector2{-1.0f, 0.0f},
                    hatchLength,
                    hatchSpacing,
                    fixedGroundThickness,
                    hatchThickness,
                    style.groundColor);
                break;
            }

            Vector2 averageDirection = Vector2{1.0f, 0.0f};
            if (connectivity.hasAverageConnectedBeamDirection)
            {
                averageDirection = NormalizeVectorSafe(
                    Vector2{
                        static_cast<float>(connectivity.averageConnectedBeamDirection.x),
                        static_cast<float>(connectivity.averageConnectedBeamDirection.y)});
            }

            if (averageDirection.x == 0.0f && averageDirection.y == 0.0f)
            {
                averageDirection = Vector2{1.0f, 0.0f};
            }

            const Vector2 supportNormal = Vector2{-averageDirection.x, -averageDirection.y};
            const Vector2 supportTangent = Vector2{-averageDirection.y, averageDirection.x};
            const Vector2 supportCenter = Vector2{
                screenCenter.x - averageDirection.x * (beamHalfThicknessPixels - fixedGroundThickness * 0.5f),
                screenCenter.y - averageDirection.y * (beamHalfThicknessPixels - fixedGroundThickness * 0.5f)};
            const Vector2 baseStart = Vector2{
                supportCenter.x - supportTangent.x * groundHalfWidth,
                supportCenter.y - supportTangent.y * groundHalfWidth};
            const Vector2 baseEnd = Vector2{
                supportCenter.x + supportTangent.x * groundHalfWidth,
                supportCenter.y + supportTangent.y * groundHalfWidth};

            DrawHatchedGroundAligned(
                baseStart,
                baseEnd,
                supportNormal,
                hatchLength,
                hatchSpacing,
                fixedGroundThickness,
                hatchThickness,
                style.groundColor);
            break;
        }

        case SupportType::None:
        default:
            break;
        }
    }
}

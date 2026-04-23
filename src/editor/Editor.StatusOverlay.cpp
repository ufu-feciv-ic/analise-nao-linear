#include "editor/Editor.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace
{
    constexpr float viewportLeftPanelWidth = 73.0f;
}

const char* Editor::GetTransformToolTitle(EditorTool tool) const
{
    switch (tool)
    {
    case EditorTool::MoveNode:
        return "Mover";

    case EditorTool::CopySelection:
        return "Copiar";

    case EditorTool::MirrorSelection:
        return "Espelhar";

    default:
        return "Transformar";
    }
}

void Editor::BuildTransformToolStatusText(
    EditorTool tool,
    std::string& title,
    std::string& detail) const
{
    title = GetTransformToolTitle(tool);

    if (!IsTransformTool(tool))
    {
        return;
    }

    const std::string mirrorModeDetail =
        state.transformTool.mirrorKeepsOriginal
            ? "Modo: manter original (O alterna)."
            : "Modo: remover original (O alterna).";

    if (state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitSelectionConfirm)
    {
        if (tool == EditorTool::MirrorSelection)
        {
            detail =
                (HasTransformSelection()
                    ? "Ajuste a selecao e pressione Espaco ou Enter para continuar. "
                    : "Selecione nos ou barras e pressione Espaco ou Enter. ") +
                mirrorModeDetail;
        }
        else
        {
            detail = HasTransformSelection()
                ? "Ajuste a selecao e pressione Espaco ou Enter para continuar."
                : "Selecione nos ou barras e pressione Espaco ou Enter.";
        }
        return;
    }

    if (state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitBasePoint)
    {
        switch (tool)
        {
        case EditorTool::MoveNode:
            detail = "Clique no ponto base do movimento.";
            break;

        case EditorTool::CopySelection:
            detail = "Clique no ponto base da copia.";
            break;

        case EditorTool::MirrorSelection:
            detail = "Clique no primeiro ponto da linha de espelho. " + mirrorModeDetail;
            break;

        default:
            detail.clear();
            break;
        }
        return;
    }

    if (state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitTargetPoint)
    {
        switch (tool)
        {
        case EditorTool::MoveNode:
            detail = "Clique no ponto de destino para mover a selecao.";
            break;

        case EditorTool::CopySelection:
            detail = "Clique no ponto de destino para criar uma copia. Continue clicando para repetir.";
            break;

        case EditorTool::MirrorSelection:
            detail = "Clique no segundo ponto da linha de espelho para espelhar a selecao. " + mirrorModeDetail;
            break;

        default:
            detail.clear();
            break;
        }
        return;
    }

    switch (tool)
    {
    case EditorTool::MoveNode:
        detail = "Selecione nos ou barras para mover.";
        break;

    case EditorTool::CopySelection:
        detail = "Selecione nos ou barras para copiar.";
        break;

    case EditorTool::MirrorSelection:
        detail = "Selecione nos ou barras para espelhar. " + mirrorModeDetail;
        break;

    default:
        detail.clear();
        break;
    }
}

void Editor::BuildViewportStatusText(
    std::string& title,
    std::string& detail) const
{
    auto appendActiveToolShortcut = [&]()
    {
        const char* shortcut = nullptr;

        switch (state.activeTool)
        {
        case EditorTool::Select:
            shortcut = "ESC";
            break;

        case EditorTool::AddNode:
            shortcut = "N";
            break;

        case EditorTool::AddBeam:
            shortcut = "B";
            break;

        case EditorTool::AddDimension:
            shortcut = "D";
            break;

        case EditorTool::MoveNode:
            shortcut = "M";
            break;

        case EditorTool::CopySelection:
            shortcut = "C";
            break;

        case EditorTool::MirrorSelection:
            shortcut = "E";
            break;

        case EditorTool::RemoveNode:
        case EditorTool::RemoveBeam:
        case EditorTool::RemovePointLoad:
        case EditorTool::RemoveDistributedLoad:
            shortcut = "DEL";
            break;

        default:
            break;
        }

        if (shortcut != nullptr && *shortcut != '\0')
        {
            title += " (";
            title += shortcut;
            title += ")";
        }
    };

    if (state.beamTool.isBeamDistanceInputOpen)
    {
        if (state.beamTool.beamDistanceMode == BeamToolState::DistanceInputMode::BeamAlongSegment)
        {
            title = "Novo no na barra";
            detail = "Digite a distancia a partir do inicio da barra e confirme.";
            appendActiveToolShortcut();
            return;
        }

        title = "Novo no por guia";
        detail = "Digite a distancia na guia ativa e confirme.";
        appendActiveToolShortcut();
        return;
    }

    if (state.beamTool.isCreatingBeam)
    {
        title = "Adicionar barra";
        detail = "Clique no no final da barra ou pressione Espaco para soltar e escolher um novo no de origem.";
        appendActiveToolShortcut();
        return;
    }

    switch (state.activeTool)
    {
    case EditorTool::AddNode:
        title = "Adicionar no";
        detail = "Clique para criar um no. Em barras e cruzamentos ele divide automaticamente.";
        break;

    case EditorTool::MoveNode:
        BuildTransformToolStatusText(EditorTool::MoveNode, title, detail);
        break;

    case EditorTool::CopySelection:
        BuildTransformToolStatusText(EditorTool::CopySelection, title, detail);
        break;

    case EditorTool::MirrorSelection:
        BuildTransformToolStatusText(EditorTool::MirrorSelection, title, detail);
        break;

    case EditorTool::RemoveNode:
        title = "Remover nos";
        detail = "Clique em um no para remover ou arraste uma janela.";
        break;

    case EditorTool::AddBeam:
        title = "Adicionar barra";
        detail = "Clique no no inicial da barra.";
        break;

    case EditorTool::RemoveBeam:
        title = "Remover barras";
        detail = "Clique em uma barra para remover ou arraste uma janela.";
        break;

    case EditorTool::AddDimension:
        title = "Adicionar cota";
        switch (state.dimensionTool.dimensionCreationStep)
        {
        case DimensionToolState::DimensionCreationStep::AwaitSecondNode:
            detail = "Clique no segundo no da cota.";
            break;

        case DimensionToolState::DimensionCreationStep::AwaitOffsetPoint:
            detail = "Clique para definir o offset da cota.";
            break;

        case DimensionToolState::DimensionCreationStep::AwaitChainedNode:
            detail = "Clique em um proximo no ou barra colinear para continuar com o mesmo offset.";
            break;

        case DimensionToolState::DimensionCreationStep::None:
        case DimensionToolState::DimensionCreationStep::AwaitFirstNode:
        default:
            detail = "Clique no primeiro no da cota ou em uma barra.";
            break;
        }
        break;

    case EditorTool::MoveDimension:
        title = "Ajustar cota";
        if (state.dimensionTool.dimensionMoveStep == DimensionToolState::DimensionMoveStep::AwaitOffsetPoint)
        {
            detail = "Clique para definir o novo offset da cota.";
        }
        else
        {
            detail = "Clique em uma cota para ajustar o offset.";
        }
        break;

    case EditorTool::SetSupportNone:
        title = "Apoio: nenhum";
        detail = "Clique ou arraste sobre nos para remover o apoio.";
        break;

    case EditorTool::SetSupportX:
        title = "Apoio em X";
        detail = "Clique ou arraste sobre nos para aplicar o apoio.";
        break;

    case EditorTool::SetSupportY:
        title = "Apoio em Y";
        detail = "Clique ou arraste sobre nos para aplicar o apoio.";
        break;

    case EditorTool::SetSupportXY:
        title = "Apoio em XY";
        detail = "Clique ou arraste sobre nos para aplicar o apoio.";
        break;

    case EditorTool::SetSupportFixed:
        title = "Engaste";
        detail = "Clique ou arraste sobre nos para aplicar o engaste.";
        break;

    case EditorTool::AddPointLoad:
        title = "Adicionar carga";
        detail = "Clique ou arraste sobre nos para aplicar a carga pendente.";
        break;

    case EditorTool::RemovePointLoad:
        title = "Remover carga";
        detail = "Clique ou arraste sobre nos para remover a carga.";
        break;

    case EditorTool::AddDistributedLoad:
        title = "Adicionar carga distribuida";
        detail = "Clique ou arraste sobre barras para aplicar a carga pendente.";
        break;

    case EditorTool::RemoveDistributedLoad:
        title = "Remover carga distribuida";
        detail = "Clique ou arraste sobre barras para remover a carga.";
        break;

    case EditorTool::Select:
    default:
        title = "Selecionar";
        detail = "Clique ou arraste para selecionar. Ctrl adiciona e Shift remove.";
        if (state.hover.entity.type == EntityType::Node)
        {
            detail = "No sob cursor. Clique para selecionar ou arraste para janela de selecao.";
        }
        else if (state.hover.entity.type == EntityType::Beam)
        {
            detail = "Barra sob cursor. Clique para selecionar ou arraste para janela de selecao.";
        }
        else if (state.hover.entity.type == EntityType::Dimension)
        {
            detail = "Cota sob cursor. Clique para selecionar ou arraste para janela de selecao.";
        }
        break;
    }

    appendActiveToolShortcut();
}

void Editor::DrawViewportStatusOverlay() const
{
    std::string title;
    std::string detail;
    BuildViewportStatusText(title, detail);

    const Font detailFont = hasViewportOverlayFont ? viewportOverlayFont : GetFontDefault();
    const Font titleFont = hasViewportOverlayTitleFont ? viewportOverlayTitleFont : detailFont;
    const float titleFontSize = 18.0f;
    const float detailFontSize = 18.0f;
    const float textSpacing = 0.0f;
    const float lineSpacing = 2.0f;
    const float paddingX = 8.0f;
    const float paddingY = 6.0f;

    const Vector2 titleSize = MeasureTextEx(titleFont, title.c_str(), titleFontSize, textSpacing);
    const Vector2 detailSize = MeasureTextEx(detailFont, detail.c_str(), detailFontSize, textSpacing);

    const float boxWidth = std::max(titleSize.x, detailSize.x) + paddingX * 2.0f;
    const float boxHeight =
        paddingY * 2.0f +
        titleSize.y +
        lineSpacing +
        detailSize.y;

    const Rectangle box = Rectangle{
        viewportLeftPanelWidth + 12.0f,
        static_cast<float>(GetScreenHeight()) - boxHeight - 12.0f,
        boxWidth,
        boxHeight};

    DrawRectangleRec(box, Color{255, 255, 255, 215});
    DrawRectangleLinesEx(box, 1.0f, Color{185, 185, 185, 220});

    float textY = box.y + paddingY;
    const Vector2 titlePosition = Vector2{box.x + paddingX, textY};
    DrawTextEx(
        titleFont,
        title.c_str(),
        titlePosition,
        titleFontSize,
        textSpacing,
        Color{45, 45, 45, 255});

    textY += titleSize.y + lineSpacing;
    DrawTextEx(
        detailFont,
        detail.c_str(),
        Vector2{box.x + paddingX, textY},
        detailFontSize,
        textSpacing,
        Color{75, 75, 75, 255});
}

void Editor::DrawGridScaleOverlay() const
{
    if (!state.view.showGrid)
    {
        return;
    }

    const float gridSpacingMeters = CalculateGridSpacing(100.0f, camera.zoom);
    if (gridSpacingMeters <= 0.0f)
    {
        return;
    }

    double displayValue = static_cast<double>(gridSpacingMeters);
    const char* unitLabel = "m";
    if (displayValue >= 1000.0)
    {
        displayValue /= 1000.0;
        unitLabel = "km";
    }
    else if (displayValue < 1.0 && displayValue >= 0.01)
    {
        displayValue *= 100.0;
        unitLabel = "cm";
    }
    else if (displayValue < 0.01)
    {
        displayValue *= 1000.0;
        unitLabel = "mm";
    }

    char spacingText[96] = {};
    std::snprintf(
        spacingText,
        sizeof(spacingText),
        "Grade: %g %s",
        displayValue,
        unitLabel);

    const Font font = hasViewportOverlayFont ? viewportOverlayFont : GetFontDefault();
    const float fontSize = 18.0f;
    const float textSpacing = 0.0f;
    const float paddingX = 8.0f;
    const float paddingY = 6.0f;
    const Vector2 textSize = MeasureTextEx(font, spacingText, fontSize, textSpacing);
    const Rectangle box = Rectangle{
        static_cast<float>(GetScreenWidth()) - textSize.x - paddingX * 2.0f - 12.0f,
        static_cast<float>(GetScreenHeight()) - textSize.y - paddingY * 2.0f - 12.0f,
        textSize.x + paddingX * 2.0f,
        textSize.y + paddingY * 2.0f};

    DrawRectangleRec(box, Color{255, 255, 255, 215});
    DrawRectangleLinesEx(box, 1.0f, Color{185, 185, 185, 220});
    DrawTextEx(
        font,
        spacingText,
        Vector2{box.x + paddingX, box.y + paddingY},
        fontSize,
        textSpacing,
        Color{45, 45, 45, 255});
}


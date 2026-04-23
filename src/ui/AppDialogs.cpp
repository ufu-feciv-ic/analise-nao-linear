#include "ui/AppDialogs.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <initializer_list>

#include "imgui.h"
#include "ui/ToolbarDialogRequest.h"

namespace
{
    constexpr const char* materialDialogPopupId = u8"Material###MaterialDialog";
    constexpr const char* sectionDialogPopupId  = u8"Seção###SectionDialog";
}

void AppDialogs::ApplyRequest(ToolbarDialogRequest request, const ProjectDocument& document, const EditorState& state)
{
    switch (request)
    {
    case ToolbarDialogRequest::OpenCreateMaterial:
        BeginCreateMaterial(document);
        closeMaterialDialogRequested = false;
        ImGui::OpenPopup(materialDialogPopupId);
        break;

    case ToolbarDialogRequest::OpenEditMaterial:
        BeginEditMaterial(document, state);
        if (materialDialogMode != DialogMode::None)
        {
            closeMaterialDialogRequested = false;
            ImGui::OpenPopup(materialDialogPopupId);
        }
        break;

    case ToolbarDialogRequest::OpenCreateSection:
        BeginCreateSection(document);
        closeSectionDialogRequested = false;
        ImGui::OpenPopup(sectionDialogPopupId);
        break;

    case ToolbarDialogRequest::OpenEditSection:
        BeginEditSection(document, state);
        if (sectionDialogMode != DialogMode::None)
        {
            closeSectionDialogRequested = false;
            ImGui::OpenPopup(sectionDialogPopupId);
        }
        break;

    case ToolbarDialogRequest::None:
    default:
        break;
    }
}

DialogFrameResult AppDialogs::Draw(const ProjectDocument& document, const EditorState& state)
{
    DialogFrameResult result{};
    result.document.material = DrawMaterialDialog(document, state);
    result.document.section = DrawSectionDialog(document, state);
    result.beamDistance = DrawBeamDistanceInputDialog(state);
    return result;
}

bool AppDialogs::HandleEscape()
{
    const bool materialPopupOpen = ImGui::IsPopupOpen(materialDialogPopupId);
    const bool sectionPopupOpen  = ImGui::IsPopupOpen(sectionDialogPopupId);

    if (materialDialogMode != DialogMode::None || materialPopupOpen)
    {
        closeMaterialDialogRequested = true;
        return true;
    }

    if (sectionDialogMode != DialogMode::None || sectionPopupOpen)
    {
        closeSectionDialogRequested = true;
        return true;
    }

    return false;
}

float AppDialogs::ComputeMaxTextWidth(std::initializer_list<const char*> labels) const
{
    float width = 0.0f;

    for (const char* label : labels)
    {
        width = std::max(width, ImGui::CalcTextSize(label).x);
    }

    return width;
}

float AppDialogs::ComputeDialogWidth(float labelWidth, float inputWidth, float unitWidth) const
{
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    return dialogWindowPaddingX * 2.0f + labelWidth + spacing + inputWidth + spacing + unitWidth;
}

bool AppDialogs::BeginPropertyTable(const char* id, float labelWidth, float inputWidth, float unitWidth) const
{
    if (!ImGui::BeginTable(id, 3, ImGuiTableFlags_SizingFixedFit))
    {
        return false;
    }

    ImGui::TableSetupColumn("Rótulo", ImGuiTableColumnFlags_WidthFixed, labelWidth);
    ImGui::TableSetupColumn("Valor", ImGuiTableColumnFlags_WidthFixed, inputWidth);
    ImGui::TableSetupColumn("Unidade", ImGuiTableColumnFlags_WidthFixed, unitWidth);

    return true;
}

void AppDialogs::DrawDoubleInputRow(
    const char* label,
    const char* inputId,
    double& value,
    const char* unitLabel,
    const char* format) const
{
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputDouble(inputId, &value, 0.0, 0.0, format);

    ImGui::TableSetColumnIndex(2);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(unitLabel);
}

void AppDialogs::DrawPositiveDoubleInputRow(
    const char* label,
    const char* inputId,
    double& value,
    double& lastValidValue,
    const char* unitLabel,
    const char* format) const
{
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputDouble(inputId, &value, 0.0, 0.0, format);

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        if (value > 0.0)
        {
            lastValidValue = value;
        }
        else
        {
            value = lastValidValue;
        }
    }
    else if (!ImGui::IsItemActive() && value > 0.0)
    {
        lastValidValue = value;
    }

    ImGui::TableSetColumnIndex(2);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(unitLabel);
}

void AppDialogs::DrawDialogButtons() const
{
    const float totalWidth = dialogButtonWidth * 2.0f + ImGui::GetStyle().ItemSpacing.x;
    const float availableWidth = ImGui::GetContentRegionAvail().x;

    if (availableWidth > totalWidth)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - totalWidth) * 0.5f);
    }
}



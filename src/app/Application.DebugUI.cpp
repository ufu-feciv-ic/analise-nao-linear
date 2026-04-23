#include "app/Application.h"

#include "imgui.h"

void Application::DrawBeamRenderTestWindow()
{
    if (!editor.state.view.showBeamRenderTestWindow)
    {
        return;
    }

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowPos(ImVec2(90.0f, 190.0f), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Teste render barras", &editor.state.view.showBeamRenderTestWindow, flags))
    {
        ImGui::End();
        return;
    }

    ImGui::SetNextItemWidth(180.0f);
    ImGui::SliderFloat("Esp. barra", &editor.state.view.beamRenderFillThickness, 1.0f, 14.0f, "%.2f");
    ImGui::SetNextItemWidth(180.0f);
    ImGui::SliderFloat("Esp. linha inferior", &editor.state.view.beamRenderLowerEdgeThickness, 0.0f, 8.0f, "%.2f");
    ImGui::SetNextItemWidth(180.0f);
    ImGui::SliderFloat("Esp. linha superior", &editor.state.view.beamRenderUpperEdgeThickness, 0.0f, 8.0f, "%.2f");

    auto editColor = [](const char* label, Color& color)
    {
        float values[4] = {
            static_cast<float>(color.r) / 255.0f,
            static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f,
            static_cast<float>(color.a) / 255.0f};

        if (ImGui::ColorEdit4(
                label,
                values,
                ImGuiColorEditFlags_PickerHueWheel |
                ImGuiColorEditFlags_DisplayRGB))
        {
            color = Color{
                static_cast<unsigned char>(values[0] * 255.0f + 0.5f),
                static_cast<unsigned char>(values[1] * 255.0f + 0.5f),
                static_cast<unsigned char>(values[2] * 255.0f + 0.5f),
                static_cast<unsigned char>(values[3] * 255.0f + 0.5f)};
        }
    };

    editColor("Cor barra", editor.state.view.beamRenderFillColor);
    editColor("Cor linha inferior", editor.state.view.beamRenderLowerEdgeColor);
    editColor("Cor linha superior", editor.state.view.beamRenderUpperEdgeColor);

    ImGui::End();
}


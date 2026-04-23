#include "app/Application.h"

#include <vector>

#include "imgui.h"
#include "rlImGui.h"

namespace
{
    const ImWchar* GetCommonFontGlyphRanges()
    {
        static const ImWchar ranges[] = {
            0x0020, 0x00FF, // ASCII + Latin-1 supplement
            0x0100, 0x017F, // Latin Extended-A
            0x0370, 0x03FF, // Greek and Coptic
            0x2013, 0x2014, // en dash, em dash
            0x2018, 0x201A, // single quotes
            0x201C, 0x201E, // double quotes
            0x2022, 0x2022, // bullet
            0x2026, 0x2026, // ellipsis
            0x20AC, 0x20AC, // euro
            0x2190, 0x2193, // arrows
            0
        };

        return ranges;
    }

    std::vector<int> BuildCommonFontCodepoints()
    {
        std::vector<int> codepoints;
        auto appendRange = [&](int first, int last)
        {
            for (int codepoint = first; codepoint <= last; ++codepoint)
            {
                codepoints.push_back(codepoint);
            }
        };

        appendRange(0x0020, 0x00FF);
        appendRange(0x0100, 0x017F);
        appendRange(0x0370, 0x03FF);
        appendRange(0x2013, 0x2014);
        appendRange(0x2018, 0x201A);
        appendRange(0x201C, 0x201E);
        appendRange(0x2022, 0x2022);
        appendRange(0x2026, 0x2026);
        appendRange(0x20AC, 0x20AC);
        appendRange(0x2190, 0x2193);

        return codepoints;
    }
}

void Application::LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    constexpr const char* uiFontPath = "resources/fonts/segoeuisl.ttf";
    constexpr const char* uiBoldFontPath = "resources/fonts/segoeuib.ttf";
    constexpr const char* dimensionFontPath = "resources/fonts/segoeui.ttf";
    std::vector<int> uiCodepoints = BuildCommonFontCodepoints();
    ImFontConfig fontConfig{};

    ImFont* font = io.Fonts->AddFontFromFileTTF(
        uiFontPath,
        18.0f,
        &fontConfig,
        GetCommonFontGlyphRanges());

    if (font != nullptr)
    {
        io.FontDefault = font;
    }

    uiRaylibFont = LoadFontEx(
        uiFontPath,
        18,
        uiCodepoints.data(),
        static_cast<int>(uiCodepoints.size()));
    hasUiRaylibFont = uiRaylibFont.texture.id != 0;
    if (hasUiRaylibFont)
    {
        SetTextureFilter(uiRaylibFont.texture, TEXTURE_FILTER_BILINEAR);
        editor.SetViewportOverlayFont(uiRaylibFont);
    }

    uiRaylibBoldFont = LoadFontEx(
        uiBoldFontPath,
        18,
        uiCodepoints.data(),
        static_cast<int>(uiCodepoints.size()));
    hasUiRaylibBoldFont = uiRaylibBoldFont.texture.id != 0;
    if (hasUiRaylibBoldFont)
    {
        SetTextureFilter(uiRaylibBoldFont.texture, TEXTURE_FILTER_BILINEAR);
        editor.SetViewportOverlayTitleFont(uiRaylibBoldFont);
    }

    dimensionRaylibFont = LoadFontEx(
        dimensionFontPath,
        22,
        uiCodepoints.data(),
        static_cast<int>(uiCodepoints.size()));
    hasDimensionRaylibFont = dimensionRaylibFont.texture.id != 0;
    if (hasDimensionRaylibFont)
    {
        SetTextureFilter(dimensionRaylibFont.texture, TEXTURE_FILTER_BILINEAR);
        editor.SetDimensionFont(dimensionRaylibFont);
    }

    rlImGuiReloadFonts();
}


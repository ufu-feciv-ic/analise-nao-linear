#include "app/Application.h"
#include "app/AppVersion.h"

#include <raylib.h>

#include "imgui.h"
#include "rlImGui.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void Application::Initialize()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, TextFormat("2d-editor %s", AppVersion::String));
    SetExitKey(KEY_NULL);
    SetTargetFPS(244);

    rlImGuiSetup(false);
    ImGui::GetIO().IniFilename = nullptr;

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_PopupBg] = style.Colors[ImGuiCol_WindowBg];
    style.FrameRounding = 2.0f;

    LoadFonts();

    editor.Initialize();
}

void Application::Shutdown()
{
    if (hasUiRaylibFont)
    {
        UnloadFont(uiRaylibFont);
        hasUiRaylibFont = false;
    }

    if (hasUiRaylibBoldFont)
    {
        UnloadFont(uiRaylibBoldFont);
        hasUiRaylibBoldFont = false;
    }

    if (hasDimensionRaylibFont)
    {
        UnloadFont(dimensionRaylibFont);
        hasDimensionRaylibFont = false;
    }

    rlImGuiShutdown();
    CloseWindow();
}

void Application::BeginFrame()
{
    BeginDrawing();
    ClearBackground(WHITE);

    rlImGuiBegin();
}

void Application::Update()
{
    bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool altDown = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
    if (ctrlDown && altDown && IsKeyPressed(KEY_F))
    {
        showFps = !showFps;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (dialogs.HandleEscape())
        {
            return;
        }

        if (editor.HandleEscape())
        {
            return;
        }
    }
}

void Application::Render()
{
    UiFrameEdits frameEdits{};
    BuildUiFrameEdits(frameEdits);
    ApplyUiFrameEdits(frameEdits);

    ImGuiIO& io = ImGui::GetIO();
    const bool mouseWorldInputAllowed = !io.WantCaptureMouse;
    const bool keyboardShortcutsAllowed = !io.WantCaptureKeyboard && !io.WantTextInput;

    editor.Update(
        frameEdits.requests,
        mouseWorldInputAllowed,
        keyboardShortcutsAllowed,
        frameEdits.dialogResults.beamDistance.action == BeamDistanceDialogResult::Action::Confirm,
        frameEdits.dialogResults.beamDistance.action == BeamDistanceDialogResult::Action::Cancel,
        frameEdits.dialogResults.beamDistance.createdNodeWorld);
    editor.Render();
}

void Application::EndFrame()
{
    rlImGuiEnd();
    
    if (showFps)
    {
        DrawFPS(80, 160);
    }
    
    EndDrawing();
}

bool Application::ShouldClose() const
{
    return WindowShouldClose();
}

void Application::UpdateDrawFrame()
{
    BeginFrame();
    Update();
    Render();
    EndFrame();
}

#if defined(PLATFORM_WEB)
void Application::UpdateDrawFrameCallback(void* arg)
{
    static_cast<Application*>(arg)->UpdateDrawFrame();
}
#endif

int Application::Run()
{
    Initialize();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg(UpdateDrawFrameCallback, this, 0, 1);
#else
    while (!ShouldClose())
    {
        UpdateDrawFrame();
    }

    Shutdown();
#endif

    return 0;
}

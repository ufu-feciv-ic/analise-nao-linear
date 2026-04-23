#pragma once

#include "ui/LeftPanel.h"
#include "ui/TopToolbar.h"
#include "editor/Editor.h"
#include "ui/AppDialogs.h"
#include "ui/DialogResult.h"
#include "editor/FrameRequests.h"
#include "raylib.h"

class Application
{
public:
    int Run();

private:
    struct UiFrameEdits
    {
        FrameRequests requests{};
        DialogFrameResult dialogResults{};
    };

private:
    void Initialize();
    void Shutdown();

    void BeginFrame();
    void Update();
    void Render();
    void EndFrame();

    void UpdateDrawFrame();
#if defined(PLATFORM_WEB)
    static void UpdateDrawFrameCallback(void* arg);
#endif

    bool ShouldClose() const;

    void LoadFonts();

    void BuildUiFrameEdits(UiFrameEdits& edits);
    void ApplyUiFrameEdits(const UiFrameEdits& edits);
    void ApplyBeamDistanceDialogResult(const BeamDistanceDialogResult& result);

    void ApplyDocumentRequest(DocumentRequest request);
    void ApplyDialogResults(const AppDialogResults& results);
    void ApplyFrameEdits(const FrameRequests& requests, const AppDialogResults& dialogResults);
    void ApplyNodalLoadEditRequest(const FrameRequests::NodalLoadSelectionEditRequest& request);
    void ApplyDistributedLoadEditRequest(const FrameRequests::DistributedLoadSelectionEditRequest& request);
    void ApplyDisplayUnitsUpdateRequest(const FrameRequests::DisplayUnitsUpdateRequest& request);
    void ApplyBeamPropertyEditRequest(const FrameRequests::BeamPropertySelectionEditRequest& request);
    void ApplyPropertyPanelStateSyncRequest(const FrameRequests::PropertyPanelStateSyncRequest& request);
    void ApplyDimensionEditRequest(const FrameRequests::DimensionSelectionEditRequest& request);
    void ApplyDimensionToolStateSyncRequest(const FrameRequests::DimensionToolStateSyncRequest& request);
    void ApplyLoadToolStateSyncRequest(const FrameRequests::LoadToolStateSyncRequest& request);
    void ApplyAnalysisRequest(const FrameRequests::AnalysisRequest& request);
    void ApplyResultsViewRequest(const FrameRequests::ResultsViewRequest& request);
    void DrawBeamRenderTestWindow();

private:
    Editor editor;
    TopToolbar topToolbar;
    LeftPanel leftPanel;
    AppDialogs dialogs;
    Font uiRaylibFont{};
    bool hasUiRaylibFont = false;
    Font uiRaylibBoldFont{};
    bool hasUiRaylibBoldFont = false;
    Font dimensionRaylibFont{};
    bool hasDimensionRaylibFont = false;
    bool showFps = false;
};

#include "app/Application.h"

void Application::BuildUiFrameEdits(UiFrameEdits& edits)
{
    topToolbar.FillRequests(edits.requests, editor.document, editor.state, editor.camera.zoom);
    leftPanel.Draw(editor.state);
    // DrawBeamRenderTestWindow();

    dialogs.ApplyRequest(edits.requests.dialog, editor.document, editor.state);
    edits.dialogResults = dialogs.Draw(editor.document, editor.state);
}

void Application::ApplyUiFrameEdits(const UiFrameEdits& edits)
{
    ApplyBeamDistanceDialogResult(edits.dialogResults.beamDistance);
    ApplyFrameEdits(edits.requests, edits.dialogResults.document);
}

void Application::ApplyBeamDistanceDialogResult(const BeamDistanceDialogResult& result)
{
    if (result.hasPreviewNode)
    {
        editor.state.beamTool.beamDistanceCreatedNodeWorld = result.previewNodeWorld;
    }

    if (result.shouldCloseDialog)
    {
        editor.state.beamTool.isBeamDistanceInputOpen = false;
    }
}

#include "editor/Editor.h"

namespace
{
    constexpr const char* debugProjectFilePath = "saved-project.json";
}

void Editor::HandleKeyboardShortcuts()
{
    const double currentTime = GetTime();
    const double doublePressWindowSeconds = 0.15;
    const bool controlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    const bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (controlDown && shiftDown && IsKeyPressed(KEY_Z))
    {
        RedoDocumentChange();
        return;
    }

    if (controlDown && IsKeyPressed(KEY_Y))
    {
        RedoDocumentChange();
        return;
    }

    if (controlDown && IsKeyPressed(KEY_Z))
    {
        UndoDocumentChange();
        return;
    }

    if (controlDown && IsKeyPressed(KEY_S))
    {
        SaveDocumentToJson(debugProjectFilePath);
        return;
    }

    if (controlDown && IsKeyPressed(KEY_L))
    {
        LoadDocumentFromJson(debugProjectFilePath);
        return;
    }

    if (state.transformTool.gridShortcutPending &&
        (currentTime - state.transformTool.lastGridShortcutTime) > doublePressWindowSeconds)
    {
        state.view.showGrid = !state.view.showGrid;
        state.transformTool.gridShortcutPending = false;
        state.transformTool.lastGridShortcutTime = -100.0;
    }

    if (state.activeTool == EditorTool::MirrorSelection && IsKeyPressed(KEY_O))
    {
        state.transformTool.mirrorKeepsOriginal = !state.transformTool.mirrorKeepsOriginal;
        return;
    }

    if (!state.beamTool.isBeamDistanceInputOpen && IsKeyPressed(KEY_SPACE))
    {
        if (state.activeTool == EditorTool::Select)
        {
            const EditorRequest lastToolRequest =
                editorRequestController.GetActivationRequestForTool(state.lastSelectedTool);
            if (lastToolRequest != EditorRequest::None)
            {
                editorRequestController.Apply(*this, lastToolRequest);
                return;
            }
        }

        const bool isTransformSelectionConfirm =
            IsTransformTool(state.activeTool) &&
            state.transformTool.moveSelectionStep == TransformToolState::MoveSelectionStep::AwaitSelectionConfirm;
        const bool isBeamContinuationPhase =
            state.activeTool == EditorTool::AddBeam && state.beamTool.isCreatingBeam;

        if (!isTransformSelectionConfirm && !isBeamContinuationPhase)
        {
            state.CancelCurrentTool();
            return;
        }
    }

    if (IsKeyPressed(KEY_N))
    {
        editorRequestController.Apply(*this, EditorRequest::ActivateAddNodeTool);
        return;
    }

    if (IsKeyPressed(KEY_B))
    {
        editorRequestController.Apply(*this, EditorRequest::ActivateAddBeamTool);
        return;
    }

    if (IsKeyPressed(KEY_D))
    {
        editorRequestController.Apply(*this, EditorRequest::ActivateAddDimensionTool);
        return;
    }

    if (IsKeyPressed(KEY_M))
    {
        editorRequestController.Apply(*this, EditorRequest::ActivateMoveNodeTool);
        return;
    }

    if (IsKeyPressed(KEY_C))
    {
        editorRequestController.Apply(*this, EditorRequest::ActivateCopySelectionTool);
        return;
    }

    if (IsKeyPressed(KEY_E))
    {
        editorRequestController.Apply(*this, EditorRequest::ActivateMirrorSelectionTool);
        return;
    }

    if (IsKeyPressed(KEY_S))
    {
        state.view.snapToGrid = !state.view.snapToGrid;
        return;
    }

    if (IsKeyPressed(KEY_G))
    {
        if (state.transformTool.gridShortcutPending &&
            (currentTime - state.transformTool.lastGridShortcutTime) <= doublePressWindowSeconds)
        {
            state.view.showGuides = !state.view.showGuides;
            state.transformTool.gridShortcutPending = false;
            state.transformTool.lastGridShortcutTime = -100.0;
            return;
        }

        state.transformTool.gridShortcutPending = true;
        state.transformTool.lastGridShortcutTime = currentTime;
        return;
    }
}


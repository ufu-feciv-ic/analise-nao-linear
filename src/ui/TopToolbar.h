#pragma once

#include "editor/FrameRequests.h"
#include "editor/EditorState.h"
#include "model/ProjectDocument.h"

class TopToolbar
{
public:
    void FillRequests(
        FrameRequests& requests,
        ProjectDocument& projectDocument,
        EditorState& editorState,
        float cameraZoom);

private:
    void DrawHomeTab(
        FrameRequests& requests,
        ProjectDocument& projectDocument,
        EditorState& editorState,
        float cameraZoom);
    void DrawLoadsTab(FrameRequests& requests, ProjectDocument& projectDocument, EditorState& editorState);
    void DrawPropertiesTab(FrameRequests& requests, ProjectDocument& projectDocument, EditorState& editorState);
};

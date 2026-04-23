#pragma once

#include "editor/EditorTool.h"
#include "editor/FrameRequests.h"

class Editor;

class EditorRequestController
{
public:
    void Apply(Editor& editor, EditorRequest request) const;
    EditorRequest GetActivationRequestForTool(EditorTool tool) const;
};

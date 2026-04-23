#pragma once

#include "editor/EditorState.h"
#include "editor/SelectionMode.h"
#include "model/ProjectDocument.h"
#include "raylib.h"

class Editor;

struct EditorContext
{
    Editor& editor;
    ProjectDocument& document;
    EditorState& state;
    Camera2D& camera;

    Vector2 mouseScreenPosition{0.0f, 0.0f};
    Vector2 mouseWorldPosition{0.0f, 0.0f};
    Vector2 snappedWorldPosition{0.0f, 0.0f};

    bool mouseWorldInputAllowed = false;
    bool keyboardShortcutsAllowed = false;

    Node* hoveredNode = nullptr;
    Beam* hoveredBeam = nullptr;
    Dimension* hoveredDimension = nullptr;
    SelectionMode selectionMode = SelectionMode::Replace;
};

#pragma once

#include "editor/Selection.h"
#include "raylib.h"

struct SelectionState
{
    Selection selection;
    bool isBoxSelecting = false;
    Vector2 boxSelectStartWorld{0.0f, 0.0f};
    Vector2 boxSelectEndWorld{0.0f, 0.0f};
};

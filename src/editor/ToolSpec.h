#pragma once

#include <string_view>

#include "editor/EditorTool.h"
#include "raylib.h"

struct ToolSpec
{
    EditorTool id = EditorTool::Select;
    std::string_view label;
    KeyboardKey shortcut = KEY_NULL;
    std::string_view shortDescription;
    bool staysActive = true;
};

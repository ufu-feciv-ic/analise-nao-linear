#pragma once

#include "editor/EditorContext.h"
#include "editor/EditorTool.h"

class ToolController
{
public:
    virtual ~ToolController() = default;

    virtual bool Handles(EditorTool tool) const = 0;
    virtual void Update(EditorContext& context) = 0;
};

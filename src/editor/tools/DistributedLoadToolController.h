#pragma once

#include "editor/tools/ToolController.h"

class DistributedLoadToolController : public ToolController
{
public:
    bool Handles(EditorTool tool) const override;
    void Update(EditorContext& context) override;
};

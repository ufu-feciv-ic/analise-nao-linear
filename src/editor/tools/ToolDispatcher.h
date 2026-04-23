#pragma once

#include "editor/EditorContext.h"
#include "editor/tools/AddNodeToolController.h"
#include "editor/tools/BeamToolController.h"
#include "editor/tools/DeleteToolController.h"
#include "editor/tools/DimensionToolController.h"
#include "editor/tools/DistributedLoadToolController.h"
#include "editor/tools/PointLoadToolController.h"
#include "editor/tools/SelectToolController.h"
#include "editor/tools/SupportToolController.h"
#include "editor/tools/TransformToolController.h"

class ToolDispatcher
{
public:
    void Dispatch(
        EditorContext& context,
        SelectToolController& selectToolController,
        AddNodeToolController& addNodeToolController,
        TransformToolController& transformToolController,
        BeamToolController& beamToolController,
        DimensionToolController& dimensionToolController,
        SupportToolController& supportToolController,
        PointLoadToolController& pointLoadToolController,
        DistributedLoadToolController& distributedLoadToolController,
        DeleteToolController& deleteToolController) const;
};

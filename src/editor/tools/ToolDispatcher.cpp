#include "editor/tools/ToolDispatcher.h"

void ToolDispatcher::Dispatch(
    EditorContext& context,
    SelectToolController& selectToolController,
    AddNodeToolController& addNodeToolController,
    TransformToolController& transformToolController,
    BeamToolController& beamToolController,
    DimensionToolController& dimensionToolController,
    SupportToolController& supportToolController,
    PointLoadToolController& pointLoadToolController,
    DistributedLoadToolController& distributedLoadToolController,
    DeleteToolController& deleteToolController) const
{
    const EditorTool activeTool = context.state.activeTool;

    if (selectToolController.Handles(activeTool))
    {
        selectToolController.Update(context);
        return;
    }

    if (addNodeToolController.Handles(activeTool))
    {
        addNodeToolController.Update(context);
        return;
    }

    if (transformToolController.Handles(activeTool))
    {
        transformToolController.Update(context);
        return;
    }

    if (beamToolController.Handles(activeTool))
    {
        EditorContext beamContext = context;
        beamContext.selectionMode = SelectionMode::Replace;
        beamContext.hoveredBeam = nullptr;
        beamContext.hoveredDimension = nullptr;
        beamToolController.Update(beamContext);
        return;
    }

    if (dimensionToolController.Handles(activeTool))
    {
        EditorContext dimensionContext = context;
        dimensionContext.selectionMode = SelectionMode::Replace;
        if (activeTool == EditorTool::MoveDimension)
        {
            dimensionContext.hoveredNode = nullptr;
            dimensionContext.hoveredBeam = nullptr;
            dimensionContext.hoveredDimension =
                context.document.FindDimensionById(context.state.hover.HoveredDimensionId());
        }
        dimensionToolController.Update(dimensionContext);
        return;
    }

    if (supportToolController.Handles(activeTool))
    {
        supportToolController.Update(context);
        return;
    }

    if (pointLoadToolController.Handles(activeTool))
    {
        pointLoadToolController.Update(context);
        return;
    }

    if (distributedLoadToolController.Handles(activeTool))
    {
        distributedLoadToolController.Update(context);
        return;
    }

    if (deleteToolController.Handles(activeTool))
    {
        deleteToolController.Update(context);
    }
}

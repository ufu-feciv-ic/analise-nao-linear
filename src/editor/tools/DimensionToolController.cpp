#include "editor/tools/DimensionToolController.h"

#include <algorithm>
#include <utility>

#include "editor/Editor.h"
#include "editor/ops/DimensionOperations.h"

bool DimensionToolController::Handles(EditorTool tool) const
{
    return tool == EditorTool::AddDimension ||
           tool == EditorTool::MoveDimension;
}

void DimensionToolController::Update(EditorContext& context)
{
    const EditorTool tool = context.state.activeTool;
    if (!Handles(tool))
    {
        return;
    }

    if (tool == EditorTool::AddDimension)
    {
        auto tryAddDimension = [&](
                                   int startNodeId,
                                   int endNodeId,
                                   DimensionType type,
                                   LengthUnit lengthUnit,
                                   DimensionOffsetMode offsetMode,
                                   double offsetPixels,
                                   double offsetWorld)
        {
            ProjectDocument beforeDocument = context.document;
            if (!DimensionOperations::TryAddDimensionBetweenNodes(
                    context.document,
                    startNodeId,
                    endNodeId,
                    type,
                    lengthUnit,
                    offsetMode,
                    offsetPixels,
                    offsetWorld))
            {
                return false;
            }

            context.state.dimensionTool.dimensionLengthUnit = lengthUnit;
            context.state.dimensionTool.dimensionType = type;
            context.state.dimensionTool.dimensionOffsetMode = offsetMode;
            context.state.dimensionTool.dimensionOffsetPixels = offsetPixels;
            context.state.dimensionTool.dimensionOffsetWorld = offsetWorld;
            context.editor.RecordDocumentChange(std::move(beforeDocument), true);
            return true;
        };

        context.state.dimensionTool.dimensionPreviewPointWorld = context.editor.GetCurrentPlacementWorldPosition();
        context.state.dimensionTool.dimensionPreviewScreenPosition = context.mouseScreenPosition;
        const Dimension* hoveredDimension =
            context.document.FindDimensionById(context.state.hover.HoveredDimensionId());

        if (context.state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitOffsetPoint)
        {
            if (hoveredDimension != nullptr)
            {
                DimensionOperations::TryAlignDimensionToReference(
                    context.document,
                    context.camera,
                    context.state.dimensionTool.dimensionStartNodeId,
                    context.state.dimensionTool.dimensionEndNodeId,
                    *hoveredDimension,
                    context.state.dimensionTool.dimensionType,
                    context.state.dimensionTool.dimensionOffsetPixels,
                    context.state.dimensionTool.dimensionOffsetWorld);
            }
            else
            {
                DimensionOperations::UpdateDimensionPreviewFromMouse(
                    context.document,
                    context.camera,
                    context.state.dimensionTool.dimensionPreviewScreenPosition,
                    context.state.dimensionTool.dimensionStartNodeId,
                    context.state.dimensionTool.dimensionEndNodeId,
                    context.state.dimensionTool.dimensionType,
                    context.state.dimensionTool.dimensionOffsetPixels,
                    context.state.dimensionTool.dimensionOffsetWorld);
            }
        }
        // AwaitChainedNode: flip start/reference if the mouse is closer to the reference node.
        // This allows the user to chain a dimension "backwards" — e.g. after placing B→C,
        // moving toward A will anchor from B instead of always anchoring from C.
        else if (context.state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitChainedNode &&
                 context.state.dimensionTool.dimensionChainReferenceNodeId >= 0)
        {
            const Node* startNode = context.document.FindNodeById(context.state.dimensionTool.dimensionStartNodeId);
            const Node* refNode   = context.document.FindNodeById(context.state.dimensionTool.dimensionChainReferenceNodeId);
            if (startNode != nullptr && refNode != nullptr)
            {
                const float dxS = static_cast<float>(startNode->position.x) - context.mouseWorldPosition.x;
                const float dyS = static_cast<float>(startNode->position.y) - context.mouseWorldPosition.y;
                const float dxR = static_cast<float>(refNode->position.x) - context.mouseWorldPosition.x;
                const float dyR = static_cast<float>(refNode->position.y) - context.mouseWorldPosition.y;
                if ((dxR * dxR + dyR * dyR) < (dxS * dxS + dyS * dyS))
                {
                    std::swap(context.state.dimensionTool.dimensionStartNodeId,
                              context.state.dimensionTool.dimensionChainReferenceNodeId);
                }
            }
        }

        if (context.state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitChainedNode &&
                 context.hoveredNode != nullptr &&
                 context.hoveredNode->id != context.state.dimensionTool.dimensionStartNodeId &&
                 DimensionOperations::IsNodeCollinearWithDimensionChain(
                     context.document,
                     context.state,
                     context.hoveredNode->id))
        {
            context.state.dimensionTool.dimensionEndNodeId = context.hoveredNode->id;
        }
        else if (context.state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitChainedNode &&
                 context.hoveredBeam != nullptr)
        {
            int chainedNodeId = -1;
            if (DimensionOperations::TryGetChainedDimensionNodeFromBeam(
                    context.document,
                    context.state,
                    *context.hoveredBeam,
                    context.mouseWorldPosition,
                    chainedNodeId))
            {
                context.state.dimensionTool.dimensionEndNodeId = chainedNodeId;
            }
            else
            {
                context.state.dimensionTool.dimensionEndNodeId = -1;
            }
        }
        else if (context.state.dimensionTool.dimensionCreationStep == DimensionToolState::DimensionCreationStep::AwaitChainedNode)
        {
            context.state.dimensionTool.dimensionEndNodeId = -1;
        }

        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            return;
        }

        switch (context.state.dimensionTool.dimensionCreationStep)
        {
        case DimensionToolState::DimensionCreationStep::None:
        case DimensionToolState::DimensionCreationStep::AwaitFirstNode:
            if (context.hoveredNode != nullptr)
            {
                context.state.dimensionTool.dimensionStartNodeId = context.hoveredNode->id;
                context.state.dimensionTool.dimensionEndNodeId = -1;
                context.state.dimensionTool.dimensionChainReferenceNodeId = -1;
                context.state.dimensionTool.dimensionCreationStep = DimensionToolState::DimensionCreationStep::AwaitSecondNode;
            }
            else if (context.hoveredBeam != nullptr)
            {
                DimensionOperations::TryBeginDimensionCreationFromBeam(
                    context.document,
                    context.state,
                    context.camera,
                    *context.hoveredBeam);
            }
            break;

        case DimensionToolState::DimensionCreationStep::AwaitSecondNode:
            if (context.hoveredNode != nullptr && context.hoveredNode->id != context.state.dimensionTool.dimensionStartNodeId)
            {
                context.state.dimensionTool.dimensionEndNodeId = context.hoveredNode->id;
                context.state.dimensionTool.dimensionLengthUnit = context.state.dimensionTool.newDimensionLengthUnit;
                context.state.dimensionTool.dimensionType = DimensionType::Aligned;
                context.state.dimensionTool.dimensionOffsetMode = context.state.dimensionTool.newDimensionOffsetMode;
                context.state.dimensionTool.dimensionOffsetPixels = 32.0;
                context.state.dimensionTool.dimensionOffsetWorld = 32.0 / static_cast<double>(context.camera.zoom);
                context.state.dimensionTool.dimensionCreationStep = DimensionToolState::DimensionCreationStep::AwaitOffsetPoint;
            }
            break;

        case DimensionToolState::DimensionCreationStep::AwaitOffsetPoint:
        {
            DimensionType targetType = context.state.dimensionTool.dimensionType;
            DimensionOffsetMode targetOffsetMode = context.state.dimensionTool.dimensionOffsetMode;
            double targetOffsetPixels = context.state.dimensionTool.dimensionOffsetPixels;
            double targetOffsetWorld = context.state.dimensionTool.dimensionOffsetWorld;
            const bool hasPreview =
                hoveredDimension != nullptr
                    ? DimensionOperations::TryAlignDimensionToReference(
                          context.document,
                          context.camera,
                          context.state.dimensionTool.dimensionStartNodeId,
                          context.state.dimensionTool.dimensionEndNodeId,
                          *hoveredDimension,
                          targetType,
                          targetOffsetPixels,
                          targetOffsetWorld)
                    : DimensionOperations::UpdateDimensionPreviewFromMouse(
                          context.document,
                          context.camera,
                          context.state.dimensionTool.dimensionPreviewScreenPosition,
                          context.state.dimensionTool.dimensionStartNodeId,
                          context.state.dimensionTool.dimensionEndNodeId,
                          targetType,
                          targetOffsetPixels,
                          targetOffsetWorld);

            if (hasPreview &&
                tryAddDimension(
                    context.state.dimensionTool.dimensionStartNodeId,
                    context.state.dimensionTool.dimensionEndNodeId,
                    targetType,
                    context.state.dimensionTool.dimensionLengthUnit,
                    targetOffsetMode,
                    targetOffsetPixels,
                    targetOffsetWorld))
            {
                context.state.dimensionTool.dimensionChainReferenceNodeId = context.state.dimensionTool.dimensionStartNodeId;
                context.state.dimensionTool.dimensionStartNodeId = context.state.dimensionTool.dimensionEndNodeId;
                context.state.dimensionTool.dimensionEndNodeId = -1;
                context.state.dimensionTool.dimensionCreationStep = DimensionToolState::DimensionCreationStep::AwaitChainedNode;
            }
            break;
        }

        case DimensionToolState::DimensionCreationStep::AwaitChainedNode:
            if (context.hoveredNode != nullptr && context.hoveredNode->id != context.state.dimensionTool.dimensionStartNodeId)
            {
                if (DimensionOperations::IsNodeCollinearWithDimensionChain(
                        context.document,
                        context.state,
                        context.hoveredNode->id))
                {
                    const int previousStartNodeId = context.state.dimensionTool.dimensionStartNodeId;
                    if (tryAddDimension(
                            previousStartNodeId,
                            context.hoveredNode->id,
                            context.state.dimensionTool.dimensionType,
                            context.state.dimensionTool.dimensionLengthUnit,
                            context.state.dimensionTool.dimensionOffsetMode,
                            context.state.dimensionTool.dimensionOffsetPixels,
                            context.state.dimensionTool.dimensionOffsetWorld))
                    {
                        context.state.dimensionTool.dimensionChainReferenceNodeId = previousStartNodeId;
                        context.state.dimensionTool.dimensionStartNodeId = context.hoveredNode->id;
                        context.state.dimensionTool.dimensionEndNodeId = -1;
                    }
                }
                else
                {
                    context.state.dimensionTool.dimensionEndNodeId = context.hoveredNode->id;
                    context.state.dimensionTool.dimensionCreationStep = DimensionToolState::DimensionCreationStep::AwaitOffsetPoint;
                }
            }
            else if (context.hoveredBeam != nullptr)
            {
                int chainedNodeId = -1;
                if (DimensionOperations::TryGetChainedDimensionNodeFromBeam(
                        context.document,
                        context.state,
                        *context.hoveredBeam,
                        context.mouseWorldPosition,
                        chainedNodeId))
                {
                    const int previousStartNodeId = context.state.dimensionTool.dimensionStartNodeId;
                    if (tryAddDimension(
                            previousStartNodeId,
                            chainedNodeId,
                            context.state.dimensionTool.dimensionType,
                            context.state.dimensionTool.dimensionLengthUnit,
                            context.state.dimensionTool.dimensionOffsetMode,
                            context.state.dimensionTool.dimensionOffsetPixels,
                            context.state.dimensionTool.dimensionOffsetWorld))
                    {
                        context.state.dimensionTool.dimensionChainReferenceNodeId = previousStartNodeId;
                        context.state.dimensionTool.dimensionStartNodeId = chainedNodeId;
                        context.state.dimensionTool.dimensionEndNodeId = -1;
                    }
                }
            }
            break;
        }

        return;
    }

    context.state.dimensionTool.dimensionPreviewScreenPosition = context.mouseScreenPosition;
    const Dimension* hoveredDimension =
        context.document.FindDimensionById(context.state.hover.HoveredDimensionId());
    auto isMovingDimensionTarget = [&](int dimensionId)
    {
        return std::find(
                   context.state.dimensionTool.movingDimensionIds.begin(),
                   context.state.dimensionTool.movingDimensionIds.end(),
                   dimensionId) != context.state.dimensionTool.movingDimensionIds.end();
    };

    if (context.state.dimensionTool.dimensionMoveStep == DimensionToolState::DimensionMoveStep::AwaitOffsetPoint)
    {
        const Dimension* movingDimension = context.document.FindDimensionById(context.state.dimensionTool.movingDimensionId);
        if (movingDimension != nullptr)
        {
            if (hoveredDimension != nullptr && !isMovingDimensionTarget(hoveredDimension->id))
            {
                DimensionOperations::TryAlignDimensionToReference(
                    context.document,
                    context.camera,
                    movingDimension->startNodeId,
                    movingDimension->endNodeId,
                    *hoveredDimension,
                    context.state.dimensionTool.dimensionType,
                    context.state.dimensionTool.dimensionOffsetPixels,
                    context.state.dimensionTool.dimensionOffsetWorld);
            }
            else
            {
                DimensionOperations::UpdateDimensionPreviewFromMouse(
                    context.document,
                    context.camera,
                    context.state.dimensionTool.dimensionPreviewScreenPosition,
                    movingDimension->startNodeId,
                    movingDimension->endNodeId,
                    context.state.dimensionTool.dimensionType,
                    context.state.dimensionTool.dimensionOffsetPixels,
                    context.state.dimensionTool.dimensionOffsetWorld);
            }
        }
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return;
    }

    if (context.state.dimensionTool.dimensionMoveStep == DimensionToolState::DimensionMoveStep::AwaitOffsetPoint)
    {
        ProjectDocument beforeDocument = context.document;
        std::vector<int> movingDimensionIds = context.state.dimensionTool.movingDimensionIds;
        if (movingDimensionIds.empty() && context.state.dimensionTool.movingDimensionId >= 0)
        {
            movingDimensionIds.push_back(context.state.dimensionTool.movingDimensionId);
        }

        if (DimensionOperations::ApplyDimensionMoveOffsets(
                context.document,
                movingDimensionIds,
                context.state.dimensionTool.dimensionType,
                context.state.dimensionTool.dimensionOffsetMode,
                context.state.dimensionTool.dimensionOffsetPixels,
                context.state.dimensionTool.dimensionOffsetWorld))
        {
            context.editor.RecordDocumentChange(std::move(beforeDocument), true);
        }
        context.state.dimensionTool.dimensionMoveStep = DimensionToolState::DimensionMoveStep::AwaitDimensionSelection;
        context.state.dimensionTool.movingDimensionId = -1;
        context.state.dimensionTool.movingDimensionIds.clear();
        return;
    }

    if (hoveredDimension != nullptr)
    {
        DimensionOperations::BeginDimensionMoveSelection(
            context.document,
            context.state,
            hoveredDimension->id);
    }
}

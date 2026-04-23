#include "editor/ops/SelectionOperations.h"

#include "editor/interaction/SelectionInteractor.h"

namespace
{
constexpr float kMinimumDragSize = 3.0f;
}

namespace SelectionOperations
{
void ApplyNodeSelection(EditorState& state, int nodeId, SelectionMode selectionMode)
{
    if (selectionMode == SelectionMode::Add)
    {
        state.AddNodeToSelection(nodeId);
        return;
    }

    if (selectionMode == SelectionMode::Remove)
    {
        state.selectionState.selection.RemoveNode(nodeId);
        return;
    }

    state.SelectSingleNode(nodeId);
}

void ApplyBeamSelection(EditorState& state, int beamId, SelectionMode selectionMode)
{
    if (selectionMode == SelectionMode::Add)
    {
        state.selectionState.selection.AddBeam(beamId);
        return;
    }

    if (selectionMode == SelectionMode::Remove)
    {
        state.selectionState.selection.RemoveBeam(beamId);
        return;
    }

    state.selectionState.selection.SelectSingleBeam(beamId);
}

void ApplyDimensionSelection(EditorState& state, int dimensionId, SelectionMode selectionMode)
{
    if (selectionMode == SelectionMode::Add)
    {
        state.selectionState.selection.AddDimension(dimensionId);
        return;
    }

    if (selectionMode == SelectionMode::Remove)
    {
        state.selectionState.selection.RemoveDimension(dimensionId);
        return;
    }

    state.selectionState.selection.SelectSingleDimension(dimensionId);
}

void CompleteSelectionBox(
    const ProjectDocument& document,
    EditorState& state,
    const Camera2D& camera,
    Vector2 mouseWorldPosition,
    SelectionMode selectionMode)
{
    state.selectionState.boxSelectEndWorld = mouseWorldPosition;

    const Rectangle selectionRect = SelectionInteractor::GetSelectionRectangle(camera, state.selectionState);
    const bool crossingSelection = SelectionInteractor::IsRightToLeftBoxSelection(camera, state.selectionState);

    state.selectionState.isBoxSelecting = false;

    if (selectionRect.width < kMinimumDragSize || selectionRect.height < kMinimumDragSize)
    {
        if (selectionMode == SelectionMode::Replace)
        {
            state.selectionState.selection.Clear();
        }
        return;
    }

    if (selectionMode == SelectionMode::Replace)
    {
        state.selectionState.selection.Clear();
    }

    for (const Node& node : document.nodes)
    {
        const Vector2 nodeScreenPosition = GetWorldToScreen2D(
            Vector2{
                static_cast<float>(node.position.x),
                static_cast<float>(node.position.y)},
            camera);

        if (!CheckCollisionPointRec(nodeScreenPosition, selectionRect))
        {
            continue;
        }

        if (selectionMode == SelectionMode::Remove)
        {
            state.selectionState.selection.RemoveNode(node.id);
        }
        else
        {
            state.AddNodeToSelection(node.id);
        }
    }

    for (const Beam& beam : document.beams)
    {
        const bool shouldSelectBeam =
            crossingSelection
                ? SelectionInteractor::DoesBeamCrossSelectionRectangle(document, camera, beam, selectionRect)
                : SelectionInteractor::IsBeamInsideSelectionRectangle(document, camera, beam, selectionRect);

        if (!shouldSelectBeam)
        {
            continue;
        }

        if (selectionMode == SelectionMode::Remove)
        {
            state.selectionState.selection.RemoveBeam(beam.id);
        }
        else
        {
            state.selectionState.selection.AddBeam(beam.id);
        }
    }

    for (const Dimension& dimension : document.dimensions)
    {
        bool shouldSelectDimension = false;
        if (crossingSelection)
        {
            shouldSelectDimension = SelectionInteractor::DoesDimensionCrossSelectionRectangle(
                document,
                camera,
                dimension,
                selectionRect);
        }
        else
        {
            const Node* startNode = document.FindNodeById(dimension.startNodeId);
            const Node* endNode = document.FindNodeById(dimension.endNodeId);
            if (startNode == nullptr || endNode == nullptr)
            {
                continue;
            }

            const Vector2 startScreen = GetWorldToScreen2D(
                Vector2{
                    static_cast<float>(startNode->position.x),
                    static_cast<float>(startNode->position.y)},
                camera);
            const Vector2 endScreen = GetWorldToScreen2D(
                Vector2{
                    static_cast<float>(endNode->position.x),
                    static_cast<float>(endNode->position.y)},
                camera);
            shouldSelectDimension =
                CheckCollisionPointRec(startScreen, selectionRect) &&
                CheckCollisionPointRec(endScreen, selectionRect);
        }

        if (!shouldSelectDimension)
        {
            continue;
        }

        if (selectionMode == SelectionMode::Remove)
        {
            state.selectionState.selection.RemoveDimension(dimension.id);
        }
        else
        {
            state.selectionState.selection.AddDimension(dimension.id);
        }
    }
}

bool FinalizeNodeBoxSelection(
    const ProjectDocument& document,
    EditorState& state,
    const Camera2D& camera,
    Vector2 mouseWorldPosition)
{
    state.selectionState.boxSelectEndWorld = mouseWorldPosition;

    const Rectangle selectionRect = SelectionInteractor::GetSelectionRectangle(camera, state.selectionState);
    state.selectionState.isBoxSelecting = false;
    state.selectionState.selection.ClearNodes();

    if (selectionRect.width < kMinimumDragSize || selectionRect.height < kMinimumDragSize)
    {
        return false;
    }

    for (const Node& node : document.nodes)
    {
        const Vector2 nodeScreenPosition = GetWorldToScreen2D(
            Vector2{
                static_cast<float>(node.position.x),
                static_cast<float>(node.position.y)},
            camera);

        if (CheckCollisionPointRec(nodeScreenPosition, selectionRect))
        {
            state.AddNodeToSelection(node.id);
        }
    }

    return true;
}

bool FinalizeBeamBoxSelection(
    const ProjectDocument& document,
    EditorState& state,
    const Camera2D& camera,
    Vector2 mouseWorldPosition)
{
    state.selectionState.boxSelectEndWorld = mouseWorldPosition;

    const Rectangle selectionRect = SelectionInteractor::GetSelectionRectangle(camera, state.selectionState);
    const bool crossingSelection = SelectionInteractor::IsRightToLeftBoxSelection(camera, state.selectionState);

    state.selectionState.isBoxSelecting = false;
    state.selectionState.selection.ClearBeams();

    if (selectionRect.width < kMinimumDragSize || selectionRect.height < kMinimumDragSize)
    {
        return false;
    }

    for (const Beam& beam : document.beams)
    {
        const bool shouldSelectBeam =
            crossingSelection
                ? SelectionInteractor::DoesBeamCrossSelectionRectangle(document, camera, beam, selectionRect)
                : SelectionInteractor::IsBeamInsideSelectionRectangle(document, camera, beam, selectionRect);

        if (shouldSelectBeam)
        {
            state.selectionState.selection.AddBeam(beam.id);
        }
    }

    return true;
}
}

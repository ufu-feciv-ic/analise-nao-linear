#include "editor/interaction/SnapResolver.h"

#include <cmath>

namespace
{
Point2D ToPoint2D(Vector2 position)
{
    return Point2D{
        static_cast<double>(position.x),
        static_cast<double>(position.y)};
}
}

namespace SnapResolver
{
Vector2 RoundPositionToGrid(Vector2 position, float spacing)
{
    position.x = roundf(position.x / spacing) * spacing;
    position.y = roundf(position.y / spacing) * spacing;
    return position;
}

Vector2 GetCurrentPlacementWorldPosition(
    const EditorState& state,
    Vector2 mouseWorldPosition,
    Vector2 snappedWorldPosition)
{
    return (state.view.showGrid && state.view.snapToGrid) ? snappedWorldPosition : mouseWorldPosition;
}

Point2D GetCurrentTransformTargetWorldPosition(
    const EditorState& state,
    Vector2 mouseWorldPosition,
    Vector2 snappedWorldPosition,
    Node* hoveredNode,
    bool allowSelectedNodeSnap)
{
    if (!state.view.snapToGrid)
    {
        Vector2 targetPosition = mouseWorldPosition;

        if (state.view.showGuides)
        {
            if (state.beamTool.activeBeamGuideXIndex >= 0)
            {
                targetPosition.x =
                    state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideXIndex)].position.x;
            }

            if (state.beamTool.activeBeamGuideYIndex >= 0)
            {
                targetPosition.y =
                    state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideYIndex)].position.y;
            }
        }

        return ToPoint2D(targetPosition);
    }

    if (hoveredNode != nullptr &&
        (allowSelectedNodeSnap || !state.selectionState.selection.IsNodeSelected(hoveredNode->id)))
    {
        return hoveredNode->position;
    }

    if (state.view.showGuides &&
        (state.beamTool.activeBeamGuideXIndex >= 0 || state.beamTool.activeBeamGuideYIndex >= 0))
    {
        Vector2 targetPosition = mouseWorldPosition;

        if (state.beamTool.activeBeamGuideXIndex >= 0)
        {
            targetPosition.x =
                state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideXIndex)].position.x;
        }

        if (state.beamTool.activeBeamGuideYIndex >= 0)
        {
            targetPosition.y =
                state.beamTool.beamGuides[static_cast<std::size_t>(state.beamTool.activeBeamGuideYIndex)].position.y;
        }

        return ToPoint2D(targetPosition);
    }

    return ToPoint2D((state.view.showGrid && state.view.snapToGrid) ? snappedWorldPosition : mouseWorldPosition);
}
}


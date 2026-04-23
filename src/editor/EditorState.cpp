#include "editor/EditorState.h"

void EditorState::ResetSelection()
{
    hover.ClearEntity();
    selectionState.selection.Clear();
}

void EditorState::CancelCurrentTool()
{
    activeTool = EditorTool::Select;

    beamTool.isCreatingBeam = false;
    beamTool.beamStartNodeId = -1;
    beamTool.beamPreviewPointWorld = Vector2{0.0f, 0.0f};
    beamTool.activeBeamGuideXIndex = -1;
    beamTool.activeBeamGuideYIndex = -1;
    beamTool.isBeamDistanceInputOpen = false;
    beamTool.beamDistanceMode = BeamToolState::DistanceInputMode::GuideAxis;
    beamTool.beamDistanceLocksX = false;
    beamTool.beamDistanceScreenPosition = Vector2{0.0f, 0.0f};
    beamTool.beamDistanceClickWorld = Point2D{0.0, 0.0};
    beamTool.beamDistanceGuideWorld = Point2D{0.0, 0.0};
    beamTool.beamDistanceSegmentStartWorld = Point2D{0.0, 0.0};
    beamTool.beamDistanceSegmentEndWorld = Point2D{0.0, 0.0};
    beamTool.beamDistanceCreatedNodeWorld = Point2D{0.0, 0.0};
    dimensionTool.dimensionCreationStep = DimensionToolState::DimensionCreationStep::None;
    dimensionTool.dimensionStartNodeId = -1;
    dimensionTool.dimensionEndNodeId = -1;
    dimensionTool.dimensionChainReferenceNodeId = -1;
    dimensionTool.dimensionLengthUnit = dimensionTool.newDimensionLengthUnit;
    dimensionTool.dimensionType = DimensionType::Aligned;
    dimensionTool.dimensionOffsetMode = DimensionOffsetMode::WorldUnits;
    dimensionTool.dimensionOffsetPixels = 32.0;
    dimensionTool.dimensionOffsetWorld = 0.32;
    dimensionTool.dimensionPreviewPointWorld = Vector2{0.0f, 0.0f};
    dimensionTool.dimensionPreviewScreenPosition = Vector2{0.0f, 0.0f};
    dimensionTool.dimensionMoveStep = DimensionToolState::DimensionMoveStep::None;
    dimensionTool.movingDimensionId = -1;
    dimensionTool.movingDimensionIds.clear();
    selectionState.isBoxSelecting = false;
    ResetMoveSelection();

    hover.ClearEntity();
}

bool EditorState::IsNodeSelected(int id) const
{
    return selectionState.selection.IsNodeSelected(id);
}

bool EditorState::IsBeamSelected(int id) const
{
    return selectionState.selection.IsBeamSelected(id);
}

bool EditorState::IsDimensionSelected(int id) const
{
    return selectionState.selection.IsDimensionSelected(id);
}

bool EditorState::HasSelection() const
{
    return selectionState.selection.HasAny();
}

bool EditorState::HasActiveToolOrOperation() const
{
    return activeTool != EditorTool::Select ||
           beamTool.isCreatingBeam ||
           beamTool.isBeamDistanceInputOpen ||
           selectionState.isBoxSelecting;
}

void EditorState::ClearSelection()
{
    selectionState.selection.Clear();
}

void EditorState::SelectSingleNode(int id)
{
    selectionState.selection.SelectSingleNode(id);
}

void EditorState::ResetMoveSelection()
{
    transformTool.moveSelectionStep = TransformToolState::MoveSelectionStep::None;
    transformTool.moveBasePointWorld = Point2D{0.0, 0.0};
    transformTool.movePreviewPointWorld = Point2D{0.0, 0.0};
    transformTool.moveSnapNodeId = -1;
    transformTool.gridShortcutPending = false;
    transformTool.lastGridShortcutTime = -100.0;
}

bool EditorState::HasSelectedNodes() const
{
    return selectionState.selection.HasNodes();
}

void EditorState::AddNodeToSelection(int id)
{
    selectionState.selection.AddNode(id);
}

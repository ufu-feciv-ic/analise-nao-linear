#pragma once

#include <cstdint>
#include <vector>

#include "editor/EditorTool.h"
#include "editor/state/BeamToolState.h"
#include "editor/state/DimensionToolState.h"
#include "editor/state/HoverState.h"
#include "editor/state/LoadToolState.h"
#include "editor/state/SelectionState.h"
#include "editor/state/TransformToolState.h"
#include "editor/state/ViewOptions.h"

#include <raylib.h>

class EditorState
{
public:
    EditorTool activeTool = EditorTool::Select;
    EditorTool lastSelectedTool = EditorTool::Select;

    ViewOptions view;
    HoverState hover;
    SelectionState selectionState;
    BeamToolState beamTool;
    DimensionToolState dimensionTool;
    TransformToolState transformTool;
    LoadToolState loadTool;

    int currentMaterialId = -1;
    int currentSectionId = -1;

public:
    void ResetSelection();

    void CancelCurrentTool();
    bool HasSelection() const;
    bool HasActiveToolOrOperation() const;

    bool IsNodeSelected(int nodeId) const;
    bool IsBeamSelected(int beamId) const;
    bool IsDimensionSelected(int dimensionId) const;

    void ClearSelection();

    void SelectSingleNode(int id);
    void ResetMoveSelection();
    bool HasSelectedNodes() const;

    void AddNodeToSelection(int id);
};

#pragma once

#include "model/Node.h"

struct TransformToolState
{
    enum class MoveSelectionStep
    {
        None,
        AwaitSelectionConfirm,
        AwaitBasePoint,
        AwaitTargetPoint
    };

    MoveSelectionStep moveSelectionStep = MoveSelectionStep::None;
    Point2D moveBasePointWorld{0.0, 0.0};
    Point2D movePreviewPointWorld{0.0, 0.0};
    int moveSnapNodeId = -1;
    bool mirrorKeepsOriginal = false;
    bool gridShortcutPending = false;
    double lastGridShortcutTime = -100.0;
};

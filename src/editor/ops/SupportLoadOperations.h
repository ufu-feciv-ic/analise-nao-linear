#pragma once

#include "editor/EditorState.h"
#include "model/ProjectDocument.h"

namespace SupportLoadOperations
{
    SupportType GetSupportTypeForTool(EditorTool tool);

    bool ApplySupportTypeToNode(
        ProjectDocument& document,
        int nodeId,
        SupportType supportType);

    bool ApplySupportTypeToSelection(
        ProjectDocument& document,
        const EditorState& state,
        SupportType supportType);

    bool ApplyPointLoadToNode(
        ProjectDocument& document,
        int nodeId,
        const NodalLoad& load);

    bool ApplyPointLoadToSelection(
        ProjectDocument& document,
        const EditorState& state,
        const NodalLoad& load);

    bool ApplyDistributedLoadToBeam(
        ProjectDocument& document,
        int beamId,
        const DistributedLoadValue& load);

    bool ApplyDistributedLoadToSelection(
        ProjectDocument& document,
        const EditorState& state,
        const DistributedLoadValue& load);

    void AssignDistributedLoadAlongBeamPath(
        ProjectDocument& document,
        int originalStartNodeId,
        int originalEndNodeId,
        const DistributedLoadValue& loadValue);
}

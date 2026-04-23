#pragma once

#include "editor/FrameRequests.h"
#include "editor/EditorState.h"
#include "model/ProjectDocument.h"
#include "ui/DialogResult.h"

namespace DocumentEditOperations
{
    bool ApplyMaterialDialogResult(
        ProjectDocument& document,
        EditorState& state,
        const MaterialDialogResult& result);

    bool ApplySectionDialogResult(
        ProjectDocument& document,
        EditorState& state,
        const SectionDialogResult& result);

    bool ApplyDocumentRequest(
        ProjectDocument& document,
        EditorState& state,
        DocumentRequest request);

    bool ApplyNodalLoadEditRequest(
        ProjectDocument& document,
        const FrameRequests::NodalLoadSelectionEditRequest& request);

    bool ApplyDistributedLoadEditRequest(
        ProjectDocument& document,
        const FrameRequests::DistributedLoadSelectionEditRequest& request);

    bool ApplyBeamPropertyEditRequest(
        ProjectDocument& document,
        EditorState& state,
        const FrameRequests::BeamPropertySelectionEditRequest& request);

    bool ApplyDimensionEditRequest(
        ProjectDocument& document,
        EditorState& state,
        const FrameRequests::DimensionSelectionEditRequest& request);

    void ApplyDisplayUnitsUpdateRequest(
        ProjectDocument& document,
        const FrameRequests::DisplayUnitsUpdateRequest& request);
}

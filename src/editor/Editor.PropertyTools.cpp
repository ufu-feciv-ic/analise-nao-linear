#include "editor/Editor.h"
#include "editor/ops/SupportLoadOperations.h"

#include <utility>

namespace
{
    NodalLoad MakeZeroNodalLoad()
    {
        return NodalLoad{0.0, 0.0, 0.0};
    }

    DistributedLoadValue MakeZeroDistributedLoad()
    {
        return DistributedLoadValue{};
    }

}

void Editor::ApplySupportTypeToSelection(SupportType supportType)
{
    ProjectDocument beforeDocument = document;
    if (SupportLoadOperations::ApplySupportTypeToSelection(document, state, supportType))
    {
        RecordDocumentChange(std::move(beforeDocument), true);
    }
}

void Editor::ApplySupportTypeInSelectionRectangle(SupportType supportType)
{
    if (!FinalizeNodeBoxSelection())
    {
        return;
    }

    ApplySupportTypeToSelection(supportType);
}

void Editor::ApplyPointLoadToSelection(const NodalLoad& load)
{
    ProjectDocument beforeDocument = document;
    if (SupportLoadOperations::ApplyPointLoadToSelection(document, state, load))
    {
        RecordDocumentChange(std::move(beforeDocument), true);
    }
}

void Editor::ApplyPointLoadInSelectionRectangle(const NodalLoad& load)
{
    if (!FinalizeNodeBoxSelection())
    {
        return;
    }

    ApplyPointLoadToSelection(load);
}

void Editor::ClearPointLoadInSelectionRectangle()
{
    ApplyPointLoadInSelectionRectangle(MakeZeroNodalLoad());
}

void Editor::ApplyDistributedLoadToSelection(const DistributedLoadValue& load)
{
    ProjectDocument beforeDocument = document;
    if (SupportLoadOperations::ApplyDistributedLoadToSelection(document, state, load))
    {
        RecordDocumentChange(std::move(beforeDocument), true);
    }
}

void Editor::ApplyDistributedLoadInSelectionRectangle(const DistributedLoadValue& load)
{
    if (!FinalizeBeamBoxSelection())
    {
        return;
    }

    ApplyDistributedLoadToSelection(load);
}

void Editor::ClearDistributedLoadInSelectionRectangle()
{
    ApplyDistributedLoadInSelectionRectangle(MakeZeroDistributedLoad());
}

void Editor::AssignDistributedLoadAlongBeamPath(
    int originalStartNodeId,
    int originalEndNodeId,
    const DistributedLoadValue& loadValue)
{
    SupportLoadOperations::AssignDistributedLoadAlongBeamPath(
        document,
        originalStartNodeId,
        originalEndNodeId,
        loadValue);
}


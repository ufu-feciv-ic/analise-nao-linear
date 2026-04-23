#include "editor/Editor.h"
#include "editor/ops/DeleteOperations.h"

#include <utility>

void Editor::DeleteSelection()
{
    ProjectDocument beforeDocument = document;
    if (DeleteOperations::DeleteSelection(document, state))
    {
        RecordDocumentChange(std::move(beforeDocument), true);
    }
}

void Editor::DeleteSelectedNodes()
{
    ProjectDocument beforeDocument = document;
    if (DeleteOperations::DeleteSelectedNodes(document, state))
    {
        RecordDocumentChange(std::move(beforeDocument), true);
    }
}

void Editor::DeleteSelectedBeams()
{
    ProjectDocument beforeDocument = document;
    if (DeleteOperations::DeleteSelectedBeams(document, state))
    {
        RecordDocumentChange(std::move(beforeDocument), true);
    }
}

void Editor::DeleteSelectedDimensions()
{
    ProjectDocument beforeDocument = document;
    if (DeleteOperations::DeleteSelectedDimensions(document, state))
    {
        RecordDocumentChange(std::move(beforeDocument), true);
    }
}

bool Editor::DeleteNodeById(int nodeId)
{
    ProjectDocument beforeDocument = document;
    if (!DeleteOperations::DeleteNodeById(document, state, nodeId))
    {
        return false;
    }

    RecordDocumentChange(std::move(beforeDocument), true);
    return true;
}

bool Editor::DeleteBeamById(int beamId)
{
    ProjectDocument beforeDocument = document;
    if (!DeleteOperations::DeleteBeamById(document, state, beamId))
    {
        return false;
    }
    RecordDocumentChange(std::move(beforeDocument), true);
    return true;
}

bool Editor::DeleteDimensionById(int dimensionId)
{
    ProjectDocument beforeDocument = document;
    if (!DeleteOperations::DeleteDimensionById(document, state, dimensionId))
    {
        return false;
    }

    RecordDocumentChange(std::move(beforeDocument), true);
    return true;
}

void Editor::PurgeInvalidSelection()
{
    DeleteOperations::PurgeInvalidSelection(document, state);
}

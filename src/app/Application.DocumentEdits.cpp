#include "app/Application.h"

#include "editor/ops/DocumentEditOperations.h"

#include <utility>

void Application::ApplyFrameEdits(
    const FrameRequests& requests,
    const AppDialogResults& dialogResults)
{
    ApplyDialogResults(dialogResults);
    ApplyDocumentRequest(requests.document);
    ApplyNodalLoadEditRequest(requests.nodalLoadEdit);
    ApplyDistributedLoadEditRequest(requests.distributedLoadEdit);
    ApplyDisplayUnitsUpdateRequest(requests.displayUnitsUpdate);
    ApplyPropertyPanelStateSyncRequest(requests.propertyPanelStateSync);
    ApplyBeamPropertyEditRequest(requests.beamPropertyEdit);
    ApplyDimensionEditRequest(requests.dimensionEdit);
    ApplyDimensionToolStateSyncRequest(requests.dimensionToolStateSync);
    ApplyLoadToolStateSyncRequest(requests.loadToolStateSync);
    ApplyAnalysisRequest(requests.analysis);
    ApplyResultsViewRequest(requests.resultsView);
}

void Application::ApplyDialogResults(const AppDialogResults& results)
{
    ProjectDocument beforeDocument = editor.document;
    bool changed = false;

    changed = DocumentEditOperations::ApplyMaterialDialogResult(
                  editor.document,
                  editor.state,
                  results.material) ||
              changed;

    changed = DocumentEditOperations::ApplySectionDialogResult(
                  editor.document,
                  editor.state,
                  results.section) ||
              changed;

    if (changed)
    {
        editor.RecordExternalDocumentChange(std::move(beforeDocument), true);
    }
}

void Application::ApplyDocumentRequest(DocumentRequest request)
{
    if (request == DocumentRequest::None)
    {
        return;
    }

    ProjectDocument beforeDocument = editor.document;
    if (DocumentEditOperations::ApplyDocumentRequest(editor.document, editor.state, request))
    {
        editor.RecordExternalDocumentChange(std::move(beforeDocument), true);
    }
}

void Application::ApplyDistributedLoadEditRequest(
    const FrameRequests::DistributedLoadSelectionEditRequest& request)
{
    if (!request.active || request.beamIds.empty())
    {
        return;
    }

    ProjectDocument beforeDocument = editor.document;
    if (DocumentEditOperations::ApplyDistributedLoadEditRequest(editor.document, request))
    {
        editor.RecordExternalDocumentChange(std::move(beforeDocument), true);
    }
}

void Application::ApplyNodalLoadEditRequest(const FrameRequests::NodalLoadSelectionEditRequest& request)
{
    if (!request.active || request.nodeIds.empty())
    {
        return;
    }

    ProjectDocument beforeDocument = editor.document;
    if (DocumentEditOperations::ApplyNodalLoadEditRequest(editor.document, request))
    {
        editor.RecordExternalDocumentChange(std::move(beforeDocument), true);
    }
}

void Application::ApplyDisplayUnitsUpdateRequest(const FrameRequests::DisplayUnitsUpdateRequest& request)
{
    DocumentEditOperations::ApplyDisplayUnitsUpdateRequest(editor.document, request);
}

void Application::ApplyBeamPropertyEditRequest(const FrameRequests::BeamPropertySelectionEditRequest& request)
{
    if (!request.active)
    {
        return;
    }

    ProjectDocument beforeDocument = editor.document;
    if (DocumentEditOperations::ApplyBeamPropertyEditRequest(editor.document, editor.state, request))
    {
        editor.RecordExternalDocumentChange(std::move(beforeDocument), true);
    }
}

void Application::ApplyPropertyPanelStateSyncRequest(const FrameRequests::PropertyPanelStateSyncRequest& request)
{
    if (!request.active)
    {
        return;
    }

    if (request.setCurrentMaterialId)
    {
        editor.state.currentMaterialId = request.currentMaterialId;
    }

    if (request.setCurrentSectionId)
    {
        editor.state.currentSectionId = request.currentSectionId;
    }
}

void Application::ApplyDimensionEditRequest(const FrameRequests::DimensionSelectionEditRequest& request)
{
    if (!request.active)
    {
        return;
    }

    ProjectDocument beforeDocument = editor.document;
    if (DocumentEditOperations::ApplyDimensionEditRequest(editor.document, editor.state, request))
    {
        editor.RecordExternalDocumentChange(std::move(beforeDocument), true);
    }
}

void Application::ApplyDimensionToolStateSyncRequest(const FrameRequests::DimensionToolStateSyncRequest& request)
{
    if (!request.active)
    {
        return;
    }

    if (request.setLengthUnit)
    {
        editor.state.dimensionTool.newDimensionLengthUnit = request.lengthUnit;
        editor.state.dimensionTool.dimensionLengthUnit = request.lengthUnit;
    }

    if (request.setOffsetMode)
    {
        editor.state.dimensionTool.newDimensionOffsetMode = request.offsetMode;
        editor.state.dimensionTool.dimensionOffsetMode = request.offsetMode;
    }
}

void Application::ApplyLoadToolStateSyncRequest(const FrameRequests::LoadToolStateSyncRequest& request)
{
    if (!request.active)
    {
        return;
    }

    if (request.setPendingNodalLoad)
    {
        editor.state.loadTool.pendingNodalLoad = request.pendingNodalLoad;
    }

    if (request.setPendingDistributedLoad)
    {
        editor.state.loadTool.pendingDistributedLoad = request.pendingDistributedLoad;
    }

    if (request.setPendingDistributedLoadVariable)
    {
        editor.state.loadTool.pendingDistributedLoadVariable = request.pendingDistributedLoadVariable;
    }

    if (request.setDistributedLoadPanelSelectionState)
    {
        editor.state.loadTool.distributedLoadPanelSelectionToken = request.distributedLoadPanelSelectionToken;
        editor.state.loadTool.distributedLoadPanelSelectionTokenInitialized =
            request.distributedLoadPanelSelectionTokenInitialized;
    }
}

void Application::ApplyAnalysisRequest(const FrameRequests::AnalysisRequest& request)
{
    if (!request.active)
    {
        return;
    }

    TraceLog(LOG_INFO, "Solicitação de Análise: %s", 
        request.type == AnalysisType::Linear ? "Linear" : "Não-Linear");
}

void Application::ApplyResultsViewRequest(const FrameRequests::ResultsViewRequest& request)
{
    if (!request.active)
    {
        return;
    }

    editor.state.view.resultsViewType = request.type;
    editor.state.view.resultsScale = request.scale;
}

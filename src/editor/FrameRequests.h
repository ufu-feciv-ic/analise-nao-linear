#pragma once

#include <cstdint>
#include <vector>

#include "model/DisplayUnits.h"
#include "model/Dimension.h"
#include "model/DistributedLoad.h"
#include "model/Node.h"
#include "ui/ToolbarDialogRequest.h"

enum class EditorRequest
{
    None,

    ActivateAddNodeTool,
    ActivateMoveNodeTool,
    ActivateCopySelectionTool,
    ActivateMirrorSelectionTool,
    InvokeNodeRemoveAction,

    ActivateAddBeamTool,
    InvokeBeamRemoveAction,
    ActivateAddDimensionTool,
    ActivateMoveDimensionTool,

    SetSupportNone,
    SetSupportX,
    SetSupportY,
    SetSupportXY,
    SetSupportFixed,

    ActivateAddPointLoadTool,
    ActivateRemovePointLoadTool,
    ActivateAddDistributedLoadTool,
    ActivateRemoveDistributedLoadTool,

    DeleteSelection
};

enum class DocumentRequest
{
    None,
    RemoveCurrentMaterial,
    RemoveCurrentSection
};

struct FrameRequests
{
    struct NodalLoadSelectionEditRequest
    {
        bool active = false;
        std::vector<int> nodeIds;
        bool setFx = false;
        bool setFy = false;
        bool setMz = false;
        double fx = 0.0;
        double fy = 0.0;
        double mz = 0.0;
    };

    struct DistributedLoadSelectionEditRequest
    {
        bool active = false;
        std::vector<int> beamIds;
        bool setQxStart = false;
        bool setQyStart = false;
        bool setQxEnd = false;
        bool setQyEnd = false;
        double qxStart = 0.0;
        double qyStart = 0.0;
        double qxEnd = 0.0;
        double qyEnd = 0.0;
    };

    struct DisplayUnitsUpdateRequest
    {
        bool active = false;
        DisplayUnits units;
    };

    struct BeamPropertySelectionEditRequest
    {
        bool active = false;
        std::vector<int> beamIds;
        bool setMaterial = false;
        bool setSection = false;
        int materialId = -1;
        int sectionId = -1;
    };

    struct PropertyPanelStateSyncRequest
    {
        bool active = false;
        bool setCurrentMaterialId = false;
        bool setCurrentSectionId = false;
        int currentMaterialId = -1;
        int currentSectionId = -1;
    };

    struct DimensionSelectionEditRequest
    {
        bool active = false;
        std::vector<int> dimensionIds;
        bool setLengthUnit = false;
        bool setOffsetMode = false;
        LengthUnit lengthUnit = LengthUnit::Meter;
        DimensionOffsetMode offsetMode = DimensionOffsetMode::WorldUnits;
        double cameraZoom = 1.0;
    };

    struct DimensionToolStateSyncRequest
    {
        bool active = false;
        bool setLengthUnit = false;
        bool setOffsetMode = false;
        LengthUnit lengthUnit = LengthUnit::Meter;
        DimensionOffsetMode offsetMode = DimensionOffsetMode::WorldUnits;
    };

    struct LoadToolStateSyncRequest
    {
        bool active = false;
        bool setPendingNodalLoad = false;
        NodalLoad pendingNodalLoad{};
        bool setPendingDistributedLoad = false;
        DistributedLoadValue pendingDistributedLoad{};
        bool setPendingDistributedLoadVariable = false;
        bool pendingDistributedLoadVariable = false;
        bool setDistributedLoadPanelSelectionState = false;
        std::uint64_t distributedLoadPanelSelectionToken = 0;
        bool distributedLoadPanelSelectionTokenInitialized = false;
    };

    EditorRequest editor = EditorRequest::None;
    DocumentRequest document = DocumentRequest::None;
    ToolbarDialogRequest dialog = ToolbarDialogRequest::None;
    NodalLoadSelectionEditRequest nodalLoadEdit;
    DistributedLoadSelectionEditRequest distributedLoadEdit;
    DisplayUnitsUpdateRequest displayUnitsUpdate;
    BeamPropertySelectionEditRequest beamPropertyEdit;
    PropertyPanelStateSyncRequest propertyPanelStateSync;
    DimensionSelectionEditRequest dimensionEdit;
    DimensionToolStateSyncRequest dimensionToolStateSync;
    LoadToolStateSyncRequest loadToolStateSync;
};

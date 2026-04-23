#include "editor/ops/DocumentEditOperations.h"

#include <algorithm>
#include <cmath>

#include "editor/ops/PropertySelectionOperations.h"

namespace DocumentEditOperations
{
    bool ApplyMaterialDialogResult(
        ProjectDocument& document,
        EditorState& state,
        const MaterialDialogResult& result)
    {
        if (result.action == MaterialDialogResult::Action::None)
        {
            return false;
        }

        if (result.action == MaterialDialogResult::Action::Create)
        {
            const int newId = document.AddMaterial(
                result.name,
                result.youngModulus,
                result.thermalExpansion);
            if (!state.selectionState.selection.HasBeams())
            {
                state.currentMaterialId = newId;
            }
            return true;
        }

        if (result.action == MaterialDialogResult::Action::Update)
        {
            StructuralMaterial* material = document.FindMaterialById(result.targetMaterialId);
            if (material == nullptr)
            {
                return false;
            }

            material->name = result.name;
            material->youngModulus = result.youngModulus;
            material->thermalExpansion = result.thermalExpansion;
            state.currentMaterialId = material->id;
            return true;
        }

        return false;
    }

    bool ApplySectionDialogResult(
        ProjectDocument& document,
        EditorState& state,
        const SectionDialogResult& result)
    {
        if (result.action == SectionDialogResult::Action::None)
        {
            return false;
        }

        if (result.action == SectionDialogResult::Action::Create)
        {
            const int newId = document.AddSection(
                result.name,
                result.area,
                result.inertia);
            if (!state.selectionState.selection.HasBeams())
            {
                state.currentSectionId = newId;
            }
            return true;
        }

        if (result.action == SectionDialogResult::Action::Update)
        {
            Section* section = document.FindSectionById(result.targetSectionId);
            if (section == nullptr)
            {
                return false;
            }

            section->name = result.name;
            section->area = result.area;
            section->inertia = result.inertia;
            state.currentSectionId = section->id;
            return true;
        }

        return false;
    }

    bool ApplyDocumentRequest(
        ProjectDocument& document,
        EditorState& state,
        DocumentRequest request)
    {
        switch (request)
        {
        case DocumentRequest::RemoveCurrentMaterial:
            if (!document.RemoveMaterial(state.currentMaterialId))
            {
                return false;
            }

            state.currentMaterialId = PropertySelectionOperations::ResolveCurrentMaterialId(
                document,
                state.currentMaterialId);
            return true;

        case DocumentRequest::RemoveCurrentSection:
            if (!document.RemoveSection(state.currentSectionId))
            {
                return false;
            }

            state.currentSectionId = PropertySelectionOperations::ResolveCurrentSectionId(
                document,
                state.currentSectionId);
            return true;

        case DocumentRequest::None:
        default:
            return false;
        }
    }

    bool ApplyNodalLoadEditRequest(
        ProjectDocument& document,
        const FrameRequests::NodalLoadSelectionEditRequest& request)
    {
        if (!request.active || request.nodeIds.empty())
        {
            return false;
        }

        bool changed = false;
        for (int nodeId : request.nodeIds)
        {
            Node* node = document.FindNodeById(nodeId);
            if (node == nullptr)
            {
                continue;
            }

            if (request.setFx && node->load.fx != request.fx)
            {
                node->load.fx = request.fx;
                changed = true;
            }
            if (request.setFy && node->load.fy != request.fy)
            {
                node->load.fy = request.fy;
                changed = true;
            }
            if (request.setMz && node->load.mz != request.mz)
            {
                node->load.mz = request.mz;
                changed = true;
            }
        }

        return changed;
    }

    bool ApplyDistributedLoadEditRequest(
        ProjectDocument& document,
        const FrameRequests::DistributedLoadSelectionEditRequest& request)
    {
        if (!request.active || request.beamIds.empty())
        {
            return false;
        }

        auto applyComponentUpdates = [&](DistributedLoadValue& load)
        {
            if (request.setQxStart)
            {
                load.qxStart = request.qxStart;
            }
            if (request.setQyStart)
            {
                load.qyStart = request.qyStart;
            }
            if (request.setQxEnd)
            {
                load.qxEnd = request.qxEnd;
            }
            if (request.setQyEnd)
            {
                load.qyEnd = request.qyEnd;
            }
        };

        bool changed = false;
        bool collectionChanged = false;
        for (int beamId : request.beamIds)
        {
            if (document.FindBeamById(beamId) == nullptr)
            {
                continue;
            }

            BeamDistributedLoad* distributedLoad = document.FindDistributedLoadByBeamId(beamId);
            DistributedLoadValue updatedLoad = distributedLoad != nullptr ? distributedLoad->value : DistributedLoadValue{};
            applyComponentUpdates(updatedLoad);

            if (updatedLoad.IsZero())
            {
                if (distributedLoad != nullptr)
                {
                    distributedLoad->beamId = -1;
                    changed = true;
                    collectionChanged = true;
                }
                continue;
            }

            if (distributedLoad != nullptr)
            {
                if (distributedLoad->value.qxStart != updatedLoad.qxStart ||
                    distributedLoad->value.qyStart != updatedLoad.qyStart ||
                    distributedLoad->value.qxEnd != updatedLoad.qxEnd ||
                    distributedLoad->value.qyEnd != updatedLoad.qyEnd)
                {
                    distributedLoad->value = updatedLoad;
                    changed = true;
                }
                continue;
            }

            document.AppendDistributedLoad(BeamDistributedLoad(
                document.nextDistributedLoadId++,
                beamId,
                updatedLoad));
            changed = true;
            collectionChanged = true;
        }

        if (collectionChanged)
        {
            document.distributedLoads.erase(
                std::remove_if(
                    document.distributedLoads.begin(),
                    document.distributedLoads.end(),
                    [](const BeamDistributedLoad& distributedLoad)
                    {
                        return distributedLoad.beamId < 0;
                    }),
                document.distributedLoads.end());
            document.InvalidateLookupIndices();
        }

        return changed;
    }

    bool ApplyBeamPropertyEditRequest(
        ProjectDocument& document,
        EditorState& state,
        const FrameRequests::BeamPropertySelectionEditRequest& request)
    {
        if (!request.active)
        {
            return false;
        }

        if (request.setMaterial)
        {
            state.currentMaterialId = request.materialId;
        }
        if (request.setSection)
        {
            state.currentSectionId = request.sectionId;
        }

        if (request.beamIds.empty())
        {
            return false;
        }

        bool changed = false;
        for (int beamId : request.beamIds)
        {
            Beam* beam = document.FindBeamById(beamId);
            if (beam == nullptr)
            {
                continue;
            }

            if (request.setMaterial && beam->materialId != request.materialId)
            {
                beam->materialId = request.materialId;
                changed = true;
            }

            if (request.setSection && beam->sectionId != request.sectionId)
            {
                beam->sectionId = request.sectionId;
                changed = true;
            }
        }

        return changed;
    }

    bool ApplyDimensionEditRequest(
        ProjectDocument& document,
        EditorState& state,
        const FrameRequests::DimensionSelectionEditRequest& request)
    {
        if (!request.active)
        {
            return false;
        }

        if (request.setLengthUnit)
        {
            state.dimensionTool.newDimensionLengthUnit = request.lengthUnit;
            state.dimensionTool.dimensionLengthUnit = request.lengthUnit;
        }

        if (request.setOffsetMode)
        {
            state.dimensionTool.newDimensionOffsetMode = request.offsetMode;
            state.dimensionTool.dimensionOffsetMode = request.offsetMode;
        }

        if (request.dimensionIds.empty())
        {
            return false;
        }

        const double safeZoom = std::abs(request.cameraZoom) > 1.0e-6 ? request.cameraZoom : 1.0;
        bool changed = false;
        for (int dimensionId : request.dimensionIds)
        {
            Dimension* dimension = document.FindDimensionById(dimensionId);
            if (dimension == nullptr)
            {
                continue;
            }

            if (request.setOffsetMode && dimension->offsetMode != request.offsetMode)
            {
                if (request.offsetMode == DimensionOffsetMode::WorldUnits)
                {
                    dimension->offsetWorld = dimension->offsetPixels / safeZoom;
                }
                else
                {
                    dimension->offsetPixels = dimension->offsetWorld * safeZoom;
                }
                dimension->offsetMode = request.offsetMode;
                changed = true;
            }

            if (request.setLengthUnit && dimension->lengthUnit != request.lengthUnit)
            {
                dimension->lengthUnit = request.lengthUnit;
                changed = true;
            }
        }

        return changed;
    }

    void ApplyDisplayUnitsUpdateRequest(
        ProjectDocument& document,
        const FrameRequests::DisplayUnitsUpdateRequest& request)
    {
        if (!request.active)
        {
            return;
        }

        document.displayUnits = request.units;
    }
}

#include "editor/ops/PropertySelectionOperations.h"

namespace PropertySelectionOperations
{
    namespace
    {
        int ResolveCurrentId(
            const std::vector<int>& availableIds,
            int currentId)
        {
            if (availableIds.empty())
            {
                return -1;
            }

            for (int availableId : availableIds)
            {
                if (availableId == currentId)
                {
                    return currentId;
                }
            }

            return availableIds.front();
        }
    }

    int ResolveCurrentMaterialId(const ProjectDocument& document, int currentMaterialId)
    {
        std::vector<int> materialIds;
        materialIds.reserve(document.materials.size());
        for (const StructuralMaterial& material : document.materials)
        {
            materialIds.push_back(material.id);
        }

        return ResolveCurrentId(materialIds, currentMaterialId);
    }

    int ResolveCurrentSectionId(const ProjectDocument& document, int currentSectionId)
    {
        std::vector<int> sectionIds;
        sectionIds.reserve(document.sections.size());
        for (const Section& section : document.sections)
        {
            sectionIds.push_back(section.id);
        }

        return ResolveCurrentId(sectionIds, currentSectionId);
    }

    BeamPropertySelectionInfo ResolveMaterialSelectionInfo(
        const ProjectDocument& document,
        const Selection& selection,
        int currentMaterialId)
    {
        BeamPropertySelectionInfo info;
        info.currentId = ResolveCurrentMaterialId(document, currentMaterialId);
        info.displayedId = info.currentId;

        for (int beamId : selection.beamIds)
        {
            const Beam* beam = document.FindBeamById(beamId);
            if (beam == nullptr)
            {
                continue;
            }

            if (!info.hasBeamValue)
            {
                info.displayedId = beam->materialId;
                info.hasBeamValue = true;
                continue;
            }

            if (info.displayedId != beam->materialId)
            {
                info.mixed = true;
                break;
            }
        }

        if (info.hasBeamValue && !info.mixed)
        {
            info.currentId = info.displayedId;
        }

        return info;
    }

    BeamPropertySelectionInfo ResolveSectionSelectionInfo(
        const ProjectDocument& document,
        const Selection& selection,
        int currentSectionId)
    {
        BeamPropertySelectionInfo info;
        info.currentId = ResolveCurrentSectionId(document, currentSectionId);
        info.displayedId = info.currentId;

        for (int beamId : selection.beamIds)
        {
            const Beam* beam = document.FindBeamById(beamId);
            if (beam == nullptr)
            {
                continue;
            }

            if (!info.hasBeamValue)
            {
                info.displayedId = beam->sectionId;
                info.hasBeamValue = true;
                continue;
            }

            if (info.displayedId != beam->sectionId)
            {
                info.mixed = true;
                break;
            }
        }

        if (info.hasBeamValue && !info.mixed)
        {
            info.currentId = info.displayedId;
        }

        return info;
    }
}

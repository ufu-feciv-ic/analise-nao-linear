#pragma once

#include "editor/Selection.h"
#include "model/ProjectDocument.h"

namespace PropertySelectionOperations
{
    struct BeamPropertySelectionInfo
    {
        int currentId = -1;
        int displayedId = -1;
        bool mixed = false;
        bool hasBeamValue = false;
    };

    int ResolveCurrentMaterialId(const ProjectDocument& document, int currentMaterialId);
    int ResolveCurrentSectionId(const ProjectDocument& document, int currentSectionId);

    BeamPropertySelectionInfo ResolveMaterialSelectionInfo(
        const ProjectDocument& document,
        const Selection& selection,
        int currentMaterialId);

    BeamPropertySelectionInfo ResolveSectionSelectionInfo(
        const ProjectDocument& document,
        const Selection& selection,
        int currentSectionId);
}

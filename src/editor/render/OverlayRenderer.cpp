#include "editor/render/EditorRendererInternal.h"

void EditorRenderer::RenderOverlayPass(
    const ProjectDocument& document,
    const ProjectDerivedData& derivedData,
    const EditorState& state,
    const Camera2D& camera,
    float zoomTarget) const
{
    DrawSupports(document, derivedData, state, camera);
    DrawPointLoads(document, derivedData, state, camera);
    DrawDistributedLoads(document, state, camera, zoomTarget);
}


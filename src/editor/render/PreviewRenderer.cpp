#include "editor/render/EditorRendererInternal.h"

void EditorRenderer::RenderPreviewPass(
    const ProjectDocument& document,
    const EditorState& state,
    const Camera2D& camera) const
{
    DrawBeamPreview(document, state, camera);
    DrawMovePreview(document, state, camera);
}


#include "editor/render/EditorRendererInternal.h"

void EditorRenderer::RenderBackgroundPass(
    const ProjectDocument& document,
    const ProjectDerivedData& derivedData,
    const EditorState& state,
    const Camera2D& camera)
{
    if (state.view.showShadows && state.view.showBeams)
    {
        DrawShadows(document, derivedData, camera);
    }

    if (!state.view.showGrid)
    {
        return;
    }

    const float targetSpacing = CalculateGridSpacing(kGridBaseSpacing, camera.zoom);
    UpdateGridTransition(targetSpacing, GetFrameTime());
    DrawGrid(camera);
}

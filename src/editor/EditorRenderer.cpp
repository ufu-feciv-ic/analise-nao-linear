#include "editor/render/EditorRendererInternal.h"

float EditorRenderer::CalculateGridSpacing(float baseSpacing, float zoomLevel) const
{
    const float safeZoom = (zoomLevel > 0.0001f) ? zoomLevel : 0.0001f;
    const float rawSpacing = baseSpacing / safeZoom;

    const float exponent = floorf(log10f(rawSpacing));
    return powf(10.0f, exponent);
}

void EditorRenderer::SetDimensionFont(Font font)
{
    m_dimensionFont = font;
    m_hasDimensionFont = font.texture.id != 0;
}

void EditorRenderer::UpdateGridTransition(float targetSpacing, float deltaTime)
{
    if (!m_gridInitialized)
    {
        m_gridInitialized = true;
        m_currentGridSpacing = targetSpacing;
        m_previousGridSpacing = targetSpacing;
        m_gridTransitionTime = kGridFadeDuration;
        m_gridTransitionActive = false;
        return;
    }

    if (fabsf(targetSpacing - m_currentGridSpacing) > 0.0001f)
    {
        m_previousGridSpacing = m_currentGridSpacing;
        m_currentGridSpacing = targetSpacing;
        m_gridTransitionTime = 0.0f;
        m_gridTransitionActive = true;
    }

    if (m_gridTransitionActive)
    {
        m_gridTransitionTime += deltaTime;

        if (m_gridTransitionTime >= kGridFadeDuration)
        {
            m_gridTransitionTime = kGridFadeDuration;
            m_gridTransitionActive = false;
            m_previousGridSpacing = m_currentGridSpacing;
        }
    }
}

void EditorRenderer::Render(
    const ProjectDocument& document,
    const ProjectDerivedData& derivedData,
    const EditorState& state,
    const Camera2D& camera,
    float zoomTarget)
{
    RenderBackgroundPass(document, derivedData, state, camera);
    RenderStructurePass(document, state, camera);
    RenderOverlayPass(document, derivedData, state, camera, zoomTarget);
    RenderDimensionPass(document, state, camera);
    RenderPreviewPass(document, state, camera);
}

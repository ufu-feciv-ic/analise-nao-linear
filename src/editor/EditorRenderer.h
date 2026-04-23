#pragma once

#include <cstdint>
#include <unordered_map>

#include "editor/cache/ProjectDerivedData.h"
#include "editor/EditorState.h"
#include "model/ProjectDocument.h"
#include "raylib.h"

class EditorRenderer
{
public:
    void Render(
        const ProjectDocument& document,
        const ProjectDerivedData& derivedData,
        const EditorState& state,
        const Camera2D& camera,
        float zoomTarget);
    void SetDimensionFont(Font font);

private:
    struct DistributedLoadArrowFadeState
    {
        int currentCount = 2;
        int previousCount = 2;
        int pendingCount = 2;
        float transitionAlpha = 1.0f;
        bool isTransitioning = false;
    };

    float CalculateGridSpacing(float baseSpacing, float zoomLevel) const;
    void UpdateGridTransition(float targetSpacing, float deltaTime);
    void DrawGrid(const Camera2D& camera);
    void DrawShadows(const ProjectDocument& document, const ProjectDerivedData& derivedData, const Camera2D& camera) const;
    void DrawSupports(const ProjectDocument& document, const ProjectDerivedData& derivedData, const EditorState& state, const Camera2D& camera) const;
    void DrawPointLoads(const ProjectDocument& document, const ProjectDerivedData& derivedData, const EditorState& state, const Camera2D& camera) const;
    void DrawDistributedLoads(const ProjectDocument& document, const EditorState& state, const Camera2D& camera, float zoomTarget) const;
    void DrawBeams(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const;
    void DrawNodes(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const;
    void DrawNodeHighlights(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const;
    void DrawDimensions(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const;
    void DrawBeamPreview(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const;
    void DrawMovePreview(const ProjectDocument& document, const EditorState& state, const Camera2D& camera) const;
    void RenderBackgroundPass(
        const ProjectDocument& document,
        const ProjectDerivedData& derivedData,
        const EditorState& state,
        const Camera2D& camera);
    void RenderStructurePass(
        const ProjectDocument& document,
        const EditorState& state,
        const Camera2D& camera) const;
    void RenderOverlayPass(
        const ProjectDocument& document,
        const ProjectDerivedData& derivedData,
        const EditorState& state,
        const Camera2D& camera,
        float zoomTarget) const;
    void RenderDimensionPass(
        const ProjectDocument& document,
        const EditorState& state,
        const Camera2D& camera) const;
    void RenderPreviewPass(
        const ProjectDocument& document,
        const EditorState& state,
        const Camera2D& camera) const;
    Font m_dimensionFont{};
    bool m_hasDimensionFont = false;
    bool m_gridInitialized = false;
    bool m_gridTransitionActive = false;
    mutable std::unordered_map<int, float> m_dimensionTextFadeById;
    mutable std::unordered_map<int, float> m_beamMaterialTextFadeById;
    mutable std::unordered_map<int, float> m_beamSectionTextFadeById;
    mutable std::unordered_map<std::uint64_t, DistributedLoadArrowFadeState> m_distributedLoadArrowFadeByKey;
    mutable std::unordered_map<std::uint64_t, float> m_distributedLoadTextFadeByKey;
    mutable float m_lastDistributedLoadZoom = 1.0f;
    mutable float m_distributedLoadZoomIdleTime = 0.0f;
    mutable bool m_distributedLoadZoomInitialized = false;

    float m_currentGridSpacing = 100.0f;
    float m_previousGridSpacing = 100.0f;
    float m_gridTransitionTime = 0.0f;

    static constexpr float kGridBaseSpacing = 100.0f;
    static constexpr float kGridFadeDuration = 0.25f;
    static constexpr float kIncomingMinAlpha = 0.35f;
    static constexpr float kDimensionFadeDuration = 0.2f;
    static constexpr float kDistributedLoadArrowFadeDuration = 0.14f;
    static constexpr float kDistributedLoadArrowZoomSettleDelay = 0.25f;
    static constexpr float kDistributedLoadArrowZoomChangeInterval = 0.01f;
};

#include "editor/interaction/BeamGuideSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
constexpr float kGuidePositionTolerance = 1.0e-6f;
constexpr float kGuideActivationDistancePixels = 10.0f;
}

namespace BeamGuideSystem
{
bool ShouldAutoCreateGuideForNewNodes(const EditorState& state)
{
    return state.view.showGuides && !state.view.showGrid;
}

void EnsureGuideAtPosition(EditorState& state, Vector2 position, double currentTime)
{
    const auto it = std::find_if(
        state.beamTool.beamGuides.begin(),
        state.beamTool.beamGuides.end(),
        [&](const BeamToolState::BeamGuide& guide)
        {
            return std::abs(guide.position.x - position.x) < kGuidePositionTolerance &&
                   std::abs(guide.position.y - position.y) < kGuidePositionTolerance;
        });

    if (it == state.beamTool.beamGuides.end())
    {
        state.beamTool.beamGuides.push_back(BeamToolState::BeamGuide{
            position,
            currentTime,
            false,
            false});
    }
}

void ResetGuideHoverState(EditorState& state)
{
    state.hover.beamGuideHoverNodeId = -1;
    state.hover.beamGuideHoverStartTime = 0.0;
    state.hover.beamGuideCreatedOnCurrentHover = false;
}

void ToggleBeamGuideAtPosition(EditorState& state, Vector2 position, double currentTime)
{
    const auto it = std::find_if(
        state.beamTool.beamGuides.begin(),
        state.beamTool.beamGuides.end(),
        [&](const BeamToolState::BeamGuide& guide)
        {
            return std::abs(guide.position.x - position.x) < kGuidePositionTolerance &&
                   std::abs(guide.position.y - position.y) < kGuidePositionTolerance;
        });

    if (it != state.beamTool.beamGuides.end())
    {
        state.beamTool.beamGuides.erase(it);
        return;
    }

    state.beamTool.beamGuides.push_back(BeamToolState::BeamGuide{
        position,
        currentTime,
        false,
        false});
}

void UpdateActiveBeamGuides(EditorState& state, Vector2 mouseWorldPosition, float cameraZoom)
{
    state.beamTool.activeBeamGuideXIndex = -1;
    state.beamTool.activeBeamGuideYIndex = -1;

    const float activationDistance = kGuideActivationDistancePixels / cameraZoom;
    float closestDistanceX = activationDistance;
    float closestDistanceY = activationDistance;
    float closestOriginDistanceX = std::numeric_limits<float>::max();
    float closestOriginDistanceY = std::numeric_limits<float>::max();

    for (BeamToolState::BeamGuide& guide : state.beamTool.beamGuides)
    {
        guide.activeX = false;
        guide.activeY = false;
    }

    for (std::size_t i = 0; i < state.beamTool.beamGuides.size(); ++i)
    {
        BeamToolState::BeamGuide& guide = state.beamTool.beamGuides[i];

        const float distanceX = std::abs(mouseWorldPosition.x - guide.position.x);
        const float dx = mouseWorldPosition.x - guide.position.x;
        const float dy = mouseWorldPosition.y - guide.position.y;
        const float originDistance = dx * dx + dy * dy;
        guide.activeX = distanceX <= activationDistance;

        if (guide.activeX &&
            (distanceX < closestDistanceX ||
             (std::abs(distanceX - closestDistanceX) < kGuidePositionTolerance &&
              originDistance < closestOriginDistanceX)))
        {
            closestDistanceX = distanceX;
            closestOriginDistanceX = originDistance;
            state.beamTool.activeBeamGuideXIndex = static_cast<int>(i);
        }

        const float distanceY = std::abs(mouseWorldPosition.y - guide.position.y);
        guide.activeY = distanceY <= activationDistance;
        if (guide.activeY &&
            (distanceY < closestDistanceY ||
             (std::abs(distanceY - closestDistanceY) < kGuidePositionTolerance &&
              originDistance < closestOriginDistanceY)))
        {
            closestDistanceY = distanceY;
            closestOriginDistanceY = originDistance;
            state.beamTool.activeBeamGuideYIndex = static_cast<int>(i);
        }
    }
}
}


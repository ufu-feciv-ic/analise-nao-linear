#pragma once

namespace
{
    constexpr float kBeamLabelFadeOutThresholdRatio = 0.55f;
    constexpr float kBeamLabelFadeInThresholdRatio = 0.72f;

    struct BeamTextLabelJob
    {
        int beamId = -1;
        bool isMaterialLabel = false;
        Vector2 screenCenter{};
        std::string text;
        Vector2 textSize{};
        float rotationDegrees = 0.0f;
        float segmentLengthPixels = 0.0f;
        Color textColor{};
    };
}

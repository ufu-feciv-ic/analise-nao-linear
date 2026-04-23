#pragma once

#include <raylib.h>
#include "editor/AnalysisTypes.h"

struct ViewOptions
{
    bool showGrid = false;
    bool showGuides = true;
    bool snapToGrid = true;
    bool showShadows = true;
    bool showBeams = true;
    bool showNodes = true;
    bool showDimensions = true;
    bool showBeamMaterials = false;
    bool showBeamSections = false;

    bool enableDimensionTextFade = true;

    bool showPointLoads = true;
    bool showPointLoadResultant = false;
    bool showDistributedLoadResultant = true;
    bool suppressPointLoadText = false;
    bool showDistributedLoadArea = true;

    float pointLoadForceMaxPixels = 50.0f;
    float pointLoadMomentMaxPixels = 50.0f;
    float distributedLoadMaxPixels = 50.0f;

    float pointLoadForceSliderLimitPixels = 150.0f;
    float pointLoadMomentSliderLimitPixels = 150.0f;
    float distributedLoadSliderLimitPixels = 150.0f;

    ResultsViewType resultsViewType = ResultsViewType::None;
    float resultsScale = 1.0f;

    bool showBeamRenderTestWindow = true;
    float beamRenderFillThickness = 6.5f;
    float beamRenderLowerEdgeThickness = 3.0f;
    float beamRenderUpperEdgeThickness = 3.0f;
    Color beamRenderFillColor{118, 118, 118, 255};
    Color beamRenderLowerEdgeColor{72, 72, 72, 255};
    Color beamRenderUpperEdgeColor{200, 200, 200, 255};
};

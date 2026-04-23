#pragma once

#include "raylib.h"

class EditorCameraController
{
public:
    void Initialize(Camera2D& camera) const;
    void Update(Camera2D& camera, bool mouseWorldInputAllowed);
    float GetTargetZoom() const { return targetZoom; }

private:
    float targetZoom = 100.0f;
    bool hasZoomFocus = false;
    Vector2 zoomFocusScreen{0.0f, 0.0f};
    Vector2 zoomFocusWorld{0.0f, 0.0f};
};

#include "editor/EditorCameraController.h"

#include "raymath.h"

namespace
{
constexpr float kMinZoom = 5.0f;
constexpr float kMaxZoom = 50000.0f;
constexpr float kCameraTargetLimit = 100.0f;
}

void EditorCameraController::Initialize(Camera2D& camera) const
{
    camera.offset = Vector2{GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
    camera.target = Vector2{0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 100.0f;
}

void EditorCameraController::Update(Camera2D& camera, bool mouseWorldInputAllowed)
{
    if (IsWindowResized())
    {
        camera.offset = Vector2{GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
    }

    camera.offset = Vector2{GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};

    if (mouseWorldInputAllowed && IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        // As soon as the user starts panning, stop preserving the last zoom focus.
        hasZoomFocus = false;
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f / camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
        camera.target.x = Clamp(camera.target.x, -kCameraTargetLimit, kCameraTargetLimit);
        camera.target.y = Clamp(camera.target.y, -kCameraTargetLimit, kCameraTargetLimit);
    }

    if (mouseWorldInputAllowed)
    {
        const float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
        {
            zoomFocusScreen = GetMousePosition();
            zoomFocusWorld = GetScreenToWorld2D(zoomFocusScreen, camera);
            hasZoomFocus = true;

            float scaleFactor = 1.0f + 0.25f * fabsf(wheel);
            if (wheel < 0.0f)
            {
                scaleFactor = 1.0f / scaleFactor;
            }

            targetZoom = Clamp(targetZoom * scaleFactor, kMinZoom, kMaxZoom);
        }
    }

    const float smoothing = 0.0000001f;
    camera.zoom = (camera.zoom - targetZoom) * powf(smoothing, GetFrameTime()) + targetZoom;
    camera.zoom = Clamp(camera.zoom, kMinZoom, kMaxZoom);

    if (hasZoomFocus)
    {
        const Vector2 focusedWorldAfterZoom = GetScreenToWorld2D(zoomFocusScreen, camera);
        const Vector2 correction = Vector2Subtract(zoomFocusWorld, focusedWorldAfterZoom);
        camera.target = Vector2Add(camera.target, correction);
        camera.target.x = Clamp(camera.target.x, -kCameraTargetLimit, kCameraTargetLimit);
        camera.target.y = Clamp(camera.target.y, -kCameraTargetLimit, kCameraTargetLimit);

        if (fabsf(camera.zoom - targetZoom) <= 0.01f)
        {
            hasZoomFocus = false;
        }
    }
    else
    {
        camera.target.x = Clamp(camera.target.x, -kCameraTargetLimit, kCameraTargetLimit);
        camera.target.y = Clamp(camera.target.y, -kCameraTargetLimit, kCameraTargetLimit);
    }
}

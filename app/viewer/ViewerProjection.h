#pragma once

// ViewerProjection maps scene marker directions through a deterministic camera
// view.
//
// The camera is defined by yaw/pitch/roll (degrees) and a vertical field of
// view (degrees), using the same angle semantics as ViewportState (yaw/roll
// normalized to [-180, 180), pitch clamped to [-90, 90], FOV clamped to
// [20, 140]). The class is stateless and independent of QtWidgets so the
// camera/view transformation can be unit tested headlessly.
//
// Conventions:
//   - The camera faces world direction +Y at yaw 0, pitch 0 (FRONT).
//   - +X is to the camera's right, +Z is up for yaw/pitch 0.
//   - Positive roll rotates the projected view content counter-clockwise on
//     screen (a clockwise camera roll as seen by the viewer).
//   - A marker whose direction equals the camera direction projects to the
//     exact center of the view.
class ViewerProjection
{
public:
    // Projects a marker direction (world yaw/pitch degrees) into the camera
    // view of the given canvas size.
    //
    // Returns false when the marker is behind the camera or effectively on the
    // 90-degree sideways plane (not deterministically visible). On true,
    // *outPixelX/*outPixelY receive the projected pixel position.
    static bool project(double markerYawDeg, double markerPitchDeg,
                        double cameraYawDeg, double cameraPitchDeg,
                        double cameraRollDeg, double fieldOfViewDeg,
                        double viewWidth, double viewHeight,
                        double *outPixelX, double *outPixelY);

private:
    ViewerProjection() = delete;
};

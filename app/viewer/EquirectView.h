#pragma once

#include <QImage>

// EquirectView renders a camera view of an equirectangular pixel source using
// a deterministic, CPU-only projection.
//
// The camera (yaw/pitch/roll/FOV) uses exactly the ViewportState /
// ViewerProjection conventions already verified in Objectives 1-3:
//   - the camera faces world direction +Y at yaw 0, pitch 0 (FRONT);
//   - +X is to the camera's right and +Z is up at yaw/pitch 0;
//   - positive yaw turns toward the world RIGHT direction (+X);
//   - positive roll rotates the projected content counter-clockwise on screen;
//   - FOV is the vertical field of view in degrees [20, 140].
// A source direction equal to the camera direction maps to the exact center.
//
// This is a low-level presentation primitive for the decode-free media-view
// pixel path. It deliberately defines no decoder/media-engine contract.
class EquirectView
{
public:
    // Cap on the rendered width used by presentation code as an
    // implementation/performance safeguard. It is NOT an architectural or
    // permanent visual-quality limit: later objectives may raise it or use an
    // optimized/GPU backend without changing the camera contract.
    static constexpr int MaxOutputWidth = 640;

    // Renders the equirectangular source as seen by the given camera into
    // *outImage (Format_ARGB32, outputWidth x outputHeight).
    //
    // Deterministically rejects invalid input (returns false without touching
    // *outImage and without crashing): null/empty source, non-positive output
    // dimensions, non-finite camera values, pitch outside [-90, 90], or FOV
    // outside [20, 140].
    static bool render(const QImage &equirectSource,
                       double cameraYawDeg, double cameraPitchDeg,
                       double cameraRollDeg, double fieldOfViewDeg,
                       int outputWidth, int outputHeight,
                       QImage *outImage);

private:
    EquirectView() = delete;
};

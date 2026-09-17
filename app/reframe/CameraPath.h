#pragma once

#include "reframe/CameraKeyframe.h"
#include "reframe/ReframePlan.h"

// CameraPath evaluates a ReframePlan into a virtual-camera state at any
// source timestamp. The evaluation is pure and deterministic: it reads no
// clock, touches no media, and always returns the same state for the same
// (plan, timeMs).
//
// Semantics:
//   - before the first keyframe: the first keyframe's camera;
//   - after the last keyframe: the last keyframe's camera;
//   - exactly on a keyframe time: that keyframe's camera;
//   - between keyframes: interpolation controlled by the earlier keyframe.
//
// Yaw interpolates along the shortest angular path and is normalized to
// [-180, 180). Pitch/roll/FOV interpolate linearly within their bounds.
struct CameraState
{
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    double rollDeg = 0.0;
    double fieldOfViewDeg = 90.0;
};

class CameraPath
{
public:
    static CameraState stateAt(const ReframePlan &plan, qint64 timeMs);

    // Interpolates between two keyframes at normalized t in [0, 1]. When a
    // uses Hold interpolation the result is a's camera.
    static CameraState interpolate(const CameraKeyframe &a,
                                   const CameraKeyframe &b, double t);

    // Shortest signed yaw difference (to - from) in (-180, 180], normalized.
    static double shortestYawDelta(double fromDeg, double toDeg);

private:
    CameraPath() = delete;
};

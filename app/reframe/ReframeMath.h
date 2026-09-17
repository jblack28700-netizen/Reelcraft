#pragma once

#include <QtGlobal>
#include <QtMath>

#include <cmath>

// Shared deterministic angle rules for the 360 reframing engine.
//
// These are exactly the ViewportState / ViewerProjection / EquirectView
// conventions, so engineered camera paths, the interactive viewer, and the
// projection math all agree on yaw/pitch/roll/FOV semantics:
//   - yaw/roll are normalized to [-180, 180);
//   - pitch is clamped to [-90, 90];
//   - vertical FOV is clamped to [20, 140].
namespace reframe {

constexpr double kMinFieldOfViewDeg = 20.0;
constexpr double kMaxFieldOfViewDeg = 140.0;
constexpr double kMaxPitchDeg = 90.0;

inline bool isFinite(double value)
{
    return std::isfinite(value);
}

inline double normalizeYawDeg(double value)
{
    while (value < -180.0) {
        value += 360.0;
    }
    while (value >= 180.0) {
        value -= 360.0;
    }
    return value;
}

inline double normalizeRollDeg(double value)
{
    return normalizeYawDeg(value);
}

inline double clampPitchDeg(double value)
{
    return qBound(-kMaxPitchDeg, value, kMaxPitchDeg);
}

inline double clampFieldOfViewDeg(double value)
{
    return qBound(kMinFieldOfViewDeg, value, kMaxFieldOfViewDeg);
}

inline bool isValidPitchDeg(double value)
{
    return isFinite(value) && value >= -kMaxPitchDeg && value <= kMaxPitchDeg;
}

inline bool isValidFieldOfViewDeg(double value)
{
    return isFinite(value) && value >= kMinFieldOfViewDeg
        && value <= kMaxFieldOfViewDeg;
}

} // namespace reframe

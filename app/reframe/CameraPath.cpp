#include "CameraPath.h"

#include "reframe/ReframeMath.h"

CameraState CameraPath::stateAt(const ReframePlan &plan, qint64 timeMs)
{
    const QList<CameraKeyframe> frames = plan.keyframes();
    CameraState state;
    if (frames.isEmpty()) {
        return state;
    }

    const auto stateOf = [](const CameraKeyframe &frame) {
        CameraState s;
        s.yawDeg = reframe::normalizeYawDeg(frame.yawDeg);
        s.pitchDeg = reframe::clampPitchDeg(frame.pitchDeg);
        s.rollDeg = reframe::normalizeRollDeg(frame.rollDeg);
        s.fieldOfViewDeg = reframe::clampFieldOfViewDeg(frame.fieldOfViewDeg);
        return s;
    };

    if (timeMs <= frames.first().timeMs) {
        return stateOf(frames.first());
    }
    if (timeMs >= frames.last().timeMs) {
        return stateOf(frames.last());
    }

    for (int i = 0; i + 1 < frames.size(); ++i) {
        const CameraKeyframe &a = frames.at(i);
        const CameraKeyframe &b = frames.at(i + 1);
        if (timeMs < a.timeMs || timeMs > b.timeMs) {
            continue;
        }
        if (!(b.timeMs > a.timeMs)) {
            // Defensive: equal times cannot occur in a valid plan.
            return stateOf(b);
        }
        const double t = static_cast<double>(timeMs - a.timeMs)
            / static_cast<double>(b.timeMs - a.timeMs);
        return interpolate(a, b, t);
    }

    // Unreachable for a valid ordering; return the last frame defensively.
    return stateOf(frames.last());
}

CameraState CameraPath::interpolate(const CameraKeyframe &a,
                                    const CameraKeyframe &b, double t)
{
    CameraState state;
    if (a.interpolation == CameraKeyframe::Interpolation::Hold) {
        state.yawDeg = reframe::normalizeYawDeg(a.yawDeg);
        state.pitchDeg = reframe::clampPitchDeg(a.pitchDeg);
        state.rollDeg = reframe::normalizeRollDeg(a.rollDeg);
        state.fieldOfViewDeg = reframe::clampFieldOfViewDeg(a.fieldOfViewDeg);
        return state;
    }

    const double clampedT = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
    const double yaw = a.yawDeg + shortestYawDelta(a.yawDeg, b.yawDeg) * clampedT;
    state.yawDeg = reframe::normalizeYawDeg(yaw);
    state.pitchDeg = reframe::clampPitchDeg(
        a.pitchDeg + (b.pitchDeg - a.pitchDeg) * clampedT);
    state.rollDeg = reframe::normalizeRollDeg(
        a.rollDeg + (b.rollDeg - a.rollDeg) * clampedT);
    state.fieldOfViewDeg = reframe::clampFieldOfViewDeg(
        a.fieldOfViewDeg + (b.fieldOfViewDeg - a.fieldOfViewDeg) * clampedT);
    return state;
}

double CameraPath::shortestYawDelta(double fromDeg, double toDeg)
{
    double delta = reframe::normalizeYawDeg(toDeg) - reframe::normalizeYawDeg(fromDeg);
    if (delta > 180.0) {
        delta -= 360.0;
    } else if (delta <= -180.0) {
        delta += 360.0;
    }
    return delta;
}

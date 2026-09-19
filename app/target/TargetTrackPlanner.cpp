#include "target/TargetTrackPlanner.h"

#include "target/EquirectProjection.h"

#include <algorithm>
#include <cmath>

namespace {

// Objective 26: deterministic, model-free smoothing of the resolved trajectory.
//
// The renderer interpolates linearly between keyframes, so a path built straight
// from discrete observations has a velocity discontinuity at every keyframe and
// inherits every sample-to-sample wobble of the detection. A centred moving
// average over the yaw/pitch sequence removes that without introducing lag,
// overshoot or timing changes -- see Config::smoothingWindow for why the window
// shrinks symmetrically rather than being clipped at the ends.
void smoothKeyframeDirections(QList<CameraKeyframe> *keyframes, int window)
{
    if (!keyframes) {
        return;
    }
    const int count = keyframes->size();
    const int radius = window / 2;
    if (radius < 1 || count < 3) {
        return;
    }

    // Unwrap yaw into a continuous angle first, so averaging happens on the
    // circle: 179 -> -179 must read as +2 degrees, not -358.
    QList<double> unwrappedYaw;
    QList<double> pitches;
    unwrappedYaw.reserve(count);
    pitches.reserve(count);
    for (int i = 0; i < count; ++i) {
        const CameraKeyframe &frame = keyframes->at(i);
        if (i == 0) {
            unwrappedYaw.append(EquirectProjection::normalizeYawDeg(frame.yawDeg));
        } else {
            unwrappedYaw.append(
                unwrappedYaw.at(i - 1)
                + EquirectProjection::shortestYawDeltaDeg(
                      unwrappedYaw.at(i - 1), frame.yawDeg));
        }
        pitches.append(frame.pitchDeg);
    }

    for (int i = 0; i < count; ++i) {
        // Symmetric shrink: the window never extends further on one side than the
        // other, so the average stays centred on i and the endpoints keep a
        // one-element window (their own value).
        const int offset = qMin(radius, qMin(i, count - 1 - i));
        const int from = i - offset;
        const int to = i + offset;

        double yawSum = 0.0;
        double pitchSum = 0.0;
        for (int j = from; j <= to; ++j) {
            yawSum += unwrappedYaw.at(j);
            pitchSum += pitches.at(j);
        }
        const double divisor = static_cast<double>(to - from + 1);

        CameraKeyframe frame = keyframes->at(i);
        frame.yawDeg = EquirectProjection::normalizeYawDeg(yawSum / divisor);
        frame.pitchDeg = EquirectProjection::clampPitchDeg(pitchSum / divisor);
        (*keyframes)[i] = frame;
    }
}

} // namespace

bool TargetTrackPlanner::planTrack(const TargetTrack &track,
                                   const ReframePlan::TimeRange &range,
                                   const ReframePlan::OutputSpec &output,
                                   const Config &config, ReframePlan *outPlan,
                                   QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };

    if (!outPlan) {
        return fail(QStringLiteral("Target track plan output is null."));
    }
    if (!range.isValid()) {
        return fail(QStringLiteral("Target track plan range is invalid."));
    }
    if (!output.isValid()) {
        return fail(QStringLiteral("Target track plan output spec is invalid."));
    }

    QList<TargetObservation> observations;
    for (const TargetObservation &observation : track.observations()) {
        if (!observation.isValid()
            || observation.confidence < config.minConfidence) {
            continue;
        }
        if (observation.timeMs < range.startMs
            || observation.timeMs > range.endMs) {
            continue;
        }
        observations.append(observation);
    }
    if (observations.isEmpty()) {
        return fail(QStringLiteral(
            "Target track has no usable observations in the requested range."));
    }

    const int maxKeyframes = qMax(1, config.maxKeyframes);
    QList<TargetObservation> selected;
    if (observations.size() <= maxKeyframes) {
        selected = observations;
    } else {
        const int count = observations.size();
        for (int i = 0; i < maxKeyframes; ++i) {
            const int index = static_cast<int>(std::llround(
                static_cast<double>(i) * static_cast<double>(count - 1)
                / static_cast<double>(maxKeyframes - 1)));
            selected.append(observations.at(qBound(0, index, count - 1)));
        }
    }

    QList<CameraKeyframe> keyframes;
    for (const TargetObservation &observation : selected) {
        CameraKeyframe frame;
        frame.timeMs = observation.timeMs;
        frame.yawDeg = EquirectProjection::normalizeYawDeg(observation.yawDeg);
        frame.pitchDeg = EquirectProjection::clampPitchDeg(observation.pitchDeg);
        frame.rollDeg = 0.0;
        frame.fieldOfViewDeg = config.fieldOfViewDeg;
        frame.interpolation = CameraKeyframe::Interpolation::Linear;
        // Keep keyframe times strictly increasing (drop duplicate timestamps).
        if (!keyframes.isEmpty() && frame.timeMs <= keyframes.last().timeMs) {
            continue;
        }
        keyframes.append(frame);
    }
    if (keyframes.isEmpty()) {
        return fail(QStringLiteral("Target track produced no keyframes."));
    }

    // Objective 26: smooth the resolved trajectory in place. Times, ordering and
    // the number of keyframes are untouched, so nothing downstream changes shape.
    smoothKeyframeDirections(&keyframes, config.smoothingWindow);

    ReframePlan plan;
    plan.setSourceRange(range);
    plan.setOutput(output);
    plan.setKeyframes(keyframes);

    QString validationError;
    if (!plan.isValid(&validationError)) {
        return fail(validationError);
    }
    *outPlan = plan;
    return true;
}

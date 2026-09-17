#include "target/TargetTrackPlanner.h"

#include "target/EquirectProjection.h"

#include <algorithm>
#include <cmath>

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

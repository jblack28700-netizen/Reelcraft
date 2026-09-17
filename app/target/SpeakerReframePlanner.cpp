#include "target/SpeakerReframePlanner.h"

#include <algorithm>

namespace {

const TargetTrack *findTrack(const QList<TargetTrack> &tracks, const QString &id)
{
    for (const TargetTrack &track : tracks) {
        if (track.id() == id) {
            return &track;
        }
    }
    return nullptr;
}

} // namespace

bool SpeakerReframePlanner::plan(const QList<SpeakerSegment> &segments,
                                 const QList<TargetTrack> &tracks,
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
        return fail(QStringLiteral("Speaker plan output is null."));
    }
    if (!range.isValid()) {
        return fail(QStringLiteral("Speaker plan range is invalid."));
    }
    if (!output.isValid()) {
        return fail(QStringLiteral("Speaker plan output spec is invalid."));
    }

    QList<CameraKeyframe> keyframes;
    QString lastTarget;
    double lastYaw = 0.0;
    double lastPitch = 0.0;

    for (const SpeakerSegment &segment : segments) {
        if (segment.verdict != SpeakerVerdict::Active
            || segment.targetId.isEmpty()) {
            continue;
        }
        if (segment.endMs < range.startMs || segment.startMs > range.endMs) {
            continue;
        }
        const TargetTrack *track = findTrack(tracks, segment.targetId);
        if (!track || track->isEmpty()) {
            continue;
        }
        const qint64 sampleTime = qBound(track->firstTimeMs(),
                                         (segment.startMs + segment.endMs) / 2,
                                         track->lastTimeMs());
        TargetObservation observation;
        if (!track->sampleAt(sampleTime, &observation)) {
            continue;
        }
        qint64 timeMs = qBound(range.startMs, segment.startMs, range.endMs);

        if (!lastTarget.isEmpty() && lastTarget != segment.targetId
            && timeMs > range.startMs + 1) {
            CameraKeyframe hold;
            hold.timeMs = timeMs - 1;
            hold.yawDeg = lastYaw;
            hold.pitchDeg = lastPitch;
            hold.rollDeg = 0.0;
            hold.fieldOfViewDeg = config.fieldOfViewDeg;
            hold.interpolation = CameraKeyframe::Interpolation::Linear;
            if (keyframes.isEmpty() || hold.timeMs > keyframes.last().timeMs) {
                keyframes.append(hold);
            }
        }

        CameraKeyframe keyframe;
        keyframe.timeMs = timeMs;
        keyframe.yawDeg = observation.yawDeg;
        keyframe.pitchDeg = observation.pitchDeg;
        keyframe.rollDeg = 0.0;
        keyframe.fieldOfViewDeg = config.fieldOfViewDeg;
        keyframe.interpolation = CameraKeyframe::Interpolation::Linear;
        if (!keyframes.isEmpty() && keyframe.timeMs <= keyframes.last().timeMs) {
            continue;
        }
        keyframes.append(keyframe);
        lastTarget = segment.targetId;
        lastYaw = observation.yawDeg;
        lastPitch = observation.pitchDeg;
    }

    if (keyframes.isEmpty()) {
        return fail(QStringLiteral(
            "No active speaker segment with a visible target in the range."));
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

#include "target/TargetTrackPlanner.h"

#include "target/EquirectProjection.h"

#include <QtMath>

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

TargetTrackPlanner::EnclosingFraming TargetTrackPlanner::enclosingFramingDeg(
    const QList<TargetObservation> &observations, int outputWidth,
    int outputHeight)
{
    EnclosingFraming framing;
    if (observations.size() < 2) {
        framing.error = QStringLiteral(
            "Framing several subjects needs at least two observations.");
        return framing;
    }
    if (outputWidth <= 0 || outputHeight <= 0) {
        framing.error = QStringLiteral(
            "Enclosing framing needs a valid output geometry.");
        return framing;
    }
    for (const TargetObservation &observation : observations) {
        if (!observation.isValid()) {
            framing.error =
                QStringLiteral("Enclosing framing has an invalid observation.");
            return framing;
        }
    }

    // Yaw is unwrapped around the first observation, so a group that spans the
    // +/-180 boundary is measured the short way round instead of across the
    // whole sphere.
    const double base =
        EquirectProjection::normalizeYawDeg(observations.first().yawDeg);
    double minYaw = 0.0;
    double maxYaw = 0.0;
    double minPitch = 0.0;
    double maxPitch = 0.0;
    bool first = true;
    for (const TargetObservation &observation : observations) {
        const double yaw = base + EquirectProjection::shortestYawDeltaDeg(
                                      base, observation.yawDeg);
        const double yawRadius = qMax(0.0, observation.yawRadiusDeg);
        const double pitchRadius = qMax(0.0, observation.pitchRadiusDeg);
        const double pitch =
            EquirectProjection::clampPitchDeg(observation.pitchDeg);
        const double lowYaw = yaw - yawRadius;
        const double highYaw = yaw + yawRadius;
        const double lowPitch = qMax(-EquirectProjection::MaxPitchDeg,
                                     pitch - pitchRadius);
        const double highPitch = qMin(EquirectProjection::MaxPitchDeg,
                                      pitch + pitchRadius);
        if (first) {
            minYaw = lowYaw;
            maxYaw = highYaw;
            minPitch = lowPitch;
            maxPitch = highPitch;
            first = false;
            continue;
        }
        minYaw = qMin(minYaw, lowYaw);
        maxYaw = qMax(maxYaw, highYaw);
        minPitch = qMin(minPitch, lowPitch);
        maxPitch = qMax(maxPitch, highPitch);
    }

    const double aspect =
        static_cast<double>(outputWidth) / static_cast<double>(outputHeight);

    // The aim is the centre of the unwrapped yaw span and of the pitch span: the
    // only preference-free choice, since it favours no subject.
    const double aimYaw = EquirectProjection::normalizeYawDeg((minYaw + maxYaw) / 2.0);
    const double aimPitch = EquirectProjection::clampPitchDeg((minPitch + maxPitch) / 2.0);

    // The requirement is computed EXACTLY in the renderer's own basis at that
    // aim (roll 0), not from the spans: EquirectView builds a pixel's ray as
    // forward + right*(ndcX*tanHalf*aspect) + up*(ndcY*tanHalf), so a direction
    // is inside the frame exactly when |lateral/forward| <= tanHalf*aspect and
    // |vertical/forward| <= tanHalf. Deriving the requirement from the spans
    // instead is only an approximation, and it under-frames where a footprint
    // occupies yaw AND pitch at once.
    const double aimYawRad = qDegreesToRadians(aimYaw);
    const double aimPitchRad = qDegreesToRadians(aimPitch);
    const double forward[3] = { std::cos(aimPitchRad) * std::sin(aimYawRad),
                                std::cos(aimPitchRad) * std::cos(aimYawRad),
                                std::sin(aimPitchRad) };
    // Same construction and same degenerate fallback as the renderer.
    double right[3] = { forward[1], -forward[0], 0.0 };
    const double rightLength = std::sqrt(right[0] * right[0]
                                         + right[1] * right[1]
                                         + right[2] * right[2]);
    if (rightLength < 1e-12) {
        right[0] = 1.0;
        right[1] = 0.0;
        right[2] = 0.0;
    } else {
        right[0] /= rightLength;
        right[1] /= rightLength;
        right[2] /= rightLength;
    }
    const double up[3] = { right[1] * forward[2] - right[2] * forward[1],
                           right[2] * forward[0] - right[0] * forward[2],
                           right[0] * forward[1] - right[1] * forward[0] };

    double requiredTanHalf = 0.0;
    for (const TargetObservation &observation : observations) {
        const double yaw = EquirectProjection::normalizeYawDeg(observation.yawDeg);
        const double yawRadius = qMax(0.0, observation.yawRadiusDeg);
        const double pitchRadius = qMax(0.0, observation.pitchRadiusDeg);
        const double pitch = EquirectProjection::clampPitchDeg(observation.pitchDeg);
        const double cornersYaw[2] = { yaw - yawRadius, yaw + yawRadius };
        const double cornersPitch[2] = {
            EquirectProjection::clampPitchDeg(pitch - pitchRadius),
            EquirectProjection::clampPitchDeg(pitch + pitchRadius)
        };
        for (double cornerYaw : cornersYaw) {
            for (double cornerPitch : cornersPitch) {
                const double yawRad = qDegreesToRadians(cornerYaw);
                const double pitchRad = qDegreesToRadians(cornerPitch);
                const double direction[3] = {
                    std::cos(pitchRad) * std::sin(yawRad),
                    std::cos(pitchRad) * std::cos(yawRad),
                    std::sin(pitchRad)
                };
                const double alongForward = direction[0] * forward[0]
                    + direction[1] * forward[1] + direction[2] * forward[2];
                if (alongForward <= 1e-9) {
                    // A footprint corner at or behind the view plane cannot be
                    // contained by any field of view from this aim: refuse rather
                    // than claim a framing that would clip it.
                    framing.error = QStringLiteral(
                        "Keeping every requested subject in frame needs more "
                        "than the supported maximum of %1 degrees of field of "
                        "view.")
                                        .arg(QString::number(
                                            EquirectProjection::MaxFieldOfViewDeg,
                                            'f', 1));
                    return framing;
                }
                const double lateral = direction[0] * right[0]
                    + direction[1] * right[1] + direction[2] * right[2];
                const double vertical = direction[0] * up[0]
                    + direction[1] * up[1] + direction[2] * up[2];
                requiredTanHalf = qMax(requiredTanHalf,
                                       qAbs(lateral / alongForward) / aspect);
                requiredTanHalf = qMax(requiredTanHalf,
                                       qAbs(vertical / alongForward));
            }
        }
    }

    if (!std::isfinite(requiredTanHalf)) {
        framing.error = QStringLiteral(
            "Enclosing framing could not be computed from the observations.");
        return framing;
    }
    const double requiredFieldOfView =
        qRadiansToDegrees(2.0 * std::atan(requiredTanHalf));
    if (requiredFieldOfView > EquirectProjection::MaxFieldOfViewDeg) {
        framing.error = QStringLiteral(
            "Keeping every requested subject in frame needs at least %1 "
            "degrees of field of view, which exceeds the supported maximum of "
            "%2 degrees.")
                            .arg(QString::number(requiredFieldOfView, 'f', 1))
                            .arg(QString::number(
                                EquirectProjection::MaxFieldOfViewDeg, 'f', 1));
        return framing;
    }
    // A framing tighter than the renderer's minimum is not renderable; raising it
    // to the minimum still contains every subject, so this is not a clamp that
    // could hide one.
    const double lens =
        qMax(requiredFieldOfView, EquirectProjection::MinFieldOfViewDeg);

    framing.ok = true;
    framing.yawDeg = aimYaw;
    framing.pitchDeg = aimPitch;
    framing.fieldOfViewDeg = lens;
    return framing;
}

bool TargetTrackPlanner::planTracks(const QList<TargetTrack> &tracks,
                                    const QList<QString> &trackIds,
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
        return fail(QStringLiteral("Multi-subject plan output is null."));
    }
    if (!range.isValid()) {
        return fail(QStringLiteral("Multi-subject plan range is invalid."));
    }
    if (!output.isValid()) {
        return fail(QStringLiteral("Multi-subject plan output spec is invalid."));
    }
    if (trackIds.size() < 2) {
        return fail(QStringLiteral(
            "Framing several subjects needs at least two resolved targets."));
    }

    // Every requested subject must have a usable observation inside the range.
    // A subject with none is refused: dropping it would answer a different
    // question than the one that was asked.
    QList<QList<TargetObservation>> usable;
    for (const QString &trackId : trackIds) {
        const TargetTrack *track = nullptr;
        for (const TargetTrack &candidate : tracks) {
            if (candidate.id() == trackId) {
                track = &candidate;
                break;
            }
        }
        if (!track) {
            return fail(QStringLiteral(
                "Resolved target '%1' is no longer among the resolved tracks.")
                            .arg(trackId));
        }
        QList<TargetObservation> selected;
        for (const TargetObservation &observation : track->observations()) {
            if (!observation.isValid()
                || observation.confidence < config.minConfidence) {
                continue;
            }
            if (observation.timeMs < range.startMs
                || observation.timeMs > range.endMs) {
                continue;
            }
            selected.append(observation);
        }
        if (selected.isEmpty()) {
            return fail(QStringLiteral(
                "Target '%1' has no usable observation in the requested range.")
                            .arg(trackId));
        }
        usable.append(selected);
    }

    // A framing can only be computed where EVERY requested subject was observed.
    // Nothing is interpolated or invented, so a subject that was not seen at a
    // timestamp simply produces no keyframe there.
    QList<qint64> jointTimes;
    const auto observationAt = [](const QList<TargetObservation> &list,
                                  qint64 timeMs) -> const TargetObservation * {
        for (const TargetObservation &observation : list) {
            if (observation.timeMs == timeMs) {
                return &observation;
            }
        }
        return nullptr;
    };
    for (const TargetObservation &observation : usable.first()) {
        bool everySubjectObserved = true;
        for (int i = 1; i < usable.size(); ++i) {
            if (!observationAt(usable.at(i), observation.timeMs)) {
                everySubjectObserved = false;
                break;
            }
        }
        if (everySubjectObserved) {
            jointTimes.append(observation.timeMs);
        }
    }
    if (jointTimes.isEmpty()) {
        return fail(QStringLiteral(
            "The requested subjects were never observed together in the "
            "requested range."));
    }

    // The framing at every joint timestamp, and the tightest lens that contains
    // every subject for the WHOLE instruction.
    QList<EnclosingFraming> framings;
    double requiredLens = 0.0;
    qint64 requiredAtMs = jointTimes.first();
    for (qint64 timeMs : jointTimes) {
        QList<TargetObservation> atTime;
        for (const QList<TargetObservation> &list : usable) {
            if (const TargetObservation *observation =
                    observationAt(list, timeMs)) {
                atTime.append(*observation);
            }
        }
        const EnclosingFraming framing =
            enclosingFramingDeg(atTime, output.width, output.height);
        if (!framing.ok) {
            return fail(QStringLiteral("%1 (at %2 ms)")
                            .arg(framing.error)
                            .arg(timeMs));
        }
        framings.append(framing);
        if (framing.fieldOfViewDeg > requiredLens) {
            requiredLens = framing.fieldOfViewDeg;
            requiredAtMs = timeMs;
        }
    }

    // One lens for the whole instruction. A lens the instruction explicitly
    // asked for is honoured when it can contain every subject, and refused when
    // it cannot — never widened behind the creator's back, and never narrowed
    // into a framing that drops someone.
    double lensDeg = requiredLens;
    if (config.requestedFieldOfViewDeg > 0.0) {
        if (config.requestedFieldOfViewDeg + 1e-9 < requiredLens) {
            return fail(QStringLiteral(
                "The requested field of view (%1 degrees) is too narrow to "
                "keep every requested subject in frame: at least %2 degrees is "
                "needed (at %3 ms).")
                            .arg(QString::number(config.requestedFieldOfViewDeg,
                                                'f', 1))
                            .arg(QString::number(requiredLens, 'f', 1))
                            .arg(requiredAtMs));
        }
        lensDeg = config.requestedFieldOfViewDeg;
    }

    QList<CameraKeyframe> keyframes;
    for (int i = 0; i < jointTimes.size(); ++i) {
        CameraKeyframe frame;
        frame.timeMs = jointTimes.at(i);
        frame.yawDeg = framings.at(i).yawDeg;
        frame.pitchDeg = framings.at(i).pitchDeg;
        frame.rollDeg = 0.0;
        frame.fieldOfViewDeg = lensDeg;
        frame.interpolation = CameraKeyframe::Interpolation::Linear;
        if (!keyframes.isEmpty() && frame.timeMs <= keyframes.last().timeMs) {
            continue;
        }
        keyframes.append(frame);
    }
    if (keyframes.isEmpty()) {
        return fail(QStringLiteral(
            "The requested subjects produced no framing keyframe."));
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


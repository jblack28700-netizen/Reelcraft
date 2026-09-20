#include "ReframePlanBuilder.h"

#include "reframe/ReframeMath.h"

#include <cmath>

namespace {

// Objective 29: the lens a plan uses when the instruction says nothing about
// framing. It is the same default CameraKeyframe and TargetTrackPlanner already
// carry, so an instruction with no framing clause produces exactly the plan it
// always did.
constexpr double kDefaultFieldOfViewDeg = 90.0;

bool findResolvedTarget(const QList<ReframeTarget> &targets,
                        const QString &reference, ReframeTarget *out)
{
    for (const ReframeTarget &target : targets) {
        if (target.id.compare(reference, Qt::CaseInsensitive) == 0) {
            *out = target;
            return true;
        }
    }
    return false;
}

} // namespace

ReframeBuildResult ReframePlanBuilder::build(
    const ReframeIntent &intent,
    const QList<ReframeTarget> &resolvedTargets,
    const ReframePlan::TimeRange &defaultRange,
    const ReframePlan::OutputSpec &defaultOutput)
{
    ReframeBuildResult result;
    result.intent = intent;
    result.notes = intent.notes;

    const ReframePlan::TimeRange range =
        intent.hasTimeRange ? ReframePlan::TimeRange{ intent.startMs, intent.endMs }
                            : defaultRange;
    if (!range.isValid()) {
        result.error = QStringLiteral("Reframe source range is invalid.");
        return result;
    }

    const ReframePlan::OutputSpec output =
        intent.hasOutput
            ? ReframePlan::OutputSpec{ intent.outputWidth, intent.outputHeight,
                                       intent.outputFps }
            : defaultOutput;
    if (!output.isValid()) {
        result.error = QStringLiteral("Reframe output specification is invalid.");
        return result;
    }

    QList<ReframeCameraMove> moves = intent.moves;
    if (moves.isEmpty()) {
        // No camera instruction: deterministic centered forward view.
        ReframeCameraMove move;
        move.label = QStringLiteral("centered forward view");
        move.hasDirection = true;
        move.yawDeg = 0.0;
        move.pitchDeg = 0.0;
        moves.append(move);
        result.notes.append(QStringLiteral(
            "No camera instruction; using a static forward camera."));
    }

    // Objective 30: a plural framing instruction needs the resolved subjects to
    // decide its framing (a direction and a lens are not enough), so the builder
    // refuses it rather than approximating one. ReframeCommandRunner resolves
    // the group and plans it through TargetTrackPlanner before reaching here.
    for (const ReframeCameraMove &move : moves) {
        if (move.subjectGroup != ReframeSubjectGroup::None) {
            result.error = QStringLiteral(
                "A multi-subject framing instruction must be resolved into "
                "framing before a plan is built.");
            return result;
        }
    }

    // Resolve every move to a direction before building any keyframe.
    QList<QPair<double, double>> directions;
    for (const ReframeCameraMove &move : moves) {
        if (move.hasDirection) {
            directions.append({ move.yawDeg, move.pitchDeg });
            continue;
        }
        if (move.targetRef.isEmpty()) {
            if (!move.hasFieldOfView) {
                result.error = QStringLiteral(
                    "Camera instruction has neither a direction nor a target.");
                return result;
            }
            // Objective 29: a framing-only instruction changes the LENS without
            // moving the camera, so it holds the direction the camera already
            // has. An instruction that opens with a lens change frames the
            // centered forward view until something aims the camera.
            directions.append(directions.isEmpty()
                                  ? QPair<double, double>{ 0.0, 0.0 }
                                  : directions.last());
            continue;
        }
        ReframeTarget resolved;
        if (!findResolvedTarget(resolvedTargets, move.targetRef, &resolved)) {
            result.error = QStringLiteral(
                "Unresolved target reference: '%1'. Resolve the target "
                "direction before building a plan.").arg(move.targetRef);
            return result;
        }
        directions.append({ resolved.yawDeg, resolved.pitchDeg });
    }

    const qint64 duration = range.durationMs();
    const int count = directions.size();
    QList<CameraKeyframe> keyframes;
    // Objective 29: the lens is part of the camera state, so a framing
    // instruction changes it FROM THAT POINT ON and the camera keeps it until
    // another instruction changes it again — which is what a zoom ring does.
    // An instruction with no framing clause therefore renders at exactly the
    // default lens it always did.
    double lensDeg = kDefaultFieldOfViewDeg;
    for (int i = 0; i < count; ++i) {
        qint64 timeMs = range.startMs;
        if (count > 1) {
            timeMs = range.startMs
                + static_cast<qint64>(std::llround(
                    static_cast<double>(i) * static_cast<double>(duration)
                    / static_cast<double>(count - 1)));
        }
        if (moves.at(i).hasFieldOfView) {
            lensDeg = moves.at(i).fieldOfViewDeg;
        }
        CameraKeyframe frame;
        frame.timeMs = timeMs;
        frame.yawDeg = reframe::normalizeYawDeg(directions.at(i).first);
        frame.pitchDeg = reframe::clampPitchDeg(directions.at(i).second);
        frame.rollDeg = 0.0;
        frame.fieldOfViewDeg = lensDeg;
        frame.interpolation = CameraKeyframe::Interpolation::Linear;
        keyframes.append(frame);
    }

    ReframePlan plan;
    plan.setSourceRange(range);
    plan.setOutput(output);
    plan.setKeyframes(keyframes);

    QString validationError;
    if (!plan.isValid(&validationError)) {
        result.error = validationError;
        return result;
    }

    result.plan = plan;
    result.ok = true;
    return result;
}

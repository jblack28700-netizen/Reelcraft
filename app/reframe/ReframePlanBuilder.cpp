#include "ReframePlanBuilder.h"

#include "reframe/ReframeMath.h"

#include <cmath>

namespace {

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

    // Resolve every move to a direction before building any keyframe.
    QList<QPair<double, double>> directions;
    for (const ReframeCameraMove &move : moves) {
        if (move.hasDirection) {
            directions.append({ move.yawDeg, move.pitchDeg });
            continue;
        }
        if (move.targetRef.isEmpty()) {
            result.error = QStringLiteral(
                "Camera instruction has neither a direction nor a target.");
            return result;
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
    for (int i = 0; i < count; ++i) {
        qint64 timeMs = range.startMs;
        if (count > 1) {
            timeMs = range.startMs
                + static_cast<qint64>(std::llround(
                    static_cast<double>(i) * static_cast<double>(duration)
                    / static_cast<double>(count - 1)));
        }
        CameraKeyframe frame;
        frame.timeMs = timeMs;
        frame.yawDeg = reframe::normalizeYawDeg(directions.at(i).first);
        frame.pitchDeg = reframe::clampPitchDeg(directions.at(i).second);
        frame.rollDeg = 0.0;
        frame.fieldOfViewDeg = 90.0;
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

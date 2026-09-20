#include "ReframePlanReview.h"

#include <QCryptographicHash>
#include <QJsonDocument>

#include <cmath>

namespace {

// Seconds with millisecond precision, so a line is stable and readable.
QString secondsText(qint64 timeMs)
{
    return QString::number(static_cast<double>(timeMs) / 1000.0, 'f', 3);
}

QString degreesText(double value)
{
    return QString::number(value, 'f', 1);
}

} // namespace

QString ReframePlanReview::digestOf(const ReframePlan &plan)
{
    const QByteArray payload =
        QJsonDocument(plan.toJsonObject()).toJson(QJsonDocument::Compact);
    return QString::fromLatin1(
        QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex());
}

ReframePlanReview ReframePlanReview::fromPlan(
    const ReframePlan &plan, const QString &instruction, const QStringList &notes,
    const QList<ReframeTarget> &resolvedTargets)
{
    ReframePlanReview review;
    // A review of an invalid plan would be a lie, so nothing is derived.
    if (!plan.isValid()) {
        return review;
    }

    review.instruction = instruction;
    review.notes = notes;
    review.plan = plan;
    review.planDigest = digestOf(plan);

    for (const ReframeTarget &target : resolvedTargets) {
        review.resolvedSubjects.append(target.id);
    }

    // --- derived from the canonical plan only -------------------------------
    const QList<CameraKeyframe> keyframes = plan.keyframes();
    review.keyframeCount = keyframes.size();
    review.startMs = plan.sourceRange().startMs;
    review.endMs = plan.sourceRange().endMs;
    for (const ReframePlan::TimeRange &segment : plan.segments()) {
        review.retainedSegments.append(qMakePair(segment.startMs, segment.endMs));
    }
    if (!keyframes.isEmpty()) {
        const CameraKeyframe &first = keyframes.first();
        const CameraKeyframe &last = keyframes.last();
        review.startYawDeg = first.yawDeg;
        review.endYawDeg = last.yawDeg;
        review.startPitchDeg = first.pitchDeg;
        review.endPitchDeg = last.pitchDeg;
        review.cameraMoves = keyframes.size() > 1
            && (qAbs(last.yawDeg - first.yawDeg) > 1e-9
                || qAbs(last.pitchDeg - first.pitchDeg) > 1e-9);
        review.lensStartDeg = first.fieldOfViewDeg;
        review.lensEndDeg = last.fieldOfViewDeg;
        for (const CameraKeyframe &keyframe : keyframes) {
            if (qAbs(keyframe.fieldOfViewDeg - first.fieldOfViewDeg) > 1e-9) {
                review.lensIsConstant = false;
                break;
            }
        }
    }
    review.outputWidth = plan.output().width;
    review.outputHeight = plan.output().height;
    review.outputFps = plan.output().fps;
    review.orientation = review.outputHeight > review.outputWidth
        ? QStringLiteral("portrait")
        : (review.outputWidth > review.outputHeight ? QStringLiteral("landscape")
                                                    : QStringLiteral("square"));

    // --- the lines a creator reads ------------------------------------------
    const int subjectCount = review.resolvedSubjects.size();
    const int spanCount = review.retainedSegments.size();

    if (subjectCount == 0) {
        review.understanding = spanCount > 0
            ? QStringLiteral("no subject reference; %1 retained source span(s)")
                  .arg(spanCount)
            : QStringLiteral("no subject reference; the whole requested range");
    } else {
        review.understanding =
            QStringLiteral("%1 subject(s) resolved (%2)%3")
                .arg(subjectCount)
                .arg(review.resolvedSubjects.join(QStringLiteral(", ")))
                .arg(spanCount > 0
                         ? QStringLiteral("; %1 retained source span(s)").arg(spanCount)
                         : QString());
    }

    if (subjectCount == 0) {
        review.framing = QStringLiteral(
            "No subject is being framed: the camera follows the instruction's own "
            "direction.");
    } else if (subjectCount == 1) {
        review.framing = review.cameraMoves
            ? QStringLiteral("Keeps the one resolved subject (%1) inside the frame "
                             "while the camera moves.")
                  .arg(review.resolvedSubjects.first())
            : QStringLiteral("Aims at the one resolved subject (%1) and holds "
                             "that direction.")
                  .arg(review.resolvedSubjects.first());
    } else {
        review.framing = review.cameraMoves
            ? QStringLiteral("Keeps all %1 resolved subjects (%2) inside one frame "
                             "while the camera moves.")
                  .arg(subjectCount)
                  .arg(review.resolvedSubjects.join(QStringLiteral(", ")))
            : QStringLiteral("Keeps all %1 resolved subjects (%2) inside one frame.")
                  .arg(subjectCount)
                  .arg(review.resolvedSubjects.join(QStringLiteral(", ")));
    }

    review.lens = review.lensIsConstant
        ? QStringLiteral("%1 deg (constant)").arg(degreesText(review.lensStartDeg))
        : QStringLiteral("%1 deg -> %2 deg")
              .arg(degreesText(review.lensStartDeg), degreesText(review.lensEndDeg));

    review.cameraMovement = review.cameraMoves
        ? QStringLiteral("%1 keyframes; yaw %2 -> %3 deg, pitch %4 -> %5 deg")
              .arg(review.keyframeCount)
              .arg(degreesText(review.startYawDeg), degreesText(review.endYawDeg),
                   degreesText(review.startPitchDeg), degreesText(review.endPitchDeg))
        : QStringLiteral("one fixed direction; yaw %1 deg, pitch %2 deg, %3 keyframe(s)")
              .arg(degreesText(review.startYawDeg), degreesText(review.startPitchDeg))
              .arg(review.keyframeCount);

    if (spanCount == 0) {
        review.timeRange = QStringLiteral("%1 s - %2 s (one continuous range)")
                               .arg(secondsText(review.startMs),
                                    secondsText(review.endMs));
    } else {
        QStringList spans;
        for (const QPair<qint64, qint64> &segment : review.retainedSegments) {
            spans.append(QStringLiteral("%1-%2 s")
                             .arg(secondsText(segment.first),
                                  secondsText(segment.second)));
        }
        review.timeRange =
            QStringLiteral("%1 retained source span(s): %2 (source %3 s - %4 s)")
                .arg(spanCount)
                .arg(spans.join(QStringLiteral(", ")),
                     secondsText(review.startMs), secondsText(review.endMs));
    }

    review.output = QStringLiteral("%1x%2 at %3 fps (%4)")
                        .arg(review.outputWidth)
                        .arg(review.outputHeight)
                        .arg(QString::number(review.outputFps, 'g', 4), review.orientation);

    // The execution policy of the renderer (Decision 048), stated from the plan's
    // own retained spans: it never claims to know whether the source has audio.
    review.audio = spanCount == 0
        ? QStringLiteral("Source audio is preserved for the rendered range when the "
                         "source has an audio track; a source without audio renders "
                         "silently.")
        : QStringLiteral("Source audio is preserved over the %1 retained source "
                         "span(s) when the source has an audio track; a source "
                         "without audio renders silently.")
              .arg(spanCount);
    return review;
}

QStringList ReframePlanReview::summaryLines() const
{
    QStringList lines;
    if (!isValid()) {
        return lines;
    }
    lines.append(QStringLiteral("Instruction: %1").arg(instruction));
    lines.append(QStringLiteral("Understood: %1").arg(understanding));
    lines.append(QStringLiteral("Framing: %1").arg(framing));
    lines.append(QStringLiteral("Camera: %1").arg(cameraMovement));
    lines.append(QStringLiteral("Lens: %1").arg(lens));
    lines.append(QStringLiteral("Time: %1").arg(timeRange));
    lines.append(QStringLiteral("Output: %1").arg(output));
    lines.append(QStringLiteral("Audio: %1").arg(audio));
    lines.append(QStringLiteral("Plan: %1 keyframe(s), digest %2")
                     .arg(keyframeCount)
                     .arg(planDigest.left(12)));
    for (const QString &note : notes) {
        lines.append(QStringLiteral("Note: %1").arg(note));
    }
    return lines;
}

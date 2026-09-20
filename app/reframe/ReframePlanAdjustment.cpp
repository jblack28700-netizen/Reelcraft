#include "ReframePlanAdjustment.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

#include "reframe/ReframeIntent.h"
#include "reframe/ReframeMath.h"

namespace {

void fail(QString *error, const QString &message)
{
    if (error) {
        *error = message;
    }
}

QString degreesText(double value)
{
    return QString::number(value, 'f', 1);
}

} // namespace

QList<double> ReframePlanAdjustment::wideningLadderDegrees()
{
    // The ladder is the set of lenses the existing framing vocabulary can ask for,
    // plus the established default lens (90 degrees, the value every planner and
    // keyframe default uses), restricted to values at or above the default and
    // sorted ascending. Nothing here is a new number: ReframeIntent owns the
    // vocabulary, and this only removes duplicates and the tightening half.
    QList<double> ladder;
    const double defaultLens = CameraKeyframe().fieldOfViewDeg;
    ladder.append(defaultLens);
    for (double candidate : ReframeIntent::framingLadderFieldOfViews()) {
        if (candidate <= defaultLens) {
            continue;
        }
        bool present = false;
        for (double existing : ladder) {
            if (qAbs(existing - candidate) < 1e-9) {
                present = true;
                break;
            }
        }
        if (!present) {
            ladder.append(candidate);
        }
    }
    std::sort(ladder.begin(), ladder.end());
    return ladder;
}

double ReframePlanAdjustment::nextWiderLensDeg(double currentFieldOfViewDeg)
{
    if (!std::isfinite(currentFieldOfViewDeg)) {
        return 0.0;
    }
    for (double candidate : wideningLadderDegrees()) {
        if (candidate > currentFieldOfViewDeg + 1e-9) {
            return candidate;
        }
    }
    return 0.0;
}

double ReframePlanAdjustment::widestKeyframeFieldOfViewDeg(const ReframePlan &plan)
{
    double widest = 0.0;
    for (const CameraKeyframe &keyframe : plan.keyframes()) {
        widest = qMax(widest, keyframe.fieldOfViewDeg);
    }
    return widest;
}

bool ReframePlanAdjustment::widenLens(const ReframePlan &plan,
                                      double targetFieldOfViewDeg,
                                      ReframePlan *outPlan, QString *error)
{
    if (error) {
        error->clear();
    }
    if (!outPlan) {
        fail(error, QStringLiteral("No destination plan was supplied."));
        return false;
    }
    if (!std::isfinite(targetFieldOfViewDeg)) {
        fail(error, QStringLiteral("The requested lens is not a finite number."));
        return false;
    }
    // The bounds are REFUSED rather than clamped: clamping would produce a plan
    // that renders a different lens than the one it stores.
    if (!reframe::isValidFieldOfViewDeg(targetFieldOfViewDeg)) {
        fail(error, QStringLiteral(
                       "A lens of %1 degrees is outside the supported range [%2, %3].")
                       .arg(degreesText(targetFieldOfViewDeg),
                            degreesText(reframe::kMinFieldOfViewDeg),
                            degreesText(reframe::kMaxFieldOfViewDeg)));
        return false;
    }

    QString planError;
    if (!plan.isValid(&planError)) {
        fail(error, QStringLiteral("The plan to adjust is not valid: %1")
                        .arg(planError.isEmpty() ? QStringLiteral("invalid plan")
                                                 : planError));
        return false;
    }

    // A request that widens nothing is refused rather than recorded as a no-op
    // "revision": there would be nothing to attribute and nothing to explain.
    bool widens = false;
    for (const CameraKeyframe &keyframe : plan.keyframes()) {
        if (targetFieldOfViewDeg > keyframe.fieldOfViewDeg + 1e-9) {
            widens = true;
            break;
        }
    }
    if (!widens) {
        fail(error,
             QStringLiteral("The plan already renders at %1 degrees or wider, so a "
                            "lens of %2 degrees would not widen any keyframe.")
                 .arg(degreesText(widestKeyframeFieldOfViewDeg(plan)),
                      degreesText(targetFieldOfViewDeg)));
        return false;
    }

    // Copy, then raise only the field of view. Every other field is copied
    // verbatim, which is what isLensWidening() later verifies.
    ReframePlan adjusted = plan;
    QList<CameraKeyframe> keyframes = plan.keyframes();
    for (CameraKeyframe &keyframe : keyframes) {
        keyframe.fieldOfViewDeg = qMax(keyframe.fieldOfViewDeg, targetFieldOfViewDeg);
    }
    adjusted.setKeyframes(keyframes);

    // Verify rather than trust: the transformation's own invariants are re-derived
    // from the two plans before anything is allowed to use the result.
    QString validationError;
    if (!isLensWidening(plan, adjusted, &validationError)) {
        fail(error, QStringLiteral("The widened plan failed its own validation: %1")
                        .arg(validationError));
        return false;
    }
    *outPlan = adjusted;
    return true;
}

bool ReframePlanAdjustment::isLensWidening(const ReframePlan &original,
                                           const ReframePlan &adjusted,
                                           QString *error)
{
    if (error) {
        error->clear();
    }
    if (original.schemaVersion() != adjusted.schemaVersion()) {
        fail(error, QStringLiteral("the schema version changed"));
        return false;
    }
    if (original.sourceMediaId() != adjusted.sourceMediaId()) {
        fail(error, QStringLiteral("the source media identity changed"));
        return false;
    }
    const ReframePlan::TimeRange originalRange = original.sourceRange();
    const ReframePlan::TimeRange adjustedRange = adjusted.sourceRange();
    if (originalRange.startMs != adjustedRange.startMs
        || originalRange.endMs != adjustedRange.endMs) {
        fail(error, QStringLiteral("the source range changed"));
        return false;
    }
    const ReframePlan::OutputSpec originalOutput = original.output();
    const ReframePlan::OutputSpec adjustedOutput = adjusted.output();
    if (originalOutput.width != adjustedOutput.width
        || originalOutput.height != adjustedOutput.height
        || originalOutput.fps != adjustedOutput.fps) {
        fail(error, QStringLiteral("the output specification changed"));
        return false;
    }
    const QList<ReframePlan::TimeRange> originalSegments = original.segments();
    const QList<ReframePlan::TimeRange> adjustedSegments = adjusted.segments();
    if (originalSegments.size() != adjustedSegments.size()) {
        fail(error, QStringLiteral("the retained segments changed"));
        return false;
    }
    for (int i = 0; i < originalSegments.size(); ++i) {
        if (originalSegments.at(i).startMs != adjustedSegments.at(i).startMs
            || originalSegments.at(i).endMs != adjustedSegments.at(i).endMs) {
            fail(error, QStringLiteral("a retained segment changed"));
            return false;
        }
    }

    const QList<CameraKeyframe> originalFrames = original.keyframes();
    const QList<CameraKeyframe> adjustedFrames = adjusted.keyframes();
    if (originalFrames.size() != adjustedFrames.size()) {
        fail(error, QStringLiteral("the keyframe count changed"));
        return false;
    }
    for (int i = 0; i < originalFrames.size(); ++i) {
        const CameraKeyframe &before = originalFrames.at(i);
        const CameraKeyframe &after = adjustedFrames.at(i);
        // EXACT comparisons: a copied field must be identical, not merely close.
        if (before.timeMs != after.timeMs) {
            fail(error, QStringLiteral("keyframe %1 changed its timestamp").arg(i));
            return false;
        }
        if (before.yawDeg != after.yawDeg) {
            fail(error, QStringLiteral("keyframe %1 changed its yaw").arg(i));
            return false;
        }
        if (before.pitchDeg != after.pitchDeg) {
            fail(error, QStringLiteral("keyframe %1 changed its pitch").arg(i));
            return false;
        }
        if (before.rollDeg != after.rollDeg) {
            fail(error, QStringLiteral("keyframe %1 changed its roll").arg(i));
            return false;
        }
        if (before.interpolation != after.interpolation) {
            fail(error, QStringLiteral("keyframe %1 changed its interpolation").arg(i));
            return false;
        }
        if (!(after.fieldOfViewDeg > before.fieldOfViewDeg
              || qAbs(after.fieldOfViewDeg - before.fieldOfViewDeg) < 1e-12)) {
            fail(error, QStringLiteral("keyframe %1 NARROWED its field of view").arg(i));
            return false;
        }
        if (!reframe::isValidFieldOfViewDeg(after.fieldOfViewDeg)) {
            fail(error, QStringLiteral("keyframe %1 leaves the supported field of "
                                       "view range").arg(i));
            return false;
        }
    }

    QString planError;
    if (!adjusted.isValid(&planError)) {
        fail(error, QStringLiteral("the adjusted plan is not valid: %1")
                        .arg(planError.isEmpty() ? QStringLiteral("invalid plan")
                                                 : planError));
        return false;
    }
    return true;
}

#include "reframe/ReframeContract.h"

#include <QStringList>
#include <QtGlobal>

namespace {

// Explicit, locale-independent numbers so a violation detail is byte-identical
// wherever it is produced.
QString formatDouble(double value)
{
    return QString::number(value, 'g', 10);
}

QString rangeText(qint64 startMs, qint64 endMs)
{
    return QStringLiteral("[%1, %2] ms").arg(startMs).arg(endMs);
}

} // namespace

QString ContractReport::summary() const
{
    if (violations.isEmpty()) {
        return QString();
    }
    QStringList parts;
    for (const ContractViolation &violation : violations) {
        parts.append(QStringLiteral("%1: %2").arg(violation.ruleId,
                                                  violation.detail));
    }
    return QStringLiteral("The reframe plan does not honour the request (%1).")
        .arg(parts.join(QStringLiteral("; ")));
}

QString ReframeContract::outputFidelityRuleId()
{
    return QStringLiteral("IPC-1");
}

QString ReframeContract::timeRangeRuleId()
{
    return QStringLiteral("IPC-2");
}

QString ReframeContract::temporalMaterialisationRuleId()
{
    return QStringLiteral("IPC-3");
}

QString ReframeContract::fieldOfViewRuleId()
{
    return QStringLiteral("IPC-4");
}

ContractReport ReframeContract::check(const ReframeIntent &intent,
                                      const ReframePlan &plan)
{
    ContractReport report;

    // IPC-1 -- output specification fidelity.
    // NotApplicable when the intent carries no output requirement: the
    // caller-provided default is then intentionally used.
    if (intent.hasOutput) {
        const ReframePlan::OutputSpec actual = plan.output();
        if (actual.width != intent.outputWidth
            || actual.height != intent.outputHeight
            || actual.fps != intent.outputFps) {
            ContractViolation violation;
            violation.ruleId = outputFidelityRuleId();
            violation.detail =
                QStringLiteral("requested %1x%2 at %3 fps but the plan renders "
                               "%4x%5 at %6 fps")
                    .arg(intent.outputWidth)
                    .arg(intent.outputHeight)
                    .arg(formatDouble(intent.outputFps))
                    .arg(actual.width)
                    .arg(actual.height)
                    .arg(formatDouble(actual.fps));
            report.violations.append(violation);
        }
    }

    // IPC-2 -- requested time-range containment.
    // NotApplicable when the intent carries no time-range requirement.
    if (intent.hasTimeRange) {
        const ReframePlan::TimeRange actual = plan.sourceRange();
        if (intent.hasTemporalRequest) {
            // A temporal edit INTENTIONALLY widens the final source range (see
            // ReframeCommandRunner::applyTemporal), so the requirement is
            // containment, never equality.
            if (actual.startMs > intent.startMs || actual.endMs < intent.endMs) {
                ContractViolation violation;
                violation.ruleId = timeRangeRuleId();
                violation.detail =
                    QStringLiteral("requested source range %1 is not contained "
                                   "by the plan range %2")
                        .arg(rangeText(intent.startMs, intent.endMs))
                        .arg(rangeText(actual.startMs, actual.endMs));
                report.violations.append(violation);
            }
        } else if (actual.startMs != intent.startMs
                   || actual.endMs != intent.endMs) {
            ContractViolation violation;
            violation.ruleId = timeRangeRuleId();
            violation.detail =
                QStringLiteral("requested source range %1 but the plan uses %2")
                    .arg(rangeText(intent.startMs, intent.endMs))
                    .arg(rangeText(actual.startMs, actual.endMs));
            report.violations.append(violation);
        }
    }

    // IPC-3 -- temporal edit materialisation.
    // NotApplicable without a temporal request, and when the request already
    // carries an error: preparation refuses such a request before any plan is
    // built, so it cannot legitimately reach this checker.
    if (intent.hasTemporalRequest && intent.temporalError.isEmpty()
        && plan.segments().isEmpty()) {
        ContractViolation violation;
        violation.ruleId = temporalMaterialisationRuleId();
        violation.detail = QStringLiteral(
            "a temporal edit was requested and resolved, but the plan retains "
            "no source segment");
        report.violations.append(violation);
    }

    // IPC-4 -- requested field-of-view fidelity (Objective 29).
    // NotApplicable when the instruction asked for no framing: the builder
    // default is then intentionally used. Every requested lens must actually be
    // REACHED by the executable plan, because a plan that never gets there has
    // silently dropped a framing request. This is deliberately not "every
    // keyframe carries a requested value": a lens change legitimately starts
    // from the lens the camera already had.
    const QList<double> requestedFieldOfViews = intent.requestedFieldOfViews();
    for (double requested : requestedFieldOfViews) {
        bool reached = false;
        for (const CameraKeyframe &frame : plan.keyframes()) {
            if (qAbs(frame.fieldOfViewDeg - requested) < 1e-6) {
                reached = true;
                break;
            }
        }
        if (!reached) {
            ContractViolation violation;
            violation.ruleId = fieldOfViewRuleId();
            violation.detail =
                QStringLiteral("requested a %1 degree field of view but no "
                               "keyframe of the plan renders at that lens")
                    .arg(formatDouble(requested));
            report.violations.append(violation);
        }
    }

    return report;
}

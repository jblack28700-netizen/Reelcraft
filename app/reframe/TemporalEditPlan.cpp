#include "reframe/TemporalEditPlan.h"

#include <QJsonArray>
#include <QJsonValue>

#include <algorithm>

namespace {

void normalizeRanges(QList<TemporalRange> *ranges)
{
    std::stable_sort(ranges->begin(), ranges->end(),
                     [](const TemporalRange &a, const TemporalRange &b) {
                         if (a.startMs != b.startMs) {
                             return a.startMs < b.startMs;
                         }
                         return a.endMs < b.endMs;
                     });

    QList<TemporalRange> merged;
    for (const TemporalRange &range : *ranges) {
        if (!merged.isEmpty() && range.startMs <= merged.last().endMs) {
            // Overlapping or adjacent: extend.
            merged.last().endMs = qMax(merged.last().endMs, range.endMs);
        } else {
            merged.append(range);
        }
    }
    *ranges = merged;
}

} // namespace

TemporalEditPlan TemporalEditPlan::keep(const QList<TemporalRange> &ranges)
{
    TemporalEditPlan plan;
    plan.m_operation = Operation::Keep;
    plan.m_ranges = ranges;
    return plan;
}

TemporalEditPlan TemporalEditPlan::remove(const QList<TemporalRange> &ranges)
{
    TemporalEditPlan plan;
    plan.m_operation = Operation::Remove;
    plan.m_ranges = ranges;
    return plan;
}

TemporalEditPlan TemporalEditPlan::targetDuration(qint64 durationMs, qint64 startMs)
{
    TemporalEditPlan plan;
    plan.m_operation = Operation::TargetDuration;
    plan.m_targetDurationMs = durationMs;
    plan.m_targetStartMs = startMs;
    return plan;
}

QString TemporalEditPlan::operationToString(Operation operation)
{
    switch (operation) {
    case Operation::Keep:
        return QStringLiteral("keep");
    case Operation::Remove:
        return QStringLiteral("remove");
    case Operation::TargetDuration:
        return QStringLiteral("target-duration");
    case Operation::None:
    default:
        return QStringLiteral("none");
    }
}

TemporalEditPlan::Operation TemporalEditPlan::operationFromString(const QString &value)
{
    if (value == QStringLiteral("keep")) {
        return Operation::Keep;
    }
    if (value == QStringLiteral("remove")) {
        return Operation::Remove;
    }
    if (value == QStringLiteral("target-duration")) {
        return Operation::TargetDuration;
    }
    return Operation::None;
}

bool TemporalEditPlan::isValid(QString *error) const
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

    switch (m_operation) {
    case Operation::None:
        return true;
    case Operation::Keep:
    case Operation::Remove:
        if (m_ranges.isEmpty()) {
            return fail(QStringLiteral(
                "Temporal edit has no ranges."));
        }
        for (const TemporalRange &range : m_ranges) {
            if (!range.isValid()) {
                return fail(QStringLiteral(
                    "Temporal edit contains an invalid range."));
            }
        }
        return true;
    case Operation::TargetDuration:
        if (m_targetDurationMs <= 0) {
            return fail(QStringLiteral(
                "Temporal target duration must be positive."));
        }
        if (m_targetStartMs < -1) {
            return fail(QStringLiteral(
                "Temporal target start is invalid."));
        }
        return true;
    default:
        return fail(QStringLiteral("Temporal edit operation is unsupported."));
    }
}

bool TemporalEditPlan::normalize(QString *error)
{
    if (error) {
        error->clear();
    }
    if (!isValid(error)) {
        return false;
    }
    if (m_operation == Operation::Keep || m_operation == Operation::Remove) {
        normalizeRanges(&m_ranges);
    }
    return true;
}

QList<TemporalRange> TemporalEditPlan::resolve(qint64 durationMs,
                                               qint64 defaultStartMs,
                                               QString *error) const
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return QList<TemporalRange>();
    };

    if (!isValid(error)) {
        return QList<TemporalRange>();
    }
    if (m_operation == Operation::None) {
        return fail(QStringLiteral("Temporal edit is not specified."));
    }
    if (durationMs <= 0) {
        return fail(QStringLiteral(
            "The source duration is required to resolve a temporal edit."));
    }

    if (m_operation == Operation::TargetDuration) {
        const qint64 startMs = m_targetStartMs >= 0 ? m_targetStartMs
                                                    : defaultStartMs;
        const qint64 endMs = startMs + m_targetDurationMs;
        if (startMs < 0 || endMs > durationMs) {
            return fail(QStringLiteral(
                "The requested duration is out of bounds for this source."));
        }
        return { TemporalRange{ startMs, endMs } };
    }

    QList<TemporalRange> normalized = m_ranges;
    normalizeRanges(&normalized);
    for (const TemporalRange &range : normalized) {
        if (range.startMs < 0 || range.endMs > durationMs) {
            return fail(QStringLiteral(
                "A temporal range is out of bounds for this source."));
        }
    }

    if (m_operation == Operation::Keep) {
        return normalized; // already validated and ordered
    }

    // Remove: retain the complement within [0, durationMs].
    QList<TemporalRange> kept;
    qint64 cursor = 0;
    for (const TemporalRange &range : normalized) {
        if (range.startMs > cursor) {
            kept.append(TemporalRange{ cursor, range.startMs });
        }
        cursor = qMax(cursor, range.endMs);
    }
    if (cursor < durationMs) {
        kept.append(TemporalRange{ cursor, durationMs });
    }
    if (kept.isEmpty()) {
        return fail(QStringLiteral(
            "The removal leaves no content to render."));
    }
    return kept;
}

QJsonObject TemporalEditPlan::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("operation"), operationToString(m_operation));
    if (m_operation == Operation::Keep || m_operation == Operation::Remove) {
        QJsonArray ranges;
        for (const TemporalRange &range : m_ranges) {
            QJsonObject entry;
            entry.insert(QStringLiteral("startMs"),
                         static_cast<double>(range.startMs));
            entry.insert(QStringLiteral("endMs"),
                         static_cast<double>(range.endMs));
            ranges.append(entry);
        }
        object.insert(QStringLiteral("ranges"), ranges);
    }
    if (m_operation == Operation::TargetDuration) {
        object.insert(QStringLiteral("targetDurationMs"),
                      static_cast<double>(m_targetDurationMs));
        object.insert(QStringLiteral("targetStartMs"),
                      static_cast<double>(m_targetStartMs));
    }
    return object;
}

bool TemporalEditPlan::readFromJsonObject(const QJsonObject &object,
                                          TemporalEditPlan *out,
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
    if (!out) {
        return fail(QStringLiteral("Temporal edit output is null."));
    }

    TemporalEditPlan plan;
    plan.m_operation = operationFromString(
        object.value(QStringLiteral("operation")).toString());
    if (plan.m_operation == Operation::Keep
        || plan.m_operation == Operation::Remove) {
        const QJsonValue rangesValue = object.value(QStringLiteral("ranges"));
        if (!rangesValue.isArray()) {
            return fail(QStringLiteral("Temporal edit is missing its ranges."));
        }
        for (const QJsonValue &value : rangesValue.toArray()) {
            if (!value.isObject()) {
                return fail(QStringLiteral(
                    "Temporal edit range entry is not an object."));
            }
            const QJsonObject entry = value.toObject();
            const QJsonValue startValue = entry.value(QStringLiteral("startMs"));
            const QJsonValue endValue = entry.value(QStringLiteral("endMs"));
            if (!startValue.isDouble() || !endValue.isDouble()) {
                return fail(QStringLiteral(
                    "Temporal edit range is missing numeric bounds."));
            }
            TemporalRange range;
            range.startMs = static_cast<qint64>(startValue.toDouble());
            range.endMs = static_cast<qint64>(endValue.toDouble());
            plan.m_ranges.append(range);
        }
    } else if (plan.m_operation == Operation::TargetDuration) {
        plan.m_targetDurationMs = static_cast<qint64>(
            object.value(QStringLiteral("targetDurationMs")).toDouble());
        plan.m_targetStartMs = static_cast<qint64>(
            object.value(QStringLiteral("targetStartMs")).toDouble(-1));
    }

    if (!plan.isValid(error)) {
        return false;
    }
    *out = plan;
    return true;
}

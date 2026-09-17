#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

// 360 Reframing Objective 14 — deterministic temporal editing representation.
//
// TemporalRange is a half-open source-time interval [startMs, endMs).
struct TemporalRange
{
    qint64 startMs = 0;
    qint64 endMs = 0;

    bool isValid() const { return startMs >= 0 && endMs > startMs; }
    qint64 durationMs() const { return endMs - startMs; }
    bool operator==(const TemporalRange &other) const
    {
        return startMs == other.startMs && endMs == other.endMs;
    }
};

// TemporalEditPlan is the deterministic, structured representation of a temporal
// editing decision (retain / remove / target-duration). It is deliberately
// INDEPENDENT of natural-language parsing and of the renderer: a command parser,
// a UI, or a future timeline editor can all produce the same value, and it can
// be executed by the existing deterministic pipeline.
//
// Operations:
//   Keep            — the listed source ranges are retained (in order).
//   Remove          — the listed source ranges are removed; the complement
//                     within the source duration is retained (needs a duration).
//   TargetDuration  — retain a window of targetDurationMs starting at
//                     targetStartMs (or the supplied default start when -1).
class TemporalEditPlan
{
public:
    enum class Operation { None, Keep, Remove, TargetDuration };

    TemporalEditPlan() = default;

    static TemporalEditPlan keep(const QList<TemporalRange> &ranges);
    static TemporalEditPlan remove(const QList<TemporalRange> &ranges);
    static TemporalEditPlan targetDuration(qint64 durationMs, qint64 startMs = -1);

    Operation operation() const { return m_operation; }
    const QList<TemporalRange> &ranges() const { return m_ranges; }
    qint64 targetDurationMs() const { return m_targetDurationMs; }
    qint64 targetStartMs() const { return m_targetStartMs; }

    bool isSpecified() const { return m_operation != Operation::None; }

    // Validates the operation and its raw input (well-formed ranges, positive
    // target duration, at least one range). Returns false with a reason.
    bool isValid(QString *error = nullptr) const;

    // Deterministically normalizes Keep/Remove ranges (sort by start, merge
    // overlapping and adjacent intervals). No-op for TargetDuration. Returns
    // false with a reason when the result is invalid.
    bool normalize(QString *error = nullptr);

    // Resolves this operation to the ordered retained source ranges within
    // [0, durationMs]. defaultStartMs anchors a TargetDuration with no explicit
    // start. Fails honestly on out-of-bounds ranges, empty results, or
    // contradictions (for example a removal that leaves no content).
    QList<TemporalRange> resolve(qint64 durationMs, qint64 defaultStartMs = 0,
                                 QString *error = nullptr) const;

    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object,
                                   TemporalEditPlan *out,
                                   QString *error = nullptr);

    static QString operationToString(Operation operation);
    static Operation operationFromString(const QString &value);

private:
    Operation m_operation = Operation::None;
    QList<TemporalRange> m_ranges;
    qint64 m_targetDurationMs = 0;
    qint64 m_targetStartMs = -1;
};

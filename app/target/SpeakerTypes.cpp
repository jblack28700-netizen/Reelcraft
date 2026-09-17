#include "target/SpeakerTypes.h"

#include <QJsonArray>
#include <QJsonValue>

#include <cmath>

namespace {

bool finiteConfidence(double value)
{
    return std::isfinite(value) && value >= 0.0 && value <= 1.0;
}

} // namespace

bool SpeakerInterval::isValid(QString *error) const
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
    if (speakerId.isEmpty()) {
        return fail(QStringLiteral("Speaker interval has no speaker id."));
    }
    if (startMs < 0 || endMs <= startMs) {
        return fail(QStringLiteral("Speaker interval has an invalid time range."));
    }
    if (!finiteConfidence(confidence)) {
        return fail(QStringLiteral("Speaker interval confidence is out of range."));
    }
    if (hasAzimuth && !std::isfinite(azimuthDeg)) {
        return fail(QStringLiteral("Speaker interval azimuth is not finite."));
    }
    return true;
}

QJsonObject SpeakerInterval::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("startMs"), static_cast<double>(startMs));
    object.insert(QStringLiteral("endMs"), static_cast<double>(endMs));
    object.insert(QStringLiteral("speakerId"), speakerId);
    object.insert(QStringLiteral("confidence"), confidence);
    object.insert(QStringLiteral("overlap"), overlap);
    if (hasAzimuth) {
        object.insert(QStringLiteral("azimuthDeg"), azimuthDeg);
    }
    if (!targetIdHint.isEmpty()) {
        object.insert(QStringLiteral("targetIdHint"), targetIdHint);
    }
    return object;
}

bool SpeakerInterval::readFromJsonObject(const QJsonObject &object,
                                         SpeakerInterval *out, QString *error)
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
        return fail(QStringLiteral("Speaker interval output is null."));
    }
    SpeakerInterval interval;
    const QJsonValue startValue = object.value(QStringLiteral("startMs"));
    const QJsonValue endValue = object.value(QStringLiteral("endMs"));
    const QJsonValue confidenceValue = object.value(QStringLiteral("confidence"));
    if (!startValue.isDouble() || !endValue.isDouble()
        || !confidenceValue.isDouble()) {
        return fail(QStringLiteral("Speaker interval is missing numeric fields."));
    }
    interval.startMs = static_cast<qint64>(startValue.toDouble());
    interval.endMs = static_cast<qint64>(endValue.toDouble());
    interval.speakerId = object.value(QStringLiteral("speakerId")).toString();
    interval.confidence = confidenceValue.toDouble();
    interval.overlap = object.value(QStringLiteral("overlap")).toBool(false);
    const QJsonValue azimuthValue = object.value(QStringLiteral("azimuthDeg"));
    if (azimuthValue.isDouble()) {
        interval.azimuthDeg = azimuthValue.toDouble();
        interval.hasAzimuth = true;
    }
    interval.targetIdHint = object.value(QStringLiteral("targetIdHint")).toString();
    if (!interval.isValid(error)) {
        return false;
    }
    *out = interval;
    return true;
}

bool SpeakerAnalysis::isValid(QString *error) const
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
    if (!available) {
        return true; // unavailable analyses are valid data; intervals must be empty
    }
    if (endMs <= startMs) {
        return fail(QStringLiteral("Speaker analysis has an invalid time range."));
    }
    for (const SpeakerInterval &interval : intervals) {
        QString intervalError;
        if (!interval.isValid(&intervalError)) {
            if (error) {
                *error = intervalError;
            }
            return false;
        }
    }
    return true;
}

QJsonObject SpeakerAnalysis::toJsonObject() const
{
    QJsonArray array;
    for (const SpeakerInterval &interval : intervals) {
        array.append(interval.toJsonObject());
    }
    QJsonObject object;
    object.insert(QStringLiteral("available"), available);
    object.insert(QStringLiteral("provider"), provider);
    object.insert(QStringLiteral("startMs"), static_cast<double>(startMs));
    object.insert(QStringLiteral("endMs"), static_cast<double>(endMs));
    object.insert(QStringLiteral("intervals"), array);
    if (!error.isEmpty()) {
        object.insert(QStringLiteral("error"), error);
    }
    return object;
}

bool SpeakerAnalysis::readFromJsonObject(const QJsonObject &object,
                                         SpeakerAnalysis *out, QString *error)
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
        return fail(QStringLiteral("Speaker analysis output is null."));
    }
    const QJsonValue availableValue = object.value(QStringLiteral("available"));
    if (!availableValue.isBool()) {
        return fail(QStringLiteral("Speaker analysis is missing its available flag."));
    }
    SpeakerAnalysis analysis;
    analysis.available = availableValue.toBool();
    analysis.provider = object.value(QStringLiteral("provider")).toString();
    analysis.startMs = static_cast<qint64>(
        object.value(QStringLiteral("startMs")).toDouble());
    analysis.endMs = static_cast<qint64>(
        object.value(QStringLiteral("endMs")).toDouble());
    analysis.error = object.value(QStringLiteral("error")).toString();
    const QJsonValue intervalsValue = object.value(QStringLiteral("intervals"));
    if (intervalsValue.isArray()) {
        for (const QJsonValue &value : intervalsValue.toArray()) {
            if (!value.isObject()) {
                return fail(QStringLiteral("Speaker interval entry is not an object."));
            }
            SpeakerInterval interval;
            QString intervalError;
            if (!SpeakerInterval::readFromJsonObject(value.toObject(), &interval,
                                                     &intervalError)) {
                return fail(intervalError);
            }
            analysis.intervals.append(interval);
        }
    }
    if (!analysis.isValid(error)) {
        return false;
    }
    *out = analysis;
    return true;
}

QString speakerVerdictToString(SpeakerVerdict verdict)
{
    switch (verdict) {
    case SpeakerVerdict::Active:
        return QStringLiteral("active");
    case SpeakerVerdict::Ambiguous:
        return QStringLiteral("ambiguous");
    case SpeakerVerdict::Overlap:
        return QStringLiteral("overlap");
    case SpeakerVerdict::Silence:
        return QStringLiteral("silence");
    case SpeakerVerdict::Unassociated:
        return QStringLiteral("unassociated");
    case SpeakerVerdict::Unavailable:
    default:
        return QStringLiteral("unavailable");
    }
}

SpeakerVerdict speakerVerdictFromString(const QString &value)
{
    if (value == QStringLiteral("active")) {
        return SpeakerVerdict::Active;
    }
    if (value == QStringLiteral("ambiguous")) {
        return SpeakerVerdict::Ambiguous;
    }
    if (value == QStringLiteral("overlap")) {
        return SpeakerVerdict::Overlap;
    }
    if (value == QStringLiteral("silence")) {
        return SpeakerVerdict::Silence;
    }
    if (value == QStringLiteral("unassociated")) {
        return SpeakerVerdict::Unassociated;
    }
    return SpeakerVerdict::Unavailable;
}

QJsonObject SpeakerEvidence::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("targetId"), targetId);
    object.insert(QStringLiteral("speakerId"), speakerId);
    object.insert(QStringLiteral("verdict"), speakerVerdictToString(verdict));
    object.insert(QStringLiteral("confidence"), confidence);
    object.insert(QStringLiteral("startMs"), static_cast<double>(startMs));
    object.insert(QStringLiteral("endMs"), static_cast<double>(endMs));
    object.insert(QStringLiteral("provider"), provider);
    object.insert(QStringLiteral("detail"), detail);
    return object;
}

bool SpeakerEvidence::readFromJsonObject(const QJsonObject &object,
                                         SpeakerEvidence *out, QString *error)
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
        return fail(QStringLiteral("Speaker evidence output is null."));
    }
    SpeakerEvidence evidence;
    evidence.targetId = object.value(QStringLiteral("targetId")).toString();
    evidence.speakerId = object.value(QStringLiteral("speakerId")).toString();
    evidence.verdict = speakerVerdictFromString(
        object.value(QStringLiteral("verdict")).toString());
    evidence.confidence = object.value(QStringLiteral("confidence")).toDouble(0.0);
    evidence.startMs = static_cast<qint64>(
        object.value(QStringLiteral("startMs")).toDouble());
    evidence.endMs = static_cast<qint64>(
        object.value(QStringLiteral("endMs")).toDouble());
    evidence.provider = object.value(QStringLiteral("provider")).toString();
    evidence.detail = object.value(QStringLiteral("detail")).toString();
    *out = evidence;
    return true;
}

bool SpeakerSegment::isValid(QString *error) const
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
    if (startMs < 0 || endMs < startMs) {
        return fail(QStringLiteral("Speaker segment has an invalid time range."));
    }
    if (!std::isfinite(confidence) || confidence < 0.0 || confidence > 1.0) {
        return fail(QStringLiteral("Speaker segment confidence is out of range."));
    }
    if (verdict == SpeakerVerdict::Active && speakerId.isEmpty()) {
        return fail(QStringLiteral("Active speaker segment has no speaker id."));
    }
    return true;
}

QJsonObject SpeakerSegment::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("startMs"), static_cast<double>(startMs));
    object.insert(QStringLiteral("endMs"), static_cast<double>(endMs));
    object.insert(QStringLiteral("speakerId"), speakerId);
    object.insert(QStringLiteral("targetId"), targetId);
    object.insert(QStringLiteral("verdict"), speakerVerdictToString(verdict));
    object.insert(QStringLiteral("confidence"), confidence);
    if (!targetIdHint.isEmpty()) {
        object.insert(QStringLiteral("targetIdHint"), targetIdHint);
    }
    return object;
}

bool SpeakerSegment::readFromJsonObject(const QJsonObject &object,
                                        SpeakerSegment *out, QString *error)
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
        return fail(QStringLiteral("Speaker segment output is null."));
    }
    SpeakerSegment segment;
    segment.startMs = static_cast<qint64>(
        object.value(QStringLiteral("startMs")).toDouble());
    segment.endMs = static_cast<qint64>(
        object.value(QStringLiteral("endMs")).toDouble());
    segment.speakerId = object.value(QStringLiteral("speakerId")).toString();
    segment.targetId = object.value(QStringLiteral("targetId")).toString();
    segment.verdict = speakerVerdictFromString(
        object.value(QStringLiteral("verdict")).toString());
    segment.confidence = object.value(QStringLiteral("confidence")).toDouble(0.0);
    segment.targetIdHint = object.value(QStringLiteral("targetIdHint")).toString();
    if (!segment.isValid(error)) {
        return false;
    }
    *out = segment;
    return true;
}

#include "ReframePlan.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>

#include "reframe/ReframeMath.h"

#include <cmath>

namespace {

constexpr int kMaxDimension = 16384;
constexpr double kMaxFps = 240.0;

// Deterministic number of output frames for one source range at the given fps.
int framesForRange(const ReframePlan::TimeRange &range, double fps)
{
    if (!range.isValid() || !std::isfinite(fps) || fps <= 0.0) {
        return 0;
    }
    const double count =
        std::floor(static_cast<double>(range.durationMs()) * fps / 1000.0 + 1e-9);
    if (!std::isfinite(count) || count < 0.0 || count > 1.0e9) {
        return 0;
    }
    return static_cast<int>(count);
}

} // namespace

bool ReframePlan::OutputSpec::isValid() const
{
    return width > 0 && width <= kMaxDimension && height > 0
        && height <= kMaxDimension && std::isfinite(fps) && fps > 0.0
        && fps <= kMaxFps;
}

void ReframePlan::addKeyframe(const CameraKeyframe &keyframe)
{
    int index = 0;
    while (index < m_keyframes.size()
           && m_keyframes.at(index).timeMs <= keyframe.timeMs) {
        ++index;
    }
    m_keyframes.insert(index, keyframe);
}

int ReframePlan::frameCount() const
{
    if (!m_output.isValid()) {
        return 0;
    }
    if (m_segments.isEmpty()) {
        return framesForRange(m_sourceRange, m_output.fps);
    }
    int total = 0;
    for (const TimeRange &segment : m_segments) {
        const int frames = framesForRange(segment, m_output.fps);
        if (frames <= 0 || total > 1.0e9 - frames) {
            return 0;
        }
        total += frames;
    }
    return total;
}

qint64 ReframePlan::frameTimeMs(int index) const
{
    if (m_segments.isEmpty()) {
        if (index < 0) {
            return m_sourceRange.startMs;
        }
        return m_sourceRange.startMs
            + static_cast<qint64>(
                std::llround(static_cast<double>(index) * 1000.0 / m_output.fps));
    }

    int remaining = qMax(0, index);
    for (const TimeRange &segment : m_segments) {
        const int frames = framesForRange(segment, m_output.fps);
        if (remaining < frames) {
            return segment.startMs
                + static_cast<qint64>(std::llround(
                    static_cast<double>(remaining) * 1000.0 / m_output.fps));
        }
        remaining -= frames;
    }
    // Beyond the end: the last frame of the last segment.
    const TimeRange &last = m_segments.last();
    const int lastFrames = qMax(1, framesForRange(last, m_output.fps));
    return last.startMs
        + static_cast<qint64>(std::llround(
            static_cast<double>(lastFrames - 1) * 1000.0 / m_output.fps));
}

bool ReframePlan::isValid(QString *error) const
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

    if (m_schemaVersion <= 0 || m_schemaVersion > CurrentSchemaVersion) {
        return fail(QStringLiteral("Reframe plan schema version is unsupported."));
    }
    if (!m_sourceRange.isValid()) {
        return fail(QStringLiteral("Reframe plan source range is invalid."));
    }
    if (!m_output.isValid()) {
        return fail(QStringLiteral("Reframe plan output specification is invalid."));
    }
    if (m_keyframes.isEmpty()) {
        return fail(QStringLiteral("Reframe plan has no camera keyframes."));
    }

    qint64 previousTime = -1;
    for (const CameraKeyframe &frame : m_keyframes) {
        if (frame.timeMs < m_sourceRange.startMs
            || frame.timeMs > m_sourceRange.endMs) {
            return fail(QStringLiteral(
                "Reframe plan keyframe time is outside the source range."));
        }
        if (previousTime >= 0 && frame.timeMs <= previousTime) {
            return fail(QStringLiteral(
                "Reframe plan keyframes must be strictly increasing in time."));
        }
        previousTime = frame.timeMs;

        if (!reframe::isFinite(frame.yawDeg) || !reframe::isFinite(frame.rollDeg)) {
            return fail(QStringLiteral(
                "Reframe plan keyframe yaw/roll must be finite."));
        }
        if (!reframe::isValidPitchDeg(frame.pitchDeg)) {
            return fail(QStringLiteral(
                "Reframe plan keyframe pitch must be within [-90, 90]."));
        }
        if (!reframe::isValidFieldOfViewDeg(frame.fieldOfViewDeg)) {
            return fail(QStringLiteral(
                "Reframe plan keyframe field of view must be within [20, 140]."));
        }
    }

    // Objective 14: optional ordered retained ranges.
    if (!m_segments.isEmpty()) {
        qint64 previousEnd = m_sourceRange.startMs;
        for (const TimeRange &segment : m_segments) {
            if (!segment.isValid()
                || segment.startMs < m_sourceRange.startMs
                || segment.endMs > m_sourceRange.endMs) {
                return fail(QStringLiteral(
                    "Reframe plan segment is invalid or outside the source range."));
            }
            if (segment.startMs < previousEnd) {
                return fail(QStringLiteral(
                    "Reframe plan segments must be ordered and non-overlapping."));
            }
            previousEnd = segment.endMs;
        }
    }

    if (frameCount() < 1) {
        return fail(QStringLiteral(
            "Reframe plan range is shorter than one output frame."));
    }

    return true;
}

QJsonObject ReframePlan::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("schemaVersion"), m_schemaVersion);
    if (!m_sourceMediaId.isEmpty()) {
        object.insert(QStringLiteral("sourceMediaId"), m_sourceMediaId);
    }

    QJsonObject range;
    range.insert(QStringLiteral("startMs"),
                 static_cast<double>(m_sourceRange.startMs));
    range.insert(QStringLiteral("endMs"),
                 static_cast<double>(m_sourceRange.endMs));
    object.insert(QStringLiteral("sourceRange"), range);

    QJsonObject output;
    output.insert(QStringLiteral("width"), m_output.width);
    output.insert(QStringLiteral("height"), m_output.height);
    output.insert(QStringLiteral("fps"), m_output.fps);
    object.insert(QStringLiteral("output"), output);

    QJsonArray keyframes;
    for (const CameraKeyframe &frame : m_keyframes) {
        keyframes.append(frame.toJsonObject());
    }
    object.insert(QStringLiteral("keyframes"), keyframes);

    if (!m_segments.isEmpty()) {
        QJsonArray segments;
        for (const TimeRange &segment : m_segments) {
            QJsonObject entry;
            entry.insert(QStringLiteral("startMs"),
                         static_cast<double>(segment.startMs));
            entry.insert(QStringLiteral("endMs"),
                         static_cast<double>(segment.endMs));
            segments.append(entry);
        }
        object.insert(QStringLiteral("segments"), segments);
    }

    return object;
}

bool ReframePlan::readFromJsonObject(const QJsonObject &object, ReframePlan *out,
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
        return fail(QStringLiteral("Reframe plan output is null."));
    }

    ReframePlan plan;

    const QJsonValue schemaValue = object.value(QStringLiteral("schemaVersion"));
    if (!schemaValue.isDouble()) {
        return fail(QStringLiteral("Reframe plan is missing schemaVersion."));
    }
    plan.m_schemaVersion = schemaValue.toInt();
    if (plan.m_schemaVersion <= 0
        || plan.m_schemaVersion > CurrentSchemaVersion) {
        return fail(QStringLiteral("Reframe plan schema version is unsupported."));
    }

    plan.m_sourceMediaId =
        object.value(QStringLiteral("sourceMediaId")).toString();

    const QJsonValue rangeValue = object.value(QStringLiteral("sourceRange"));
    if (!rangeValue.isObject()) {
        return fail(QStringLiteral("Reframe plan is missing sourceRange."));
    }
    const QJsonObject rangeObject = rangeValue.toObject();
    const QJsonValue startValue = rangeObject.value(QStringLiteral("startMs"));
    const QJsonValue endValue = rangeObject.value(QStringLiteral("endMs"));
    if (!startValue.isDouble() || !endValue.isDouble()) {
        return fail(QStringLiteral(
            "Reframe plan sourceRange is missing numeric bounds."));
    }
    plan.m_sourceRange.startMs = static_cast<qint64>(startValue.toDouble());
    plan.m_sourceRange.endMs = static_cast<qint64>(endValue.toDouble());

    const QJsonValue outputValue = object.value(QStringLiteral("output"));
    if (!outputValue.isObject()) {
        return fail(QStringLiteral("Reframe plan is missing output."));
    }
    const QJsonObject outputObject = outputValue.toObject();
    const QJsonValue widthValue = outputObject.value(QStringLiteral("width"));
    const QJsonValue heightValue = outputObject.value(QStringLiteral("height"));
    const QJsonValue fpsValue = outputObject.value(QStringLiteral("fps"));
    if (!widthValue.isDouble() || !heightValue.isDouble() || !fpsValue.isDouble()) {
        return fail(QStringLiteral(
            "Reframe plan output is missing numeric fields."));
    }
    plan.m_output.width = widthValue.toInt();
    plan.m_output.height = heightValue.toInt();
    plan.m_output.fps = fpsValue.toDouble();

    const QJsonValue keyframesValue = object.value(QStringLiteral("keyframes"));
    if (!keyframesValue.isArray()) {
        return fail(QStringLiteral("Reframe plan is missing keyframes."));
    }
    const QJsonArray keyframes = keyframesValue.toArray();
    for (const QJsonValue &value : keyframes) {
        if (!value.isObject()) {
            return fail(QStringLiteral(
                "Reframe plan keyframe entry is not an object."));
        }
        CameraKeyframe frame;
        QString frameError;
        if (!CameraKeyframe::readFromJsonObject(value.toObject(), &frame,
                                                &frameError)) {
            return fail(frameError);
        }
        plan.m_keyframes.append(frame);
    }

    const QJsonValue segmentsValue = object.value(QStringLiteral("segments"));
    if (segmentsValue.isArray()) {
        for (const QJsonValue &value : segmentsValue.toArray()) {
            if (!value.isObject()) {
                return fail(QStringLiteral(
                    "Reframe plan segment entry is not an object."));
            }
            const QJsonObject entry = value.toObject();
            const QJsonValue startValue = entry.value(QStringLiteral("startMs"));
            const QJsonValue endValue = entry.value(QStringLiteral("endMs"));
            if (!startValue.isDouble() || !endValue.isDouble()) {
                return fail(QStringLiteral(
                    "Reframe plan segment is missing numeric bounds."));
            }
            plan.m_segments.append(TimeRange{
                static_cast<qint64>(startValue.toDouble()),
                static_cast<qint64>(endValue.toDouble()) });
        }
    }

    QString validationError;
    if (!plan.isValid(&validationError)) {
        return fail(validationError);
    }

    *out = plan;
    return true;
}

bool ReframePlan::save(const QString &filePath, QString *error) const
{
    if (error) {
        error->clear();
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    const QByteArray data =
        QJsonDocument(toJsonObject()).toJson(QJsonDocument::Indented);
    if (file.write(data) < 0) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    return true;
}

ReframePlan ReframePlan::load(const QString &filePath, bool *ok, QString *error)
{
    if (ok) {
        *ok = false;
    }
    if (error) {
        error->clear();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = file.errorString();
        }
        return ReframePlan();
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = parseError.errorString();
        }
        return ReframePlan();
    }

    ReframePlan plan;
    if (!readFromJsonObject(document.object(), &plan, error)) {
        return ReframePlan();
    }
    if (ok) {
        *ok = true;
    }
    return plan;
}

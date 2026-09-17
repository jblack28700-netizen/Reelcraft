#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

#include "reframe/CameraKeyframe.h"

// ReframePlan is the structured, deterministic, inspectable representation of
// a 360 -> flat reframing decision. It is the boundary between "what should
// happen" (an AI decision or a creator instruction) and "how it is executed"
// (the deterministic renderer).
//
// The plan never references decoded pixels and never modifies the source
// media. It records:
//   - the source media id the decision applies to;
//   - the source time range to use;
//   - the flat output specification (width/height/fps);
//   - an ordered list of virtual-camera keyframes.
//
// The same plan always produces the same output frames for the same source,
// which makes the pipeline testable and reproducible.
class ReframePlan
{
public:
    static constexpr int CurrentSchemaVersion = 1;

    struct TimeRange
    {
        qint64 startMs = 0;
        qint64 endMs = 0;

        bool isValid() const { return endMs > startMs && startMs >= 0; }
        qint64 durationMs() const { return endMs - startMs; }
    };

    struct OutputSpec
    {
        int width = 1920;
        int height = 1080;
        double fps = 30.0;

        bool isValid() const;
    };

    ReframePlan() = default;

    QString sourceMediaId() const { return m_sourceMediaId; }
    void setSourceMediaId(const QString &sourceMediaId) { m_sourceMediaId = sourceMediaId; }

    TimeRange sourceRange() const { return m_sourceRange; }
    void setSourceRange(const TimeRange &sourceRange) { m_sourceRange = sourceRange; }

    OutputSpec output() const { return m_output; }
    void setOutput(const OutputSpec &output) { m_output = output; }

    QList<CameraKeyframe> keyframes() const { return m_keyframes; }
    void setKeyframes(const QList<CameraKeyframe> &keyframes) { m_keyframes = keyframes; }

    // Inserts a keyframe keeping the list ordered by timeMs (stable for equal
    // times; validity rejects duplicates).
    void addKeyframe(const CameraKeyframe &keyframe);

    int schemaVersion() const { return m_schemaVersion; }

    // Deterministic output frame count for the range at the requested fps.
    // Zero when the range/output are invalid.
    int frameCount() const;

    // Deterministic source timestamp of the given output frame index.
    qint64 frameTimeMs(int index) const;

    // Validates the plan: schema, range, output spec, at least one keyframe,
    // strictly increasing in-range keyframe times, and bounded camera values.
    bool isValid(QString *error = nullptr) const;

    QJsonObject toJsonObject() const;
    static bool readFromJsonObject(const QJsonObject &object, ReframePlan *out,
                                   QString *error = nullptr);

    bool save(const QString &filePath, QString *error = nullptr) const;
    static ReframePlan load(const QString &filePath, bool *ok = nullptr,
                            QString *error = nullptr);

private:
    QString m_sourceMediaId;
    TimeRange m_sourceRange;
    OutputSpec m_output;
    QList<CameraKeyframe> m_keyframes;
    int m_schemaVersion = CurrentSchemaVersion;
};

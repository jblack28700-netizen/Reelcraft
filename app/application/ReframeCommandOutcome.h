#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMetaType>
#include <QPair>
#include <QString>
#include <QStringList>

#include "reframe/ReframeIntent.h" // ReframeTarget

// ReframeCommandOutcome is the application-visible, structured result of one 360
// reframe command (Objective 9). It records the source reference, the
// instruction, the effective time range, the output specification, the generated
// output path, the success/failure state, and structured error information, so
// the application/UI can present and log a command without reaching into the
// deterministic engine or the perception layers.
//
// It is a plain QtCore value type (headless-testable). Generated renders are
// session state for now and are NOT persisted in the project schema (see
// docs/DECISIONS.md Decision 026).
struct ReframeCommandOutcome
{
    bool ok = false;
    QString instruction;
    QString sourceMediaId;
    QString sourcePath;
    QString outputPath;
    qint64 startMs = 0;
    qint64 endMs = 0;
    int outputWidth = 0;
    int outputHeight = 0;
    double outputFps = 0.0;
    int frameCount = 0;
    QString error;
    QStringList notes;
    QStringList unresolvedReferences;
    QList<ReframeTarget> resolvedTargets;
    // Objective 14: ordered retained source ranges of a temporal edit. Empty
    // means the render used the single [startMs, endMs] range.
    QList<QPair<qint64, qint64>> temporalSegments;

    bool hasRange() const { return endMs > startMs && startMs >= 0; }
    bool hasTemporalSegments() const { return !temporalSegments.isEmpty(); }

    QJsonObject toJsonObject() const
    {
        QJsonObject object;
        object.insert(QStringLiteral("ok"), ok);
        object.insert(QStringLiteral("instruction"), instruction);
        object.insert(QStringLiteral("sourceMediaId"), sourceMediaId);
        object.insert(QStringLiteral("sourcePath"), sourcePath);
        object.insert(QStringLiteral("outputPath"), outputPath);
        object.insert(QStringLiteral("startMs"), static_cast<double>(startMs));
        object.insert(QStringLiteral("endMs"), static_cast<double>(endMs));
        object.insert(QStringLiteral("outputWidth"), outputWidth);
        object.insert(QStringLiteral("outputHeight"), outputHeight);
        object.insert(QStringLiteral("outputFps"), outputFps);
        object.insert(QStringLiteral("frameCount"), frameCount);
        if (!error.isEmpty()) {
            object.insert(QStringLiteral("error"), error);
        }
        QJsonArray notesArray;
        for (const QString &note : notes) {
            notesArray.append(note);
        }
        object.insert(QStringLiteral("notes"), notesArray);
        QJsonArray unresolvedArray;
        for (const QString &reference : unresolvedReferences) {
            unresolvedArray.append(reference);
        }
        object.insert(QStringLiteral("unresolvedReferences"), unresolvedArray);
        QJsonArray targetArray;
        for (const ReframeTarget &target : resolvedTargets) {
            QJsonObject targetObject;
            targetObject.insert(QStringLiteral("id"), target.id);
            targetObject.insert(QStringLiteral("yawDeg"), target.yawDeg);
            targetObject.insert(QStringLiteral("pitchDeg"), target.pitchDeg);
            targetArray.append(targetObject);
        }
        object.insert(QStringLiteral("resolvedTargets"), targetArray);
        if (!temporalSegments.isEmpty()) {
            QJsonArray segmentArray;
            for (const QPair<qint64, qint64> &segment : temporalSegments) {
                QJsonObject entry;
                entry.insert(QStringLiteral("startMs"),
                             static_cast<double>(segment.first));
                entry.insert(QStringLiteral("endMs"),
                             static_cast<double>(segment.second));
                segmentArray.append(entry);
            }
            object.insert(QStringLiteral("temporalSegments"), segmentArray);
        }
        return object;
    }

    // Restores one record. Lenient about optional fields, but a record must
    // identify where the result was written (a non-empty output path), so
    // malformed entries can be skipped deterministically by callers.
    static bool readFromJsonObject(const QJsonObject &object,
                                   ReframeCommandOutcome *out,
                                   QString *error = nullptr)
    {
        if (error) {
            error->clear();
        }
        if (!out) {
            if (error) {
                *error = QStringLiteral("Reframe command outcome output is null.");
            }
            return false;
        }
        ReframeCommandOutcome outcome;
        outcome.ok = object.value(QStringLiteral("ok")).toBool(false);
        outcome.instruction =
            object.value(QStringLiteral("instruction")).toString();
        outcome.sourceMediaId =
            object.value(QStringLiteral("sourceMediaId")).toString();
        outcome.sourcePath = object.value(QStringLiteral("sourcePath")).toString();
        outcome.outputPath = object.value(QStringLiteral("outputPath")).toString();
        outcome.startMs = static_cast<qint64>(
            object.value(QStringLiteral("startMs")).toDouble());
        outcome.endMs = static_cast<qint64>(
            object.value(QStringLiteral("endMs")).toDouble());
        outcome.outputWidth = object.value(QStringLiteral("outputWidth")).toInt();
        outcome.outputHeight =
            object.value(QStringLiteral("outputHeight")).toInt();
        outcome.outputFps = object.value(QStringLiteral("outputFps")).toDouble();
        outcome.frameCount = object.value(QStringLiteral("frameCount")).toInt();
        outcome.error = object.value(QStringLiteral("error")).toString();

        const QJsonValue notesValue = object.value(QStringLiteral("notes"));
        if (notesValue.isArray()) {
            for (const QJsonValue &value : notesValue.toArray()) {
                if (value.isString()) {
                    outcome.notes.append(value.toString());
                }
            }
        }
        const QJsonValue unresolvedValue =
            object.value(QStringLiteral("unresolvedReferences"));
        if (unresolvedValue.isArray()) {
            for (const QJsonValue &value : unresolvedValue.toArray()) {
                if (value.isString()) {
                    outcome.unresolvedReferences.append(value.toString());
                }
            }
        }
        const QJsonValue targetsValue =
            object.value(QStringLiteral("resolvedTargets"));
        if (targetsValue.isArray()) {
            for (const QJsonValue &value : targetsValue.toArray()) {
                if (!value.isObject()) {
                    continue;
                }
                const QJsonObject targetObject = value.toObject();
                ReframeTarget target;
                target.id = targetObject.value(QStringLiteral("id")).toString();
                target.yawDeg =
                    targetObject.value(QStringLiteral("yawDeg")).toDouble();
                target.pitchDeg =
                    targetObject.value(QStringLiteral("pitchDeg")).toDouble();
                outcome.resolvedTargets.append(target);
            }
        }

        const QJsonValue segmentsValue =
            object.value(QStringLiteral("temporalSegments"));
        if (segmentsValue.isArray()) {
            for (const QJsonValue &value : segmentsValue.toArray()) {
                if (!value.isObject()) {
                    continue;
                }
                const QJsonObject entry = value.toObject();
                const QJsonValue startValue =
                    entry.value(QStringLiteral("startMs"));
                const QJsonValue endValue = entry.value(QStringLiteral("endMs"));
                if (!startValue.isDouble() || !endValue.isDouble()) {
                    continue;
                }
                outcome.temporalSegments.append(qMakePair(
                    static_cast<qint64>(startValue.toDouble()),
                    static_cast<qint64>(endValue.toDouble())));
            }
        }

        if (outcome.outputPath.isEmpty()) {
            if (error) {
                *error = QStringLiteral(
                    "Reframe command outcome has no output path.");
            }
            return false;
        }
        *out = outcome;
        return true;
    }
};

Q_DECLARE_METATYPE(ReframeCommandOutcome)

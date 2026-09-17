#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMetaType>
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

    bool hasRange() const { return endMs > startMs && startMs >= 0; }

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
        return object;
    }
};

Q_DECLARE_METATYPE(ReframeCommandOutcome)

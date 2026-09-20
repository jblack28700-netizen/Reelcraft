#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMetaType>
#include <QPair>
#include <QString>
#include <QStringList>

#include "reframe/EditDecision.h"
#include "reframe/ReframeIntent.h" // ReframeTarget

// ReframeCommandOutcome is the application-visible, structured result of one 360
// reframe command (Objective 9). It records the source reference, the
// instruction, the effective time range, the output specification, the generated
// output path, the success/failure state, and structured error information, so
// the application/UI can present and log a command without reaching into the
// deterministic engine or the perception layers.
//
// It is a plain QtCore value type (headless-testable). Records are persisted
// additively in the project (Decision 027). Since Objective 16 a record also
// carries the resolved EditDecision that produced it, so the render can be
// reproduced without re-parsing the instruction and without any perception
// provider (Decision 033).
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

    // --- Objective 16: the reproducible decision behind this render ---------
    // True only when the record carries a decision that loaded AND validated.
    // A record may legitimately have no decision (every record written before
    // Objective 16, and any command that never reached an output target).
    bool hasEditDecision() const { return m_hasEditDecision; }
    EditDecision editDecision() const { return m_editDecision; }

    // Non-empty only when the record DID carry an editDecision that could not be
    // loaded. Empty means "no decision", which is not an error. The distinction
    // matters: absent is normal, unreadable is a reportable failure.
    QString editDecisionError() const { return m_editDecisionError; }

    // Attaches the decision that produced this render. An invalid decision is
    // not attached (hasEditDecision() stays false); instead the refusal is
    // recorded in editDecisionError() with the validation reason, so a rejected
    // decision is never a silent skip.
    void setEditDecision(const EditDecision &decision)
    {
        m_rawEditDecision = QJsonValue(QJsonValue::Undefined);
        m_editDecisionError.clear();
        QString validationError;
        if (decision.isValid(&validationError)) {
            m_editDecision = decision;
            m_hasEditDecision = true;
        } else {
            m_editDecision = EditDecision();
            m_hasEditDecision = false;
            m_editDecisionError = validationError;
        }
    }

    // Records that this render deliberately carries no decision, and why. Used
    // when the decision stage produced no valid plan: the absence is explained
    // rather than being indistinguishable from "this record predates decisions".
    void setEditDecisionUnavailable(const QString &reason)
    {
        m_editDecision = EditDecision();
        m_hasEditDecision = false;
        m_rawEditDecision = QJsonValue(QJsonValue::Undefined);
        m_editDecisionError = reason;
    }

    // --- Objective 38: a preserved unreadable render record -------------------
    // A persisted render record that cannot be parsed is PRESERVED verbatim rather
    // than discarded, exactly as an unreadable edit decision is (Objective 16). It
    // keeps its position in the record list and is re-emitted byte-identically when
    // the project is saved again, so opening and re-saving a project can never
    // destroy data this build cannot interpret -- the resolution recorded for this
    // gap in KNOWN_ISSUES.
    //
    // A preserved record is NOT a usable record: it carries no output path and no
    // decision, so every execution path (preview, playback, replay, revision,
    // review, provenance, destination claiming) refuses it. Only persistence sees
    // it.
    bool hasRawRecord() const { return !m_rawRecord.isUndefined(); }
    QJsonValue rawRecord() const { return m_rawRecord; }
    void setRawRecord(const QJsonValue &raw) { m_rawRecord = raw; }

    // The record as a JSON VALUE: a preserved unreadable entry is returned exactly
    // as it was persisted (whatever JSON type it had); a normal record as its
    // object form. Persistence uses THIS, so nothing is normalized away.
    QJsonValue toJsonValue() const
    {
        return m_rawRecord.isUndefined() ? QJsonValue(toJsonObject()) : m_rawRecord;
    }

    QJsonObject toJsonObject() const
    {
        if (!m_rawRecord.isUndefined()) {
            // A preserved entry has no parsed form. An object entry is returned
            // verbatim; a non-object entry has no object form at all and is only
            // representable as a VALUE (toJsonValue()).
            return m_rawRecord.isObject() ? m_rawRecord.toObject() : QJsonObject();
        }
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
        // Objective 16. A decision that failed to load is re-emitted verbatim so
        // that opening and re-saving a project can never destroy data this build
        // cannot interpret (lenient-record policy, Decision 033).
        if (m_hasEditDecision) {
            object.insert(QStringLiteral("editDecision"),
                          m_editDecision.toJsonObject());
        } else if (!m_rawEditDecision.isUndefined()) {
            object.insert(QStringLiteral("editDecision"), m_rawEditDecision);
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

        // Objective 16. Lenient-record policy (Decision 033): an unreadable
        // decision never fails the whole record, because the record is a
        // historical fact about a render that really happened. The decision is
        // refused (never guessed), flagged with its reason, and preserved
        // verbatim for re-serialization.
        const QJsonValue decisionValue =
            object.value(QStringLiteral("editDecision"));
        if (!decisionValue.isUndefined() && !decisionValue.isNull()) {
            EditDecision decision;
            QString decisionError;
            if (decisionValue.isObject()
                && EditDecision::readFromJsonObject(decisionValue.toObject(),
                                                   &decision, &decisionError)) {
                outcome.m_hasEditDecision = true;
                outcome.m_editDecision = decision;
            } else {
                outcome.m_hasEditDecision = false;
                outcome.m_editDecisionError = decisionError.isEmpty()
                    ? QStringLiteral("Edit decision entry is not an object.")
                    : decisionError;
                outcome.m_rawEditDecision = decisionValue;
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

private:
    // Objective 16 (Decision 033). m_rawEditDecision holds an editDecision that
    // could not be loaded, byte-for-byte as it was persisted.
    bool m_hasEditDecision = false;
    EditDecision m_editDecision;
    QString m_editDecisionError;
    // NOTE: Qt's default-constructed QJsonValue is Null, not Undefined, so this
    // is initialized explicitly. Getting it wrong silently emits
    // "editDecision": null for every record that has no decision.
    QJsonValue m_rawEditDecision = QJsonValue(QJsonValue::Undefined);

    // Objective 38: the persisted entry, byte-for-byte, of a render record this
    // build could not parse. Same initialization hazard as above.
    QJsonValue m_rawRecord = QJsonValue(QJsonValue::Undefined);
};

Q_DECLARE_METATYPE(ReframeCommandOutcome)

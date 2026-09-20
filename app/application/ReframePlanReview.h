#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

#include "reframe/ReframeIntent.h" // ReframeTarget
#include "reframe/ReframePlan.h"

// Creator Review (Objective 34) — a READ-ONLY VIEW over the canonical plan.
//
// Creator Review answers three questions before anything is rendered: what
// Reelcraft understood, what it intends to do, and whether the creator accepts
// that. This type is the answer to the first two, and it is deliberately NOT a
// second edit representation:
//
//   * the authoritative artifact stays the validated `ReframePlan` (and, once a
//     render is recorded, the `EditDecision` that carries it);
//   * every field below is either copied from the plan, derived from it by a
//     pure function, or context the decision stage reported (the resolved
//     subjects, the notes). Nothing is invented, and nothing here is ever
//     serialized as a plan schema of its own;
//   * the plan it carries is the exact plan Accept hands to the deterministic
//     renderer, so review cannot change what executes.
struct ReframePlanReview
{
    // What Reelcraft understood (context from the decision stage).
    QString instruction;
    QString understanding;
    QStringList resolvedSubjects;
    QStringList notes;

    // What it will do (derived from the canonical plan).
    qint64 startMs = 0;
    qint64 endMs = 0;
    QList<QPair<qint64, qint64>> retainedSegments;
    int keyframeCount = 0;
    bool cameraMoves = false;
    double startYawDeg = 0.0;
    double endYawDeg = 0.0;
    double startPitchDeg = 0.0;
    double endPitchDeg = 0.0;
    bool lensIsConstant = true;
    double lensStartDeg = 0.0;
    double lensEndDeg = 0.0;
    int outputWidth = 0;
    int outputHeight = 0;
    double outputFps = 0.0;
    QString orientation;

    // Human-readable lines, all derived from the fields above.
    QString framing;
    QString cameraMovement;
    QString lens;
    QString timeRange;
    QString output;
    QString audio;

    // The canonical plan, unchanged: what Accept executes.
    ReframePlan plan;
    QString planDigest;

    bool isValid() const { return planDigest.size() == 64 && keyframeCount > 0; }

    // Deterministic and pure. `resolvedTargets` and `notes` are the decision
    // stage's own report; everything else comes from `plan`.
    static ReframePlanReview fromPlan(const ReframePlan &plan,
                                      const QString &instruction,
                                      const QStringList &notes,
                                      const QList<ReframeTarget> &resolvedTargets);

    // SHA-256 over the plan's canonical JSON: the review's fingerprint of the
    // exact plan it will execute.
    static QString digestOf(const ReframePlan &plan);

    // The review as display lines ("Label: value"), in a fixed order, so the UI
    // and the tests read the same thing.
    QStringList summaryLines() const;
};

Q_DECLARE_METATYPE(ReframePlanReview)

#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "reframe/TemporalEditPlan.h"

// ReframeTarget is a RESOLVED view direction for a named subject. Resolution
// (person/object detection, tracking, calibration) is a separate, replaceable
// capability; the intent layer never invents coordinates. A target that cannot
// be resolved stays an unresolved reference instead of a fabricated direction.
struct ReframeTarget
{
    QString id;
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
};

// One ordered camera instruction parsed from a natural-language request.
struct ReframeCameraMove
{
    QString label;      // human-readable source clause
    QString targetRef;  // subject reference when no explicit direction exists
    bool hasDirection = false;
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    // True when the clause asks the camera to KEEP a subject framed over time
    // ("follow me", "keep me centered") rather than to aim at it once
    // ("look at the car"). A follow move is executed as a camera path through
    // the subject's resolved track; an aim move stays a single fixed direction.
    // This is an in-memory intent field only: ReframeIntent is never persisted
    // (EditDecision stores the resolved plan), so it carries no schema impact.
    bool followSubject = false;
};

// Structured interpretation of a natural-language reframing request. This is
// the deterministic first implementation of the AI->edit-plan boundary; an AI
// provider can produce the same structure later without changing the media
// engine.
struct ReframeIntent
{
    bool recognized = false;
    bool hasOutput = false;
    int outputWidth = 1920;
    int outputHeight = 1080;
    double outputFps = 30.0;
    bool hasTimeRange = false;
    qint64 startMs = 0;
    qint64 endMs = 0;
    QList<ReframeCameraMove> moves;
    QStringList unresolvedTargets;
    QStringList notes;

    // Objective 14: structured temporal editing request (independent of any
    // rendering). hasTemporalRequest is true when a temporal operation was
    // recognized, even if it is invalid; temporalError then explains why.
    bool hasTemporalRequest = false;
    TemporalEditPlan temporalEdit;
    QString temporalError;
    // True when the command referred to "this section"/"the current part"
    // without explicit timestamps; the runner substitutes the effective range.
    bool temporalUsesDefaultRange = false;

    // Objective 15: true when the command carries both a temporal edit and a
    // camera/target instruction. The documented semantic rule is that the
    // camera/target applies to the entire retained temporal range.
    bool hasCompoundEdit() const
    {
        return hasTemporalRequest && !moves.isEmpty();
    }
};

// Deterministic, rule-based parser for a small, documented instruction
// grammar. It recognizes output aspect/platform, time ranges, and camera
// directions (explicit yaw/pitch or named anchors) plus subject references.
// Unrecognized text is ignored and recorded in notes; unresolved subjects are
// reported rather than guessed.
class ReframeIntentParser
{
public:
    static ReframeIntent parse(const QString &text);
};

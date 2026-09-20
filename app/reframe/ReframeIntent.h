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

// Objective 30: a PLURAL framing request. The parser records WHICH GROUP was
// asked for and never which tracks: resolution happens against the current
// tracks and identity state at command time, exactly as a single reference
// does, so nothing is fabricated at parse time and nothing is persisted.
enum class ReframeSubjectGroup
{
    // An ordinary single-subject (or direction-only) instruction.
    None,
    // "both of us", "us both", "the two of us": the creator and the one other
    // visible person. Resolved through the existing identity rules, so an
    // unselected creator or more than one other person is refused honestly.
    CreatorAndOther,
    // "both people", "both of them": exactly two visible people, in the
    // selector's canonical order. More (or fewer) than two is ambiguous and is
    // refused rather than silently choosing.
    TwoPeople,
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
    // Objective 29: the lens this instruction asks for, when it asks for one.
    // A framing clause ("zoom in", "go wide", "close-up", "field of view 60")
    // sets it, and a clause that carries ONLY framing — neither a direction nor
    // a subject — is a framing instruction that changes the lens without moving
    // the camera. The value is an absolute vertical field of view in degrees,
    // inside the range ReframePlan already validates.
    //
    // In-memory only, exactly like followSubject: ReframeIntent is never
    // persisted, and the executable plan already carries each keyframe's field
    // of view, so this introduces no schema change.
    bool hasFieldOfView = false;
    double fieldOfViewDeg = 90.0;

    // Objective 30: set when the clause asked to keep SEVERAL subjects framed
    // together ("keep both of us in frame"). Such a move carries no single
    // `targetRef` and no direction: the group is resolved at command time and
    // executed as one camera path that keeps every resolved subject inside the
    // frame. In-memory only, like every other field here.
    ReframeSubjectGroup subjectGroup = ReframeSubjectGroup::None;
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

    // Objective 29: the DISTINCT field-of-view values this instruction asked
    // for, in instruction order. Empty means the instruction said nothing about
    // framing — which is deliberately different from "framing was requested at
    // the default value", because only the first is compatible with silently
    // using the default lens.
    QList<double> requestedFieldOfViews() const;
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

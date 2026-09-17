#pragma once

#include <QList>
#include <QString>
#include <QStringList>

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

#pragma once

#include <QString>
#include <QStringList>

#include "reframe/ReframeFrameProvider.h"
#include "reframe/ReframeIntent.h"
#include "reframe/ReframePlan.h"

class MediaDurationProbe;

// ReframePipeline is the end-to-end orchestration entry point for the 360 ->
// flat vertical slice:
//
//   360 source media -> parsed intent -> resolved targets -> ReframePlan
//     -> deterministic per-frame reframing -> encoded flat video
//
// It composes the existing seams (FrameExtractor-backed provider, CameraPath,
// EquirectView, FFmpeg encoder) and keeps the AI decision layer (intent) and
// the deterministic executor (plan + renderer) separate. The source media is
// only read.
class ReframePipeline
{
public:
    struct Request
    {
        QString sourcePath;
        QString sourceMediaId;
        QString instruction;
        QString outputPath;
        ReframePlan::TimeRange defaultRange;
        ReframePlan::OutputSpec defaultOutput;
        QList<ReframeTarget> resolvedTargets;
    };

    struct Result
    {
        bool ok = false;
        QString error;
        ReframePlan plan;
        ReframeIntent intent;
        int frameCount = 0;
        QString outputPath;
        QStringList notes;
        QStringList renderedFramePaths;
    };

    static Result run(const Request &request);

    // Renders an already-validated plan (the decision stage's output) without
    // re-parsing or re-building it. This lets a caller whose plan is produced by
    // a different planner (for example SpeakerReframePlanner) reuse the exact
    // same deterministic renderer/encoder. The source media is only read.
    //
    // Objective 28: the rendered output also carries the source audio over the
    // plan's retained source spans (its ordered segments when it has them,
    // otherwise its source range), so a retimed render keeps the audio that
    // belongs to the picture it kept. The audio decision is made from a FACT
    // reported by a MediaDurationProbe (the same seam the media-analysis
    // technical layer uses); probe is an optional injection point for callers
    // and tests, and defaults to the external ffprobe probe. A source with no
    // audio track renders exactly as before, and a probe that cannot answer
    // renders without audio and records the reason in Result::notes.
    static Result renderPlan(const ReframePlan &plan, const QString &sourcePath,
                             const QString &outputPath,
                             ReframeFrameProvider *provider = nullptr,
                             MediaDurationProbe *probe = nullptr);
};

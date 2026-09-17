#pragma once

#include <QString>
#include <QStringList>

#include "reframe/ReframeIntent.h"
#include "reframe/ReframePlan.h"

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
};

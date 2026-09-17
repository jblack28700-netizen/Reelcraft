#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "reframe/ReframeFrameProvider.h"
#include "reframe/ReframeIntent.h"
#include "reframe/ReframePlan.h"
#include "target/TargetDetector.h"
#include "target/TargetIdentity.h"
#include "target/TargetResolver.h"

// 360 Reframing Objective 8 — end-to-end 360 user-command execution.
//
// ReframeCommandRunner is the single composition entry point that turns a
// natural-language/structured user command plus a 360 source into a validated
// deterministic output:
//
//   user instruction -> parsed intent -> subject references
//     -> target resolution (replaceable detector + tracker)
//     -> identity/selection (replaceable) -> resolved directions
//     -> validated ReframePlan -> deterministic render -> flat video
//
// It composes existing pieces and adds no perception of its own: the detector
// is the replaceable TargetDetector seam, identity/selection is
// TargetIdentityRegistry + TargetSelector, and execution is the deterministic
// ReframePipeline/ReframeRenderer. The source media is only read.
//
// The command path never fabricates a subject direction: an unresolved or
// ambiguous reference is reported as an error with the reason, and no plan is
// produced for it. A direction-only instruction (for example "pan right") needs
// no detector.
//
// prepare() is the decision stage (parse + resolve + plan); it is deterministic
// and model-free when an in-memory provider/detector is injected. run() adds the
// deterministic execution stage.

struct ReframeCommandRequest
{
    QString sourcePath;
    QString sourceMediaId;
    QString instruction;
    QString outputPath;
    ReframePlan::TimeRange defaultRange;
    ReframePlan::OutputSpec defaultOutput;

    // Optional creator seed. When set, the identity is bound (and refreshed)
    // before subject references are resolved, so "me" can resolve. A seed that
    // does not bind is reported in notes but does not invent a direction.
    bool hasCreatorSelection = false;
    CreatorTargetSelection creatorSelection;

    // Target resolution inputs (used only when the instruction references a
    // subject). targetQuery defaults to person detection.
    TargetResolveConfig resolveConfig;
    TargetQuery targetQuery{ QStringLiteral("person"), QString(), 0.35 };
    TargetIdentityRegistry::Config identityConfig;
    // Absolute source timestamps to resolve; when empty they are derived from
    // the effective range (maxResolveSamples evenly spaced samples).
    QList<qint64> resolveTimestamps;
    int maxResolveSamples = 5;
};

struct ReframeCommandResult
{
    bool ok = false;
    QString error;
    ReframeIntent intent;
    QList<TargetTrack> tracks;
    QList<ReframeTarget> resolvedTargets;
    QStringList unresolvedReferences;
    ReframePlan plan;
    QStringList notes;
    // Execution (run() only).
    int frameCount = 0;
    QString outputPath;
};

class ReframeCommandRunner
{
public:
    // Decision stage: parse the command, resolve every subject reference
    // against the detector/tracker, bind the creator identity when seeded, and
    // build the validated deterministic plan. Never renders; never fabricates a
    // direction. provider may be null when the instruction has no subject
    // reference and sourcePath is unused; otherwise it is built from sourcePath.
    static ReframeCommandResult prepare(const ReframeCommandRequest &request,
                                        TargetDetector *detector,
                                        ReframeFrameProvider *provider);

    // Decision + execution: prepare, then render/encode through the existing
    // ReframePipeline. provider is used for target resolution only; execution
    // always reads sourcePath.
    static ReframeCommandResult run(const ReframeCommandRequest &request,
                                    TargetDetector *detector,
                                    ReframeFrameProvider *provider = nullptr);
};

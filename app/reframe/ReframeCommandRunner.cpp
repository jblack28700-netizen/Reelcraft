#include "reframe/ReframeCommandRunner.h"

#include <QFileInfo>

#include <cmath>
#include <memory>

#include "media/FrameExtractor.h"
#include "reframe/FfmpegSeekFrameProvider.h"
#include "reframe/ReframePipeline.h"
#include "reframe/ReframePlanBuilder.h"
#include "target/TargetSelector.h"

namespace {

// Evenly spaced resolution timestamps across the effective range, inclusive.
QList<qint64> deriveTimestamps(const ReframePlan::TimeRange &range, int samples)
{
    QList<qint64> timestamps;
    if (samples <= 1 || range.durationMs() <= 0) {
        timestamps.append(range.startMs);
        return timestamps;
    }
    const qint64 duration = range.durationMs();
    for (int i = 0; i < samples; ++i) {
        const qint64 timeMs =
            range.startMs
            + static_cast<qint64>(std::llround(
                static_cast<double>(i) * static_cast<double>(duration)
                / static_cast<double>(samples - 1)));
        if (timestamps.isEmpty() || timestamps.last() != timeMs) {
            timestamps.append(timeMs);
        }
    }
    return timestamps;
}

} // namespace

ReframeCommandResult ReframeCommandRunner::prepare(
    const ReframeCommandRequest &request, TargetDetector *detector,
    ReframeFrameProvider *provider)
{
    ReframeCommandResult result;
    result.intent = ReframeIntentParser::parse(request.instruction);
    result.outputPath = request.outputPath;

    const ReframePlan::TimeRange range =
        result.intent.hasTimeRange
            ? ReframePlan::TimeRange{ result.intent.startMs, result.intent.endMs }
            : request.defaultRange;
    if (!range.isValid()) {
        result.error = QStringLiteral("Reframe source range is invalid.");
        return result;
    }

    // Unique subject references, in instruction order.
    QStringList references;
    for (const ReframeCameraMove &move : result.intent.moves) {
        if (!move.targetRef.isEmpty() && !references.contains(move.targetRef)) {
            references.append(move.targetRef);
        }
    }

    if (!references.isEmpty()) {
        if (!detector) {
            result.error = QStringLiteral(
                "Instruction references a subject (%1) but no target detector "
                "was provided.").arg(references.join(QStringLiteral(", ")));
            return result;
        }

        // The provider is only needed for resolution; when absent it is built
        // from the source path. The source media is only read.
        std::unique_ptr<ReframeFrameProvider> ownedProvider;
        ReframeFrameProvider *frameProvider = provider;
        if (!frameProvider) {
            if (request.sourcePath.isEmpty()
                || !QFileInfo::exists(request.sourcePath)) {
                result.error = QStringLiteral(
                    "Source media does not exist; cannot resolve the target.");
                return result;
            }
            const QString ffmpeg = FrameExtractor::defaultExecutablePath();
            if (ffmpeg.isEmpty()) {
                result.error = QStringLiteral(
                    "FFmpeg is unavailable; cannot decode frames for target "
                    "resolution.");
                return result;
            }
            ownedProvider = std::make_unique<FfmpegSeekFrameProvider>(
                request.sourcePath, ffmpeg);
            frameProvider = ownedProvider.get();
        }

        QList<qint64> timestamps = request.resolveTimestamps;
        if (timestamps.isEmpty()) {
            timestamps =
                deriveTimestamps(range, qMax(1, request.maxResolveSamples));
        }

        TargetResolver resolver(request.resolveConfig);
        QList<TargetTrack> tracks;
        QString resolveError;
        if (!resolver.resolveSequence(frameProvider, timestamps,
                                      request.targetQuery, detector, &tracks,
                                      &resolveError)) {
            result.error = resolveError.isEmpty()
                ? QStringLiteral("Target resolution failed.")
                : resolveError;
            return result;
        }
        result.tracks = tracks;
        for (const QString &note : resolver.notes()) {
            result.notes.append(note);
        }

        TargetIdentityRegistry registry(request.identityConfig);
        if (request.hasCreatorSelection) {
            QString bindError;
            if (registry.bindFromSelection(request.creatorSelection, tracks,
                                           &bindError)) {
                registry.update(tracks, timestamps.last());
            } else {
                result.notes.append(QStringLiteral(
                    "Creator selection did not bind: %1").arg(bindError));
            }
        }

        QList<ReframeTarget> resolved;
        for (const QString &reference : references) {
            const TargetSelectionResult selection =
                TargetSelector::select(reference, tracks, registry);
            if (selection.resolved) {
                // selection.target.id is the normalized reference, which the
                // ReframePlanBuilder matches case-insensitively.
                resolved.append(selection.target);
                result.notes.append(QStringLiteral(
                    "Resolved '%1' to %2 (%3).")
                                        .arg(reference, selection.targetId,
                                             selection.method));
            } else if (selection.ambiguous) {
                result.unresolvedReferences.append(reference);
                result.notes.append(QStringLiteral(
                    "'%1' is ambiguous across %2 candidate(s).")
                                        .arg(reference)
                                        .arg(selection.candidates.size()));
            } else {
                result.unresolvedReferences.append(reference);
                result.notes.append(QStringLiteral("'%1' was not resolved: %2")
                                        .arg(reference, selection.error));
            }
        }
        result.resolvedTargets = resolved;
        // The intent now reflects the resolved command state.
        result.intent.unresolvedTargets = result.unresolvedReferences;
        if (result.unresolvedReferences.isEmpty()) {
            // The parser recorded every subject reference as unresolved; once
            // they are resolved that stale note must not survive into the plan.
            for (int i = result.intent.notes.size() - 1; i >= 0; --i) {
                if (result.intent.notes.at(i).startsWith(
                        QStringLiteral("Unresolved subject reference(s):"))) {
                    result.intent.notes.removeAt(i);
                }
            }
        }

        if (!result.unresolvedReferences.isEmpty()) {
            result.error = QStringLiteral(
                "Unresolved subject reference(s): %1. No direction was "
                "fabricated.").arg(result.unresolvedReferences.join(
                    QStringLiteral(", ")));
            return result;
        }
    }

    const ReframeBuildResult built = ReframePlanBuilder::build(
        result.intent, result.resolvedTargets, request.defaultRange,
        request.defaultOutput);
    for (const QString &note : built.notes) {
        result.notes.append(note);
    }
    if (!built.ok) {
        result.error = built.error;
        return result;
    }

    result.plan = built.plan;
    result.plan.setSourceMediaId(request.sourceMediaId);
    result.ok = true;
    return result;
}

ReframeCommandResult ReframeCommandRunner::run(
    const ReframeCommandRequest &request, TargetDetector *detector,
    ReframeFrameProvider *provider)
{
    ReframeCommandResult result = prepare(request, detector, provider);
    if (!result.ok) {
        return result;
    }

    ReframePipeline::Request pipelineRequest;
    pipelineRequest.sourcePath = request.sourcePath;
    pipelineRequest.sourceMediaId = request.sourceMediaId;
    pipelineRequest.instruction = request.instruction;
    pipelineRequest.outputPath = request.outputPath;
    pipelineRequest.defaultRange = request.defaultRange;
    pipelineRequest.defaultOutput = request.defaultOutput;
    pipelineRequest.resolvedTargets = result.resolvedTargets;

    const ReframePipeline::Result executed =
        ReframePipeline::run(pipelineRequest);
    if (!executed.ok) {
        result.ok = false;
        result.error = executed.error;
        return result;
    }

    result.plan = executed.plan;
    result.frameCount = executed.frameCount;
    result.outputPath = executed.outputPath;
    result.ok = true;
    return result;
}

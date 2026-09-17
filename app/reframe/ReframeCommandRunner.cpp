#include "reframe/ReframeCommandRunner.h"

#include <QFileInfo>

#include <cmath>
#include <memory>

#include "media/FrameExtractor.h"
#include "reframe/FfmpegSeekFrameProvider.h"
#include "reframe/ReframePipeline.h"
#include "reframe/ReframePlanBuilder.h"
#include "target/SpeakerEvidenceAnalyzer.h"
#include "target/SpeakerReframePlanner.h"
#include "target/SpeakerTargetAssociator.h"
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

// Objective 11: does the reference ask for the active speaker?
bool isSpeakerReference(const QString &reference)
{
    static const QStringList aliases = {
        QStringLiteral("speaker"),
        QStringLiteral("active speaker"),
        QStringLiteral("current speaker"),
        QStringLiteral("person speaking"),
        QStringLiteral("speaking person"),
        QStringLiteral("person talking"),
        QStringLiteral("talking person"),
        QStringLiteral("whoever is speaking"),
        QStringLiteral("who is speaking"),
    };
    return aliases.contains(TargetSelector::normalizeReference(reference));
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

    // Unique subject references, in instruction order, classified into speaker
    // references (Objective 11) and ordinary subject references.
    QStringList references;
    QStringList speakerReferences;
    QStringList otherReferences;
    for (const ReframeCameraMove &move : result.intent.moves) {
        if (move.targetRef.isEmpty() || references.contains(move.targetRef)) {
            continue;
        }
        references.append(move.targetRef);
        if (isSpeakerReference(move.targetRef)) {
            speakerReferences.append(move.targetRef);
        } else {
            otherReferences.append(move.targetRef);
        }
    }

    const bool wantsSpeaker = !speakerReferences.isEmpty();
    if (wantsSpeaker) {
        // Only an unambiguous speaker-follow request is supported in one
        // command; anything mixed is reported rather than silently
        // reinterpreted.
        if (!otherReferences.isEmpty()) {
            result.error = QStringLiteral(
                "A command cannot mix a speaker reference with other subject "
                "references.");
            return result;
        }
        for (const ReframeCameraMove &move : result.intent.moves) {
            if (move.hasDirection) {
                result.error = QStringLiteral(
                    "A command cannot mix an explicit camera direction with a "
                    "speaker reference.");
                return result;
            }
        }
        if (!request.speakerProvider) {
            result.error = QStringLiteral(
                "Instruction references a speaker (%1) but no speaker evidence "
                "provider was provided.")
                               .arg(speakerReferences.join(
                                   QStringLiteral(", ")));
            return result;
        }
    }

    QList<TargetTrack> tracks;
    qint64 lastResolvedAtMs = range.endMs;
    if (!references.isEmpty()) {
        if (!request.resolvedTracks.isEmpty()) {
            // The caller already resolved tracks; use them as-is.
            tracks = request.resolvedTracks;
            result.notes.append(
                QStringLiteral("Using %1 pre-resolved target track(s).")
                    .arg(tracks.size()));
        } else {
            if (!detector) {
                result.error = QStringLiteral(
                    "Instruction references a subject (%1) but no target "
                    "detector was provided.")
                                   .arg(references.join(QStringLiteral(", ")));
                return result;
            }

            // The provider is only needed for resolution; when absent it is
            // built from the source path. The source media is only read.
            std::unique_ptr<ReframeFrameProvider> ownedProvider;
            ReframeFrameProvider *frameProvider = provider;
            if (!frameProvider) {
                if (request.sourcePath.isEmpty()
                    || !QFileInfo::exists(request.sourcePath)) {
                    result.error = QStringLiteral(
                        "Source media does not exist; cannot resolve the "
                        "target.");
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
            lastResolvedAtMs = timestamps.last();

            TargetResolver resolver(request.resolveConfig);
            QString resolveError;
            if (!resolver.resolveSequence(frameProvider, timestamps,
                                          request.targetQuery, detector,
                                          &tracks, &resolveError)) {
                result.error = resolveError.isEmpty()
                    ? QStringLiteral("Target resolution failed.")
                    : resolveError;
                return result;
            }
            for (const QString &note : resolver.notes()) {
                result.notes.append(note);
            }
        }
    }
    result.tracks = tracks;

    // --- Speaker-follow command (Objective 11) ------------------------------
    // Audio is evidence: the active speaker selects the camera target, but an
    // explicit creator speaker binding is honoured first and nothing is
    // fabricated when the speaker cannot be associated.
    if (wantsSpeaker) {
        if (request.sourcePath.isEmpty()) {
            result.error = QStringLiteral(
                "A speaker command requires the source media path.");
            return result;
        }

        SpeakerTargetAssociator associator;
        for (const QPair<QString, QString> &binding : request.speakerBindings) {
            associator.bind(binding.first, binding.second);
        }
        SpeakerEvidenceAnalyzer analyzer;
        const SpeakerEvidenceAnalyzer::Result analysis =
            analyzer.analyze(request.sourcePath, range.startMs, range.endMs,
                             tracks, request.speakerProvider, associator);
        result.speakerSegments = analysis.segments;
        result.speakerCommand = true;
        for (const QString &note : analysis.notes) {
            result.notes.append(note);
        }
        if (!analysis.analysis.available) {
            result.error = analysis.analysis.error.isEmpty()
                ? QStringLiteral("Speaker evidence is unavailable.")
                : analysis.analysis.error;
            return result;
        }

        const ReframePlan::OutputSpec output =
            result.intent.hasOutput
                ? ReframePlan::OutputSpec{ result.intent.outputWidth,
                                           result.intent.outputHeight,
                                           result.intent.outputFps }
                : request.defaultOutput;
        ReframePlan speakerPlan;
        QString speakerPlanError;
        if (!SpeakerReframePlanner::plan(analysis.segments, tracks, range,
                                         output,
                                         SpeakerReframePlanner::Config{},
                                         &speakerPlan, &speakerPlanError)) {
            result.error = speakerPlanError.isEmpty()
                ? QStringLiteral("No active speaker could be associated with a "
                                 "visible target.")
                : speakerPlanError;
            return result;
        }

        // Report the associated directions (evidence only; the planner owns the
        // keyframes).
        QList<ReframeTarget> targets;
        for (const SpeakerSegment &segment : analysis.segments) {
            if (segment.verdict != SpeakerVerdict::Active
                || segment.targetId.isEmpty()) {
                continue;
            }
            bool present = false;
            for (const ReframeTarget &existing : targets) {
                if (existing.id == segment.targetId) {
                    present = true;
                }
            }
            if (present) {
                continue;
            }
            for (const TargetTrack &track : tracks) {
                if (track.id() != segment.targetId) {
                    continue;
                }
                ReframeTarget target = track.representativeTarget();
                target.id = segment.targetId;
                targets.append(target);
                result.notes.append(
                    QStringLiteral("Active speaker associated with %1.")
                        .arg(segment.targetId));
            }
        }
        result.resolvedTargets = targets;
        result.intent.unresolvedTargets.clear();
        result.plan = speakerPlan;
        result.plan.setSourceMediaId(request.sourceMediaId);
        result.ok = true;
        return result;
    }

    // --- Ordinary subject / direction command -------------------------------
    if (!references.isEmpty()) {
        TargetIdentityRegistry registry(request.identityConfig);
        if (request.hasCreatorSelection) {
            QString bindError;
            if (registry.bindFromSelection(request.creatorSelection, tracks,
                                           &bindError)) {
                registry.update(tracks, lastResolvedAtMs);
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

    // Render the plan produced by the decision stage directly: a speaker plan
    // comes from SpeakerReframePlanner, not ReframePlanBuilder, so it must not
    // be re-derived. Execution stays the deterministic ReframePipeline.
    const ReframePipeline::Result executed = ReframePipeline::renderPlan(
        result.plan, request.sourcePath, request.outputPath, nullptr);
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

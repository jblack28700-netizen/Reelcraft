#include "reframe/ReframeCommandRunner.h"

#include <QFileInfo>

#include <cmath>
#include <memory>

#include "media/FrameExtractor.h"
#include "media/FfprobeDurationProbe.h"
#include "reframe/ReframeContract.h"
#include "reframe/FfmpegSeekFrameProvider.h"
#include "reframe/ReframePipeline.h"
#include "reframe/ReframePlanBuilder.h"
#include "reframe/ReframeStreamFrameProvider.h"
#include "target/SpeakerEvidenceAnalyzer.h"
#include "target/SpeakerReframePlanner.h"
#include "target/SpeakerTargetAssociator.h"
#include "target/TargetSelector.h"
#include "target/TargetTrackPlanner.h"

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

// How many trajectory samples a FOLLOW instruction may take across its range:
// one every followSampleIntervalMs, bounded by the caller's maximum, and never
// fewer than two so both ends of the range are covered. Deterministic, and a
// pure function of the request and the range.
int followSampleCountFor(const ReframePlan::TimeRange &range,
                         const ReframeCommandRequest &request)
{
    const qint64 intervalMs = qMax<qint64>(1, request.followSampleIntervalMs);
    const int cap = qMax(2, request.followResolveSamplesMax);
    const qint64 count = range.durationMs() / intervalMs + 1;
    if (count < 2) {
        return 2;
    }
    return count > cap ? cap : static_cast<int>(count);
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


    // Objective 14: resolve a structured temporal edit against the known
    // source duration. Deterministic, model-free, and honest: it never invents
    // a timestamp and fails when the request cannot be satisfied.
    QList<TemporalRange> temporalSegments;
    const bool hasTemporalEdit = result.intent.hasTemporalRequest;
    if (hasTemporalEdit) {
        if (!result.intent.temporalError.isEmpty()) {
            result.error = result.intent.temporalError;
            return result;
        }
        TemporalEditPlan edit = result.intent.temporalEdit;
        if (result.intent.temporalUsesDefaultRange) {
            if (!range.isValid()) {
                result.error = QStringLiteral(
                    "A temporal command that refers to 'this section' needs a "
                    "valid current selection.");
                return result;
            }
            const QList<TemporalRange> seed{
                TemporalRange{ range.startMs, range.endMs } };
            edit = edit.operation() == TemporalEditPlan::Operation::Keep
                ? TemporalEditPlan::keep(seed)
                : TemporalEditPlan::remove(seed);
        }
        if (!edit.isSpecified()) {
            result.error = QStringLiteral("Temporal edit is not specified.");
            return result;
        }
        if (request.sourceDurationMs <= 0) {
            result.error = QStringLiteral(
                "The source duration is required to resolve a temporal edit.");
            return result;
        }
        QString resolveError;
        temporalSegments = edit.resolve(
            request.sourceDurationMs, range.startMs, &resolveError);
        if (temporalSegments.isEmpty()) {
            result.error = resolveError.isEmpty()
                ? QStringLiteral("Temporal edit could not be resolved.")
                : resolveError;
            return result;
        }
        result.notes.append(QStringLiteral(
            "Temporal edit resolves to %1 retained source range(s).")
            .arg(temporalSegments.size()));
    }

    const auto applyTemporal = [&](ReframePlan *plan) {
        if (!hasTemporalEdit || !plan) {
            return;
        }
        QList<ReframePlan::TimeRange> segments;
        for (const TemporalRange &segment : temporalSegments) {
            segments.append(
                ReframePlan::TimeRange{ segment.startMs, segment.endMs });
        }
        plan->setSegments(segments);
        // Keep the plan valid: the source range must still contain both the
        // keyframes and every retained segment.
        qint64 minStart = plan->sourceRange().startMs;
        qint64 maxEnd = plan->sourceRange().endMs;
        for (const TemporalRange &segment : temporalSegments) {
            minStart = qMin(minStart, segment.startMs);
            maxEnd = qMax(maxEnd, segment.endMs);
        }
        plan->setSourceRange(ReframePlan::TimeRange{ minStart, maxEnd });
    };

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

    // Objective 23: a FOLLOW instruction ("follow me", "keep me centered") asks
    // the camera to keep the subject framed over time, so it is executed as a
    // camera path through that subject's resolved track. The track id is
    // captured during resolution below. Only a single target-referencing follow
    // move qualifies; everything else keeps the existing behaviour.
    QString followReference;
    // Objective 29: the follow path builds its own keyframes through
    // TargetTrackPlanner, so a lens requested by the same instruction is carried
    // across explicitly rather than assumed.
    bool followHasFieldOfView = false;
    double followFieldOfViewDeg = 0.0;
    if (result.intent.moves.size() == 1) {
        const ReframeCameraMove &move = result.intent.moves.first();
        if (move.followSubject && !move.hasDirection
            && !move.targetRef.isEmpty() && !isSpeakerReference(move.targetRef)) {
            followReference = move.targetRef;
            followHasFieldOfView = move.hasFieldOfView;
            followFieldOfViewDeg = move.fieldOfViewDeg;
        }
    }
    QString followTrackId;

    // Objective 30: a plural framing request ("keep both of us in frame"). The
    // group is recorded by the parser and resolved HERE, against the current
    // tracks and identity state, so nothing is fabricated at parse time.
    ReframeSubjectGroup pluralGroup = ReframeSubjectGroup::None;
    for (const ReframeCameraMove &move : result.intent.moves) {
        if (move.subjectGroup != ReframeSubjectGroup::None) {
            pluralGroup = move.subjectGroup;
            break;
        }
    }
    const bool wantsMultiSubject = pluralGroup != ReframeSubjectGroup::None;
    if (wantsMultiSubject) {
        // One framing decision cannot be combined with other camera
        // instructions: the combined framing would have to drop one of them.
        if (result.intent.moves.size() != 1) {
            result.error = QStringLiteral(
                "A multi-subject framing instruction cannot be combined with "
                "other camera instructions.");
            return result;
        }
        if (result.intent.moves.first().hasDirection) {
            result.error = QStringLiteral(
                "A multi-subject framing instruction cannot also carry an "
                "explicit camera direction.");
            return result;
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
    if (!references.isEmpty() || wantsMultiSubject) {
        if (!request.resolvedTracks.isEmpty()) {
            // The caller already resolved tracks; use them as-is.
            tracks = request.resolvedTracks;
            result.notes.append(
                QStringLiteral("Using %1 pre-resolved target track(s).")
                    .arg(tracks.size()));
        } else {
            if (!detector) {
                result.error = wantsMultiSubject
                    ? QStringLiteral(
                          "Instruction references several subjects but no target "
                          "detector was provided.")
                    : QStringLiteral(
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
                // Objective 25: resolve the trajectory through ONE persistent
                // decoding process instead of one FFmpeg process per sample.
                //
                // Resolving N timestamps used to cost N FFmpeg invocations. On
                // this device an invocation costs ~2.5 s whatever the frame size,
                // because process start-up dominates under proot, so a follow
                // command's denser sampling (up to 24 samples) multiplied that
                // fixed cost. ReframeStreamFrameProvider is the provider the
                // render path already uses: it keeps one stream open at an anchor
                // and reads forward, re-anchoring when a request moves beyond its
                // bounded window, and it falls back to the positioned seek path
                // whenever streaming cannot serve a request. Objective 20 proved
                // its frames byte-identical to the seek path's.
                //
                // The source frame rate is read once (through the existing
                // ffprobe seam) because the sequential cursor needs a frame step.
                // When it is unknown the provider streams nothing and every
                // request goes to the positioned seek exactly as before, so the
                // worst case is the previous behaviour, never a different frame.
                double sourceFps = 0.0;
                {
                    FfprobeDurationProbe rateProbe;
                    QString rateError;
                    rateProbe.frameRate(request.sourcePath, &sourceFps, &rateError);
                }
                ownedProvider = std::make_unique<ReframeStreamFrameProvider>(
                    request.sourcePath, ffmpeg, sourceFps);
                frameProvider = ownedProvider.get();
            }

            QList<qint64> timestamps = request.resolveTimestamps;
            if (timestamps.isEmpty()) {
                // A follow instruction samples the range at its own temporal
                // resolution; every other command keeps the existing budget.
                // A multi-subject framing needs a trajectory like a follow
                // command, so it uses the follow sampling budget (Objective 24).
                const int samples =
                    (followReference.isEmpty() && !wantsMultiSubject)
                        ? qMax(1, request.maxResolveSamples)
                        : followSampleCountFor(range, request);
                timestamps = deriveTimestamps(range, samples);
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
        // Objective 29: a speaker plan's keyframes belong to
        // SpeakerReframePlanner, so a requested lens is applied through its
        // config. A LENS CHANGE cannot be expressed there, and rendering one
        // fixed lens instead would silently drop half the request, so it is
        // refused honestly.
        const QList<double> speakerFieldOfViews =
            result.intent.requestedFieldOfViews();
        SpeakerReframePlanner::Config speakerConfig;
        if (speakerFieldOfViews.size() > 1) {
            QStringList requested;
            for (double fieldOfView : speakerFieldOfViews) {
                requested.append(QString::number(fieldOfView, 'g', 10));
            }
            result.error = QStringLiteral(
                "A command cannot combine a speaker reference with a lens "
                "change (requested fields of view: %1 degrees).")
                               .arg(requested.join(QStringLiteral(", ")));
            return result;
        }
        if (speakerFieldOfViews.size() == 1) {
            speakerConfig.fieldOfViewDeg = speakerFieldOfViews.first();
        }

        ReframePlan speakerPlan;
        QString speakerPlanError;
        if (!SpeakerReframePlanner::plan(analysis.segments, tracks, range,
                                         output, speakerConfig,
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
        applyTemporal(&result.plan);
        // Objective 18 (Decision 035): the final plan must honour the executable
        // requirements of the intent it was built from.
        const ContractReport contract =
            ReframeContract::check(result.intent, result.plan);
        if (!contract.isConsistent()) {
            result.error = contract.summary();
            return result;
        }
        result.plan.setSourceMediaId(request.sourceMediaId);
        result.ok = true;
        return result;
    }

    // --- Multi-subject framing (Objective 30) --------------------------------
    // "keep both of us in frame" resolves two EXISTING identities and produces
    // one camera path that keeps both inside the frame. Every reference must
    // resolve: an unresolved or ambiguous subject fails the command instead of
    // being dropped, and a framing that cannot contain both is refused with the
    // measured requirement instead of being clamped.
    if (wantsMultiSubject) {
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

        QStringList groupReferences;
        if (pluralGroup == ReframeSubjectGroup::CreatorAndOther) {
            // The creator and the one other visible person, resolved through the
            // existing identity rules ("me" needs a selection; "the other
            // person" is ambiguous when more than one other is visible).
            groupReferences = { QStringLiteral("me"),
                                QStringLiteral("the other person") };
        } else {
            // "both people": EXACTLY two visible people, in the selector's own
            // canonical order. More or fewer is ambiguous and is reported with
            // its candidates rather than choosing.
            QList<TargetTrack> active;
            for (const TargetTrack &track : tracks) {
                if (track.active() && !track.isEmpty()) {
                    active.append(track);
                }
            }
            QList<TargetTrack> people;
            for (const TargetTrack &track : active) {
                if (track.label().compare(QStringLiteral("person"),
                                          Qt::CaseInsensitive) == 0) {
                    people.append(track);
                }
            }
            if (people.isEmpty()) {
                people = active;
            }
            if (people.size() != 2) {
                QStringList ids;
                for (const TargetTrack &track : people) {
                    ids.append(track.id());
                }
                result.error = QStringLiteral(
                    "Keeping both people in frame needs exactly two visible "
                    "people; %1 were resolved (%2).")
                                   .arg(people.size())
                                   .arg(ids.isEmpty()
                                            ? QStringLiteral("none")
                                            : ids.join(QStringLiteral(", ")));
                return result;
            }
            groupReferences = { QStringLiteral("person 1"),
                                QStringLiteral("person 2") };
        }

        QList<ReframeTarget> resolved;
        QStringList resolvedIds;
        for (const QString &reference : groupReferences) {
            const TargetSelectionResult selection =
                TargetSelector::select(reference, tracks, registry);
            if (!selection.resolved) {
                result.error = QStringLiteral(
                    "Multi-subject framing could not resolve '%1': %2")
                                   .arg(reference, selection.error);
                return result;
            }
            if (resolvedIds.contains(selection.targetId)) {
                result.error = QStringLiteral(
                    "Multi-subject framing resolved two references to the same "
                    "target (%1).")
                                   .arg(selection.targetId);
                return result;
            }
            resolvedIds.append(selection.targetId);
            resolved.append(selection.target);
            result.notes.append(QStringLiteral("Resolved '%1' to %2 (%3).")
                                    .arg(reference, selection.targetId,
                                         selection.method));
        }
        result.resolvedTargets = resolved;

        const ReframePlan::OutputSpec pluralOutput =
            result.intent.hasOutput
                ? ReframePlan::OutputSpec{ result.intent.outputWidth,
                                           result.intent.outputHeight,
                                           result.intent.outputFps }
                : request.defaultOutput;

        // Objective 29 composes: a named lens is honoured when it can contain
        // both subjects. A lens CHANGE cannot reach here at all — two lenses
        // need two clauses, which is two camera instructions, and the check at
        // the top of this branch refuses that combination rather than rendering
        // one framing decision for a request that asked for two.
        const QList<double> pluralFieldOfViews =
            result.intent.requestedFieldOfViews();
        TargetTrackPlanner::Config pluralConfig;
        pluralConfig.minConfidence = request.resolveConfig.minConfidence;
        if (pluralFieldOfViews.size() == 1) {
            pluralConfig.requestedFieldOfViewDeg = pluralFieldOfViews.first();
        }

        ReframePlan pluralPlan;
        QString pluralError;
        if (!TargetTrackPlanner::planTracks(tracks, resolvedIds, range,
                                            pluralOutput, pluralConfig,
                                            &pluralPlan, &pluralError)) {
            result.error = pluralError.isEmpty()
                ? QStringLiteral("Multi-subject framing could not be planned.")
                : pluralError;
            return result;
        }
        result.plan = pluralPlan;
        result.notes.append(QStringLiteral(
            "Framed %1 subjects for the whole instruction.")
                                .arg(resolvedIds.size()));
        applyTemporal(&result.plan);
        const ContractReport pluralContract =
            ReframeContract::check(result.intent, result.plan);
        if (!pluralContract.isConsistent()) {
            result.error = pluralContract.summary();
            return result;
        }
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
                if (!followReference.isEmpty() && reference == followReference) {
                    followTrackId = selection.targetId;
                }
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

    // Objective 23: when the instruction FOLLOWS a subject, replace the single
    // fixed direction the builder produced with a camera path through that
    // subject's resolved track. TargetTrackPlanner already turns a track's
    // time-ordered observations into the keyframes the deterministic renderer
    // understands; it was implemented and tested but never reached by the
    // command path, so "follow me while I'm walking" rendered a locked-off shot.
    //
    // The builder above still runs, so it remains the validity gate and the
    // fallback: if the track has no usable observation inside the range, the
    // command degrades to the previous behaviour and says so instead of failing.
    // Explicit directions, multi-move camera paths and speaker commands never
    // take this path.
    if (!followTrackId.isEmpty()) {
        for (const TargetTrack &track : tracks) {
            if (track.id() != followTrackId) {
                continue;
            }
            const ReframePlan::OutputSpec followOutput =
                result.intent.hasOutput
                    ? ReframePlan::OutputSpec{ result.intent.outputWidth,
                                               result.intent.outputHeight,
                                               result.intent.outputFps }
                    : request.defaultOutput;
            TargetTrackPlanner::Config followConfig;
            followConfig.minConfidence = request.resolveConfig.minConfidence;
            if (followHasFieldOfView) {
                // Objective 29: "follow me and zoom in" must follow at the
                // requested lens, not at the planner's default.
                followConfig.fieldOfViewDeg = followFieldOfViewDeg;
            }
            ReframePlan followPlan;
            QString followError;
            if (TargetTrackPlanner::planTrack(track, range, followOutput,
                                              followConfig, &followPlan,
                                              &followError)) {
                result.plan = followPlan;
                result.notes.append(
                    QStringLiteral("Following %1 through %2 camera keyframe(s).")
                        .arg(followTrackId)
                        .arg(followPlan.keyframes().size()));
            } else {
                result.notes.append(
                    QStringLiteral("The subject was resolved but could not be "
                                   "followed continuously (%1); using a fixed "
                                   "camera instead.")
                        .arg(followError));
            }
            break;
        }
    }

    applyTemporal(&result.plan);
    // Objective 18 (Decision 035): the final plan must honour the executable
    // requirements of the intent it was built from.
    const ContractReport contract =
        ReframeContract::check(result.intent, result.plan);
    if (!contract.isConsistent()) {
        result.error = contract.summary();
        return result;
    }
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
    // Objective 28: execution diagnostics (for example why an output carries no
    // audio) travel with the command result, so the application outcome and the
    // caller see them.
    result.notes.append(executed.notes);
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

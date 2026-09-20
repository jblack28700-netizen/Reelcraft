# Reelcraft — AI Development Handoff

## Purpose

This document is the entry point for any AI agent continuing development of Reelcraft.

An AI agent must read this document before making project changes.

This document does not replace the other project documentation. It tells the agent which documents establish the project's source of truth and how they should be used.

The canonical operating policy for *how* an agent executes an approved objective — autonomous execution, efficiency per run, risk-based validation, scope control, and completion — is `docs/AGENT_WORKFLOW.md`. It governs execution; this document and the core state documents govern what the project is and what the objective is.

---

# 1. First Rule

Do not assume the current state of Reelcraft from chat history.

The repository documentation and actual project state are authoritative.

Before making changes:

1. Inspect the current Git status.
2. Read `MASTER_GUIDE.md`.
3. Read `CURRENT_STATE.md`.
4. Read `NEXT_TASK.md`.
5. Read `ARCHITECTURE.md`.
6. Read relevant decisions in `DECISIONS.md`.
7. Inspect the actual files relevant to the task.
8. Read the canonical operating policy in `docs/AGENT_WORKFLOW.md`.
9. Confirm the active development objective.

---

# 2. Project Identity

Reelcraft is an AI-native video editing platform designed primarily for:

- Content creators
- Vloggers
- 360° video creators
- Social-video creators
- Other creators who want AI-assisted editing

The long-term goal is to allow creators to provide footage and describe what they want in natural language while retaining meaningful control over the resulting edit.

---

# 3. Current Development Stage

The project has moved well past the documentation-foundation stage. A working Qt 6 desktop shell, project/media model, deterministic single-frame review path, and a Phase 3 media-engine foundation (replaceable media-source seam, frame pump, player/timing subsystem) exist and are verified.

The **current human-approved priority is the 360 editing/reframing capability**: 360 source -> scene/target understanding -> request interpretation -> structured reframe plan -> virtual-camera decisions -> deterministic execution -> flat video output.

A deterministic 360 reframing vertical slice is implemented and verified under `app/reframe/` (Decision 018): structured `ReframePlan`/`CameraKeyframe`, pure `CameraPath`, replaceable `ReframeFrameProvider`, deterministic `ReframeRenderer`, a deterministic natural-language `ReframeIntent` boundary, and an end-to-end `ReframePipeline`. The automated suite is 180 passed / 0 failed / 0 skipped.

The Phase 3 Objective 5 (Application-level player lifecycle) entry in `NEXT_TASK.md` remains valid history but must not be started mechanically while the 360 priority is active.

Target/subject resolution is also implemented under `app/target/` (Decision 019): a replaceable `TargetDetector` seam (with a dependency-free subprocess adapter), pure 360 geometry, deterministic tangent-view coverage, a deterministic spherical tracker, and a resolver/track-planner that produce `ReframeTarget`s and `ReframePlan`s for the existing engine. A real detector now runs through that seam: `tools/detector_helper/` implements the `ProcessTargetDetector` protocol with OpenCV Zoo YOLOX (Apache-2.0) + OpenCV DNN on CPU, and real detection -> tracking -> plan -> flat render was verified on real 360 footage (Decision 020). The helper/model are optional and external; the normal unit suite is model-free and skips the real integration test unless configured. Structured creator identity and deterministic selection are implemented (Decision 021): `TargetIdentityRegistry` binds "me" to a creator-selected track and `TargetSelector` resolves multi-person references deterministically, feeding the existing planner/renderer. "Me" is geometric identity, not biometric: it does not re-identify after long absence or among similar people, and it reports unresolved/ambiguous rather than guessing. An optional appearance-based re-identification layer is implemented (Decision 022): `AppearanceProvider`/`ProcessAppearanceProvider` (external helper, no ML runtime) and `IdentityReidentifier` maintain an appearance profile and apply a documented precedence — explicit selection > tracker continuity > unique geometric continuation confirmed/vetoed by appearance > appearance-only single-strong re-acquisition > unresolved. It can never override an explicit creator selection and is not biometric. An optional audio/speaker evidence layer is implemented (Decision 023): `SpeakerEvidenceProvider`/`ProcessSpeakerProvider` (external helper, Silero VAD, MIT) plus `SpeakerTargetAssociator`, `SpeakerTimeline`, `SpeakerEvidenceAnalyzer`, and `SpeakerReframePlanner`. Audio is evidence and a selection signal only: `TargetIdentityRegistry::annotateSpeaker` never changes identity resolution, and the precedence remains explicit selection > tracker continuity > geometry > appearance > audio > unresolved. Automatic audio-visual attribution without an explicit creator binding, GPU optimization, and UI integration remain future objectives.

Do not assume that planned architecture has already been implemented; read `CURRENT_STATE.md` and `NEXT_TASK.md` for exact status.

A further distinction now applies. **Objective 18 (Deterministic Intent -> Plan Contract Checker) is IMPLEMENTED** (Decision 035): `app/reframe/ReframeContract.{h,cpp}` is invoked after final temporal application on both the main and speaker paths in `ReframeCommandRunner::prepare()`. It is a pure two-argument function over the final `(ReframeIntent, ReframePlan)` pair with three FATAL rules (IPC-1 output fidelity, IPC-2 requested range containment, IPC-3 temporal materialisation). All three currently hold by construction, so the checker is a contract guard against future divergence rather than a fix for an existing defect. Keep it pure: no I/O, persistence, planner provenance, schema change or new dependency may be introduced, and the keyframe-count/direction rules remain deliberately excluded.

An end-to-end 360 **user-command execution** path is implemented (Decision 025): `app/reframe/ReframeCommandRunner` composes the parsed instruction, the replaceable detector/tracker, identity/selection, the validated `ReframePlan`, and the deterministic renderer behind a single entry point (`prepare()` = decision stage; `run()` = decision + execution). Unresolved or ambiguous subject references are reported, never fabricated; a direction-only command needs no detector. The normal suite stays model-free; an env-gated `realUserCommandIntegration` test exercises the path on real footage.

That command path is now reachable from the application (Decision 026): `Application::runReframeCommand()`/`runReframeCommandTo()` select the active media, validate application state and the output location, delegate to `ReframeCommandRunner`, and emit a structured `ReframeCommandOutcome`; a minimal `MainWindow` command input/range/run/result surface and `main.cpp` wiring complete the user-visible flow. The detector and frame provider are optional, non-owned inputs (a `ProcessTargetDetector` is built from `REELCRAFT_TARGET_*` when configured). Generated renders are session state (not yet persisted), and range-less commands use a caller-supplied fallback range until duration metadata exists.

Objective 10 (Decision 027) closed both gaps: a replaceable `MediaDurationProbe`/`FfprobeDurationProbe` seam (external ffprobe, no linked codec) resolves a zero ("whole clip") range to `[0, durationMs]`, and generated renders are persisted as an additive `reframeOutputs` project section (`CurrentSchemaVersion` 3) surfaced through `reframeOutputsChanged`. The application still never modifies the source media, and the deterministic command runner is unchanged.

Speaker-aware commands are implemented (Decision 028, Objective 11): `ReframeCommandRunner` recognizes speaker references ("follow the speaker", "keep the speaker centered") and reuses the Objective 6/7 `SpeakerEvidenceAnalyzer` + `SpeakerReframePlanner` to turn the associated speaker timeline into the deterministic plan. Audio is evidence: an optional `SpeakerEvidenceProvider` is required, explicit creator `speakerId -> targetId` bindings are honoured first, and an unassociated/ambiguous speaker or a mixed speaker/subject/direction command is reported honestly. `ReframePipeline::renderPlan()` renders an already-validated plan so the speaker plan is not re-derived. `main.cpp` builds a `ProcessSpeakerProvider` from `REELCRAFT_SPEAKER_*` when configured.

The 360 command workflow is now usable from the application (Decision 029, Objective 12): `Application::selectCreatorTargetFromViewport()` seeds the creator "me" identity from the viewport direction and preview time (passed into every command request, cleared on new/open project), and `Application::previewReframeOutput()` decodes the first frame of a persisted render record through an injectable decoder seam (defaulting to the external-FFmpeg `FrameExtractor`) for flat viewer presentation. The minimal UI adds creator-selection controls, a render-preview action, and a provider status line. The creator selection is session state, and continuous playback remains the deferred Phase 3 player-lifecycle objective.

Compound natural-language editing is now implemented (Decision 032, Objective 15): the existing `ReframeIntentParser` strips a temporal clause's time ranges (or a target-duration phrase) before camera/target extraction, so a single command such as "Keep 0:00 to 0:30 and follow me." preserves both the temporal edit and the target/reframe instruction. `ReframeIntent::hasCompoundEdit()` makes the composition explicit, and the deterministic default rule is that the camera/target applies to the entire retained temporal range. Supported compositions are temporal + target, temporal + explicit target, temporal + speaker, and target-duration + target; unsupported compositions (a camera instruction with its own separate time interval) fail honestly, and invalid/ambiguous/contradictory temporal edits remain honest. `TemporalEditPlan`, `ReframePlan`, the renderer, and Objective 13 playback are reused unchanged.
Deterministic temporal editing is now implemented (Decision 031, Objective 14): `app/reframe/TemporalEditPlan.{h,cpp}` is a validated, JSON-serializable Keep/Remove/TargetDuration value independent of parsing; `ReframeIntentParser` produces `ReframeIntent::temporalEdit` from "cut from X to Y", "remove X to Y", "keep X and Y", "make a N-second version", and "this section"; `ReframePlan` gained an additive ordered `segments` list (empty = the existing single source range) that changes only frame timing, so the existing `ReframeRenderer`/`ReframePipeline` render the retained ranges in order with no new architecture; `ReframeCommandRunner` resolves the edit against the known source duration and composes it with target/identity/speaker resolution; and `Application` supplies the probed duration and persists the retained segments in `ReframeCommandOutcome`, which plays through Objective 13. Invalid, ambiguous, contradictory, out-of-bounds, and duration-unknown requests are reported honestly and no timestamp is invented; a timeline editor, transitions, captions, and audio editing remain out of scope.

Rendered-result playback is now implemented (Decision 030, Objective 13): `Application` owns the playback `FrameSource`/`FramePump`/`Player` for a selected persisted render (default `FfmpegFrameSource` opened with the record's geometry) and drives `Player::tick()` from its own `QTimer`; the Player owns no event loop. `startReframeOutputPlayback`/`pauseReframeOutputPlayback`/`resumeReframeOutputPlayback`/`stopReframeOutputPlayback` are deterministic, frames are emitted as `reframePlaybackFrameReady` and presented flat, and the existing `FrameSource`/`FramePump`/`Player`/`Playhead`/`Clock`/`PacingPolicy` seams are reused (no parallel playback architecture). Playback is limited to persisted rendered results; general/active-media playback, audio, timeline editing, and duration metadata remain out of scope, and the Objective 10 preview-time contract is unchanged.

Real **360 source playback** is now implemented (Decision 036, Objective 19): `Application::startSourcePlayback()`/`pauseSourcePlayback()`/`resumeSourcePlayback()`/`stopSourcePlayback()`/`seekSourcePlayback()`/`tickSourcePlayback()` play the **active media** continuously through the existing persistent-subprocess seams (`FfmpegFrameSource` + `FramePump` + `Player`), never one decoder per displayed frame. Playback decodes a bounded 2:1 proxy (1024x512) that FFmpeg scales inside the decoder, so viewing cost is independent of the source resolution; the original media stays read-only and aspect is preserved by letterboxing. Seeking reopens the stream with an input seek and preserves whether playback was running; the source frame rate is read once per open through the existing ffprobe seam. Presentation reuses the projection-routed preview path, so the 360 viewpoint controls keep working while the footage plays. Source playback and rendered-result playback (Objective 13) are separate pipelines sharing the single event-loop timer and are mutually exclusive. **Audio is deferred** (Decision 036): Reelcraft has no audio output, so source playback is video-only.

Rendering is no longer process-per-frame (Decision 037, Objective 20). `ReframePipeline` now uses `app/reframe/ReframeStreamFrameProvider`, which keeps one persistent `FfmpegFrameSource` open at an **anchor** timestamp and answers subsequent requests by reading forward from it inside a bounded 3000 ms sequential window; a far-forward or backwards jump, a stream failure, the end of the media, or an unknown frame rate re-anchors. The positioned seek path remains the fallback and the geometry-discovery path, so the worst case is the previous cost and never a different frame. Frame identity with the previous path is a proven contract (decoded frames and byte-identical MP4 containers are compared for continuous, trimmed and multiple disjoint segments), and the source frame rate is read once per source through the existing ffprobe seam. Measured on a real 360 render of 6 output frames, FFmpeg process creations fell from 9 to 5 with no decoder scaling with the frame count. **Do not regress this**: no render-path change may reintroduce a decoder process per output frame, and any change to the streaming provider must keep the seek fallback and the frame-identity equivalence test green.

**Build-integrity rule (learned the hard way during Objective 20):** `reelcraft/` and `tests/` each have their own generated `Makefile`. After adding, renaming or removing a source or header, re-run qmake in **both** directories and run `make` twice (the second pass must build nothing). A makefile that predates a new header can omit it from an existing translation unit's dependency list, producing a binary that mixes two revisions of the same class — the observed symptoms were a `*** stack smashing detected ***` SIGABRT and a spurious frame-selection mismatch in otherwise-correct code. `Makefile` files are git-ignored, so this cannot be fixed by committing them. See `DEVELOPMENT_ENVIRONMENT.md` and `KNOWN_ISSUES.md`.

Real **persistent media analysis** now exists (Decisions 038-042, Objective 21). `app/analysis/MediaAnalysis.{h,cpp}` is the versioned artifact holding what Reelcraft learned about ONE piece of media: a small closed envelope (addressing, provenance, lifecycle, coverage, confidence) over named, **independently versioned capability layers**. `app/analysis/MediaAnalysisRunner.{h,cpp}` performs one whole-video pass with a **single persistent decoder** at a configured **perception resolution** -- deliberately not the viewer's 1024x512 display proxy and never the source resolution -- sampling on an interval and recording both the resolution and the interval with the layer. Two capabilities exist: `technical` (deterministic, from the existing ffprobe seam; facts it cannot determine are named, never defaulted) and `targets` (the existing `TargetResolver` + `SphericalTargetTracker` + equirect coverage, unchanged, persisted as normalized spherical observations and tracks). With no detector configured the `targets` layer is recorded **Unavailable with a reason** and opens no decoder -- a capability that cannot run is never an empty successful result.

Four invariants govern any future work here, and breaking any of them is a stop-and-report condition:

1. **Analysis is evidence, never editorial decision.** There is no plan, cut, keyframe or A-roll/B-roll label in the artifact. The test: if two creators with different intents could disagree, it is reasoning, not analysis. Reasoning consumes `(analysis, project context, intent)` and produces only the existing validated `ReframePlan`.
2. **Analysis is never on the deterministic replay path.** `replayEditDecision()` consumes `EditDecision::plan()` and nothing else; deleting every analysis artifact must leave rendering, replay and project loading fully functional. `replayIsIndependentOfMediaAnalysis` enforces this behaviourally and must stay green.
3. **A capability that cannot run is `Unavailable` with a reason -- never an empty result.** `Unavailable`, `Failed` and an empty successful layer are three different things. Any layer claiming evidence must carry the time ranges it actually covers, because "nothing was found at 07:30" must stay distinguishable from "we never looked at 07:30".
4. **Analysis is derived data owned outside the project file.** The project stores only a small `analysisRefs` reference (schema stays 3). Missing, unreadable, mismatched, stale or source-changed artifacts are reported STATUSES, never project corruption. Anything a build cannot interpret is preserved verbatim and re-emitted byte-for-byte.

Also note: the source-reference vocabulary is **shared** -- `core/MediaSourceReference.{h,cpp}` defines the reference, the `Matches`/`FileMissing`/`FingerprintMismatch` status and the fingerprint comparison, and both `EditDecision` and `MediaAnalysis` use it through type aliases. Add new artifacts to that vocabulary rather than duplicating it.

**Deliberately NOT built, and not to be assumed:** transcript/dialogue, diarization, scene or shot segmentation, quality/salience metrics, B-roll reasoning, LLM or editorial reasoning, Creator Memory / creator-preference learning, embeddings or semantic search, generated coverage, and multi-source editing. **Creator Memory has no decision recorded** -- it is a future architectural topic, deliberately kept separate from analysis because it is cross-project, non-regenerable and personal, whereas analysis is per-media, regenerable and derived. The next natural checkpoint is the **Analysis -> Reasoning boundary**, which must be reviewed before implementation.



Subject-**follow** instructions now produce a real camera path (Decision 043, Objective 23). `ReframeCameraMove::followSubject` is set by `ReframeIntentParser` for follow-class clauses ("follow X", "keep me centered", "keep X centered") and left clear for aim-class clauses ("look at X", "move to X", "centered on X", "center X"); `ReframeCommandRunner::prepare()` then builds the plan with the existing `TargetTrackPlanner::planTrack()` from the subject's resolved track instead of the single direction `ReframePlanBuilder` derives. `ReframeIntent` is **not persisted**, so this flag carries no schema impact.

Rules to preserve: only a **single target-referencing follow move** takes the track path; explicit directions, multi-move camera paths, speaker commands and direction-only commands must keep their existing behaviour; **aim instructions must stay a single fixed direction** (locked by `reframeCommandRunnerResolvesSubjectAndBuildsPlan` asserting exactly one keyframe); and the ordinary builder must keep running first, because it is both the validity gate and the fallback — an unusable track degrades to the old fixed camera with an honest note, never to a fabricated path.

Two recorded limitations, neither fixed here. First, a follow path has at most as many keyframes as the resolver has samples (five by default), so "smoother following" means denser resolution sampling — a caller/config concern (`resolveTimestamps`, `maxResolveSamples`), which is the next quality step for the documented trajectory-to-camera-path candidate. Second, and worth knowing before touching perception: on real footage the covering-view detector can report **one subject from two overlapping cover views** whose spherical centroids are further apart than the tracker's default 8-degree `mergeDistanceDeg` (measured at 9.6 degrees for a synthetic subject much larger than a person). The tracker then keeps two identities and subject references honestly resolve as *ambiguous* rather than guessing. Tuning that threshold for real person-sized targets is Decision 019 territory and needs its own evidence; do not change it as a side effect.



Follow instructions now sample the subject's trajectory at their own temporal resolution (Decision 044, Objective 24). `ReframeCommandRequest::followSampleIntervalMs` (default 250 ms) and `followResolveSamplesMax` (default 24) are used **instead of** `maxResolveSamples` **only** when the command is a follow command; `followSampleCountFor()` derives the count from the range, the interval and the cap. Aim, direction and speaker commands must keep `maxResolveSamples` and their existing behaviour and cost, and an explicit `resolveTimestamps` list must keep winning over both budgets — `reframeCommandRunnerFollowSamplingDensity` locks all three down.

It is an **interval, not a count**, deliberately: a count degrades with duration (25 samples over a 60 s clip is a 2.5 s step) while an interval holds resolution constant and lets the cap bound cost. The shared budget is the thing to avoid re-introducing.

Two measured facts worth keeping. First, density maps one-for-one into camera keyframes, and the **angular span is identical at every density** (78.7° on the fixture) — so this mechanism changes temporal resolution and never the trajectory; if a future change makes the span density-dependent, something else has started happening. Second, density also protects **association**: 2000 ms steps move the fixture subject 40° per step, past the tracker's 25° gate, and the subject splits into separate identities (resolving as honestly ambiguous), while 1000 ms steps associate cleanly. When a follow command refuses as ambiguous, sampling spacing is a legitimate thing to check before suspecting perception.

Recorded follow-up, deliberately not done here: resolution still opens one decoder process per sample when no frame provider is injected (~2.5 s each on this device). Reusing a persistent decoder for resolution is a resolution-seam change and belongs to its own objective. Also still open and untouched: the Objective 23 duplicate-identity / `mergeDistanceDeg` finding, which awaits its own Decision 019 investigation.



Trajectory resolution now runs through the **persistent decoding stream** (Decision 045, Objective 25). When no frame provider is injected, `ReframeCommandRunner::prepare()` constructs `ReframeStreamFrameProvider` with a frame rate read once through `FfprobeDurationProbe` — the same pattern `ReframePipeline::renderPlan` has used since Objective 20 — instead of `FfmpegSeekFrameProvider`. Nothing else about the resolution seam changed: it is still a `ReframeFrameProvider`, still replaceable, and an injected provider is still used verbatim and never routed through FFmpeg.

Why: on this device a single FFmpeg invocation costs ~2.5 s **whatever the frame size** (a 360x180 frame and a 4K frame measure the same), because process start-up and proot's syscall cost dominate. Process count is the lever, not pixels — worth remembering before optimising any other decode path here. Measured with a counting wrapper: six samples cost six processes / 15.2 s on the seek path and three processes / 10 s on the persistent path, with identical observations, keyframes and exactly equal keyframe angles. The persistent provider's process count is bounded by anchors (one geometry decode plus one stream open per bounded window), not by sample count, so the saving grows with density.

Do not regress this: the follow sampling behaviour (`followSampleIntervalMs` 250 ms, `followResolveSamplesMax` 24), the explicit `resolveTimestamps` override, aim/direction sampling, injected-provider behaviour and the fallback semantics must all stay as they are, and `reframeResolutionReusesDecoderProcesses` plus the Objective 20 stream-provider tests (frame equality, jumps and fallback, bad input and end of source, lifecycle) lock them down. Failure handling is inherited, not new: an unknown frame rate, a failed stream or the end of the media falls back to the positioned seek, so the worst case is the old cost and never a different frame.

Two measured caveats: within a bounded window the stream decodes intervening frames at native resolution (reasoned to be cheaper, since per-invocation cost is start-up rather than decoding, but not measured on a 4K source here), and resolution now performs one extra ffprobe call per pass.

Still open and untouched: the Objective 23 duplicate-identity / `mergeDistanceDeg` finding (its own Decision 019 investigation), and the deterministic smoothing/framing layer over the trajectory.



Follow paths are now **smoothed** inside `TargetTrackPlanner` (Decision 046, Objective 26), controlled by `Config::smoothingWindow` (default 5; **1 disables**). The window is **centred and shrinks symmetrically at the ends** — that detail carries three properties the follow path depends on, all of which are asserted: constant-velocity motion is reproduced exactly (no lag, no flattening), no smoothed value can leave the range of the values averaged (no overshoot), and keyframe times/count/ordering are never touched (start/end timing and span preserved). Yaw is unwrapped before averaging and re-normalised after, so the ±180° boundary is traversed the short way; pitch is averaged linearly and clamped.

Two rules before touching this. First, **it belongs in the planner** and only there: `planTrack` is called by the follow path alone — the speaker planner builds its own keyframes — so smoothing there cannot leak into aim, direction, speaker or multi-move commands, and a separate layer would be an abstraction with one caller. If a future change makes another command reach this planner, smoothing becomes a shared behaviour and must be reconsidered explicitly. The second rule is the synthetic-window choice: do not replace the symmetric shrink with a clipped window, because a one-sided neighbourhood is exactly what introduces lag, and `targetTrackPlannerSmoothsJitterAndPreservesMotion` (where a linear ramp must come through unchanged) plus the pre-existing `targetTrackPlannerBuildsFollowPlan` would both catch it.

**Framing is deliberately centred and must stay that way.** "follow the person", "keep me centered" and "keep X centered" all mean centred framing; a lead-room or rule-of-thirds offset would silently change what those commands ask for. The framing envelope remains the keyframe field of view. Do not add cinematic behaviour here without a decision that changes the command semantics.

Measured: a ±20°-per-sample wobble drops from a 40° maximum step between keyframes to 4°, while a linear ramp and a stationary subject are unchanged. Cost is negligible — no decode, detection or FFmpeg work is added.

Still open and untouched: the Objective 23 duplicate-identity / `mergeDistanceDeg` finding (its own Decision 019 investigation).



Trajectory duplicates from **covering views** are now consolidated by the footprint each observation already reports (Decision 047, Objective 27), closing the Objective 23 finding that Decisions 043-046 each recorded as open. `SphericalTargetTracker` now merges two observations when their centroids are within `mergeDistanceDeg` **or** when `separation <= a.yawRadiusDeg + b.yawRadiusDeg`, through the predicate `sameTargetAcrossCoveringViews` in `app/target/SphericalTargetTracker.cpp`. `mergeDistanceDeg` itself is **unchanged (8.0°)**.

Why this rule and not a wider threshold: the covering plan overlaps its views deliberately, so a subject near a boundary is seen twice, and the neighbour sees a **clipped** silhouette whose box centre is pulled toward that neighbour's axis. Measured on the reproduction: one report at yaw 6.20° / yawRadius **11.68°**, the other at yaw 15.82° / yawRadius **1.35°**, centres **9.61°** apart. The displacement is a property of where the boundary cut the box and it is **unbounded** — it grows with apparent size — so no constant can absorb it without falsely declaring that people are never that far apart. Raising `mergeDistanceDeg` to 24° was the Objective 23 workaround and it is exactly what this objective removed.

Rules to preserve. First, **do not "fix" this by raising the merge distance**: the new test's contrast block raises it to 24° and shows it merging two genuinely separate subjects, which the default now keeps apart. Second, **over-merging is the risk this change was judged on**, and it is asserted: two 3°-radius subjects 12° apart stay **2 identities**, subjects at ±40° stay **2**, a single subject stays **1**. Third, the extension is conditioned on reported size, so a **zero-radius** observation (injected doubles, synthetic unit observations) can only ever satisfy the original distance test — keep it that way, or injected behaviour changes silently. Fourth, only the **yaw** footprint is used, deliberately: yaw is the axis covering views are laid out along and the axis the clipping was measured on.

The four `mergeDistanceDeg = 24.0` overrides that Objections 23-26 carried in the suite have been **deleted**, so the follow, sampling-density, decoder-reuse, smoothing and real-media pipeline tests now run at the shipped default. If one of them fails, suspect the default path, not a missing override. (The only remaining 24.0 in the suite is the deliberate contrast inside `coveringViewMergeDoesNotOverMerge`.)

Nothing else changed: no covering-view coverage, field of view, view count, projection geometry, detector, association-gate, identity-selection, sampling-density (Objective 24), smoothing (Objective 26), renderer, FFmpeg/decoder or persisted-schema change; no new dependency and no model.

Rendered 360 -> flat output now **preserves the source audio** (Decision 048, Objective 28). The audio is taken over the plan's retained source spans — `ReframePlan::segments()` in order when present, otherwise `sourceRange()` — trimmed per span, concatenated, and bounded to the rendered picture; `ReframeRenderer::encodeVideoWithAudio()` implements it and `ReframePipeline::renderPlan()` decides it from a FACT reported by `MediaDurationProbe::streamSummary()` (an optional injection point, defaulting to the external ffprobe probe).

Rules to preserve. First, **the picture must keep going through the unchanged `encodeVideo` path**: pass 1 writes the video-only file and pass 2 remuxes it with `-c:v copy`, which is what makes a video-only source's container byte-identical to the standalone encoder's output and keeps every Objective 16/20 equivalence guarantee meaningful — `reframeRenderSilentSourceStaysSilent` asserts that byte-equality directly. Second, **the audio is always re-encoded, never stream-copied**: a copy cannot be trimmed to an arbitrary span and would tie the output to the source codec. Third, **the concatenated audio must stay bounded to `frameCount/fps`**: `framesForRange` floors, so an unbounded span leaves an audio-only tail past the last frame (measured: 3000 ms container for 2000 ms of picture). Fourth, **the audio map must stay required (`[aout]`, not `1:a?`)**: a source whose audio cannot be produced must fail loudly rather than quietly rendering silent. Fifth, **absence of audio is not a degradation**: a video-only source renders the previous output with no error and no note, while an unusable *probe* renders the same output *with* the reason recorded in `Result::notes` — `reframeRenderUnusableAudioFactsDegradesHonestly` locks the second and the first is locked by the empty-notes assertion in the silent-source test.

Determinism: equal **decoded** video and equal **decoded PCM audio** for repeated renders and for replay (`reframeRenderPreservesSourceAudio`, `replayReproducesRenderedAudio`). Container byte-equality is asserted only where it is valid — the video-only path — because a container that carries a stream is a different container. Recorded limitations: spatial audio is carried unrotated, and Reelcraft still cannot *play* audio (in-app playback stays video-only, Decision 036).


Reframe instructions can now set the **lens**, not only the direction (Decision 049, Objective 29). A framing clause ("zoom in", "go wide", "close-up", "field of view 60", "slightly closer") sets `ReframeCameraMove::hasFieldOfView` / `fieldOfViewDeg` in the parser; `ReframePlanBuilder` puts it on the keyframe it already builds, the lens **persists** until another instruction changes it, and a clause carrying only framing becomes a real move that changes the lens while the camera holds its direction. Follow commands pass the lens into `TargetTrackPlanner::Config`; speaker commands pass a single requested lens into `SpeakerReframePlanner::Config`. Contract rule **IPC-4** requires that every requested field of view is REACHED by the final plan.

Rules to preserve. First, **the framing phrase is consumed before direction detection**: `directionFromClause` reads "back" as a half-turn, so `pull back` and `back up` would otherwise turn the camera 180° while widening the lens — `reframeIntentParsesFraming` asserts that `pull back` has a lens and no direction. Second, **a trailing framing clause must keep being stripped from a subject capture** (both for "and" and for a bare comma, since neither separates clauses): without it `follow me and zoom in` captures `me and zoom in` as the subject and resolves as ambiguous. Third, **word boundaries are load-bearing**: `wide` must never match `widescreen`, which is an output aspect mapped by `applyAspect`. Fourth, **the default lens of every path is unchanged** (builder and target planner 90, speaker planner 75), so an instruction that does not mention framing produces exactly the plan it always did. Fifth, **IPC-4 is containment, not equality**: a lens change legitimately starts from the previous lens, so the rule checks that each requested value is reached, and changing it to an all-keyframes equality would false-positive on `start wide, then push in on me`. Sixth, **the lens change on the speaker path is refused, not approximated** — that planner owns its keyframes and cannot express a lens change.

Recorded limitations: the vocabulary is a fixed ladder plus explicit numbers (no "2x" multiplier, no continuous dial); framing offsets (lead room / rule of thirds) remain deliberately absent (Decision 046); multi-subject framing is implemented for exactly two subjects by Objective 30 (Decision 050). Nothing about perception, identity, timing, audio, replay or any persisted artifact changed: the lens lives in the plan Reelcraft already stores.


Reframing can now frame **two subjects at once** (Decision 050, Objective 30). "keep both of us in frame" / "keep both people in frame" sets `ReframeCameraMove::subjectGroup` (`CreatorAndOther` / `TwoPeople`) in the parser — a GROUP, never tracks — and `ReframeCommandRunner` resolves it at command time through the existing selector/identity rules, then builds ONE framing through `TargetTrackPlanner::planTracks()` using `enclosingFramingDeg()`.

Rules to preserve. First, **the group is resolved, never fabricated**: "both of us" is {"me", "the other person"} and "both people" requires EXACTLY two visible people; a third person visible is ambiguity reported with candidates, not a silent choice of two. Second, **the framing rule is the conservative inversion of the renderer's own basis** (`|sin t·cos p| ≤ tan(v/2)·aspect`, `|sin p| ≤ tan(v/2)`), with the aim at the MIDPOINT of the two reported footprints — do not introduce a preference for one subject, a lead-room offset, or a constant minimum separation; the footprints and the output aspect are what drive the lens. Third, **yaw is unwrapped around the first subject** so a pair straddling ±180 is framed the short way round; `reframeMultiSubjectFramingGeometry` asserts the aim is at ±180, not 0. Fourth, **framing exists only at timestamps where EVERY requested subject was observed** — nothing is interpolated or invented — and the lens is the widest requirement of the whole instruction so no subject leaves the frame at any joint sample. Fifth, **nothing may be dropped or clamped**: an unobserved subject, subjects never seen together, a pair that cannot fit inside 140°, or a named lens narrower than the requirement each fail with a specific reason (`reframeCommandRunnerRejectsUnsatisfiableMultiSubject`). Sixth, the pair path is deliberately **not smoothed** — the existing symmetric smoothing is per-direction only and could move the camera off the contained framing — so do not apply `smoothingWindow` to `planTracks` without a containment-preserving design.

**The contract checker deliberately does NOT check this.** By Decision 035 the plan retains camera coordinates only (no subject identity, no subject geometry), so containment is undecidable from `(ReframeIntent, ReframePlan)`. Do not add a cosmetic IPC-5 that only asserts "two ids resolved"; the guarantee is enforced in the command runner before planning and asserted by tests that evaluate the exact camera-basis containment condition at every keyframe.

Recorded limitations: exactly two subjects; a pair observed together only once yields a single static framing; joint framing requires exact timestamp agreement between the two tracks (what one resolver pass produces); framing offsets and multiplier zoom remain absent. Single-target reframing, follow (with smoothing), aim, speaker commands, temporal editing, audio, replay and every persisted artifact are unchanged.

---

# 4. Current Development Camera

The current development camera is the Insta360 X5.

This is a development target, not a permanent architectural dependency.

Reelcraft must remain camera-agnostic through adapter boundaries.

Do not redesign the core architecture around the X5 alone.

---

# 5. Core Architectural Rule

The most important system boundary is:

AI reasoning
 structured edit plan
 validation
 deterministic media execution

AI decides what should happen.

The deterministic media system performs the actual media operations.

AI must not directly modify source media.

---

# 6. Source Media Protection

Original media is sacred.

Normal Reelcraft editing operations must be non-destructive.

Do not:

- Overwrite source footage
- Delete source footage
- Modify original media as part of an edit
- Assume generated files are safe replacements for originals

Use project/edit instructions, derived media, proxies, and renders as separate artifacts.

If a task could affect original media, stop and verify the requirement before proceeding.

---

# 7. One Active Objective

Only one meaningful development objective should be active at a time.

Every task must define:

- Objective
- Scope
- Non-goals
- Definition of Done
- Verification requirements

Do not combine unrelated features or refactors into the same task.

If the requested change conflicts with the active objective, stop and resolve the conflict before proceeding.

---

# 8. Inspect Before Changing

Before modifying an existing file:

1. Read the relevant file.
2. Inspect related files when necessary.
3. Inspect Git status.
4. Determine whether the file is already part of working functionality.
5. Understand the current behavior.
6. Identify the smallest safe change.

Never overwrite existing work based solely on assumptions.

**Environment constraint (2026-09-17, aarch64 Termux/proot device):** the workspace filesystem denies `link()`, so any file tool that writes through an atomic temp-file + hard-link strategy fails with `EACCES` even though the directory is writable; write files with shell redirection (`cat > file <<'EOF'`) or an in-place editor (`python3`, `sed -i`) instead. `write`/`edit`-style tools are unusable on this host; see `KNOWN_ISSUES.md`.

---

# 9. Scope Protection

Do not make unrelated changes.

If a task says to implement one component, do not automatically:

- Redesign other components
- Upgrade unrelated dependencies
- Reformat unrelated files
- Refactor unrelated code
- Replace working technology
- Add speculative features

If an unrelated problem blocks the active task, document it and determine whether it should become a separate task.

---

# 10. Definition of Done

Do not claim a task is complete merely because code was written.

A task is complete only when:

- The requested implementation exists.
- The implementation matches the defined scope.
- Relevant tests pass.
- Existing functionality has not regressed.
- Appropriate manual verification has been performed.
- Documentation is updated.
- Git state has been reviewed.
- A checkpoint exists when required.

---

# 11. Testing Requirements

Testing should be proportional to the change.

Where applicable, perform:

- Unit testing
- Integration testing
- Regression testing
- Manual functional testing
- Error-path testing
- Performance testing

Media-related changes should be tested with representative media rather than assumed to work.

AI-related changes should be tested for malformed, unexpected, and incomplete model output.

---

# 12. Git and Checkpoints

Git is part of the development safety system.

Before meaningful or risky changes:

- Inspect Git status.
- Create or confirm a recoverable checkpoint when appropriate.

After meaningful changes:

- Review the diff.
- Verify the result.
- Commit the verified state.

Do not create a commit merely to hide an unverified or broken state.

Keep commits understandable and logically scoped.

---

# 13. Documentation Requirements

The following documents form the persistent project-control system:

- `MASTER_GUIDE.md`
- `CURRENT_STATE.md`
- `NEXT_TASK.md`
- `AI_HANDOFF.md`
- `AGENT_WORKFLOW.md` (canonical operating policy)
- `PROJECT_HISTORY.md`
- `DEVELOPMENT_LOG.md`
- `KNOWN_ISSUES.md`
- `DECISIONS.md`
- `CHANGELOG.md`
- `ARCHITECTURE.md`
- `DEVELOPMENT_ENVIRONMENT.md`

Relevant documentation must be updated when project state changes.

---

# 14. Architecture Protection

Do not make major architectural changes casually.

If implementation reveals that the architecture needs to change:

1. Identify the existing decision.
2. Explain why it needs reconsideration.
3. Document the evidence.
4. Record the new decision.
5. Identify affected systems.
6. Update architecture documentation.
7. Test affected functionality when implementation exists.
8. Record the change in project history.
9. Create a Git checkpoint.

**Load-bearing invariant (Objective 16, Decision 033):** the replay path must remain
**perception-free**. `EditDecision` and the `ReframePipeline::renderPlan` call graph must
not acquire `ReframeIntentParser`, `TargetDetector`, or any perception-provider symbol: a
persisted decision plus its source path is sufficient to reproduce a render, and that is
the property the artifact exists to guarantee. Verify at **object-code level** (`nm` /
`objdump` on the relevant objects), not by source grep alone — a provider symbol appearing
in that call graph is a design regression, not a test detail.


**Load-bearing invariant (Objective 17, Decision 034):** persisted decisions are
**immutable** and lineage is a **single-parent chain**. A revision is a NEW
`EditDecision` produced by `EditDecision::revisedFrom()`, referencing the decision it
revises through one `parentDecisionHash`; no mutator may be added for a stored decision,
and no graph, back-pointer, or traversal infrastructure should appear. Two further rules
are easy to break accidentally: `origin` and `parentDecisionHash` must stay **omitted when
unset** (writing them unconditionally invalidates the digest of every legacy decision), and
a loaded decision must **retain the schema version it was loaded with** rather than being
upgraded in place. Replay must never re-stamp `origin`. Enforced by
`editDecisionV1CompatibilityRetainsVersionAndHash` and `replayPreservesDecisionOrigin`.


**Objective 19 (Decision 036):** source-media playback exists and is **video-only**. `Application` owns a separate source-playback pipeline (own `FrameSource`/`FramePump`/`Player`) that shares the single event-loop timer with the rendered-result path; the two are mutually exclusive and the rendered path must not be unified with it casually. Playback decodes a bounded 2:1 proxy (1024x512) through `FfmpegFrameSource`, so per-frame cost is independent of source resolution and the original media is never modified. Audio is deliberately absent with a recorded follow-up — do not add an audio output subsystem as a side effect of another objective.

Never silently overwrite architectural history.

---

# 15. Technology Selection

Do not treat technology candidates in `ARCHITECTURE.md` as permanent commitments.

The project currently intentionally leaves several choices open, including:

- UI framework
- Application runtime
- Project file format
- Storage implementation
- AI providers
- AI models
- Local/cloud workload allocation
- Rendering architecture
- GPU strategy
- Database requirements
- Plugin architecture
- Packaging/deployment

Technology should be selected based on actual requirements, testing, performance, compatibility, maintainability, privacy, and cost.

---

# 16. AI Provider Independence

Do not build core project logic directly around one AI provider.

Provider-specific code should remain behind an abstraction boundary.

The project should remain capable of supporting different models and providers where practical.

---

# 17. 360° Requirements

360° video is a first-class capability.

Do not implement 360° functionality as an afterthought if the affected subsystem establishes foundational media behavior.

The architecture must eventually support concepts including:

- Equirectangular media
- 360° metadata
- Orientation
- Viewer orientation
- Virtual-camera positioning
- Reframing
- Keyframed viewpoints
- 360° export
- Flat-video reframing

---

# 18. Media Engine Boundary

The deterministic media engine is responsible for executing validated edit instructions.

AI agents should not bypass this boundary by directly manipulating media as a substitute for implementing the proper edit pipeline.

Tools such as FFmpeg may eventually be used internally, but higher-level Reelcraft components should communicate through defined media-engine interfaces.

---

# 19. Error Handling

AI output must be treated as untrusted structured input.

Before execution, validate:

- Schema
- Media references
- Timeline ranges
- Supported operations
- Required dependencies
- 360° parameters
- Rendering requirements

Invalid instructions should be rejected or corrected before execution.

---

# 20. Performance

Do not optimize blindly.

When performance becomes relevant:

1. Measure the current behavior.
2. Identify the bottleneck.
3. Make the smallest justified change.
4. Measure again.
5. Verify that correctness has not regressed.

Proxy workflows, GPU acceleration, caching, parallelism, and cloud processing should be introduced based on evidence and requirements.

---

# 21. Privacy and Security

Treat creator media and project information as potentially sensitive.

Do not expose source media, credentials, tokens, or private project information unnecessarily.

When implementing cloud AI or remote processing, explicitly consider:

- What data leaves the device
- Where it is processed
- How long it is retained
- What provider receives it
- What credentials are required
- What artifacts remain after processing

---

# 22. Current Task Procedure

When beginning a new task:

1. Read the project state.
2. Identify `NEXT_TASK.md`.
3. Confirm the objective.
4. Define or confirm the Definition of Done.
5. Inspect relevant implementation.
6. Plan the smallest logical change.
7. Implement incrementally.
8. Test after meaningful steps.
9. Perform regression checks.
10. Update documentation.
11. Review Git diff/status.
12. Create the appropriate checkpoint.

---

# 23. Stop-and-Ask Conditions

An AI agent must stop rather than guess when:

- Requirements are contradictory.
- A requested change conflicts with the architecture.
- The correct behavior is materially ambiguous.
- A change could destroy or overwrite original media.
- A change would require a major architectural decision that has not been evaluated.
- Existing functionality would need to be removed without explicit justification.
- A dependency or technology choice would create significant lock-in.
- Tests reveal an unexpected regression that cannot be safely resolved within scope.
- The task would expand substantially beyond its defined scope.

When stopping, clearly explain the conflict and identify the decision that needs to be made.

---

# 24. How to Determine Where to Continue

The AI agent should determine the current position using:

1. `CURRENT_STATE.md`
2. `NEXT_TASK.md`
3. Git status/history
4. Relevant architecture and decision documents
5. Actual project files

Do not infer progress from the existence of a planned document or feature.

A feature is implemented only when the repository contains the implementation and verification supports that conclusion.

---

# 25. Handoff Completion

Before ending a development session, update the persistent state so another agent can continue.

At minimum, ensure:

- Current task is accurate.
- Current state is accurate.
- Recent work is documented.
- Known problems are recorded.
- Relevant architectural decisions are recorded.
- Git state is understandable.
- The next logical objective is identified.

The next AI should be able to begin by reading the repository documentation rather than reconstructing the previous conversation.

---

# 26. Authority Order

When information conflicts, use this general order of authority:

1. Actual repository state
2. Verified test results
3. Current project documentation
4. Recorded architectural decisions
5. Current task definition
6. Previous development history
7. Chat instructions or assumptions

If the conflict cannot be resolved safely, stop and ask.

---

# 27. Final Principle

Reelcraft should be built deliberately.

The objective is not merely to produce code quickly.

The objective is to create a reliable, extensible, AI-native editing platform whose architecture, project state, and development history remain understandable to both humans and future AI agents.

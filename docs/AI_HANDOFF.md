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

# Reelcraft — Project History

## Purpose

This document records major milestones in the development of Reelcraft.

It provides chronological context for the project so future developers and AI agents do not need to reconstruct history from chat conversations.

---

# 2026-09-02 — Project Foundation

Reelcraft was established as an AI-native video editing platform focused on content creators, vloggers, and especially 360° video creators.

The long-term direction is to allow creators to provide footage and describe editing intent naturally while AI analyzes the media and produces structured editing decisions that the creator can review and control.

---

# 2026-09-02 — Documentation System Established

A persistent Markdown documentation system was established as the project's long-term source of truth.

The system is designed so future AI agents can continue development without depending on chat history.

Core documentation areas include:

- Master product vision
- Current project state
- Next development task
- AI handoff
- Architecture
- Architectural decisions
- Project history
- Development log
- Known issues
- Changelog
- Development environment

---

# 2026-09-02 — Initial Architecture Defined

The first formal technical architecture was documented.

Major subsystem boundaries were established for:

- User Interface
- Project / Edit Model
- Media Ingestion
- Camera / Media Adapters
- AI Orchestration
- AI Provider / Model Abstraction
- Deterministic Media Engine
- Rendering / Export
- Storage / Project Management

The core AI-to-media boundary was established as:

AI reasoning
 structured edit plan
 validation
 deterministic media execution

No implementation technology was permanently locked at this stage.

---

# 2026-09-02 — Controlled Development Workflow Established

The project adopted a controlled incremental development methodology.

The workflow requires:

1. One active development objective at a time.
2. Explicit scope.
3. Explicit non-goals.
4. Definition of Done.
5. Inspect-before-changing.
6. Incremental implementation.
7. Testing and regression verification.
8. Documentation updates.
9. Git review and checkpointing.
10. Stop-and-ask behavior when requirements or architecture conflict.

---

# 2026-09-02 — Git Repository Established

Git was initialized for the Reelcraft project.

The primary development branch was established as:

`main`

The first documentation foundation was committed as:

`b317aa1`

Commit message:

`docs: establish Reelcraft project foundation`

---

# 2026-09-02 — Architecture and Task Foundation Checkpointed

The following files were added and verified:

- `docs/ARCHITECTURE.md`
- `docs/NEXT_TASK.md`

They were committed with:

`docs: formalize architecture and development workflow`

This checkpoint established the formal architecture and active documentation objective.

---

# 2026-09-02 — Architectural Decision Record Established

`docs/DECISIONS.md` was created to preserve the reasoning behind significant architectural choices.

Initial decisions cover:

- AI/media execution separation
- Non-destructive editing
- First-class 360° support
- Camera adapters
- Replaceable AI providers
- Portable project state
- Human review authority
- Incremental development
- Inspect-before-changing
- Documentation as source of truth
- Evidence-based technology selection
- Controlled architecture evolution

Architectural changes must be documented rather than silently replacing previous decisions.

---

# 2026-09-02 — AI Handoff System Established

`docs/AI_HANDOFF.md` was created as the entry point for future AI development agents.

It establishes:

- Repository inspection requirements
- Documentation reading order
- Scope protection
- Testing expectations
- Git checkpoint requirements
- Architecture protection
- Stop-and-ask conditions
- Project-state verification
- Session handoff requirements

The goal is for a future AI agent to understand the project and continue work without reconstructing the previous conversation.

---

# Current Historical Position

Reelcraft has completed its initial project identity, architecture, documentation foundation, architectural decision record, and AI handoff system.

The application implementation has not yet begun.

The next stage is to complete the remaining project-control documentation and then define the first small implementation objective for Phase 1 — Foundation.

---

# Historical Recording Rule

Future meaningful milestones should be added chronologically.

Each entry should describe:

- What happened
- Why it mattered
- What changed
- What was verified
- What checkpoint or commit resulted

Do not rewrite history to make previous decisions appear as though they were always known.

If a major decision changes, preserve the previous historical record and document the reason for the change.

## 2026-09-02 — Product, Project, and AI Contracts Established

The conceptual foundation was expanded with three supporting contracts:

- `REQUIREMENTS.md` defines the product and system requirements while preserving technology flexibility.
- `PROJECT_MODEL.md` defines the conceptual structure and ownership of project state.
- `AI_EDIT_CONTRACT.md` defines the boundary between AI reasoning and deterministic media execution.

These documents reinforce the core architectural rule that AI determines intended edits while deterministic systems execute validated structured instructions.

The project also explicitly deferred creation of a substantial production application skeleton. Technology and runtime choices remain open and will be evaluated before implementation to reduce premature lock-in.

`NEXT_TASK.md` was updated to establish a focused technology/runtime evaluation as the next objective, and `CURRENT_STATE.md` / `DEVELOPMENT_LOG.md` were updated to reflect this transition.

This milestone does not represent production feature implementation.

## 2026-09-02 — Technology and Runtime Evaluation Completed

The focused technology and runtime evaluation was completed sufficiently to establish a validated technical direction for the first implementation stage.

The evaluation covered:

- Desktop runtime and UI approaches
- Deterministic media-processing integration
- AI integration and replaceable provider abstraction
- Voice interaction using the same intent architecture as text
- First-class 360° video requirements
- Local, cloud, and hybrid processing
- Large 4K/8K/360° media workloads
- Performance and GPU requirements
- Project state and storage
- Rendering and export
- Testing
- Packaging and deployment
- Integration, portability, dependency, licensing, vendor, and platform lock-in risks

The evaluation established several technology-agnostic requirements:

- Reelcraft requires a GPU-capable desktop application rendering layer capable of interactive 2D/3D content and future 360° presentation.
- The UI/application layer must remain separate from deterministic media processing.
- AI reasoning must produce structured, validated instructions rather than directly manipulating media.
- Original media must remain untouched.
- AI providers must remain replaceable.
- Voice and text should enter the same underlying intent architecture.
- Local, cloud, and hybrid processing must remain possible.
- Background processing must not unnecessarily block the interactive application.
- Project state must remain separate from original media and remain portable/recoverable.
- Automated testing and regression protection are required.

Qt 6, Electron, and Tauri 2 remain evaluated candidate approaches rather than permanent downstream commitments. Wails v3 remains a lower-priority/watchlist option because its current v3 status requires additional caution.

No production media engine, 360° editing pipeline, AI provider integration, voice editing system, or major application feature was implemented as part of this evaluation.

`DEVELOPMENT_LOG.md` records the detailed evaluation outcome. `CURRENT_STATE.md` now identifies validated technical direction as the current stage, and `NEXT_TASK.md` now defines the smallest Phase 1 application-foundation objective.

This milestone represents completion of the evaluation stage and transition into controlled Phase 1 foundation implementation planning. It does not represent completion of the Reelcraft application.

## 2026-09-06 — Phase 2 (360 Viewer Review & Navigation) Complete

Phase 2 was re-scoped by Decision 016 to "360 Viewer Review & Navigation": import and manage real media, select active media, deterministic single-frame preview, time stepping/seek, look-around orientation (keyboard and pointer), correct flat/equirectangular presentation, and consistent viewer/project state. Continuous/paced playback, duration-aware transport, and audio are explicitly deferred to a future playback/media-engine phase.

Objectives 1-15 were implemented and verified incrementally: viewer state/camera foundations, synthetic scene and equirectangular presentation (bilinear), media import/availability/library/active-media contracts, FFmpeg-CLI single-frame decode seam, time navigation, pointer orientation controls, projection declaration and flat routing, viewer-state consistency, and end-to-end real-media review-path validation.

Verification at closeout: 136 passed / 0 failed / 0 skipped; application and test builds succeed; offscreen smoke SMOKE_EXIT=124; clean-checkout verification of the closeout commit passes; working tree clean. Phase 2 formally closed 2026-09-06 (closeout commit; prior HEAD ae317f5). Phase 3 (Media Engine / playback decision) has not started.

## 2026-09-06 — Phase 3 Opening Decisions Recorded (Media Engine)

Phase 3 (Media Engine / continuous playback) was opened with a documentation-only decision objective after a read-only feasibility discovery was approved. Decision 017 records: opening architecture = persistent FFmpeg streaming subprocess behind a replaceable media/player seam (currently feasible path); FrameExtractor remains the deterministic single-frame preview/validation seam; linked FFmpeg libraries and QtMultimedia are deferred (no installation; revisit at a real desktop/deployment dependency decision); ffprobe duration metadata is reopened as its own future decision gate; the Objective 10 preview-time contract is preserved (playback extends it); and the architecture boundary is decode seam → player/timing → Application → Viewer, with timeline/editor/AI above Application. No media-engine/playback/duration/audio implementation exists. Phase 2 remains formally closed. Changes were documentation-only.

## 2026-09-16 — Phase 3 Objective 3: Media-Engine Foundation (Replaceable Source + Frame Pump)

Phase 3 continued with its first production media-engine foundation objective. An abstract, transport-agnostic decode/media-source seam (`FrameSource`) was introduced along with a persistent FFmpeg-subprocess implementation (`FfmpegFrameSource`: rawvideo/rgb24, caller-supplied geometry, deterministic bounded reads and process cleanup) and a synchronous, caller-driven `FramePump` that requests one frame per `advance()` and emits `frameReady`/`streamEnded`/`streamFailed`. No Application/UI playback, continuous/paced playback, duration metadata/ffprobe, audio, timeline, schema, or dependency changes were made; the Objective 10 preview-time contract is unchanged and `FrameExtractor` remains the single-frame preview/validation seam. The concrete interface, rawvideo transport, and pump mechanics remain implementation details under Decision 017 rather than new decisions. Verification: 144 passed / 0 failed / 0 skipped; application and test builds succeed; official build-and-test script green; offscreen smoke SMOKE_EXIT=124. Implemented and verified 2026-09-16.

## 2026-09-16 — Phase 3 Objective 4: Player/Timing Subsystem Foundation

Phase 3 added its second production foundation: the deterministic player/timing subsystem above the decode/media-source seam. `Playhead` provides clock-free position state (frame count, current index, presentation timestamp from an explicit frame interval); `Clock`/`SystemClock` and `PacingPolicy`/`DefaultPacingPolicy` are replaceable, injectable abstractions so playback is deterministic and tested without wall-clock delays; and `Player` is a `Stopped`/`Playing`/`Paused` state machine with `play()`/`pause()`/`stop()`, clock/pacing-driven `tick()`, explicit `stepOnce()`, and state/position/frame/end/error signals. The `FramePump` remains a passive, caller-driven decoder and is consumed only through its existing signals; the Player owns no timer or thread. No Application/UI playback wiring, continuous playback UX, duration metadata/ffprobe, audio, timeline, schema, or dependency changes were made, and the Objective 10 preview-time contract is unchanged. The design follows Decision 017, which already fixes the player/timing boundary and treats clock/pacing implementation as an implementation detail, so no new decision was recorded. Verification: clean application and test builds; 154 passed / 0 failed / 0 skipped (144 prior + 10 player/timing tests); official build-and-test script green; offscreen smoke SMOKE_EXIT=124. Implemented and verified 2026-09-16.

## 2026-09-17 — 360 Reframing Engine (Deterministic Vertical Slice)

The project's human-approved priority moved to making the core 360 editing/reframing capability functional rather than mechanically continuing the Phase 3 queue. A deterministic 360-to-flat reframing engine was implemented under `app/reframe/`: a structured, versioned, validated `ReframePlan`/`CameraKeyframe` boundary; a pure `CameraPath` evaluator (shortest-yaw, hold/linear, bounded); a replaceable `ReframeFrameProvider` seam with an FFmpeg-CLI implementation; a deterministic `ReframeRenderer` producing flat frames and an FFmpeg-encoded video; a deterministic natural-language `ReframeIntent` boundary; a `ReframePlanBuilder` that resolves named/explicit directions and reports unresolved subject references instead of fabricating them; and a `ReframePipeline` composing the full path. Source media remains read-only. Decision 018 records the architecture. Verification: 180 passed / 0 failed / 0 skipped (154 prior + 26 reframing tests), including a real end-to-end FFmpeg render; clean application and test builds; offscreen smoke SMOKE_EXIT=124. Target detection/tracking, AI-provider integration, plan persistence, and UI wiring remain future objectives.

## 2026-09-17 — 360 Reframing Objective 2: Target/Subject Resolution

Reelcraft gained the "eyes" behind its reframing engine. A replaceable target-resolution layer was implemented under `app/target/`: pure 360 geometry (`EquirectProjection`) that maps equirectangular and perspective-view pixels to spherical yaw/pitch using the exact inverse of the existing camera model; deterministic overlapping tangent-view coverage (`EquirectViewPlan`) chosen to avoid equirectangular pole/seam distortion; a replaceable `TargetDetector` seam with a dependency-free subprocess adapter (`ProcessTargetDetector`); a deterministic `SphericalTargetTracker`; a `TargetResolver` that produces honest spherical observations and reports unresolved queries; and a `TargetTrackPlanner` that turns tracks into validated `ReframePlan`s for the existing `CameraPath`/`ReframeRenderer`. Detected targets bridge to the existing planner through `resolvedTargets()`. The detection-technology and licensing evaluation is recorded in `docs/TARGET_RESOLUTION_TECHNOLOGY.md` (Ultralytics YOLO's AGPL-3.0 rejected as a default; YOLOX/RT-DETR/OpenCV Zoo/MediaPipe/ONNX Runtime recommended). Decision 019 records the architecture. Verification: 219 passed / 0 failed / 0 skipped (180 prior + 39 target-resolution tests), including model-free end-to-end detection -> direction -> ReframeTarget -> plan -> render; clean application and test builds; offscreen smoke SMOKE_EXIT=124. A real detection model, "me" identity, speaker localization, and UI integration remain future objectives.

## 2026-09-17 — 360 Reframing Objective 3: Real Detector Integration

Reelcraft's target-resolution boundary was connected to a real detector. An optional external helper (`tools/detector_helper/yolox_detector.py`) implements the existing `ProcessTargetDetector` file/JSON protocol using OpenCV Zoo YOLOX (2022nov, Apache-2.0 code and weights) and OpenCV DNN 4.10 on CPU; the C++ application still links no computer-vision library. On the project's real 3840x1920 equirect clip `360_TEST_4K.mp4` (a lightweight proxy was used for speed; the original was untouched), real detection found two presenters in tangent views at spherical yaw -27.9/pitch -28.6 and yaw +26.9/pitch -27.8 (independently reproduced), the strongest person track held id `t1` across five sampled frames (mean confidence 0.923), and the resulting `ReframePlan` produced a 640x360 H.264 clip visibly centered on the detected presenter. Decision 020 records the selection and the preserved boundary; the detection technology/licensing record is updated in `docs/TARGET_RESOLUTION_TECHNOLOGY.md`. The normal suite stays model-free (219 passed, 0 failed, 1 skipped); the real integration test passed separately. "Me" identity, speaker localization, GPU inference, and production tracking remain future objectives.

## 2026-09-17 — 360 Reframing Objective 4: Target Identity & Deterministic Selection

Reelcraft gained structured creator identity above detection/tracking. Under `app/target/`, `TargetIdentity` defines `CreatorTargetSelection`/`IdentityBinding` and a `TargetIdentityRegistry` that binds the canonical identity "me" to a tracker track id from a structured creator seed (direction/time or track id) and refreshes it against live tracks, re-binding only through unique geometric continuity. `TargetSelector` resolves a documented vocabulary ("me"/selected aliases, "the other person", "person N"/ordinals, left/right, exact track id, unique label) using a canonical order (first observation time, numeric track id, id string) and reports ambiguity rather than guessing. The tracker was hardened with bounded constant-velocity prediction (crossing) and a bounded re-entry gate (temporary loss/occlusion), still with no appearance model. On real 360 footage, 9 people were detected and "me" was bound to the strongest presenter (t1, 5 frames, mean confidence 0.923); `person 1` resolved to t1, `the other person` was honestly reported ambiguous with 8 candidates, and the bound track produced a `ReframePlan` and a 640x360 H.264 output centered on the selected presenter. Decision 021 records the architecture. Model-free suite: 240 passed / 0 failed / 1 skipped; real integration PASS. "Me" is geometric identity, not biometric; speaker association and appearance re-identification remain future objectives.

## 2026-09-17 — 360 Reframing Objective 5: Appearance-Based Re-Identification

Reelcraft gained an optional, replaceable visual-appearance layer above the geometric identity. Under `app/target/`, `AppearanceTypes` defines unit-normalized embeddings, cosine similarity, explicit accept/reject thresholds, and structured `AppearanceEvidence` verdicts; `AppearanceProvider`/`ProcessAppearanceProvider` keep inference in an external helper with no ML runtime linked into the C++ core; `TargetCropExtractor` produces deterministic crops via `EquirectView`; and `IdentityReidentifier` maintains an appearance profile and applies a documented precedence (explicit selection > tracker continuity > unique geometric continuation confirmed or vetoed by appearance > appearance-only single-strong re-acquisition > unresolved). The identity registry stores profiles and applies appearance decisions, recording vetoed tracks so geometry cannot silently re-accept them. The selected model is OpenVINO OMZ `person-reidentification-retail-0277` (Apache-2.0 code and weights, internal training data) via ONNX Runtime; OpenCV Zoo YoutuReID and OSNet/torchreid weights were rejected for unclear research-dataset weight licensing. Model-free suite: 264 passed / 0 failed / 1 skipped; real appearance inference and a controlled re-acquisition were verified on real 360 footage. Decision 022 records the architecture. Active-speaker association, GPU optimization, and UI integration remain future objectives.

## 2026-09-17 — 360 Reframing Objective 6: Optional Audio/Speaker Evidence

Reelcraft gained an optional, replaceable audio/speaker evidence layer above the identity/selection system. Under `app/target/`, `SpeakerTypes` defines structured speech intervals, per-target speaker evidence, timeline segments, and explicit verdicts; `SpeakerEvidenceProvider`/`ProcessSpeakerProvider` keep audio inference in an external helper with no runtime linked into the C++ core; `SpeakerTargetAssociator` deterministically maps provider speaker ids to existing visible target tracks (explicit creator binding, optional direction of arrival, or single-visible person; otherwise ambiguous/unassociated); and `SpeakerTimeline` applies documented hysteresis so rapid alternation does not thrash the camera. Audio never rebinds identity: `TargetIdentityRegistry::annotateSpeaker` records evidence only, and the precedence remains explicit selection > tracker continuity > geometry > appearance > audio > unresolved. `SpeakerReframePlanner` turns the associated speaker timeline into a `ReframePlan` with cuts at speaker changes, executed by the existing deterministic renderer. The selected provider is Silero VAD (MIT code and weights, local CPU via ONNX Runtime); pyannote, SpeechBrain/torchreid, cloud APIs, and audio-visual active-speaker models were rejected or deferred. Model-free suite: 295 passed / 0 failed / 1 skipped; real speech detection and speaker-associated rendering were verified on real 360 footage (audio+video proxy; original untouched). Decision 023 records the architecture. Automatic audio-visual attribution without an explicit binding, GPU optimization, and UI integration remain future objectives.

## 2026-09-17 — 360 Reframing Objective 7: Audio-Visual Provider Attribution Seam

Reelcraft completed the audio-visual attribution seam that Objective 6 left open, but deliberately did not ship an unreliable perception subsystem. `SpeakerInterval` and `SpeakerSegment` gained an optional `targetIdHint` — an existing target track id that a diarization or audio-visual active-speaker provider attributes to the speech — and `SpeakerTargetAssociator` honours a *visible* hint after an explicit creator binding (explicit > visible provider hint > spatial direction-of-arrival > single-visible-person > ambiguous/unassociated), recording the method as `provider-hint`; a hint for a non-visible target falls through and a hint never invents a target or overrides the creator. `SpeakerTimeline` carries the hint onto the coalesced segment and `SpeakerEvidenceAnalyzer` honours it end to end. The decision rests on recorded feasibility evidence: the real footage audio is mono (no direction of arrival), a face-detection + mouth-motion/audio-envelope correlation probe scored only 0.250 for the speaking presenter versus 0.192 for the other presenter, and no permissively licensed, clearly commercial audio-visual active-speaker model was identified in this environment; this confirms Decision 023's licensing finding. The result is a complete, model-free-tested seam (5 new tests; suite 300 passed / 0 failed / 1 skipped) that a future licensed provider can satisfy without touching the core, while the reliable "follow the speaker" path remains the Objective 6 explicit binding. Decision 024 records the seam and the feasibility boundary. Per the human-approved priority, the next objective is end-to-end 360 user-command testing rather than further perception subsystems.

## 2026-09-17 — 360 Reframing Objective 8: End-to-End User-Command Execution

Reelcraft gained a single end-to-end 360 command path. `app/reframe/ReframeCommandRunner` composes a user instruction into a validated `ReframePlan` and a rendered flat video: it parses the command (`ReframeIntentParser`), resolves subject references through the replaceable `TargetResolver` (detector + tracker), binds the optional creator identity and resolves references via `TargetIdentityRegistry`/`TargetSelector`, builds the validated plan (`ReframePlanBuilder`), and executes it through the unchanged `ReframePipeline`/`ReframeRenderer`. `prepare()` is a deterministic, model-free-testable decision stage; `run()` adds deterministic execution. The runner adds no perception of its own and never fabricates a subject direction — unresolved or ambiguous references are reported, and a direction-only command needs no detector. 8 new model-free tests bring the suite to 308 passed / 0 failed / 2 skipped, and a new env-gated `realUserCommandIntegration` test exercises the full command path on real 360 footage with the Apache-2.0 YOLOX detector. Decision 025 records the composition boundary. The next objective is Application-level 360 command orchestration (making the path usable from the product); it requires its own scoped objective.

## 2026-09-17 — 360 Reframing Objective 9: Application-Level 360 Command Orchestration

Reelcraft's 360 command path became reachable from the product. `Application` gained `runReframeCommand()`/`runReframeCommandTo()`, which resolve the active media, validate application state and the output location, construct a `ReframeCommandRequest`, delegate to the existing library-level `ReframeCommandRunner` (no duplicated logic), and expose a structured, JSON-serializable `ReframeCommandOutcome` (source reference, instruction, effective time range, output specification and path, frame count, notes, unresolved references, resolved target directions) through a `reframeCommandFinished` signal. The target detector and frame provider are optional, non-owned, replaceable inputs; `main.cpp` constructs a `ProcessTargetDetector` from the `REELCRAFT_TARGET_*` environment when configured, so subject-referencing commands work without linking an ML runtime into the core. The command executor is an injectable seam defaulting to `ReframeCommandRunner::run`, which keeps application tests model-free and deterministic. A minimal `MainWindow` surface (command input, start/end seconds, run button, result label) completes the path from user command to application-visible output, and the original media is never modified. 17 new model-free tests (plus a resolver-robustness test) bring the suite to 326 passed / 0 failed / 3 skipped, and a new env-gated `realApplicationCommandIntegration` test exercises the application path on real 360 footage with the Apache-2.0 YOLOX detector. Decision 026 records the orchestration boundary. Output persistence and duration-aware full-clip ranges remain future work.

## 2026-09-17 — 360 Reframing Objective 10: Persisted Outputs and Duration-Aware Ranges

Reelcraft's 360 command path gained persistence and duration-aware ranges, closing the two gaps Objective 9 recorded. A replaceable media-duration seam (`MediaDurationProbe` + `FfprobeDurationProbe`) reports a clip's duration with the external `ffprobe` — resolved via `REELCRAFT_FFPROBE`, a sibling of the resolved ffmpeg, or `PATH` — reading only and failing deterministically, so the application links no codec dependency. A zero start/end range now means "the whole clip" and is resolved through the probe to `[0, durationMs]`; explicit ranges never probe, and an unknown duration is reported honestly (or the command's own range is used). Generated renders are now persisted: every command that reaches an output target appends its structured `ReframeCommandOutcome` (success or failure with its error), `Project` gained an additive `reframeOutputs` section with `CurrentSchemaVersion` 3, the application emits `reframeOutputsChanged`, and the UI re-lists records and defaults its range controls to 0/0 (whole clip). 13 new model-free tests bring the suite to 339 passed / 0 failed / 3 skipped, and the env-gated `realApplicationCommandIntegration` now validates whole-clip probing and save/open persistence on real 360 footage with the Apache-2.0 YOLOX detector. Decision 027 records the duration-probe and persistence boundaries. Speaker-aware commands and GPU optimization remain future work.

## 2026-09-17 — 360 Reframing Objective 11: Speaker-Aware 360 Commands

Reelcraft's 360 command path learned to follow the speaker. `ReframeCommandRunner` now recognizes speaker references ("follow the speaker", "keep the speaker centered", "center the speaker", and related phrasings) and reuses the existing Objective 6/7 layers — `SpeakerEvidenceAnalyzer` (provider -> timeline -> association) and `SpeakerReframePlanner` (-> `ReframePlan` with cuts at speaker changes) — so no perception is duplicated and no parallel command system is introduced. Audio remains evidence: an optional, replaceable `SpeakerEvidenceProvider` is required for a speaker command, explicit creator `speakerId -> targetId` bindings are honoured first, and an unassociated or ambiguous speaker, or a command mixing a speaker reference with another subject or an explicit direction, is reported honestly with no fabricated plan. A new `ReframePipeline::renderPlan()` renders an already-validated plan with the same deterministic renderer (and `run()` delegates to it), so the speaker plan is executed rather than re-derived. `Application` holds a non-owned speaker provider and optional bindings and copies them into each request, and `main.cpp` builds a `ProcessSpeakerProvider` from `REELCRAFT_SPEAKER_*` when configured. 9 new model-free tests bring the suite to 348 passed / 0 failed / 4 skipped, and a new env-gated `realSpeakerCommandIntegration` runs "follow the speaker" on real 360 footage with the Apache-2.0 YOLOX detector, the Silero VAD provider, and an explicit creator speaker binding. Decision 028 records the audio-as-evidence command boundary. GPU optimization and the deferred Phase 3 player-lifecycle objective remain future work.

## 2026-09-17 — 360 Reframing Objective 12: 360 Command UI and Rendered-Result Preview

Reelcraft made its 360 command workflow usable from the application. No scoped Objective 12 existed in the documentation (Objective 11's next-objective entry named only candidates and required its own scope), so the human selected "360 command UI + preview rendered results"; Decision 029 records that scope. `Application::selectCreatorTargetFromViewport()` now seeds the canonical "me" identity from the current viewport yaw/pitch and preview time, and `clearCreatorSelection()` clears it; the selection is passed into every command request so "follow me" / "keep me centered" resolve without a known track id (session state, cleared on new/open project). Generated renders can be previewed: `Application::previewReframeOutput()` decodes the first frame of a persisted render record through an injectable decoder seam defaulting to the external-FFmpeg `FrameExtractor`, and the viewer presents it flat (no equirectangular transform); invalid indices, missing outputs, and decode failures are reported honestly. The minimal UI adds creator-selection buttons and a readout, a "Preview Selected Render" action on the existing render list, and a provider status line, all wired in `main.cpp`. 11 new model-free tests bring the suite to 359 passed / 0 failed / 4 skipped; no expensive real-media validation was required because the preview decode reuses the already-verified FFmpeg seam. Decision 029 records the scope. Continuous playback (the deferred Phase 3 player-lifecycle objective), creator-selection persistence, and GPU optimization remain future work.

## 2026-09-17 — 360 Reframing Objective 13: 360 Rendered-Result Playback

Reelcraft turned its single-frame rendered-result preview into continuous playback, scoped specifically to serving the 360 workflow. The human selected the deferred Phase 3 Application-level player lifecycle (Decision 030), limited to persisted 360 -> flat rendered results. `Application` now owns/creates/replaces/disposes the playback `FrameSource` (default `FfmpegFrameSource` opened with the rendered record's geometry), `FramePump`, and `Player` for the selected render, and drives the existing `Player::tick()` from its own `QTimer` — the Player owns no event loop. `startReframeOutputPlayback`/`pauseReframeOutputPlayback`/`resumeReframeOutputPlayback`/`stopReframeOutputPlayback` are deterministic, frames are emitted as `reframePlaybackFrameReady` and presented flat, and state/position/end signals report progress. Failures (invalid index, missing output, unknown dimensions, source-open failure, decode error, end-of-stream) are honest, lifecycle disposal closes the source, and the Objective 10 preview-time contract plus the single-frame preview are preserved. The existing Phase 3 seams (`FrameSource`/`FfmpegFrameSource`/`FramePump`/`Player`/`Playhead`/`Clock`/`PacingPolicy`) are reused with no parallel playback architecture. 9 new model-free tests bring the suite to 368 passed / 0 failed / 5 skipped, and a new env-gated `realReframePlaybackIntegration` renders a real 360 clip to a flat result and plays it back. Decision 030 records the scope and Definition of Done. The real-media completion validation also surfaced and drove a fix for a pre-existing media-seam bug: `FfmpegFrameSource::readNextFrame` reported end-of-stream as soon as the ffmpeg process exited, discarding output still buffered in the pipe, so a paced consumer lost the tail (~10 of 20 frames); it now drains buffered output before deciding EOF and plays all 20 frames. General/active-media playback, audio, timeline editing, duration metadata, creator-selection persistence, and GPU optimization remain future work.

## 2026-09-17 — 360 Reframing Objective 14: 360 Temporal Editing Operations

Reelcraft gained deterministic temporal editing that composes with its 360 reframing workflow. The human selected a scoped temporal-editing objective (Decision 031) rather than a timeline editor. A new parser-independent, JSON-serializable \`TemporalEditPlan\` (\`app/reframe/TemporalEditPlan.{h,cpp}\`) represents Keep, Remove, and TargetDuration over ordered source ranges, with validation, deterministic normalization, and \`resolve(durationMs, defaultStartMs)\` that rejects reversed, zero-length, negative, out-of-bounds, empty, and non-positive-duration edits. The existing \`ReframeIntentParser\` was extended (no parallel parser) to produce a structured \`ReframeIntent::temporalEdit\` from "cut from X to Y", "remove X to Y", "keep X and Y", "make a N-second version", and "this section", including word timestamps; invalid, ambiguous, contradictory, and out-of-bounds requests are reported honestly and no timestamp is invented. \`ReframePlan\` gained an additive ordered \`segments\` list (empty = the existing single source range) that changes only frame timing and camera-path evaluation time, so the existing \`ReframeRenderer\`/\`ReframePipeline\` render the retained ranges in order with no new architecture. \`ReframeCommandRunner\` resolves the edit against the known source duration and composes it with target/identity/speaker resolution, \`Application\` supplies the probed whole-clip duration and persists the retained segments in \`ReframeCommandOutcome\`, and the resulting flat render plays through the existing Objective 13 playback. The source media stays read-only. 11 new model-free tests bring the suite to 379 passed / 0 failed / 6 skipped, and a new env-gated \`realTemporalEditIntegration\` retained two ranges of a real 360 clip, rendered a 320x180 @ 10 fps result, and played it back. Decision 031 records the scope and Definition of Done; Decisions 017-030 are preserved.

## 2026-09-17 — 360 Reframing Objective 15: 360 Compound Natural-Language Editing

Reelcraft fixed a composition gap in its 360 natural-language pipeline. Objective 14's parser classified any clause containing a temporal operation and time tokens as temporal and skipped it in the camera-subject parser, so a single compound command such as "Keep 0:00 to 0:30 and follow me." kept only the temporal edit. Under the human-selected Objective 15 scope (Decision 032), the existing ReframeIntentParser now strips a temporal clause's time ranges (or a target-duration phrase) before camera/target extraction, retaining the operation keyword so "keep <subject> centered" still parses and trimming residual sentence punctuation. ReframeIntent::hasCompoundEdit() explicitly represents the composition; the deterministic default rule is that the camera/target instruction applies to the entire retained temporal range, with multiple camera moves keeping the existing interpolation. Supported compositions are temporal + target, temporal + explicit target, temporal + speaker, and target-duration + target; invalid, ambiguous, and contradictory temporal edits remain honest, and a camera instruction that supplies its own separate time interval is rejected as unsupported. TemporalEditPlan, ReframePlan, the renderer, and Objective 13 playback are reused unchanged, and the source media stays read-only. 2 new model-free tests bring the suite to 381 passed / 0 failed / 7 skipped, and a new env-gated realCompoundCommandIntegration ran "Keep 0:00 to 0:01 and look left." on real 360 footage, retained one range with the left camera direction, rendered 10 frames, and played them back. Decision 032 records the scope and Definition of Done; Decisions 017-031 are preserved.

## 2026-09-17 — 360 Reframing Objective 16: Persisted, Reproducible Edit Decisions

Reelcraft closed the last gap between "a render happened" and "a render can be reproduced". Until now a persisted render record was an audit trail — instruction, ranges, output specification, frame count — but not a reproduction recipe: the resolved camera plan was computed, used, and discarded, so reproducing a render meant re-running the original command, re-parsing the natural language and, for any subject reference, re-running the entire perception stack (detector, tracker, identity, appearance, speaker providers). Under Objective 16 (Decision 033) Reelcraft persists a versioned `EditDecision` — the already-resolved `ReframePlan` with its concrete camera keyframes, retained segments and output specification, plus the media it was made against by id, path and fingerprint, the originating instruction as provenance only, and a SHA-256 `decisionHash` over a canonical compact payload. Decisions embed additively inside the existing render records, so the project schema stays at 3 and no new database, store, format, renderer, parser or pipeline was introduced; the artifact's own `schemaVersion` is the compatibility gate, and its loader refuses to mis-parse missing, unknown or future versions, malformed source references, invalid plans, or tampered digests rather than guessing. Record loading is deliberately lenient about the decision and strict about its contents: a record whose decision cannot be read still loads, because the record is the historical fact that a render happened, and the failure is flagged with its reason, preserved verbatim so that re-saving cannot destroy data an older build does not understand, and reported to the creator. `Application::replayEditDecision()` re-renders a stored decision to a caller-supplied path through the existing `ReframePipeline::renderPlan()`, validating completely before any render — index, decision presence, source fingerprint, empty path, source-equal path, and a path that already exists — and appends a new record carrying the same decision while leaving the original byte-identical. Reproduction is verified in-process and in a genuinely fresh OS process: both match the original's decoded frames exactly, with no parser and no perception provider anywhere on the replay path — a property confirmed at object-code level by disassembling the call graph rather than by reading the source. Original media remains read-only throughout. 19 new tests bring the suite to 401 passed / 0 failed / 8 skipped. The objective also resolved a long-standing development-environment fault: a leaked Termux `PATH` entry had the Android ffmpeg running against Debian libraries at ~15 s per frame against a 15 s budget, which intermittently failed real-render tests and made `scripts/build_and_test.sh`'s 240 s ceiling unviable; installing Debian's ffmpeg cut a frame to ~2.6 s, the suite to ~221-241 s, and restored the documented verification workflow. Decision 033 records the scope and Definition of Done, including the forward constraint that persisted decisions are immutable and that Objective 17 must express revisions as new decisions with lineage rather than mutations.


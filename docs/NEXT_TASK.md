# Reelcraft — Next Task

## CURRENT PRIORITY (human-approved) — 360 Reframing / Editing Capability

The current highest-priority product objective is a **working 360 video editing/reframing capability**, not the mechanical continuation of the numbered task queue.

Priority pipeline: 360 source -> scene/subject understanding -> natural-language or structured request -> structured edit/reframe plan -> virtual-camera decisions -> deterministic execution -> flat video output.

**360 Reframing Objective 1 — Deterministic Reframing Vertical Slice — is complete and verified** (2026-09-17; 180 passed / 0 failed / 0 skipped). See the detail section at the end of this file and `CURRENT_STATE.md`.

The Phase 3 Objective 5 entry below remains valid project history but must NOT be started mechanically while the 360 priority is active. It should resume only when it directly serves the 360 capability (for example, previewing/playing reframed results).

**Current 360 status:** Reframing Objectives 1–8 are complete and verified (deterministic vertical slice; target/subject resolution; real detector integration; target identity/selection; appearance re-identification; audio/speaker evidence; audio-visual provider-attribution seam; end-to-end user-command execution). The next objective is Application-level 360 command orchestration (see the end of this file); it requires its own scoped objective.

---

## Phase 2 Status

- Phase 1 Desktop Application Foundation — complete and verified.
- Phase 2 Objective 1 — Viewer State Foundation — complete and verified (viewport orientation model, application ownership, UI readout/keyboard controls, JSON serialization and persistence, schema versioning).
- Phase 2 Objective 2 — Viewer Presentation Foundation — complete and verified at checkpoint `58cd561` (deterministic synthetic 360° test scene in `ViewerScene`; minimal presentation surface in `ViewerWidget` embedded in the desktop UI).
- Phase 2 Objective 3 — Viewer Camera Integration — complete and verified at checkpoint `fb7bb49` (deterministic `ViewerProjection` camera/view transform; camera-driven `ViewerWidget` presentation following the authoritative application-owned `ViewportState`; Application→UI wiring in `main.cpp`).
- Phase 2 Objective 4 — Real Media Foundation — complete and verified (`MediaItem` media records; `Application`-owned import/validation/deduplication; optional additive project `media` JSON section persisted on save and restored deterministically on reopen; Import Media UI; original files never modified).
- Phase 2 Objective 5 — Media Reference Availability and Open-Time Integrity — complete and verified (reopen-time revalidation of media references; deterministic list normalization/deduplication on open; unavailable-media status surfaced through the existing signal boundary).
- Phase 2 Objective 6 — Project Media Library Management (List & Remove) — complete and verified (`mediaListChanged` structured notification; deterministic `removeMedia(id)`; media list and Remove action in the shell; UI kept in sync without polling).
- Phase 2 Objective 7 — Active (Selected) Media — "Viewer Source" Contract — complete and verified (Application-owned active media id; `setActiveMedia`/`activeMediaId`/`activeMediaItem`; `activeMediaChanged`; deterministic list-consistency rules; optional additive persisted `activeMediaId`; Set Active/label affordance).
- Phase 2 Objective 8 — Equirectangular Frame Presentation Foundation — complete and verified (deterministic CPU `EquirectView` equirectangular→camera pixel renderer consistent with the ViewportState/ViewerProjection conventions; `ViewerWidget` optional source-image presentation path; marker-scene path unchanged).
- Phase 2 Objective 9 — Active-Media Frame Presentation (FFmpeg-CLI single-frame decode) — complete and verified (isolated `FrameExtractor` seam invoking the external ffmpeg CLI to decode one frame to PNG→QImage; `Application::previewActiveMediaFrame` → `framePreviewReady(QImage)` → viewer pixel path; Preview Active Frame action in the shell).
- Phase 2 Objective 10 — Active-Media Time Navigation (single-frame stepping/seek) — complete and verified (`FrameExtractor::extractFrameAt` with `-ss` seek; Application preview-position state + `previewActiveMediaFrameAt`/`stepActiveMediaPreview`/`previewTimeChanged`; Step −1 s / +1 s controls and time readout; position resets on new/open/active-change/active-removal).
- Phase 2 Objective 11 — Pointer-Based Viewer Orientation Control — complete and verified (left-drag yaw/pitch deltas: drag right = yaw increases, drag up = pitch increases; wheel FOV: up = decrease = zoom in, down = increase = zoom out; routed through the existing Application adjust slots; viewer remains presentation-only).
- Phase 2 Objective 12 — Projection Declaration & Flat-Media Preview Path — complete and verified (optional additive `projection` on `MediaItem` — equirectangular | flat | Unknown; declared via shell controls; Unknown routes to equirectangular; opt-in flat fit/letterbox presentation with no camera transform; equirect/marker paths unchanged).
- Phase 2 Objective 13 — Viewer Presentation State Consistency — complete and verified (stale decoded frames and flat mode are cleared when the project or active-media context changes: new project, open, active-media change, active-media removal; the viewer returns to the deterministic marker scene until the user previews again).
- Phase 2 Objective 14 — Presentation Quality: Bilinear Equirectangular Rendering — complete and verified (deterministic bilinear sampling in EquirectView; all camera/seam/pole/validation contracts preserved; MaxOutputWidth stays 640; measured 36.98 ms/frame at 640x320, informational 148.69 ms at 1280x640).
- Phase 2 Objective 15 — Real-Media Review Path Validation — complete and verified (deterministic FFmpeg-generated 30 s equirect clip fixture; full review path exercised: import/select, preview, stepping across frames, look-around during frame presentation, equirect routing, state resets; environment limitation recorded — no physical camera here; informational perf: ~633 ms/step decode, ~41 ms/paint render).
- Automated suite: 136 passed, 0 failed. Phase 2 formally closed at the closeout commit (docs-only; prior HEAD `ae317f5`).

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Phase 2 — 360 Viewer Review & Navigation — Formally Closed

Status: **Complete — formally closed 2026-09-06 (closeout commit).**

- Objectives 1-15 complete and verified; automated suite 136 passed / 0 failed / 0 skipped; builds succeed; offscreen smoke SMOKE_EXIT=124; clean-checkout verification of the closeout commit passes.
- Scope per Decision 016: import/manage/select media, deterministic single-frame preview, time stepping/seek, look-around orientation (keyboard + pointer), flat/equirectangular correctness, consistent viewer/project state. Continuous/paced playback, duration-aware transport, and audio are explicitly deferred.
- Closeout was documentation-only (CURRENT_STATE, PROJECT_HISTORY, DEVELOPMENT_LOG, CHANGELOG v0.2.33); no production source/test/script/schema/dependency/architecture changes.

## Phase 3 — Media Engine (OPEN — decode/media-source and player/timing foundations underway)

Status: **Open (opening architecture recorded 2026-09-06, Decision 017; decode/media-source foundation (Objective 3) and player/timing foundation (Objective 4) implemented 2026-09-16). No Application/UI playback wiring, continuous playback UX, duration metadata, or audio implementation exists.**

- Phase 3 opening architecture (approved): persistent FFmpeg streaming subprocess behind a replaceable media/player seam — the currently feasible implementation path in this environment.
- FrameExtractor remains the deterministic single-frame preview/validation seam (not the permanent engine).
- Linked FFmpeg libraries and QtMultimedia: deferred (do not install; revisit at a real desktop/deployment dependency decision).
- ffprobe duration metadata: reopened as its own future Phase 3 decision gate (not implemented).
- Objective 10 preview-time position contract is preserved; playback must extend rather than replace it.
- Architecture boundary: decode/media-source seam → player/timing → Application (orchestration) → Viewer (presentation); timeline/editor/AI above Application, never reaching into decoding.
- Phase 2 remains formally closed (`dcd876b`). No implementation has started.

## Phase 3, Objective 2 — Persistent FFmpeg Streaming Feasibility Probe — Complete

Status: **Complete — implemented and verified (2026-09-06; automated suite 139 passed, 0 failed).**

### Objective (tests-only feasibility evidence)

Prove, with temporary test-only probe helpers (no production changes), that a persistent FFmpeg subprocess can stream frames, reach EOF cleanly, fail deterministically on missing input, and be killed/restarted cleanly in this environment.

### Scope (implemented)

- Temporary probe helpers + 3 tests in `tests/test_project.cpp` (clearly marked Phase 3 Objective 2 feasibility; no production/engine code).
- Persistent rawvideo stream read of a 25-frame @5 fps deterministic equirect clip; bounded waits everywhere.
- Measurements recorded informationally (no timing gate): 25/25 frames delivered at ~39.9 frames/s (unthrottled rawvideo, 160x80), ~1.0 ms inter-frame delivery latency.
- EOF normal-exit; missing-file deterministic error; terminate/kill then clean restart verified.
- No dependency, ffprobe, schema, Obj-10, FrameExtractor, or production changes; rawvideo/transport details intentionally NOT decisions.

### Verification

- Focused probe tests pass; full regression 139/0/0; build success; offscreen smoke SMOKE_EXIT=124.

## Phase 3, Objective 3 — Replaceable Media-Source Seam & Deterministic Frame Pump — Complete

Status: **Complete — implemented and verified (2026-09-16; automated suite 144 passed, 0 failed).**

### Objective

Introduce the replaceable decode/media-source seam and a deterministic frame pump over the persistent-subprocess stream, reusing Objective 2 evidence, without changing the Objective 10 preview-time contract.

### Scope (implemented)

- `FrameSource` abstract seam (Ok/EndOfStream/Error/Timeout, bounded reads; opening excluded because it is implementation-specific).
- `FfmpegFrameSource` persistent FFmpeg-subprocess implementation (rawvideo/rgb24, caller-supplied geometry, deterministic open/close/error behavior, guaranteed process cleanup).
- `FramePump` synchronous, caller-driven consumer emitting `frameReady`/`streamEnded`/`streamFailed`; no timer, clock, pacing, or worker thread.
- Compiled into the application build and the test build; no Application/UI/Objective-10/schema/dependency changes; `FrameExtractor` unchanged.

### Verification

- Focused seam/pump tests pass, including an in-memory replaceable-source double that exercises every `ReadResult` branch with no FFmpeg subprocess.
- Full regression 144 passed / 0 failed / 0 skipped; official `scripts/build_and_test.sh` re-run green.
- Offscreen smoke SMOKE_EXIT=124.

## Phase 3, Objective 4 — Player/Timing Subsystem Foundation — Complete

Status: **Complete — implemented and verified (2026-09-16; automated suite 154 passed, 0 failed).**

### Objective

Introduce the deterministic player/timing foundation above the `FramePump`, defining playhead/position, play/pause/stop state, deterministic advancement, and replaceable clock and pacing abstractions, without changing the Objective 10 contract and without UI wiring.

### Scope (implemented)

- `Playhead` — deterministic frame count / current index / position from an explicit playback frame interval.
- `Clock` abstraction + `SystemClock` (monotonic `QElapsedTimer`); tests inject a manual clock.
- `PacingPolicy` abstraction + `DefaultPacingPolicy` (one frame per elapsed interval, drops frames when behind).
- `Player` state machine (`Stopped`/`Playing`/`Paused`) with `play()`/`pause()`/`stop()`, `tick()` (clock/pacing-driven, bounded catch-up), `stepOnce()`, and `stateChanged`/`positionChanged`/`framePresented`/`playbackEnded`/`errorOccurred` signals.
- `FramePump` stays passive and caller-driven; no timer, thread, UI, audio, duration metadata, or timeline.

### Verification

- Clean application and test builds succeed.
- Full regression 154 passed / 0 failed / 0 skipped; official `scripts/build_and_test.sh` re-run green.
- Offscreen smoke SMOKE_EXIT=124.

## Next Objective (Phase 3, Objective 5) — NOT STARTED (deferred behind the 360 priority)

Application-level player lifecycle orchestration: have `Application` own/create/replace/dispose the media source, frame pump, and player in step with the active-media contract, and define how an event-loop driver (not the `Player`) invokes `tick()`. Preserve the Objective 10 preview-time contract and keep the viewer presentation-only. UI playback controls, duration/ffprobe, audio, and timeline remain deferred. Requires its own scoped objective before implementation. Do not begin automatically while the 360 reframing priority is active.

## 360 Reframing Objective 1 — Deterministic Reframing Vertical Slice — Complete

Status: **Complete — implemented and verified (2026-09-17; automated suite 180 passed, 0 failed, 0 skipped).**

### Objective
Prove and implement a deterministic path from 360 source media and a reframing request to a usable flat video output, keeping decisions (AI or creator) separate from deterministic execution.

### Scope (implemented)
- `ReframePlan` + `CameraKeyframe` (`app/reframe/`): structured, versioned, JSON-serializable, validated reframing decisions (source media id, source range, output spec, ordered camera keyframes with linear/hold interpolation).
- `CameraPath`: pure deterministic camera evaluation (shortest-path yaw, bounded pitch/roll/FOV, hold outside the keyframe range).
- `ReframeFrameProvider` replaceable seam + `FfmpegSeekFrameProvider` (existing FFmpeg-CLI single-frame seam).
- `ReframeRenderer`: deterministic per-frame reframing via `EquirectView`, PNG-sequence output, and FFmpeg H.264 encode.
- `ReframeIntent` + `ReframeIntentParser`: deterministic natural-language boundary (aspect/platform, time ranges, named/explicit directions, subject references); unresolved subjects are reported, not fabricated.
- `ReframePlanBuilder`: intent + resolved target directions -> validated plan.
- `ReframePipeline`: end-to-end orchestration (360 source -> intent -> plan -> deterministic reframing -> flat video); source media is read-only.
- 26 new tests in `tests/test_project.cpp` (plan/keyframe round-trip and rejection, frame timing, camera interpolation incl. shortest-yaw/hold/bounds, rendering determinism, intent parsing, plan building, and a real FFmpeg end-to-end render).

### Explicitly not implemented
- Target/subject detection or tracking; speaker/dialogue analysis; content-based cut/segment selection.
- AI provider/model integration (the deterministic parser is the first implementation of the intent boundary).
- Reframe-plan persistence in the project schema; Application/UI integration of reframing; playback of reframed output.

## 360 Reframing Objective 2 — Target/Subject Resolution — Complete

Status: **Complete — implemented and verified (2026-09-17; automated suite 219 passed, 0 failed, 0 skipped).**

### Objective
Give Reelcraft the ability to resolve targets in 360 footage into deterministic spherical/view directions usable by the existing reframing pipeline, behind a replaceable boundary.

### Scope (implemented)
- `app/target/EquirectProjection`: equirect pixel <-> spherical direction, tangent-view pixel <-> direction (exact inverse of `EquirectView`), detection box -> spherical centre + angular extent, seam-safe angular distance, pitch/FOV bounds.
- `app/target/EquirectViewPlan`: deterministic overlapping perspective-view coverage (tangent views) with polar views when needed — chosen over direct equirectangular detection because of pole/seam distortion.
- `app/target/TargetDetector` seam + `app/target/ProcessTargetDetector` (dependency-free subprocess file/JSON protocol; no CV library linked).
- `app/target/TargetTypes`: `TargetDetection`, `TargetQuery`, `TargetObservation` (timestamp, identity, yaw, pitch, confidence, class, angular extent, evidence) and `TargetTrack`.
- `app/target/SphericalTargetTracker`: deterministic spherical NMS + greedy nearest-neighbour association with gate and miss counting.
- `app/target/TargetResolver`: views -> detector -> spherical observations -> tracks; `resolvedTargets()` feeds the existing `ReframePlanBuilder`; unresolved queries are reported, never fabricated.
- `app/target/TargetTrackPlanner`: track -> validated `ReframePlan` for `CameraPath`/`ReframeRenderer`.
- 39 new tests, including a model-free end-to-end path.
- Technology/licensing evaluation: `docs/TARGET_RESOLUTION_TECHNOLOGY.md`; Decision 019.

### Explicitly not implemented
- A bundled real detection model (no CV runtime in this environment; the subprocess seam is ready).
- "Me" identity / re-identification; speaker localization; semantic/open-vocabulary classification; production tracking quality.

## 360 Reframing Objective 3 — Real Detector Integration — Complete

Status: **Complete — implemented and verified (2026-09-17).**

### Objective
Make the first real computer-vision detector work with actual 360 footage through the existing `ProcessTargetDetector` boundary: real 360 footage -> tangent views -> real detection -> spherical coordinates -> existing tracker -> TargetTrack -> ReframePlan -> deterministic flat render.

### Scope (implemented)
- Optional external helper `tools/detector_helper/yolox_detector.py` implementing the existing file/JSON protocol, using OpenCV Zoo YOLOX (Apache-2.0) + OpenCV DNN (CPU). Reelcraft links no CV library. `tools/detector_helper/README.md` records model, weights, runtime, licenses, install, GPU note, protocol, and limitations.
- New integration test `realDetectorIntegration`, skipped unless detector/model/clip env vars are configured; the normal suite stays model-free.
- No changes to the `TargetDetector`/`ProcessTargetDetector` boundary or the 360 geometry.

### Verified on real footage
- Real 3840x1920 equirect clip; a 1920x960 proxy of the 114-126 s segment was used for speed (original untouched).
- Detected two presenters in tangent views: spherical yaw -27.9/pitch -28.6 and yaw +26.9/pitch -27.8.
- Strongest `person` track held id `t1` across five sampled frames (mean confidence 0.923).
- Track -> validated `ReframePlan` -> 640x360 H.264 output visibly centered on the detected presenter.
- Normal suite: 219 passed, 0 failed, 1 skipped (the real-detector test when unconfigured).

### Explicitly not implemented
- "Me" identity / re-identification; speaker localization; semantic/open-vocabulary classification; production tracking quality; GPU/RunPod inference (no credentials available).

## 360 Reframing Objective 4 — Target Identity & Deterministic Selection — Complete

Status: **Complete — implemented and verified (2026-09-17).**

### Objective
Represent a creator-selected target ("me") as structured data, distinguish it deterministically from other detected people, and keep that identity across frames, feeding the existing TargetTrack/ReframePlan/renderer path.

### Scope (implemented)
- `app/target/TargetIdentity.{h,cpp}`: `CreatorTargetSelection`, `IdentityBinding`, `TargetIdentityRegistry` (seed binding, explicit track binding, claim conflicts, active-state refresh, unique continuity re-binding, JSON round-trip).
- `app/target/TargetSelector.{h,cpp}`: documented deterministic references and a canonical track order.
- `SphericalTargetTracker` hardening: bounded constant-velocity prediction and a bounded re-entry gate (no appearance model).
- 21 model-free tests; the real-detector integration test extended with identity selection and selection ambiguity.

### Supported references
"me"/"myself"/"my"/"the person I selected"/"my selection"/"the selected person"/"the person I picked"; "the other person"/"the other one"/"the other"; "person N"/"the first person"/"the second person"/...; "the person on the left"/"on the right"; exact track id ("t2"); a label when it is unique.

### Documented meaning of "me"
"Me" = the track the creator explicitly selected (seed direction/time or track id), or its unique geometric continuation. It is NOT biometric identity and does NOT re-identify a person after a long absence or among similar people; that requires a future appearance/re-ID seam. If identity is unresolved or ambiguous it is reported, never guessed.

### Not implemented
Speaker/active-speaker association; appearance/embedding re-identification; open-vocabulary classes; GPU/RunPod inference.

## 360 Reframing Objective 5 — Appearance-Based Re-Identification — Complete

Status: **Complete — implemented and verified (2026-09-17).**

### Objective
Add an optional, replaceable visual-appearance layer that strengthens the creator identity when geometry is insufficient, without coupling the core to a model runtime or claiming biometric identity.

### Scope (implemented)
- `app/target/AppearanceTypes.{h,cpp}`: embeddings, math, profile, structured evidence/verdicts, JSON.
- `app/target/AppearanceProvider.h`, `ProcessAppearanceProvider.{h,cpp}`: optional external helper boundary.
- `app/target/TargetCropExtractor.{h,cpp}`: deterministic crop via `EquirectView`.
- `app/target/IdentityReidentifier.{h,cpp}`: profile maintenance + precedence policy.
- `TargetIdentityRegistry` appearance extensions (profile storage, re-bind, annotate, veto).
- Optional real helper `tools/appearance_helper/` with a permissively licensed model.

### Precedence
Explicit creator selection / active binding > valid tracker continuity > unique geometric continuation (appearance confirms or vetoes) > appearance-only single-strong re-acquisition > unresolved/ambiguous. Appearance never overrides an explicit selection; a disagreement yields unresolved, never a silent swap.

### Not implemented
Active-speaker/audio-visual association; GPU/CUDA optimization; UI integration; face recognition/biometric identity (explicitly out of scope).

## 360 Reframing Objective 6 — Optional Audio/Speaker Evidence — Complete

Status: **Complete — implemented and verified (2026-09-17).**

### Objective
Add an optional, replaceable audio/speaker evidence layer that answers "which visible tracked person is speaking" and feeds the existing deterministic selection/plan/render pipeline, without making audio the authoritative identity source.

### Scope (implemented)
- `app/target/SpeakerTypes.{h,cpp}`: structured speech intervals, analysis, per-target evidence, timeline segments, and verdicts.
- `app/target/SpeakerEvidenceProvider.h` + `ProcessSpeakerProvider.{h,cpp}`: external provider seam (fail-safe).
- `app/target/SpeakerTargetAssociator.{h,cpp}`: deterministic speakerId -> target association (explicit > spatial DoA > single visible > ambiguous/unassociated).
- `app/target/SpeakerTimeline.{h,cpp}`: documented temporal hysteresis (pauses, switch confirmation, overlap).
- `app/target/SpeakerEvidenceAnalyzer.{h,cpp}` and `SpeakerReframePlanner.{h,cpp}`.
- `TargetIdentityRegistry::annotateSpeaker`: evidence only; never changes resolution.
- Optional helper `tools/speaker_helper/` with Silero VAD (MIT).

### Precedence
Identity precedence is unchanged: explicit creator selection > tracker continuity > unique geometric continuation > appearance > audio evidence > unresolved. Audio never rebinds identity and never overrides an explicit selection; it drives deterministic *selection* for "follow the speaker".

### Not implemented
Automatic audio-visual speaker attribution without an explicit binding (diarization / active-speaker models); GPU/CUDA optimization; UI integration.

## 360 Reframing Objective 7 — Audio-Visual Provider Attribution Seam — Complete

Status: **Complete — implemented and verified (2026-09-17).**

### Objective

Provide automatic audio-visual speaker attribution without a creator binding — using diarization or an audio-visual active-speaker model — behind the existing replaceable provider seam, under the explicit efficiency guardrail: the minimum reliable capability for "follow the person who is speaking", not maximum perception sophistication.

### Scope (implemented)

- `SpeakerInterval`/`SpeakerSegment` optional `targetIdHint`: an existing target track id an audio-visual/diarization provider attributes. `SpeakerTargetAssociator` precedence becomes explicit creator binding > visible provider hint > spatial DoA > single visible > ambiguous/unassociated; method `provider-hint`. A non-visible hint falls through; a hint never invents a target and never overrides the creator.
- `SpeakerTimeline` propagates the hint onto the coalesced segment (first non-empty wins; cleared for overlap/silence); `SpeakerEvidenceAnalyzer` honours it end to end.
- 5 new model-free tests; full model-free suite 300 passed / 0 failed / 1 skipped.

### Feasibility boundary (why no model-backed provider is shipped)

- Real footage audio is **mono** → no direction of arrival; spatial attribution impossible.
- A face-detection + mouth-motion/audio-envelope correlation probe at 12 fps gave max correlation 0.250 (speaker) vs 0.192 (listener) — weak, not a reliable discriminator.
- No permissively licensed, clearly commercial audio-visual active-speaker model identified (TalkNet/LoCoNet/AV-HuBERT research-grade/unclear weights; pyannote gated; SpeechBrain/torchreid heavy with VoxCeleb provenance).
- The seam and tests are complete, so a licensed provider can be dropped in later without core changes. Architecture decision: Decision 024.

## 360 Reframing Objective 8 — End-to-End 360 User-Command Execution — Complete

Status: **Complete — implemented and verified (2026-09-17).**

### Objective

Exercise the full 360 command path end to end on real footage — user instruction -> parsed intent -> target resolution/identity -> validated `ReframePlan` -> deterministic render — behind a single composition entry point, and identify integration gaps and failure modes rather than adding perception subsystems.

### Scope (implemented)

- `app/reframe/ReframeCommandRunner.{h,cpp}`: the composition entry point. `prepare()` parses the command, resolves subject references through the replaceable detector/tracker, binds the optional creator identity, and builds the validated plan (deterministic and model-free with an injected provider/detector). `run()` adds deterministic execution through the unchanged `ReframePipeline`.
- Reuses `TargetResolver`, `TargetIdentityRegistry` + `TargetSelector`, `ReframePlanBuilder`, and `ReframePipeline`/`ReframeRenderer`; adds no perception of its own and never fabricates a direction (unresolved/ambiguous references are reported, a direction-only command needs no detector).
- 8 new model-free tests; full model-free suite 308 passed / 0 failed / 2 skipped. New env-gated `realUserCommandIntegration` exercises the command path on real footage.
- Architecture decision: Decision 025.

## Next Objective — Application-Level 360 Command Orchestration — NOT STARTED

Make the 360 command path usable from the product: have `Application` own the command runner, source media, and output, preserve the Objective 10 preview-time contract, and expose a deterministic run/preview boundary (an event-loop driver invokes execution; the viewer stays presentation-only). Speaker-aware commands and GPU optimization remain further candidates. Requires its own scoped objective; do not begin automatically.



# Reelcraft — Next Task

## CURRENT PRIORITY (human-approved) — 360 Reframing / Editing Capability

The current highest-priority product objective is a **working 360 video editing/reframing capability**, not the mechanical continuation of the numbered task queue.

Priority pipeline: 360 source -> scene/subject understanding -> natural-language or structured request -> structured edit/reframe plan -> virtual-camera decisions -> deterministic execution -> flat video output.

**360 Reframing Objective 1 — Deterministic Reframing Vertical Slice — is complete and verified** (2026-09-17; 180 passed / 0 failed / 0 skipped). See the detail section at the end of this file and `CURRENT_STATE.md`.

The Phase 3 Objective 5 entry below remains valid project history but must NOT be started mechanically while the 360 priority is active. It should resume only when it directly serves the 360 capability (for example, previewing/playing reframed results).

**Next 360 objective:** Reframing Objective 2 — target/subject resolution (detection/tracking) and speaker localization behind the existing resolved-target boundary.

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

## Next Objective (360 Reframing Objective 3) — NOT STARTED

Identity and evidence: resolve "me"/the creator and the active speaker into a target identity (creator-selected seed, re-identification, or audio-visual speaker association), and integrate a real permissively licensed detector helper (for example YOLOX or RT-DETR via ONNX Runtime) behind the existing `ProcessTargetDetector` protocol. Requires its own scoped objective before implementation; must keep the real model optional so the unit suite stays model-free.



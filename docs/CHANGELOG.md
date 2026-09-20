# Reelcraft — Changelog

All notable Reelcraft project changes should be recorded here.

This changelog focuses on meaningful project-level changes and user-visible capabilities. Internal development details belong in DEVELOPMENT_LOG.md.

---

# [0.1.0] — 2026-09-02

## Added

- Established the Reelcraft project identity and product vision.
- Established the persistent project documentation system.
- Established the controlled AI development workflow.
- Established the initial technical architecture.
- Established architectural decision tracking.
- Established AI handoff documentation.
- Established project history tracking.
- Established development logging.
- Established known-issue tracking.

## Project Status

Reelcraft is currently in the documentation and architecture foundation stage.

No production application functionality has been released yet.

---

# Changelog Rules

Future entries should:

- Use the appropriate project version.
- Include the date of the change.
- Describe meaningful user-facing or project-level changes.
- Avoid claiming functionality that has not been implemented and verified.
- Keep internal implementation details primarily in DEVELOPMENT_LOG.md.
- Preserve historical entries rather than rewriting them.

## 0.1.1 — Product and AI Architecture Contracts

### Added

- Product requirements contract in `REQUIREMENTS.md`.
- Conceptual project model in `PROJECT_MODEL.md`.
- AI-to-deterministic editing contract in `AI_EDIT_CONTRACT.md`.
- Technology/runtime evaluation task in `NEXT_TASK.md`.

### Updated

- `CURRENT_STATE.md` now reflects the transition toward validated technical direction.
- `DEVELOPMENT_LOG.md` records the new product and AI architecture foundation.
- `PROJECT_HISTORY.md` records the milestone chronologically.

### Architectural Clarification

The project now explicitly preserves the boundary:

    AI reasoning → Structured Edit Plan → Validation → Deterministic Media Execution

AI output is untrusted until validated. Original media remains protected, 360° remains a first-class capability, camera-specific behavior remains behind adapters, and voice/text share the same intent architecture.

### Deferred

A substantial production application skeleton remains intentionally deferred until the technology/runtime evaluation provides sufficient evidence for a safe implementation direction.

No production video editor, media engine, AI provider integration, voice system, or 360° editing system was implemented in this milestone.

## v0.1.2 — Technology Direction Validated

Date: 2026-09-02

- Completed the focused technology and runtime evaluation.
- Established validated requirements for the desktop application rendering layer.
- Confirmed the separation between UI/application systems, AI reasoning, and deterministic media execution.
- Confirmed first-class 360° requirements without prematurely implementing the 360° pipeline.
- Preserved replaceable AI providers and local/cloud/hybrid processing options.
- Preserved non-destructive media handling and portable project-state requirements.
- Established automated testing and regression protection as Phase 1 requirements.
- Kept Qt 6, Electron, and Tauri 2 as evaluated candidates rather than permanent technology commitments.
- Kept Wails v3 as a lower-priority/watchlist option.
- Transitioned the project from technology-direction evaluation to controlled Phase 1 application-foundation planning.
- No major editor feature or production media pipeline was implemented in this milestone.

## v0.1.3 — Phase 1 Desktop Application Foundation Complete
Date: 2026-09-03

- Added minimal Qt 6 desktop application shell with UI/application-core separation
- Added Project create/identify/persist/reopen/close lifecycle
- Added non-destructive JSON project storage
- Added background task demonstration with QtConcurrent
- Added automated Qt Test suite (9 tests)
- Added error handling for invalid project save/open paths
- Preserved architecture boundaries; no production editor/media/AI systems implemented

## v0.1.4 — Phase 1 UI Integration Verification

Date: 2026-09-03

- Added UI-level automated tests for MainWindow signals and labels
- Added Application save/open error-path tests
- Made MainWindow file-path chooser injectable for headless verification
- All 14 tests pass offscreen

## v0.1.5 — Reproducible Build-and-Test Workflow

Date: 2026-09-03

- Added scripts/build_and_test.sh
- Verified offscreen test run: 14 passed, 0 failed
- Updated development environment documentation

## v0.1.6 — Original Media Safety Verification

Date: 2026-09-03

- Added automated original-media non-modification test
- Verified byte and SHA-256 integrity across project lifecycle
- Expanded Qt Test suite to 15 passing tests

## v0.1.7 — Platform Strategy Documentation

Date: 2026-09-03

- Recorded Decision 014: Desktop first-class, Android planned future
- Recorded platform boundary constraints
- Classified Termux/Ubuntu as development-only
- No application source changes


## v0.1.8 — Apple Platform Portability Classification

Date: 2026-09-03

- Extended Decision 014 platform strategy
- Classified macOS, iOS, and iPadOS as future portability targets
- Recorded platform-neutral architectural constraints
- No application source changes

## v0.1.9 — Real Desktop Smoke Test

Date: 2026-09-03

- Completed real desktop smoke test via Termux:X11.
- Verified UI render, new/save/open project lifecycle, background responsiveness.
- Regression tests pass.
- No application source changes.

## v0.1.10 — Clean-Checkout Workflow Verification

Date: 2026-09-03

- Verified Phase 1 build/test workflow from clean checkout
- 15 automated tests pass
- No application source changes

## v0.1.11 — Environment Documentation Alignment

Date: 2026-09-03

- Corrected verified test count to 15 in DEVELOPMENT_ENVIRONMENT.md.
- Documented manual desktop smoke path.
- No application source changes.

## v0.1.12 — Phase 1 Desktop Foundation Complete

Date: 2026-09-03

- Formally declared Phase 1 Desktop Application Foundation complete.
- Consolidated verified Phase 1 evidence.
- No application source changes.

## v0.2.0 — Phase 2 Viewer State Foundation

Date: 2026-09-03

- Added platform-independent ViewportState model
- Added viewer orientation/FOV validation
- Added automated viewer-state tests
- Existing Phase 1 tests continue to pass
- No rendering, media, playback, or camera implementation introduced

## v0.2.1 — Application-Level Viewport State

Date: 2026-09-03

- Integrated ViewportState into Application.
- Added resetViewport behavior.
- Added automated application-level viewer state tests.
- Full test suite: 23 passed, 0 failed.

## v0.2.2 — UI Viewer Readout

Date: 2026-09-03

- Added viewer yaw/pitch/roll/FOV readout.
- Added Reset Viewport UI action.
- Added UI-level viewer state tests.
- Full test suite: 25 passed, 0 failed.

## v0.2.3 — Viewer State Integration Tests

Date: 2026-09-03

- Added automated integration tests for viewer state signal flow.
- Verified UI readout and Reset Viewport behavior.
- Full test suite: 27 passed, 0 failed.

## v0.2.4 — Keyboard Viewer Controls

Date: 2026-09-03

- Added keyboard controls for viewer yaw/pitch/roll/FOV.
- Added automated keyboard-control tests.
- Full test suite: 29 passed, 0 failed.

## v0.2.5 — New Project Viewer Reset

Date: 2026-09-03

- Added viewer-state reset on new project creation.
- Added automated reset test.
- Full test suite: 30 passed, 0 failed.

## v0.2.6 — Open Project Viewer Reset

Date: 2026-09-04

- Added viewer-state reset on open project.
- Added automated open-project reset test.
- Full test suite: 31 passed, 0 failed.


## v0.2.7 — Viewer State JSON Serialization

Date: 2026-09-04

- Added ViewportState JSON export/import.
- Added automated serialization tests.
- Full test suite: 33 passed, 0 failed.


## v0.2.8 — Viewer State Persistence

Date: 2026-09-04

- Persist viewer state inside project JSON.
- Restore viewer state on open.
- Preserve backward compatibility for projects without viewer state.
- Full test suite: 34 passed, 0 failed.


## v0.2.9 — Restored Viewer State UI Integration Test

Date: 2026-09-04

- Added UI integration test for restored viewer state.
- Full test suite: 35 passed, 0 failed.


## v0.2.10 — Focused Button Keyboard Input

Date: 2026-09-04

- Keyboard viewer controls now work while buttons have focus.
- Added focused-button keyboard test.
- Full test suite: 36 passed, 0 failed.


## v0.2.11 — Invalid Viewer State Fallback Verification

Date: 2026-09-04

- Added automated invalid viewer state fallback test.
- Full test suite: 37 passed, 0 failed.


## v0.2.12 — Application-Level Viewer Adjust Slots

Date: 2026-09-04

- Moved viewer adjustment operations into Application.
- Added application-level adjust slot test.
- Full test suite: 38 passed, 0 failed.


## v0.2.13 — Phase 2 Clean-Checkout Verification

Date: 2026-09-04

- Verified Phase 2 workflow from clean checkout.
- 38 automated tests pass.
- No application source changes.


## v0.2.14 — Project Schema Versioning

Date: 2026-09-04

- Added project schema versioning.
- Preserved backward compatibility with version-less project files.
- Full test suite: 40 passed, 0 failed.


## v0.2.15 — Project Schema Version Round Trip

Date: 2026-09-04

- Added automated schema version round-trip verification.
- Full test suite: 41 passed, 0 failed.


## v0.2.16 — Future Schema Version Rejection

Date: 2026-09-04

- Reject project files created with a newer unsupported schema version.
- Full test suite: 42 passed, 0 failed.


## v0.2.17 — Application Open Future Schema Fails Safely

Date: 2026-09-04

- Added application-level future schema rejection verification.
- Full test suite: 43 passed, 0 failed.


## v0.2.18 — Phase 2 Clean-Checkout Re-Verification

Date: 2026-09-04

- Re-verified Phase 2 workflow from clean checkout.
- 43 automated tests pass.
- No application source changes.

## v0.2.19 — Minimal Viewer Presentation Surface

Date: 2026-09-04

- Added a deterministic synthetic 360° test scene (10 oriented markers, no media).
- Added a minimal viewer presentation surface rendering the scene as an identity equirectangular view.
- The desktop shell now displays the viewer surface.
- Added deterministic render and scene tests.
- Full test suite: 48 passed, 0 failed.
- No ViewportState→camera integration, media, playback, or rendering pipeline yet.

## v0.2.20 — Viewer Camera Integration

Date: 2026-09-04

- Added a deterministic camera/view transform (yaw/pitch/roll/FOV) for the viewer.
- The viewer presentation now follows the application-owned viewport state: rotating the viewport deterministically changes the camera view of the synthetic 360° scene.
- Added camera transform and Application→viewer integration tests.
- Full test suite: 55 passed, 0 failed.
- No media, playback, timeline, AI, export, reframing, or camera-specific logic yet.

## v0.2.21 — Real Media Foundation

Date: 2026-09-04

- Added real media file import: select/import a media file, validated as an existing, readable, regular file.
- Media references recorded with deterministic metadata (stable id, path, file name, size, last-modified, format tag).
- Media records persist in the project file (optional additive section) and restore deterministically on reopen; projects without media are unaffected.
- Original media files are never modified (verified byte/SHA-256 invariance).
- Added media validation, integrity, and persistence/reopen tests.
- Full test suite: 68 passed, 0 failed.
- No decoding, playback, timeline, AI, export, audio, effects, reframing, or camera-specific logic yet.

## v0.2.22 — Media Reference Availability and Open-Time Integrity

Date: 2026-09-04

- Projects reopened with unavailable media references now report them deterministically (media is never removed because its file is missing).
- Duplicate media records in a project file are normalized on open; re-saving stays normalized.
- Added availability and normalization tests.
- Full test suite: 73 passed, 0 failed.
- No decoding, playback, timeline, AI, export, effects, or camera-specific logic yet.

## v0.2.23 — Project Media Library Management

Date: 2026-09-05

- Imported media is now listed in the shell (file name and format tag) and stays in sync with the project automatically.
- Added a Remove Media action: media records can be removed from a project deterministically; referenced files are never touched.
- Added media-list sync and removal tests.
- Full test suite: 81 passed, 0 failed.
- No decoding, playback, timeline, AI, export, effects, or camera-specific logic yet.

## v0.2.24 — Active Media ("Viewer Source") Contract

Date: 2026-09-05

- A project can now mark one imported media record as active/selected (the "viewer source" contract).
- The active selection is deterministic: import never auto-selects; removing or starting a new project clears it; reopening a project restores it only when it still resolves to an available media record.
- The active media id persists with the project (optional additive field; no schema change).
- Added Set Active/label affordance in the media library; referenced files are never touched.
- Added selection, consistency, and persistence tests.
- Full test suite: 90 passed, 0 failed.
- No decoding, playback, timeline, AI, export, effects, or camera-specific logic yet.

## v0.2.25 — Equirectangular Frame Presentation Foundation

Date: 2026-09-05

- Added a deterministic CPU renderer that views an equirectangular image through the existing viewer camera (yaw/pitch/roll/FOV), consistent with the established viewer conventions.
- The viewer presentation surface now accepts an optional in-memory equirectangular source image and renders it through the camera; clearing the source returns to the synthetic test scene.
- Added camera-anchor, roll-direction, FOV, invalid-input, determinism, and viewer-integration tests plus a CPU performance sanity measurement (~14 ms/frame at 640×320).
- Full test suite: 100 passed, 0 failed.
- No real media decoding/playback yet — this is the decode-free pixel presentation foundation.

## v0.2.26 — Active-Media Frame Presentation

Date: 2026-09-05

- The active media record can now produce a real decoded preview frame (single-frame extraction via the external ffmpeg CLI, isolated behind a replaceable adapter seam).
- A "Preview Active Frame" action decodes and presents the active media's frame through the existing viewer camera path.
- Added decode, preview, and end-to-end presentation tests (they skip cleanly if ffmpeg is unavailable).
- Full test suite: 107 passed, 0 failed.
- No playback/streaming/audio, timeline, AI, export, or effects yet; original media is never modified.

## v0.2.27 — Active-Media Time Navigation

Date: 2026-09-06

- The active media preview can now be stepped or seeked through time: single-frame extraction at a requested position.
- Added Step −1 s / +1 s controls and a preview-time readout in the shell.
- Time navigation is on-demand and deterministic; beyond-end requests fail cleanly without moving the position.
- Added seek, stepping, position-state, and reset tests (they skip cleanly if ffmpeg is unavailable).
- Full test suite: 116 passed, 0 failed.
- No continuous playback, audio, streaming, timeline, AI, export, or effects yet; original media is never modified.

## v0.2.28 — Pointer-Based Viewer Orientation Control

Date: 2026-09-06

- The 360 viewer now supports drag-to-look: drag right increases yaw, drag up increases pitch.
- The mouse wheel adjusts field of view: wheel up zooms in (FOV decreases), wheel down zooms out.
- Viewer gestures route through the same authoritative viewport state as the keyboard controls.
- Added deterministic pointer-interaction tests.
- Full test suite: 120 passed, 0 failed.
- No continuous playback, audio, streaming, timeline, AI, export, or effects yet; original media is never modified.

## v0.2.29 — Projection Declaration & Flat Preview

Date: 2026-09-06

- Media records can be marked as flat or 360 equirectangular; the declared projection persists with the media record.
- Flat media now previews correctly: fitted, centered, letterboxed, with no 360 wrap distortion.
- Undeclared media keeps the existing equirectangular preview behavior.
- Added projection, routing, and flat-presentation tests.
- Full test suite: 128 passed, 0 failed.
- No continuous playback, audio, streaming, timeline, AI, export, or effects yet; original media is never modified.

## v0.2.30 — Viewer State Consistency

Date: 2026-09-06

- Creating/opening a project or switching/removing the active media now clears any stale preview frame so the viewer never shows the previous media's content.
- The viewer returns to its deterministic test scene until the next explicit preview.
- Added viewer state-consistency tests.
- Full test suite: 131 passed, 0 failed.
- No continuous playback, audio, streaming, timeline, AI, export, or effects yet; original media is never modified.

## v0.2.31 — Bilinear Equirectangular Rendering

Date: 2026-09-06

- Equirectangular preview rendering now uses deterministic bilinear sampling for smoother single-frame review (no more blocky nearest-neighbor pixels when zoomed).
- Camera/orientation behavior, seam handling, and validation are unchanged.
- Added interpolation and seam/pole tests.
- Full test suite: 133 passed, 0 failed.
- Rendering resolution safeguard remains 640 px (unchanged); no continuous playback, audio, streaming, timeline, AI, export, or effects yet; original media is never modified.

## v0.2.32 — Real-Media Review Path Validation

Date: 2026-09-06

- The full review workflow (select, single-frame preview, stepping/seek, look-around orientation, equirect routing, state resets) is now validated end-to-end against deterministic FFmpeg-generated equirectangular video (no physical camera required).
- Added review-path validation fixtures/tests; test-runner timeout raised accordingly in the dev script.
- Informational performance recorded (decode ~633 ms/step; render ~41 ms/paint at 640 px).
- Full test suite: 136 passed, 0 failed.
- No continuous playback, audio, streaming, timeline, AI, export, or effects yet; original media is never modified.

## v0.2.33 — Phase 2 Complete

Date: 2026-09-06

- Phase 2 — "360 Viewer Review & Navigation" (Decision 016) is formally complete.
- Final suite: 136 passed, 0 failed; builds succeed; offscreen smoke passes; clean-checkout verification of the closeout commit passes.
- Closeout is documentation-only (no production source/test/script/schema/dependency changes).
- Phase 3 (media engine / continuous playback) has not started; a Phase 3 opening decision/discovery is the next step.

## v0.2.34 — Phase 3 Opening Decisions Recorded

Date: 2026-09-06

- Phase 3 (Media Engine) opened with documentation-only decisions (Decision 017): persistent FFmpeg streaming subprocess behind a replaceable media/player seam as the opening architecture.
- FrameExtractor remains the deterministic single-frame preview/validation seam; linked FFmpeg libraries and QtMultimedia are deferred; ffprobe duration metadata is reopened as a future decision gate.
- Objective 10's preview-time contract is preserved; playback will extend it.
- No media-engine/playback/duration/audio implementation, dependencies, or source changes; Phase 2 remains formally closed.

## v0.2.35 — Phase 3 Media-Engine Feasibility Probe

Date: 2026-09-06

- Tests-only feasibility evidence (no production changes): a persistent FFmpeg subprocess streams frames, reaches EOF cleanly, fails deterministically on missing input, and supports clean kill/restart.
- Informational measurement: ~39.9 frames/s unthrottled rawvideo delivery at 160x80, ~1.0 ms inter-frame latency (not a performance gate).
- Full test suite: 139 passed, 0 failed.
- No media-engine/playback/duration/audio implementation, dependencies, ffprobe, or Obj-10 contract changes.

## v0.2.36 — Phase 3 Media-Engine Foundation: Replaceable Media Source & Frame Pump

Date: 2026-09-16

- Introduced the replaceable decode/media-source seam (`FrameSource`) and its persistent FFmpeg-subprocess implementation (`FfmpegFrameSource`), plus a deterministic, caller-driven `FramePump` over that stream.
- The seam is transport-agnostic and replaceable; a future linked-FFmpeg or QtMultimedia backend can implement it without changing consumers. `FrameExtractor` remains the single-frame preview seam.
- No continuous/paced playback, duration metadata/ffprobe, audio, timeline, or viewer playback wiring yet; the Objective 10 preview-time contract and original-media non-destructiveness are unchanged.
- Full test suite: 144 passed, 0 failed, 0 skipped.

## v0.2.37 — Phase 3 Player/Timing Subsystem Foundation

Date: 2026-09-16

- Introduced the deterministic player/timing foundation above the frame pump: `Player` (`Stopped`/`Playing`/`Paused`, `play`/`pause`/`stop`, `tick`/`stepOnce`, playhead and frame-presentation signals) plus a small `Playhead` position value.
- Clock and pacing are replaceable abstractions (`Clock`/`SystemClock`, `PacingPolicy`/`DefaultPacingPolicy`) with injected dependencies, so playback behavior is deterministic and tested without wall-clock delays.
- `FramePump` remains a passive, caller-driven decoder; no timer, thread, UI wiring, audio, duration/ffprobe, or timeline exists yet, and the Objective 10 preview-time contract is unchanged.
- Full test suite: 154 passed, 0 failed, 0 skipped.

## v0.2.38 — 360 Reframing Engine (Deterministic Vertical Slice)

Date: 2026-09-17

- Added a deterministic 360-to-flat reframing engine (`app/reframe/`): a structured, versioned, validated `ReframePlan`/`CameraKeyframe` boundary; a pure `CameraPath` evaluator; a replaceable frame-provider seam with an FFmpeg-CLI implementation; a deterministic renderer; and an end-to-end `ReframePipeline`.
- Added a deterministic natural-language intent boundary: aspect/platform ("16:9", "TikTok", square), time ranges ("from 00:30 to 01:00"), and camera directions/moves. Subject references are resolved only when a target direction is supplied; unresolved references are reported, never guessed.
- The source media is read-only; reframing writes only derived outputs. The same plan and source produce the same frames.
- 26 new tests (plan/keyframe round-trip and validation, frame timing, camera interpolation, rendering determinism, intent parsing, plan building, and a real FFmpeg end-to-end render). Full test suite: 180 passed, 0 failed, 0 skipped.
- No AI provider/model, target detection/tracking, speaker analysis, reframe-plan persistence, or Application/UI integration yet.

## v0.2.39 — 360 Target/Subject Resolution

Date: 2026-09-17

- Added a replaceable 360 target-resolution layer (`app/target/`): detects in overlapping perspective (tangent) views and reprojects detections into the existing spherical camera coordinates, avoiding equirectangular pole/seam distortion.
- Added a replaceable `TargetDetector` seam with a dependency-free subprocess adapter (`ProcessTargetDetector`, file/JSON protocol). No computer-vision library is linked into Reelcraft.
- Added a deterministic `SphericalTargetTracker` (spherical NMS + greedy nearest-neighbour association with gate and miss counting) producing identity-persistent `TargetTrack`s.
- Added `TargetResolver` and `TargetTrackPlanner`: resolved targets bridge to the existing `ReframePlanBuilder`, and full tracks become `ReframePlan`s consumed by `CameraPath`/`ReframeRenderer`. Unresolved queries are reported, never fabricated.
- Recorded the detection-technology and licensing evaluation in `docs/TARGET_RESOLUTION_TECHNOLOGY.md` (Ultralytics YOLO AGPL-3.0 rejected as a default; permissive candidates recommended).
- 39 new tests (geometry, seam/pitch boundaries, view coverage, tracking, resolver, subprocess protocol, planner, and a model-free detection -> plan -> render path). Full test suite: 219 passed, 0 failed, 0 skipped.
- Not implemented yet: a bundled real detection model, "me" identity, speaker localization, semantic classification, production tracking quality, reframe-plan persistence, and UI integration.

## v0.2.40 — Real Detector Integration (OpenCV Zoo YOLOX)

Date: 2026-09-17

- Connected a real computer-vision detector through the existing replaceable `ProcessTargetDetector` boundary: an optional external helper (`tools/detector_helper/`) using OpenCV Zoo YOLOX 2022nov (Apache-2.0 code and weights) and OpenCV DNN 4.10 on CPU. Reelcraft links no computer-vision library; the model and runtime remain replaceable.
- Verified on real 360 footage (3840x1920 equirect): real person detections, spherical target coordinates, stable target identity across sampled frames, a generated `ReframePlan`, and a 640x360 H.264 output visibly centered on the detected person.
- Added a separate real-detector integration test, skipped unless a helper/model/clip are configured; the normal unit suite stays model-free (219 passed, 0 failed, 1 skipped).
- Documented the exact model, weights, runtime, licenses, install, GPU options, protocol, results, and failure cases in `docs/TARGET_RESOLUTION_TECHNOLOGY.md` and `tools/detector_helper/README.md`; Decision 020 records the selection.
- Not implemented yet: "me" identity, speaker localization, open-vocabulary classes, production tracking quality, GPU/RunPod inference, reframe-plan persistence, and UI integration.

## v0.2.41 — Target Identity & Deterministic Selection

Date: 2026-09-17

- Added structured creator identity (`app/target/TargetIdentity.*`): a creator-selected target ("me") is represented as a JSON-serializable binding between an identity key and a tracker track id, established from a structured seed (direction/time or track id) and refreshed against live tracks.
- Added deterministic target selection (`app/target/TargetSelector.*`) for a documented vocabulary: "me"/selected aliases, "the other person", "person N"/ordinals, left/right, exact track id, and a unique label. Canonical ordering never depends on detector output order; ambiguous references are reported with candidates instead of guessed.
- Hardened the tracker with bounded constant-velocity prediction (crossing trajectories) and a bounded re-entry gate (temporary loss/occlusion/re-entry); still no appearance model.
- Real-footage validation: 9 people detected; "me" bound to the strongest presenter (t1, 5 frames, mean confidence 0.923); identity fed the existing `ReframePlan`/`ReframeRenderer` and produced a 640x360 output centered on the selected person.
- "Me" is geometric identity, not biometric; it does not re-identify after long absence or among similar people. Speaker association and appearance re-identification remain future work.
- 21 new model-free tests; full suite 240 passed, 0 failed, 1 skipped. Decision 021 recorded; Decisions 017-020 preserved.

## v0.2.42 — Appearance-Based Re-Identification

Date: 2026-09-17

- Added an optional, replaceable appearance/re-identification layer: `AppearanceProvider` + `ProcessAppearanceProvider` (external helper, no ML runtime linked into the C++ core), `AppearanceTypes` (unit embeddings, cosine similarity, explicit accept/reject thresholds, structured `AppearanceVerdict` evidence), `TargetCropExtractor` (deterministic crop via `EquirectView`), and `IdentityReidentifier` (documented precedence: explicit selection > tracker continuity > unique geometric continuation confirmed/vetoed by appearance > appearance-only re-acquisition > unresolved).
- Extended `TargetIdentityRegistry` with structured appearance profiles and decisions; appearance-vetoed tracks cannot be silently re-accepted by geometry.
- Selected a permissively licensed model for real validation: OpenVINO OMZ `person-reidentification-retail-0277` (Apache-2.0 code and weights, internal training data) via ONNX Runtime. Rejected OpenCV Zoo YoutuReID (unlicensed weight source, research datasets) and OSNet/torchreid weights.
- 24 new model-free tests; full suite 264 passed, 0 failed, 1 skipped. Decision 022 recorded; Decisions 017-021 preserved.
- Not implemented: active-speaker/audio-visual association, GPU optimization, UI integration, biometric face recognition.

## v0.2.43 — Optional Audio/Speaker Evidence (Active Speaker)

Date: 2026-09-17

- Added an optional, replaceable audio/speaker layer: `SpeakerEvidenceProvider` + `ProcessSpeakerProvider` (external helper, no audio/ML runtime linked), structured `SpeakerTypes` (intervals, analysis, per-target evidence, timeline segments, explicit verdicts), `SpeakerTargetAssociator` (deterministic speaker->target mapping), `SpeakerTimeline` (documented hysteresis), `SpeakerEvidenceAnalyzer`, and `SpeakerReframePlanner` (speaker-follow `ReframePlan` with cuts).
- Audio is evidence only: `TargetIdentityRegistry::annotateSpeaker` records speaker evidence without changing identity resolution; explicit creator selection remains authoritative. Identity precedence is unchanged (explicit > continuity > geometry > appearance > audio > unresolved).
- Selected Silero VAD (MIT code and weights) via ONNX Runtime for real speech activity; rejected/deferred pyannote, SpeechBrain/torchreid, cloud APIs, and audio-visual active-speaker models.
- 31 new model-free tests; full suite 295 passed, 0 failed, 1 skipped. Real speech detection and speaker-associated rendering verified on real 360 footage (audio+video proxy; original untouched). Decision 023 recorded; Decisions 017-022 preserved.
- "Follow whoever is speaking" works through the structured selection layer; automatic audio-visual attribution without an explicit binding, GPU optimization, and UI integration remain future work.

## v0.2.44 — Audio-Visual Provider Attribution Seam

Date: 2026-09-17

- Completed the audio-visual **provider-attribution seam** under the objective's efficiency guardrail: `SpeakerInterval`/`SpeakerSegment` gained an optional `targetIdHint` (an existing target track id an audio-visual/diarization provider attributes), and `SpeakerTargetAssociator` precedence is now explicit creator binding > visible provider hint (`provider-hint`) > spatial DoA > single visible > ambiguous/unassociated. A non-visible hint falls through; a hint never invents a target and never overrides the creator.
- `SpeakerTimeline` propagates the hint onto the coalesced segment (first non-empty wins; cleared for overlap/silence); `SpeakerEvidenceAnalyzer` honours it end to end.
- Documented the feasibility boundary that prevents shipping a model-backed automatic provider here: the real footage audio is mono (no direction of arrival); a face-detection + mouth-motion/audio-envelope correlation probe scored 0.250 (speaker) vs 0.192 (listener) — not a reliable discriminator; and no permissively licensed, clearly commercial audio-visual active-speaker model was identified. The seam is complete so a licensed provider can be added later without core changes.
- 5 new model-free tests; full model-free suite 300 passed, 0 failed, 1 skipped. Decision 024 recorded; Decisions 017-023 preserved.
- Next per the human-approved priority: end-to-end 360 user-command testing rather than further perception subsystems.

## v0.2.45 — End-to-End 360 User-Command Execution

Date: 2026-09-17

- Added `app/reframe/ReframeCommandRunner`: a single composition entry point that turns a user instruction plus a 360 source into a validated `ReframePlan` and, optionally, a rendered flat video. It parses the command, resolves subject references through the existing replaceable detector/tracker, applies identity/selection ("me", ordinals, left/right, unique label), builds the validated plan, and executes it through the unchanged `ReframePipeline`.
- `prepare()` is a deterministic, model-free-testable decision stage (parse + resolve + plan) when an in-memory detector/provider is injected; `run()` adds deterministic execution. The runner adds no perception of its own and never fabricates a subject direction: unresolved or ambiguous references produce an explicit error, and a direction-only command needs no detector.
- 8 new model-free tests (subject resolution + plan, direction-only without a detector, unresolved/ambiguous honesty, creator-identity "follow me", missing detector, invalid range, determinism). Full model-free suite 308 passed / 0 failed / 2 skipped.
- New env-gated `realUserCommandIntegration` test exercises the full command path on real 360 footage with the Apache-2.0 YOLOX detector; the normal suite stays model-free.
- Decision 025 records the composition boundary; Decisions 017-024 preserved.

## v0.2.46 — Application-Level 360 Command Orchestration

Date: 2026-09-17

- Wired the 360 command path into the application. `Application::runReframeCommand()` / `runReframeCommandTo()` select the active media, validate application state and the output location, build a `ReframeCommandRequest`, delegate to the existing `ReframeCommandRunner`, and expose a structured `ReframeCommandOutcome` (source reference, instruction, effective range, output spec/path, frame count, notes, unresolved references, resolved targets; JSON-serializable). No runner logic is duplicated and the source media is never modified.
- Added optional, non-owned detector/frame-provider inputs and an injectable command-executor seam; `main.cpp` builds a `ProcessTargetDetector` from `REELCRAFT_TARGET_*` when configured, so subject commands work in the product without linking an ML runtime.
- Added a minimal command UI to `MainWindow` (command input, start/end seconds, run button, result label) wired through `reframeCommandRequested`/`reframeCommandFinished`.
- 17 new model-free application tests (validation, delegation, outcome/error propagation, determinism, source non-modification, UI). Full model-free suite 326 passed / 0 failed / 3 skipped. New env-gated `realApplicationCommandIntegration`.
- Hardened `TargetResolver::resolveSequence`: a single undecodable sample (for example the exact end of a clip) is now recorded and skipped instead of aborting the whole sequence, so range-end sampling cannot fail resolution.
- Decision 026 records the orchestration boundary; Decisions 017-025 preserved.

## v0.2.47 — Persisted Outputs and Duration-Aware Ranges

Date: 2026-09-17

- Added a replaceable media-duration seam: `MediaDurationProbe` + `FfprobeDurationProbe` report a clip's duration with the external `ffprobe` (resolved via `REELCRAFT_FFPROBE`, a sibling of the ffmpeg executable, or `PATH`). No codec dependency is linked, the source is only read, and every failure is deterministic.
- A zero start/end range now means "the whole clip": `Application::runReframeCommandTo()` resolves it through the probe to `[0, durationMs]`. Explicit ranges are used directly and never probe; when the duration is unknown the command runner honestly reports an invalid range (or uses a range in the command). The UI range controls default to 0/0 (whole clip).
- Generated render records are now persisted: every command that reaches an output target appends its structured `ReframeCommandOutcome` (success or failure with its error); `Project` gained an additive `reframeOutputs` section and `CurrentSchemaVersion` is 3 (schema-2 projects load with an empty list; future schemas are still rejected). The application emits `reframeOutputsChanged` and the UI re-lists the records.
- 13 new model-free tests; full model-free suite 339 passed / 0 failed / 3 skipped. `realApplicationCommandIntegration` extended to whole-clip ranges and save/open persistence. Decision 027 recorded; Decisions 017-026 preserved.

## v0.2.48 — Speaker-Aware 360 Commands

Date: 2026-09-17

- The 360 command path now understands speaker references: `ReframeCommandRunner` recognizes "follow the speaker", "keep the speaker centered", "center the speaker", and related phrasings and reuses the existing Objective 6/7 layers (`SpeakerEvidenceAnalyzer` + `SpeakerReframePlanner`) to turn the associated speaker timeline into the deterministic plan. No new perception and no parallel command system.
- Audio is evidence: an optional, replaceable `SpeakerEvidenceProvider` is required for a speaker command; explicit creator `speakerId -> targetId` bindings are honoured first; an unassociated or ambiguous speaker, or a command mixing a speaker reference with another subject or an explicit direction, is reported honestly with no fabricated plan.
- Added `ReframePipeline::renderPlan()` to render an already-validated plan with the same deterministic renderer; `run()` delegates to it and `ReframeCommandRunner::run()` renders the prepared speaker plan instead of re-deriving it.
- `Application` holds a non-owned speaker provider and optional bindings and copies them into each command request; `main.cpp` builds a `ProcessSpeakerProvider` from `REELCRAFT_SPEAKER_PY`/`_SCRIPT`/`REELCRAFT_SILERO_MODEL` when configured.
- 9 new model-free tests; full model-free suite 348 passed / 0 failed / 4 skipped. New env-gated `realSpeakerCommandIntegration`. Decision 028 recorded; Decisions 017-027 preserved.

## v0.2.49 — 360 Command UI and Rendered-Result Preview

Date: 2026-09-17

- The 360 command workflow is now usable from the application. `Application::selectCreatorTargetFromViewport()` seeds the creator identity "me" from the current viewport direction and preview time, and `clearCreatorSelection()` clears it; the selection is passed into every command request so "follow me" / "keep me centered" can resolve without a known track id. It is session state and is cleared on new/open project.
- Added rendered-result preview: `Application::previewReframeOutput(index)` decodes the first frame of a persisted render record through an injectable decoder seam (defaulting to the external-FFmpeg `FrameExtractor`) and emits `reframeOutputPreviewReady`; the viewer presents it flat (no equirectangular camera transform). Invalid indices, missing outputs, and decode failures are reported honestly.
- Minimal UI: "Select Center as Me"/"Clear Me" with a selection readout, "Preview Selected Render" on the existing render list, and a provider status line. `main.cpp` wires them; external detector/speaker providers remain configured through `REELCRAFT_*`.
- 11 new model-free tests; full model-free suite 359 passed / 0 failed / 4 skipped. Decision 029 records the human-selected Objective 12 scope; Decisions 017-028 preserved.

## v0.2.50 — 360 Rendered-Result Playback

Date: 2026-09-17

- Added deterministic continuous playback of persisted 360 -> flat rendered results: `Application` owns/creates/replaces/disposes the playback `FrameSource` (default `FfmpegFrameSource` opened with the record's geometry), `FramePump`, and `Player` for the selected render, with `startReframeOutputPlayback`/`pauseReframeOutputPlayback`/`resumeReframeOutputPlayback`/`stopReframeOutputPlayback`/`tickReframeOutputPlayback`.
- The Application owns the event-loop driver (a `QTimer`); the Player owns no timer or thread. Reuses the existing Phase 3 `FrameSource`/`FfmpegFrameSource`/`FramePump`/`Player`/`Playhead`/`Clock`/`PacingPolicy` seams — no parallel playback architecture.
- Playback frames are emitted as `reframePlaybackFrameReady` and presented flat; `reframePlaybackStateChanged`, `reframePlaybackPositionChanged`, and `reframePlaybackEnded` report progress. Invalid index, missing output, unknown dimensions, source-open failure, decode error, and end-of-stream are honest; the source media and outputs remain read-only.
- The Objective 10 preview-time contract and the single-frame render preview are preserved. Minimal UI adds Play/Pause/Stop Render controls and a playback position readout.
- 9 new model-free tests; full model-free suite 368 passed / 0 failed / 5 skipped. New env-gated `realReframePlaybackIntegration` renders a real 360 clip to a flat result and plays it back. Decision 030 records the scope and Definition of Done; Decisions 017-029 preserved.

## v0.2.51 — 360 Temporal Editing Operations

Date: 2026-09-17

- Added a deterministic temporal-editing representation: \`app/reframe/TemporalEditPlan.{h,cpp}\` is a validated, JSON-serializable, parser-independent value with Keep / Remove / TargetDuration operations, ordered multi-ranges, deterministic normalization, and \`resolve(durationMs, defaultStartMs)\`. Reversed, zero-length, negative, and out-of-bounds ranges, empty results, and non-positive durations are rejected; Remove resolves to the complement and TargetDuration to a window.
- Extended the existing \`ReframeIntentParser\` / \`ReframeIntent\` (no parallel parser) to produce \`temporalEdit\` from "cut from X to Y", "remove X to Y", "keep X and Y", "make a N-second version", and "this section", including word timestamps such as "35 seconds" and "1 minute 10". Invalid time ranges, contradictory keep+remove, unsupported/ambiguous "cut", and out-of-bounds ranges are explicit and honest; no timestamp is invented.
- \`ReframePlan\` gained an additive ordered \`segments\` list (empty = the existing single source range) that changes only frame timing and camera-path evaluation time; \`frameCount()\`/\`frameTimeMs()\`/\`isValid()\` honor it and the existing \`ReframeRenderer\`/\`ReframePipeline\` execute it with no new rendering architecture. \`ReframeCommandRunner\` resolves the edit against the known source duration and composes it with the existing target/identity/speaker resolution.
- \`Application::runReframeCommandTo()\` supplies the probed whole-clip duration and the resolved retained ranges are persisted in \`ReframeCommandOutcome\` (JSON round-trippable). The resulting flat render is a normal record that plays through the existing Objective 13 playback. The source media remains read-only.
- 11 new model-free tests plus an env-gated \`realTemporalEditIntegration\`; full model-free suite 379 passed / 0 failed / 6 skipped. Real 360-footage validation retained two ranges, rendered a 320x180 @ 10 fps result, and played all frames back. Decision 031 records the scope and Definition of Done; Decisions 017-030 preserved.

## v0.2.52 — 360 Compound Natural-Language Editing

Date: 2026-09-17

- Fixed the Objective 14 limitation where a single natural-language command combining a temporal edit and a camera/target instruction lost one of the operations. The existing `ReframeIntentParser` now strips the temporal clause's time ranges (or a target-duration phrase) before camera/target extraction, so both halves of a compound "and" command survive: "Keep 0:00 to 0:30 and follow me." now yields KEEP [0:00, 0:30] plus the "me" target. No second parser, temporal representation, renderer, or playback path was added.
- `ReframeIntent::hasCompoundEdit()` explicitly represents the composition. The deterministic default rule is that the camera/target instruction applies to the entire retained temporal range; multiple camera moves keep the existing camera-path interpolation.
- Supported compositions: temporal + target ("Keep 0:00 to 0:30 and follow me."), temporal + explicit target ("From 0:35 to 1:10, keep the person I selected centered."), temporal + speaker ("Keep 0:35 to 1:10 and follow whoever is speaking."), and target-duration + target ("Make a 30-second version and keep me centered."). Invalid, ambiguous, and contradictory temporal edits remain honest, and a camera instruction with its own separate time interval is rejected as unsupported.
- 2 new model-free tests plus an env-gated `realCompoundCommandIntegration`; full model-free suite 381 passed / 0 failed / 7 skipped. Real 360-footage validation retained one range and the left camera direction, rendered a 320x180 @ 10 fps result, and played it back. Decision 032 records the scope and Definition of Done; Decisions 017-031 preserved.

## v0.2.53 — Persisted, Reproducible Edit Decisions

Date: 2026-09-17

- A generated 360 render can now be reproduced from the project alone. A new versioned `EditDecision` artifact (`app/reframe/EditDecision.{h,cpp}`) records the resolved `ReframePlan` — concrete camera keyframes, retained segments and output specification — together with the media it was made against (id, path, size + modification-time fingerprint), the originating instruction as provenance, and a `decisionHash`. Replay needs no natural-language parsing and no perception provider: the stored plan plus the source path is sufficient.
- The artifact carries `schemaVersion` from day one and its loader refuses to mis-parse: missing, unknown, future, or reversed versions, malformed source references, invalid plans, and digests that disagree with the recomputed value all fail with descriptive errors. Serialization is deterministic (Qt sorts JSON keys; `decisionHash` is SHA-256 over the compact payload with the `decisionHash` key excluded and `createdUtc` included), so digests are stable across processes and meaningful for diffs.
- Decisions are embedded additively inside the existing render records, so the project schema stays at 3 and no new database, store, format, renderer or pipeline was introduced. The artifact's own `schemaVersion` is the compatibility gate.
- Record loading is lenient about the decision but strict about its contents: a record whose decision cannot be read still loads — the record is the historical fact that a render happened — and is flagged with the reason, preserved verbatim so re-saving cannot destroy it, and reported to the user. Records written before this version load unchanged.
- `Application::replayEditDecision()` re-renders a stored decision to a caller-supplied path through the existing `ReframePipeline::renderPlan()`, validating fully before any render: index, decision presence, source fingerprint (a missing file and a changed file are reported as separate failures), a non-empty path, a path that is not the source media, and a path that does not already exist. Every refusal appends no record and leaves existing files untouched. On success it appends a new record carrying the same decision; the original record is never modified.
- Reproducibility is verified in-process and in a genuinely fresh OS process: both re-render from the persisted artifact and match the original's decoded frames byte for byte. The frame-level guarantee holds across ffmpeg builds; byte-identical container output is asserted here but is environment-specific.
- Verification also fixed a long-standing development-environment fault: a leaked Termux `PATH` entry made the Android ffmpeg run against Debian libraries, costing ~15 s per frame against a 15 s budget. With Debian's ffmpeg installed, a frame costs ~2.6 s, the suite runs in ~221-241 s instead of ~850-1,021 s, and `scripts/build_and_test.sh` is viable again. See `DEVELOPMENT_ENVIRONMENT.md`.
- 19 new tests; full suite 401 passed / 0 failed / 8 skipped. Decision 033 records the scope and Definition of Done; Decisions 017-032 are preserved.


## v0.2.54 — Creator Decision Provenance & Revision

Date: 2026-09-18

- A synced render decision can now record where it came from and be revised without ever altering the original. `EditDecision` gained an optional `origin` (`command` or `creator-revision`) and an optional single-parent `parentDecisionHash`; both are omitted when unset so that every existing v1 decision keeps its exact payload and digest.
- The decision schema advanced from v1 to v2. Existing v1 decisions remain fully readable, and a loaded decision **keeps its own version** rather than being silently upgraded — a v1 decision re-serializes byte-identically and its recorded digest still verifies.
- Creator revision is immutable by construction: a revision produces a **new** decision whose only parent is the decision it revises. There is no mutator for a stored decision, and the parent record is left byte-identical. Revisions use free text through the existing parser -> plan builder -> render pipeline; there is no structured operation or timeline editing.
- New application surface: a revision method that refuses a bad record, an empty instruction, a colliding output path, or a source whose fingerprint has drifted; and a read-only provenance view exposing origin, instruction, lineage and source-fingerprint status, which resolves the recorded parent against the records actually held rather than trusting a well-formed hash.
- Replay is unchanged and deliberately does not re-stamp provenance: a replayed record carries the same decision, preserving its digest and origin.
- Fixed a silent failure: render records that could not be restored were previously discarded without any message. They are now reported through the application's status channel.
- Added structured decision-lifecycle logging (created / loaded / refused / revised) using Qt logging categories, with no new dependency.
- 9 new tests; targeted regression 31 passed / 0 failed / 0 skipped. Decision 034 records the scope, the version-retention and omission rules, and the out-of-scope list.


## v0.2.55 — Objective 18 Scoped: Deterministic Intent -> Plan Contract Checker

Date: 2026-09-18

- Objective 18 was formally scoped in Decision 035. It is a **documentation-only** entry: no code, schema, dependency or behaviour changed, and implementation is not authorized.
- The objective is a deterministic, pure contract checker over the final `(ReframeIntent, ReframePlan)` pair, bounded to three rules: output specification fidelity, requested time-range containment, and temporal edit materialisation. Each is fatal on violation.
- Rule families that cannot be decided from the pair are explicitly excluded, with reasons recorded: keyframe-count <-> move-count and per-move direction correspondence (the speaker planner owns its keyframes), target identity, media identity, and anything already guaranteed by `ReframePlan::isValid()` or by command preparation.
- Seven intentional behaviours are recorded as exceptions that must never be reported as violations: temporal edits may widen the final source range; empty camera instructions may synthesize a centered-forward keyframe; missing output and time range use caller defaults; target references are resolved into camera coordinates and not retained; labels and notes are descriptive only; speaker-path keyframes are planner-owned.
- No persisted schema change and no stored checker result are authorized; persisted edit decisions remain immutable.


## v0.2.56 — Deterministic Intent -> Plan Contract Checker

Date: 2026-09-18

- Added a deterministic contract checker that verifies a generated reframe plan against the request it was built from, so a plan that would not honour the request is refused instead of silently executed.
- Three checks run on every command, on both the ordinary and the speaker code paths: the output specification must match what was asked for; the requested time range must be covered (allowing the intentional widening that temporal edits perform); and a requested cut must actually retain at least one segment. Each check is skipped where the caller's default legitimately supplies the value, and each is fatal on failure — the command fails with a clear message and nothing is rendered or persisted.
- Failures name the rule and both observed values (for example, the requested and actual output specification), so a mismatch is diagnosable without re-running the command.
- No behaviour change for valid requests: every existing command path, including temporal edits, compound commands and speaker-following, produces plans that satisfy the contract. The checker exists to catch future regressions, and it deliberately does not attempt to judge semantic understanding of the request.


## v0.2.57 — Real 360 Source Playback

Date: 2026-09-18

- Real 360 footage can now be watched inside Reelcraft. The active media plays continuously rather than one frame at a time, and the creator can look around while it plays: the 360 viewpoint controls keep working during playback.
- Playback controls: play, pause, seek to a position, resume and stop, with a source position readout. Reaching the end of the media stops cleanly.
- Playback uses one persistent decoding process for the whole session instead of spawning a decoder per displayed frame, and decodes a bounded 2:1 proxy so viewing cost does not scale with the source resolution. The original footage is only ever read.
- Seeking re-opens the stream at the requested position and keeps whether playback was running. Playback speed follows the source frame rate when it can be determined, and falls back to a documented default otherwise.
- Rendered-result playback is unchanged, and the two playback modes are mutually exclusive: starting one stops the other.
- **Known limitation:** source playback is silent. Reelcraft still has no audio output, and adding one is a separate objective; source playback was kept extensible for it rather than expanding this one. Rendered output remains silent as before.
- 9 new tests; targeted regression 44 passed / 0 failed / 0 skipped; real-media validation passed on a real 360 clip. Decision 036 records the architecture.


## v0.2.58 — Persistent Render Decoding and Deterministic Throughput

Date: 2026-09-18

- Rendering a 360 source no longer starts a new video decoder for every frame it produces. The render path keeps one decoding stream open across the frames it renders and reads forwards from it, so the number of decoder processes follows the number of edit anchors rather than the number of output frames.
- What is rendered is unchanged: the new path produces exactly the same frames and the same output files as before, including for trimmed edits and for edits built from several separate time ranges. This is verified by comparing decoded frames and the output files themselves.
- A far jump forward in the source re-opens the stream at that position instead of decoding through everything in between, and every case the streaming path cannot serve — an unknown frame rate, an unreadable source, the end of the media — falls back to the previous frame-accurate seek path. No frame is ever guessed.
- Source frame rate is read once per source, and when it cannot be determined rendering behaves exactly as it did before.
- Measured on a real 360 render of six output frames, FFmpeg process creations fell from 9 to 5, with no decoder process scaling with the frame count.
- 6 new tests; full suite 431 passed / 0 failed / 9 skipped. Decision 037 records the architecture.


## v0.2.59 — Persistent Media Analysis

Date: 2026-09-18

- Reelcraft now records what it has learned about a piece of footage, instead of re-examining the same material every time it is asked to do something.
- The record is a separate, versioned file kept beside the project rather than inside it, so it does not bloat the project file and can be moved, deleted or regenerated safely. The project itself stays compatible with existing projects and simply keeps a small reference to it.
- What the system knows is stored as independent capability layers, each with its own status and the time ranges it actually covers. A capability that could not run is reported as unavailable with a reason, and is never confused with a capability that ran and found nothing.
- Two capabilities are included: a technical layer (duration, frame rate, resolution, aspect ratio, whether the footage has audio, and its declared projection) and a target layer that records where detected people or objects are, over time, as positions on the sphere rather than positions in a particular view.
- Analysis is done in one pass with a single decoder, at a resolution chosen for analysis rather than for playback, and never at the full source resolution. Both the analysis resolution and how densely the footage was sampled are recorded with the results.
- Anything written by a newer version of Reelcraft is preserved rather than discarded, so opening and re-saving a project cannot destroy information this version does not understand.
- **No new intelligence was added.** There is still no transcription, no scene understanding, no shot or B-roll reasoning, no language model and no creator-preference learning; those remain future work. This release adds the place where such capabilities will be recorded.
- Existing behaviour is unchanged: renders and replays produce byte-identical results whether analysis is present, absent, stale or unreadable. 15 new tests; targeted regression 31 passed / 0 failed / 0 skipped. Decisions 038-042 record the design.


## v0.2.60 — Following a subject now actually follows

Date: 2026-09-18

- Asking Reelcraft to **follow** someone now produces a camera that moves with that person over time. Previously a follow instruction — "follow me", "keep me centered" — resolved the subject's position and then held the camera still there for the whole clip, which is not what following means.
- Instructions that ask to **aim** at something stay exactly as before: "look at the car" points the camera at the car and holds it. The difference is now explicit in how the instruction is understood.
- When the subject cannot be tracked continuously across the requested range, the command falls back to the previous fixed-camera behaviour and says why, rather than failing or inventing a camera move.
- Nothing else changed: rendering, playback, replay and saved decisions behave exactly as before, and renders remain reproducible.
- 4 new tests cover the behaviour, including an end-to-end run on real 360 footage in which the subject walks across the sphere; the rendered result is decoded back to confirm it is a valid video. Targeted regression 45 passed / 0 failed / 0 skipped.


## v0.2.61 — Follow camera paths move more smoothly

Date: 2026-09-18

- When Reelcraft follows a subject, the camera now tracks that subject's movement several times more finely than before. Previously a follow instruction sampled the subject's position only a handful of times across the whole clip, so the camera moved in a few large jumps regardless of how long the section was.
- How finely the subject is sampled is now a property of following specifically. Instructions that aim at something — "look at the car" — behave exactly as before, with the same accuracy and the same cost.
- The sampling is measured in time rather than in a fixed number of looks, so a longer section does not become coarser, and it is capped so that following cannot become disproportionately expensive on a long clip.
- Measured on a fixed test subject sweeping across the frame: a four-second follow section now produces seventeen camera positions at quarter-second spacing, where it previously produced five at one-second spacing. The path covers exactly the same movement — only its resolution changed.
- Nothing about how subjects are detected, tracked or identified changed, and nothing about how edits are stored or replayed changed.
- 1 new test measuring five sampling densities; targeted regression 46 passed / 0 failed / 0 skipped. Decision 044 records the design.


## v0.2.62 — Following a subject costs far less to compute

Date: 2026-09-18

- Following a subject no longer starts a fresh video decoder for every position it samples. One decoder is now kept running and reused across the whole trajectory, so following is both faster and lighter.
- Measured on the development device, resolving a six-position trajectory went from six decoder launches in about fifteen seconds to three launches in about ten seconds, with exactly the same result. Real 360 footage follows the same pattern, and the saving grows the more finely the subject is sampled.
- The frames used are identical to the ones used before — this changes how they are fetched, not what is seen — so the resulting edit and camera movement are unchanged.
- If the footage's frame rate cannot be determined, or the reusable decoder cannot be used for any reason, Reelcraft falls back to the previous behaviour automatically rather than failing.
- Nothing about how subjects are detected, tracked or identified changed, and no stored edit or project data changed.
- 51 targeted regression tests passed with none failing. Decision 045 records the design.


## v0.2.63 — Follow camera movement is smoother

Date: 2026-09-18

- The camera now moves more smoothly when following a subject. It previously tracked the detected position of the subject exactly, sample by sample, so any wobble in the detection appeared as a wobble in the camera, and the movement changed speed abruptly at every sample.
- The smoothing is deliberately built so that it cannot make the camera lag or drift: a subject moving steadily across the frame is followed exactly as before, and a subject standing still produces a perfectly steady camera. Only jitter is removed.
- Following a subject who walks around behind the camera — across the point where the view wraps around — continues the short way round instead of spinning the long way.
- Following is still centred framing. "Follow the person" and "keep me centered" behave exactly as they always have.
- Instructions that aim at something rather than follow it are untouched, as are explicit camera directions.
- Nothing about how subjects are detected or identified changed, no stored edit or project data changed, and following costs no extra decoding.
- 4 new tests; targeted regression 62 passed / 0 failed / 0 skipped. Decision 046 records the design.



## v0.2.64 — Someone standing near the edge of a camera angle is now one person

Date: 2026-09-18

- Reelcraft now recognises the same person when two of its viewing angles see them at once. Previously, a subject standing near the boundary between two angles could be counted twice, because the second angle only saw a thin edge of them and reported the position of that edge rather than the position of the person. The result was that asking Reelcraft to follow someone in that spot could refuse, saying it could not tell which person was meant.
- The fix uses information the system already had. Each detection reports how large the subject appears, so a duplicate is now recognised when the two reports overlap in size as well as position — the situation that only arises when one view has caught part of a subject another view has already seen whole.
- Two people standing close together are still treated as two people. This was verified directly: two subjects twelve degrees apart remain separate, two subjects far apart remain separate, and a single subject remains one.
- The threshold at which two positions are considered the same person was deliberately **not** widened. Widening it would have solved this case by declaring that people are never more than a certain distance apart, which is not true, and would have merged genuinely separate people elsewhere in the frame.
- Four tests that had been carrying a wider setting to work around this problem no longer do, so the behaviour that ships is the behaviour the test suite checks.
- Nothing else about how subjects are detected, tracked or identified changed, no stored edit or project data changed, and following resolves at the normal settings. 3 new tests; targeted regression 73 passed / 0 failed / 0 skipped. Decision 047 records the design.


## v0.2.65 — Reframed results now keep their sound

Date: 2026-09-18

- A 360 reframe no longer comes out silent. The flat video Reelcraft produces now carries the audio of the footage it kept, so a reframed clip can be watched (or handed on) as a finished result instead of a mute picture.
- When an edit keeps only part of the footage — a trimmed section, or several separate sections joined together — the sound follows exactly the same sections, in the same order, and starts at the beginning of the result. Nothing from the parts that were left out is heard.
- The picture is unchanged in every measurable way: it comes out of exactly the same encoding path as before. For footage that simply has no sound, the result is byte-for-byte the file Reelcraft produced before this change.
- The sound is re-encoded with fixed settings rather than copied, so the same edit always produces the same result; the original file is only ever read.
- If the footage's audio cannot even be identified, Reelcraft renders the silent result it produced before and says why, rather than guessing. If audio is identified but cannot be produced, the render fails honestly instead of quietly handing back a silent file.
- Still to come: Reelcraft cannot *play* sound. Playback inside the application remains pictures-only, and adding that is a separate piece of work with its own dependency decision; this change is about what Reelcraft exports.
- 7 new tests, all on generated fixtures, none requiring real footage; targeted regression 20 passed / 0 failed / 0 skipped and the full model-free suite 466 passed / 0 failed / 9 skipped. Decision 048 records the design.


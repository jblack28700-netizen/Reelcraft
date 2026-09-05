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

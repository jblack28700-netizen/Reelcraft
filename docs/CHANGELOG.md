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

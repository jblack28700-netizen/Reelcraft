# Reelcraft — Current State

## Current Version

0.1.0

## Current Branch

main

## Current Stage

Phase 1 Desktop Application Foundation — Complete

## Project Status

Reelcraft contains the minimal, testable Phase 1 desktop application foundation.

The repository establishes the persistent product requirements, project model, AI editing contract, architecture, development workflow, and project-control documentation required before production implementation begins.

## Completed Foundation Work

- Master project guide
- Product requirements
- Conceptual project model
- AI-to-deterministic editing contract
- Technical architecture
- Architectural decisions
- AI handoff instructions
- Project history
- Development log
- Known issues
- Changelog
- Development environment documentation
- Controlled incremental development workflow

## Current Objective

Define and implement the smallest safe Phase 1 application foundation using the validated technical direction.

The objective is to establish a working, testable desktop application shell with clean boundaries between the UI/application layer, project state, future AI orchestration, and deterministic media execution. The implementation must remain narrowly scoped and must not prematurely lock downstream systems or implement major editor features.

## Current Work

The project has transitioned from:

    Documentation + conceptual architecture

to:

    Documentation + validated technical direction

The technology/runtime evaluation is complete enough to define and begin the smallest safe Phase 1 application-foundation objective.

Production implementation remains strictly scoped to the active Phase 1 objective. Major editor features and downstream systems must not be started automatically.

## Application Implementation
Status: Phase 1 foundation implemented and verified.

A minimal Qt 6 desktop application shell exists with UI/application-core separation, project lifecycle support, background task demonstration, and an automated Qt Test suite.

## Media Processing

Status: Not implemented.

The deterministic media-processing architecture is defined conceptually, but no production media engine has been implemented.

## AI Editing

Status: Not implemented.

The conceptual AI-to-deterministic edit contract has been established, but no production AI editing system or provider integration has been implemented.

## 360° Video

Status: Architectural and contractual requirement only.

360° video is a first-class requirement. The project has documented conceptual requirements for 360° media, but 360° viewing, processing, reframing, and export systems have not yet been implemented.

## Testing

Status: Application testing infrastructure not yet implemented.

Documentation and repository verification are being performed during the foundation stage. Production application testing infrastructure will be established as part of the implementation architecture.

## Technology Direction

Status: Not yet finalized.

The following remain intentionally open pending technology evaluation:

- Final UI framework
- Final application runtime
- Project file format/schema
- Storage implementation
- AI provider selection
- AI model selection
- Local/cloud workload allocation
- Rendering architecture
- GPU acceleration strategy
- Database requirements
- Plugin/extension architecture
- Packaging and deployment strategy

Technology candidates must be evaluated against the documented requirements and architectural boundaries rather than selected solely by familiarity or popularity.

## Known Limitations

- No production application implementation exists.
- No production media engine exists.
- No production AI provider integration exists.
- No production voice system exists.
- No 360° viewer or reframing system exists.
- Automated application testing infrastructure is not yet implemented.
- Final technology choices remain unresolved pending evaluation.

These limitations are intentional at the current stage.

## Development Environment

Current development uses:

- Android
- Termux
- Ubuntu through proot-distro
- Bash
- Git
- Node.js
- npm
- FFmpeg

Active project directory:

    /root/reelcraft

## Development Rules

All implementation must follow MASTER_GUIDE.md and AI_HANDOFF.md.

Core rules:

- One active development objective at a time.
- Inspect before changing.
- Protect existing work.
- Make small logical changes.
- Test after meaningful changes.
- Perform regression testing.
- Use Git checkpoints for meaningful or risky changes.
- Keep experiments separated from stable work.
- Update persistent documentation.
- Never claim completion without verification.
- Stop when requirements or architecture conflict.
- Preserve technology flexibility until sufficient evidence supports a decision.

## Next Stage

The technology/runtime evaluation is complete enough to establish a validated technical direction for Phase 1.

The next objective is to define and implement the smallest safe Phase 1 application foundation.

The Phase 1 implementation task must have explicit scope, success criteria, verification steps, and a Definition of Done.

Do not automatically begin a major feature after the foundation task.

## Phase 1 Desktop Application Foundation — Verified

Status: Complete.

Implemented:
- Minimal Qt 6 desktop application shell
- UI/application-core boundary with signal/slot communication
- Project lifecycle: create, identify, persist, reopen, close
- Non-destructive JSON project state storage
- Background task demonstration using QtConcurrent
- Error handling for invalid save/open paths
- Automated Qt Test suite covering Project and Application behavior

Verification:
- Application build: BUILD_EXIT=0
- Automated tests: 9 passed, 0 failed, TESTS_RUN_EXIT=0
- Offscreen launch smoke test: event loop remained alive until timeout (RUN_EXIT=124)

Boundaries preserved:
- Original media is not modified
- AI reasoning remains separate from deterministic execution
- Production media engine, timeline, and AI editing remain unimplemented

## Phase 1 UI Integration Verification — Complete

Status: Complete.

Added automated UI-level verification:
- MainWindow button signal emission
- Project label updates
- Status label updates
- Application save/open error paths

Verification:
- Application build: BUILD_EXIT=0
- Tests build: TESTS_BUILD_EXIT=0
- Automated tests: 14 passed, 0 failed, TESTS_RUN_EXIT=0

## Phase 1 Original Media Safety Verification — Complete

Status: Complete.

Added automated evidence that project lifecycle operations do not modify original media.

Verification:
- Test creates a byte fixture representing source media
- Test computes SHA-256 before and after create/save/reopen
- Test verifies bytes and hash remain identical
- Automated tests: 15 passed, 0 failed

## Platform Strategy

Status: Documented.

- Desktop: first-class supported platform at the current stage.
- Android: planned future platform, not a current product target.
- Termux/Ubuntu on Android: development environment only; does not constitute native Android support.

This classification is recorded in DECISIONS.md and ARCHITECTURE.md and does not introduce Android support or modify application source.


## Expanded Platform Strategy — 2026-09-03

Status: Documented.

- Linux desktop: first-class / currently verified
- Windows desktop: planned future desktop platform
- macOS: planned future desktop platform
- Android: planned future platform
- iOS: planned future consideration
- iPadOS: planned future consideration
- Termux/Ubuntu: development environment only

No Apple or Android implementation has been started. No application source files were changed for this classification.

## Phase 1 Real Desktop Smoke Test — Complete

Status: Complete.

Verified against a real Qt X11 desktop session using Termux:X11:
- Reelcraft launched and rendered the main window.
- New Project displayed generated project name and UUID.
- Save Project persisted JSON project to /tmp/reelcraft-smoke.reel.
- Open Project reopened the persisted project with matching name and UUID.
- Run Background Demo updated status without freezing the UI.
- Automated regression tests passed: 15 passed, 0 failed.

Environment observations:
- No window title/decorations visible because Termux:X11 lacks a window manager; not considered a Reelcraft defect.
- Known Qt locale warning appears; no functional impact.

## Phase 1 Clean-Checkout Workflow Verification — Complete

Status: Complete.

Verified that the existing build-and-test workflow works from a clean Git checkout:
- Cloned current commit into a temporary directory
- Ran scripts/build_and_test.sh
- Application build succeeded
- Test build succeeded
- Automated tests: 15 passed, 0 failed
- No application source changes


## Phase 1 Formal Completion

Status: Complete — 2026-09-03.

Phase 1 Desktop Application Foundation is formally complete.

Verified Definition-of-Done evidence:
- Desktop runtime/UI foundation builds successfully
- Reelcraft launches against a real Qt X11 session via Termux:X11
- Basic UI shell renders reliably
- UI-to-application/core communication is demonstrated
- Minimal project create/persist/reopen workflow works
- Original media is never modified by lifecycle operations
- Background work executes without freezing the UI
- Expected failure paths are handled safely
- Automated test suite: 15 passed, 0 failed
- Clean-checkout build-and-test workflow verified
- Architecture boundaries preserved
- Working tree clean

## Phase 2 Viewer State Foundation — Complete

Status: Complete.

Implemented the platform-independent `ViewportState` model for the future 360 viewer:
- Yaw and roll normalized to [-180, 180)
- Pitch clamped to [-90, 90]
- Field of view clamped to [20, 140]
- Change signals emitted only on actual state changes
- Independent of QtWidgets, rendering, playback, and camera/media code

Automated tests: 21 passed, 0 failed.

## Phase 2 Application-Level Viewport State — Complete

Status: Complete.

Integrated `ViewportState` into the `Application` layer:
- `Application` now owns a viewer state object.
- `Application::viewportState()` returns a valid non-null pointer.
- `Application::resetViewport()` restores default yaw/pitch/roll/FOV.

No rendering, playback, media, or camera-specific code was introduced.

## Phase 2 UI Viewer Readout — Complete

Status: Complete.

MainWindow now displays viewer yaw/pitch/roll/FOV and exposes Reset Viewport.
Viewer state is read-only at the UI layer and remains owned by Application.
No rendering, playback, media, or camera code was introduced.

## Phase 2 Viewer State Integration Tests — Complete

Status: Complete.

Added automated integration tests verifying the full viewer-state signal flow:
- Application-owned ViewportState updates MainWindow yaw/pitch/roll/FOV labels.
- Reset Viewport button returns MainWindow labels to defaults.

Automated tests: 27 passed, 0 failed.

## Phase 2 Keyboard Viewer Controls — Complete

Status: Complete.

Added keyboard controls to MainWindow:
- Left/Right adjusts yaw.
- Up/Down adjusts pitch.
- Q/E adjusts roll.
- Plus/Minus adjusts field of view.
- Deltas are emitted as signals; viewer state remains owned by Application.

Automated tests: 29 passed, 0 failed.

## Phase 2 New Project Viewer Reset — Complete

Status: Complete.

Application::newProject now resets viewer yaw/pitch/roll/FOV to defaults.
This ensures each new project starts from a neutral viewer orientation.

Automated tests: 30 passed, 0 failed.

## Phase 2 Open Project Viewer Reset — Complete

Status: Complete.

Application::openProject now resets viewer yaw/pitch/roll/FOV to defaults.
This ensures each opened project starts from a neutral viewer orientation.

Automated tests: 31 passed, 0 failed.


## Phase 2 Viewer State JSON Serialization — Complete

Status: Complete.

ViewportState now supports JSON object serialization/deserialization:
- toJsonObject()
- readFromJsonObject()
- Independent from Project persistence.

Automated tests: 33 passed, 0 failed.


## Phase 2 Viewer State Persistence — Complete

Status: Complete.

Viewer state is now persisted inside project JSON:
- Project stores an optional `viewerState` object.
- Application saves current viewer state before project save.
- Application restores viewer state after project open.
- Existing projects without viewer state remain valid.

Automated tests: 34 passed, 0 failed.


## Phase 2 Restored Viewer State UI Integration Test — Complete

Status: Complete.

Added an automated UI integration test proving that viewer state restored by Application::openProject updates MainWindow yaw/pitch/roll/FOV labels through the existing signal wiring.

Automated tests: 35 passed, 0 failed.


## Phase 2 Focused Button Keyboard Input — Complete

Status: Complete.

MainWindow now installs an event filter on viewer-related buttons so keyboard yaw/pitch/roll/FOV controls still work while a button has focus.

Automated tests: 36 passed, 0 failed.


## Phase 2 Invalid Viewer State Fallback Verification — Complete

Status: Complete.

Added an automated test verifying that a project containing invalid persisted viewer state still opens successfully and falls back to default viewer orientation.

Automated tests: 37 passed, 0 failed.


## Phase 2 Application-Level Viewer Adjust Slots — Complete

Status: Complete.

Viewer adjustment is now an application-layer operation:
- Application owns adjustViewportYaw/Pitch/Roll/FieldOfView slots.
- MainWindow delta signals connect to Application slots instead of UI-layer lambdas.

Automated tests: 38 passed, 0 failed.


## Phase 2 Clean-Checkout Verification — Complete

Status: Complete.

Verified the Phase 2 build-and-test workflow from a clean Git checkout:
- Cloned current commit into a temporary directory.
- Ran scripts/build_and_test.sh.
- Application build succeeded.
- Test build succeeded.
- Automated tests: 38 passed, 0 failed.
- No application source changes.


## Phase 2 Project Schema Versioning — Complete

Status: Complete.

Project JSON now includes a `schemaVersion` field.
- New projects default to schema version 2.
- Existing project files without `schemaVersion` load as version 1.
- `schemaVersion()` exposes the loaded/default version.

Automated tests: 40 passed, 0 failed.


## Phase 2 Project Schema Version Round Trip — Complete

Status: Complete.

Added an automated round-trip test verifying that a newly saved project preserves Project::CurrentSchemaVersion when loaded.

Automated tests: 41 passed, 0 failed.


## Phase 2 Future Schema Version Rejection — Complete

Status: Complete.

Project::load now rejects project files whose schemaVersion is newer than the current supported schema version.

Automated tests: 42 passed, 0 failed.


## Phase 2 Application Open Future Schema Fails Safely — Complete

Status: Complete.

Added an application-level test verifying that opening a project with a future unsupported schema version fails safely, preserves the current project, and emits an error through backgroundCompleted.

Automated tests: 43 passed, 0 failed.


## Phase 2 Clean-Checkout Re-Verification — Complete

Status: Complete.

Re-verified the Phase 2 build-and-test workflow from a clean Git checkout after schema, persistence, UI, and application integration changes:
- Cloned current commit into a temporary directory.
- Ran scripts/build_and_test.sh.
- Application build succeeded.
- Test build succeeded.
- Automated tests: 43 passed, 0 failed.
- No application source changes.


## Phase 2 Objective 2 — Minimal Viewer Presentation Surface — Complete

Status: Complete.

Implemented the first Phase 2 Objective 2 (Viewer Presentation Foundation) subtask — a minimal viewer presentation surface with a deterministic synthetic 360° test scene:

- `app/viewer/ViewerScene`: deterministic synthetic 360° test scene of 10 oriented markers (FRONT/BACK/LEFT/RIGHT/UP/DOWN and 4 diagonals) using ViewportState-compatible angle conventions. QtCore-only; no media, camera, or rendering logic.
- `app/ui/ViewerWidget`: minimal presentation surface rendering the scene as an identity equirectangular unwrap (background, 30° orientation grid, colored labeled markers). No camera state and no ViewportState connection yet.
- MainWindow now embeds the viewer surface (`viewerWidget` object name). No Application/core/project/schema changes.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 48 passed, 0 failed (43 regression + 5 new).
- New tests: deterministic scene ground truth, marker range validity, ViewerWidget scene presence, MainWindow viewer surface, deterministic render pixel checks.

Boundaries preserved:
- ViewportState→camera behavior not implemented (next Objective 2 subtask, not started).
- No real media, video decoding/playback, timeline, AI editing, export, audio, effects, reframing, virtual camera, or camera-specific logic introduced.
- No new dependencies introduced.


## Phase 2 Objective 3 — Viewer Camera Integration — Complete

Status: Complete.

Connected the application-owned ViewportState to the ViewerWidget presentation layer so viewport changes deterministically change the synthetic 360° scene's camera/view:

- `app/viewer/ViewerProjection`: stateless, deterministic camera/view transform mapping scene marker directions through camera yaw/pitch/roll/FOV (ViewportState-compatible semantics; positive roll rotates projected content counter-clockwise; markers behind or on the sideways plane are not visible). QtWidgets-independent for headless unit testing.
- `ViewerWidget` now renders the deterministic scene through the camera view read-only from the authoritative ViewportState supplied via `setViewportState()`; it owns no viewport state and never becomes a second source of yaw/pitch/roll/FOV. Identity defaults apply when no state is supplied.
- `Application` remains the single owner of viewport state; wiring added in `main.cpp` supplies the state to the viewer (existing signal/slot boundary drives repaints).
- `MainWindow` exposes `viewerWidget()` for this wiring.
- Existing synthetic `ViewerScene` preserved unchanged.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 55 passed, 0 failed (48 prior, of which one presentation test updated for the intended camera-driven change, plus 7 new).
- New tests: projection identity centering, yaw aiming at side markers, pitch aiming at poles, pitch mirror symmetry, roll content rotation about center, FOV scaling, and Application→ViewerWidget integration (adjust yaw +90 centers the RIGHT marker; reset restores FRONT).

Boundaries preserved:
- No real media, video decoding/playback, timeline, AI editing, export, audio, effects, reframing, virtual camera, or camera-specific logic introduced.
- No schema changes; no new dependencies.
- ViewerScene and all non-presentation behavior unchanged.


## Phase 2 Objective 4 — Real Media Foundation — Complete

Status: Complete.

Established the foundation for importing and representing real media files while preserving Reelcraft's non-destructive architecture:

- `app/core/MediaItem`: QtCore-only deterministic media record — stable id (SHA-256 of the canonical path), stored path, file name, size, last-modified UTC, extension-derived format tag, optional free-form attributes (future home for content/360° classification metadata). Validation accepts only existing, readable, regular files; the original file is never modified. JSON serialization/deserialization with strict validation.
- `Application`-owned media management: `mediaItems()`, headless `importMediaFile(path)` slot — requires an active project, validates, deduplicates the same canonical path idempotently, preserves import order, and reports through the existing `backgroundCompleted` signal boundary.
- `Project`: optional additive `media` JSON section (mirroring the `viewerState` pattern) written only when non-empty and read back on load; legacy/absent-media projects unaffected. No schema-version bump or migration.
- Save serializes the current media list into the project; open restores and revalidates it deterministically; invalid persisted entries fall back safely (empty media) without failing the open.
- `MainWindow`: Import Media button with injectable file chooser and `importMediaRequested` signal; wired to `Application::importMediaFile` in `main.cpp`.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 68 passed, 0 failed (55 prior + 13 new).
- New tests: media metadata recording, invalid-path rejection, id determinism, JSON round trip, invalid-JSON rejection, import requires active project, deduplication and ordering, invalid-import rejection, persistence/reopen determinism, original-media byte/SHA-256 invariance, legacy project with empty media, invalid persisted media fallback, and Import Media button signal.

Boundaries preserved:
- No decoding/playback, timeline, editing, AI, export, audio, effects, camera-specific logic, viewer/presentation changes, schema migration, or new dependencies.
- Media management is separate from the viewer/playback layers; 360° remains first-class conceptually (attributes placeholder) without camera-specific implementation.


## Phase 2 Objective 5 — Media Reference Availability and Open-Time Integrity — Complete

Status: Complete.

Completed the media-validation lifecycle from Objective 4:

- Project open now revalidates each restored media reference using the existing `MediaItem::referenceExists()` (point-in-time availability).
- `Application::restoreMediaFromJson()` normalizes deterministically: invalid records are dropped and duplicate media ids keep only the first occurrence, preserving order.
- New `Application` helpers: `hasUnavailableMedia()` and `unavailableMediaCount()`.
- `openProject()` emits exactly one deterministic `backgroundCompleted` status message only when a reopened project contains unavailable media.
- No `MediaItem`, `Project`, viewer/presentation, schema, or persistence-format changes; no new dependencies.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 73 passed, 0 failed (68 prior + 5 new).
- New tests: silent identical reopen when all media available; unavailable flag + single status message after deleting a referenced file; unavailable after moving a referenced file; duplicate persisted media entries normalized on open and re-save; new project clears unavailable state.

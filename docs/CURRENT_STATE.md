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

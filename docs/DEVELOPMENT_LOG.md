# Reelcraft — Development Log

## Purpose

This document records meaningful development activity chronologically.

It is an operational record of what was attempted, what changed, what was verified, and what resulted from each development session.

The development log must remain factual. Do not rewrite failed attempts as successful work.

---

# 2026-09-02

## Session: Project Foundation and Architecture Documentation

### Work Performed

- Established the Reelcraft project documentation foundation.
- Created the Master Guide defining the product vision, principles, workflow, roadmap, and development rules.
- Created the Current State document.
- Initialized the Git repository.
- Created the `main` branch.
- Created the initial project foundation checkpoint.
- Defined the technical architecture at the planning level.
- Established architectural decision documentation.
- Established AI handoff documentation.
- Established project history documentation.

### Verification

- Documentation files were created and inspected.
- Git repository was initialized successfully.
- Git branch was established as `main`.
- The initial documentation foundation was committed.
- Architecture and development workflow documentation was checkpointed.
- The project backup was verified using its SHA-256 checksum.

### Current Result

Reelcraft remains in the documentation and architecture foundation stage.

No application implementation has been intentionally introduced as part of this documentation objective.

### Next Work

Complete the remaining project-control documentation, verify consistency across all documentation, update project state, and create a final Git checkpoint before defining the first implementation objective.

---

# Development Log Rules

Future entries should include:

- Date
- Session or objective
- Work performed
- Verification performed
- Problems encountered
- Result
- Next work

Record failures and corrections honestly.

Do not delete historical entries merely because an approach was unsuccessful.

## 2026-09-02 — Product and AI Architecture Contracts Established

### Objective

Complete the conceptual product and AI architecture contracts needed before production implementation.

### Work Completed

- Established the product requirements contract in `REQUIREMENTS.md`.
- Established the conceptual project model in `PROJECT_MODEL.md`.
- Established the AI-to-deterministic editing contract in `AI_EDIT_CONTRACT.md`.
- Updated `NEXT_TASK.md` to make technology/runtime evaluation the next controlled objective.
- Updated `CURRENT_STATE.md` to reflect the transition toward validated technical direction.

### Architectural Position

The project continues to preserve the following boundaries:

- AI systems decide what should happen.
- Deterministic media systems execute validated instructions.
- AI output is treated as untrusted structured input until validated.
- Original media is never modified by AI editing operations.
- 360° media remains a first-class capability.
- Camera-specific behavior remains behind adapter boundaries.
- Voice and text use the same intent architecture.
- AI providers remain replaceable.
- Local, cloud, and hybrid processing remain architectural options.
- Final implementation technology choices remain open until evaluated.

### Important Deferral

A production application skeleton is intentionally deferred.

The documentation foundation identified several unresolved technology decisions, including the desktop runtime, UI approach, media-processing integration, AI integration, voice, 360° processing, storage, GPU strategy, testing, packaging, and deployment.

Beginning substantial implementation before evaluating these areas could create unnecessary architectural lock-in.

### Verification

- Documentation files created and reviewed.
- `NEXT_TASK.md` verified.
- `CURRENT_STATE.md` verified.
- Git repository remains under controlled incremental development.

### Result

The project is ready to begin a focused technology/runtime evaluation rather than prematurely committing to an implementation stack.

### Next Work

Evaluate one technical area at a time, document evidence and consequences, record material decisions, update architecture documentation when required, and establish the smallest safe Phase 1 implementation objective only after the evaluation is complete.

---

## 2026-09-02 — Technology and Runtime Evaluation Completed

### Objective

Evaluate the implementation-level technical direction required to safely begin Reelcraft production application development without prematurely locking the project into an unsuitable runtime, UI framework, media architecture, AI provider, rendering strategy, or deployment model.

### Evaluation Completed

The following areas were evaluated against the established Reelcraft requirements and architecture:

- Desktop-first application runtime approaches.
- Desktop UI and application architecture approaches.
- Deterministic media-processing integration.
- AI integration and provider abstraction.
- Voice interaction using the same intent architecture as text.
- First-class 360° video requirements.
- Local, cloud, and hybrid processing models.
- Performance, background processing, and GPU considerations.
- Project state, storage, portability, and recovery considerations.
- Rendering and export architecture.
- Automated testing requirements.
- Packaging and deployment considerations.
- Integration risks, portability risks, licensing considerations, and technology lock-in.

### Runtime and UI Direction

The primary desktop application approaches evaluated were:

- Qt 6
- Electron
- Tauri 2

Wails v3 was considered as a lower-priority/watchlist option because its v3 release remains less mature than the established alternatives.

No framework is recorded as a permanent dependency solely because it was evaluated.

The evaluation established that Reelcraft requires a desktop application rendering layer capable of:

- Reliable desktop UI rendering.
- High-resolution video presentation.
- Interactive 2D/3D rendering.
- GPU-accelerated rendering where available.
- Interactive 360° presentation.
- Integration between video frames and interactive visual content.
- Separation between UI/application work and deterministic media processing.

### Media and Rendering Direction

The deterministic media engine must remain separate from the UI and AI reasoning layers.

The application must support background processing so expensive media operations do not block interactive UI work.

Rendering and export should remain behind deterministic media-system boundaries rather than being directly controlled by AI-generated arbitrary commands.

The evaluation did not justify prematurely fixing a complete production media stack, codec strategy, GPU pipeline, or rendering implementation.

### AI Direction

The existing AI-to-deterministic boundary remains validated:

AI reasoning → Structured Edit Plan → Validation → Deterministic Media Execution

AI output remains untrusted until validated.

AI providers and models remain replaceable.

Voice interaction should produce the same underlying editing intent representation as text rather than becoming a separate editing architecture.

### 360° Direction

360° video remains a first-class media capability.

The application foundation must therefore be capable of supporting a future interactive 360° viewport and related GPU-capable rendering requirements without treating 360° as a later conversion layer.

The evaluation did not justify implementing the complete 360° pipeline during Phase 1.

### Processing Model

Local, cloud, and hybrid processing remain valid architectural options.

The evaluation did not justify permanently assigning every workload to local or cloud execution at this stage.

The architecture should preserve the ability to choose processing location per workload as implementation evidence becomes available.

### Performance and GPU Direction

GPU capability is an important architectural consideration for Reelcraft because the product must eventually handle high-resolution video, interactive 360° presentation, and complex visual rendering.

The current Android/Termux/Ubuntu development environment is not sufficient by itself to establish production desktop GPU performance characteristics.

Performance-sensitive implementation choices must therefore be validated through representative desktop testing rather than assumptions based on the development environment.

### Storage and Project Direction

Project state must remain separate from original media.

Original media must remain untouched by editing operations.

The project representation should remain portable and recoverable while allowing large media assets to remain separately managed.

A complete production project schema is intentionally deferred until the selected runtime and implementation architecture provide sufficient evidence.

### Testing Direction

Automated testing is a required part of the implementation workflow.

Phase 1 must establish enough testing infrastructure to verify:

- Application startup.
- Core/UI communication.
- Project persistence and reopening.
- Boundary/error behavior.
- Background processing behavior.
- Regression protection for established functionality.

### Technology-Agnostic Conclusions

The following conclusions are architectural requirements rather than commitments to a specific technology:

- Desktop-first is required.
- AI reasoning and deterministic media execution remain separate.
- AI output must be validated before execution.
- Editing remains non-destructive.
- Original media remains protected.
- 360° video is first-class.
- Camera-specific behavior uses adapters.
- AI providers remain replaceable.
- Voice and text share the same intent architecture.
- Local/cloud/hybrid processing remains possible.
- Performance-sensitive work must not block the interactive UI.
- GPU acceleration should be used where appropriate and available.
- Project state must remain portable and recoverable.
- Automated testing is required.
- Human review remains the final authority.
- Technology choices must remain evidence-based.

### Technology-Specific Evaluation Status

The evaluated desktop approaches remain candidates rather than permanent commitments.

The evaluation provides enough direction to begin a narrowly scoped application-foundation implementation, but it does not justify prematurely locking every downstream subsystem.

Technology selection for the Phase 1 application foundation must remain limited to what is required to establish the desktop shell, application/core boundary, project-state foundation, testing foundation, and reproducible development workflow.

### Important Constraints

The following remain intentionally unresolved until implementation evidence justifies them:

- Final production media-engine implementation.
- Complete GPU/video-frame pipeline.
- Final AI providers and models.
- Complete local/cloud workload allocation.
- Final production project schema.
- Full database/storage architecture.
- Complete 360° processing and reframing implementation.
- Final rendering/export implementation.
- Plugin architecture.
- Production packaging and deployment details.

### Result

The project has moved from:

    Documentation + conceptual architecture

to:

    Documentation + validated technical direction

The technology evaluation is complete enough to define the smallest safe Phase 1 implementation objective without beginning a major editor feature prematurely.

### Next Work

Define and implement the smallest Phase 1 application-foundation objective using the validated technical direction.

The next task must preserve the controlled workflow:

Inspect
  ↓
Define Objective
  ↓
Define Scope
  ↓
Define Definition of Done
  ↓
Implement Small Change
  ↓
Test
  ↓
Verify
  ↓
Document
  ↓
Git Checkpoint
  ↓
Define Next Objective

## 2026-09-03 — Phase 1 Desktop Application Foundation Verified

- Implemented Project state model in app/core/Project.cpp/h
- Implemented Application lifecycle and background demo
- Implemented MainWindow UI shell with new/save/open/background controls
- Updated reelcraft.pro to include Qt concurrent and Project files
- Added Qt Test suite in tests/
- Fixed ISODateWithMs precision and linker issues
- Verified app build, automated tests, and offscreen runtime launch

## 2026-09-03 — Phase 1 UI Integration Verification

- Added object names to MainWindow widgets
- Made file-path chooser injectable for headless UI tests
- Added TestMainWindow override to avoid modal file dialogs
- Added UI signal and label update tests
- Added Application save/open error-path tests
- Fixed test preconditions for invalid-path save
- Verified 14 automated tests pass offscreen

## 2026-09-03 — Reproducible Build-and-Test Workflow

- Added scripts/build_and_test.sh
- Script builds app and tests using /usr/lib/qt6/bin/qmake
- Script runs offscreen Qt Test suite
- Verified: 14 tests pass, WORKFLOW_EXIT=0
- Updated DEVELOPMENT_ENVIRONMENT.md testing capability

## 2026-09-03 — Original Media Safety Verification

- Added test fixture representing original media
- Added SHA-256 before/after project create/save/reopen checks
- Verified source bytes remain unchanged
- Full suite now: 15 passed, 0 failed

## 2026-09-03 — Platform Strategy Recorded

- Added Decision 014: Desktop first-class, Android planned future
- Documented platform boundary constraints
- Classified Android/Termux as development-only
- Confirmed no application source modifications
- Existing automated tests still pass


## 2026-09-03 — Apple Platform Portability Classification

- Verified QFileDialog/path selection remains isolated in MainWindow
- Verified Project and Application layers use QtCore only
- Extended platform strategy without creating a new decision
- Recorded macOS/iOS/iPadOS classification and portability constraints
- No application source files changed

## 2026-09-03 — Real Desktop Smoke Test Completed

- Established real graphical Qt X11 session using Termux:X11 and Ubuntu proot.
- Verified Qt XCB platform plugin path.
- Launched reelcraft against DISPLAY=:0.
- New Project: label updated with UUID.
- Save Project: saved /tmp/reelcraft-smoke.reel with correct JSON.
- Open Project: reopened same project and label matched.
- Run Background Demo: status updated, UI remained responsive.
- Regression: 15 tests pass, git status clean.
- No application source changes.

## 2026-09-03 — Clean-Checkout Workflow Verification

- Created temporary clean clone from current commit
- Ran existing build-and-test script
- Confirmed application and test builds succeed
- Confirmed 15 automated tests pass
- Original repository remained clean
- No source changes

## 2026-09-03 — Environment Documentation Alignment

- Corrected verified automated test count from 14 to 15.
- Corrected Qt Test suite count from 14 to 15.
- Documented manual desktop smoke via Termux:X11/XCB.
- No application source changes.

## 2026-09-03 — Phase 1 Formally Declared Complete

- Recorded formal Phase 1 completion in CURRENT_STATE.md.
- Recorded verified Definition-of-Done evidence.
- Set NEXT_TASK.md to Phase 2 planning state.
- No application source changes.

## 2026-09-03 — Phase 2 Viewer State Foundation

- Added app/viewer/ViewportState.h and .cpp
- Added yaw/pitch/roll/FOV model with clamping and normalization
- Added signal emission only on actual value changes
- Added viewer-state tests to existing test suite
- Full suite: 21 passed, 0 failed
- No Phase 1 implementation changes

## 2026-09-03 — Application-Level Viewport State Integration

- Application now owns a ViewportState instance.
- Added viewportState accessor.
- Added resetViewport slot.
- Added tests for application-level viewer state and reset behavior.
- Full suite: 23 passed, 0 failed.

## 2026-09-03 — UI Viewer Readout

- Added yaw/pitch/roll/FOV readout labels to MainWindow.
- Added Reset Viewport button and resetViewportRequested signal.
- Wired ViewerState signals in main.cpp.
- Added UI tests for viewer labels and reset button.
- Full suite: 25 passed, 0 failed.

## 2026-09-03 — Viewer State Integration Tests

- Added end-to-end viewer state integration tests.
- Verified Application ViewportState updates MainWindow labels.
- Verified Reset Viewport updates UI labels through signal wiring.
- Full suite: 27 passed, 0 failed.

## 2026-09-03 — Keyboard Viewer Controls

- Added MainWindow keyPressEvent handling.
- Added viewport delta signals for yaw/pitch/roll/FOV.
- Wired keyboard deltas to Application-owned ViewportState in main.cpp.
- Added tests for keyboard signal emission and state updates.
- Full suite: 29 passed, 0 failed.

## 2026-09-03 — New Project Resets Viewer State

- Application::newProject now calls resetViewport.
- Added test verifying viewer state resets on new project.
- Full suite: 30 passed, 0 failed.

## 2026-09-04 — Open Project Resets Viewer State

- Application::openProject now calls resetViewport.
- Added test verifying viewer state resets on open.
- Full suite: 31 passed, 0 failed.


## 2026-09-04 — Viewer State JSON Serialization

- Added toJsonObject and readFromJsonObject.
- Added round-trip and invalid-JSON tests.
- Full suite: 33 passed, 0 failed.


## 2026-09-04 — Viewer State Persistence

- Added viewerState object to Project model.
- Save stores application viewer state.
- Open restores viewer state if present.
- Added save/open persistence integration test.
- Full suite: 34 passed, 0 failed.


## 2026-09-04 — Restored Viewer State UI Integration Test

- Added test for restored viewer state updating MainWindow labels.
- Full suite: 35 passed, 0 failed.


## 2026-09-04 — Focused Button Keyboard Input

- Added MainWindow::eventFilter.
- Installed event filter on viewer-related buttons.
- Added focused-button keyboard delta test.
- Full suite: 36 passed, 0 failed.


## 2026-09-04 — Invalid Viewer State Fallback Verification

- Added invalid persisted viewer state fallback test.
- Verified project opens and resets viewer state safely.
- Full suite: 37 passed, 0 failed.


## 2026-09-04 — Application-Level Viewer Adjust Slots

- Added Application viewer adjustment slots.
- Replaced main.cpp UI lambdas with application-slot connections.
- Added application adjustment slot test.
- Full suite: 38 passed, 0 failed.


## 2026-09-04 — Phase 2 Clean-Checkout Verification

- Created temporary clean clone at current HEAD.
- Ran existing build-and-test script.
- Confirmed app/test builds and 38 passing tests.
- Original repository remained clean.
- No source changes.


## 2026-09-04 — Project Schema Versioning

- Added schemaVersion field to Project persistence.
- New projects default to current schema version.
- Legacy projects without version load as schema 1.
- Added default and legacy-version tests.
- Full suite: 40 passed, 0 failed.


## 2026-09-04 — Project Schema Version Round Trip

- Added project schema version round-trip test.
- Verified new projects preserve current schema version after save/load.
- Full suite: 41 passed, 0 failed.


## 2026-09-04 — Future Schema Version Rejection

- Project::load now rejects newer unsupported schema versions.
- Added future schema rejection test.
- Full suite: 42 passed, 0 failed.

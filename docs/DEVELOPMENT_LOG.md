# Reelcraft — Development Log

## Purpose and update rule

This is the **authoritative chronological engineering record**: one entry per objective, written
once, append-only (never rewrite history). Other documents must not restate it — `CURRENT_STATE.md`
records current system state, `NEXT_TASK.md` the capability register and validation debt,
`DECISIONS.md` the architectural decisions, `CHANGELOG.md` user-facing changes, and
`PROJECT_HISTORY.md` the milestone-level narrative. See `AGENT_WORKFLOW.md` §10.

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


## 2026-09-04 — Application Open Future Schema Fails Safely

- Added application-level future schema failure test.
- Verified current project is preserved.
- Verified backgroundCompleted error is emitted.
- Full suite: 43 passed, 0 failed.


## 2026-09-04 — Phase 2 Clean-Checkout Re-Verification

- Created temporary clean clone at current HEAD.
- Ran existing build-and-test script.
- Confirmed app/test builds and 43 passing tests.
- Original repository remained clean.

## 2026-09-04 — Phase 2 Objective 2: Minimal Viewer Presentation Surface

### Objective

Implement the first Objective 2 subtask: a minimal viewer presentation surface, a deterministic synthetic 360° test scene, and the minimum UI wiring to display it. ViewportState→camera integration was explicitly out of scope.

### Work Completed

- Added `app/viewer/ViewerScene.{h,cpp}` — deterministic synthetic 360° test scene of 10 oriented markers (FRONT/BACK/LEFT/RIGHT/UP/DOWN + 4 diagonals) using ViewportState-compatible angle conventions (yaw normalized to [-180,180), pitch clamped to [-90,90]). QtCore-only; no media or camera logic.
- Added `app/ui/ViewerWidget.{h,cpp}` — minimal presentation surface that renders the scene as an identity equirectangular unwrap (background, 30° orientation grid, colored markers with labels). No camera state, no ViewportState connection, no media.
- Wired the viewer surface into MainWindow (`viewerWidget` object name) with no Application/core changes.
- Extended `reelcraft.pro` and `tests/tests.pro` with the new sources/headers.

### Verification

- Application build succeeded (offscreen launch smoke: event loop alive until timeout, SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 48 passed, 0 failed (43 regression + 5 new viewer tests).
- New tests cover: deterministic scene ground truth, marker range validity, ViewerWidget scene presence, MainWindow viewer surface, and deterministic render pixel checks (background, FRONT/LEFT/RIGHT marker colors).

### Boundary Notes

- No ViewportState→camera behavior, media, playback, timeline, AI, export, audio, effects, reframing, or camera-specific logic was introduced.
- No new dependencies were added (QtWidgets/QtCore only, already in use).

## 2026-09-04 — Phase 2 Objective 3: Viewer Camera Integration

### Objective

Connect the application-owned ViewportState to the ViewerWidget presentation layer so viewport changes deterministically change the synthetic 360° scene's camera/view, using the formal Objective 3 definition previously recorded in NEXT_TASK.md.

### Work Completed

- Added `app/viewer/ViewerProjection.{h,cpp}` — stateless deterministic camera/view transform (marker direction through camera yaw/pitch/roll + vertical FOV). Conventions documented: marker equal to camera direction centers; positive roll rotates projected content counter-clockwise; markers behind or effectively on the 90° sideways plane are not visible (fp guard); QtWidgets-independent.
- `ViewerWidget` now presents the scene through the camera, reading yaw/pitch/roll/FOV read-only from the authoritative ViewportState supplied via `setViewportState()` (identity defaults when none). The widget owns no viewport state.
- Added `MainWindow::viewerWidget()` accessor; `main.cpp` supplies the application-owned viewport state to the viewer (existing signal/slot boundary drives repaints).
- Extended `reelcraft.pro` and `tests/tests.pro` with ViewerProjection.
- `ViewerScene` preserved unchanged; no schema or dependency changes.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 55 passed, 0 failed. Regression: all prior tests green; `viewerWidgetRenderIsDeterministic` updated to the intended camera-driven presentation (identity defaults center FRONT; former equirect side-marker positions are now plain background).
- New tests: projection identity centering; yaw aims at side markers; pitch aims at poles; pitch mirror symmetry; roll rotates content about center; larger FOV brings markers closer; Application→ViewerWidget integration (yaw +90 centers RIGHT marker, reset restores FRONT).

### Boundary Notes

- No media, decoding/playback, timeline, AI, export, audio, effects, reframing, virtual camera, camera-specific logic, schema changes, or new dependencies introduced.

## 2026-09-04 — Phase 2 Objective 4: Real Media Foundation

### Objective

Implement the formal Objective 4 definition (NEXT_TASK.md): import/reference real media files, validate references, record minimal factual metadata, and persist/reopen them non-destructively — with no decoding, playback, or viewer changes.

### Work Completed

- Added `app/core/MediaItem.{h,cpp}` — QtCore-only deterministic media record (stable id = SHA-256 of canonical path; path, file name, size bytes, last-modified UTC, extension-derived format tag; optional attributes placeholder). Creation validates existing/readable/regular files; JSON round trip with strict validation; original media never modified.
- `Application`: `mediaItems()` accessor and `importMediaFile(path)` slot (active-project requirement, dedupe of canonical path, deterministic import order, errors via `backgroundCompleted`). `newProject` clears media; `saveProject` serializes the media list into the project; `openProject` restores/revalidates it.
- `Project`: optional additive `media` JSON section (array) written only when non-empty; load reads only arrays; legacy and no-media projects unaffected; no schema-version bump or migration.
- `MainWindow`: Import Media button (`importMediaButton`) with injectable `chooseMediaFilePath()`; `importMediaRequested` signal; wired to the application slot in `main.cpp`.
- Extended `reelcraft.pro` and `tests/tests.pro` with MediaItem.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 68 passed, 0 failed (55 prior + 13 new, all green on the full run).
- New tests cover: metadata recording, invalid-path rejection, id determinism, JSON round trip/invalid JSON, import requires a project, deduplication + ordering, invalid import rejection, persistence/reopen determinism, original-media byte/SHA-256 invariance, legacy no-media project, invalid persisted media fallback, and Import Media button signal.

### Boundary Notes

- No decoding, playback, timeline, editing, AI, export, audio, effects, camera-specific logic, viewer/presentation changes, schema migration, or new dependencies.
- 360° remains conceptually first-class via the attributes placeholder; no camera-specific implementation.

## 2026-09-04 — Phase 2 Objective 5: Media Reference Availability and Open-Time Integrity

### Objective

Complete the media-validation lifecycle: revalidate media references on project open, normalize the restored list deterministically, and surface unavailable references — no decode, viewer, schema, or persistence-format changes.

### Work Completed

- `Application::openProject()` now emits exactly one deterministic `backgroundCompleted` status message when the reopened project contains unavailable media (`Media references unavailable: N of M.`).
- `Application::restoreMediaFromJson()` normalizes deterministically: invalid records dropped; duplicate media ids keep only the first occurrence, preserving order.
- Added `Application::hasUnavailableMedia()` and `unavailableMediaCount()` (point-in-time filesystem availability via the existing `MediaItem::referenceExists()`).
- No changes to `MediaItem`, `Project`, the viewer layer, `.pro` files, or the schema.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 73 passed, 0 failed (68 prior + 5 new).
- New tests: silent identical reopen when media available; unavailable flag + single status message after deleting a referenced file; unavailable after moving a referenced file; duplicate persisted media entries normalized on open and re-save; new project clears unavailable state.

## 2026-09-05 — Phase 2 Objective 6: Project Media Library Management (List & Remove)

### Objective

Complete the media-management slice: let the user see the media referenced by the current project and remove records — no decode, viewer, schema, or dependency changes.

### Work Completed

- `Application::removeMedia(mediaId)`: id-based removal of exactly the matching record (active-project requirement; deterministic messages: `Removed media: <name>`, `Remove failed: media not found.`, `No project to remove media from.`); only the record is removed, never the file.
- `Application::mediaListChanged(const QList<MediaItem> &)`: structured notification emitted after a successful appending import, a successful removal, a project open, and a new project (empty); duplicate imports and failed removals do not emit.
- `MainWindow`: media list widget (`mediaListWidget`; `fileName  [formatTag]`, id in `Qt::UserRole`) and Remove Media action (`removeMediaButton`); Remove with no selection shows `No media selected to remove.`; `showMediaList()` repopulates from the authoritative list.
- `main.cpp`: wires `mediaListChanged` → `showMediaList` and `removeMediaRequested` → `removeMedia`.
- `Q_DECLARE_METATYPE(MediaItem)` in `MediaItem.h`; meta-type registration in tests for the custom-type signal.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 81 passed, 0 failed (73 prior + 8 new).
- New tests: list emission on import/open/new/remove; removal correctness and persistence; unknown-id and no-project failure paths; removal of an unavailable entry clears unavailable state; MainWindow list population and Remove wiring (with and without selection).

### Boundary Notes

- No decoding, playback, timeline, editing, AI, export, audio, effects, camera-specific logic, active-source/viewer binding, schema migration, or new dependencies.
- Media records are metadata only; removal never touches underlying files (Decision 002).

## 2026-09-05 — Phase 2 Objective 7: Active (Selected) Media — "Viewer Source" Contract

### Objective

Define the minimal deterministic contract for which imported media record is active/selected for viewing, keeping the viewer unchanged.

### Work Completed

- `Application`: owns `m_activeMediaId`; accessors `activeMediaId()` and `activeMediaItem()` (resolves to exactly one current record or nullptr).
- `Application::setActiveMedia(mediaId)`: requires an active project (`No project to select media in.`), an existing id (`Select failed: media not found.`), and an available file (`Select failed: media is unavailable.`); idempotent when already active (`Media already active: <name>`); success emits `Active media: <name>` and `activeMediaChanged`.
- `Application::activeMediaChanged(const QString &)` emitted only on actual change (empty = cleared).
- Consistency rules: import never auto-selects; removing the active record clears it with one emission; removing others preserves it; `newProject` clears it; `openProject` normalizes the media list first and restores the persisted id only when it resolves to a current, available record (dangling/invalid/unavailable → cleared deterministically).
- Persistence: `Project` carries an optional additive `activeMediaId` (written only when non-empty, read leniently); no schema-version bump, no migration.
- `MainWindow`: `setActiveButton` acting on the list current row, `activeMediaLabel` (`Active media: <name>`/`None`), `▶` marking of the active row, `showActiveMedia`, `setActiveRequested`; wired in `main.cpp`.
- `Project.h/.cpp`, `Application.h/.cpp`, `MainWindow.h/.cpp`, `main.cpp`, `tests/test_project.cpp`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 90 passed, 0 failed (81 prior + 9 new, all green on the first full run).
- New tests: set/clear semantics and idempotency; import never auto-selects; removal rules; new-project clearing; save→reopen round trip; open-time normalization (dangling/legacy/duplicate persisted ids); unavailable media cannot become active; MainWindow Set Active/label/marking.

### Boundary Notes

- No decoding, playback, timeline, editing, AI, export, audio, effects, camera-specific logic, viewer/presentation changes, schema migration, or new dependencies.
- Original media files never touched; active selection requires an available reference at selection/restore time (point-in-time snapshot; no watchers).

## 2026-09-05 — Phase 2 Objective 8: Equirectangular Frame Presentation Foundation

### Objective

Establish the smallest safe, deterministic, decode-free pixel presentation path: render an equirectangular image through the existing authoritative ViewportState camera, consistent with the verified ViewerProjection conventions. No real decoding/playback.

### Work Completed

- Added `app/viewer/EquirectView.{h,cpp}` — deterministic CPU equirectangular→camera renderer. Uses exactly the ViewportState/ViewerProjection conventions (identity → FRONT; +90° yaw → RIGHT; −90° → LEFT; ±90° pitch → UP/DOWN; positive roll rotates content counter-clockwise; vertical FOV [20,140]); camera basis built identically to ViewerProjection; nearest-neighbor equirect sampling; output Format_ARGB32.
- Deterministic input rejection: null/empty source, non-positive output dimensions, non-finite camera values, |pitch|>90, FOV outside [20,140]; failures never partially mutate `*outImage`; `MaxOutputWidth = 640` documented as an implementation/performance safeguard (not an architectural limit).
- `ViewerWidget` optional source-image path: `setSourceImage(QImage)` / `hasSourceImage()`; paintEvent renders a valid source through the current ViewportState camera (aspect-preserving capped resolution); when no/cleared source exists the existing synthetic marker-scene path is unchanged (protected regression contract). Widget remains presentation-only with no camera-state ownership.
- Registered EquirectView in `reelcraft.pro` and `tests/tests.pro`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 100 passed, 0 failed (90 prior + 10 new, all green on the first full run).
- New tests: identity/yaw/pitch anchors; positive-roll direction verified against the ViewerProjection convention via quadrant analysis; FOV coverage change; invalid-input rejection incl. no-partial-mutation sentinel; pixel-for-pixel determinism; ViewerWidget source-image integration; clearing source restores marker-scene rendering; CPU performance sanity.
- Performance sanity: EquirectView CPU render ≈ 14.11 ms/frame at 640×320 (recorded from the suite; generous bound asserted).

### Boundary Notes

- No decoding/probing/playback/streaming/audio, FFmpeg/GStreamer/QtMultimedia/OpenCV, active-media-to-viewer binding, timeline/editing/AI/export/effects, reframing/keyframes, camera-specific logic, schema migration, or new dependencies.
- Marker-scene rendering is a protected regression contract; the source-image path is opt-in.
- Resolution cap is a documented safeguard; later objectives may raise it or add optimized/GPU rendering without redesigning the camera contract.

## 2026-09-05 — Phase 2 Objective 9: Active-Media Frame Presentation (FFmpeg-CLI decode)

### Objective

Close the final Phase-2 gap: decode a single real frame from the active media record and present it through the verified equirectangular pixel path. Approved decode approach: FFmpeg-CLI adapter (Option A; Decision 015).

### Work Completed

- Added `app/media/FrameExtractor.{h,cpp}` — isolated QtCore seam invoking the external `ffmpeg` executable via QProcess (never linked) to decode one frame (`-v error -nostdin -i <file> -frames:v 1 -an -f image2 -c:v png pipe:1`), parsed in-process to a QImage. Executable resolved via `REELCRAFT_FFMPEG` override then `QStandardPaths::findExecutable`; deterministic errors (unavailable executable, missing/invalid file, start/timeout/exit failure, empty output, undecodable frame); no partial output mutation; media file never modified.
- `Application::previewActiveMediaFrame()`: requires an active project, an active media record, and an available file (`No project to preview media in.`, `No active media to preview.`, unavailable/ffmpeg-missing/decoder-error paths); success emits `framePreviewReady(QImage)` and `Frame extracted: <name>`.
- `MainWindow`: Preview Active Frame button (`previewFrameButton`) → `previewFrameRequested`; `showFramePreview(QImage)` → `viewerWidget()->setSourceImage` (viewer pixel path unchanged).
- `main.cpp`: wires `previewFrameRequested` → `previewActiveMediaFrame` and `framePreviewReady` → `showFramePreview`.
- Registered FrameExtractor in `reelcraft.pro`/`tests.tests.pro`. Recorded Decision 015 in `DECISIONS.md`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 107 passed, 0 failed (100 prior + 7 new; 0 skipped — ffmpeg available in this environment).
- New tests: FrameExtractor invalid-input rejection (no ffmpeg needed); availability + single-frame decode of a real PNG media file (exact dimensions/colors); deterministic repeatability; Application guards (no project / no active media); preview emits the correct frame content; Preview button signal; end-to-end decode → viewer center assertion (identity camera centers FRONT in the decoded equirect test pattern). Decode-dependent tests QSKIP cleanly when ffmpeg is unavailable.
- Diagnostic during development confirmed ffmpeg decode is lossless for the deterministic PNG pattern (decoded center = #ffd54f); the only test failure was a fixed-pixel sampling assumption inside an in-layout widget, corrected to dynamic center sampling.

### Boundary Notes

- FrameExtractor is the decode seam; replaceable by a future media engine without caller/viewer changes.
- External ffmpeg CLI reliance is the approved trade-off (Decision 015); injectable path + feature detection.
- Decode is synchronous single-frame (foundation only); playback/streaming require a separate objective.
- No schema, compiled-dependency, media-contract, or viewer changes; no Objective 10 work.

## 2026-09-06 — Phase 2 Objective 10: Active-Media Time Navigation

### Objective

Add on-demand single-frame time navigation (stepping/seek) through the active media record via the approved FFmpeg-CLI Option A seam — no continuous playback.

### Work Completed

- `FrameExtractor::extractFrameAt(filePath, execPath, seconds, out, error)`: fast input seek (`-ss <seconds> -i <file> … -frames:v 1 … pipe:1` PNG); `extractFirstFrame` now delegates with 0.0 s (Obj 9 contract preserved); negative/non-finite times rejected deterministically (`invalid seek time`). Position is a requested seek/preview, not an exact frame/timestamp guarantee.
- `Application`: session-only `m_previewTimeSeconds`; `previewTimeSeconds()`; `previewActiveMediaFrameAt` (negative→0 clamp); `stepActiveMediaPreview(delta)` (floor 0); private `decodePreviewFrameAt` centralizing the Obj 9 guards; position updates only on decode success; `previewTimeChanged(double)` emitted only on actual change; resets (with emission) on new project, project open, active-media change, and removal of the active media; beyond-end/undecodable requests fail deterministically (`Preview failed: …`) leaving position unchanged.
- `MainWindow`: Step −1 s / +1 s buttons (`stepBackButton`/`stepForwardButton`) → `previewStepRequested`; `previewTimeLabel` + `showPreviewTime`; Preview Active Frame decodes at the current position.
- `main.cpp`: wires `previewStepRequested` → `stepActiveMediaPreview` and `previewTimeChanged` → `showPreviewTime`.
- Files: `app/media/FrameExtractor.{h,cpp}`, `app/application/Application.{h,cpp}`, `app/ui/MainWindow.{h,cpp}`, `app/main.cpp`, `tests/test_project.cpp`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 116 passed, 0 failed (107 prior + 9 new; 0 skipped — ffmpeg available; suite ~17 s including deterministic generated video fixtures).
- New tests: invalid seek-time rejection; frame-at-time extraction (0 s red-dominant vs 2 s blue-dominant on an all-keyframe 1 fps fixture); seek determinism; Application guards; position update with single time emission; same-position re-request emits no time change; step advance and below-zero clamp; beyond-end failure deterministic with position unchanged; position resets (new project / active change / active removal); step buttons and time readout.

### Boundary Notes

- Preview position is session state only — intentionally not persisted; no schema surface.
- Fast input seek is approximate (keyframe semantics); documented; fixture uses `-g 1` for deterministic tests.
- One-shot subprocess per request; no timers/threads/loops; continuous playback/audio/streaming remain out of scope (future decision).
- No ffprobe/duration probing (deferred by approval); Obj 1–9 contracts preserved.

## 2026-09-06 — Phase 2 Objective 11: Pointer-Based Viewer Orientation Control

### Objective

Complete Phase-2 orientation control with the primary 360-viewer pointer gestures (drag-to-look, wheel FOV), routed through the existing authoritative ViewportState and Application adjust slots; the viewer remains presentation-only.

### Work Completed

- `ViewerWidget` pointer handling: left-button drag emits `viewportYawDeltaRequested` (drag right = yaw increases) and `viewportPitchDeltaRequested` (drag up = pitch increases) at a deterministic 0.25 °/px; mouse wheel emits `viewportFovDeltaRequested` (wheel up = −5° per step = zoom in; wheel down = +5° = zoom out); non-left drags ignored; release-without-move emits nothing; no modifiers/inertia/multi-touch.
- New `ViewerWidget` delta signals wired in `main.cpp` to the existing Application adjust slots (same receivers as the keyboard controls).
- Sensitivity constants (`kLookDegreesPerPixel = 0.25`, `kFovDegreesPerWheelStep = 5.0`) documented as deterministic defaults.
- Files: `app/ui/ViewerWidget.{h,cpp}`, `app/main.cpp`, `tests/test_project.cpp`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 120 passed, 0 failed (116 prior + 4 new; 0 skipped).
- New tests: drag emits expected yaw/pitch deltas; non-left drags and release-without-move emit nothing; wheel-up decreases FOV / wheel-down increases FOV; wired drags update Application ViewportState deterministically (incl. pitch clamp at 90 and FOV change).

### Boundary Notes

- The viewer never owns or mutates viewport state; it only emits delta requests (Application remains the single state owner).
- No rendering, media/decode, time-navigation, schema, persistence, or dependency changes; no Objective 12 work.

## 2026-09-06 — Phase 2 Objective 12: Projection Declaration & Flat-Media Preview Path

### Objective

Make source projection an explicit, creator-declared per-media property and add a correct undistorted flat preview — leaving the equirectangular path unchanged and routing unknown projection to it (status quo).

### Work Completed

- `MediaItem::Projection { Unknown, Equirectangular, Flat }` + `projectionToString/FromString`; optional additive `projection` JSON key serialized only when declared; absent/unrecognized values read as Unknown; `isValid`/dedupe/normalization unchanged (no schemaVersion change/migration).
- `Application::declareMediaProjection(mediaId, projectionValue)`: guards (no project / media not found / unknown projection value); updates the record; re-emits `mediaListChanged`.
- `ViewerWidget::setFlatSourceMode(bool)` (default false) + `drawFlatSourceImage`: aspect-preserving fit, centered, letterboxed on the existing background, no camera/equirectangular transform; equirect and marker-scene paths unchanged.
- `MainWindow`: projection stored per media row (role data); `showFramePreview` routes by the active row's projection (flat ⇒ flat mode; otherwise equirectangular); Mark Flat / Mark Equirect controls on the selected row; `setMediaProjectionRequested` signal; wired in `main.cpp`.
- Files: `app/core/MediaItem.{h,cpp}`, `app/application/Application.{h,cpp}`, `app/ui/ViewerWidget.{h,cpp}`, `app/ui/MainWindow.{h,cpp}`, `app/main.cpp`, `tests/test_project.cpp`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 128 passed, 0 failed (120 prior + 8 new; 0 skipped).
- New tests: projection default/parse semantics; JSON round trip incl. unknown fallback; Application declaration guards/validation/emission; declared projection persists on reopen; flat mode letterbox/center; flat mode ignores camera transforms; projection buttons emit requests; preview routing honors declared projection (flat vs equirect center anchors).

### Boundary Notes

- Unknown projection intentionally routes to equirectangular (backward-compatible status quo).
- Additive optional key consistent with `viewerState`/`media`/`activeMediaId` precedents — not a schema change.
- Flat mode is opt-in (default off); equirect/marker tests unchanged; no ffprobe detection; no pan/zoom/reframing; no Objective 13 work.

## 2026-09-06 — Phase 2 Objective 13: Viewer Presentation State Consistency

### Objective

Make the viewer's on-screen content and presentation mode deterministically consistent with Application state: clear stale decoded frames and flat mode when the project or active-media context changes.

### Work Completed

- `MainWindow::showProject` now clears the viewer source + flat mode (projectChanged: new/open).
- `MainWindow::showActiveMedia` clears when the active id changes (including cleared to none); same-id re-announcement is a no-op.
- New private `MainWindow::clearViewerSource()` — `viewerWidget()->setSourceImage(QImage())` + `setFlatSourceMode(false)` (returns to the deterministic marker scene).
- Files: `app/ui/MainWindow.{h,cpp}`, `tests/test_project.cpp`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 131 passed, 0 failed (128 prior + 3 new; 0 skipped).
- New tests: new project after a preview clears the frame and flat mode (marker scene visible at identity); active-media switch and active-media removal clear the frame; same-active re-announcement preserves the presented frame.

### Boundary Notes

- Clearing happens only on context-change signals; preview/step/save never clear.
- Preview remains an explicit user action (no auto-present on active change).
- No Application/media/decode/time-navigation/schema/render changes; no Objective 14 work.

## 2026-09-06 — Phase 2 Objective 14: Presentation Quality — Bilinear Equirectangular Rendering

### Objective

Replace nearest-neighbor equirectangular sampling with deterministic bilinear interpolation to improve single-frame review quality; preserve every rendering/camera/validation contract.

### Work Completed

- `EquirectView::render` sampling replaced with deterministic bilinear interpolation: four-texel straight-space (unpremultiplied) channel blend with alpha, horizontal seam wrap ((x0+1) mod width), vertical pole clamp, integer-rounded output. Edge handling for u/v at exactly 1.0 added.
- Camera conventions (ViewportState/ViewerProjection), roll/FOV math, input validation, and no-partial-output semantics unchanged; output Format_ARGB32 unchanged; `MaxOutputWidth = 640` unchanged and non-configurable.
- No changes to flat mode, ViewerWidget architecture, FrameExtractor, media/time/audio, schema, or dependencies.
- Files: `app/viewer/EquirectView.cpp`, `tests/test_project.cpp`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 133 passed, 0 failed (131 prior + 2 new; 0 skipped). All existing EquirectView/viewer/presentation tests pass unmodified.
- New tests: bilinear quarter-blend anchor at texel (1.5,1.5) on a 4×4 source (proves interpolation, not nearest); seam (±180°) and pole (±90°) robustness and determinism.
- Performance: bilinear 36.98 ms/frame at 640×320 (nearest was ~15–23 ms); informational 148.69 ms at 1280×640 (quantifies a future cap-raise decision).

### Boundary Notes

- 640px safeguard remains unchanged; cap configurability explicitly deferred.
- Interaction-latency note: per-paint render cost rose to ~37 ms; drag/wheel re-render per event — render caching/invalidation or a GPU path are follow-up candidates, not this objective.
- Uniform-patch interiors are identical under bilinear, so color-anchor/determinism tests were preserved without modification.

## 2026-09-06 — Phase 2 Objective 15: Real-Media Review Path Validation

### Objective

Validate the full Phase-2 review workflow end-to-end against deterministic FFmpeg-generated equirectangular multi-frame media — with no physical camera hardware.

### Work Completed

- Added test fixture generator `createEquirectReviewVideo` (+`buildReviewFrame`): 30 s, 1 fps, all-keyframe, 2:1 equirect video (360×180) with deterministic per-frame FRONT content (red/green/blue cycle) and a fixed RIGHT marker (magenta); reuses the FFmpeg-CLI fixture pattern (Decision 015); QSKIP when ffmpeg unavailable.
- Added three focused tests: fixture frame distinctness/seekability (t0 red / t1 green / t5 blue with dominance checks); end-to-end review path on the clip (preview t0 red → step +1 green → +90° yaw look-around centers magenta → reset → seek t5 blue; position tracking; viewer state via wiring); informational performance (no gate).
- Approved dev-script change: `scripts/build_and_test.sh` test-runner timeout raised 20 s → 240 s (Objective-15 FFmpeg fixture suites exceed the old cap; confirmed via a DECISION REQUIRED stop).
- Files: `tests/test_project.cpp`, `scripts/build_and_test.sh`.

### Verification

- Application build succeeded; offscreen launch smoke event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 136 passed, 0 failed (133 prior + 3 new; 0 skipped; suite ~26 s).
- Informational performance: single-frame decode ~633 ms/step on the equirect clip; per-paint render ~40.7 ms at 640×320 (no timing gate).

### Boundary Notes

- Pure test/validation layer; no production source, dependency, schema, or architecture change.
- Environment limitation recorded: synthetic-but-real encoded media is the review proxy here; hardware 360-camera validation is out of scope.
- Measured subprocess decode (~633 ms/step) recorded as interaction-latency data for future caching/engine decisions; no Objective 16 work.

## 2026-09-06 — Phase 2 Formally Declared Complete

- Read-only Phase 2 completion audit concluded PHASE 2 COMPLETE (Objectives 1-15, 136/0/0, build/smoke green, Decision 016 scope satisfied, no genuine blockers).
- Formal closeout performed: documentation-only updates to CURRENT_STATE (formal completion record), PROJECT_HISTORY (Phase 2 milestone), NEXT_TASK (closed state + Phase 3 direction), CHANGELOG (v0.2.33).
- Clean-checkout verification of the Phase 2 closeout commit: fresh clone -> scripts/build_and_test.sh -> application and test builds succeed, Totals: 136 passed, 0 failed, 0 skipped; offscreen smoke of the built application SMOKE_EXIT=124.
- No production source, test, script, schema, dependency, or architecture changes. Phase 3 not started.

## 2026-09-06 — Phase 3 Opening Decisions Recorded (Objective 1 — documentation)

- Read-only Phase 3 opening feasibility discovery completed and approved (verified environment facts: Qt 6.10.2 proot, QtMultimedia absent, Termux FFmpeg 8.1.2 with dev tree only outside the proot ABI; persistent-subprocess path feasible today, linked FFmpeg/QtMultimedia require installs).
- Recorded Decision 017 (documentation only): Phase 3 opening architecture = persistent FFmpeg streaming subprocess behind a replaceable media/player seam; FrameExtractor remains the single-frame preview/validation seam; linked FFmpeg and QtMultimedia deferred; ffprobe duration metadata reopened as a future decision gate; Objective 10 preview-time contract preserved (playback extends it); architecture boundary decode seam → player/timing → Application → Viewer.
- Synchronized CURRENT_STATE (Phase 3 OPEN record), PROJECT_HISTORY (Phase 3 opening milestone), NEXT_TASK (Phase 3 open; next = feasibility/contract objective), CHANGELOG (v0.2.34).
- Verification: focused documentation consistency checks (anchors/headings/decision text present; docs-only diff); Phase 2 remains formally closed; no implementation, dependency, or schema changes; one docs-only Git checkpoint.

## 2026-09-06 — Phase 3 Objective 2: Persistent FFmpeg Streaming Feasibility Probe (tests only)

- Added temporary test-only probe helpers + 3 tests in `tests/test_project.cpp` (clearly marked Phase 3 Objective 2 feasibility; no production code): persistent rawvideo stream read of a 25-frame @5 fps deterministic equirect clip with bounded waits everywhere; EOF normal-exit; missing-file deterministic error; terminate/kill then clean restart.
- Informational measurement (no gate): 25/25 frames delivered at ~39.9 frames/s (unthrottled rawvideo, 160x80), ~1.0 ms inter-frame delivery latency. Rawvideo/transport/pacing details intentionally NOT decisions (Decision 017).
- Verification: probe tests pass; full regression 139 passed / 0 failed (136 prior + 3 new; 0 skipped); build success; offscreen smoke SMOKE_EXIT=124.
- No dependency, ffprobe, schema, Obj-10 contract, FrameExtractor, or production changes; docs updated (NEXT_TASK/CURRENT_STATE/CHANGELOG v0.2.35); one Git checkpoint.

## 2026-09-16 — Phase 3 Objective 3: Replaceable Media-Source Seam & Deterministic Frame Pump

### Objective

Introduce the replaceable decode/media-source seam and a deterministic frame pump over the persistent FFmpeg-subprocess stream, reusing the Objective 2 feasibility evidence, without changing the Objective 10 preview-time contract.

### Work Completed

- Added the abstract `FrameSource` seam (`app/media/FrameSource.{h,cpp}`): `readNextFrame(timeoutMs, &result, &image)` with `ReadResult` Ok/EndOfStream/Error/Timeout, plus `close()`, `isOpen()`, and `errorString()`. Opening is deliberately implementation-specific and not part of the abstract contract (rawvideo requires caller-supplied geometry).
- Added `FfmpegFrameSource` (`app/media/FfmpegFrameSource.{h,cpp}`): persistent FFmpeg CLI subprocess streaming `rawvideo`/`rgb24`, scaled to caller-supplied geometry; deterministic failures for missing/non-positive geometry, missing/non-file paths, unavailable ffmpeg, non-zero exit, and timeout; `close()`/destructor always terminate then kill and reap the process.
- Added `FramePump` (`app/media/FramePump.{h,cpp}`): a synchronous, caller-driven consumer. `advance()` requests exactly one frame and emits `frameReady` / `streamEnded` / `streamFailed`; no timer, clock, pacing, buffering policy, threading, or playback state.
- Registered the three source/header pairs in both `reelcraft.pro` and `tests/tests.pro`.
- Added 5 tests to `tests/test_project.cpp`: an in-memory `FrameSource` test double proving replaceability and every `ReadResult` branch with no ffmpeg; missing-source failure; ffmpeg streaming order/EOF with bounded waits; deterministic open validation, close, and reopen/restart; and an end-to-end pump-over-ffmpeg-subprocess stream to EOF.
- No Application/UI, Objective 10, schema, dependency, or `FrameExtractor` changes. Decision 017 already governs the seam; the concrete interface, rawvideo transport, and pump mechanics remain implementation details per its "intentionally NOT decisions" list, so no new decision was recorded.

### Verification

- Focused Objective 3 tests: 7 ran (5 new + init/cleanup), all passed.
- Full regression: 144 passed, 0 failed, 0 skipped.
- Application build succeeded; test build succeeded.
- Official `scripts/build_and_test.sh`: builds succeed, 144/0/0.
- Offscreen smoke: event loop alive until timeout (SMOKE_EXIT=124).

### Boundary Notes

- Build/verification for this objective ran in the DSH x86_64 Ubuntu 24.04 agent container against Ubuntu `qt6-base-dev` 6.4.2 (installed for verification), not the Termux/proot primary development device; recorded in DEVELOPMENT_ENVIRONMENT.md.
- No continuous/paced playback, duration metadata/ffprobe, audio, timeline, or viewer playback wiring exists; `FrameExtractor` remains the single-frame preview/validation seam.
- A production FFmpeg-library or QtMultimedia replacement remains deferred (Decision 017) and can implement `FrameSource` behind the same boundary.

## 2026-09-16 — Phase 3 Objective 4: Player/Timing Subsystem Foundation

### Objective

Introduce the deterministic player/timing foundation above the Objective 3 `FramePump`: playhead/position, play/pause/stop state, deterministic advancement, and replaceable clock and pacing abstractions, without changing the Objective 10 contract and without UI wiring.

### Work Completed

- Added `Playhead` (`app/playback/Playhead.{h,cpp}`): frame count since reset, current frame index (-1 before any frame), and presentation timestamp derived from an explicit frame interval. Pure and clock-free.
- Added the `Clock` abstraction (`app/playback/Clock.h`) and `SystemClock` (`app/playback/SystemClock.{h,cpp}`, monotonic `QElapsedTimer`). Tests inject a manual clock so no test depends on wall-clock timing.
- Added the `PacingPolicy` abstraction (`app/playback/PacingPolicy.h`) and `DefaultPacingPolicy` (`app/playback/DefaultPacingPolicy.{h,cpp}`): elapsed-time-to-frame-count with catch-up frame dropping. Replaceable and independently testable.
- Added `Player` (`app/playback/Player.{h,cpp}`): `Stopped`/`Playing`/`Paused` state machine with `play()`/`pause()`/`stop()`, playhead getters, `tick()` (clock/pacing-driven, bounded per-tick catch-up), `stepOnce()` (explicit single frame), and `stateChanged`/`positionChanged`/`framePresented`/`playbackEnded`/`errorOccurred` signals. It consumes the `FramePump` strictly through its existing `frameReady`/`streamEnded`/`streamFailed` signals.
- Registered the subsystem in `reelcraft.pro` and `tests/tests.pro`.
- Added 10 tests to `tests/test_project.cpp` covering playhead math, initial state/defaults, play/pause/stop transitions, injected clock/pacing behavior, default interval pacing, `stepOnce`, end-of-stream, pump error propagation, invalid configuration/boundaries (null pump, invalid interval, null policy, non-monotonic clock), and bit-for-bit repeatability without wall-clock delays.
- `FramePump` behavior and Objective 3 tests are unchanged. No Application/UI wiring, timer, thread, audio, duration/ffprobe, timeline, schema, dependency, or Objective 10 changes.
- No new DECISIONS entry: Decision 017 already fixes the player/timing boundary and explicitly lists the clock implementation and playback-rate/pacing implementation as implementation details, not decisions.

### Verification

- Clean application and test builds succeed.
- Focused Objective 4 tests: 12 ran (10 new + init/cleanup), all passed in ~1 ms (no wall-clock dependence).
- Full regression: 154 passed, 0 failed, 0 skipped.
- Offscreen smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Official `scripts/build_and_test.sh`: builds succeed, 154/0/0.

### Boundary Notes

- The `Player` owns no timer and no thread; a future Application-level objective must supply the event-loop driver that calls `tick()` and will orchestrate source/pump/player lifecycle against the active-media contract.
- `stop()` resets the playhead but does not rewind or reopen the underlying source; restarting a stream requires source/pump lifecycle management (deferred).
- The playhead is frame-interval-based (a playback pacing parameter) rather than duration/timestamp-based; ffprobe duration metadata remains a future decision gate (Decision 017).
- Build/verification again ran in the DSH x86_64 Ubuntu 24.04 agent container against Ubuntu `qt6-base-dev` 6.4.2; no dependency or environment change for this objective.

---

# 2026-09-17 — 360 Reframing Engine (Deterministic Vertical Slice)

### Context

The human-approved product priority moved to making the core 360 editing/reframing capability functional rather than mechanically continuing the Phase 3 queue. Inspection of the repository, documentation, and Git history confirmed reusable Phase 2/3 primitives (the ViewportState/ViewerProjection/EquirectView camera conventions, `FrameExtractor`, the `FrameSource`/`FramePump` seam, and the `Player`/`Playhead` foundation) but no structured reframing representation and no deterministic execution path from decisions to rendered output. Phase 3 Objective 5 (Application-level player lifecycle) was therefore intentionally not started.

### Work completed

- Added `app/reframe/ReframePlan.{h,cpp}` and `CameraKeyframe.{h,cpp}`: a structured, versioned, JSON-serializable, validated reframing plan (source media id, source time range, output width/height/fps, ordered keyframes with yaw/pitch/roll/FOV and linear/hold interpolation). Validation rejects empty plans, out-of-range or non-increasing keyframe times, non-finite/out-of-bounds camera values, invalid output, and ranges shorter than one frame.
- Added `app/reframe/CameraPath.{h,cpp}`: a pure, clock-free deterministic camera evaluator (shortest-path yaw interpolation, bounded pitch/roll/FOV, hold/linear segments, deterministic hold outside the keyframe range).
- Added the replaceable `ReframeFrameProvider` seam and the first production implementation `FfmpegSeekFrameProvider`, reusing the existing FFmpeg-CLI single-frame seam and never modifying the source media.
- Added `ReframeRenderer`: deterministic per-frame reframing through `EquirectView`, PNG-sequence output, and an FFmpeg H.264 encode step.
- Added `ReframeIntent` + `ReframeIntentParser`: a deterministic natural-language boundary recognizing aspect/platform, time ranges, named/explicit camera directions, and subject references. Unresolved subjects are recorded, never fabricated.
- Added `ReframePlanBuilder`: converts an intent plus resolved `ReframeTarget` directions into a validated plan; unresolved references produce a deterministic error.
- Added `ReframePipeline`: end-to-end orchestration (360 source -> intent -> plan -> deterministic reframing -> encoded flat video).
- Registered all new files in `reelcraft.pro` and `tests/tests.pro`.
- Added 26 tests to `tests/test_project.cpp` (keyframe/plan JSON round-trip and rejection, frame timing, camera interpolation incl. shortest-yaw, hold, and bound normalization, rendering determinism and provider-failure handling, PNG/encode, intent parsing, plan building, and a real FFmpeg end-to-end render).

### Verification

- Clean application and test builds succeed (Qt 6.10.2, Debian GCC 15) with zero compilation errors.
- Full automated suite: 180 passed, 0 failed, 0 skipped, ~41.5 s (154 prior + 26 reframing tests).
- End-to-end pipeline test generates a real equirect clip with FFmpeg, runs intent -> plan -> reframing -> H.264, and decodes the output to the expected dimensions.
- Offscreen application launch smoke: event loop alive until timeout (SMOKE_EXIT=124).

### Environment notes (this session)

- This session ran on the aarch64 proot/Termux development device. The Debian GCC 15 driver could not locate `cc1`/`cc1plus` (libexec layout) or `ld` when invoked as bare `g++`; this was repaired non-destructively by symlinking them into `/usr/lib/gcc/aarch64-linux-gnu/15/` and invoking `/usr/bin/g++` so the driver computes its prefix. Builds pass `QMAKE_CC=/usr/bin/gcc QMAKE_CXX=/usr/bin/g++`.
- FFmpeg is not on the Debian PATH; the Termux build works from this environment and is supplied via `REELCRAFT_FFMPEG=/data/data/com.termux/files/usr/bin/ffmpeg`.
- The `bash` tool remains unavailable (the workspace-write bwrap sandbox backend cannot start on this host; escalation was declined). All inspection, builds, and tests were performed through the in-process code runtime and detached background processes, without requesting wider sandbox permissions. See `KNOWN_ISSUES.md`.

### Boundary notes / not implemented

- No target/subject detection or tracking, speaker/dialogue analysis, content-based cut selection, or AI-provider/model integration.
- Reframe plans are validated and serializable but are not yet persisted in the project schema, and reframing is not wired into `Application`/`MainWindow`.
- Requests that reference an unresolved subject fail deterministically rather than guessing a direction.

### Decisions

- Decision 018 recorded: structured reframing plan boundary; replaceable frame-provider seam; deterministic renderer; deterministic intent boundary; unresolved targets reported.

---

# 2026-09-17 — 360 Reframing Objective 2: Target/Subject Resolution

### Context

Objective 1 delivered the deterministic reframing engine but no way to find a target. A normal 2D detector on a full equirectangular frame is unreliable near the poles and the 0/360-degree seam. After inspecting the existing architecture and researching detection/tracking options and licenses, the objective was implemented as a replaceable, dependency-free target-resolution layer above the existing reframing engine.

### Research and licensing

- Recorded in `docs/TARGET_RESOLUTION_TECHNOLOGY.md`. Verified from each project's own LICENSE and the Ultralytics licensing page:
  - Rejected as a default: Ultralytics YOLO (AGPL-3.0; enterprise license required for proprietary/closed-source/SaaS use); YOLO-NAS (restrictive/non-commercial weight terms).
  - Recommended permissive candidates: YOLOX (Apache-2.0), RT-DETR (Apache-2.0), OpenCV / OpenCV Zoo (Apache-2.0, per-model check), MediaPipe (Apache-2.0), ONNX Runtime (MIT).
- Chosen strategy: detect in overlapping perspective (tangent) views and reproject to the sphere, reusing the tested `EquirectView` projection; no linked CV dependency.

### Work completed

- Added `app/target/EquirectProjection.{h,cpp}`: pure 360 geometry (equirect pixel <-> direction, tangent-view pixel <-> direction exactly matching EquirectView, detection box -> spherical centre + angular radii, seam-safe great-circle distance, yaw normalization, pitch/FOV bounds).
- Added `app/target/EquirectViewPlan.{h,cpp}`: deterministic overlapping view coverage with automatic polar views.
- Added `app/target/TargetDetector.h` and `app/target/ProcessTargetDetector.{h,cpp}`: replaceable detector seam plus a subprocess file/JSON adapter.
- Added `app/target/TargetTypes.{h,cpp}`: detection/query/observation types (timestamp, identity, yaw/pitch, confidence, class, angular extent, evidence) and `TargetTrack` with shortest-yaw sampling and representative-target selection.
- Added `app/target/SphericalTargetTracker.{h,cpp}`: deterministic spherical NMS (preferring larger angular footprint on overlap) plus greedy nearest-neighbour association with gate and miss counting.
- Added `app/target/TargetResolver.{h,cpp}`: view -> detector -> spherical observations -> tracks; notes unresolved queries; `resolvedTargets()` bridges to the existing `ReframePlanBuilder`.
- Added `app/target/TargetTrackPlanner.{h,cpp}`: track -> validated `ReframePlan` consumed by `CameraPath`/`ReframeRenderer`.
- Registered the module in `reelcraft.pro` and `tests/tests.pro`.
- Added 39 tests (geometry/round-trip/seam/pitch, view coverage/determinism/poles, tracking identity/merge/gate/misses/confidence/seam/determinism, track sampling/representative, resolver detection/query/unresolved/invalid/determinism/trajectory, `resolvedTargets` -> `ReframePlanBuilder` -> `CameraPath`, subprocess parse/execute/failure, track planning, and a detection -> plan -> render integration).

### Failure recovery

- First test run: 212/7. Root cause: a radians/degrees unit error in the equirect yaw mapping in `EquirectProjection` (yaw returned in radians). Fixed both conversion directions.
- Second test run: 218/1. Root cause: within-frame duplicate suppression kept a target sliver clipped at a neighbouring view edge over the full detection. Fixed by preferring the larger angular footprint when confidence ties.

### Verification

- Clean application and test builds (Qt 6.10.2, Debian GCC 15) with zero compilation errors.
- Full automated suite: 219 passed, 0 failed, 0 skipped (~40 s).
- Model-free end-to-end tests: synthetic equirect -> color detector -> spherical direction -> `ReframeTarget` -> `ReframePlanBuilder` -> `CameraPath`; and detection -> track -> plan -> `ReframeRenderer` render.
- Offscreen application smoke: SMOKE_EXIT=124.

### Boundary notes / not implemented

- No real detection model is bundled or executed: the environment has no OpenCV/ONNX/TFLite runtime and no pip for the Debian Python. The seam and subprocess protocol are ready; a permissive helper (e.g. YOLOX/RT-DETR via ONNX Runtime) can be added later. This is clearly a "model integration not verified" result, not "real detection verified".
- "Me" identity, speaker localization, semantic classification, production tracking quality, reframe-plan persistence, and UI integration remain future work.

### Decisions

- Decision 019 recorded (tangent-view detection, replaceable detector, deterministic tracker, no linked CV dependency, unresolved targets reported). Decision 018 preserved unchanged.

---

# 2026-09-17 — 360 Reframing Objective 3: Real Detector Integration

### Context

Objective 2 established the replaceable `TargetDetector`/`ProcessTargetDetector` boundary but bundled no real model, so real detection was unverified. The objective was to run a real, permissively licensed detector through that boundary on actual 360 footage and carry the result through tracking, planning, and deterministic rendering. No RunPod credentials were available, so lightweight CPU inference was used on-device; the boundary keeps the runtime replaceable.

### Model and licensing

- Selected OpenCV Zoo `object_detection_yolox_2022nov.onnx` (Apache-2.0 code and weights), COCO 80 classes, ~35.8 MB. Runtime: Python 3 + OpenCV DNN 4.10 (`python3-opencv`) on CPU. Verified from the model directory `LICENSE`.
- Install: `apt-get install -y --no-install-recommends python3-opencv python3-numpy`; weights downloaded to `~/.cache/reelcraft/models/` (not committed).
- Ultralytics YOLO remains rejected (AGPL-3.0). Recorded in `docs/TARGET_RESOLUTION_TECHNOLOGY.md` and Decision 020.

### Work completed

- Added optional helper `tools/detector_helper/yolox_detector.py` implementing the existing file/JSON protocol, plus `coco_classes.py` and `README.md` (model, weights, runtime, licenses, install, GPU, protocol, limitations). The helper code is adapted from the OpenCV Zoo demo (Apache-2.0); the C++ build links no CV library.
- Added the `realDetectorIntegration` test to `tests/test_project.cpp`; it is skipped unless `REELCRAFT_TARGET_DETECTOR_PY`, `REELCRAFT_TARGET_DETECTOR_SCRIPT`, `REELCRAFT_TARGET_YOLOX_MODEL`, and `REELCRAFT_TARGET_CLIP` are set, so the normal suite stays model-free.
- No changes to the target boundary or the 360 geometry.

### Real detection result

- Footage: the project's real `360_TEST_4K.mp4` (3840x1920 VP9, 2:1 equirect, 501 s). A 1920x960 x264 proxy of the 114-126 s segment was used for speed; the original was not modified.
- Full-equirect sanity check (source t=120 s): 4 `person` detections, top two at confidence 0.89.
- Tangent-view pipeline (front view, vertical FOV 110, 512x512): presenters mapped to yaw -27.9/pitch -28.6 and yaw +26.9/pitch -27.8. An independent reproduction of the same projection produced identical coordinates, confirming the C++ mapping.
- Tracker: 9 raw tracks across five sampled frames (proxy 5.5-7.5 s); the strongest `person` track held id `t1` for all five observations (mean confidence 0.923).
- Reframe: the track produced a validated `ReframePlan`; `ReframeRenderer` produced a 640x360 H.264 clip (4 frames) whose frames visibly center the detected presenter.

### Verification

- Real integration test passed (PASS, ~115 s including 30 detector invocations and rendering).
- Normal model-free suite: 219 passed, 0 failed, 1 skipped (the real test when unconfigured).
- Application build clean; offscreen smoke remains green historically (no app source changed).

### Boundary notes / not implemented

- No RunPod/CUDA path was exercised (no credentials); CPU only. The helper can select CUDA/TIM-VX/CANN backends when available.
- "Me" identity, speaker localization, open-vocabulary classes, and production tracking quality remain future work. The tracker had an easy case (two static presenters about 55 degrees apart).
- The helper is process-per-view and reloads the model each invocation; intentionally unoptimized.

### Decisions

- Decision 020 recorded (real detector selection and preserved boundary). Decisions 018 and 019 preserved unchanged.

---

# 2026-09-17 — 360 Reframing Objective 4: Target Identity & Deterministic Selection

### Context

Real detection/tracking existed (Objective 3) but there was no structured creator identity and no deterministic multi-person selection. The objective was to represent "this person is me" as structured data, distinguish the selected target from other people deterministically, and keep identity across frames without adding appearance re-identification, speaker association, or GPU work.

### Work completed

- Added `app/target/TargetIdentity.{h,cpp}`: `CreatorTargetSelection` (identity, time, yaw/pitch, optional track id/label/evidence), `IdentityBinding`, and `TargetIdentityRegistry` (deterministic seed binding, explicit track binding, claim conflicts, active-state refresh, unique continuity re-binding, JSON round-trip). All structured and inspectable.
- Added `app/target/TargetSelector.{h,cpp}`: deterministic reference resolution and a canonical track order (first observation time, numeric track id, id string). Ambiguity is reported with candidates; nothing is guessed.
- Hardened `SphericalTargetTracker`: bounded constant-velocity prediction (yaw/pitch) for crossing trajectories, plus a bounded re-entry gate for temporary loss/occlusion/re-entry. No appearance model.
- Registered the new files in `reelcraft.pro` and `tests/tests.pro`.
- Added 21 model-free tests and extended the real-detector integration test with seed selection, identity binding, canonical selection, and ambiguity reporting.

### Failure recovery

- The selector initially set the returned `ReframeTarget.id` to the raw reference; the NL builder matches the parsed and normalized subject, so the id was changed to the normalized reference. Verified by the identity -> builder -> CameraPath test.

### Verification (model-free)

- Full suite: 240 passed, 0 failed, 1 skipped (~42 s). New tests cover crossing under prediction, re-entry within/beyond window, prediction determinism, seed binding, claim conflicts, active-state resolution, unique vs ambiguous continuity re-binding, identity JSON round-trip, creator aliases, other-person ambiguity, ordinal determinism regardless of input order, left/right, track-id/label, and identity -> `ReframePlanBuilder` -> `CameraPath`.

### Verification (real footage)

- Real integration test PASS (~128 s) on `360_TEST_4K.mp4` via the 1920x960 proxy (original untouched): 9 person tracks; identity "me" bound by seed to the strongest presenter (t1, 5 observations, mean confidence 0.923); `person 1` -> t1; `the other person` -> ambiguous with 8 candidates; the bound track produced a `ReframePlan` and a 640x360 H.264 output centered on the selected presenter.

### Boundary notes / not implemented

- "Me" is geometric, creator-selected identity, not biometric. It does not re-identify after long absence or among similar people; the registry reports unresolved/ambiguous rather than guessing.
- Speaker/active-speaker association, appearance/embedding re-identification, open-vocabulary classes, and GPU inference remain future work.

### Decisions

- Decision 021 recorded. Decisions 017-020 preserved unchanged.

---

# 2026-09-17 — 360 Reframing Objective 5: Appearance-Based Re-Identification

### Context

Identity was geometric only (Decision 021), so a person could not be re-acquired after a long absence or among similar people. The objective was an optional, replaceable appearance layer that strengthens the creator identity without coupling the C++ core to a model runtime and without claiming biometric identity.

### Research and licensing

- Selected OpenVINO Open Model Zoo `person-reidentification-retail-0277` (ONNX `person-reidentification-retail-0265.onnx`, 256-d, input 256x128 BGR 0-255): code and weights **Apache-2.0**, trained on an internal dataset. Runtime: ONNX Runtime (MIT).
- Rejected: OpenCV Zoo `person_reid_youtureid` (directory LICENSE is Apache-2.0 but the weights come from the unlicensed `ReID-Team/ReID_extra_testdata`, trained on Market1501/DukeMTMC/MSMT17); OSNet/torchreid pretrained weights (research-dataset provenance); Ultralytics YOLO (AGPL-3.0).
- Verified the ReID ONNX discrimination on real crops: same presenter cosine ~0.97, different presenter ~0.51.

### Work completed

- `app/target/AppearanceTypes.{h,cpp}`: `AppearanceEmbedding` (unit L2), `AppearanceMath` (normalize/cosine/aggregate), `AppearanceProfile`, `AppearanceEvidence`, explicit `AppearanceVerdict`; JSON round-trip.
- `app/target/AppearanceProvider.h` + `ProcessAppearanceProvider.{h,cpp}`: external file/JSON helper; fail-safe on missing executable/model, non-zero exit, timeout, malformed output, or wrong embedding dimension. No ML runtime linked.
- `app/target/TargetCropExtractor.{h,cpp}`: deterministic crop via the existing `EquirectView`.
- `app/target/IdentityReidentifier.{h,cpp}`: profile maintenance and the documented precedence (explicit/active binding > tracker continuity > unique geometric continuation confirmed/vetoed by appearance > appearance-only single-strong re-acquisition > unresolved/ambiguous).
- `TargetIdentityRegistry` extensions: appearance profile storage and structured decisions (`setAppearanceProfile`, `rebindWithAppearance`, `annotateAppearance`, `markUnresolved`, `rejectTarget`); vetoed tracks cannot be silently re-accepted by geometry.
- Optional helper `tools/appearance_helper/reid_onnx_helper.py` + README with the full licensing record.
- 24 model-free tests; real integration test extended with appearance agreement/discrimination and a controlled re-acquisition.

### Failure recovery

- A `-j4` compile crashed under memory pressure and left a corrupt `AppearanceTypes.o` (656 bytes, no symbols) that `make` reused, causing link-time undefined references. Fixed with `make clean` and a serial `-j1` rebuild.
- The real integration test initially checked the appearance profile under the track id (`t1`) instead of the identity key (`me`); corrected to `TargetIdentityRegistry::creatorIdentity()`.

### Verification

- Model-free suite: 264 passed, 0 failed, 1 skipped (~45 s). New tests cover embedding JSON/invalidity, normalization/similarity, aggregation, verdict/evidence/profile JSON, process provider parse/execute/failure modes, crop determinism, registry profile/veto/re-bind, and the strong/weak/ambiguous/wrong-person/conflict/precedence/missing-provider/provider-failure/determinism re-identification cases.
- Real-footage integration (proxy; original untouched): real embeddings agree for the same presenter and disagree for the other; a controlled tracker gap plus a returning candidate is re-acquired by appearance and fed through identification -> `ReframePlan` -> render.

### Boundary notes

- Appearance is not biometric identity; it supports bounded re-acquisition and can veto a geometric transfer, but never overrides an explicit creator selection.
- No active-speaker/audio-visual association, GPU optimization, or UI integration.

### Decisions

- Decision 022 recorded. Decisions 017-021 preserved unchanged.

---

# 2026-09-17 — 360 Reframing Objective 6: Optional Audio/Speaker Evidence

### Context

"Follow whoever is speaking" requires audio evidence about which visible tracked person is speaking. Audio must remain evidence, not the authoritative identity source, and must never override an explicit creator selection or swap an identity on model confidence alone.

### Research and licensing

- Selected **Silero VAD** (`silero_vad.onnx`): **MIT** code and weights, local CPU via ONNX Runtime (MIT), FFmpeg audio decode. Verified on the real clip and a positive control (Silero's own MIT test audio).
- Rejected/deferred: pyannote diarization (gated models plus PyTorch), SpeechBrain/torchreid speaker embeddings (heavy; VoxCeleb provenance), cloud speaker APIs (no cloud dependence), audio-visual active-speaker models (research-grade or unclear weight licensing).

### Work completed

- `app/target/SpeakerTypes.{h,cpp}`: `SpeakerInterval`, `SpeakerAnalysis`, `SpeakerEvidence`, `SpeakerSegment`, explicit `SpeakerVerdict`; JSON round-trip; strict parsing (an explicit `available` flag is required, so malformed responses fail rather than masquerading as "unavailable").
- `app/target/SpeakerEvidenceProvider.h` + `ProcessSpeakerProvider.{h,cpp}`: external file/JSON helper; fail-safe on missing executable/media, non-zero exit, timeout, malformed JSON, or invalid intervals. No audio/ML runtime linked.
- `app/target/SpeakerTargetAssociator.{h,cpp}`: deterministic association (explicit binding > spatial DoA > single visible > ambiguous/unassociated); never invents a target.
- `app/target/SpeakerTimeline.{h,cpp}`: documented hysteresis, plus a coalescing pass so held speakers become one segment.
- `app/target/SpeakerEvidenceAnalyzer.{h,cpp}` and `SpeakerReframePlanner.{h,cpp}`.
- `TargetIdentityRegistry::annotateSpeaker` (evidence only; resolution unchanged; JSON round-trip).
- Optional helper `tools/speaker_helper/` (Silero VAD) and README with the licensing record.
- 31 model-free tests.

### Failure recovery

- The real integration test initially ran against the video-only proxy from Objective 5 (created with `-an`), so the audio helper correctly reported "no decodable audio stream". Fixed by creating an audio+video proxy of the same window; the original media is untouched.
- A malformed `{}` speaker response was accepted as "unavailable"; parsing was tightened to require an explicit `available` boolean.
- Rapid-alternation timeline output split a held speaker into two adjacent Active segments; added a deterministic coalescing pass.

### Verification

- Model-free suite: 295 passed, 0 failed, 1 skipped (~46 s). New tests cover the evidence model/JSON, process provider parse/execute/failure, association (explicit/not-visible/single-visible/ambiguous/spatial), timeline (single, change, held pause, rapid alternation, overlap, filters), analyzer (association, explicit binding, no provider, provider failure, unassociated speech, tracking loss, overlap, determinism, evidence-does-not-change-identity), registry annotation, and the speaker planner (single, cut on change, no active segment).
- Real footage: Silero VAD detects speech in the real 360 clip; with a creator speaker->target binding the evidence associates to the visible presenter; `SpeakerReframePlanner` produces a `ReframePlan` rendered by the existing renderer.

### Boundary notes / not implemented

- Audio is evidence and a selection signal only; it never rebinds identity.
- Automatic audio-visual attribution without an explicit binding, GPU optimization, and UI integration remain future work.

### Decisions

- Decision 023 recorded. Decisions 017-022 preserved unchanged.

---

## 2026-09-17 — 360 Reframing Objective 7: Audio-Visual Provider Attribution Seam

### Objective and guardrail

Provide automatic audio-visual speaker attribution behind the existing replaceable provider seam, under the explicit efficiency guardrail: the minimum reliable "follow the speaker" capability, using existing components, no speculative infrastructure, and real-media tests only for unresolved questions.

### Work completed

- `app/target/SpeakerTypes.{h,cpp}`: optional `targetIdHint` on `SpeakerInterval` and `SpeakerSegment` (JSON round-trip; absent field decodes to empty, backward compatible).
- `app/target/SpeakerTargetAssociator.{h,cpp}`: honours a visible provider hint as `provider-hint`, after an explicit creator binding and before the spatial rule; a non-visible hint falls through and never invents a target.
- `app/target/SpeakerTimeline.{h,cpp}`: carries the hint from originating intervals onto the coalesced segment (first non-empty wins; cleared for overlap/silence).
- `app/target/SpeakerEvidenceAnalyzer.cpp`: passes `segment.targetIdHint` into association, so the hint reaches the timeline/evidence path end to end.
- 5 new model-free tests.

### Feasibility research (negative result, recorded as the reason no provider ships)

- Real footage audio is mono in the original and the A/V proxy → direction-of-arrival/spatial attribution impossible.
- `cv2.FaceDetectorYN` + OpenCV Zoo YuNet (MIT) detected faces in both presenter tangent views, but a mouth-region-motion vs audio-envelope correlation probe at 12 fps (48 frames, 4 s, lags −3..+3) scored max correlation 0.250 for the speaking presenter vs 0.192 for the other — weak and not a reliable discriminator (the listener even had higher motion energy).
- No permissively licensed, clearly commercial audio-visual active-speaker model was identified (TalkNet/LoCoNet/AV-HuBERT research-grade/unclear weights; pyannote gated; SpeechBrain/torchreid heavy with VoxCeleb provenance).

### Verification

- Model-free suite: 300 passed, 0 failed, 1 skipped. New tests: hint JSON round-trip/backward compatibility, visible-hint attribution among multiple people, non-visible-hint fallback to spatial, explicit binding precedence over a hint, and analyzer/timeline propagation.
- No detector, geometry, identity-resolution, or renderer changes; audio remains evidence-only below appearance.

### Boundary notes / not implemented

- No model-backed automatic attribution is shipped: the available models are either unreliable on this mono footage or not clearly licensed for commercial use. The seam is complete so a licensed provider can be added later.
- GPU optimization and UI integration remain future work.

### Decisions

- Decision 024 recorded. Decisions 017-023 preserved unchanged.
---

## 2026-09-17 — 360 Reframing Objective 8: End-to-End User-Command Execution

### Objective

Provide a single end-to-end 360 command path — user instruction -> parsed intent -> target resolution/identity -> validated `ReframePlan` -> deterministic render — and validate it on real footage, per the human-approved priority (end-to-end command testing rather than more perception subsystems).

### Work completed

- `app/reframe/ReframeCommandRunner.{h,cpp}`: the composition entry point. `prepare()` parses the instruction, resolves subject references through `TargetResolver` (replaceable detector + tracker), binds the optional creator identity (`TargetIdentityRegistry`) and resolves references (`TargetSelector`), and builds the validated plan (`ReframePlanBuilder`). `run()` adds deterministic execution through the unchanged `ReframePipeline`.
- The runner adds no perception of its own and never fabricates a direction: an unresolved or ambiguous reference produces an explicit error naming the reference; a direction-only instruction needs no detector; a creator seed that fails to bind is recorded in notes.
- Registered in `reelcraft.pro` and `tests/tests.pro`; 8 new model-free tests; new env-gated `realUserCommandIntegration`.

### Verification

- Model-free suite: 308 passed, 0 failed, 2 skipped (~47 s). New tests: subject resolution + plan, direction-only without a detector, unresolved honesty, ambiguous honesty, creator-identity "follow me", missing detector, invalid range, determinism.
- Real footage (audio+video proxy; original untouched): `realUserCommandIntegration` ran the command "follow person 1" through the full path with the real YOLOX detector. See `CURRENT_STATE.md` for the recorded result.
- Change-impact: the runner composes existing layers; no detector, geometry, identity, appearance, speaker, planner, or renderer internals changed.

### Boundary notes / not implemented

- The command path is library-level: not yet wired into the Application/UI, plans are not persisted in the project schema, and speaker-aware commands are not parsed. These remain future objectives.
- Inference is CPU-only.

### Decisions

- Decision 025 recorded. Decisions 017-024 preserved unchanged.
---

## 2026-09-17 — 360 Reframing Objective 9: Application-Level 360 Command Orchestration

### Objective

Wire the library-level `ReframeCommandRunner` into the application so a user can submit a natural-language 360 editing command and the product executes the existing deterministic pipeline, with the application owning inputs, lifecycle, output handling, user feedback, and errors.

### Work completed

- `app/application/ReframeCommandOutcome.h`: structured, application-visible command result (source reference, instruction, effective range, output spec/path, frame count, notes, unresolved references, resolved targets; JSON-serializable).
- `Application::runReframeCommand()` / `runReframeCommandTo()`: active-media and application-state validation, output-location validation (directory exists; output != source), request construction, delegation to the injected executor (default `ReframeCommandRunner::run`), and result mapping; emits `reframeCommandFinished`.
- Optional non-owned `TargetDetector`/`ReframeFrameProvider` inputs and an injectable `ReframeCommandExecutor` seam; `main.cpp` builds a `ProcessTargetDetector` from the `REELCRAFT_TARGET_*` environment.
- Minimal `MainWindow` command UI (input, start/end seconds, run button, result label) and `main.cpp` wiring.
- 17 new model-free application tests; new env-gated `realApplicationCommandIntegration`.

### Verification

- Model-free suite: 326 passed, 0 failed, 3 skipped (~60 s).
- Standalone application build and test build both succeed.
- Real footage (audio+video proxy; original untouched): `realApplicationCommandIntegration` ran "follow person 1" through the application path; it resolved to track t1 (ordinal) and rendered 24 frames to a 640x360 clip. The derived range-end sample (12000 ms) was honestly recorded as undecodable and skipped, proving the robustness fix.
- Change-impact: the new application boundary and UI wiring were added; the only lower-layer change is the resolver robustness fix below.

### Failure recovery

- The first real application validation failed during target resolution: the runner's derived sample timestamps include the exact range end (12000 ms), and the 12 s proxy has no decodable frame past ~11.95 s. `TargetResolver::resolveSequence` previously aborted on the first undecodable sample; it now records the sample in its notes and continues, so one boundary/corrupt frame no longer fails the whole sequence (hard detector/invalid-frame errors remain fatal). A model-free `targetResolverSequenceSkipsUndecodableSample` test covers it.

### Boundary notes / not implemented

- Generated renders are session state: `ReframeCommandOutcome` is not persisted in the project schema. A dedicated project output section remains future work.
- No duration/ffprobe metadata: range-less commands use the UI-supplied fallback range rather than the whole clip.
- Speaker-aware commands, GPU optimization, and output persistence remain future work.

### Decisions

- Decision 026 recorded. Decisions 017-025 preserved unchanged.
---

## 2026-09-17 — 360 Reframing Objective 10: Persisted Outputs and Duration-Aware Ranges

### Objective

Persist generated renders in the project and add duration-aware full-clip ranges via the deferred ffprobe duration metadata, so range-less commands default to the whole clip.

### Work completed

- `app/media/MediaDurationProbe.h` + `app/media/FfprobeDurationProbe.{h,cpp}`: a replaceable, external, fail-safe duration seam (external ffprobe; `REELCRAFT_FFPROBE`, a sibling of the resolved ffmpeg, then `PATH`). Reads only; deterministic failures; no codec dependency linked.
- `Application::runReframeCommandTo()`: a zero start/end range ("whole clip") is resolved through the probe to `[0, durationMs]`; explicit ranges never probe; if the duration is unknown the request keeps an invalid range and the runner honestly reports it (or uses the command's own range).
- Render records: every command that reaches an output target appends its `ReframeCommandOutcome` (success or failure with its error) to the authoritative list; `Project` gained an additive `reframeOutputs` section and `CurrentSchemaVersion` 3; `Application` emits `reframeOutputsChanged`; `ReframeCommandOutcome::readFromJsonObject` restores records.
- `MainWindow`: range controls default to 0/0 (whole clip) and a generated-render list is populated from `reframeOutputsChanged`.
- 13 new model-free tests; `realApplicationCommandIntegration` extended to whole-clip + save/open persistence.

### Verification

- Model-free suite: 339 passed, 0 failed, 3 skipped (~54 s).
- Focused Objective 10 tests: 13 passed (including the ffmpeg/ffprobe-gated probe test, which ran).
- Test build and standalone application build both succeed.
- Real footage (audio+video proxy; original untouched): the whole-clip probe resolved 0..12012 ms (the 12012 ms sample was honestly recorded as undecodable and skipped), "follow person 1" resolved to t1 and rendered 24 frames to a 640x360 clip, and the render record survived save/open.
- Change-impact: the probe is a new leaf seam; the record is the existing outcome; `ReframeCommandRunner`, the pipeline/renderer, and all perception/identity layers are unchanged.

### Boundary notes / not implemented

- The whole-clip default requires a working `ffprobe`; without it, range-less commands must specify a range (reported honestly). General playback duration metadata remains future work (Decision 017's broader gate).
- Records are kept even if their output file later disappears; there is no render-availability revalidation yet.
- Speaker-aware commands and GPU optimization remain future work.

### Decisions

- Decision 027 recorded. Decisions 017-026 preserved unchanged.
---

## 2026-09-17 — 360 Reframing Objective 11: Speaker-Aware 360 Commands

### Objective

Reuse the Objective 6/7 speaker evidence layer inside the application command path so commands such as "follow the speaker" select the active speaking target, keeping audio as evidence and never overriding an explicit creator selection.

### Work completed

- `ReframeCommandRunner`: speaker references are recognized and routed through `SpeakerEvidenceAnalyzer` (provider -> timeline -> association) and `SpeakerReframePlanner` (-> `ReframePlan` with cuts at speaker changes). `ReframeCommandRequest` gained an optional non-owned `speakerProvider`, optional explicit `speakerBindings` (speakerId -> targetId), and optional pre-resolved tracks.
- Honest failure modes: no provider, unavailable evidence, unassociated/ambiguous speaker, and unsupported mixing (speaker + subject, speaker + explicit direction) all produce an error and no plan. No direction is fabricated.
- `ReframePipeline::renderPlan()` renders an already-validated plan; `run()` delegates to it; `ReframeCommandRunner::run()` renders the prepared plan so a speaker plan is not re-derived by `ReframePlanBuilder`.
- `ReframeIntentParser` recognizes "keep the speaker centered" / "center the speaker".
- `Application` holds a non-owned speaker provider + bindings and copies them into each request; `main.cpp` builds a `ProcessSpeakerProvider` from `REELCRAFT_SPEAKER_*`.
- 9 new model-free tests; new env-gated `realSpeakerCommandIntegration`.

### Verification

- Model-free suite: 348 passed, 0 failed, 4 skipped (~65 s).
- Focused Objective 11 tests: 9 passed.
- Test build and standalone application build both succeed.
- Real footage (audio+video proxy; original untouched): `realSpeakerCommandIntegration` used 9 pre-resolved tracks, associated the Silero VAD speech to t1 through an explicit creator speaker binding, and rendered 24 frames to a 640x360 clip.
- Change-impact: the deterministic renderer, detector, geometry, identity, appearance, and speaker layers are unchanged; the command path now composes the existing speaker layer and renders the prepared plan.

### Boundary notes / not implemented

- On mono, multi-person footage a provider-local speaker id still needs an explicit creator binding (or a single visible person) to associate; fully automatic attribution depends on a future licensed audio-visual/diarization provider.
- Speaker-aware commands are supported for a single unambiguous speaker request; mixing is reported, not silently reinterpreted.
- GPU optimization and the deferred Phase 3 player-lifecycle objective remain future work.

### Decisions

- Decision 028 recorded. Decisions 017-027 preserved unchanged.
---

## 2026-09-17 — 360 Reframing Objective 12: 360 Command UI and Rendered-Result Preview

### Objective

Human-selected scope (Decision 029): make the existing 360 command path usable from the application and let the creator see a generated reframe, without building a general player/timeline or a parallel command system.

### Work completed

- `Application`: `selectCreatorTargetFromViewport()` seeds "me" from the current viewport yaw/pitch and preview time; `clearCreatorSelection()`; `hasCreatorSelection()`/`creatorSelection()`; `creatorSelectionChanged(bool)`. The selection is session state, cleared on new/open project, and passed into every `ReframeCommandRequest`.
- `Application`: `previewReframeOutput(int)` decodes the first frame of a persisted render record and emits `reframeOutputPreviewReady`; a `ReframePreviewDecoder` seam defaults to `FrameExtractor::extractFirstFrame`. Honest failures for invalid index, missing output, and decode failure.
- `MainWindow`: "Select Center as Me"/"Clear Me" + selection readout, "Preview Selected Render" (acts on the render list selection), and a provider status line; `showReframeOutputPreview` presents flat via the existing viewer flat mode.
- `main.cpp` wiring for all new signals and an initial provider-status update.

### Verification

- Model-free suite: 359 passed, 0 failed, 4 skipped.
- Focused Objective 12 tests: 11 passed.
- Test build and standalone application build both succeed.
- No real-media validation was run: this objective is UI/orchestration and the preview decode reuses the already-verified FFmpeg `FrameExtractor` seam.
- Change-impact: the deterministic renderer, command runner, perception/identity/speaker layers, and viewer rendering are unchanged; the viewer flat mode already existed.

### Boundary notes / not implemented

- The creator selection is session state (not persisted) and generated renders can be previewed as a single flat frame, not played continuously (the deferred Phase 3 player-lifecycle objective).
- In-app model/helper path configuration is not added; external providers remain configured through `REELCRAFT_*` and are reported as configured/not configured.
- GPU optimization and continuous playback remain future work.

### Decisions

- Decision 029 recorded. Decisions 017-028 preserved unchanged.
---

## 2026-09-17 — 360 Reframing Objective 13: 360 Rendered-Result Playback

### Objective

Human-selected scope (Decision 030): turn the Objective 12 single-frame rendered-result preview into deterministic continuous playback of persisted 360 -> flat rendered results, using the existing Phase 3 media/player seams.

### Work completed

- `Application`: owns/creates/replaces/disposes the playback `FrameSource` (default `FfmpegFrameSource` opened with the rendered record's geometry), `FramePump`, and `Player` for the selected result; `startReframeOutputPlayback(index)` (resumes a paused same-record), `pauseReframeOutputPlayback()`, `resumeReframeOutputPlayback()`, `stopReframeOutputPlayback()`, `tickReframeOutputPlayback()`, plus active/playing/position/frame-count/index accessors.
- Event-loop driver: the Application owns a `QTimer` that invokes `Player::tick()`; the Player owns no timer/thread. Injectable `PlaybackSourceFactory` and `Clock`/`PacingPolicy` keep tests model-free; the frame interval derives from the record's output fps (fallback 40 ms), with the driver interval clamped.
- Signals `reframePlaybackFrameReady`, `reframePlaybackStateChanged`, `reframePlaybackPositionChanged`, and `reframePlaybackEnded`. Lifecycle disposal on new/open project and when replacing the selected result; the source is always closed.
- `MainWindow`: Play/Pause/Stop Render controls and a playback position readout; `main.cpp` wires the new signals. Playback frames reuse the existing flat presentation path.
- 9 new model-free tests; new env-gated `realReframePlaybackIntegration`.

### Verification

- Model-free suite: 368 passed, 0 failed, 5 skipped.
- Focused Objective 13 tests: 9 passed.
- Test build and standalone application build both succeed.
- Real media: `realReframePlaybackIntegration` renders a real 360 clip to a flat result through the existing pipeline and plays it back (all 20 frames; see the failure recovery below).
- Change-impact: the media/player seams, deterministic renderer, command runner, perception/identity/speaker layers, and the Objective 10 preview-time contract are unchanged; only Application-level orchestration and the minimal UI were added.

### Failure recovery

- The first real-media completion run played only ~10 of the rendered clip's 20 frames. Diagnosis: a tight read of the completed clip returned all 20 frames, but paced playback ended early with the ffmpeg process exiting 0 after a partial frame. Root cause: `FfmpegFrameSource::readNextFrame` broke out of its read loop when the process was no longer running even though output remained buffered in the pipe, so a slow/paced consumer lost the tail. Fix: read any buffered output first, and drain remaining output before deciding end-of-stream. The real validation then played all 20 frames. This is a correctness fix inside the existing media seam (no architecture change).

### Boundary notes / not implemented

- Playback is limited to persisted rendered results; general/active-media playback, audio, timeline editing, and duration/ffprobe work remain out of scope.
- The creator "me" selection is still session state (not persisted).
- GPU optimization remains future work.

### Decisions

- Decision 030 recorded. Decisions 017-029 preserved unchanged.

---

## 2026-09-17 — 360 Reframing Objective 14: 360 Temporal Editing Operations

### Objective

Human-selected scope (Decision 031): add deterministic temporal editing (retain / remove / target-duration) that composes with the existing reframing, persistence, and playback paths, without building a timeline editor or changing the renderer/projection/playback architecture.

### Work completed

- \`app/reframe/TemporalEditPlan.{h,cpp}\`: parser-independent, JSON-serializable Keep / Remove / TargetDuration with ordered multi-ranges, validation, deterministic normalization, and \`resolve(durationMs, defaultStartMs)\`. Reversed/zero-length/negative/out-of-bounds ranges, empty results, and non-positive durations are rejected.
- \`ReframeIntentParser\` / \`ReframeIntent\` (extended, no parallel parser): \`temporalEdit\` from "cut from X to Y", "remove X to Y", "keep X and Y", "make a N-second version", and "this section", including word timestamps ("35 seconds", "1 minute 10"). Invalid, ambiguous, contradictory, and out-of-bounds requests are explicit; temporal clauses are not misread as camera windows and are skipped by the camera parser.
- \`ReframeCommandRunner\`: resolves the structured edit against the known source duration and applies the retained ranges as an ordered \`ReframePlan::segments\` list (additive; empty = the existing single source range). The source range is expanded to keep keyframes and segments valid. Composes with target/identity/speaker resolution.
- \`ReframePlan\`: \`frameCount()\`/\`frameTimeMs()\`/\`isValid()\` honor segments; JSON round-trips them; the existing \`ReframeRenderer\`/\`ReframePipeline\` execute them unchanged.
- \`Application\`: supplies the probed whole-clip duration as \`sourceDurationMs\`, records the resolved segments in \`ReframeCommandOutcome\`, and sets the outcome range to the segments' bounding range.
- \`ReframeCommandOutcome\`: additive \`temporalSegments\` (JSON round-trippable).
- 11 new model-free tests plus an env-gated \`realTemporalEditIntegration\`.

### Verification

- Model-free suite: 379 passed, 0 failed, 6 skipped.
- Focused Objective 14 tests: 11 passed.
- Test build succeeds.
- Real media (\`realTemporalEditIntegration\`, audio+video proxy; original untouched): "Keep 0:00 to 0:01 and 0:04 to 0:05." retained 0..1000 ms and 4000..5000 ms, rendered a 320x180 @ 10 fps result (20 frames), and played it back through Objective 13.
- Change-impact: the renderer, projection, camera path, perception/identity/speaker layers, and the playback architecture are unchanged; the additions are the temporal representation, parser clauses, plan segments, runner resolution, and outcome persistence.

### Failure recovery

- The first parser version fed the temporal clause to the camera-subject parser, so "Keep 0:00 to 0:30, then follow me." produced a spurious unresolved subject "0:30" (pattern "to <subject>"). Fix: the camera parser now skips temporal clauses (\`isTemporalClause\`). A test instruction that used "1:00" intending one second was corrected to "0:01".

### Boundary notes / not implemented

- No timeline editor, drag/drop/scrubbing UI, captions, transitions, effects, color grading, or audio editing.
- "this section" resolves to the command's effective range; there is still no persisted selection, and a temporal clause combined with a camera clause inside one clause without "then" (for example "keep 0:00 to 0:30 and follow me") loses the camera clause.
- The creator "me" selection remains session state; GPU optimization remains future work.

### Decisions

- Decision 031 recorded. Decisions 017-030 preserved unchanged.

---

## 2026-09-17 — 360 Reframing Objective 15: 360 Compound Natural-Language Editing

### Objective

Human-selected scope (Decision 032): a single natural-language command combining a temporal edit and a camera/target instruction must preserve both operations (fixing the Objective 14 limitation where a temporal clause was skipped wholesale by the camera parser). Composition only; no second parser, temporal representation, renderer, or playback path.

### Work completed

- ReframeIntentParser: a temporal clause's time ranges (or a target-duration phrase) are stripped before camera/target extraction; the operation keyword is retained so "keep <subject> centered" still matches, and residual trailing sentence punctuation is removed so the end-anchored subject patterns still match. Temporal clauses are still never interpreted as target subjects.
- ReframeIntent::hasCompoundEdit() explicitly represents the composition; the parser records the documented whole-retained-range applicability rule in the intent notes.
- Unsupported composition (a camera instruction with its own separate time interval) is detected from leftover time tokens after the temporal ranges are removed and fails honestly with a "separate time interval" error. Invalid, ambiguous, and contradictory temporal edits remain honest.
- No changes to TemporalEditPlan, ReframePlan, ReframeRenderer/ReframePipeline, or the Objective 13 playback path; the source stays read-only.

### Verification

- Model-free suite: 381 passed, 0 failed, 7 skipped.
- Focused Objective 15 tests: 2 passed (parser composition and runner composition), run alongside the affected Objective 14 intent/command tests (13 passed in the focused batch).
- Test build and standalone application build both succeed.
- Real media (realCompoundCommandIntegration, audio+video proxy; original untouched): "Keep 0:00 to 0:01 and look left." retained 0..1000 ms with the left camera direction, rendered 10 frames to a 320x180 @ 10 fps result, and played all 10 back through Objective 13 (13.8 s).
- Performance: full model-free regression ~68 s, real compound validation ~13.8 s; no unexplained regression.

### Failure recovery

- Class B ("From 0:35 to 1:10, keep the person I selected centered.") initially produced no camera move because the residual sentence period defeated the end-anchored "keep ... centered" subject pattern. Fix: trim trailing punctuation from the temporal camera residue before camera parsing.

### Boundary notes / not implemented

- The applicability rule is whole-retained-range; a camera instruction with its own separate interval is unsupported. Arbitrary sequential applicability ("follow me for the first 20 seconds, then ...") remains out of scope.
- The creator "me" selection remains session state; GPU optimization remains future work.

### Decisions

- Decision 032 recorded. Decisions 017-031 preserved unchanged.

## 2026-09-17 — 360 Reframing Objective 16: Persisted, Reproducible Edit Decisions

### Objective

Human-selected scope (Decision 033): a persisted, versioned `EditDecision` artifact sufficient to reproduce a render without re-parsing the natural-language command and without requiring a perception provider. Original media must remain untouched; the artifact must carry `schemaVersion` from day one with a loader that refuses to silently mis-parse; serialization must be deterministic; the existing persistence mechanism and renderer must be reused with no parallel pipeline, database or ORM.

### Work completed

- `app/reframe/EditDecision.{h,cpp}` (new): versioned artifact holding the resolved `ReframePlan` verbatim, a source reference (mediaId, path, sizeBytes, lastModifiedUtc, optional contentSha256), the instruction as provenance, `createdUtc`, and `decisionHash`. `payloadWithoutHash()` excludes only the digest; `decisionHash()` is SHA-256 over its compact JSON. Strict `readFromJsonObject`/`load`, `save`, `checkSource` returning a `SourceStatus` enum (`Matches`/`FileMissing`/`FingerprintMismatch`), and `isValid`.
- `ReframeCommandOutcome` gained `hasEditDecision()`, `editDecision()`, `editDecisionError()`, `setEditDecision()` (refuses an invalid decision *with its reason*) and `setEditDecisionUnavailable()`; the decision is serialized into and read from the existing record.
- `Application::runReframeCommandTo()` takes a by-value `MediaItem` snapshot at validation time and attaches the decision in the single `finish` path whenever `result.plan.isValid()`. `Application::appendReframeOutput()` became the single append/emit path shared with replay. `Application::restoreReframeOutputsFromJson()` surfaces unreadable decisions through `backgroundCompleted`.
- `Application::replayEditDecision(int, const QString&)` returns `ReplayResult { ok, newRecordIndex, error }`; validation completes before any render (index, decision, source status, empty path, source-equal path, existing path), then renders via `ReframePipeline::renderPlan` and appends a new record. `ReframeReplayRenderer` is an injectable seam defaulting to `renderPlan`.
- 19 new tests: 7 artifact unit tests, 3 record-integration/back-compat tests, 3 decision-attachment tests, 5 replay tests including a fresh-process child driven by re-invoking the test binary with a single QtTest function name plus `REELCRAFT_TEST_REPLAY_*` variables.

### Verification

- Full model-free suite: **401 passed / 0 failed / 8 skipped** (the 8th skip is the child-only slot `replayFreshProcessChild`, which skips by design when run standalone).
- Replay equivalence verified twice: same-process and fresh-process, each asserting decoded-frame SHA-256 equality (primary) and byte-identical containers (secondary, environment-specific).
- Perception-absence verified at object-code level: `EditDecision.o`'s external surface is Qt plus `ReframePlan::readFromJsonObject`; the disassembled call graph of `ReframePipeline::renderPlan` and of the fresh-process child contains no `ReframeIntentParser`, `TargetDetector` or provider symbol.
- Measured cost of the by-value `MediaItem` snapshot: ~209 ns per command (2M-iteration benchmark against the real object), against a command that performs file-system checks and a render measured in hundreds of milliseconds.

### Environment correction (supersedes the initial working hypothesis)

- During this objective the real-render tests failed intermittently (0, 12, and 14 failures across identical runs). The initial attribution was environmental intermittency; that attribution was **wrong** and is superseded by the diagnosis below.
- Cause: a specific, diagnosable **FFmpeg PATH leak**. Inside `proot-distro login ubuntu`, `PATH` ends with `/data/data/com.termux/files/usr/bin`, and Debian had no `ffmpeg` of its own, so `ffmpeg` resolved to the Termux Android/bionic build loaded against Debian glibc (`/lib/aarch64-linux-gnu/libm.so: invalid ELF header`). A single seek+decode measured **14,607 / 15,570 / 15,939 ms** against `FrameExtractor`'s fixed `kProcessTimeoutMs = 15000` budget, so runs passed or failed by a few hundred milliseconds.
- Fix: `apt-get install -y ffmpeg` inside the container (Debian 8.0.1-3ubuntu2). `which ffmpeg` is now `/usr/bin/ffmpeg`, `ldd` is clean, a seek+decode measures **2,590-2,640 ms**, the suite runs in **221-241 s** instead of 850-1,021 s, and every previously failing real-render test passes.
- Consequence: `scripts/build_and_test.sh` runs the suite under `timeout 240`; with the leaked Termux ffmpeg it could never have completed inside that ceiling. This was the first time the documented verification workflow was actually viable on this device. Recorded in `DEVELOPMENT_ENVIRONMENT.md` (as REQUIRED setup) and `KNOWN_ISSUES.md`.

### Boundary notes / not implemented

- Decisions are persisted only when a command reaches a non-empty output path; early application-state and validation failures produce no record and therefore no decision.
- No sidecar decision files, no project schema bump (stays 3), no `allowOverwrite` override (recorded as a forward extension in Decision 033), no decision mutation/editing, and no retention policy for accumulating decisions (recorded in `KNOWN_ISSUES.md`).
- Creator-facing inspection and revision of decisions is Objective 17 and was not started.

### Decisions

- Decision 033 recorded: the artifact, embedding in the existing record, schema 3 with per-artifact versioning, timestamp quantization (record/load/compare), lenient-record/strict-decision, absent-hash tolerance, the reproducibility claim, the known outputPath limitation, the Obj17 immutability/lineage constraint, the `allowOverwrite` forward extension, and the enforced output-path refusals. Decisions 017-032 preserved.


## 2026-09-18 — 360 Reframing Objective 17: Creator Decision Provenance & Revision

### Objective

Human-locked scope (Decision 034): give persisted `EditDecision` artifacts provenance and an immutable revision path, reusing the existing free-text pipeline, without touching target-identity state, without operation-level editing, and without weakening any existing invariant.

### Work completed

- `EditDecision` schema v1 -> v2 (`CurrentSchemaVersion = 2`). The load gate already accepted every version in `1..Current`, so no legacy-read path was needed.
- Investigated the load/serialize logic before implementing, as required. Finding: the loader stores the version it read (`decision.m_schemaVersion = schemaValue.toInt()`) and `payloadWithoutHash()` writes that stored value, so a loaded v1 decision is already version-retaining. Implementing a silent upgrade to v2 would have changed the payload, invalidated the recorded digest, and caused the strict loader to refuse previously-valid decisions. A regression test locks this in.
- `origin` (`command` | `creator-revision`) and optional `parentDecisionHash`, inserted into the canonical payload only when non-empty. Omission is a correctness requirement: unconditional writing would mutate every legacy payload and break its digest.
- `EditDecision::revisedFrom(parent, plan, media, instruction, createdUtc)` -- a new immutable child carrying the parent digest. `isValidOrigin()` and `isValidDecisionHash()` (64 lowercase hex).
- `Application`: `runReframeCommandTo()` split into a thin wrapper plus `runReframeCommandInternal(..., const EditDecision *parentDecision)`, shared by `reviseEditDecision()` so there is one command path and one append gate. New `RevisionResult` and `DecisionProvenance` types.
- `Application::decisionProvenance()` performs **referential** lineage validation against the held records, not merely format validation.
- `restoreReframeOutputsFromJson()` now counts and reports unrestorable records (previously dropped in silence).
- `QLoggingCategory` `reelcraft.decision` for created / loaded / refused / revised.
- 9 new tests added.

### Verification

- Focused Objective 17 run: `Totals: 11 passed, 0 failed, 0 skipped` (9 new tests plus init/cleanup), 652 ms.
- Targeted regression across affected areas (edit-decision artifact, record persistence/restore, replay, command path, plus both ffmpeg-gated replay tests): `Totals: 31 passed, 0 failed, 0 skipped`, 56.7 s. Zero skips means the ffmpeg-gated tests actually executed.
- Incremental `-j1` build only; no clean checkout, no cold build, no configuration change. Build completed in ~4 min (test_project.cpp alone ~2 m 37 s, being a single ~14,400-line translation unit at -O2).
- No warnings or errors in the test logs; no stray processes left after the runs.

### Boundary notes / not implemented

- Target-identity persistence (`TargetIdentityRegistry`, `CreatorTargetSelection`) is explicitly out of scope.
- Accept/Reject is session-only; `AI_EDIT_CONTRACT.md` §9's status vocabulary remains conceptual.
- The deterministic intent -> plan completeness checker remains its own future objective.
- An unparseable render record is now reported but still discarded; unlike an unreadable decision it is not preserved verbatim.
- No `Project` schema bump (stays 3), no retention policy, no remote telemetry, no new dependency.

### Decisions

- Decision 034 recorded. Decisions 017-033 preserved.


## 2026-09-18 — Objective 18 Scoping: Deterministic Intent -> Plan Contract Checker

### Objective

Formal scoping only (Decision 035). No implementation, no build, no tests, no source changes.

### Work completed

- Read-only architectural discovery of the intent -> plan mapping: `ReframePlanBuilder::build()` (output/range selection at `:32-41`, empty-moves synthesis at `:42-51`, keyframe construction at `:84-101`), the second transform `applyTemporal` (`ReframeCommandRunner.cpp:125-148`, applied at `:340` and `:419`), and every validation layer from parser reporting through `ReframePlan::isValid()` and `ReframePipeline::renderPlan`.
- Established the information-preservation table: which intent fields map directly (`hasOutput`, range), which are transformed (`moves[].yaw/pitch` via normalise/clamp; `temporalEdit` -> `segments`), which are discarded (`targetRef`, `label`, `notes`, `unresolvedTargets`), and which have no downstream representation (`recognized`, `hasCompoundEdit()`).
- Identified two false-positive hazards that constrain the rule set: after a temporal edit `plan.sourceRange()` is intentionally widened beyond the requested range, and an empty `moves` list intentionally yields one synthesized keyframe (so a move-count <-> keyframe-count rule is unsound).
- Determined the decisive boundary: the speaker path applies the **same** output rule (`:289-294`) and both paths call `setSourceRange`/`setOutput`, so output and range rules are path-independent; but the speaker planner is documented as owning its keyframes (`:308`), so any keyframe-count or direction rule would false-positive there.
- Verified the IPC-3 premise before recording the rule. `TemporalEditPlan::resolve()` can return an empty list, but `prepare` converts an empty resolution into a hard error (`:113-121`) before any plan is built, and `applyTemporal` copies every resolved segment (`:126-133`). The premise holds, and IPC-3 is recorded as a contract assertion rather than as new coverage.

### Verification

- Read-only inspection only. No build, no tests, no source or `.pro` modification, no dependency change, no commit.
- Working tree contains documentation edits only.

### Boundary notes / not implemented

- Excluded from Objective 18 and recorded with reasons: keyframe-count <-> move-count, per-move direction correspondence, target identity, media identity, labels/notes, and anything `ReframePlan::isValid()` or preparation already guarantees.
- No schema bump, no stored checker result, no command-runner convergence refactor, no parser/renderer change, no LLM.
- Retained candidates for later objectives: creator target-selection persistence and the decision retention/compaction policy.

### Decisions

- Decision 035 recorded (scoped, not implemented). Decisions 017-034 preserved unchanged.


## 2026-09-18 — Objective 18 Implementation: Deterministic Intent -> Plan Contract Checker

### Objective

Implement the scope recorded in Decision 035: a pure deterministic contract checker over the final `(ReframeIntent, ReframePlan)` pair, invoked at both final-plan points. No scope expansion.

### Work completed

- Inspected both integration points and the error convention before changing anything: the speaker path (`result.plan = speakerPlan; applyTemporal(...);`) and the main path (`result.plan = built.plan; applyTemporal(...);`) both set `result.error` and return before `result.ok` is set.
- Added `app/reframe/ReframeContract.{h,cpp}`: `ContractViolation`, `ContractReport`, and `ReframeContract::check()` implementing IPC-1, IPC-2 and IPC-3 in a fixed order. Rule ids are exposed via `outputFidelityRuleId()`, `timeRangeRuleId()` and `temporalMaterialisationRuleId()`; numbers are formatted with `QString::number(value, 'g', 10)` for locale independence.
- Registered the new source and header in both `.pro` files and re-ran qmake (required because sources were added; no flag, dependency or configuration change).
- Inserted the checker at both call sites immediately after `applyTemporal`, each as a four-line guard using the existing error path. No convergence refactor.
- Added 6 tests. The speaker-path anti-false-positive test was initially inserted before its helper declarations and failed to compile; it was relocated after `SpeakerScriptProvider`/`speakerInterval`, which is the same ordering the existing speaker command test relies on.

### Verification

- Focused run of the 6 new tests: `Totals: 8 passed, 0 failed, 0 skipped` (1.57 s).
- Targeted regression across the affected area (runner, builder, intent, plan, temporal, speaker, pipeline, application command paths, plus the new tests): `Totals: 55 passed, 0 failed, 0 skipped` (21.3 s).
- Incremental `-j1` build only; no clean checkout, no cold build, no dependency change.

### Recorded finding (honesty note)

All three rules hold by construction on both paths today: both planners derive output and range from the same intent-or-default rule, `applyTemporal` only ever widens the range, and preparation already refuses an empty temporal resolution. **No reachable input can currently violate the contract.** The checker's value is as an executable contract against future divergence, and the FATAL wiring is therefore covered by rule-level tests plus inspection of the two call sites rather than by an end-to-end integration test.

### Boundary notes / not implemented

- Excluded per Decision 035: keyframe-count <-> move-count, per-move direction correspondence, target identity, media identity, labels/notes, and anything `ReframePlan::isValid()` or preparation already guarantees.
- No schema bump, no persisted checker result, no parser/plan/`EditDecision`/renderer/playback change, no LLM, no convergence refactor.

### Decisions

- Decision 035 updated with an implementation-outcome section; its scope text is unchanged. Decisions 017-034 preserved.


## 2026-09-18 — Objective 19 Implementation: Real 360 Source Playback

### Objective

Make real equirectangular 360 media observable inside Reelcraft: continuous playback, seek, viewpoint interaction, clean stop/end-of-media, without a decoder process per displayed frame. Automatic reframing explicitly out of scope.

### Inspection (before any change)

- Media import path (`MediaItem::createFromFilePath`, `Application::importMediaFile`), `FfmpegFrameSource`, `FramePump`, `Player`/`Playhead`, `EquirectView` + the projection path, the single-frame preview path, the rendered-result playback path (Objective 13), audio handling (none), and the playback tests.
- Key findings: `FfmpegFrameSource::open()` already applies an FFmpeg `scale=` filter and requires caller-supplied geometry; there is no seek support; `Player` paces presentation but has no seek; audio is absent everywhere (`-an`, no QtMultimedia); the rendered-result path already demonstrates the persistent-subprocess playback pattern and the QTimer-driven `tick()`. Reuse was therefore chosen over a new architecture.

### Work completed

- `FfmpegFrameSource::open` gained two OPTIONAL parameters: an input seek (`-ss` before `-i`) and an aspect-preserving scale mode. The original three-argument call and its argument list are unchanged.
- `MediaDurationProbe` gained `frameRate()` with a default implementation reporting *unknown* (so existing probes and test doubles stayed valid); `FfprobeDurationProbe` overrides it, parsing `r_frame_rate`.
- `Application` gained a source-playback pipeline (own `FrameSource`/`FramePump`/`Player`), sharing the single event-loop timer which now dispatches by active player; the two playbacks are mutually exclusive and tear down on project lifecycle events.
- Added `SourcePlaybackSourceFactory` as an injectable seam, the four signals, position/duration/interval readouts, and `seekSourcePlayback()`.
- UI: Play/Pause/Stop Source, a seek spinbox + button, and a position readout; `main.cpp` wires the signals, presenting source frames through the existing projection-routed preview path.
- 9 new tests plus an env-gated `realSourcePlaybackIntegration`.

### Verification

- Focused run of the 9 new tests: `Totals: 11 passed, 0 failed, 0 skipped`.
- Targeted regression (source playback, rendered playback, media import/identity, preview/seeking, ffprobe duration, decision/replay/contract): `Totals: 44 passed, 0 failed, 0 skipped` (40.3 s).
- Real-media validation (`realSourcePlaybackIntegration`) against a real 360 proxy: passed (72.9 s) — import, equirect declaration, continuous play, pause, seek to midpoint, viewpoint change, resume, end-of-media, original untouched.

### Failure recovery

- A test-authoring bug, not a product defect, caused two iterations of the real-media test: it called `resumeSourcePlayback()` after a seek that had preserved the *playing* state (resume correctly refuses an already-playing stream), and then asserted *playing* immediately after a seek landing 300 ms before the end, where the stream can end before the assertion. The test now asserts on the ended signal.
- One genuine product fix came out of testing: a seek performed while paused originally left the player in the *Stopped* state (unresumable). `openSourcePlaybackAt` now always enters Playing and pauses again when the open must not run, so a paused seek stays positioned and resumable.
- Environment note (separate from code correctness): a single 4K frame decode on this device measured ~2.6 s, making a validation run against the 779 MB 4K original take ~518 s. Validation therefore uses a 12 s 1280x640 2:1 proxy extracted from the same real footage (original untouched), which is also the reason the playback stream is a bounded proxy. Recorded in `DEVELOPMENT_ENVIRONMENT.md`.

### Boundary notes / not implemented

- Audio: deferred (Decision 036) with the smallest viable follow-up recorded. Playback is video-only.
- No automatic 360 detection or spherical metadata parsing (creator declaration remains sufficient).
- No perception, tracking, camera-path generation, export or parser change.

### Decisions

- Decision 036 recorded (source-playback architecture: separate pipeline, single event-loop driver, bounded proxy stream, seek-by-reopen, frame-rate pacing, audio deferred). Decisions 017-035 preserved.


## 2026-09-18 — Phase 4 Objective 20: Persistent Render Decoding and Deterministic Throughput

### Objective

The human-authorized scope (Decision 037) carried one core requirement: **FFmpeg process creation must no longer scale one-for-one with the number of rendered output frames.** The previous render path asked `FfmpegSeekFrameProvider` for each output frame independently, and that provider spawns one FFmpeg process per request — a real render of N frames created O(N) decoders.

### What was built

- `app/reframe/ReframeStreamFrameProvider.{h,cpp}` (new): a `ReframeFrameProvider` that holds a persistent `FfmpegFrameSource` open at an anchor timestamp and replays forwards from it. A request inside a bounded window is answered by reading the next frame from the already-open stream; only an anchor costs a process.
- The bounded sequential window (`kMaxSequentialSpanMs = 3000`) is the mechanism that keeps the cursor honest: a far-forward jump re-anchors rather than decoding hundreds of frames to reach the target, and re-anchoring also re-aligns the cursor with the true source timeline.
- The positioned seek path is retained deliberately as the fallback and as the geometry-discovery path. An unknown frame rate, invalid input, an unreadable source, the end of the media, or any stream failure falls back to it, so the worst case is the old cost and never a different frame.
- `ReframePipeline` now constructs the streaming provider for real sources, reading the source frame rate once through the existing `FfprobeDurationProbe` seam.
- Both `.pro` files gained the new source and header.

### Verification

- 6 new tests: streaming/seek frame equality across 20 timestamps, process count independent of frame count (60 consecutive requests => at most 3 stream opens, exactly 1 geometry decode), jumps and fallback (backwards, far-forward, unknown frame rate), invalid input and end of source, streaming/seek render equivalence, and frame-source lifecycle safety.
- `reframeRenderEquivalenceStreamingVersusSeek` asserts identical frame counts, identical decoded frames, and **byte-identical MP4 containers** for continuous, trimmed and multiple disjoint temporal segments (148 s).
- Targeted re-run of all six on a fully consistent build: 6 passed / 0 failed / 0 stack smashing.
- Full model-free suite at the checkpoint: **431 passed / 0 failed / 9 skipped** (563 s).
- Core requirement measured with a counting `REELCRAFT_FFMPEG` wrapper on `reframePipelineRendersRealVideoEndToEnd` (real 360 source, 6 output frames): FFmpeg process creations **9 -> 5**, of which exactly one is the render decode stream; the remainder are the test's own fixture encoding and verification decodes. No decoder process scales with the frame count.

### Failure recovery — the two blockers, and what actually caused them

Two blocking defects appeared during this objective: an intermittent `*** stack smashing detected ***` SIGABRT (exit 134) that killed three of the new tests, and a deterministic frame-selection mismatch at the far-forward request t=5500. Both were investigated to root cause rather than worked around.

- The crash was localised to the test binary, not the provider: a standalone media-layer repro (open a stream, optionally read, then close, and destroy without closing) passed cleanly in every case, which exonerated `FfmpegFrameSource` and `QProcess` teardown. An `LD_PRELOAD` interposer for `__stack_chk_fail` was built to capture a backtrace, but by the time it was used the crash no longer reproduced.
- The decisive evidence was artefact-level: between the failing and the passing runs **the only thing that changed was `tests/test_project.o`**, recompiled incidentally when the temporary repro test was added, plus the relink. The provider object and every other object were untouched.
- Root cause confirmed by inspecting the generated makefile: `tests/Makefile` was generated at 11:03:22, before the new header's last modification at 11:17:00, and its rule for `test_project.o` listed `ReframePipeline.h` but **not `ReframeStreamFrameProvider.h`** — even though `test_project.cpp` includes it. Editing the header therefore never recompiled the translation unit that instantiates `ReframeStreamFrameProvider` **on the stack**, so the test binary mixed two layouts of the class; the constructor wrote past the caller-reserved frame and smashed the stack canary, and the adjacent test locals (`FfmpegSeekFrameProvider seek`, the clip path) were corrupted into the spurious t=5500 mismatch.
- Fix: regenerate `tests/Makefile` with `qmake`, after which the same rule lists the header. Verified by rebuilding and re-running: all six tests pass, twice each, no stack smashing. Recorded in KNOWN_ISSUES.md and DEVELOPMENT_ENVIRONMENT.md as an operational requirement, because a stale makefile silently produces binaries that mix object revisions.

### Boundary notes / not implemented

- No change to what is rendered: the streaming path is byte-identical to the seek path, proven at container level.
- No audio, perception, tracking, camera-path generation, auto-reframing, parser, plan, decision-artifact, export or UI change, and no change to the Objective 19 1024x512 playback proxy.
- Suite cost recorded honestly: the focused real-FFmpeg provider tests add roughly 280 s, taking the full suite to about 563 s.

### Decisions

- Decision 037 recorded (persistent render decoding architecture: anchored stream with a bounded sequential window, seek fallback, frame-identity contract, per-source frame rate). Decisions 017-036 preserved.


## 2026-09-18 — Phase 4 Objective 21: Persistent Media Analysis

### Objective

The product-vision investigation found a genuine architectural gap: MASTER_GUIDE's workflow and AI_EDIT_CONTRACT's canonical flow both place **Media Analysis** between source media and creator intent, PROJECT_MODEL section 6 defines analysis as derived data, and REQUIREMENTS section 5 lists the analysis capabilities -- but the Project schema (v3) had no analysis section and the implementation had no whole-video analysis stage. Perception ran per command, over a command-scoped range, evenly sampled, and was discarded.

The authorized objective (Decision 038) was to close that gap with the smallest foundation future capabilities can plug into: the artifact, the lifecycle, the persistence, the invalidation and one real whole-video pass -- and explicitly NOT transcript, diarization, scene understanding, quality ranking, B-roll, an LLM, Creator Memory or multi-source editing.

### What was built

- **`app/analysis/MediaAnalysis.{h,cpp}`** -- a small, closed envelope (addressing, provenance, lifecycle, coverage, confidence) containing named capability layers. Deliberately shaped like `EditDecision`: its own `schemaVersion`, a source reference with a cheap fingerprint, a three-way source status, a specification identity, a SHA-256 `analysisId` over the compact payload, a strict envelope loader, version-retaining serialization, and verbatim preservation of layers it cannot interpret.
- **Seven layer states** with mandatory coverage for any state carrying evidence. `Unavailable` (cannot run here), `Failed` (attempted and errored) and an empty successful result are three different things, and both non-results carry a deterministic reason -- the same discipline already used by `SpeakerAnalysis.available` and the probe seam's "unknown" defaults.
- **Two capabilities.** `technical` is deterministic and model-free, built on the existing ffprobe seam; facts it cannot determine are named in an `unavailable` list rather than defaulted. `targets` reuses `TargetResolver`, `SphericalTargetTracker` and the equirect tangent-view coverage unchanged, and persists normalized spherical observations and tracks -- never provider or view-pixel coordinates.
- **`app/analysis/MediaAnalysisRunner.{h,cpp}`** -- the whole-video pass: one persistent `FfmpegFrameSource` per run at a configured perception resolution, sequential decoding with interval sampling, honest coverage, and a persisted perception/sampling specification. `decoderOpens` is surfaced so the single-decoder contract cannot be violated silently.
- **Project references, not data.** An additive `analysisRefs` section (schema stays 3) holding a small record per media, sufficient only to locate the artifact and verify it still matches the expected source.
- **Shared vocabulary.** `core/MediaSourceReference` now defines the source reference, the `Matches`/`FileMissing`/`FingerprintMismatch` status, the fingerprint comparison and the chunked content digest; `EditDecision` reuses them through type aliases so the two artifacts cannot drift apart. The decision's serialized payload is unchanged.
- **Additive probe extension.** `MediaDurationProbe::streamSummary()` follows the rule `frameRate()` established: a new optional virtual whose default reports *unavailable*, so existing probes and test doubles stayed valid.

### Design decisions taken during implementation

- **The probed duration is the END of the last frame, not a decodable timestamp.** For a 2 s 10 fps clip the frames run 0..1900 ms, so an unclamped scope asks the decoder for a frame that was never encoded. The analysis scope is now clamped to the last usable timestamp, which keeps coverage truthful instead of reporting a sample the stream cannot serve.
- **Coverage means "the span the sampling covers", not "every frame was examined".** The sampling interval is persisted in the layer spec so the difference is explicit rather than implied.
- **Preservation is per layer, strictness is per envelope.** An unknown kind or a newer `layerVersion` is preserved and re-emitted byte-for-byte; a layers entry that is not an object at all is envelope corruption. The boundary was chosen so that no data which could ever be interpreted is discarded.

### Failure recovery

Two focused tests failed on first run, both because of a real defect the tests correctly caught: `resolveMediaAnalysisReference()` reported `ArtifactMissing` as `ArtifactUnreadable`, collapsing "the artifact file is not there" (a routine cache miss) into "the file is there but this build cannot read it". The resolution logic now checks the artifact's existence before loading and the two outcomes are distinct, matching the enum's documented meaning. Both tests then passed unmodified.

Two compile failures occurred and were fixed: a missing `m_config` member on the runner, and the `QCryptographicHash`/`QFile` includes that were still needed by `EditDecision.cpp` after the shared helpers moved to core.

### Verification

- 15 new tests: artifact round trip and identity; schema/version and digest handling; source fingerprint status; seven lifecycle states with coverage; Unavailable vs Failed vs empty; unknown-layer preservation across repeated re-saves; the technical layer on real generated media; the detector-unavailable target layer; spherical track persistence and round trip; specification identity and staleness; single-persistent-decoder whole-video sampling with recorded perception resolution and coverage; truncation to `Partial`; project reference persistence without a schema bump; missing/stale/invalid references being non-fatal; and replay independence.
- Targeted regression across the touched components (EditDecision artifact, replay, Project schema, MediaItem, ffprobe probe, Application persistence): **31 passed / 0 failed / 0 skipped**.
- Full model-free suite run at the checkpoint; results recorded in the Objective 21 checkpoint report.

### Boundary notes / not implemented

- No transcript, diarization, scene/shot segmentation, quality or salience metrics, B-roll reasoning, LLM, Creator Memory, embeddings, generated coverage or multi-source editing.
- No change to `ReframePlan` semantics, camera-path semantics, the renderer, Objective 20's streaming render path, `EditDecision` immutability, replay guarantees or source-media read-only guarantees.
- No new dependency and no ML runtime.
- Recorded limitation: observations made on a sampling grid cannot justify frame-accurate edits; fine-grained placement would need targeted re-analysis, which the coverage model now makes decidable.

### Decisions

- Decisions 038-042 recorded (persistent layered artifact; analysis is evidence not decision; analysis is never on the replay path; coverage-aware independently-available layers; one decode pass at a recorded perception resolution). Decision 043 (Creator Memory) is deliberately NOT recorded: Creator Memory remains a future architectural topic.


## 2026-09-18 — Phase 4 Objective 23: Subject-Follow Camera Paths

### Objective

Resume the primary product objective — 360 footage plus an English instruction to a deterministic rendered result — by finding and closing the next genuinely missing link in the pipeline, from repository evidence rather than assumption.

### What inspection found

Every stage existed and was tested. The parser produced a `ReframeIntent`; the command runner resolved subjects through the replaceable detector/tracker; `ReframePlanBuilder` produced a validated `ReframePlan`; `ReframePipeline` rendered it deterministically. But the **subject trajectory never reached the camera path**: `ReframePlanBuilder` emits exactly one keyframe per camera move from a single resolved direction, so the flagship instruction class — "follow me", "keep me centered" — rendered a locked-off shot. `TargetTrackPlanner::planTrack()`, which converts a track's time-ordered observations into camera keyframes, was implemented and unit-tested and was **called by no production code**.

### What was built

- `ReframeCameraMove` gained `followSubject`, an in-memory flag distinguishing continuous framing from a one-shot aim. `ReframeIntent` is not persisted, so there is no schema or compatibility impact.
- `ReframeIntentParser` sets the flag from the same pattern table that extracts the subject, so classification and subject extraction can never disagree: follow-class patterns are flagged, aim-class patterns are not.
- `ReframeCommandRunner::prepare()` resolves the subject's track id during identity selection and, for a single target-referencing follow move, builds the plan with the existing `TargetTrackPlanner` in place of the builder's single-direction plan. The builder still runs first and unchanged, so it remains the validity gate and the fallback; an unusable track degrades to exactly the previous behaviour with a note explaining why.
- Nothing else changed: no new seam, type, dependency, schema, or renderer/camera-path/playback/replay behaviour.

### Failure recovery (and a genuine finding)

The new real-media test failed three times before the actual cause was established, and each wrong hypothesis was eliminated with evidence rather than by adjusting the test:

1. First failure: the command reported the subject unresolved. A fixture sanity check (decode one frame from the encoded clip and resolve it directly) was added and **passed**, proving decoding and detection were fine and moving the investigation into the command path.
2. The failure message was then made to report the runner's own state, which showed **two tracks** and an honest "'person' is ambiguous across 2 candidate(s)" refusal — not a detection failure at all.
3. A per-frame diagnostic showed the real mechanism: at one sample the same subject yielded **two observations 9.6 degrees apart**, one from each of two overlapping cover views, which exceeds the tracker's default 8-degree merge distance, so two parallel identities were kept and the reference legitimately became ambiguous.

The fixture's subject was also reduced from an 18-degree to a 12-degree cap, because a subject that large straddles cover-view boundaries and widens the two centroids further. Widening `mergeDistanceDeg` for the fixture (its subject is deliberately far larger than a person) restored one identity and the test passes. The perception behaviour itself was **deliberately not changed**: tuning the merge distance for real person-sized targets is Decision 019 territory and needs its own evidence.

### Verification

- 4 new tests: intent follow/aim classification; a model-free moving-subject command producing a monotonic multi-keyframe follow path whose endpoints track the subject, paired with an aim control that stays at one keyframe; an honest fallback when the resolved track has no usable observation inside the range; and a real 360 media end-to-end test (an encoded equirect clip in which the subject walks across the sphere) that runs resolution, planning and the deterministic renderer and decodes the resulting MP4.
- Targeted regression across the affected area (parser, builder, command runner, contract checker, tracker/planner, application command path, and the existing real-video render test): **45 passed / 0 failed / 0 skipped**.
- Full model-free suite run at the checkpoint.

### Boundary notes / not implemented

- Only a single target-referencing follow move takes the new path. Multi-move camera paths, explicit directions, speaker commands and direction-only commands are unchanged, and aim instructions deliberately remain a single fixed direction.
- Recorded limitation: a follow path has at most as many keyframes as the resolver samples (five by default). Smoother following needs denser resolution sampling, which is already a caller/config concern and was not changed here.
- No UI work, no new AI feature, no architecture redesign, no dependency change.

### Decisions

- Decision 043 recorded (follow instructions execute as a camera path through the resolved track; aim instructions stay a static direction; the builder remains the gate and fallback). Decisions 017-042 preserved.


## 2026-09-18 — Phase 4 Objective 24: Follow Trajectory Sampling Density

### Objective

Objective 23 connected the resolved subject trajectory to the camera path; the trajectory itself was still sampled with one budget shared by every subject command. Increase the temporal resolution of the follow path only, without touching perception, thresholds, the planner, the renderer or any persisted schema.

### What inspection found

`ReframeCommandRequest::maxResolveSamples` (default 5) was applied to every subject command through a single call to `deriveTimestamps(range, samples)` in `prepare()`, with an explicit `resolveTimestamps` list as the only override. The budget was therefore already configurable, but not separable by instruction class: raising it globally would have changed aim commands too, both in cost and in the direction they select.

### What was built

- `followSampleIntervalMs` (250 ms) and `followResolveSamplesMax` (24) on `ReframeCommandRequest`, used instead of `maxResolveSamples` only when the command is a follow command (the same `followReference` condition Objective 23 already computes). A small deterministic helper derives the sample count from the range, the interval and the cap.
- Expressed as an **interval rather than a count**, because a count degrades with duration: 25 samples over a 60-second clip is a 2.5-second step, which is the coarseness the objective exists to remove. The cap bounds the cost at 24 decodes per follow command.
- Nothing else changed: no perception threshold, detector, cover-view association, identity semantic, planner, renderer, parser or persisted schema.

### Measurement

On a fixed 4-second fixture whose subject sweeps 80 degrees across the range, with one track:

| interval | cap | samples | observations | keyframes | spacing | span |
|---|---|---|---|---|---|---|
| 1000 ms | 40 | 5 (old budget) | 5 | 5 | 1000.0 ms | 78.7° |
| 500 ms | 40 | 9 | 9 | 9 | 500.0 ms | 78.7° |
| 250 ms | 24 (new default) | 17 | 17 | 17 | 250.0 ms | 78.7° |
| 100 ms | 40 | 40 | 40 | 40 | 102.6 ms | 78.7° |
| 100 ms | 12 | 12 | 12 | 12 | 363.6 ms | 78.7° |

Density maps one-for-one to observations and keyframes; spacing follows the interval exactly; **the angular span is identical at every density**, which is the evidence that this changes temporal resolution and not the trajectory. Model-free cost rose from ~0.5 s (5 samples) to ~2.5 s (17 samples) per command in the synthetic fixture, where detection dominates; the real cost in production is one decoder process per sample and is bounded by the cap.

A second measured effect: sampling density also protects association. A three-timestamp override spaced 2000 ms apart moves the subject 40 degrees per step, past the tracker's 25-degree association gate, and the subject splits into separate identities and resolves as ambiguous; the same trajectory at 1000 ms spacing (20 degrees per step) associates cleanly. Density is therefore not only a smoothing concern.

### Verification

- 1 new test measuring five densities and asserting the observation/keyframe mapping, the spacing, the invariant span, the cap, the follow-only scoping (an aim command over the same range still takes exactly five samples and produces one keyframe) and the explicit-timestamp override.
- Targeted regression across the affected area plus the Objective 23 follow tests and the real-video render test: **46 passed / 0 failed / 0 skipped**.
- Full model-free suite run at the checkpoint.

### Boundary notes / not implemented

- No smoothing, interpolation or easing; no change to `TargetTrackPlanner`, the renderer, the merge distance, cover-view association or identity semantics. The Objective 23 duplicate-identity finding is preserved for its own Decision 019 investigation.
- Recorded follow-up: resolution still opens one decoder per sample when no frame provider is injected. Reusing a persistent decoder is a resolution-seam change and belongs to its own objective.
- The real-media follow test bounds its own density to eight samples, because each sample is a separate decoder process on this device while the shipped default is measured model-free.

### Decisions

- Decision 044 recorded. Decisions 017-043 preserved.


## 2026-09-18 — Phase 4 Objective 25: Persistent Decoder Reuse for Trajectory Resolution

### Objective

Objective 24 raised follow sampling to as many as 24 trajectory samples, and resolution paid one FFmpeg process per sample. Reduce that decode/process overhead without changing sampling behaviour, perception, the planner, the renderer or any persisted schema.

### What inspection found

`ReframeCommandRunner::prepare()` built an `FfmpegSeekFrameProvider` when no provider was injected; every `frameAt` call went through `FrameExtractor::extractFrameAt`, which constructs and destroys a `QProcess` per call. `TargetResolver::resolveSequence` calls it once per timestamp, so a follow command opened up to 24 processes. The cost per invocation is essentially fixed — a 360x180 frame and a 4K frame both cost ~2.5 s — because process start-up and the proot container's syscall cost dominate, so process count, not pixels, was the lever.

Meanwhile `ReframeStreamFrameProvider` already existed as a `ReframeFrameProvider`: one persistent stream per anchor, forward reads, a bounded re-anchor window, a positioned-seek fallback, and frames proven byte-identical to the seek path by Objective 20. `ReframePipeline::renderPlan` already constructs it from a frame rate read through `FfprobeDurationProbe`.

### What was built

- The default resolution provider is now `ReframeStreamFrameProvider`, constructed exactly as `ReframePipeline::renderPlan` does it: probe the frame rate once through the existing seam, then hand over to the streaming provider. Two includes and roughly twenty lines in the command runner.
- No new seam, type, interface or dependency; the replaceable resolution seam is untouched and only its default implementation changed.

### Measurement

With every FFmpeg invocation counted through a recording wrapper, on the same source, timestamps and follow command (six samples):

| provider | processes | resolution time | observations | keyframes |
|---|---|---|---|---|
| positioned seek (previous default) | 6 | 15.2–16.0 s | 6 | 6 |
| persistent stream (new default) | 3 | 9.9–10.1 s | 6 | 6 |

Keyframe count, timestamps and every yaw/pitch value are **exactly** equal between the two paths. The real-media follow test dropped from 37.6 s to 26.8 s. The persistent path's process count is bounded by anchors rather than samples, so the benefit grows with density.

### Failure and cleanup coverage

No new failure mechanism was introduced: the reuse path is the one Objective 20 already tested. `reframeStreamProviderMatchesSeekProvider` (frame equality), `reframeStreamProviderHandlesJumpsAndFallback` (far-forward, backwards, unknown frame rate), `reframeStreamProviderRejectsBadInputAndEndOfSource` (missing input, premature end of media) and `ffmpegFrameSourceLifecycleIsSafe` (opened-but-unread, closed twice, destroyed without close) all pass unchanged, and the new test adds the resolution-level integration: same timestamps, same frames, same plan, fewer processes.

### Verification

- 1 new test; targeted regression of 51 tests including all follow tests, the Objective 20 stream-provider suite, the contract/builder/parser areas, the application command path and the real-video render test: **51 passed / 0 failed / 0 skipped**.
- Full model-free suite run at the checkpoint.

### Boundary notes / not implemented

- Follow sampling behaviour, the explicit `resolveTimestamps` override, aim and direction sampling, subject identity resolution, detector and tracker thresholds, `TargetTrackPlanner`, camera-path semantics, the renderer, the parser, all persisted schemas, injected frame providers and original-media immutability are unchanged.
- Recorded caveat: within a bounded window the stream decodes intervening frames at native resolution. Reasoning says this is still cheaper, because the per-invocation cost is start-up rather than decoding (a 360x180 frame and a 4K frame cost the same per invocation), but it was not measured on a 4K source here.
- Recorded cost: one extra ffprobe call per resolution pass.
- The Objective 23 duplicate-identity finding and any smoothing/framing work remain out of scope.

### Decisions

- Decision 045 recorded. Decisions 017-044 preserved.


## 2026-09-18 — Phase 4 Objective 26: Deterministic Follow Trajectory Smoothing

### Objective

Objectives 23-25 made the follow path connected, densely sampled and cheap to resolve. Improve its quality as camera movement with a deterministic smoothing layer over the already-resolved trajectory, without touching perception, sampling, the renderer or any persisted schema.

### What inspection found

`TargetTrackPlanner` emitted one keyframe per resolved observation carrying the detected direction verbatim, and `CameraPath::stateAt` interpolates linearly between keyframes along the shortest yaw path. So the follow path was piecewise linear through the raw detections: velocity discontinuous at every keyframe, every detector wobble visible in the camera, and denser sampling (Objective 24) only made the wobbles smaller rather than absent.

Placement was settled by two facts. `planTrack` is called **only** by the follow path in `ReframeCommandRunner` — the speaker planner builds its own keyframes — so smoothing there cannot reach any other command. And the planner's stated job is to convert a resolved track into the reframing representation, which is exactly where trajectory shaping belongs; a separate layer would have added an abstraction with one caller.

### What was built

- `TargetTrackPlanner::Config::smoothingWindow` (default 5; 1 disables) and a deterministic, model-free smoothing step applied after keyframe construction and before plan validation.
- The window is **centred and shrinks symmetrically** at the ends rather than being clipped one-sided. That single choice delivers three properties: constant-velocity motion is reproduced exactly (no lag, no flattening), no smoothed value can leave the range of the values averaged (no overshoot), and keyframe times, count and ordering are untouched (start/end timing and span preserved by construction).
- Yaw is unwrapped into a continuous angle before averaging and re-normalised afterwards, so the circular boundary is handled correctly. Pitch is averaged linearly and clamped.

### Measurement

On a synthetic wobble of ±20 degrees per sample the largest angular step between consecutive keyframes falls from **40.0 to 4.0 degrees**. A linear ramp is reproduced to within 1e-9 — the strongest available evidence that the smoothing does not delay or flatten genuine motion — and this is corroborated by the pre-existing `targetTrackPlannerBuildsFollowPlan` test, whose observations are a linear ramp, passing unchanged. A stationary subject is reproduced exactly. A trajectory crossing ±180 degrees keeps every step at 8.0 degrees and every direction beyond 150 degrees, instead of folding toward zero as a naive average of the raw sawtooth would.

### Framing

Investigated and deliberately left centred. The follow path aims directly at the subject, and "follow the person", "keep me centered" and "keep X centered" all mean centred framing; introducing a lead-room or rule-of-thirds offset would change the meaning of those commands. The framing envelope therefore remains the keyframe field of view, and the subject stays inside it because a centred average never moves the camera further from the subject than the raw extremes did.

### Verification

- 4 new tests: jitter versus motion-preservation versus stationary versus short track; yaw wraparound; timing, bounds and determinism (including rendering both the raw and the smoothed plan); and runner-level aim/direction/follow non-regression with a deterministic-repeat comparison.
- Targeted regression including the camera-path wraparound and interpolation tests, the Objective 20/25 decoder tests, the real-media follow pipeline and the application command path: **62 passed / 0 failed / 0 skipped**.
- Full model-free suite run at the checkpoint.

### Boundary notes / not implemented

- Aim, direction, speaker and multi-move commands are untouched; they never reach this planner.
- No change to perception, thresholds, cover-view association, identity semantics, sampling density, decoding, the renderer, the parser or any persisted schema; no new dependency and no model.
- No cinematic behaviour was invented: framing stays centred and no easing, interpolation redesign or lead-room policy was added. Cost is negligible — no decode, detection or FFmpeg work is added.

### Decisions

- Decision 046 recorded, including the contract note that `planTrack` now returns a smoothed trajectory rather than one passing exactly through each observation. Decisions 017-045 preserved.



## 2026-09-18 — Phase 4 Objective 27: Covering-View Duplicate Identity

### Objective

Close the Objective 23 finding that Decisions 043-046 each recorded and deliberately left alone: a single subject seen by two overlapping covering views can be reported with centroids further apart than the tracker's person-tuned merge distance, so the tracker keeps two identities and every reference to that subject honestly refuses as *ambiguous*. Determine the **smallest correct architectural fix** that handles duplicate representations caused by 360 covering views **without** incorrectly merging genuinely separate people.

### What inspection found

The failure was not re-derived from theory; it was reproduced from the real geometry by driving the production covering-view plan, renderer, detector and projection by hand and reading the **raw per-view detections** for a single synthetic subject straddling a view boundary:

| report | yaw | pitch | yawRadius | pitchRadius | source view |
|---|---|---|---|---|---|
| complete | 6.20° | 0.00° | 11.68° | 11.86° | yaw 0° |
| clipped sliver | 15.82° | 0.26° | **1.35°** | 6.95° | yaw 60° |

Separation **9.61°** against a merge distance of **8.0°** — the Objective 23 number, now explained. The covering plan deliberately overlaps its views, so a subject near a boundary is necessarily seen twice, and the neighbour sees a **clipped** silhouette whose box centre is pulled toward that neighbour's own axis. That displacement is a property of where the boundary cut the box, not of where the subject is — and it is unbounded, which is why no fixed distance can absorb it.

The decisive fact was already in the data: `detectionToDirection` reports each detection's angular half-extents. The clipped report's yaw half-extent has collapsed (1.35° against 11.68°). The information that separates "one subject seen twice" from "two subjects" was on the observation; only the merge test was ignoring it.

A diagnostic premise also had to be corrected during the work: setting `mergeDistanceDeg = 0` no longer exposes raw per-view detections once the footprint rule exists (the rule applies regardless), so the reproduction test iterates the covering views itself rather than trying to switch the fix off.

### What was built

- One named predicate in `app/target/SphericalTargetTracker.cpp`: `sameTargetAcrossCoveringViews(a, b, mergeDistanceDeg)` — same target if `separation <= mergeDistanceDeg` **or** if `separation <= a.yawRadiusDeg + b.yawRadiusDeg`. The merge loop calls it instead of the inline distance test; nothing else about merging changed (footprint size is still the survivor tie-breaker; association, ordering, gating and track assignment are untouched).
- The **default merge distance is unchanged at 8.0°**. The alternative — raising the constant to 24°, which was the Objective 23 workaround — was rejected explicitly: it asserts that people are never more than 24° apart, which is false, and it merges separate people everywhere on the sphere to repair a boundary-local displacement.
- Only the **yaw** footprint is used: yaw is the axis along which covering views are laid out and along which the measured clipping occurs. The smallest rule the evidence supports was preferred over a general combined-footprint rule.
- A zero-radius observation cannot trigger the extension, so injected doubles and unit-style observations behave exactly as before.

### Measurement

- The reproduction now asserts both sides: **2 raw per-view detections** (complete at an 11.68° yaw half-extent, sliver at 1.35°; separation 9.61° > 8.0°) and **exactly 1 identity** at the shipped default, at yaw ≈ 6.20° with a footprint above 10° — the complete detection survives, and a freshly constructed resolver reproduces it identically.
- **Non-over-merging is asserted against the fix's own failure mode**: two 3°-radius subjects 12° apart (distinct colours, both labelled a person) stay **2 identities** at the default and still resolve as **2** at ±40°; a single 8°-radius subject resolves as **1**. A contrast block raising `mergeDistanceDeg` to 24° merges the nearby pair, which demonstrates the rejected route is not what is doing the work.
- Follow resolution at the **default** merge distance now succeeds on a moving subject (`ok`, one resolved target, no unresolved references, multi-keyframe valid plan).

### Verification

- 3 new tests: `targetResolverReproducesCoveringViewDuplicate`, `coveringViewMergeDoesNotOverMerge`, `followResolvesWithDefaultMergeDistance`.
- **All four Objective 23 `mergeDistanceDeg = 24.0` workarounds were removed** from the suite (real-media follow, density, resolution-decode, smoothing runner) and those tests pass at the shipped default, so the fix rather than a test setting is doing the work. The only remaining 24.0 is the deliberate contrast inside the new non-over-merge test.
- Targeted regression across perception, tracking, planning, camera paths, the command runner, resolution, the streaming provider and the real-media pipeline: **73 passed / 0 failed / 0 skipped** (240 s).
- Full model-free suite run at the checkpoint.

### Boundary notes / not implemented

- No change to covering-view coverage, fields of view or view counts; no change to projection geometry; no detector change; no association-gate, identity-selection or reference-semantics change; no sampling-density change (Objective 24 untouched); no smoothing change (Objective 26 untouched); no renderer, FFmpeg, decoder or persisted-schema change; no new dependency and no model. Source media stays read-only.
- Nothing was tuned to make the 9.6° case pass: the case is a consequence of the rule, and the rule is the footprint the detector already reported.

### Decisions

- Decision 047 recorded, including the explicit contract change that `mergeNearDuplicates` no longer decides duplicates by distance alone. Decisions 017-046 preserved; the Objective 23 finding recorded in Decisions 043-046 as open is closed by this objective, with their historical text left unchanged.



## 2026-09-18 — Phase 4 Objective 28: Rendered Output Preserves Source Audio

### Objective

Close the last incomplete stage of the priority pipeline. Every stage of *360 source -> understanding -> request -> plan -> virtual camera -> deterministic execution -> flat video output* existed and was tested, and the artifact the last stage produced was silent: `ReframeRenderer::encodeVideo` accepted only a rendered-PNG pattern and wrote H.264 video with no audio input, no audio map and no audio codec, `ReframePipeline::renderPlan` never handed the source path to the encoder, and the decode seams pass `-an`. Objective 14 had already made silence *wrong* rather than merely incomplete: retained `segments` move the output onto a different timeline from the source, so audio must be mapped through the same spans the picture was built from.

### What inspection found

- The gap is structural, not a missing flag: the encoder has no second input at all, and no test observes the absence because every generated fixture is video-only (`generateTestClip`, `createProviderTestClip`, `createEquirectReviewVideo`).
- The information needed to decide *whether* there is audio already exists behind a seam: `MediaDurationProbe::streamSummary()` reports `hasAudio`, `audioSampleRate` and `audioChannels`, and `FfprobeDurationProbe` implements it for the media-analysis technical layer. Nothing new had to be invented or linked.
- `ReframePlan::frameCount()` uses `framesForRange`, which **floors**: a retained span can be longer than the frames it produced, so a naive "sum of the spans" audio would outlast the picture. Measured directly before implementation: a 2333 ms span at 2 fps renders 4 frames (2000 ms of picture) and an unbounded audio produced a **3000 ms container** with an audio-only tail.
- `MediaDurationProbe` already has the established additive-seam pattern (a virtual whose default reports *unavailable*), which is what makes the degradation branches testable with the existing `FakeDurationProbe` double.

### What was built

- **`ReframeRenderer::AudioSpec` + `ReframeRenderer::encodeVideoWithAudio()`** (additive; `encodeVideo`'s signature and body semantics are unchanged apart from extracting the shared process runner). Two passes: the picture is rendered and encoded by the **unchanged** video-only encoder into a temporary file, then remuxed with `-c:v copy` plus an audio filtergraph. The video elementary stream of an output that carries audio is therefore the very stream the verified encoder produced.
- **The filtergraph is plan-driven**: one `atrim` per retained span with `asetpts=PTS-STARTPTS`, ordered `concat`, then `atrim=end=<frameCount/fps>` so the audio ends with the picture. Times are written with fixed six decimals, so the generated command is a pure function of the spec.
- **Audio is always re-encoded** (AAC, 192 kbit/s), never stream-copied: a copy is keyframe-bound and cannot be trimmed to an arbitrary span, and it would tie the output to the source's codec.
- **The source format is preserved from a reported fact**: `-ar` from the probed sample rate, `-ac` only for the unambiguous mono/stereo layouts, so an uncommon layout is preserved by the muxer rather than approximated. Spatial audio is carried as-is and **not rotated** by the virtual camera — recorded as a limitation rather than approximated.
- **`ReframePipeline::renderPlan()` gained an optional `MediaDurationProbe` injection point** (defaulting to the external ffprobe probe, exactly as the frame rate already is). The decision is a fact, never a guess: no audio track -> the previous silent output with no error and nothing reported; the probe cannot answer -> the previous silent output plus an explicit recorded reason in `Result::notes`; audio present but unmappable -> honest failure. The audio map is required (`[aout]`, not `1:a?`), so nothing can silently degenerate into a silent success.
- **Execution notes now reach the caller**: `ReframePipeline::run()` and `ReframeCommandRunner::run()` append their execution notes to their results, so a degradation reason appears in the application outcome instead of only inside the inner call.
- **Nothing persisted changed**: no `ReframePlan`, `CameraKeyframe`, `EditDecision` or `Project` schema change, no new field, no new artifact. Audio is a deterministic function of `(source, plan)`, so the existing perception-free replay reproduces it by re-executing.

### Verification

- **7 new model-free tests** on generated fixtures whose audio is a 440 Hz tone except inside one silent window, so "did the output keep the right audio?" is answered by *where the signal is*: `reframeRenderPreservesSourceAudio` (stream present, channel count and sample rate preserved, container duration matches the picture, tone/silence content, repeated renders equal in decoded PCM and decoded frames, source bytes and mtime unchanged), `reframeRenderAudioFollowsRetainedSegments` (both retained spans sound, the dropped silent middle does not, container is the picture's length), `reframeRenderAudioTrimsToSourceRange` (a late range keeps only its own audio; a range whose frame count rounds down leaves no tail), `reframeRenderSilentSourceStaysSilent`, `reframeRenderUnusableAudioFactsDegradesHonestly`, `reframeRenderLeavesSourceMediaUntouched` (stereo layout preserved, fingerprint via `checkMediaSourceStatus` and content digest unchanged), `replayReproducesRenderedAudio`.
- **The video-only regression is asserted directly, not assumed**: for a source with no audio, the pipeline's container is **byte-identical** to the standalone `encodeVideo` output of the same frames, and no note is recorded.
- **The picture is proven untouched by adding audio**: the same plan and source rendered with audio and rendered with a probe that reports nothing produce an identical **video elementary stream** (copied out of both files without re-encoding) while their containers necessarily differ.
- Targeted regression over the renderer, pipeline, replay, command-runner and real-media follow tests: **20 passed / 0 failed / 0 skipped** (14 s), including `reframeRenderEquivalenceStreamingVersusSeek` unchanged.
- Full model-free suite: **466 passed / 0 failed / 9 skipped** (59.9 s on this container; six minutes faster than the recorded proot figure).
- The 9 skips are unchanged and by design: 8 environment-gated real-media/model integrations plus the child-only fresh-process replay slot. No real 360 clip or helper is configured in this environment, so verification here is model-free and fixture-based, which is stated rather than glossed.

### Boundary notes / not implemented

- Deliberately out of scope: audio editing (mute, volume, fades, mixing, music), any parser keyword for it, transcription/diarization/audio analysis, spatial or ambisonic rendering, in-app audio **playback** (Decision 036's separate follow-up, which needs its own dependency decision), and any change to `ReframePlan`, `CameraKeyframe`, `EditDecision`, the `Project` schema, `ReframeIntent`, the parser, the contract checker, perception, target tracking, camera-path generation, analysis, reasoning or the UI.
- Recorded limitation: the project's real 360 footage is mono, so channel-layout preservation beyond mono/stereo is exercised only by generated fixtures in this environment.
- Recorded limitation: Reelcraft still cannot play audio, so the creator cannot *hear* a result inside the application; the rendered file is where the audio lives.

### Decisions

- Decision 048 recorded (the rendered output preserves the source audio of the plan's retained spans; audio is an execution policy, not a plan field, and the picture keeps the encoder it always had). Decisions 017-047 preserved unchanged.



## 2026-09-18 — Phase 4 Objective 29: Natural-Language Framing (Lens) Control

### Objective

The 360 pipeline could aim and follow the camera but could not change its lens. Give a natural-language reframe instruction control of the framing (`zoom in`, `go wide`, `close-up`, explicit field of view), using the executable representation that already exists rather than inventing one.

### What inspection found

- The gap was structural, not a missing keyword. `ReframePlanBuilder` wrote `frame.fieldOfViewDeg = 90.0` for every keyframe it built, and the other two planners own their own constants (`TargetTrackPlanner::Config::fieldOfViewDeg = 90`, `SpeakerReframePlanner::Config::fieldOfViewDeg = 75`). No instruction could reach any of them.
- "zoom in" was not even *recognized*: `clauseHasCameraKeyword` lists camera verbs and `zoom` is not among them, so the clause was skipped and the instruction reported as unrecognized.
- The executable representation needed **nothing new**: `CameraKeyframe::fieldOfViewDeg` already exists, is already serialized (a round-trip test already covers it), is already interpolated by `CameraPath::interpolate`, is already rendered by `ReframeRenderer` through `EquirectView`, and is already validated to [20, 140]. That is what made this a small objective instead of a plan change.
- Two traps were found by reading the existing parser rather than by testing: `directionFromClause` treats `back` as a 180° turn, so the natural widening phrases `pull back` and `back up` would have produced a turn AND a lens change; and a subject capture is end-anchored and greedy, so `follow me and zoom in` would have captured `me and zoom in` as the subject (the same reason Objective 15 needed to strip a trailing temporal word).

### What was built

- **`ReframeCameraMove::hasFieldOfView` / `fieldOfViewDeg`** — in-memory only, exactly like `followSubject`. No `ReframePlan`, `CameraKeyframe`, `EditDecision` or `Project` schema change and no new persisted field.
- **A deterministic framing vocabulary** in the parser: named absolute levels checked most specific first, explicit numbers (`field of view 60`, `60 degree field of view`, `fov=75`), and a `slightly`/`a bit`/`a little` softener that halves the step toward 90. Matching is whole-word, so `wide` never matches `widescreen` (which is an output aspect) and `closer` never matches a word it merely appears inside.
- **The consumed phrase is removed from the text used for direction detection**, so `pull back`/`back up` change the lens without turning the camera — the same "strip the other half's words before parsing this half" technique Objective 15 established for temporal ranges.
- **Framing composes with direction, subject and the follow class**: one clause carrying framing plus a direction or subject is one move carrying both; a trailing framing clause is stripped from a subject capture for `and` and for a bare comma, so `follow me and zoom in` and `follow me, zoom in` both keep the follow.
- **A framing-only clause becomes a real move** that holds the direction the camera already has (centered forward when the instruction opens with a lens change), which turns `start wide, then push in on me` into a genuine two-keyframe move interpolated by the existing `CameraPath`.
- **The lens persists until it is changed again** in the builder, and each execution path is wired explicitly: the builder per move, the follow path through `TargetTrackPlanner::Config`, the speaker path through `SpeakerReframePlanner::Config` for a single requested lens. A lens **change** on the speaker path is refused through the existing preparation error path rather than rendered at one lens.
- **IPC-4** extends the Objective 18 contract checker: every requested field of view must be *reached* by the executable plan. It is deliberately not equality over every keyframe, because a lens change legitimately starts from the lens the camera already had.
- **An unsatisfiable lens is reported, never clamped**: the intent note names the supported range and the existing plan validator refuses the plan; the applied framing is reported (`Framing: field of view 60 degrees.`) and reaches the application outcome through the existing notes plumbing.

### Verification

- 6 new model-free tests: `reframeIntentParsesFraming` (ladder, explicit numbers, softener, word boundaries, the `pull back` direction trap, subject capture with `and` and with a comma, compound temporal+framing, an unsatisfiable value reported, a two-state lens change); `reframeBuilderAppliesRequestedFraming` (framing-only, direction+framing, the wide→tight path with the mid-path lens checked, lens persistence, the unchanged default, honest refusal); `reframeContractFieldOfViewFidelity` (NotApplicable, consistent, IPC-4 on a dropped lens, containment for a lens change); `reframeCommandRunnerFollowsAtRequestedFraming` (the same trajectory at the requested lens — timestamps, yaw and keyframe count identical — plus a detector-free direction+framing command); `reframeCommandRunnerSpeakerFramingIsHonest` (a single lens honoured, a lens change refused); `reframePipelineRendersRequestedFraming` (two real FFmpeg renders whose decoded pictures must differ, the framing note reported, and an Application command whose stored decision carries the lens and whose replay reproduces the frames).
- Targeted regression over the parser, builder, contract, runner, camera path, follow/sampling/smoothing, planners, pipeline, render equivalence, replay and application-command tests: **73 passed / 0 failed / 0 skipped** (29.6 s).
- Official `scripts/build_and_test.sh`: exit 0, **472 passed / 0 failed / 9 skipped** (64.2 s). The skips are unchanged and by design (8 environment-gated real-media/model integrations plus the child-only replay slot).

### Boundary notes / not implemented

- No change to directions, aim/follow classification, temporal editing, compound composition, source-audio preservation, the planners' own default lenses, perception, target tracking, analysis, reasoning, replay or any persisted artifact. No new dependency, no model, no LLM, no network.
- Recorded limitations: the vocabulary is a fixed ladder plus explicit numbers (no "2x" multiplier, no continuous dial); framing offsets (lead room / rule of thirds) remain deliberately absent per Decision 046; a lens change is not expressible on the speaker path and is refused rather than approximated; multi-subject framing ("keep both of us in frame") is not implemented.

### Decisions

- Decision 049 recorded (natural-language framing is a lens on the existing plan: a framing clause is a camera move, the lens persists until changed, and IPC-4 guarantees a requested lens is reached). Decisions 017-048 preserved unchanged.



## 2026-09-18 — Phase 4 Objective 30: Multi-Subject Framing

### Objective

Make "keep both of us in frame" work: resolve two EXISTING identities and produce one deterministic camera path that keeps both inside the frame whenever the geometry and the supported field of view permit it — without new perception, a new plan type or any persisted-schema change, and without ever dropping, substituting or clamping a subject.

### What inspection found

Three questions decided whether this was ready, and all three were answered from the code before anything was written:

- **Representation already sufficient.** A `CameraKeyframe` carries yaw, pitch, roll and a vertical field of view, and `CameraPath` already interpolates all four. A framing that contains two subjects is a keyframe aimed between them at a lens wide enough for both, repeated over joint observation times. Nothing in `ReframePlan`, `CameraKeyframe`, `EditDecision` or the `Project` schema needed to change.
- **Perception already sufficient.** `TargetResolver` returns every track it resolved; `TargetTrack` reports each observation's angular footprint (`yawRadiusDeg`/`pitchRadiusDeg`) and interpolates with `sampleAt`; `TargetSelector` resolves "me", "the other person" and the canonical "person 1"/"person 2" order deterministically, reporting ambiguity with candidates rather than choosing. No new detector, provider or model was needed — and none was added.
- **The mathematics has an exact answer.** `EquirectView` builds its rays as `forward + right·(ndcX·tanHalfFov·aspect) + up·(ndcY·tanHalfFov)`, so a subject is inside the frame exactly when `|sin t·cos p| ≤ tan(v/2)·aspect` and `|sin p| ≤ tan(v/2)`. Inverting that conservatively gives the framing rule, with no invented threshold.
- One gap was identified and deliberately **not** papered over: `ReframeContract` cannot verify this. Decision 035 established that the plan retains camera coordinates only, so "both subjects are inside the frame" is undecidable from `(intent, plan)`; a cosmetic IPC-5 asserting that two ids resolved would prove nothing. The guarantee is enforced where the identities and footprints exist — in the runner, before planning — and asserted by tests using the exact containment condition.

### What was built

- **`ReframeCameraMove::subjectGroup`** (`None`/`CreatorAndOther`/`TwoPeople`), an in-memory field like `followSubject`; the parser records the GROUP and never the tracks, so resolution happens at command time against current tracks and identity state and nothing is persisted.
- **Plural phrasing** with a required framing verb: "both of us", "us both", "the two of us" (creator + the one other visible person) and "both people", "both persons", "both of them", "the two people", "the two of them" (exactly two visible people). A passing mention of two people is not turned into an instruction, and a plural clause that also carries a direction keeps both so the command can refuse the combination instead of silently dropping half of it.
- **`TargetTrackPlanner::enclosingFramingDeg()`** — pure framing mathematics: yaw unwrapped around the first subject (so ±180 is framed the short way round), spans built from each subject's reported footprint, aim at the midpoint, lens from the conservative inversion of the renderer's basis and the output aspect, renderable limits enforced.
- **`TargetTrackPlanner::planTracks()`** — one keyframe per timestamp where EVERY requested subject was actually observed (nothing interpolated or invented), ONE lens for the whole instruction (the tightest that contains both), and `CameraPath` holding the framing in between.
- **A dedicated runner branch** that resolves the group through the existing selector/identity rules, uses the follow-resolution sampling budget (Objective 24), composes with the Objective 29 lens vocabulary and Objective 14 temporal edits, and refuses every unsatisfiable case with a specific reason.
- **`ReframePlanBuilder`** refuses an unresolved plural move rather than approximating one.

### Verification

- 6 new model-free tests: `reframeIntentParsesMultiSubjectFraming`; `reframeMultiSubjectFramingGeometry` (containment checked with the exact camera basis, pitch-dominated pair, vertical output, ±180 wraparound, impossible pair refused, fewer than two observations rejected); `reframeCommandRunnerFramesTwoSubjects` (two identities, one keyframe per joint observation, one lens, containment everywhere, determinism, requested wide lens honoured, single-target follow unchanged); `reframeCommandRunnerResolvesTwoDetectedPeople` (two really detected people, plus the flagship "keep both of us in frame" through a creator selection and "the other person"); `reframeCommandRunnerRejectsUnsatisfiableMultiSubject` (cannot fit, requested lens too narrow, combination with another camera instruction, three people for "both people", never observed together, one subject unusable in range, unselected creator, plural plus direction); `reframeCommandRunnerFramesMovingSubjectsAndReplays` (a pair walking apart, rendered twice with equal frames, the plan persisted in an `EditDecision` and replayed to identical frames, source bytes/mtime/hash unchanged).
- Targeted regression over the parser, builder, contract, command runner, planners, camera path, selector/resolver/tracker, pipeline, render equivalence, replay, application commands and the Objective 28/29 behaviours: **102 passed / 0 failed / 0 skipped** (39.7 s).
- Official `scripts/build_and_test.sh`: exit 0, **478 passed / 0 failed / 9 skipped** (66.1 s). Objective 29 baseline was 472/0/9; the delta is exactly the 6 new tests, and the 9 skips are unchanged and by design (8 environment-gated real-media/model integrations plus the child-only replay slot).

### Boundary notes / not implemented

- Only TWO subjects are supported. Groups larger than two, dynamic group acquisition and per-subject framing differencing are out of scope.
- The pair path is deliberately **not smoothed**: the existing symmetric smoothing is defined for one direction sequence, and smoothing a per-sample enclosure could move the camera off the framing that guarantees containment. A containment-preserving smoothing is a recorded follow-up.
- Framing offsets/lead room remain deliberately absent (Decision 046); multiplier zoom is not implemented; a pair observed together only once produces a single static enclosing framing; joint framing requires exact timestamp agreement between the two tracks, which is exactly what one resolver pass produces.
- Nothing else changed: no detection, tracking, identity, threshold, sampling-density, smoothing, camera-path, renderer, decoding, audio, temporal-edit, contract-rule, replay or persisted-schema change. No new dependency, no model, no LLM, no network; source media stays read-only.

### Decisions

- Decision 050 recorded (multi-subject framing is one enclosing framing decision on the existing plan; the contract cannot verify containment, so the guarantee is enforced before planning where identities and footprints exist). Decisions 017-049 preserved unchanged.



## 2026-09-18 — Process Objective P1: Development-Management Consolidation

### Objective

Make the development-management layer as scalable as the engineering loop: remove duplicated
documentation, replace stale candidate prose with a capability register, make validation debt
visible, make stable architecture and test relationships cheap to look up, and give batches of
related objectives a controlled execution mechanism. Documentation and policy only — no product
behaviour, schema, semantics or test change.

### What inspection found (measured, not assumed)

- **11,518 doc lines / ~900 KB**; ~200 new doc lines per objective written into 5-8 documents, so
  each milestone was narrated 4-8 times.
- `CURRENT_STATE.md` held **74 objective narrative sections** — exactly one per dated
  `DEVELOPMENT_LOG.md` entry — plus stale Phase 1 boilerplate ("no 360 viewer or reframing system
  exists", "next stage: implement the Phase 1 foundation", project directory `/root/reelcraft`).
- `NEXT_TASK.md` mixed a candidate prose list (stale twice) with per-objective history; the
  actionable content was ~20 lines at the tail of 726.
- No seam index and no behaviour→test index existed anywhere (`ARCHITECTURE.md` never used the word
  "seam"), so each objective re-derived its regression set and re-read stable interfaces.
- Validation debt was invisible: Objectives 28 (rendered audio), 29 (lens control) and 30
  (two-subject framing) are fixture-only, and nothing surfaced that without reading thousands of
  lines.
- `DEVELOPMENT_ENVIRONMENT.md` still claimed the suite runs under `timeout 240` while
  `scripts/build_and_test.sh` uses 900 — a live example of claim drift.
- `AGENT_WORKFLOW.md` already is the canonical policy and already says prompts should reference it,
  so a second protocol document would have added a fourth overlapping governance file.

### What changed

- **`NEXT_TASK.md` rewritten as the operational register** (726 -> 119 lines): priority, current
  objective, a 30-row capability table (status, seam, dependency, validation level, verification
  commit), an explicit **validation debt** section naming the 9 skips and the three fixture-only
  objectives, a candidates table with prerequisites, and blocked/deferred entries each stating the
  missing prerequisite.
- **`CURRENT_STATE.md` rewritten as current state** (1,352 -> 132 lines): version, priority, stage,
  subsystem table, test baseline, validation status, current limitations, environment, rules, a
  "where information lives" map, and a completed-objectives index with commits. The removed narrative
  is authoritatively in `DEVELOPMENT_LOG.md` (verified 74 headings -> 74 entries) and in git history.
- **`ARCHITECTURE.md` gained §23 seam index** (31 seams: implementation, guarantee, consumers) **and
  §24 behaviour→test index** (23 rows mapping guarantees to test groups).
- **`AGENT_WORKFLOW.md` gained §12 controlled batches and gates** (human approves a capability family
  and 2-3 dependency-adjacent objectives; per-objective commit/tests/docs unchanged; a mandatory
  prerequisite gate between objectives with an explicit stop list; autonomy for routine engineering,
  never for product direction) **and §13 checkpoint hygiene**; §4 now points at the test index, §9/§11
  refer to §12, and §10 states the one-home-per-fact rule.
- **`scripts/checkpoint_check.sh`** (new, ~90 lines, no dependencies): build-tree freshness in both
  trees, present-tense script/document value drift, documented-vs-actual test totals, and a decision
  count. It immediately caught the real drift and a missing canonical baseline line.
- **`AI_HANDOFF.md` §3 compressed** (126 -> 61 lines) from a second changelog into orientation:
  priority, pipeline map, the stop-and-report invariants, the four known traps, what is deliberately
  not built, and the build-integrity rule — all pointing at the decisions that own them.
- **Convention notes** added to `DEVELOPMENT_LOG.md` (authoritative record), `PROJECT_HISTORY.md`
  (milestone-level from now on; existing entries preserved unchanged), `CHANGELOG.md` (user-facing
  only) and `DEVELOPMENT_ENVIRONMENT.md` (timeout claim corrected to 900 s, with the historical
  sentence preserved as history).
- **Decision 051** records the policy: one home per fact, the register, the indexes, batches with
  gates, checkpoint hygiene.

### Preserved deliberately

Decisions 001-050 byte-identical (one appended hunk); all per-objective detail retained in
`DEVELOPMENT_LOG.md` and git history; historical numbers and plans inside dated entries left exactly
as they were (including the old 240 s timeout references); no product source, test or schema file
touched.

### Verification

- Documentation-only diff; no `app/`, `tests/` or script-behaviour change other than the new
  `scripts/checkpoint_check.sh`.
- `scripts/checkpoint_check.sh` passes, including against the Objective 30 log.
- Full suite: **478 passed / 0 failed / 9 skipped** — identical to the Objective 30 baseline.

### Decisions

- Decision 051 recorded. Decisions 001-050 preserved unchanged.



## 2026-09-18 — 360 Reframing Objective 31: N-Way Group Framing

### Objective

Extend Objective 30's two-subject framing to deterministic N-way group framing for "the three of us",
"all of us", "the three people" and "everyone", reusing the generalised enclosure geometry where it is
correct — and testing the real spherical camera basis against every target, which is what exposed the
defect below.

### What inspection found

- **The engine was already N-general.** `enclosingFramingDeg` requires only ≥2 observations and
  `planTracks` only ≥2 track ids; the joint-timestamp loop, the per-timestamp enclosure and the
  single-lens rule all iterate every requested track. Nothing in the planner needed to change.
- **The pair restriction lived entirely in the decision layer**: `ReframeSubjectGroup` knew two
  groups, `pluralGroupFromClause` knew "both" phrasings only, and the runner expanded the group into
  a hard-coded pair (`{me, the other person}` or `{person 1, person 2}`) with a `size() != 2` gate.
- **The enclosure rule was an approximation, and the containment test was too permissive.**
  Objective 30 derived the required field of view from the group's yaw and pitch *spans*, and the
  test helper compared direction *cosines*. The renderer's condition is a *tangent* condition, so the
  rule under-frames when a footprint occupies yaw and pitch together, and the helper accepted
  directions outside the frame. The new property sweep reproduced it immediately (six subjects,
  ±13° yaw, ±11° pitch, 1920x1080: a corner escaped the computed 22° lens).

### What was built

- **Count-carrying groups.** `ReframeSubjectGroup` is now `CreatorAndOthers` / `VisiblePeople`,
  with `ReframeCameraMove::subjectCount` (0 = count-free). The vocabulary adds named sizes as words
  or digits ("the three of us", "all three of us", "the 5 of us", "the three people", "three of
  them", "4 people") and count-free forms ("all of us", "us all", "everyone", "everybody", "all of
  them", "all of the people"), most specific phrase first. A size below two is not a group request.
- **N-way resolution in the runner.** The group is expanded through the selector's canonical order:
  the creator (through the existing identity rules, with the creator leading the group) plus the
  other visible people, or the visible people themselves. A named size must be satisfied exactly and
  is refused with its candidates otherwise; a count-free group needs at least two resolvable subjects
  (or, for "of us", at least one other person). Nothing depends on detector or container order.
- **The enclosure rule is now exact in the renderer's basis**: aim at the centre of the unwrapped
  yaw/pitch span, built exactly as `EquirectView` builds its basis (including the degenerate
  fallback), with the required half-tangent taken over **every footprint corner**. Containment holds
  for the whole footprint; a corner at or behind the view plane, or a requirement above 140°, refuses
  with the measured reason; the 20° floor still contains everything.
- **Containment assertions now use the tangent form and the actual reported footprints**, including
  in the Objective 30 tests (two of their assertions assumed a footprint size the detector had not
  reported).

### Verification

- 5 new model-free tests: `reframeIntentParsesGroupFraming` (vocabulary, sizes, precedence, sizes
  below two, passing mentions, single-subject non-regression, the reported note);
  `reframeGroupFramingResolvesCanonicalSets` (three exact, four by digit, "everyone" count-free, the
  creator leading "the three of us" and "all of us", canonical order regardless of input order,
  repeated planning identical); `reframeGroupFramingRefusesHonestly` (count mismatch with
  candidates, the creator family counting the others, no creator selected, a single subject for
  "everyone", a group beyond the renderable maximum, a named lens too narrow plus the same geometry
  accepted at a wide lens, and single/two-subject non-regression);
  `reframeGroupFramingGeometrySweep` (a deterministic sweep over 3-6 subjects, three base yaws
  including ±170, four spreads including 150, two pitch levels including 55°, non-uniform footprints
  and four output aspects — every accepted framing asserted with the exact basis condition against
  every footprint corner, every refusal asserted to name the renderable maximum, both outcomes
  exercised, and the rule shown to be a pure function); `reframeGroupFramingRendersAndReplays`
  (three-subject plan executed through the real pipeline twice with identical decoded frames, wrapped
  in an `EditDecision`, re-loaded and replayed to identical frames, with the source's size, mtime and
  content hash unchanged).
- Targeted regression: **101 passed / 0 failed / 0 skipped** (40.7 s).
- Full model-free suite at the checkpoint (baseline 478/0/9).

### Boundary notes / not implemented

- No schema, persisted artifact, plan type, dependency, provider or rendering change. Single-subject
  and two-subject behaviour, the follow path (with its smoothing), lens control, temporal editing,
  audio preservation and replay are unchanged; the contract still cannot verify containment
  (Decision 050).
- Recorded limits: sizes up to ten; group membership fixed for the instruction; the group path is not
  smoothed; framing offsets remain absent. Explicit subject-reference sets are Objective 32.
- `PROJECT_HISTORY.md` was deliberately not touched: under the Objective P1 convention it records
  milestone-level narrative, and this family's milestone closes when Objective 32 lands.

### Decisions

- Decision 052 recorded (N-way group framing, plus the enclosure rule computed exactly in the
  renderer's basis and the corrected containment assertions). Decisions 001-051 preserved.



## 2026-09-18 — 360 Reframing Objective 32: Explicit Multi-Subject References

### Objective

Let a command name its subjects instead of relying on group phrases ("keep me and person 2 in frame",
"frame the presenter and the guest"), reusing Objective 31's generic N-way resolution and framing path
rather than adding a second multi-subject implementation.

### What inspection found

- The path was already N-general downstream, but the *parser* could carry at most one reference: the
  singular capture takes the first subject in a clause and ignores the rest, so an explicit set had no
  representation at all.
- Three different things are joined by "and" in this grammar (a temporal clause, a lens clause, a
  group phrase), and Objective 31's gate had fixed the detection order. Two concrete traps were found
  by reading rather than by testing:
  - **"follow me and zoom in"** — splitting on "and" without first consuming the lens phrase would
    turn "zoom in" into a subject and break a working command.
  - **"From 0:35 to 1:10, keep the person I selected centered."** — the temporal clause leaves the
    residue "From" behind, which a naive splitter reads as a second reference and reinterprets a
    legitimate single-subject command as a set. This one was caught by the regression suite and fixed
    with a deterministic filler-fragment rule.
- A third trap surfaced in the focused tests: collapsing duplicate references in the *parser*
  ("person 1 and person 1") left one reference, so the clause stopped being a set and the command
  silently fell back to a centered camera — a silent reinterpretation of a multi-subject request.

### What was built

- **`ReframeSubjectGroup::ExplicitSet`** plus `ReframeCameraMove::subjectReferences` (textual
  order, in-memory only). No schema, no persisted artifact, no new identity system.
- **A guarded detector** (`explicitSubjectSetFromClause`) that runs after group detection and before
  the singular capture: it requires a framing verb, consumes the lens phrase and the trailing framing
  words, removes the framing verbs, then splits on "and"/"&"/","/"; a fragment made solely of
  instruction or filler words is not a reference, and fewer than two references means the clause is
  not a set and the existing single-subject path handles it unchanged.
- **Runner resolution for the set**: every reference resolved through `TargetSelector` (creator
  aliases, ordinals, left/right, track ids, unique labels), each failure reported by name with no
  substitution, duplicates de-duplicated by track id, a set of fewer than two distinct subjects
  refused, and the resolved set ordered canonically with the creator leading. The resolved set feeds
  the *same* `planTracks` path, so the exact tangent-containment geometry of Decision 052 applies
  unchanged; the enclosure mathematics was not touched.
- **Parser note** naming the explicit set and its size, and **regression assertions** pinning the
  filler-residue and target-duration guards.

### Verification

- 4 new model-free tests (parser semantics and seven guard cases; canonical resolution with order and
  container independence, three explicit references, label resolution, ±180 wraparound and
  per-keyframe containment of every real footprint; seven honest refusals plus group/mixed-direction
  non-regression; deterministic execution, decision round-trip, replay and source immutability).
- Targeted regression: **105 passed / 0 failed / 0 skipped** (42.9 s).
- Full model-free suite at the checkpoint (baseline 483/0/9).

### Boundary notes / not implemented

- No new group vocabulary, no smoothing, no framing offsets, no dynamic membership, no subject
  preference, no UI, no provider, no dependency, no schema or persisted-artifact change; single-target
  and Objective 30/31 group behaviour are unchanged.
- Recorded limits: the reference vocabulary is the resolver's existing one; a name matching several
  tracks is ambiguous and refused; references longer than a phrase are not treated as a set; a lens
  change inside one explicit set is refused (two lenses are two camera instructions).
- The multi-subject framing family (Objectives 30-32) is now closed; `PROJECT_HISTORY.md` gets one
  milestone entry for it rather than a per-objective entry, per the Objective P1 convention.

### Decisions

- Decision 053 recorded (explicit sets are references resolved at command time, split only inside a
  framing construction; duplicates resolved then de-duplicated; canonical order decides the set).
  Decisions 001-052 preserved.



## 2026-09-18 — Validation Readiness Assessment: Real-Media Sweep for Objectives 28-32

### Objective

Determine whether this RunPod environment can actually perform real-media validation of the 360
reframing behaviour accumulated in Objectives 28-32 (rendered-output audio, lens control, two-subject
framing, N-way group framing, explicit subject sets). Readiness only: no product code, no substitutes.

### What inspection found

- **The documented assets are absent here.** `~/360_TEST_4K.mp4`, `~/.cache/reelcraft/media/` and
  `~/.cache/reelcraft/models/` do not exist, and a filesystem search found no video file larger than
  20 MB anywhere on the root filesystem or the 94 GB workspace volume (which holds only this
  repository). The documentation does identify `360_TEST_4K.mp4` as the project's real footage
  (`DEVELOPMENT_ENVIRONMENT.md`, Decision 023, the Objective 3/5 log entries), so no substitute was
  sought and none was invented.
- **The helper runtimes are absent.** `numpy`, `cv2` and `onnxruntime` all fail to import
  (`ModuleNotFoundError`); apt offers `python3-numpy` and `python3-opencv` but no
  `python3-onnxruntime`; no model weights exist.
- **The environment is otherwise capable**: ffmpeg 6.1.1 with libx264/aac, 8 CPUs, ~1 TB RAM, ~9 GB
  free disk, no GPU (the helpers are documented CPU-only).
- **The gated harness exists and works, but predates these objectives.** Running the 8 `real*` tests
  skips them with their documented messages in 1 ms, verifying the invocation contract. Reading their
  bodies shows they assert Objectives 13/14/19 behaviour only — rendered playback, temporal segments,
  a single-subject "follow person 1", source playback, project reopen — and **nothing** for output
  audio (Obj 28), a requested lens reaching the render (Obj 29), or multi-subject framing, group and
  explicit references, containment against real footprints and honest refusals (Obj 30-32).
- Conclusion: the sweep is blocked twice over — by missing assets **and** by the absence of an
  Objective 28-32 real-media harness. Fabricating either would be exactly the substitution this
  objective forbids.

### What changed

- Documentation only, with measured facts: `NEXT_TASK.md` §4 (validation debt now records the
  two-part blocker, the exact missing prerequisites and the minimum human action),
  `DEVELOPMENT_ENVIRONMENT.md` (new section: AVAILABLE / MISSING / OPTIONAL inventory, the exact probe
  commands, and the invocation contract for the existing harness), `CURRENT_STATE.md` (validation
  status).
- No product code, test, script, schema or dependency change. The model-free baseline stands at the
  Objective 32 checkpoint: **487 passed / 0 failed / 9 skipped** (no code changed, so the suite was
  not re-run merely for ceremony). No real-media validation result is claimed anywhere.

### Boundary notes

- The distinction the objective demanded is preserved: nothing here reports that a code path executed,
  that a renderer produced output, that a real detector produced evidence, or that framing is visually
  acceptable on real footage. **No real-media evidence exists for Objectives 28-32.**
- Validation debt was **not** reduced; it was made precisely actionable.
- No decision entry: no architectural decision was required.

### Decisions

- None. Decisions 001-053 preserved.



## 2026-09-18 — 360 Reframing Objective 33: Real-Media Validation Harness for Objectives 28-32

### Objective

Build the environment-gated machinery that will validate the Objective 28-32 behaviour on real
footage, useful immediately as an executable contract, without performing or claiming that validation
and without touching product code.

### What inspection found

- The existing gated tests (`realDetectorIntegration`, `realUserCommandIntegration`,
  `realApplicationCommandIntegration`, `realTemporalEditIntegration`, `realCompoundCommandIntegration`,
  `realSpeakerCommandIntegration`, `realSourcePlaybackIntegration`, `realReframePlaybackIntegration`)
  establish the conventions worth reusing: read `REELCRAFT_TARGET_*`, skip with the exact variable list
  when unset, guard on `FrameExtractor::isAvailable()`, build `ProcessTargetDetector` from the helper
  script and weights, resolve through `TargetResolver`, order with `TargetSelector::canonicalOrder`,
  write outputs to `REELCRAFT_TARGET_OUTPUT` or a `QTemporaryDir`, and log measured facts with
  `qInfo`. They assert Objectives 13/14/19 behaviour and were left untouched.
- They also hard-code a 12 s proxy window (5500-7500 ms). The harness derives its sample times from the
  probed clip duration instead, so any supplied clip works.

### What was built

Five gated tests plus a small shared prerequisite block (no new framework):

- `realMediaAudioPreservation` (Obj 28) — renders a retained span from an audio-bearing clip and
  asserts the output carries an audio stream with the source's sample rate (and channel layout for the
  mono/stereo cases the policy forces), a duration matching the retained span, and a source whose size,
  mtime and content digest are unchanged. Skips explicitly when the clip has no audio track.
- `realMediaLensRequestReachesOutput` (Obj 29) — renders the same direction with and without a lens
  request and asserts the plan's field of view (90 vs 60 degrees) plus decoded-frame inequality, so the
  evidence is measurable rather than visual.
- `realMediaMultiSubjectContainment` (Obj 30/31) — resolves real person tracks, frames two of them and
  three when the footage provides three, and asserts the **exact tangent containment rule** against the
  detector's own reported footprints at every keyframe, with determinism re-checked. When the footage
  yields only two tracks the 3+ case is recorded as a limitation via `qInfo` instead of being invented.
- `realMediaGroupInfeasibilityIsHonest` (Obj 31) — computes the group's real requirement with the
  planner's own rule and asserts whichever honest refusal that geometry implies: above the renderable
  maximum, or a requested lens too narrow, in both cases with no plan produced. When the supplied
  footage offers no naturally infeasible group it skips and says so, rather than manufacturing one.
- `realMediaExplicitReferencesResolve` (Obj 32) — asserts "person 1 and person 2" resolves to the
  canonical real tracks, that the resulting plan equals the equivalent group phrasing (same N-way path),
  and that a creator seeded from a real track plus a numbered person yields two distinct real tracks.

### Prerequisite contract (verified in this environment)

| Situation | Behaviour (demonstrated) |
|---|---|
| `REELCRAFT_TARGET_CLIP` unset | SKIP naming the variable and pointing at `DEVELOPMENT_ENVIRONMENT.md` |
| clip path does not exist | SKIP "missing media" naming the path |
| detector variables unset | SKIP naming all three variables and the weights directory |
| weights path set but absent | SKIP "missing model weights" naming the path |
| ffmpeg unavailable | SKIP |
| clip supplied but not probeable | **FAIL** with "(invalid media)" — a real problem, not a prerequisite |
| all prerequisites present, execution fails | FAIL (the normal path) |

### Verification

- The five tests compile and skip cleanly with their precise messages (1 ms).
- The four distinct skip branches and the invalid-media failure branch were demonstrated individually
  with temporary environment settings and a scratch non-media file in `/tmp`; no repository file and no
  media file was written or altered.
- Targeted regression over the Objective 28-32 synthetic tests, the harness and the adjacent
  parser/runner/planner/pipeline/replay/application tests: **36 passed / 0 failed / 5 skipped**.
- Official `scripts/build_and_test.sh`: exit 0, **487 passed / 0 failed / 14 skipped** — the same 487
  passes as the Objective 32 baseline and five additional skips, which is exactly the new harness.

### Boundary notes

- No product code, no product behaviour, no schema, no framing geometry, no parser/resolver semantics,
  no audio policy and no dependency changed: the diff is confined to `tests/test_project.cpp` and
  documentation. The five new tests cannot pass without real assets.
- **No real-media validation is claimed.** The Objective 28-32 debt is unchanged and execution remains
  blocked until the documented prerequisites are supplied; what changed is that the sweep is now one
  command away instead of needing a new harness.
- No decision entry: no architectural decision was required.

### Decisions

- None. Decisions 001-053 preserved.



## 2026-09-18 — 360 Reframing Objective 34: Creator Review Foundation (Plan Preview Before Render)

### Objective

Let a creator inspect the structured edit/reframe plan **before** committing to a render, and then
accept or reject it — as a *view* of the canonical plan, with no revision, no timeline editing and no
undo, and with Accept executing exactly the reviewed plan through the existing deterministic render
path.

### What inspection found

- **The decision and the execution were welded together.** `ReframeCommandRunner::prepare` (parse,
  resolve, plan) already existed separately from `run`, but the application only ever called the pair:
  `runReframeCommandInternal` built the request, called `m_commandExecutor`, and the `finish` lambda
  attached the `EditDecision` and appended the record. Nothing in the application could stop between
  the plan and the render.
- **The two seams needed for a safe review already existed.** `appendReframeOutput` is the single
  append gate for render records (shared by the command path and by `replayEditDecision`), and
  `m_replayRenderer` is a plan-plus-source-plus-destination render function defaulting to
  `ReframePipeline::renderPlan`. A reviewed plan therefore needs no new execution path: it is the
  input replay already uses.
- **The valuable invariant was already available**: `ReframePlan::toJsonObject()` is deterministic and
  `EditDecision::fromPlan` hashes it, so "the plan you reviewed is the plan that ran" is a checkable
  statement rather than a claim about intent.
- **The risk was drift between the two paths.** If review built its own request, a reviewed plan could
  differ from the command path's plan in a way nobody would notice (a different effective range, a
  different output path, a different creator seed). Inspection of `runReframeCommandInternal` showed
  the validation/request construction was a clean, self-contained block, so it could be extracted
  rather than copied.

### What was built

- **`ReframePlanReview`** (`app/application/ReframePlanReview.{h,cpp}`): a plain QtCore value type
  derived from a plan by a pure function — instruction and understanding, resolved subject ids, notes,
  the source range and ordered retained spans, keyframe count, start/end yaw and pitch, whether the
  camera moves, whether the lens is constant and its start/end value, output width/height/fps and
  orientation, plus human-readable framing/camera/lens/time/output/audio lines and
  `summaryLines()` ("Label: value", fixed order). The audio line states the renderer's execution
  policy for exactly the plan's retained spans and never claims to know whether the source has audio.
  It carries the canonical plan by value and `digestOf(plan)` (SHA-256 over the plan's compact
  canonical JSON). A review of an invalid plan is invalid and shows nothing.
- **One shared request builder.** `Application::buildReframeCommandContext` is the extracted
  validation + `ReframeCommandRequest` construction, used by the direct command path, the revision
  path and review preparation; `applyCommandResultToOutcome` applies the decision stage's result to an
  outcome for both the command path and preparation. Behaviour is unchanged — verified by the existing
  command tests, which pass unmodified.
- **`Application::prepareReframeCommand(instruction, startMs, endMs)`**: decision stage only, through
  the new `m_commandPreparer` seam (default `ReframeCommandRunner::prepare`, same signature as the
  executor). Renders nothing, appends nothing, writes nothing; emits `reframeReviewChanged(review)` on
  success. A failure appends no record — it never reached an output target — and is reported through
  the existing `reframeCommandFinished` signal with an empty output path, so the reason is visible
  without a second failure channel.
- **`Application::acceptReframeReview(outputPath = {})`**: validates that the reviewed plan's media is
  still the active, available one and that the destination is usable (directory exists, path differs
  from the source), then calls the existing renderer seam with **exactly** the reviewed plan, records
  the result through `appendReframeOutput`, attaches `EditDecision::fromPlan` of that same plan, emits
  `reframeCommandFinished`, and consumes the review either way (a failed render cannot be re-accepted).
- **`Application::rejectReframeReview()`** and the review lifetime: cleared on accept, reject, new
  project, project open, active-media change and removal of the active media, and replaced by a new
  preparation.
- **UI**: a "Review Plan" button, an "Accept & Render" and a "Reject" button (both inert until a plan
  is waiting), and a word-wrapped summary label showing the review's own lines with the plan digest;
  signals `reframeReviewRequested` / `acceptReframeReviewRequested` / `rejectReframeReviewRequested`
  wired in `main.cpp`. The panel displays; it cannot edit.
- **No schema or persistence change.** Review state is session-only; the rendered record is an ordinary
  Objective 16 record, and a project saved before and after a preparation is byte-identical.

### Verification

- 13 new model-free tests (listed in Decision 054) covering: the derived view and its digest, the
  temporal/audio lines, reject, accept rendering the exact reviewed plan, destination validation, an
  honest render failure, a preparation failure leaving no review and no stale review, shared
  validation with the command path, invalidation by every context change, "not a second plan
  representation" (purity, no persistence, no schema change), the unchanged direct command path, the
  record as the only carrier of the reviewed plan, and the panel's inert/enabled states and requests.
- Targeted regression (command path, records, lifecycle, creator selection, playback, replay,
  revision, provenance, touched UI): **87 passed / 0 failed / 0 skipped** (9.1 s).
- Official `scripts/build_and_test.sh`: exit 0, **500 passed / 0 failed / 14 skipped** (79.9 s) —
  the previous 487 passes plus the 13 new tests, with the skip count unchanged.

### Boundary notes / not implemented

- Deliberately **not** built: revising a reviewed plan, a timeline editor, undo/redo, a plan mutator,
  a persisted review, a review of a finished render (Objective 12 already previews those), an editable
  destination field, and any integration of `reviseEditDecision` into the review surface.
- Accept keeps the command path's destination policy and does **not** adopt replay's never-overwrite
  rule: accept continues a command, replay reproduces a historical record.
- The Objective 33 harness was not touched and still skips; **no real-media validation is claimed** for
  this objective, because it adds no perception or rendering behaviour — its validation level is
  fixture by design, and it adds no validation debt.
- No new dependency, provider, plan type or serialized form; the renderer, encoder, audio policy,
  camera geometry, parser, resolver, planner and decision artifact are unchanged.
- `PROJECT_HISTORY.md` was deliberately not touched: under the Objective P1 convention it records
  milestone-level narrative, and the creator-review family is not complete — revision UI is still
  absent.

### Decisions

- Decision 054 recorded (creator review as a read-only view; accept executes the exact reviewed plan
  through the existing render seam and the single append gate; shared command context builder).
  Decisions 001-053 preserved.


## 2026-09-20 — 360 Reframing Objective 35: Creator Revision v1 (Record-Level Revision and Provenance)

### Objective

Make the Objective 17 revision mechanism reachable from the product: a creator looks at a **rendered**
edit, asks for a change in words, and gets a new attributed render — with the "why" of a recorded
decision readable. Four new semantics were formalized first as Decision 055 (fresh sibling
destination, never overwrite the parent, supersession derived at read time, existing
`creator-revision` attribution and parent hash); the mechanism itself is used unchanged.

### What inspection found

- **The mechanism existed and was tested but unreachable.** `EditDecision::revisedFrom`,
  `Application::reviseEditDecision` and `decisionProvenance` were implemented by Objective 17 with 9
  tests, and `grep -rniE 'revis|provenance|lineage|parent' app/ui/ app/main.cpp` returned only Qt
  `QWidget *parent` constructor arguments: no revision and no provenance surface existed anywhere.
- **The destination question was genuinely open.** The command path accepts any output path and will
  overwrite an existing file; replay refuses; `reviseEditDecision` refuses only the *parent record's*
  path. Nothing derived a name, so no surface could offer a safe default.
- **A revision has no persisted parent unless a decision exists**, which is why the objective stayed
  at the record level (pre-render revision of a reviewed plan would need its own semantic).
- **`DecisionProvenance` lived inside `Application.h`**, so a UI readout would have pulled the whole
  Application into a widget header. It was extracted to `app/application/DecisionProvenance.h`
  (mirroring `ReframeCommandOutcome.h`/`ReframePlanReview.h`); nothing else used it.

### What was built

- **`Application::revisionOutputPath(index)`** — the deterministic fresh sibling destination of Decision
  055: `<media base>_reframe_rev<N>.mp4` in the revised record's directory, `N` the smallest positive
  integer that is free.
- **`Application::reviseReframeOutput(index, revisedInstruction)`** — the surface entry point: it checks
  the two classes it owns (unknown index, a record with no usable decision) and then delegates to the
  existing `reviseEditDecision(index, instruction, derivedPath)`, inheriting every Objective 17
  refusal unchanged. It renders nothing itself and adds no new execution path.
- **`Application::revisionsOf(index)`** — supersession DERIVED at read time: the held records whose
  decision names this record's decision as its parent. Nothing is stored; no immutable artifact gained a
  status field.
- **UI**: a revision instruction input, "Revise Selected Render" and "Why This Decision?" actions (both
  acting on the selected render list row and refusing locally when there is nothing to act on), and a
  provenance readout presenting origin, instruction, the revised-from hash **and whether that parent is
  held**, source status, and the plan summary. Wiring lives in `app/main.cpp`; a refusal is surfaced
  through the existing status channel with the application's own reason.
- **No persistence change.** No `Project` schema bump, no new artifact, no status field, no change to
  `decisionHash`, the loader, replay or the single append gate.

### Defect found during implementation

The literal Decision 055 rule ("the smallest N whose path does not already exist") was **insufficient**,
and two focused tests failed on it: a record's output file can legitimately be absent (the creator
deleted the render, or an injected executor wrote nothing), in which case the derivation returned the
record's own path, the revision path correctly refused it, and **every later revision of that record
stalled on the same refusal**. A candidate is now fresh when no file exists there **and** no held record
claims that path. The refinement is recorded as an implementation note appended to Decision 055 — the
original semantics text was left as recorded, and the case that required the refinement is stated.

### Verification

- 8 new model-free tests: `applicationRevisionDerivesFreshSiblingPath` (the derived name, skipping an
  occupied candidate, stability, and no destination for an unusable index or record);
  `applicationRevisionProducesAttributedChildRecord` (attribution `creator-revision` + the parent hash,
  a different decision hash, byte-identical parent JSON and unchanged parent decision, source untouched,
  and provenance resolving the lineage); `applicationRevisionChainPreservesLineageAndSupersession`
  (`_rev1`/`_rev2`, a single parent per step, derived `revisionsOf`, and the absence of any stored
  status key); `applicationRevisionNeverOverwritesAnExistingFile` (the parent render and a pre-existing
  `_rev1` both keep their bytes through two revisions);
  `applicationRevisionRefusesHonestly` (unknown index, empty instruction, record without a decision — no
  render, no append, no file); `applicationRevisionRefusesDriftedOrMissingSource` (the two source
  failure classes stay distinct); `applicationRevisionLineageSurvivesReopen` (the chain reloads
  byte-identically, the next revision continues the sequence, and a project holding only a child reports
  its lineage honestly as unresolved); `mainWindowRevisionAndProvenanceSurface` (local refusals, request
  payloads, and the readout's facts and honest "no decision" state).
- Targeted regression across revision, decisions, replay, records, the command path, the Objective 34
  review path and the touched UI: see the checkpoint record in `CURRENT_STATE.md`.
- Official `scripts/build_and_test.sh`: recorded in `CURRENT_STATE.md`; zero failures, no new skips.

### Boundary notes / not implemented

- Deliberately **not** built: structured/operation-level editing, keyframe or timeline editing, plan-level
  (pre-render) revision, persisted reviews or accept/reject status, any decision-status state machine,
  retention/compaction, and any unification of output-path policy across command/revision/replay.
- The Objective 34 review path, the direct command path, replay, and every persisted artifact are
  unchanged; no new dependency, provider, model or plan type; no real-media claim (the objective adds no
  perception or rendering behaviour, so it adds no validation debt).
- `PROJECT_HISTORY.md` was deliberately not touched (milestone-level narrative only).

### Decisions

- Decision 055 recorded (four revision semantics) with an implementation note recording the refinement
  the focused tests forced. Decisions 001-054 preserved.


## 2026-09-20 — 360 Reframing Objective 36: Revision Safety and Visible Supersession

### Objective

Two narrow, evidence-driven changes to the Objective 35 surface: close a reachable data-loss path in the
revision API, and make the derived supersession that Decision 055 defined actually visible to the
creator.

### The defect, reproduced before it was fixed

`Application::reviseEditDecision` refused only **the parent record's** output path. Revising record 0
while naming **record 1's** output path was therefore accepted: the revision rendered over record 1's
file and the project ended up holding two records claiming the same path, one of which no longer
contained what its own decision said it contains — a render the creator could previously replay,
silently destroyed.

Reproduced first, with distinguishable file contents (each injected render writes its own instruction):
`applicationRevisionRefusesAnotherRecordsOutputPath` failed with *"a revision targeted another held
record's output path"*, and the second record's file content had changed after the revision. Only then
was the fix written; the reproduction is kept as the regression.

### What was built

- **One definition of "a path a render record owns"**: the private helper
  `Application::recordHoldingOutputPath(path)` (index of the held record claiming that path, or -1).
- **The revision path refuses any path a held record claims** (Decision 056), with its own message naming
  the owning record, distinct from the parent-specific refusal whose wording is unchanged. The check is
  on **records, not the filesystem**: a path a record claims is never a valid revision target even if
  that record's file is currently missing, because the record is a historical fact and replay must be
  able to reproduce it.
- **`revisionOutputPath` now shares that helper** instead of carrying its own inline claim check, so the
  derivation and the refusal cannot drift apart.
- **Derived supersession is visible**: the provenance readout states whether a record has been revised and
  by which held records ("superseded: revised by record 1" / "…records 1, 2" / "no later revision"),
  using `revisionsOf()`. This is presentation of Decision 055 point (3) — nothing is stored.

### Boundary notes

- Deliberately unchanged: the general command path still accepts any output path (a separate question
  Decision 056 explicitly does not answer), the derived revision destination, `creator-revision`
  attribution, replay, hashing, the strict loader, the Objective 34 review path, and every persisted
  artifact. No schema, dependency, provider or plan type changed; no real-media claim.
- Decision 056 supersedes exactly one clause of Decision 055 (that the explicit-path form's behaviour is
  unchanged) and preserves the rest verbatim.

### Verification

- 3 new tests (the reproduction/regression, the API-shape test proving a fresh explicit path still works
  and the parent-specific refusal keeps its own message, and the readout's supersession states).
- Targeted regression across revision, decisions, replay, records, the command path, the review path and
  the touched UI; official `scripts/build_and_test.sh`. Numbers are recorded in `CURRENT_STATE.md`.

### Decisions

- Decision 056 recorded (and one clause of Decision 055 superseded, as stated there). Decisions 001-055
  otherwise preserved.


## 2026-09-20 — 360 Reframing Objective 37: Render Destinations Never Overwrite a Recorded Render

### Objective

Close the wider class of defect that Decision 056 deliberately left open — a render writing over a file a
render record owns — and lock the creator workflow end to end with the test that found it.

### The defect, found by the workflow test and reproduced

Writing an end-to-end test of the documented creator sequence (command -> review -> accept -> revise ->
replay) failed on an assertion that looked trivial: *record 0's rendered file still contains record 0's
render*. It did not. **Accepting the reviewed plan wrote onto the same default destination**
(`<base>_reframe.mp4`) and silently replaced record 0's render, leaving two records — and two immutable
decisions — claiming one file whose content matched only the newer one. Replaying the older decision would
then produce a file that disagrees with what the project says that record was.

The same hazard applied to the everyday path: running the same command twice wrote the second render over
the first. Decision 056 had already established the principle for revisions (a path a record owns is never
a valid destination) and had explicitly declined to answer the wider question. This objective answers it.

### What was built (Decision 057)

- **One rule, every render surface**: no render may write to a path a render record the application holds
  owns. Enforced in `buildReframeCommandContext` (the direct command path and review preparation) and in
  `acceptReframeReview`, through the single `recordHoldingOutputPath()` definition introduced by
  Objective 36.
- **An explicitly named destination that a record owns is REFUSED**, with the owning record named —
  refusal rather than silent redirection, because a caller who names a path must be told it is
  unavailable.
- **An unspecified destination is derived FRESH**: `<base>_reframe.mp4` for the first render of a clip
  (the documented default is unchanged in the ordinary case), then `<base>_reframe_2.mp4`,
  `<base>_reframe_3.mp4`, … taking the first name that is neither an existing file nor owned by a record.
  A pre-existing unrelated file on the default name is left untouched.
- **Review acceptance derives its destination at ACCEPT time.** The destination is a filesystem fact, not
  part of the reviewed plan, so choosing it when the creator commits removes the whole "the destination was
  taken while the plan waited for review" class and never forces a re-review (which would re-run
  perception for a filesystem accident). The now-unused `m_pendingReviewOutputPath` member was removed.

### Verification

- The end-to-end workflow test is both the reproduction and the regression:
  `creatorWorkflowEndToEndPreservesInvariants` drives command -> review -> accept -> revise -> replay ->
  save -> reopen with injected seams and asserts, among other invariants, that record 0's file is
  untouched, that the accepted render is EXACTLY the reviewed plan, that replay is perception-free (the
  command executor is never consulted) and reproduces the record's own plan from the record's own source,
  that no earlier record's JSON changes, that the decision chain resolves, and that the whole workflow
  survives a reopen byte-identically.
  `creatorWorkflowSupersessionIsDecisionLevel` records the honest limit that supersession is
  decision-level: a replay carries its source decision, so a later revision of that decision is reported
  against every record carrying it.
- 3 new destination tests:
  `applicationRenderDestinationsNeverOverwriteARecordedRender` (the derived sequence, and an unrelated
  file on the default name left alone), `applicationCommandRefusesAnExplicitPathHeldByARecord` (refusal
  naming the record, nothing appended, file unchanged, and a fresh explicit path still accepted), and
  `applicationReviewAcceptRefusesAClaimedDestination` (refusal on the review surface, the review
  surviving so no re-review is needed, then a derived fresh destination rendering exactly the reviewed
  plan).
- Targeted regression across revision, decisions, replay, records, the command path, the review path and
  the touched UI; official `scripts/build_and_test.sh`. Numbers are in `CURRENT_STATE.md`.

### Boundary notes

- `KNOWN_ISSUES.md` gained the decision-level-supersession limitation found here, and its stale skip
  counts were corrected.
- Deliberately unchanged: replay's own stricter policy (it still refuses any existing file), the revision
  `_rev<N>` sequence, hashing, the strict loader, the renderer/encoder, and every persisted artifact. No
  schema bump, no new dependency, provider or plan type, no real-media claim.
- Visible behaviour change, recorded: running a command twice into one directory now produces
  `…_reframe.mp4` and `…_reframe_2.mp4` instead of one file written twice. The first render of a clip is
  unaffected, which is why almost all existing tests were untouched.

### Decisions

- Decision 057 recorded, superseding exactly the one sentence of Decision 056 that deferred this question.


## 2026-09-20 — 360 Reframing Objective 38: Unreadable Render Records Are Preserved, Not Discarded

### Objective

Implement the resolution already recorded in `KNOWN_ISSUES.md` for a documented data-loss gap: a render
record that a build cannot parse was reported and then **discarded**, so opening and re-saving a project
destroyed it.

### What inspection found

- `Application::restoreReframeOutputsFromJson` counted two failure classes (a non-object entry, and an
  object that fails `ReframeCommandOutcome::readFromJsonObject`) and reported both, but **dropped the
  value in both cases**. Since `saveProject` writes `reframeOutputsJson()` — which rebuilt the array from
  the parsed list — the entry was gone on the next save.
- The project already had the right precedent one level down: `ReframeCommandOutcome::m_rawEditDecision`
  preserves an **unreadable decision** byte-for-byte and re-emits it verbatim, precisely so that
  "opening and re-saving a project can never destroy data this build cannot interpret" (Decision 033).
  The record level simply had not been brought up to that standard; `KNOWN_ISSUES.md` named the fix as
  the planned resolution.

### What was built

- **`ReframeCommandOutcome` can carry a preserved raw entry** (`hasRawRecord()`/`rawRecord()`/
  `setRawRecord()`), with the same explicit `QJsonValue::Undefined` initialization that the decision
  field needs, and `toJsonValue()` returning the entry exactly as persisted (whatever JSON type it had)
  or the record's object form otherwise.
- **`Application::reframeOutputsJson()` serializes VALUES**, so a preserved entry is re-emitted
  byte-identically and in its original position; a normal record is unchanged.
- **Both restore failure paths preserve in place** instead of dropping, and the status message now says so
  ("could not be read and were preserved unchanged; they cannot be replayed or revised") rather than
  claiming they were skipped.
- **A preserved entry is never usable.** It has no output path and no decision, so preview, playback,
  replay, revision, review, provenance and destination claiming all refuse it unchanged — verified rather
  than assumed. It also cannot claim a destination, so it can never block a later render.
- **The render list says so honestly** ("[unreadable] this persisted render record could not be read; it is
  preserved unchanged") instead of showing an empty record.

### Verification

- `applicationPreservesUnreadableRecordsAcrossReopen` (the reproduction and the guarantee): a project
  mixing two usable records with a non-object entry and an object missing its required output path opens
  with all four entries in position; re-saving produces an array whose preserved entries are byte-identical
  and whose usable records still carry their own instruction/output path; a second open/save cycle is
  stable.
- `preservedRenderRecordIsNeverUsedAsARecord`: every execution path refuses the preserved entry
  (destination derivation, supersession, provenance, revision, preview, replay) and nothing is appended.
- `mainWindowListsUnreadableRecordHonestly` pins the list label.
- `restoreReframeOutputsReportsUnrestorableRecord` was UPDATED, not deleted: its intent ("reported, never
  dropped in silence") is preserved and strengthened — it now asserts both entries survive in place, and it
  matches the corrected message wording.
- Targeted regression across record persistence/restore, decisions, replay, revision, review and the
  touched UI; official `scripts/build_and_test.sh`. Numbers are in `CURRENT_STATE.md`.

### Boundary notes / not implemented

- **No new decision was required.** This implements the resolution already recorded for the gap, and
  applies Decision 033's existing preservation principle one level up. No schema bump (the array already
  holds arbitrary JSON entries), no new artifact, no change to what a *usable* record serializes as, and no
  change to the decision-preservation path.
- Deliberately not attempted: repairing or interpreting a damaged record (the whole point is that this
  build must not guess), retention/compaction, and any change to how projects are saved otherwise.
- No real-media claim (no perception or rendering behaviour is involved).

### Decisions

- None required; the recorded `KNOWN_ISSUES.md` resolution was implemented, and both of its statements
  (here and in the revision-boundary-limits entry) were updated from "not retained" to the implemented
  behaviour. Decisions 001-057 preserved.


## 2026-09-20 — 360 Reframing Objective 39: The Render List Keeps the Creator's Selection

### Objective

Fix an interaction defect in the creator surfaces built over Objectives 34-37: the render list is rebuilt on
every change, which discarded the creator's selection, and every action on that list acts on the selected
record.

### What inspection found

- `MainWindow::showReframeOutputs` calls `clear()` and re-adds every row, and `QListWidget::clear()`
  resets the current row. Nothing restored it.
- The list is refreshed on every change (`reframeOutputsChanged`), so after ANY render — a command, an
  accepted review, or a revision — the creator's selection was gone, and the next click on "Preview
  Selected Render", "Play Render", "Revise Selected Render" or "Why This Decision?" reported
  *"No generated render selected."* for a record they had just selected and were still looking at.
- This is exactly the workflow the last four objectives built, which is why it was worth fixing rather
  than filing.

### What was built

- `showReframeOutputs` remembers the current row, rebuilds the list, and **restores the selection when that
  row still exists**. An identical refresh is stable; an appended record leaves the creator where they were.
- When the remembered row no longer exists, the list is left with **no** selection rather than being
  silently pointed at a different record: the actions act on a record, so acting on a different one by
  accident would be worse than acting on none. A refresh with nothing selected invents no selection either.

### Verification

- `mainWindowRenderListKeepsSelectionAcrossRefresh` pins all four cases: append keeps the row, an identical
  refresh is stable, selecting another row is honoured, a vanished row clears the selection, and no
  selection is invented.
- `mainWindowRevisionWorksAfterARefresh` is the defect as the creator experiences it: select record 1,
  let the list refresh (as it does after any render), then "Revise Selected Render" and "Why This Decision?"
  still act on record 1 instead of reporting that nothing is selected.
- Targeted regression over the render list, the revision/provenance/review surfaces and the records they
  read; official `scripts/build_and_test.sh`. Numbers are in `CURRENT_STATE.md`.

### Boundary notes

- Presentation only: no Application behaviour, no persistence, no schema, no destination or decision
  semantics changed, and no new capability. No decision entry was required (this is a defect fix, not a
  semantic choice).
- The provenance readout deliberately keeps describing the record the creator last asked about; it is a
  read-out of that record, not a mirror of the list selection.

### Decisions

- None required. Decisions 001-057 preserved.


## 2026-09-20 — 360 Reframing Objective 40: Creator Lens Widening (Constrained Plan Adjustment)

### Objective

Implement the one constrained plan-level creator adjustment Decision 058 permits: raise every keyframe's
field of view of a PERSISTED render's plan by a target, with everything else unchanged, and record the
result as a new immutable creator-revision decision — preserving containment by construction, reusing the
existing render seam, lineage, destination and append-gate machinery, with no perception and no parsing.

### What inspection found

- **The containment claim was verifiable in the renderer's own terms.** `EquirectView::render` builds a
  pixel's ray as `forward + right·lateral + up·vertical` over an **FOV-independent** orthonormal basis,
  with `lateral = tanHalf·(ndcX·aspect·cosRoll + ndcY·sinRoll)` and
  `vertical = tanHalf·(−ndcX·aspect·sinRoll + ndcY·cosRoll)`, and `forward·direction ≡ 1`. Inverting it
  gives exactly the predicate the planner chooses lenses with (`TargetTrackPlanner::enclosingFramingDeg`)
  and the tests assert (`subjectInsideFrame`): `|a| ≤ tanHalf·aspect ∧ |b| ≤ tanHalf`. Raising `tanHalf`
  only weakens both inequalities, and `CameraPath` interpolates FOV as a convex combination plus a
  monotone clamp, so the property holds at every rendered time, not only at keyframes.
- **Narrowing has no such proof available**: the requirement a tighter lens would have to satisfy is
  computed from footprint corners at build time and is stored nowhere (the plan carries no footprint data,
  and the review retains only directions), so it cannot be checked from a plan at all.
- **The existing framing vocabulary already contains the ladder** (40/60/120/140 by phrase, with 90 as the
  established default), so the widening steps are derived from it rather than invented.

### What was built

- **`ReframePlanAdjustment`** (`app/reframe/ReframePlanAdjustment.{h,cpp}`): the pure transformation
  `FOV_new,i = max(FOV_old,i, T)` with every other field copied verbatim; refusals for a non-finite target,
  a target outside `[20,140]`, a target that widens no keyframe (which is every narrowing request, and an
  already-widest plan), and an invalid input plan; `widestKeyframeFieldOfViewDeg`,
  `wideningLadderDegrees` and `nextWiderLensDeg` for the ladder; and **`isLensWidening(original, adjusted)`**,
  which re-derives the invariants from the two plans and asserts field-by-field (with EXACT comparison,
  because the transformation must copy rather than recompute) that the field of view is the only thing that
  changed and only upward. `widenLens` runs that validator on its own output before returning.
- **`ReframeIntent::framingLadderFieldOfViews()`** (additive): the distinct lens values the parser's own
  table can request, so the ladder cannot drift from the vocabulary a creator's words match.
- **`Application::widenRenderedLens(index, targetFovDeg)`**: validates the record (exists, has a decision,
  source still matches the record), applies the transformation, resolves the record's own media by id,
  derives the existing fresh destination, refuses any destination a held record owns, renders through the
  SAME `m_replayRenderer` seam every render uses, and records the result through the existing
  `appendReframeOutput` gate as a NEW decision built with `EditDecision::revisedFrom`, so
  `origin=creator-revision` and `parentDecisionHash` (both hashed) attest the modification. The parent
  record and its file are never touched. `Application::nextWiderLensFor(index)` exposes the ladder step.
- **One UI action** ("Widen Lens of Selected Render") acting on the selected render, inert without a
  selection, with every other refusal reported by the application in its own words. No narrowing control
  exists anywhere.
- **No schema, artifact, origin value, renderer, decision artifact or replay change.** The instruction
  recorded on the new decision is the parent's verbatim (the plan *descends from* it and was not re-derived
  from it); the parameter of the change is recorded in the record's existing notes, while the FACT of the
  modification is the hashed origin/parent pair.

### Verification

- 7 new model-free tests covering all 22 required behaviours: the pure transformation and its refusals
  (including narrowing and out-of-bounds, and the distinction that a mixed-lens plan IS widened by a target
  that only affects some keyframes), input immutability, the field-by-field validator including eleven
  perturbations that each invalidate it, idempotence, plan validity, the ladder being derived from the
  vocabulary, **containment preservation asserted with the existing tangent predicate at every keyframe and
  at 17 sampled intermediate camera times** (over moving subjects, so the aim genuinely interpolates, with a
  non-vacuity check), attributed child records with the correct parent hash, parent record and file
  unchanged, fresh destinations, replay of the widened decision with perception provably never consulted,
  deterministic repeated execution both within one application and across independent runs, and the UI
  request/refusal behaviour.
- Targeted regression and the official full suite: recorded in `CURRENT_STATE.md`.

### Boundary notes / not implemented

- Deliberately NOT built (Decision 058): narrowing, aim/roll/timing/segment/output-geometry edits, keyframe
  or timeline editing, subject substitution, pre-render adjustment of a reviewed plan, arbitrary plan
  mutation, and any new artifact or origin value.
- IPC-4 is not re-applied (no intent exists to check against); IPC-1..3 remain true because the fields they
  constrain are untouched. This is stated in Decision 058 and the record carries hashed attribution.
- No perception, no parsing, no second rendering path, no new dependency; no real-media claim (the
  objective adds no perception or rendering behaviour, so it adds no validation debt).

### Decisions

- Decision 058 recorded (the 16 approved boundaries). Decisions 001-057 otherwise preserved.


## 2026-09-20 — 360 Reframing Objective 41: Creator Modifications Are Visible in the Creator Surfaces

### Objective

Close a visibility gap created by the revision and adjustment work: the application RECORDS why a plan
differs from its instruction (Objective 35's revision notes, Objective 40's lens-widening note, the hashed
creator-revision attribution), but the creator could not see any of it. In the render list a widened
result looked identical to an original command, and the recorded explanation was readable nowhere.

### What inspection found

- The list built every row from `[ok]/[failed] instruction -> output`, so a record produced by a creator
  modification (origin `creator-revision`, with a parent decision) was indistinguishable from an original
  command unless the creator clicked "Why This Decision?" on that exact row.
- `DecisionProvenance` carried origin, instruction, lineage, source status and a plan summary, but **not
  the record's notes** — and the notes are where a modification explains its parameter ("Lens widened from
  90.0 to 120.0 degrees …"). The explanation was persisted, hashed-adjacent and completely invisible.
- Both facts were already stored, so this needed presentation only: no new field on any artifact, no new
  semantic, and nothing to decide.

### What was built

- **`DecisionProvenance` gained `notes`** (additive, display-only value type), populated by
  `Application::decisionProvenance()` from the record's own `notes`.
- **The render list names lineage**: a record whose decision carries a parent is marked
  `[creator revision of <first 12 hex of the parent decision hash>]`; originals, failures and preserved
  unreadable entries keep their existing labels (an unreadable entry still shows `[unreadable]`). The
  marker is derived from the stored decision, so it survives a project reopen.
- **The provenance readout shows the recorded notes** ("note: Lens widened from 90.0 to 120.0 degrees by
  the creator (Decision 058) …"), alongside the origin, the revised-from hash and whether that parent is
  held, the source status, the supersession and the plan summary. A record without notes shows none.

### Verification

- `mainWindowListsCreatorRevisionsDistinctly`: an original command has no lineage marker, a widened record
  names the parent decision hash it descends from, an unreadable record is still `[unreadable]`, and the
  marker survives save/reopen.
- `mainWindowProvenanceShowsRecordedNotes`: the widened record's readout reports
  `origin: creator-revision`, the resolved parent, the supersession line and the widening explanation; the
  original record's readout reports `origin: command` and no notes; an unavailable view still says so
  honestly; and the view's notes are the record's notes verbatim (no re-derivation).
- `applicationWidenRenderedLensRefusesHonestly` was EXTENDED with the Objective 38 ↔ Objective 40
  interaction: a preserved unreadable record has no decision to widen, offers no ladder step, and is refused
  honestly.
- The pre-existing UI tests pass unchanged, which is the evidence that the row format change did not break
  the established behaviour.
- Targeted regression and the official full suite: recorded in `CURRENT_STATE.md`.

### Boundary notes / not implemented

- Presentation only: no new persisted field, no schema or artifact change, no change to decisions, plans,
  lineage or the append gate, no new signal, and no interactive per-row controls.
- Supersession remains in the provenance readout (it is an Application-level derivation); the list marker is
  the stored `parentDecisionHash`, so the list never claims more than the artifact says.
- No decision was required. No real-media claim.

### Decisions

- None required. Decisions 001-058 preserved.

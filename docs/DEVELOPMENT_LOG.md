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




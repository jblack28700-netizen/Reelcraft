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


## Phase 2 Objective 6 — Project Media Library Management (List & Remove) — Complete

Status: Complete.

Completed the media-management slice begun in Objectives 4–5:

- `Application::removeMedia(mediaId)`: id-based removal of exactly the matching record (requires an active project; deterministic messages; original files never touched).
- `Application::mediaListChanged(const QList<MediaItem> &)`: structured notification emitted when the authoritative list is determined — after a successful appending import, a successful removal, a project open, and a new project (empty). Duplicate imports and failed removals do not emit.
- `MainWindow`: media list widget (`mediaListWidget`, file name + format tag, id carried in item data) and Remove Media action (`removeMediaButton`); Remove with no selection is a no-op with status feedback.
- `main.cpp`: wires `mediaListChanged` → `MainWindow::showMediaList` and `removeMediaRequested` → `Application::removeMedia`.
- `Q_DECLARE_METATYPE(MediaItem)` and meta-type registration support the custom-type signal in tests.
- No schema, dependency, `Project`, `MediaItem`-logic, or viewer-layer changes.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 81 passed, 0 failed (73 prior + 8 new).
- New tests: list emission on import/open/new/remove; removal correctness and persistence; unknown-id and no-project failure paths; removal of an unavailable entry clears unavailable state; MainWindow list population and Remove wiring (with and without selection).


## Phase 2 Objective 7 — Active (Selected) Media — "Viewer Source" Contract — Complete

Status: Complete.

Defined the minimal deterministic "which media is active/selected for viewing" contract; viewer unchanged.

- `Application` owns `m_activeMediaId`; accessors `activeMediaId()` and `activeMediaItem()` (resolves to exactly one current record; nullptr when none/unresolvable).
- `setActiveMedia(mediaId)`: active-project requirement, id membership, and file availability (`referenceExists()`) validated; idempotent when already active; deterministic messages through `backgroundCompleted`.
- `activeMediaChanged(mediaId)` signal emitted only on actual change (empty = cleared).
- Consistency rules: import never auto-selects; removing the active record clears it (one emission); removing others preserves it; new project clears it; open normalizes the media list first, then restores the persisted id only when it resolves to a current, available record — dangling/invalid/unavailable ids are cleared deterministically (no dangling active reference, invariant enforced).
- Persistence: optional additive top-level `activeMediaId` project JSON key (viewerState pattern) — written only when set, read leniently; no schema-version bump, no migration.
- MainWindow: `setActiveButton` (acts on list current row; no selection → status), `activeMediaLabel`, `▶` marking of the active row, `showActiveMedia` slot, `setActiveRequested` signal; wired in `main.cpp`.
- Media files never touched; media identity/dedup/availability/ordering/persistence and all viewer/presentation code unchanged.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 90 passed, 0 failed (81 prior + 9 new).
- New tests: set/clear semantics + idempotency; import never auto-selects; removal rules; new-project clearing; save→reopen round trip; open-time normalization (dangling/legacy/duplicate persisted ids); unavailable media cannot become active; MainWindow Set Active/label/marking.


## Phase 2 Objective 8 — Equirectangular Frame Presentation Foundation — Complete

Status: Complete.

Established the smallest safe, deterministic, decode-free pixel presentation path: equirectangular image viewed through the existing authoritative ViewportState camera, consistent with ViewerProjection conventions.

- `app/viewer/EquirectView.{h,cpp}`: deterministic CPU renderer `render(source, yaw, pitch, roll, fov, outW, outH, out) -> bool`. Same conventions as ViewportState/ViewerProjection (identity → FRONT centered; +90° yaw → RIGHT; −90° → LEFT; ±90° pitch → UP/DOWN; positive roll rotates content counter-clockwise; vertical FOV [20,140]); nearest-neighbor sampling; rejects invalid input (empty source, non-positive output, non-finite camera values, |pitch| > 90, FOV outside [20,140]) without partial output mutation. `MaxOutputWidth = 640` is a documented implementation/performance safeguard, not an architectural limit.
- `ViewerWidget` optional source-image path: `setSourceImage`/`hasSourceImage`; valid source renders through the current `ViewportState` camera (capped, aspect-preserving); absent/cleared source falls back to the existing marker-scene path (protected regression contract, opt-in only). Widget stays presentation-only and camera-state-free.
- No Application, Project, MediaItem, activeMediaId, media-library, or schema changes; no decoder/media-engine contract; no dependencies.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 100 passed, 0 failed (90 prior + 10 new).
- New tests: identity/yaw/pitch anchors; positive-roll direction verified against the ViewerProjection convention (quadrant analysis); FOV coverage change; invalid-input rejection incl. no-partial-mutation; pixel-for-pixel determinism; ViewerWidget source-image integration; clearing source restores scene rendering; CPU performance sanity.
- Performance sanity: EquirectView CPU render ~14 ms/frame at 640×320 (recorded).


## Phase 2 Objective 9 — Active-Media Frame Presentation — Complete

Status: Complete.

Closed the final Phase-2 gap: decode a single real frame from the active media record and present it through the verified equirectangular pixel path. Approved decode approach: FFmpeg-CLI adapter (Decision 015).

- `app/media/FrameExtractor.{h,cpp}`: isolated QtCore seam invoking the external `ffmpeg` CLI via QProcess (never linked) to decode one frame (`-frames:v 1 -f image2 -c:v png pipe:1`) parsed to a QImage. Executable resolved via `REELCRAFT_FFMPEG` override then `QStandardPaths::findExecutable`; deterministic errors (unavailable executable, missing/invalid file, start/timeout/exit failure, empty/undecodable frame); no partial output; media file never modified.
- `Application::previewActiveMediaFrame()`: requires an active project, active media record, and available file; success emits `framePreviewReady(QImage)` + status; deterministic failures via `backgroundCompleted`.
- `MainWindow`: Preview Active Frame button (`previewFrameButton`) → `previewFrameRequested`; `showFramePreview(QImage)` → `viewerWidget()->setSourceImage` (viewer pixel path unchanged).
- `main.cpp` wiring; Decision 015 recorded in `DECISIONS.md`.
- No schema, dependency, media-contract, or viewer changes; no playback/streaming.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 107 passed, 0 failed (100 prior + 7 new; 0 skipped — ffmpeg available).
- New tests: FrameExtractor invalid-input rejection; availability + single-frame decode of a real PNG (exact dims/colors); deterministic repeatability; Application guards (no project / no active media); preview emits correct frame; Preview button signal; end-to-end decode→viewer center (identity camera centers FRONT in the decoded equirect test pattern). Decode tests QSKIP gracefully when ffmpeg is absent.


## Phase 2 Objective 10 — Active-Media Time Navigation — Complete

Status: Complete.

Added on-demand single-frame time navigation (stepping/seek) through the active media record.

- `FrameExtractor::extractFrameAt(filePath, execPath, seconds, out, error)`: FFmpeg-CLI PNG-pipe decode with fast input seek (`-ss <seconds> -i …`); `extractFirstFrame` delegates at 0.0 s (contract preserved). Negative/non-finite times rejected deterministically. Requested position is a seek/preview request, not a guarantee of exact frame/presentation-timestamp accuracy.
- `Application`: session-only `m_previewTimeSeconds` + `previewTimeSeconds()`; `previewActiveMediaFrameAt(seconds)` (negative→0); `stepActiveMediaPreview(delta)` (floor 0); private `decodePreviewFrameAt(target)` with the Objective 9 guards; position updates only on successful decode; `previewTimeChanged(double)` emitted only on actual change; position resets (with emission) on new project, open, active-media change, and active-media removal. Beyond-end/undecodable requests fail deterministically leaving the position unchanged.
- `MainWindow`: Step −1 s / +1 s buttons → `previewStepRequested`; `previewTimeLabel` + `showPreviewTime`; existing Preview Active Frame decodes at the current position.
- `main.cpp` wiring for the two new connections.
- No schema, linked-dependency, viewer, media-contract, or ffprobe changes; original media never modified.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 116 passed, 0 failed (107 prior + 9 new; 0 skipped — ffmpeg available; suite ~17 s incl. generated video fixtures).
- New tests: invalid seek-time rejection; frame-at-time extraction (0 s red-dominant vs 2 s blue-dominant on an all-keyframe 1 fps fixture); seek determinism; Application guards; position update + single time emission; same-position re-request emits no time change; step advance and below-zero clamp; beyond-end failure deterministic with position unchanged; position resets (new project / active change / active removal); step buttons and time readout.


## Phase 2 Objective 11 — Pointer-Based Viewer Orientation Control — Complete

Status: Complete.

Added the primary 360-viewer pointer gestures, routed through the existing authoritative ViewportState and Application adjust slots.

- `ViewerWidget` pointer handling (Objective 11): left-button drag emits `viewportYawDeltaRequested` (drag right = yaw increases) and `viewportPitchDeltaRequested` (drag up = pitch increases) at a deterministic 0.25 °/px; mouse wheel emits `viewportFovDeltaRequested` (wheel up = −5° = zoom in; wheel down = +5° = zoom out per 120-unit step); non-left drags ignored; no modifiers/inertia/multi-touch gestures.
- New `ViewerWidget` delta signals wired in `main.cpp` to the existing Application adjust slots (same receivers as keyboard controls).
- Sensitivity constants documented as deterministic defaults; widget never owns or mutates viewport state.
- No rendering, media, decode, time-navigation, schema, persistence, or dependency changes.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 120 passed, 0 failed (116 prior + 4 new; 0 skipped).
- New tests: drag emits expected yaw/pitch deltas; non-left drags and release-without-move emit nothing; wheel-up decreases FOV / wheel-down increases FOV; wired drags update Application ViewportState deterministically (incl. pitch clamp).


## Phase 2 Objective 12 — Projection Declaration & Flat-Media Preview Path — Complete

Status: Complete.

Made source projection an explicit, creator-declared per-media property with a correct undistorted flat preview; equirectangular path unchanged.

- `MediaItem::Projection { Unknown, Equirectangular, Flat }`: optional additive `projection` JSON key (`"equirectangular"` | `"flat"`; absent/unrecognized ⇒ Unknown; serialized only when declared; no schemaVersion bump/migration).
- `Application::declareMediaProjection(mediaId, projectionValue)`: project/id/value guards; updates the record; re-emits `mediaListChanged`.
- `ViewerWidget::setFlatSourceMode(bool)` (default false): flat mode draws the source aspect-preserving, centered, letterboxed on the existing background with NO camera/equirectangular transformation (drag/wheel harmless); equirect and marker-scene paths unchanged.
- `MainWindow`: projection role data per row; `showFramePreview` routes by active row projection (flat ⇒ flat mode; else equirectangular); Mark Flat / Mark Equirect controls on the selected row; wired in `main.cpp`.
- Unknown projection routes to equirectangular (backward-compatible default), documented and tested.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 128 passed, 0 failed (120 prior + 8 new; 0 skipped).
- New tests: projection default/parse semantics; projection JSON round trip incl. unknown fallback; Application declaration guards/validation/emission; declared projection persists across reopen; flat mode letterbox/center; flat mode ignores camera transforms; projection buttons emit requests; preview routing honors declared projection (flat vs equirect center anchors).


## Phase 2 Objective 13 — Viewer Presentation State Consistency — Complete

Status: Complete.

Made the viewer's on-screen content deterministic with Application context changes.

- `MainWindow::showProject` (project changed: new/open) clears the viewer source and flat mode (stale decoded frame from the previous project removed).
- `MainWindow::showActiveMedia` clears when the active media id changes (including cleared to none); same-id re-announcement does not clear.
- `MainWindow::clearViewerSource()` returns the viewer to the deterministic marker scene (`setSourceImage(QImage())`, `setFlatSourceMode(false)`).
- Preview/step and flat-vs-equirect routing are unchanged; Application/viewer contracts unchanged.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 131 passed, 0 failed (128 prior + 3 new; 0 skipped).
- New tests: new project after a preview clears the frame and flat mode (marker scene visible); active-media switch and active-media removal clear; same-active re-announcement preserves the presented frame.


## Phase 2 Objective 14 — Presentation Quality: Bilinear Equirectangular Rendering — Complete

Status: Complete.

Replaced nearest-neighbor with deterministic bilinear equirectangular sampling.

- `EquirectView::render` samples bilinearly (four-texel straight-space blend with alpha; horizontal seam wrap; vertical pole clamp; integer-rounded, deterministic).
- All camera/yaw/pitch/roll/FOV conventions, validation, and no-partial-output semantics preserved; output format unchanged; flat mode and all other subsystems untouched.
- `MaxOutputWidth = 640` remains unchanged and non-configurable (cap/configurability deferred).
- Performance measured: bilinear 36.98 ms/frame at 640×320 (nearest was ~15–23 ms); informational 148.69 ms at 1280×640 (supports keeping the cap).
- Interaction-latency note: per-paint render cost rises (~37 ms); drag/wheel re-render per event — render caching/invalidation or a GPU path are follow-up candidates, explicitly not this objective.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 133 passed, 0 failed (131 prior + 2 new; 0 skipped).
- New tests: bilinear quarter-blend anchor at texel (1.5,1.5) on a 4×4 source (proves interpolation, not nearest); seam (±180°) and pole (±90°) robustness and determinism. All prior EquirectView/viewer/presentation tests pass unmodified.


## Phase 2 Objective 15 — Real-Media Review Path Validation — Complete

Status: Complete.

Validated the full Phase-2 review path end-to-end against deterministic FFmpeg-generated equirectangular multi-frame media (no physical camera).

- Test fixture generator: 30 s, 1 fps, all-keyframe 2:1 equirect video with deterministic time-varying FRONT content (red/green/blue per-frame cycle) and a fixed RIGHT marker (magenta), built with the existing FFmpeg-CLI pattern (Decision 015; QSKIP if ffmpeg unavailable).
- Focused end-to-end validation: fixture frame distinctness and seekability; import → set active → preview (t=0 red) → step +1 (green) → look-around at +90° yaw (magenta RIGHT centered) → seek t=5 (blue) with position tracking and viewer state — plus prior state/routing guards.
- Informational performance (no gate): single-frame decode ~633 ms/step (subprocess), per-paint render ~41 ms at 640×320 — recorded as interaction-latency data for future caching/engine decisions.
- Environment limitation recorded: synthetic-but-real encoded media is the review proxy; hardware 360-camera validation is out of scope in this environment.
- Approved dev-script change: test-runner timeout raised 20 s → 240 s (FFmpeg fixture suites exceed the old cap).

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 136 passed, 0 failed (133 prior + 3 new; 0 skipped; suite ~26 s).
- New tests: equirect fixture frames distinct and seekable; end-to-end review path on the equirect clip; informational performance. No production source changed.


## Phase 2 Milestone Definition — "360 Viewer Review & Navigation" — Recorded

Status: Recorded (2026-09-06, Decision 016).

Phase 2 is officially scoped as "360 Viewer Review & Navigation": import and manage real media, select active media, deterministic single-frame preview, time stepping/seek, look-around orientation (keyboard and pointer), correct flat/equirectangular presentation, and consistent viewer/project state. Continuous/paced playback, duration-aware transport, audio, and the future production media/player engine are explicitly deferred to a future playback/media-engine phase (see `DECISIONS.md` Decision 016 and the updated MASTER_GUIDE roadmap). Documentation-only change; no source/test/schema/dependency changes.

## Phase 2 Formal Completion

Status: Complete — 2026-09-06 (Phase 2 closeout commit; verified at the closeout commit).

Phase 2 — 360 Viewer Review & Navigation (Decision 016) is formally complete.

Verified Definition-of-Done evidence:
- Objectives 1-15 complete and verified (media import/management/selection, availability, active-media contract, equirectangular presentation with deterministic bilinear sampling, flat-media routing, keyboard and pointer orientation controls, single-frame decode, time stepping/seek, viewer-state consistency, real-media review-path validation).
- Automated test suite: 136 passed, 0 failed, 0 skipped.
- Application and test builds succeed.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Working tree clean at the closeout checkpoint.
- Clean-checkout verification of the closeout commit: build + tests pass (136/0/0) and offscreen smoke succeeds.
- Decision 016 scope satisfied: no continuous playback, duration metadata, ffprobe probing, audio, or media-engine implementation exists.
- No production source, test, script, schema, dependency, or architecture changes were made by this closeout (documentation only).

Phase 3 (per Decision 016 / roadmap Phase 3 - Media Engine) has not started; the next step is a Phase 3 opening decision/discovery, not implementation.

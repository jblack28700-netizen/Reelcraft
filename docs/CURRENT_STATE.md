# Reelcraft — Current State

## Current Version

0.2.41

## Current Branch

main

## Current Stage

Phase 4 — 360 Reframing Engine (deterministic vertical slice), Target/Subject Resolution, real detector integration, and structured target identity/selection implemented and verified. Phase 3 Media Engine remains open; its player-lifecycle objective is intentionally superseded for now by the human-approved 360 priority.

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

Status: Partially implemented.

A replaceable decode/media-source seam, a persistent FFmpeg-subprocess frame source, a deterministic frame pump, and a player/timing foundation exist (Phase 3 Objectives 2-4). A deterministic 360 reframing engine (structured plan, camera path, provider seam, renderer, pipeline) now renders flat video from 360 sources. No continuous playback UX, duration/ffprobe metadata, audio, timeline, or Application-level player lifecycle wiring exists yet.

## AI Editing

Status: Partially implemented.

The conceptual AI-to-deterministic edit contract is now backed by a concrete structured boundary for 360 reframing: a deterministic natural-language `ReframeIntent` parser produces structured decisions, a validated `ReframePlan` carries them, and a deterministic renderer executes them. No AI provider/model, scene understanding, target detection/tracking, speaker analysis, or content-based cut selection is integrated yet; unresolved subject references are reported rather than guessed.

## 360° Video

Status: Partially implemented — deterministic reframing vertical slice complete.

360° video is a first-class requirement. The viewer supports equirectangular presentation and look-around; media records carry a declared projection; and a deterministic reframing engine now turns a 360 source plus a structured (or natural-language) request into a flat H.264 output through the `ReframePlan`/`CameraPath` contract. Target/subject resolution is implemented behind a replaceable detector seam (tangent-view detection reprojected to spherical directions, deterministic identity tracking, and a subprocess model adapter). A real detector (OpenCV Zoo YOLOX, Apache-2.0) now runs through that seam on real 360 footage and has produced a flat output centered on a detected person. A structured creator identity (`app/target/TargetIdentity.*`, `TargetSelector.*`) now binds "me" to a selected person and resolves multi-person references deterministically; it is geometric identity, not biometric, and speaker localization is not solved. A real model is required for detection (given the clip/helper), and reframing is not yet wired into the UI.

## Testing

Status: Implemented and continuously verified.

A Qt Test suite covers project state, viewer/media, the media-source seam and frame pump, player/timing, the 360 reframing engine (plan validation/round-trip, camera interpolation, rendering determinism, intent parsing, plan building, and a real FFmpeg end-to-end render), and target resolution (equirect/view geometry, seam and pitch boundaries, view coverage, deterministic tracking, resolver behavior, the subprocess detector protocol, the track planner, and a model-free detection -> plan -> render path). Current result: 240 passed, 0 failed, 1 skipped (~42 s). The single skip is the real-detector integration test, which runs only when a detector helper, model, and clip are configured; the normal suite stays model-free.

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


## Phase 3 — Media Engine — Opening Decisions Recorded

Status: Open (decisions recorded 2026-09-06, Decision 017). Phase 2 remains formally closed.

- Phase 3 opening architecture (approved): persistent FFmpeg streaming subprocess behind a replaceable media/player seam — the currently feasible implementation path in this environment.
- FrameExtractor remains the deterministic single-frame preview/validation seam (not the permanent continuous-playback engine).
- Linked FFmpeg libraries and QtMultimedia: deferred (no installation; revisit only at a real desktop/deployment dependency decision).
- ffprobe duration metadata: reopened for Phase 3 as its own future decision gate (not implemented).
- Objective 10 preview-time position contract preserved; playback extends rather than replaces it.
- Architecture boundary: decode/media-source seam → player/timing (playhead, state, rate, pacing, clock) → Application (active-media state, player-lifecycle orchestration) → Viewer (presentation); timeline/editor/AI remain above Application and never reach into decoding.
- No media-engine/playback/duration/audio implementation exists; no dependencies installed or changed; documentation-only changes in this objective.


## Phase 3 Objective 2 — Persistent FFmpeg Streaming Feasibility Probe — Complete

Status: Complete.

- Tests-only feasibility probe (no production code): a persistent FFmpeg subprocess streams 25/25 frames of a deterministic equirect clip (rawvideo), reaches EOF cleanly (exit 0), errors deterministically on a missing file, and supports clean terminate/kill then restart.
- Informational measurement (no gate): ~39.9 frames/s unthrottled delivery at 160x80, ~1.0 ms inter-frame delivery latency — evidence that a persistent-subprocess frame path is CPU-plausible in this environment; rawvideo/transport details intentionally not decisions (Decision 017).
- No dependency, ffprobe, schema, Obj-10 contract, FrameExtractor, or production changes.

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 139 passed, 0 failed (136 prior + 3 probe tests; 0 skipped).

## Phase 3 Objective 3 — Replaceable Media-Source Seam & Deterministic Frame Pump — Complete

Status: Complete (2026-09-16).

- Introduced the replaceable decode/media-source seam `FrameSource` (`app/media/FrameSource.h`): a transport-agnostic abstraction that delivers decoded frames as `QImage` values and reports a distinct `ReadResult` (Ok / EndOfStream / Error / Timeout) with bounded waits. Opening is implementation-specific and deliberately excluded from the abstract contract.
- Introduced `FfmpegFrameSource` (`app/media/FfmpegFrameSource.{h,cpp}`): the persistent-subprocess implementation proven in Objective 2. It streams `rawvideo`/`rgb24` frames from the FFmpeg CLI, scales to caller-supplied geometry (ffprobe geometry discovery remains deferred), fails deterministically on missing/zero geometry, missing files, or a failed process, and guarantees process cleanup on `close()`.
- Introduced `FramePump` (`app/media/FramePump.{h,cpp}`): a small deterministic consumer. One explicit `advance()` requests exactly one frame and emits `frameReady` / `streamEnded` / `streamFailed`; it has no timer, no playback clock, no pacing/rate policy, and no worker thread. Callers drive it.
- The seam is replaceable and directly testable: tests drive `FramePump` from an in-memory `FrameSource` test double with no FFmpeg subprocess, proving every `ReadResult` branch deterministically.
- Objective 10 preview-time contract unchanged; no Application/UI playback behavior, no continuous/paced playback, no duration metadata, no audio, and no schema/dependency changes. `FrameExtractor` remains the single-frame preview/validation seam.
- New source files are compiled into the production application build (`reelcraft.pro`) and the test build (`tests/tests.pro`).

Verification:
- Application build succeeded.
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Test build succeeded.
- Automated tests: 144 passed, 0 failed, 0 skipped (139 prior + 5 seam/pump tests).
- Official `scripts/build_and_test.sh` workflow re-run green.

## Phase 3 Objective 4 — Player/Timing Subsystem Foundation — Complete

Status: Complete (2026-09-16).

- Introduced the player/timing subsystem (`app/playback/`) above the Objective 3 `FramePump`, preserving the architecture boundary decode seam → player/timing → Application → Viewer.
- `Playhead` (`app/playback/Playhead.{h,cpp}`): deterministic current-position value (frame count, current frame index, presentation timestamp) derived from an explicit playback frame interval; it performs no pacing and reads no clock.
- `Clock` (`app/playback/Clock.h`) abstraction + `SystemClock` (`app/playback/SystemClock.{h,cpp}`, monotonic `QElapsedTimer`). Tests inject a manual clock, so playback behavior is verified without wall-clock delays.
- `PacingPolicy` (`app/playback/PacingPolicy.h`) abstraction + `DefaultPacingPolicy` (`app/playback/DefaultPacingPolicy.{h,cpp}`): maps elapsed time to a frame count (one frame per interval, dropping frames when behind). Pacing is replaceable and independently testable.
- `Player` (`app/playback/Player.{h,cpp}`): a `Stopped`/`Playing`/`Paused` state machine with `play()`/`pause()`/`stop()`, playhead getters, `tick()` (clock/pacing-driven with bounded per-tick catch-up) and `stepOnce()` (explicit single frame), and `stateChanged`/`positionChanged`/`framePresented`/`playbackEnded`/`errorOccurred` signals. It reacts to `FramePump` `frameReady`/`streamEnded`/`streamFailed`.
- `Player` has no timer, thread, UI, audio, duration metadata, or timeline; `FramePump` remains a passive, caller-driven decoder. The frame interval is a playback pacing parameter, not media metadata.
- Objective 10 preview-time contract unchanged; no Application/UI wiring, schema, or dependency changes. New files are compiled into both the application and test builds.

Verification:
- Application build succeeded (clean rebuild).
- Test build succeeded (clean rebuild).
- Automated tests: 154 passed, 0 failed, 0 skipped (144 prior + 10 player/timing tests).
- Offscreen launch smoke: event loop alive until timeout (SMOKE_EXIT=124).
- Official `scripts/build_and_test.sh` workflow re-run green.

## Phase 4 — 360 Reframing Engine (Deterministic Vertical Slice) — Complete

Status: Complete (2026-09-17). Human-approved priority: make the core 360 editing/reframing capability functional before continuing the Phase 3 player-lifecycle objective.

- Added `app/reframe/`:
  - `ReframePlan` + `CameraKeyframe` — structured, versioned, JSON-serializable, validated reframing decisions (source media id, source time range, output spec, ordered camera keyframes).
  - `CameraPath` — pure deterministic camera evaluation: shortest-path yaw interpolation, bounded pitch/roll/FOV, hold/linear segments, deterministic hold outside the keyframe range.
  - `ReframeFrameProvider` (replaceable seam) + `FfmpegSeekFrameProvider` (existing FFmpeg-CLI single-frame seam).
  - `ReframeRenderer` — deterministic per-frame reframing through the existing `EquirectView`, PNG-sequence output, and H.264 encode.
  - `ReframeIntent` + `ReframeIntentParser` — deterministic natural-language boundary (aspect/platform, time ranges, named/explicit directions, subject references). Unresolved subjects are reported, never fabricated.
  - `ReframePlanBuilder` — intent + resolved target directions -> validated plan.
  - `ReframePipeline` — end-to-end orchestration: 360 source -> intent -> plan -> deterministic reframing -> flat video. Source media is read-only.
- Registered in `reelcraft.pro` and `tests/tests.pro`.
- Not implemented: AI provider/model, person/object detection/tracking, speaker analysis, content-based cut selection, reframe-plan persistence in the project schema, and Application/UI integration.

Verification:
- Application build succeeded (clean rebuild).
- Test build succeeded (clean rebuild).
- Automated tests: 180 passed, 0 failed, 0 skipped (154 prior + 26 reframing tests), ~41.5 s.
- End-to-end test renders a real FFmpeg-generated equirect clip through intent -> plan -> reframing -> H.264 output and decodes the result to the expected dimensions.
- Offscreen launch smoke: SMOKE_EXIT=124.

Architecture decision: Decision 018 records the structured plan boundary, replaceable frame provider, deterministic renderer, and deterministic intent boundary.

Next: target/subject resolution (detection/tracking) behind the resolved-target boundary — see `NEXT_TASK.md`.

## Phase 4 Objective 2 — Target/Subject Resolution — Complete

Status: Complete (2026-09-17). Follows the completed deterministic reframing engine (Decision 018) and implements the "eyes" needed to resolve targets.

- Added `app/target/`:
  - `EquirectProjection` — pure 360 geometry: equirectangular pixel <-> spherical direction, perspective/tangent-view pixel <-> direction (exact inverse of `EquirectView`), detection box -> spherical centre + angular extent, great-circle distance, seam-safe yaw deltas, pitch/FOV bounds.
  - `EquirectViewPlan` — deterministic overlapping perspective-view coverage of the sphere (with polar views when needed).
  - `TargetDetector` seam + `ProcessTargetDetector` — a dependency-free subprocess adapter (file/JSON protocol) so no computer-vision library is linked into Reelcraft.
  - `TargetTypes` — `TargetDetection`, `TargetQuery`, `TargetObservation` (timestamp, identity, yaw/pitch, confidence, class, angular extent, evidence) and `TargetTrack` with shortest-yaw sampling.
  - `SphericalTargetTracker` — deterministic spherical NMS plus greedy nearest-neighbour association with a gate and miss counting.
  - `TargetResolver` — orchestrates views -> detector -> spherical observations -> tracks, reports unresolved queries honestly, and exposes `resolvedTargets()` for the existing `ReframePlanBuilder`.
  - `TargetTrackPlanner` — turns a resolved track into a validated `ReframePlan` consumed unchanged by `CameraPath`/`ReframeRenderer`.
- Registered in `reelcraft.pro` and `tests/tests.pro`.
- Recorded the detection-technology and licensing evaluation in `docs/TARGET_RESOLUTION_TECHNOLOGY.md`; Decision 019 records the architecture.

Verification:
- Application build succeeded; offscreen smoke SMOKE_EXIT=124.
- Test build succeeded.
- Automated tests: 219 passed, 0 failed, 0 skipped (180 prior + 39 target-resolution tests), ~40 s.
- Model-free end-to-end test: synthetic equirect frame -> color detector -> spherical direction -> ReframeTarget -> `ReframePlanBuilder` -> `CameraPath`, and a separate detection -> track -> plan -> `ReframeRenderer` render.

Not implemented: "me" identity, speaker localization, semantic classification, production tracking quality, reframe-plan persistence, and UI integration.

## Phase 4 Objective 3 — Real Detector Integration — Complete

Status: Complete (2026-09-17). Follows the target-resolution boundary (Decision 019) and proves real detection on real 360 footage.

- Added an optional external helper `tools/detector_helper/yolox_detector.py` (plus `README.md`), implementing the existing `ProcessTargetDetector` file/JSON protocol. Selected model: **OpenCV Zoo YOLOX (2022nov)**, Apache-2.0 (code and weights), COCO 80 classes; runtime Python 3 + OpenCV DNN 4.10 (CPU). Reelcraft links no computer-vision library.
- Added the `realDetectorIntegration` test, skipped unless configured; the normal suite remains model-free.
- No changes to the `TargetDetector`/`ProcessTargetDetector` boundary or the 360 geometry.

Verification (real footage, proxy used for speed; original untouched):
- Real 3840x1920 equirect clip (`360_TEST_4K.mp4`); 1920x960 proxy of the 114-126 s segment.
- Real YOLOX detected two presenters in tangent views; spherical directions yaw -27.9/pitch -28.6 and yaw +26.9/pitch -27.8 (independently reproduced).
- The strongest `person` track held id `t1` across five sampled frames (mean confidence 0.923).
- The track produced a validated `ReframePlan`; `ReframeRenderer` produced a 640x360 H.264 clip visibly centered on the detected presenter.
- Normal suite: 219 passed, 0 failed, 1 skipped.

Architecture decision: Decision 020 records the real-detector selection and the preserved boundary.

## Phase 4 Objective 4 — Target Identity & Deterministic Selection — Complete

Status: Complete (2026-09-17). Adds structured creator identity above detection/tracking; no speaker/audio association and no appearance re-identification.

- Added `app/target/TargetIdentity.{h,cpp}`: `CreatorTargetSelection`, `IdentityBinding`, and `TargetIdentityRegistry`. Creator identity is a structured, JSON-serializable binding (canonical "me") to a tracker track id, established from a creator seed direction/time (or an explicit track id) and refreshed against live tracks.
- Added `app/target/TargetSelector.{h,cpp}`: deterministic reference resolution for "me"/selected aliases, "the other person", "person N"/ordinals, left/right, exact track id, and a unique label. Canonical ordering is (first observation time, numeric track id, id string); ambiguity is reported with candidates instead of guessing.
- Hardened `SphericalTargetTracker` with deterministic constant-velocity prediction (yaw/pitch, bounded horizon) for crossing trajectories and a bounded re-entry gate for temporary loss/occlusion/re-entry; still no appearance model.
- Registered in `reelcraft.pro` and `tests/tests.pro`; identity and selection feed the existing `ReframePlanBuilder` and `TargetTrackPlanner` unchanged.

Verification (model-free):
- 21 new deterministic tests: crossing under prediction, re-entry within/beyond window, prediction determinism, seed binding, claim conflicts, active-state resolution, unique vs ambiguous continuity re-binding, identity JSON round-trip, creator aliases, other-person ambiguity, ordinal determinism regardless of input order, left/right, track-id/label, and identity -> `ReframePlanBuilder` -> `CameraPath`.
- Full model-free suite: 240 passed, 0 failed, 1 skipped (the real-detector test when unconfigured).

Verification (real footage, `360_TEST_4K.mp4`, proxy; original untouched):
- 9 person tracks; identity "me" bound by seed to the strongest presenter (t1, 5 observations, mean confidence 0.923).
- `person 1` -> t1 (canonical order); `the other person` -> ambiguous with 8 candidates (honest ambiguity in a crowded scene).
- The bound identity's track produced the `ReframePlan` and a 640x360 H.264 output centered on the selected presenter.
- Real integration test PASS.

Architecture decision: Decision 021 records identity representation, selection semantics, and the future appearance/re-ID seam.

Next: speaker/active-speaker association and appearance-based re-identification (`NEXT_TASK.md` Objective 5).






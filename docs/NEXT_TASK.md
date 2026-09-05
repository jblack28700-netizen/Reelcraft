# Reelcraft — Next Task

## Phase 2 Status

- Phase 1 Desktop Application Foundation — complete and verified.
- Phase 2 Objective 1 — Viewer State Foundation — complete and verified (viewport orientation model, application ownership, UI readout/keyboard controls, JSON serialization and persistence, schema versioning).
- Phase 2 Objective 2 — Viewer Presentation Foundation — complete and verified at checkpoint `58cd561` (deterministic synthetic 360° test scene in `ViewerScene`; minimal presentation surface in `ViewerWidget` embedded in the desktop UI).
- Phase 2 Objective 3 — Viewer Camera Integration — complete and verified at checkpoint `fb7bb49` (deterministic `ViewerProjection` camera/view transform; camera-driven `ViewerWidget` presentation following the authoritative application-owned `ViewportState`; Application→UI wiring in `main.cpp`).
- Phase 2 Objective 4 — Real Media Foundation — complete and verified (`MediaItem` media records; `Application`-owned import/validation/deduplication; optional additive project `media` JSON section persisted on save and restored deterministically on reopen; Import Media UI; original files never modified).
- Phase 2 Objective 5 — Media Reference Availability and Open-Time Integrity — complete and verified (reopen-time revalidation of media references; deterministic list normalization/deduplication on open; unavailable-media status surfaced through the existing signal boundary).
- Phase 2 Objective 6 — Project Media Library Management (List & Remove) — complete and verified (`mediaListChanged` structured notification; deterministic `removeMedia(id)`; media list and Remove action in the shell; UI kept in sync without polling).
- Phase 2 Objective 7 — Active (Selected) Media — "Viewer Source" Contract — complete and verified (Application-owned active media id; `setActiveMedia`/`activeMediaId`/`activeMediaItem`; `activeMediaChanged`; deterministic list-consistency rules; optional additive persisted `activeMediaId`; Set Active/label affordance).
- Phase 2 Objective 8 — Equirectangular Frame Presentation Foundation — complete and verified (deterministic CPU `EquirectView` equirectangular→camera pixel renderer consistent with the ViewportState/ViewerProjection conventions; `ViewerWidget` optional source-image presentation path; marker-scene path unchanged).
- Phase 2 Objective 9 — Active-Media Frame Presentation (FFmpeg-CLI single-frame decode) — complete and verified (isolated `FrameExtractor` seam invoking the external ffmpeg CLI to decode one frame to PNG→QImage; `Application::previewActiveMediaFrame` → `framePreviewReady(QImage)` → viewer pixel path; Preview Active Frame action in the shell).
- Automated suite: 107 passed, 0 failed. Working tree clean at the Objective 9 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 9: Active-Media Frame Presentation

Status: **Complete — implemented and verified (2026-09-05; automated suite 107 passed, 0 failed).**

### Objective

Close the final Phase-2 gap: decode a single real frame from the active media record and present it through the existing verified equirectangular pixel path. Approved decode approach: FFmpeg-CLI adapter (Decision 015).

### Scope (implemented)

- `app/media/FrameExtractor.{h,cpp}`: isolated QtCore seam — invokes the external `ffmpeg` executable via QProcess (never linked) to decode one frame (`-frames:v 1 -f image2 -c:v png pipe:1`), parsed in-process to a QImage. Executable discovered via `REELCRAFT_FFMPEG` override then `QStandardPaths::findExecutable`; deterministic errors for unavailable executable, missing/invalid file, start/timeout/exit failure, empty output, undecodable frame; no partial output mutation; media file never modified.
- `Application::previewActiveMediaFrame()`: requires an active project, an active media record, and an available file; extracts a frame; on success emits `framePreviewReady(QImage)` and a status message; all failures report deterministically through `backgroundCompleted`.
- `MainWindow`: Preview Active Frame button (`previewFrameButton`) → `previewFrameRequested`; `showFramePreview(QImage)` presents the frame through `viewerWidget()->setSourceImage` (viewer pixel path, no viewer code changes).
- `main.cpp`: wires `previewFrameRequested` → `previewActiveMediaFrame` and `framePreviewReady` → `showFramePreview`.
- Decision 015 recorded in `DECISIONS.md` (FFmpeg-CLI decode adapter; replaceable behind the FrameExtractor seam).

### Explicit Non-Goals

- Playback, streaming, seek, multi-frame/video pipelines, audio
- Timeline/clips/tracks; editing engine; AI analysis/editing; export/render; effects/transitions
- Media engine; QtMultimedia/OpenCV/GStreamer or linked FFmpeg libraries (Option B/C deferred)
- Binding/auto-presenting active media without user action; schema changes; new compiled dependencies; camera-specific behavior; 360° classification/reframing; UI redesign

### Definition of Done

- FrameExtractor decodes one deterministic frame from a real media file (ffmpeg available) with clean errors otherwise.
- Application preview requires project + active + available media; emits exactly one `framePreviewReady` on success with correct frame; deterministic messages on failure.
- Preview flows to the viewer and renders through the EquirectView camera path (end-to-end verified: decoded equirect test pattern centers FRONT at identity).
- Focused tests pass; the 100-test regression suite remains green (suite now 107). Decode-dependent tests skip cleanly (reported) when ffmpeg is unavailable.
- Build + offscreen smoke pass; docs updated; one Git checkpoint; clean tree; no schema/dependency changes.

### Verification Strategy

- New tests: FrameExtractor invalid-input rejection (no ffmpeg needed); availability + single-frame decode of a real PNG media file (color/dims exact); deterministic repeatability; Application preview guards (no project / no active); preview emits frame with correct content; Preview button signal; end-to-end decode→viewer center assertion (identity camera centers FRONT). All decode tests QSKIP gracefully when ffmpeg is absent.
- Regression: full suite offscreen via the existing build-and-test script.

### Architectural Boundaries / Risks

- `FrameExtractor` is the decode seam; implementation may be replaced by a future media engine without changing callers or the viewer contract.
- External ffmpeg CLI reliance is the approved trade-off (Decision 015); path injectable (`REELCRAFT_FFMPEG`), availability feature-detected.
- Decode is synchronous (single frame) — acceptable foundation; playback/streaming would require a different objective.
- Preview does not modify Application/Project/schema/media contracts beyond the new slot/signal/UI wiring.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 9 commit, parent `1a47c01`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 9 state. Do not begin automatically.

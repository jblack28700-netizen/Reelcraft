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
- Phase 2 Objective 10 — Active-Media Time Navigation (single-frame stepping/seek) — complete and verified (`FrameExtractor::extractFrameAt` with `-ss` seek; Application preview-position state + `previewActiveMediaFrameAt`/`stepActiveMediaPreview`/`previewTimeChanged`; Step −1 s / +1 s controls and time readout; position resets on new/open/active-change/active-removal).
- Phase 2 Objective 11 — Pointer-Based Viewer Orientation Control — complete and verified (left-drag yaw/pitch deltas: drag right = yaw increases, drag up = pitch increases; wheel FOV: up = decrease = zoom in, down = increase = zoom out; routed through the existing Application adjust slots; viewer remains presentation-only).
- Phase 2 Objective 12 — Projection Declaration & Flat-Media Preview Path — complete and verified (optional additive `projection` on `MediaItem` — equirectangular | flat | Unknown; declared via shell controls; Unknown routes to equirectangular; opt-in flat fit/letterbox presentation with no camera transform; equirect/marker paths unchanged).
- Phase 2 Objective 13 — Viewer Presentation State Consistency — complete and verified (stale decoded frames and flat mode are cleared when the project or active-media context changes: new project, open, active-media change, active-media removal; the viewer returns to the deterministic marker scene until the user previews again).
- Automated suite: 131 passed, 0 failed. Working tree clean at the Objective 13 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 13: Viewer Presentation State Consistency

Status: **Complete — implemented and verified (2026-09-06; automated suite 131 passed, 0 failed).**

### Objective

Make the viewer's on-screen content and presentation mode deterministically consistent with Application state: when the project or active-media context changes (new project, open, active-media change, active-media removal), any stale decoded frame and flat mode are cleared and the viewer returns to the deterministic marker scene until the user previews again.

### Scope (implemented)

- `MainWindow::showProject` (projectChanged: new/open) clears the viewer source and flat mode.
- `MainWindow::showActiveMedia` clears when the active media id changes (including cleared), never on same-id re-announcement.
- Private `MainWindow::clearViewerSource()`: `viewerWidget()->setSourceImage(QImage())` + `setFlatSourceMode(false)`.
- Preview/step/routing flows unchanged; Application and viewer contracts unchanged.

### Explicit Non-Goals

- Application/Project/MediaItem/media/decode/time-navigation changes; EquirectView or rendering changes
- Auto-presenting frames on active change (preview stays an explicit user action)
- Continuous playback/audio/streaming; ffprobe/duration (gated); schema/persistence/dependencies
- UI redesign; camera-specific logic; Objective 14 work

### Definition of Done

- Stale frames cleared on new project, open, active-media change, and active-media removal; same-active no-op.
- Preview/step/flat-equirect routing behavior unchanged.
- Focused tests pass; the 128-test regression suite remains green (suite now 131).
- Build + offscreen smoke pass; docs updated; one Git checkpoint; clean tree; no schema/dependency/contract changes.

### Verification Strategy

- Focused: after a frame preview, newProject clears source + flat mode (marker scene visible at identity); active-media switch and active-media removal clear; same-active re-announcement does not clear.
- Regression: full suite offscreen via the existing build-and-test script.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 13 commit, parent `2e2eb64`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 13 state. Do not begin automatically.

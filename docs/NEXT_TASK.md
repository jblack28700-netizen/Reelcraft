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
- Automated suite: 128 passed, 0 failed. Working tree clean at the Objective 12 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 12: Projection Declaration & Flat-Media Preview Path

Status: **Complete — implemented and verified (2026-09-06; automated suite 128 passed, 0 failed).**

### Objective

Make source projection an explicit, creator-declared per-media property and add a correct undistorted flat preview mode — while leaving the equirectangular presentation path byte-for-byte unchanged and routing unknown projection to it (status quo).

### Scope (implemented)

- `MediaItem::Projection { Unknown, Equirectangular, Flat }`: optional additive `projection` JSON key (`"equirectangular"` | `"flat"`; absent/unrecognized ⇒ Unknown; serialized only when declared; no schemaVersion change/migration).
- `Application::declareMediaProjection(mediaId, projectionValue)`: guards (project/media id/value), updates the record, re-emits `mediaListChanged`.
- `ViewerWidget`: `setFlatSourceMode(bool)` (default false). Flat mode draws the source aspect-preserving, centered, letterboxed on the existing background, with NO equirectangular/camera transformation; drag/wheel may still run but do not transform flat presentation. Equirect and marker-scene paths unchanged.
- `MainWindow`: projection stored per media row (role data); `showFramePreview` routes by the active row's projection (flat ⇒ flat mode; else equirectangular); Mark Flat / Mark Equirect controls on the selected row.
- `main.cpp`: wires the projection request signal to the Application slot.

### Explicit Non-Goals

- ffprobe/probing auto-detection (deferred decision); other projections (fisheye/cylindrical); flat pan/zoom/reframing
- Continuous playback/audio/streaming; time-navigation changes (Obj 10 preserved)
- Media-engine/timeline/clips/tracks/AI/export/effects; camera-specific logic; UI redesign
- schemaVersion bump/migration; new dependencies; changes to equirectangular/marker rendering

### Definition of Done

- Projection concept persisted additively and round-tripped; unknown fallback routing to equirectangular.
- Flat preview is deterministic (fit, centered, letterboxed, camera-independent); equirect/marker paths unchanged.
- Focused tests pass; the 120-test regression suite remains green (suite now 128).
- Build + offscreen smoke pass; docs updated; one Git checkpoint; clean tree; no dependency/schemaVersion changes.

### Verification Strategy

- New tests: projection default/parse; JSON round trip incl. unknown fallback; Application declaration guards/validation/emission; persistence across reopen; flat mode letterbox/center; flat mode ignores camera transforms; projection buttons emit requests; preview routing honors declared projection (flat vs equirect center anchors).
- Regression: full suite offscreen via the existing build-and-test script.

### Architectural Boundaries / Risks

- Unknown projection intentionally routes to equirectangular (backward-compatible status quo), documented and tested.
- Additive optional key consistent with the `viewerState`/`media`/`activeMediaId` precedents — not a schema change.
- Flat mode is opt-in and off by default; equirect pixel/marker tests unchanged.
- Viewer remains presentation-only; Application owns media/projection state.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 12 commit, parent `e73f2cc`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 12 state. Do not begin automatically.

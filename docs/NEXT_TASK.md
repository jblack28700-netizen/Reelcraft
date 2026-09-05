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
- Automated suite: 120 passed, 0 failed. Working tree clean at the Objective 11 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 11: Pointer-Based Viewer Orientation Control

Status: **Complete — implemented and verified (2026-09-06; automated suite 120 passed, 0 failed).**

### Objective

Complete Phase-2 orientation control with the primary 360-viewer gestures: drag-to-look and wheel FOV, routed through the existing authoritative ViewportState and Application adjust slots. The viewer remains presentation-only.

### Scope (implemented)

- `ViewerWidget` pointer handling: left-button drag emits `viewportYawDeltaRequested` (drag right = yaw increases) and `viewportPitchDeltaRequested` (drag up = pitch increases) at a deterministic sensitivity (0.25 °/px); mouse wheel emits `viewportFovDeltaRequested` (wheel up = −5° = zoom in; wheel down = +5° = zoom out, per 120-unit wheel step); non-left drags ignored; no modifiers/inertia/multi-touch.
- New `ViewerWidget` delta signals wired in `main.cpp` to the existing Application adjust slots (same receivers as keyboard controls).
- Sensitivity constants documented as deterministic defaults (revisitable without contract change).

### Explicit Non-Goals

- Continuous playback/auto-advance/audio/streaming; time-navigation changes (Obj 10 preserved)
- Rendering changes; flat-vs-equirect routing; media/decode/schema/persistence changes
- Multi-touch/pinch/inertia/modifier gestures; roll via pointer; double-click zoom
- Camera-specific logic; UI redesign; new dependencies; Objective 12 work

### Definition of Done

- Pointer deltas emitted with the approved semantics and deterministic sensitivity; Application-owned ViewportState updated through existing slots with clamping honored.
- New focused tests pass; the 116-test regression suite remains green (suite now 120).
- Build + offscreen smoke pass; docs updated; one Git checkpoint; clean tree; no schema/dependency/render changes.

### Verification Strategy

- New tests: drag emits expected yaw/pitch deltas; non-left drags and release-without-move emit nothing; wheel-up decreases FOV / wheel-down increases FOV; wired drags update Application ViewportState deterministically (incl. pitch clamp and FOV change).
- Regression: full suite offscreen via the existing build-and-test script.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 11 commit, parent `00f57ec`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 11 state. Do not begin automatically.

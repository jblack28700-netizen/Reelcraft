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
- Phase 2 Objective 14 — Presentation Quality: Bilinear Equirectangular Rendering — complete and verified (deterministic bilinear sampling in EquirectView; all camera/seam/pole/validation contracts preserved; MaxOutputWidth stays 640; measured 36.98 ms/frame at 640x320, informational 148.69 ms at 1280x640).
- Automated suite: 133 passed, 0 failed. Working tree clean at the Objective 14 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 14: Presentation Quality — Bilinear Equirectangular Rendering

Status: **Complete — implemented and verified (2026-09-06; automated suite 133 passed, 0 failed).**

### Objective

Replace nearest-neighbor equirectangular sampling with deterministic bilinear interpolation to improve single-frame review quality, preserving every rendering/camera/validation contract.

### Scope (implemented)

- `EquirectView::render` now samples bilinearly (four-texel straight-space blend with alpha; horizontal wrap at the seam; vertical clamp at poles; integer-rounded, deterministic).
- Camera conventions (ViewportState/ViewerProjection), roll/FOV math, validation and no-partial-output semantics unchanged; output Format_ARGB32 unchanged.
- `MaxOutputWidth = 640` unchanged and not configurable (this objective).
- No changes to flat mode, ViewerWidget architecture, FrameExtractor, media/time/audio/engine, schema, or dependencies.

### Explicit Non-Goals

- Raising or configuring `MaxOutputWidth` (deferred; informational 1280x640 cost measured)
- Render caching/invalidation or GPU path (follow-up if interactive latency warrants)
- Flat presentation, decode seam, media/time/audio changes; new dependencies; schema/contract changes; Objective 15 work

### Definition of Done

- Bilinear sampling deterministic and correct (focused blend-anchor and seam/pole tests).
- All existing EquirectView/viewer/presentation tests pass unmodified.
- Full regression green (131 prior + 2 new = 133); build + offscreen smoke pass.
- Performance measured and recorded (36.98 ms/frame at 640x320 bilinear; informational 148.69 ms at 1280x640).
- Docs updated; one Git checkpoint; clean tree; no dependency/schema/contract change.

### Verification Strategy

- Focused: exact quarter-blend at texel (1.5,1.5) on a 4x4 source (bilinear, not nearest); seam (+/-180 deg) and pole (+/-90 deg) robustness and determinism.
- Regression: full suite offscreen via the existing build-and-test script.

### Architectural Boundaries / Risks

- Uniform-patch interiors are identical under bilinear, so all color-anchor and pixel tests are preserved unmodified.
- Per-paint cost rises (~37 ms at 640x320 vs ~15-23 nearest); drag/wheel re-render per event — recorded as an interaction-latency concern; caching/GPU are follow-up candidates, not this objective's scope.
- 640px cap remains and configurability is explicitly deferred.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 14 commit, parent `3c1046d`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 (360 Viewer Review & Navigation) development objective from the verified Phase 2 Objective 14 state. Do not begin automatically.

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
- Automated suite: 100 passed, 0 failed. Working tree clean at the Objective 8 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 8: Equirectangular Frame Presentation Foundation

Status: **Complete — implemented and verified (2026-09-05; automated suite 100 passed, 0 failed).**

### Objective

Establish the smallest safe, deterministic, decode-free pixel presentation foundation: render an equirectangular image through the existing authoritative `ViewportState` camera, reusing the verified ViewerProjection conventions — a reusable pixel path that later real-media decoding may feed.

### Scope (implemented)

- `app/viewer/EquirectView.{h,cpp}`: deterministic CPU equirectangular→camera renderer (`render(source, yaw, pitch, roll, fov, w, h, out) -> bool`). Conventions identical to ViewportState/ViewerProjection (identity → FRONT centered; +90° yaw → RIGHT centered; −90° → LEFT; ±90° pitch reaches UP/DOWN; positive roll rotates content counter-clockwise; vertical FOV [20,140]); nearest-neighbor sampling; deterministic rejection of empty source, non-positive output, non-finite camera values, |pitch|>90, FOV outside [20,140] — failure never partially mutates output. `MaxOutputWidth = 640` documented as an implementation/performance safeguard, not an architectural limit.
- `ViewerWidget` optional source-image path: `setSourceImage(QImage)` / `hasSourceImage()`; when a valid source is present the widget renders it through the current `ViewportState` camera (capped, aspect-preserving); when absent/cleared the existing synthetic marker-scene path is unchanged (protected regression contract). Widget remains presentation-only; it owns no camera state.
- No Application, Project, MediaItem, activeMediaId, media-library, or schema changes. No media/decoder contract defined.

### Explicit Non-Goals

- Real media decoding/probing/playback/streaming/audio; FFmpeg/GStreamer/QtMultimedia/OpenCV
- Binding activeMediaId/real decoded frames to the viewer (future, separately gated objective)
- Timeline/clips/tracks; editing engine; AI analysis/editing; export/render pipelines; audio; effects/transitions
- 360° reframing/keyframes; camera-specific behavior; 360° classification
- GPU rendering/shaders; resolution cap is a documented safeguard only, not an architectural limit
- Media-library redesign; schema changes/migrations; new dependencies; unrelated refactoring/cleanup

### Definition of Done

- Equirectangular pixel path renders deterministically through the verified camera conventions (behavioral anchors: identity FRONT, +90 yaw RIGHT, −90 yaw LEFT, ±90 pitch UP/DOWN, roll direction, FOV coverage).
- Invalid input rejected deterministically without partial output mutation or crashes.
- Pixel-for-pixel repeatability for identical inputs.
- ViewerWidget source-image path follows the ViewportState camera; clearing the source restores the marker-scene rendering byte-for-byte behaviorally (existing render tests unchanged and green).
- CPU rendering cost measured and recorded (performance sanity).
- New focused tests pass; the 90-test regression suite remains green (suite now 100).
- App and tests build/pass offscreen; smoke unaffected; docs updated; one Git checkpoint; clean tree; no dependencies, no schema change.

### Verification Strategy

- New tests: identity/yaw/pitch anchors; positive-roll direction verified against the ViewerProjection convention using quadrant analysis of a pitched stripe; FOV coverage change; invalid-input rejection incl. no-partial-mutation; pixel-for-pixel determinism; ViewerWidget source-image integration and clear-restores-scene; CPU render performance sanity (recorded ~14 ms/frame at 640×320).
- Regression: full suite offscreen via the existing build-and-test script.

### Architectural Boundaries / Risks

- No second camera model or duplicate camera state; camera semantics derived from and consistent with ViewportState/ViewerProjection.
- ViewerWidget remains presentation-only; the in-memory source image is low-level presentation data and defines no decoder/media-engine contract.
- Marker-scene rendering is a protected regression contract (opt-in source path only).
- Resolution cap is a safeguard; future objectives may raise it or introduce optimized/GPU rendering without redesigning the camera contract.
- CPU cost bounded by capped output; measured and recorded; no timers/polling/background threads.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 8 commit, parent `91979d3`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 8 state. Do not begin automatically.

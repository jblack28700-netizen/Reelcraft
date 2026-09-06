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
- Phase 2 Objective 15 — Real-Media Review Path Validation — complete and verified (deterministic FFmpeg-generated 30 s equirect clip fixture; full review path exercised: import/select, preview, stepping across frames, look-around during frame presentation, equirect routing, state resets; environment limitation recorded — no physical camera here; informational perf: ~633 ms/step decode, ~41 ms/paint render).
- Automated suite: 136 passed, 0 failed. Working tree clean at the Objective 15 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 15: Real-Media Review Path Validation

Status: **Complete — implemented and verified (2026-09-06; automated suite 136 passed, 0 failed).**

### Objective

Validate the full Phase-2 review workflow (import/select, single-frame preview, stepping/seek, look-around orientation, equirect routing, state resets) end-to-end against deterministic FFmpeg-generated equirectangular multi-frame media — with no physical camera hardware.

### Scope (implemented)

- Test-side fixture generator: 2:1 equirect video (30 s @ 1 fps, all-keyframe) whose frames carry deterministic time-varying FRONT content (red/green/blue cycle) and a fixed RIGHT marker (magenta), reusing the existing FFmpeg-CLI fixture pattern (Decision 015; QSKIP if ffmpeg unavailable).
- Focused end-to-end tests: fixture frame distinctness/seekability; full review path (preview t=0 red, step +1 green, +90° yaw look-around centers magenta, seek t=5 blue, position tracking, viewer state via wiring); informational performance.
- No production source changes; no dependencies; no ffprobe/duration/playback/audio/hardware.
- Dev script change (approved): test-runner timeout raised 20 s -> 240 s in `scripts/build_and_test.sh` because FFmpeg fixture suites exceed the old cap.

### Explicit Non-Goals

- Continuous playback/duration/ffprobe/audio (deferred, Decision 016); hardware-camera validation (out of scope here — recorded limitation)
- Production-code changes; FrameExtractor/Application/ViewerWidget/EquirectView/media/schema/dependency changes; GPU/cap changes

### Definition of Done

- Deterministic equirect long-clip fixture produces verifiable, distinct frames.
- New review-path tests pass (stepping across frames, look-around with frame present, routing, state, position).
- Full regression (133 prior + 3 new = 136) green; build + offscreen smoke pass; informational performance recorded; docs updated; one checkpoint; clean tree; no dependency/schema/architecture change.

### Verification Strategy

- Focused: fixture sanity + end-to-end review path on the clip; informational timing (no gate).
- Regression: full suite offscreen via the existing build-and-test script (raised timeout).

### Architectural Boundaries / Risks

- Pure test/validation layer; all production contracts preserved.
- Environment limitation recorded: synthetic-but-real encoded media is the proxy; no hardware 360 camera in this environment.
- Measured informational costs: ~633 ms/step decode (subprocess), ~41 ms/paint render at 640x320 — input to future interaction/caching decisions, not gates.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 15 commit, parent `8d11e85`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 (360 Viewer Review & Navigation) development objective from the verified Phase 2 Objective 15 state. Do not begin automatically.

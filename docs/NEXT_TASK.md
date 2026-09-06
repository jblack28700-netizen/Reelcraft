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
- Automated suite: 136 passed, 0 failed. Phase 2 formally closed at the closeout commit (docs-only; prior HEAD `ae317f5`).

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Phase 2 — 360 Viewer Review & Navigation — Formally Closed

Status: **Complete — formally closed 2026-09-06 (closeout commit).**

- Objectives 1-15 complete and verified; automated suite 136 passed / 0 failed / 0 skipped; builds succeed; offscreen smoke SMOKE_EXIT=124; clean-checkout verification of the closeout commit passes.
- Scope per Decision 016: import/manage/select media, deterministic single-frame preview, time stepping/seek, look-around orientation (keyboard + pointer), flat/equirectangular correctness, consistent viewer/project state. Continuous/paced playback, duration-aware transport, and audio are explicitly deferred.
- Closeout was documentation-only (CURRENT_STATE, PROJECT_HISTORY, DEVELOPMENT_LOG, CHANGELOG v0.2.33); no production source/test/script/schema/dependency/architecture changes.

## Phase 3 — Media Engine (OPEN — opening architecture recorded; implementation NOT authorized)

Status: **Open (decisions recorded 2026-09-06, Decision 017). No media-engine/playback/duration/audio implementation exists.**

- Phase 3 opening architecture (approved): persistent FFmpeg streaming subprocess behind a replaceable media/player seam — the currently feasible implementation path in this environment.
- FrameExtractor remains the deterministic single-frame preview/validation seam (not the permanent engine).
- Linked FFmpeg libraries and QtMultimedia: deferred (do not install; revisit at a real desktop/deployment dependency decision).
- ffprobe duration metadata: reopened as its own future Phase 3 decision gate (not implemented).
- Objective 10 preview-time position contract is preserved; playback must extend rather than replace it.
- Architecture boundary: decode/media-source seam → player/timing → Application (orchestration) → Viewer (presentation); timeline/editor/AI above Application, never reaching into decoding.
- Phase 2 remains formally closed (`dcd876b`). No implementation has started.

## Phase 3, Objective 2 — Persistent FFmpeg Streaming Feasibility Probe — Complete

Status: **Complete — implemented and verified (2026-09-06; automated suite 139 passed, 0 failed).**

### Objective (tests-only feasibility evidence)

Prove, with temporary test-only probe helpers (no production changes), that a persistent FFmpeg subprocess can stream frames, reach EOF cleanly, fail deterministically on missing input, and be killed/restarted cleanly in this environment.

### Scope (implemented)

- Temporary probe helpers + 3 tests in `tests/test_project.cpp` (clearly marked Phase 3 Objective 2 feasibility; no production/engine code).
- Persistent rawvideo stream read of a 25-frame @5 fps deterministic equirect clip; bounded waits everywhere.
- Measurements recorded informationally (no timing gate): 25/25 frames delivered at ~39.9 frames/s (unthrottled rawvideo, 160x80), ~1.0 ms inter-frame delivery latency.
- EOF normal-exit; missing-file deterministic error; terminate/kill then clean restart verified.
- No dependency, ffprobe, schema, Obj-10, FrameExtractor, or production changes; rawvideo/transport details intentionally NOT decisions.

### Verification

- Focused probe tests pass; full regression 139/0/0; build success; offscreen smoke SMOKE_EXIT=124.

## Next Objective (Phase 3, Objective 3) — NOT STARTED

Media-engine foundation: introduce the replaceable player/media-source seam and a deterministic frame pump over the persistent-subprocess stream (reusing probe evidence), without changing the Objective 10 contract. Do not begin automatically.

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
- Automated suite: 116 passed, 0 failed. Working tree clean at the Objective 10 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 10: Active-Media Time Navigation

Status: **Complete — implemented and verified (2026-09-06; automated suite 116 passed, 0 failed).**

### Objective

Allow navigation of the active media record in time by requesting decoded frames at specified offsets through the existing verified pixel path — on-demand single-frame stepping/seek only (no continuous playback).

### Scope (implemented)

- `FrameExtractor::extractFrameAt(filePath, execPath, seconds, out, error)`: same FFmpeg-CLI PNG-pipe decode with fast input seek (`-ss <seconds> -i …`); `extractFirstFrame` now delegates with 0.0 s (contract preserved). Negative/non-finite times rejected deterministically. The requested position is a seek/preview request, not a guarantee of exact frame/presentation-timestamp accuracy.
- `Application`: session-only `m_previewTimeSeconds`; `previewTimeSeconds()`; `previewActiveMediaFrameAt(seconds)` (negative clamps to 0); `stepActiveMediaPreview(delta)` (floor 0); private `decodePreviewFrameAt(target)` with the Objective 9 guards; position updates only on successful decode; `previewTimeChanged(double)` emitted only on actual position change; position resets to 0 (with emission) on new project, project open, active-media change, and removal of the active media. Beyond-end/undecodable requests fail deterministically with position unchanged.
- `MainWindow`: Step −1 s / Step +1 s buttons (`stepBackButton`/`stepForwardButton`) → `previewStepRequested(delta)`; `previewTimeLabel` readout + `showPreviewTime(seconds)`; existing Preview Active Frame now decodes at the current position.
- `main.cpp` wiring for `previewStepRequested` and `previewTimeChanged`.
- No schema, linked-dependency, viewer, or media-contract changes; no ffprobe/duration probing (deferred by approval); original media never modified.

### Explicit Non-Goals

- Continuous playback / auto-advance, audio, streaming, rate control, playback loops (require a separate future decision)
- ffprobe-based duration probing or duration metadata (deferred by approval)
- Exact presentation-timestamp/frame-accuracy semantics (requested seek position only)
- Timeline/clips/tracks; editing engine; AI analysis/editing; export/render; effects/transitions
- Persisting the preview position; schema changes; linked FFmpeg/QtMultimedia; camera-specific behavior; 360° classification/reframing; UI redesign

### Definition of Done

- `extractFrameAt` decodes distinct frames at different offsets deterministically (validated on a generated all-keyframe fixture); `extractFirstFrame` behavior preserved.
- Application position state advances only on success; negative steps floor at 0; beyond-end requests fail deterministically leaving position unchanged.
- Position resets (with emission) on new project, open, active-media change, and active-media removal.
- Step/seek results flow through `framePreviewReady` → viewer unchanged.
- Focused tests pass; the 107-test regression suite remains green (suite now 116; 0 skipped — ffmpeg available).
- Build + offscreen smoke pass; docs updated; one Git checkpoint; clean tree; no schema/dependency changes.

### Verification Strategy

- New tests: invalid seek time rejection; frame-at-time extraction (0 s red-dominant vs 2 s blue-dominant on the fixture); seek determinism; Application guards (no project / no active); position update + single time emission on success; re-request same position does not re-emit time; step advance and below-zero clamp; beyond-end failure leaves position/messages deterministic; position resets across new-project/active-change/active-removal; step buttons and time readout in the shell. Decode-dependent tests QSKIP when ffmpeg is absent.
- Regression: full suite offscreen via the existing build-and-test script.

### Architectural Boundaries / Risks

- Position is session state only; it intentionally is not persisted and adds no schema surface.
- Seek is approximate (fast input seek, keyframe semantics); documented; fixture uses all-keyframe encoding for deterministic tests.
- Decode remains a one-shot subprocess per request; no timers/threads/loops introduced.
- All Objective 1–9 contracts preserved (active-media invariants, availability, `framePreviewReady`, EquirectView/ViewportState behavior, original-media protection).

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 10 commit, parent `a3db869`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 10 state. Do not begin automatically.

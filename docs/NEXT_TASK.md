# Reelcraft — Next Task

## Phase 2 Status

- Phase 1 Desktop Application Foundation — complete and verified.
- Phase 2 Objective 1 — Viewer State Foundation — complete and verified (viewport orientation model, application ownership, UI readout/keyboard controls, JSON serialization and persistence, schema versioning).
- Phase 2 Objective 2 — Viewer Presentation Foundation — complete and verified at checkpoint `58cd561` (deterministic synthetic 360° test scene in `ViewerScene`; minimal presentation surface in `ViewerWidget` embedded in the desktop UI).
- Phase 2 Objective 3 — Viewer Camera Integration — complete and verified at checkpoint `fb7bb49` (deterministic `ViewerProjection` camera/view transform; camera-driven `ViewerWidget` presentation following the authoritative application-owned `ViewportState`; Application→UI wiring in `main.cpp`).
- Phase 2 Objective 4 — Real Media Foundation — complete and verified (`MediaItem` media records; `Application`-owned import/validation/deduplication; optional additive project `media` JSON section persisted on save and restored deterministically on reopen; Import Media UI; original files never modified).
- Phase 2 Objective 5 — Media Reference Availability and Open-Time Integrity — complete and verified (reopen-time revalidation of media references; deterministic list normalization/deduplication on open; unavailable-media status surfaced through the existing signal boundary).
- Phase 2 Objective 6 — Project Media Library Management (List & Remove) — complete and verified (`mediaListChanged` structured notification; deterministic `removeMedia(id)`; media list and Remove action in the shell; UI kept in sync without polling).
- Automated suite: 81 passed, 0 failed. Working tree clean at the Objective 6 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 6: Project Media Library Management (List & Remove)

Status: **Complete — implemented and verified (2026-09-05; automated suite 81 passed, 0 failed).**

### Objective

Complete the media-management slice begun in Objectives 4–5: let the user see the media referenced by the current project and remove records — with no decode, viewer, schema, or dependency changes.

### Scope (implemented)

- `Application::removeMedia(mediaId)`: removes the matching record only (id-based, idempotent, deterministic messages), requires an active project; original files are never touched.
- `Application::mediaListChanged(const QList<MediaItem> &)`: structured notification emitted after a successful appending import, a successful removal, a project open, and a new project (empty); duplicate imports and failed removals do not emit.
- `MainWindow`: media list widget (`mediaListWidget`, file name + format tag) and Remove Media action (`removeMediaButton`); `showMediaList` populates the list; Remove with no selection is a no-op with status feedback.
- `main.cpp`: wires `mediaListChanged` → `showMediaList` and `removeMediaRequested` → `Application::removeMedia`.
- `Q_DECLARE_METATYPE(MediaItem)` and meta-type registration for the custom-type signal.

### Explicit Non-Goals

- Decoding, probing, playback, preview, timeline, editing, AI, export, audio, effects, camera-specific logic
- 360° classification/detection (conceptual only)
- Active/selected-media ("viewer source") concepts or any media-to-viewer binding
- Moving/copying media files, path rewriting, or reference repair; multi-select/drag-drop/confirm dialogs
- Viewer camera/presentation/scene changes
- Schema-version bump or migration (no new persisted fields)
- Performance/GPU optimization; new dependencies; unrelated refactoring/cleanup

### Definition of Done

- Media list UI reflects imports, removals, opens, and new projects deterministically without polling.
- `removeMedia` removes the correct record, preserves ordering, reports deterministically, and persists on the next save.
- Unknown id and no-active-project removals fail safely; Remove with no selection is a no-op with feedback.
- Removing an entry whose file is unavailable clears unavailable state when it was the last one.
- `mediaListChanged` fires exactly once per list-determining mutation with the correct list.
- Original media files never modified (byte/SHA-256 invariant maintained by the Objective 4 test).
- New tests pass; the 73-test regression suite remains green (suite now 81).
- App and tests build/pass offscreen; documentation updated; one Git checkpoint; clean tree; no dependencies, no schema change.

### Verification Strategy

- New tests: list emission on import/open/new/remove; removal correctness + persistence; unknown-id and no-project failures; removal clears unavailable state; MainWindow list population and Remove wiring (with and without selection).
- Regression: full suite offscreen via the existing build-and-test script.

### Architectural Boundaries / Risks

- `Application` remains the single owner of the media list; the UI only mirrors it via `mediaListChanged` (no polling, no UI-owned state).
- Media records are metadata only; removal never touches the underlying files (Decision 002).
- The conditional status message on open fires only for unavailable media; existing success-path signal tests are unaffected.
- Custom-type signal args require meta-type registration (covered by tests).
- MainWindow additions use new object names/signals; no existing widget or signal behavior changed.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 6 commit, parent `5f60e9c`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 6 state. Do not begin automatically.

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
- Automated suite: 90 passed, 0 failed. Working tree clean at the Objective 7 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 7: Active (Selected) Media — "Viewer Source" Contract

Status: **Complete — implemented and verified (2026-09-05; automated suite 90 passed, 0 failed).**

### Objective

Define the minimal deterministic concept of which imported media record is active/selected for viewing, keeping the viewer itself unchanged.

### Scope (implemented)

- Application-owned `m_activeMediaId`; accessors `activeMediaId()` and `activeMediaItem()` (resolves to exactly one current record; `nullptr` when none/unresolvable).
- `setActiveMedia(mediaId)`: requires an active project, an existing id, and an available referenced file; idempotent when already active; deterministic messages via the existing signal boundary.
- `activeMediaChanged(mediaId)` emitted only on actual change (empty = cleared).
- Deterministic list-consistency rules: import never auto-selects; removing the active record clears it; new project clears it; open normalizes the media list first and then restores the persisted id only when it resolves to a current, available record (dangling/invalid/unavailable ids are cleared deterministically).
- Persistence: optional additive top-level `activeMediaId` project JSON key following the `viewerState` precedent — written only when set, read leniently; no schema-version bump, no migration.
- MainWindow: Set Active button (`setActiveButton`) acting on the media-list current row, `activeMediaLabel` readout, and `▶` marking of the active row; `showActiveMedia` slot; `setActiveRequested` signal; wired in `main.cpp`.
- Availability is enforced at selection/restore time (point-in-time `referenceExists()`); no filesystem watching.

### Explicit Non-Goals

- Decoding, probing, playback, preview, or any media dependency
- Viewer changes (`ViewerWidget`, `ViewerProjection`, `ViewerScene`, `ViewportState` untouched); no media-to-viewer binding yet
- Media-library redesign, multi-select, drag-and-drop, confirm dialogs
- Timeline, editing, AI analysis/editing, export, audio, effects, camera-specific logic, 360° classification
- Schema-version bump or migration; new dependencies; unrelated refactoring/cleanup

### Definition of Done

- Selection requires an active project and a current, available record; idempotent; deterministic failure messages.
- `activeMediaChanged` fires exactly once per real change and never on no-ops.
- Invariant holds: active id is empty or resolves to exactly one current record whose file was available at selection/restore time.
- Import never auto-selects; removal of the active record clears it; removal of others preserves it; new project clears it.
- Reopen restores a valid persisted id after normalization; dangling/invalid/unavailable ids are cleared deterministically.
- Save→reopen round trip preserves the active id and record identity.
- Original media never modified; media identity/dedup/availability/ordering/persistence unchanged.
- New tests pass; the 81-test regression suite remains green (suite now 90).
- App and tests build/pass offscreen; smoke unaffected; docs updated; one Git checkpoint; clean tree; no dependencies, no schema change.

### Verification Strategy

- New tests: set/clear semantics and idempotency; import never auto-selects; removal rules; new-project clearing; save→reopen round trip; open-time normalization with dangling/legacy/duplicate persisted ids; unavailable media cannot become active; MainWindow Set Active/label/marking.
- Regression: full suite offscreen via the existing build-and-test script.

### Architectural Boundaries / Risks

- `Application` remains the single owner of the active id and the media list; the UI only mirrors display state.
- Availability is a point-in-time snapshot at selection/restore; no watchers or real-time revocation.
- Additive key caveat (older readers drop the key on resave) is the documented `viewerState`/`media` precedent.
- `activeMediaItem()` returns a pointer into the internal list; it must not outlive a list mutation (accessor-only usage, covered by tests).

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 7 commit, parent `8c71edd`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 7 state. Do not begin automatically.

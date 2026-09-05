# Reelcraft — Next Task

## Phase 2 Status

- Phase 1 Desktop Application Foundation — complete and verified.
- Phase 2 Objective 1 — Viewer State Foundation — complete and verified (viewport orientation model, application ownership, UI readout/keyboard controls, JSON serialization and persistence, schema versioning).
- Phase 2 Objective 2 — Viewer Presentation Foundation — complete and verified at checkpoint `58cd561` (deterministic synthetic 360° test scene in `ViewerScene`; minimal presentation surface in `ViewerWidget` embedded in the desktop UI).
- Phase 2 Objective 3 — Viewer Camera Integration — complete and verified at checkpoint `fb7bb49` (deterministic `ViewerProjection` camera/view transform; camera-driven `ViewerWidget` presentation following the authoritative application-owned `ViewportState`; Application→UI wiring in `main.cpp`).
- Phase 2 Objective 4 — Real Media Foundation — complete and verified (`MediaItem` media records; `Application`-owned import/validation/deduplication; optional additive project `media` JSON section persisted on save and restored deterministically on reopen; Import Media UI; original files never modified).
- Phase 2 Objective 5 — Media Reference Availability and Open-Time Integrity — complete and verified (reopen-time revalidation of media references; deterministic list normalization/deduplication on open; unavailable-media status surfaced through the existing signal boundary).
- Automated suite: 73 passed, 0 failed. Working tree clean at the Objective 5 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 5: Media Reference Availability and Open-Time Integrity

Status: **Complete — implemented and verified (2026-09-04; automated suite 73 passed, 0 failed).**

### Objective

Complete the media-validation lifecycle begun in Objective 4: on project open, revalidate each restored media reference, normalize the restored list deterministically, and surface unavailable references — with no decode, playback, viewer, or schema changes.

### Scope (implemented)

- On open, each restored media reference is revalidated using the existing `MediaItem::referenceExists()` (point-in-time availability).
- `Application::restoreMediaFromJson()` normalizes deterministically: invalid records are dropped and duplicate media ids keep only the first occurrence, preserving order.
- `Application` exposes `hasUnavailableMedia()` / `unavailableMediaCount()`.
- `openProject()` emits exactly one deterministic `backgroundCompleted` status message only when a reopened project contains unavailable media.
- Import-time validation/deduplication, new-project clearing, and persistence round trips are unchanged.

### Explicit Non-Goals

- Decoding, probing, playback, timeline, editing, AI, export, audio, effects, camera-specific logic
- 360° classification/detection (conceptual only)
- Media removal/selection UI or any new dialogs
- Viewer camera/presentation/scene changes
- Schema-version bump or migration (no new persisted fields)
- Performance/GPU optimization; new dependencies; unrelated refactoring/cleanup

### Definition of Done

- Reopen with all media available: identical list, no unavailable media, no status message.
- Reopen after deletion/move of a referenced file: record retained, availability flagged, exactly one deterministic status message.
- Project files with duplicate media entries open deduplicated with stable order; re-save stays normalized.
- Invalid persisted entries still fall back safely; legacy/no-media projects behave exactly as before.
- Original media never modified (byte/SHA-256 invariant maintained by the Objective 4 test).
- New tests pass; the 68-test Objective 4 regression suite remains green (suite now 73).
- App and tests build/pass offscreen; documentation updated; one Git checkpoint; clean tree; no dependencies, no schema change.

### Verification Strategy

- New tests: silent identical reopen when available; unavailable flags + single status message after deleting a referenced file; unavailable after moving a referenced file; duplicate persisted entries normalized on open and re-save; new project clears unavailable state.
- Regression: full suite offscreen via the existing build-and-test script.

### Architectural Boundaries / Risks

- Availability is a point-in-time filesystem snapshot (paths remain machine-specific; Decision 006 caution).
- The conditional status message fires only when unavailable media exists, so existing success-path signal tests are unaffected.
- Media records are never removed because a file is missing; deterministic normalization is by media id (first occurrence), preserving order.
- No changes to `MediaItem`, `Project`, the viewer layer, `.pro` files, or the schema.

### Implementation Gate

This objective is complete at its checkpoint. The implementing agent for the next objective must re-read the project state, confirm one active objective, preserve the verified checkpoint (Objective 5 commit, parent `430c018`), and stop-and-ask on any conflict with the recorded architecture.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 5 state. Do not begin automatically.

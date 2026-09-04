# Reelcraft — Next Task

## Phase 2 Status

- Phase 1 Desktop Application Foundation — complete and verified.
- Phase 2 Objective 1 — Viewer State Foundation — complete and verified (viewport orientation model, application ownership, UI readout/keyboard controls, JSON serialization and persistence, schema versioning).
- Phase 2 Objective 2 — Viewer Presentation Foundation — complete and verified at checkpoint `58cd561` (deterministic synthetic 360° test scene in `ViewerScene`; minimal presentation surface in `ViewerWidget` embedded in the desktop UI).
- Phase 2 Objective 3 — Viewer Camera Integration — complete and verified at checkpoint `fb7bb49` (deterministic `ViewerProjection` camera/view transform; camera-driven `ViewerWidget` presentation following the authoritative application-owned `ViewportState`; Application→UI wiring in `main.cpp`).
- Phase 2 Objective 4 — Real Media Foundation — complete and verified (`MediaItem` media records; `Application`-owned import/validation/deduplication; optional additive project `media` JSON section persisted on save and restored deterministically on reopen; Import Media UI; original files never modified).
- Automated suite: 68 passed, 0 failed. Working tree clean at the Objective 4 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 4: Real Media Foundation

Status: **Complete — implemented and verified (2026-09-04; automated suite 68 passed, 0 failed).**

### Objective

Establish the smallest controlled foundation for importing and representing real media files while preserving Reelcraft's non-destructive architecture:

1. **Import/select real media files** — a user can choose a real media file through the desktop UI, and the application layer can import a media path headlessly.
2. **Validate media references** — an import is accepted only when it refers to an existing, readable, regular file; directories, missing files, and unreadable paths are rejected with a clear error.
3. **Record minimal media metadata** — each imported file is represented by a deterministic media record (stable id, stored path, file name, size, last-modified time, extension-derived format tag) without decoding or probing the file content.
4. **Preserve original media unchanged** — importing, saving, and reopening never modify the original file (byte/SHA-256 invariant).
5. **Deterministic project persistence/reopen** — imported media records are saved with the project and restored deterministically on reopen; projects without media behave exactly as today.
6. **Clear separation** — media management lives outside the viewer/playback and presentation layers; the viewer, camera, and presentation code are untouched.
7. **360° first-class, no camera-specific implementation** — the media record leaves room for future 360°/content classification metadata but performs no 360° detection or camera-specific logic now.
8. **Headless-testable logic** — media/project logic is QtCore-based and covered by automated offscreen tests using synthetic real files in temporary directories.

**Persistence constraint (no schema migration):** media records are persisted as an optional, additive `media` section inside the project JSON, mirroring the verified optional `viewerState` precedent: absent in legacy and older projects; written only when non-empty; readers that ignore unknown fields remain safe. No schema-version bump and no migration is planned. If the smallest correct implementation proves this additive approach is not safe and a schema-version change/migration is truly required, the implementing agent must stop and report rather than proceed.

### Scope

- A QtCore-only media record model (e.g., `MediaItem`): deterministic id derived from the canonical absolute path (stable across repeated imports), stored path, file name, size in bytes, last-modified UTC timestamp, and an extension-derived format tag; validation of the referenced file; equality; and an optional free-form attributes object that is the future home for content/360° classification metadata (no classification logic now).
- Application-owned media management with a clear media/application boundary: an import slot that validates and deduplicates (importing the same canonical path twice is idempotent), a deterministic media list with stable ordering, and error reporting through the existing application signal boundary (mirroring the save/open error handling).
- Project persistence/reopen following the verified `viewerState` pattern: the application serializes its current media list into the project's optional `media` section on save and restores/revalidates it on open; legacy and no-media projects are unaffected; invalid persisted media entries fall back deterministically without failing the open.
- Real file selection in the UI: an Import Media action using the existing injectable file-chooser pattern, wired through the existing signal/slot application boundary, with minimal status feedback.
- Automated tests using synthetic real files created in temporary directories (no real media assets, no decode).
- No new dependencies; the existing build-and-test workflow is preserved.

### Explicit Non-Goals

- Video decoding, demuxing, frame reading, or probing of any kind (no FFmpeg/ffprobe)
- Playback, preview, or presentation of media content
- Timeline, clips, tracks, or sequencing
- Editing engine, cuts, or edit decisions
- AI editing, analysis, or provider integration
- Export or render pipelines and format handling
- Audio processing or audio waveform/metadata extraction
- Effects, transitions, captions, or color
- Camera-specific hardware logic, adapters, metadata, stitching, or Insta360-specific behavior
- 360° detection, classification, reframing, or virtual-camera behavior
- Viewer camera, presentation, or scene changes (`ViewerWidget`, `ViewerProjection`, `ViewerScene`, `ViewportState` wiring are untouched)
- Schema-version bump or schema migration (see persistence constraint above)
- Performance optimization, GPU strategy, or production rendering architecture
- New dependencies. If a dependency appears necessary to satisfy this objective, stop and report instead of adding it.
- Unrelated refactoring or documentation cleanup

### Definition of Done

A task is complete only when:

- Import selection records validated media per scope: existing/readable/regular files accepted with deterministic id and metadata; missing, unreadable, and directory paths rejected with an error surfaced through the application boundary.
- Duplicate imports of the same canonical path are idempotent; media ordering is deterministic.
- Media records persist through project save and restore deterministically on reopen; projects without media behave exactly as before; legacy project files still load.
- Original media files are never modified: an automated test verifies byte and SHA-256 invariance across import, save, and reopen.
- Media management is clearly separated: no decode, playback, viewer, camera, or presentation code was touched or introduced.
- New automated tests pass; the pre-existing 55-test regression suite remains green.
- The application and tests build and pass through the existing workflow offscreen.
- No new dependencies and no schema migration were performed.
- Relevant documentation is updated (CURRENT_STATE / DEVELOPMENT_LOG / CHANGELOG per the established convention).
- A Git checkpoint exists and the working tree is clean.

### Verification Strategy

- **Unit tests:** media record validation and deterministic id stability (same canonical path → same id; distinct paths → distinct ids); metadata recorded correctly.
- **Application integration:** import success, duplicate/idempotent import, and error paths (missing/unreadable/directory) — mirroring the existing save/open error tests via the application signal boundary.
- **Persistence:** project save→reopen round trip with media, without media, and with legacy project files; invalid persisted `media` entries fall back safely and deterministically.
- **Media safety:** byte/SHA-256 invariance of the original file across import→save→reopen.
- **UI-level:** Import Media signal emission and the injectable file chooser, following the existing headless MainWindow test pattern.
- **Regression:** all 55 baseline tests remain green.
- **Offscreen run:** full suite via the existing build-and-test script.
- **Optional real-session smoke** via Termux:X11 after implementation, following the Phase 1 precedent.

### Architectural Boundaries

- Media records and application-level media management are QtCore-only and headless-testable; windowing/file-chooser concerns stay in the UI layer (injectable chooser, mirroring the open/save pattern).
- The application owns the authoritative media list; the `Project` object carries only the opaque, optional `media` JSON section (same pattern as the verified `viewerState` persistence) — no schema-version change.
- The viewer/presentation layers (`ViewerScene`, `ViewerProjection`, `ViewerWidget`, `ViewportState` ownership and wiring) are untouched.
- Original media is never written, moved, or deleted; editing remains non-destructive (Decision 002).
- No decoding/media-engine, AI, timeline, or export subsystem is introduced; 360° remains a first-class conceptual requirement without camera-specific implementation (Decision 003).

### Risks / Unknowns

- Media references are machine-specific absolute paths; reopening on another machine or after the file moves can make a reference unavailable. Validation on reopen must flag this without failing the open; cross-machine reference portability is a later concern (Decision 006 caution).
- The additive `media` section keeps the current schema version intentionally. Older v2 readers that ignore unknown fields are safe but could drop the section if they resave; acceptable now only because media is auxiliary, and the persistence constraint above requires a stop-and-report if this becomes unsafe.
- Deterministic ids derived from canonical paths are stable for a given file but change if the file is renamed/moved; duplicate detection is defined per canonical path.
- Content type and 360° status cannot be determined without decoding; only factual metadata is recorded, plus the attributes placeholder. No guessing.
- A file may change or disappear between import and reopen; revalidation treats it as recorded-but-unavailable rather than corrupting the project.
- MainWindow additions must not regress existing tests; existing object names, signals, and behavior must be preserved.
- Scope-creep risk toward decoding/playback/probing; non-goals must be enforced.

### Implementation Gate

This objective is defined and recorded only. Implementation begins in a separate controlled task only after this definition is accepted. The implementing agent must re-read the project state, confirm this objective and its non-goals, preserve the one-active-objective rule and the verified checkpoint (`fb7bb49`), and stop-and-ask on any conflict with the recorded architecture — including before adding any dependency and before any schema-version change or migration.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 4 state. Do not begin automatically.

# Reelcraft — Next Task

## Phase 2 Status

Phase 2 clean-checkout re-verification complete — 2026-09-04 (commit `919eef6`; automated suite 43 passed, 0 failed).

The completed Phase 2 viewer-state foundation work (viewport orientation model, application ownership, UI readout/keyboard controls, JSON serialization and persistence, schema versioning) is documented in `CURRENT_STATE.md`.

Phase 2 Objective 2 is active. Its first subtask — **Minimal Viewer Presentation Surface** — is implemented and verified (2026-09-04; automated suite now 48 passed, 0 failed); see `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md`.

## Active Objective — Phase 2, Objective 2: Viewer Presentation Foundation

Status: **Implementation in progress — first subtask (Minimal Viewer Presentation Surface) complete and verified. The remaining Objective 2 subtask (ViewportState→camera connection) is not started and must not begin automatically.**

Defined: 2026-09-04.

### Objective

Establish the smallest safe presentation foundation on the path toward the future interactive 360° viewer:

1. **Deterministic 360° test scene** — a synthetic, non-media scene (e.g., spherical/equirectangular test markers or a labeled 3D grid) with known ground truth.
2. **Viewer presentation layer** — a minimal viewer presentation surface/canvas in the desktop UI that renders the test scene from a viewer camera.
3. **ViewportState-to-camera connection** — the viewer camera is driven by the existing application-owned `ViewportState` so yaw/pitch/roll/FOV changes are reflected in the presented view.
4. **Yaw/pitch/roll/FOV behavior** — orientation and field-of-view semantics consistent with the verified `ViewportState` model (yaw/roll normalized to [−180, 180), pitch clamped to [−90, 90], FOV clamped to [20, 140]).
5. **Automated tests** — deterministic tests for scene ground truth, camera/scene math, and the ViewportState→camera connection, plus an offscreen presentation check where the environment permits.

### Scope

- A synthetic, deterministic 360° test scene only (no media files).
- A camera/presentation model whose behavior is testable without a windowing platform.
- A minimal UI presentation surface wired through the existing signal/slot application boundary.
- Reuse of the existing `ViewportState` as the single source of orientation/FOV; no duplicate state.
- Automated tests added to the existing Qt Test suite, runnable offscreen.
- The build/test workflow must continue to run through the existing build-and-test workflow without new external dependencies.

### Explicit Non-Goals

- Real media import, decoding, frames, or playback of any kind
- Video processing, codecs, FFmpeg, or proxies
- Timeline, clips, tracks, or sequencing
- AI editing, analysis, or provider integration
- Export or render pipelines and format handling
- Audio, captions, effects, transitions, or color
- Camera-specific logic, adapters, metadata, stitching, or Insta360-specific behavior
- 360° reframing, keyframed viewpoints, or virtual-camera automation
- Performance optimization, GPU strategy, or production rendering architecture
- Project schema/persistence changes (no new fields)
- Dependencies, packaging, or platform additions

### Definition of Done

A task is complete only when:

- The deterministic 360° test scene and camera behavior exist per scope.
- The existing `ViewportState` is connected as the camera controller, and yaw/pitch/roll/FOV behavior is verified.
- A minimal viewer presentation surface renders the scene deterministically and respects the application/UI boundary.
- New automated tests pass and the full regression suite remains green.
- The application and tests build and pass through the existing workflow offscreen.
- Relevant documentation is updated (CURRENT_STATE / DEVELOPMENT_LOG / CHANGELOG per the established convention).
- No new dependencies were introduced, and no out-of-scope system was started.
- A Git checkpoint exists and the working tree is clean.

### Verification Strategy

- **Unit tests:** scene ground truth; camera math, including normalization/clamping consistent with `ViewportState`.
- **Integration:** `ViewportState` changes drive the expected camera orientation/FOV.
- **UI-level:** the presentation surface is connected through signals; all existing 43 tests still pass.
- **Offscreen run:** full suite via the existing build-and-test script; any rendered output is verified through deterministic checks (e.g., image invariants) rather than visual assertion.
- **Optional real-session smoke** via Termux:X11 after implementation, following the Phase 1 precedent.

### Architectural Boundaries

- `Application` keeps owning `ViewportState`; the presentation layer only displays it.
- Scene/camera math remains independent of QtWidgets where feasible for headless testing (consistent with the platform-neutral core rule); windowing/rendering surface stays in the UI layer.
- AI and media boundaries are untouched; no access to original media.
- The Project model and project schema are untouched.

### Risks / Unknowns

- The offscreen QPA plugin may constrain OpenGL; a software/deterministic presentation path may be required (environment limitation already recorded in KNOWN_ISSUES).
- The concrete rendering approach (e.g., painter-based projection vs. OpenGL widget) is intentionally unselected and must be chosen on evidence during implementation, recorded as a decision if material.
- UI layout changes risk regressing existing MainWindow tests; existing object names and signals must be preserved.
- Scope-creep risk toward reframing, playback, or real media; non-goals must be enforced.

### Implementation Gate

This objective is defined and recorded only. Implementation begins in a separate controlled task only after this definition is accepted. The implementing agent must re-read the project state, confirm this objective and its non-goals, preserve the one-active-objective rule and the verified checkpoint (`919eef6`), and stop-and-ask on any conflict with the recorded architecture.

# Reelcraft — Next Task

## Phase 2 Status

- Phase 1 Desktop Application Foundation — complete and verified.
- Phase 2 Objective 1 — Viewer State Foundation — complete and verified (viewport orientation model, application ownership, UI readout/keyboard controls, JSON serialization and persistence, schema versioning).
- Phase 2 Objective 2 — Viewer Presentation Foundation — complete and verified at checkpoint `58cd561` (deterministic synthetic 360° test scene in `ViewerScene`; minimal presentation surface in `ViewerWidget` embedded in the desktop UI).
- Phase 2 Objective 3 — Viewer Camera Integration — complete and verified (deterministic `ViewerProjection` camera/view transform; camera-driven `ViewerWidget` presentation following the authoritative application-owned `ViewportState`; Application→UI wiring in `main.cpp`).
- Automated suite: 55 passed, 0 failed. Working tree clean at the Objective 3 checkpoint commit.

See `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md` for the verified record.

## Active Objective — Phase 2, Objective 3: Viewer Camera Integration

Status: **Complete — implemented and verified (2026-09-04; automated suite 55 passed, 0 failed).**

### Objective

Connect the existing application-owned `ViewportState` to the existing `ViewerWidget` presentation layer so the deterministic synthetic 360° test scene is presented through a viewer camera whose orientation (yaw/pitch/roll) and field of view follow `ViewportState`:

1. **Camera/view transform** — a deterministic transform that maps a scene marker direction (yaw/pitch) through a camera defined by yaw/pitch/roll/FOV into view/screen coordinates, suitable for headless unit testing.
2. **ViewportState→camera behavior** — yaw/pitch/roll/FOV behavior consistent with the already-verified `ViewportState` semantics (yaw/roll normalized to [−180, 180), pitch clamped to [−90, 90], FOV clamped to [20, 140]).
3. **Presentation consumes the camera** — `ViewerWidget` renders the existing synthetic scene from the camera view (orientation and FOV honored) rather than a fixed identity unwrap.
4. **Wiring through the existing boundary** — changes to the application-owned `ViewportState` reach the presentation through the established signal/slot application/UI boundary. `Application` remains the single owner of viewer state; no duplicate orientation state.
5. **Automated tests** — deterministic tests for the camera/view transform, the ViewportState→viewer integration, and an updated offscreen presentation check.

The existing synthetic 360° scene (`ViewerScene`) is preserved unchanged as the deterministic test environment.

### Scope

- A camera/view transform model whose behavior is testable without a windowing platform and independent of QtWidgets where feasible.
- Camera-driven presentation of the existing deterministic scene in `ViewerWidget` (default `ViewportState` must remain consistent with the verified defaults: yaw/pitch/roll 0, FOV 90, FRONT marker centered at identity).
- Wiring from the application-owned `ViewportState` to the presentation via the existing signal/slot application boundary.
- Reuse of `ViewportState` as the single source of orientation/FOV; no duplicate state.
- Automated tests added to the existing Qt Test suite, runnable offscreen.
- Updates to the existing deterministic render test only where the intended camera-driven presentation change requires it; expectations must remain deterministic and verified.
- The build/test workflow must continue to run through the existing build-and-test workflow without new external dependencies.

### Explicit Non-Goals

- Real media import, decoding, frames, or playback of any kind
- Video processing, codecs, FFmpeg, or proxies
- Timeline, clips, tracks, or sequencing
- AI editing, analysis, or provider integration
- Export or render pipelines and format handling
- Audio, captions, effects, transitions, or color
- Camera-specific hardware logic, adapters, metadata, stitching, or Insta360-specific behavior
- 360° reframing, keyframed viewpoints, or virtual-camera automation
- Project schema/persistence changes (no new fields). If the defined objective turns out to require a schema change, stop and explain rather than proceeding.
- Performance optimization, GPU strategy, or production rendering architecture
- New dependencies. If a dependency appears necessary to satisfy this objective, stop and report instead of adding it.
- Unrelated documentation cleanup
- Any change to `ViewerScene` or its deterministic ground truth

### Definition of Done

A task is complete only when:

- The camera/view transform exists per scope and is verified by deterministic unit tests (marker directions mapped through known camera yaw/pitch/roll/FOV to expected view coordinates).
- `ViewerWidget` presents the existing synthetic scene through the camera, honoring yaw/pitch/roll/FOV consistent with the verified `ViewportState` semantics.
- Application-owned `ViewportState` changes drive the presented view through the existing signal/slot boundary; no duplicate orientation state exists.
- New automated tests pass, including ViewportState→viewer integration; the full test suite is green.
- The pre-existing 48-test regression suite remains green; any update to camera-affected presentation test expectations is deliberate, deterministic, and documented in the change.
- The application and tests build and pass through the existing workflow offscreen.
- No new dependencies, no schema changes, and no out-of-scope system were started.
- Relevant documentation is updated (CURRENT_STATE / DEVELOPMENT_LOG / CHANGELOG per the established convention).
- A Git checkpoint exists and the working tree is clean.

### Verification Strategy

- **Unit tests:** camera/view transform math — FRONT centered at default orientation; yaw/pitch changes move known markers to expected view positions (e.g., yaw +90 brings a side marker toward center); roll rotates the view plane; FOV changes coverage/apparent placement; behavior consistent with ViewportState normalization/clamping.
- **Integration:** `Application` adjust slots (`adjustViewportYaw/Pitch/Roll/FieldOfView`, reset) result in the expected presented camera orientation/FOV through the existing wiring.
- **UI-level:** the viewer surface reflects application-owned state; existing MainWindow readout tests continue to pass.
- **Regression:** all 48 baseline tests remain green; presentation-test expectation updates are limited to the camera-driven behavior change and are deterministic.
- **Offscreen run:** full suite via the existing build-and-test script; rendered output is verified through deterministic pixel/image invariants rather than visual assertion.
- **Optional real-session smoke** via Termux:X11 after implementation, following the Phase 1 precedent.

### Architectural Boundaries

- `Application` keeps owning `ViewportState`; the presentation layer only displays its effect and never becomes a second source of orientation state.
- Camera/view math remains independent of QtWidgets where feasible for headless testing (consistent with the platform-neutral core rule); the windowing/rendering surface stays in the UI layer.
- `ViewerScene` and the deterministic test scene are unchanged.
- AI, media, project-model, and persistence boundaries are untouched; no access to original media; no schema change.

### Risks / Unknowns

- The camera-driven presentation intentionally changes how the scene is drawn; the existing deterministic render test may require a justified, deterministic update (baseline count otherwise unchanged).
- The exact projection model (e.g., pinhole perspective with chosen FOV semantics vs. equirectangular reprojection) is intentionally unselected; it must be chosen on evidence during implementation, kept deterministic/offscreen-safe, and recorded as a decision if material.
- Roll and FOV semantics must be defined precisely and consistently with `ViewportState` (which stores them but does not define presentation meaning); ambiguity here must be resolved before implementation, not guessed.
- The offscreen QPA plugin may constrain rendering; a software/deterministic presentation path is required (environment limitation already recorded in KNOWN_ISSUES).
- Wiring must avoid state drift: exactly one owner of orientation/FOV (`ViewportState` in `Application`).
- Scope-creep risk toward reframing, playback, or real media; non-goals must be enforced.

### Implementation Gate

This objective is defined and recorded only. Implementation begins in a separate controlled task only after this definition is accepted. The implementing agent must re-read the project state, confirm this objective and its non-goals, preserve the one-active-objective rule and the verified checkpoint (`58cd561`), and stop-and-ask on any conflict with the recorded architecture — including before adding any dependency or making any schema change.

## Next Logical State

Define the next smallest Phase 2 development objective from the verified Phase 2 Objective 3 state. Do not begin automatically.

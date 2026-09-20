# Reelcraft — Architectural Decisions

## Purpose

This document records significant architectural and development decisions made for Reelcraft.

The purpose is to preserve the reasoning behind decisions so that future development can continue consistently without relying on chat history.

Decisions should be changed only when new evidence, testing, requirements, or constraints justify doing so.

---

## Decision Format

Each significant decision should document:

- Decision
- Status
- Context
- Rationale
- Consequences

Superseded decisions should remain in this document for historical traceability.

---

# Decision 013 — Qt 6 as the Phase 1 Desktop Foundation

**Status:** Accepted

## Context

Reelcraft has completed the technology/runtime evaluation and is ready to establish the smallest executable Phase 1 application foundation.

The requirements call for a desktop-first application capable of supporting responsive UI, background processing, future GPU-accelerated presentation, interactive 2D/3D content, and first-class 360° workflows.

The technology evaluation considered Qt 6, Electron, and Tauri 2. No candidate was treated as a permanent dependency solely because it was evaluated.

## Decision

Qt 6 will be used as the current desktop application foundation for Phase 1.

Qt will provide the application/UI foundation and GPU-capable presentation layer while remaining separate from:

- AI reasoning and provider integrations
- Project/edit-state ownership
- Deterministic media processing
- Rendering/export implementation
- Storage/database implementation
- Local/cloud workload allocation

The selection is an implementation decision, not a new product requirement. The architecture must preserve clear boundaries so that downstream systems are not unnecessarily coupled to Qt.

## Rationale

Qt 6 provides a strong native foundation for Reelcraft's requirements involving GPU-capable desktop presentation, interactive 2D/3D rendering, video integration, and the future first-class 360° viewport.

It also provides a mature cross-platform application framework with native multimedia and graphics capabilities, reducing the need to construct critical video/graphics integration primarily through a web-rendering layer.

The decision is supported by the current requirements and technology evaluation, while recognizing that actual implementation, compatibility, licensing, performance, and maintainability evidence may justify revisiting the choice.

## Consequences

Phase 1 implementation may introduce Qt 6 dependencies.

The project must verify Qt installation, compiler/toolchain compatibility, licensing implications, build reproducibility, application startup, UI responsiveness, and integration boundaries during implementation.

If implementation evidence demonstrates that Qt 6 is unsuitable for Reelcraft's actual requirements, the decision may be revisited through the documented architecture-evolution and technology-evaluation process.

---

# Decision 001 — AI and Media Execution Must Remain Separate

**Status:** Accepted

## Context

Reelcraft is an AI-native video editor. AI will eventually analyze footage, understand creator intent, and determine editing decisions.

AI systems should not directly manipulate source media because doing so would make behavior difficult to validate, reproduce, test, and control.

## Decision

AI systems will generate structured editing instructions.

A deterministic media-processing subsystem will validate and execute those instructions.

The intended boundary is:

AI reasoning
 structured edit plan
 validation
 deterministic execution

## Rationale

This separation provides:

- Predictability
- Testability
- Reproducibility
- Easier debugging
- Provider independence
- Human review
- Safer AI integration
- Non-destructive editing

## Consequences

AI models can change without requiring the media engine to change.

The media engine can also evolve independently of AI providers.

---

# Decision 002 — Editing Must Be Non-Destructive

**Status:** Accepted

## Context

Creators need to preserve original footage while experimenting with AI-generated and manually modified edits.

## Decision

Original source media must never be modified by normal Reelcraft editing operations.

Editing decisions will be represented separately from source media.

## Rationale

This protects creator footage and allows:

- Revisions
- Undo/redo
- Alternative edits
- AI experimentation
- Re-rendering
- Different export formats
- Project portability

## Consequences

Reelcraft requires a structured project/edit representation rather than destructive file modification.

---

# Decision 003 — 360° Video Is a First-Class Media Type

**Status:** Accepted

## Context

Reelcraft is intended to support 360° creators and immersive video workflows.

Treating 360° video as ordinary flat video would limit future capabilities.

## Decision

360° video will be represented and processed as a first-class media type.

The architecture must support:

- Equirectangular media
- 360° metadata
- Orientation
- Virtual-camera positioning
- Reframing
- Keyframed viewpoints
- 360° export
- Flat-video extraction from 360° sources

## Rationale

360° editing requires concepts that do not exist in ordinary flat-video editing.

## Consequences

360° requirements must be considered when designing the media model, viewer, timeline, analysis systems, and rendering pipeline.

---

# Decision 004 — Camera-Specific Behavior Must Use Adapters

**Status:** Accepted

## Context

The Insta360 X5 is the current development camera, but Reelcraft must eventually support additional cameras and media sources.

## Decision

Camera-specific behavior will be isolated behind camera/media adapter boundaries.

## Rationale

The core Reelcraft architecture must not become dependent on one manufacturer's metadata, file structure, or processing behavior.

## Consequences

Adding support for another camera should primarily require implementing or extending an adapter rather than rewriting the core system.

---

# Decision 005 — AI Providers Must Be Replaceable

**Status:** Accepted

## Context

AI capabilities and providers will continue to evolve.

Locking Reelcraft to one provider would create unnecessary technical and business dependency.

## Decision

AI capabilities will be accessed through provider/model abstraction boundaries.

The architecture should support:

- Multiple providers
- Multiple models
- Local models
- Cloud models
- Hybrid processing

## Rationale

This allows Reelcraft to adapt as model capabilities, pricing, availability, hardware, and privacy requirements change.

## Consequences

Provider-specific implementation details must remain outside the core project/edit model.

---

# Decision 006 — Project State Must Be Portable

**Status:** Accepted

## Context

Reelcraft may eventually run across different hardware and processing environments.

## Decision

Project state must remain conceptually independent of the machine performing the processing.

## Rationale

A project should be recoverable, transferable, and reproducible across supported environments.

## Consequences

Project metadata, edit decisions, analysis information, and processing references require a defined persistent representation.

---

# Decision 007 — Human Review Remains the Final Authority

**Status:** Accepted

## Context

AI-generated edits can be useful but may be incorrect or inconsistent with creator intent.

## Decision

Creators retain final authority over AI-generated editing decisions.

AI suggestions must remain reviewable and adjustable.

## Rationale

Reelcraft is intended to augment creators rather than remove creator control.

## Consequences

The UI and edit model must distinguish AI-generated decisions from creator modifications where appropriate.

---

# Decision 008 — Development Must Be Incremental and Checkpointed

**Status:** Accepted

## Context

Reelcraft is a complex system involving media processing, AI, UI, storage, and potentially significant computational workloads.

Large uncontrolled changes increase the risk of regressions and make failures difficult to diagnose.

## Decision

Development will proceed through small logical tasks.

Each active task must have:

- A clear objective
- Explicit scope
- Definition of Done
- Verification
- Regression testing where applicable
- Documentation updates
- Git checkpointing

## Rationale

Small verified changes make the system easier to understand, recover, and maintain.

## Consequences

Large feature requests must be decomposed into smaller development tasks before implementation.

---

# Decision 009 — Inspect Before Changing

**Status:** Accepted

## Context

AI agents and developers may work on Reelcraft at different times.

Changing files without first understanding their current state risks overwriting valid work.

## Decision

Before modifying existing project files, the current state must be inspected.

## Rationale

Persistent project state is more authoritative than assumptions or previous chat instructions.

## Consequences

AI agents must inspect relevant files, Git state, architecture, and current task state before making changes.

---

# Decision 010 — Documentation Is Part of the Project Source of Truth

**Status:** Accepted

## Context

Multiple AI agents may contribute to Reelcraft over time.

Chat history alone is not a reliable development record.

## Decision

Persistent Markdown documentation is part of the project's source of truth.

Important project state, architecture, decisions, known issues, development history, and handoff information must be recorded in the repository.

## Rationale

This allows future development to continue without reconstructing project state from conversations.

## Consequences

Documentation updates are part of the Definition of Done for meaningful work.

---

# Decision 011 — Technology Choices Must Be Evidence-Based

**Status:** Accepted

## Context

The project is currently at the architecture/planning stage.

Prematurely locking the project to frameworks, vendors, or infrastructure could create unnecessary constraints.

## Decision

Technology choices will be evaluated when implementation requirements justify them.

Candidates may be documented without becoming permanent dependencies.

## Rationale

Requirements, performance measurements, compatibility, cost, and maintainability should drive technology selection.

## Consequences

The current architecture intentionally leaves several implementation choices unresolved.

---

# Decision 012 — Architecture Is Allowed to Evolve

**Status:** Accepted

## Context

Some architectural assumptions cannot be fully validated until implementation and testing begin.

## Decision

Reelcraft architecture may evolve when implementation evidence, performance testing, security requirements, creator workflow requirements, or other significant findings justify a change.

Significant changes must be recorded in this document and reflected in project history.

## Rationale

The architecture should be durable without becoming rigid.

## Consequences

Architectural changes require explicit documentation and verification rather than silent modification.

---

# Decision Change Procedure

When a significant architectural decision changes:

1. Identify the existing decision.
2. Record why it is being reconsidered.
3. Document evidence supporting the change.
4. Record the new decision.
5. Identify affected systems and documentation.
6. Update relevant architecture documentation.
7. Test affected functionality when implementation exists.
8. Record the change in project history.
9. Create a Git checkpoint.

Never silently overwrite architectural history.

---

# Decision 014 — Platform Strategy: Desktop First-Class, Android Planned Future

**Status:** Accepted

## Context

Phase 1 uses Qt 6 as the desktop application foundation. Current development occurs in an Android/Termux/Ubuntu environment. This environment is not a native Android deployment target and must not be treated as evidence of Android support. Reelcraft is desktop-first per REQUIREMENTS.md.

## Decision

Desktop is the first-class supported platform at the current stage.

Android is a planned future platform, not a current product target.

Termux/Ubuntu on Android is a development environment only and does not constitute native Android support.

## Rationale

- Desktop-first requirements remain authoritative.
- The current development environment is a Linux userspace hosted by Android, not native Android Qt deployment.
- QtCore/QtWidgets separation already provides a useful portability boundary.
- Expanding Android support now would be scope creep and unsupported by evidence.

## Consequences

- Phase 1 remains desktop-only.
- No Android SDK/NDK, Termux:X11, or native Android packaging work is authorized.
- Future architecture work should preserve platform-independent core boundaries.
- Qt Widgets is not permanently committed as the mobile/UI implementation solely because it is used in Phase 1.
- Android/Termux is recorded as development-only and is not representative of desktop or Android performance.


## Expanded Platform Classification — 2026-09-03

The platform strategy from Decision 014 is extended without creating a new decision number.

Verified current platform classification:

- Linux desktop: first-class / currently verified
- Windows desktop: planned future desktop platform
- macOS: planned future desktop platform
- Android: planned future platform
- iOS: planned future consideration
- iPadOS: planned future consideration
- Termux/Ubuntu: development environment only

Desktop remains the current first-class product priority.

This classification does not authorize Apple SDKs, toolchains, targets, dependencies, Android implementation, UI replacement, Termux:X11 configuration, or native Apple/Android builds.


# Decision 015 — FFmpeg-CLI Decode Adapter for Single-Frame Preview

**Status:** Accepted (2026-09-05, Objective 9)

## Context

Phase 2 Objective 9 requires presenting a real decoded frame from the active media record through the verified EquirectView pixel path. The project has no decode capability and no decode dependency. Qt6Multimedia is not installed in the current proot/Termux development environment; the external `ffmpeg`/`ffprobe` binaries ARE available on the host PATH. A full media engine and production playback are later-phase concerns.

## Decision

For the Objective 9 single-frame presentation foundation, decode via the external `ffmpeg` CLI: `FrameExtractor` invokes ffmpeg with QProcess to emit one PNG frame on stdout (`-frames:v 1 -f image2 -c:v png pipe:1`), parsed in-process to a QImage. ffmpeg is never linked; the executable is resolved via the `REELCRAFT_FFMPEG` environment override then `QStandardPaths::findExecutable`, and its availability is feature-detected.

## Alternatives Considered

- Qt6Multimedia module (requires installing qt6-multimedia-dev in the environment and `QT += multimedia`; backend viability offscreen/Termux unproven) — deferred.
- Linked FFmpeg libraries (`libavcodec-dev`, etc.) — full dependency; deferred.
- Defer decode entirely (decode-free objectives only) — rejected: leaves the media→viewer loop unclosed.

## Rationale

- No `.pro`/system dependency change is required for this foundation slice.
- The decode behavior is isolated behind a replaceable seam (`FrameExtractor`): a future media engine or Qt/FFmpeg-linked backend can replace the implementation without changing callers or the viewer contract.
- Deterministic, headless-testable; tests skip cleanly when ffmpeg is unavailable.

## Consequences

- Objective 9 introduces reliance on an external executable in the development environment (recorded limitation; feature-detected and injectable).
- Playback, streaming, audio, and multi-frame pipelines remain explicitly out of scope and are not authorized by this decision.
- A production media-engine decision is deferred; this does not commit Reelcraft to shipping with an ffmpeg-CLI architecture.
- Original media files are never modified by the decode path.


# Decision 016 — Phase 2 Scope: "360 Viewer Review & Navigation" (Playback Deferred)

**Status:** Accepted (2026-09-06)

## Context

Phase 2 reached Objective 13 with a complete, verified review pipeline (media import/management/selection, availability and active-media contracts, deterministic single-frame decode, time stepping/seek, keyboard and pointer orientation control, flat/equirectangular projection handling, and consistent viewer/project state). A read-only Phase 2 audit showed the roadmap title ("360° Viewer: Playback and orientation control") describing a capability — continuous playback — that every Phase 2 objective had deliberately gated out (audio, ffprobe probing, and continuous playback were each deferred during Objectives 9–12 approvals). The Phase 2 roadmap wording therefore no longer matched the executed, verified scope.

## Decision

Phase 2 is officially redefined as:

**"Phase 2 — 360 Viewer Review & Navigation"**

In scope for Phase 2:
- Import and manage real media.
- Select active media.
- Deterministic single-frame preview.
- Time stepping/seek.
- Look-around orientation using keyboard and pointer controls.
- Correct flat/equirectangular presentation.
- Consistent viewer/project state.

Explicitly deferred (belong to a future playback/media-engine phase):
- Continuous/paced playback.
- Duration-aware playback transport (and ffprobe-based duration metadata).
- Audio.
- The future production media/player engine.

## Rationale

- The delivered review pipeline provides the product's near-term value (import, select, and review footage frame-by-frame with orientation control) and is fully verified.
- Continuous playback requires pacing, duration metadata, and a frame-stream decode path; a per-frame ffmpeg subprocess is not a viable pacing substrate, and a real player/decoder is media-engine territory (Phase 3 direction) that would require a new dependency/subsystem decision.
- Deferring keeps Phase 2's architectural boundary clean: review now, player/engine later, without reworking Phase-2 contracts.

## Consequences

- The Phase 2 milestone wording is updated in MASTER_GUIDE §9 (Roadmap) and referenced in project state documentation; the deferral is intentional and recorded, not an unfinished omission.
- `FrameExtractor` remains the review decode seam for single-frame stepping/seek. It is stable and replaceable; the future playback/media-engine phase may introduce a streaming/linked decoder behind the same or an engine-level boundary without altering Phase-2 viewer/media contracts.
- Time stepping/seek (Objective 10 position state and step controls), projection routing (Objective 12), and the pointer/keyboard orientation surface remain the intentional interim time/orientation model.
- A future playback/media-engine phase may reopen this decision; it should reuse the Objective 10 position state and transport surface, and will require its own dependency/architecture decision (linked FFmpeg vs QtMultimedia vs ffmpeg streaming subprocess) before implementation.
- No schema, dependency, source, or test changes result from this decision; it is a scope/boundary record.


# Decision 017 — Phase 3 Opening Architecture: Persistent FFmpeg Streaming Subprocess; Linked FFmpeg & QtMultimedia Deferred

**Status:** Accepted (2026-09-06, Phase 3 Objective 1 — documentation only)

## Context

Phase 2 formally closed at `dcd876b`. A read-only Phase 3 opening feasibility discovery was completed and approved. Verified environment facts: Qt 6.10.2 (proot) with QtMultimedia absent; FFmpeg 8.1.2 binaries present (Termux) with development headers/libs present only in the Termux tree (bionic runtime, not linkable into the proot-glibc Qt application); therefore persistent-subprocess use of the existing FFmpeg CLI is feasible today, while linked FFmpeg libraries and QtMultimedia would require new package installation that is deferred.

## APPROVED decisions

- **Phase 3 opening architecture:** use a persistent FFmpeg streaming subprocess behind a replaceable media/player seam. This subprocess approach is the currently feasible implementation path in this environment.
- **FrameExtractor classification:** FrameExtractor remains the deterministic single-frame preview/validation seam. It is NOT to be retrofitted into the permanent continuous-playback engine.
- **Objective 10 contract:** the existing public/application-level preview-time contract (requested-time semantics, success-only position updates, step/floor and -ss keyframe semantics, reset rules, beyond-end deterministic failure) is preserved. Playback must extend this contract rather than replace or silently redefine it.
- **Architecture boundary:** decode/media-source seam owns media decoding and frame/segment delivery; player/timing subsystem owns playhead, playback state, rate, pacing, and clock; Application owns active-media/application state and orchestrates player lifecycle; Viewer owns presentation only; future timeline/editor/AI systems remain above Application and must not reach directly into decoding.

## DEFERRED decisions

- **Linked FFmpeg libraries (libavformat/libavcodec/libavutil/libswscale):** deferred. Do not install FFmpeg development packages as part of this decision; revisit only when a real desktop/deployment dependency decision requires it.
- **QtMultimedia:** deferred. Do not install QtMultimedia. Current feasibility discovery found it absent and judged it a weaker fit for Reelcraft's deterministic frame-oriented architecture.

## FUTURE decision gates

- **ffprobe duration metadata:** explicitly reopened for Phase 3 and requires its own decision/objective (NOT implemented now). The future decision must evaluate: whether duration is required for playback UX; ffprobe availability; deterministic parsing; malformed/unsupported media behavior; caching/persistence implications; whether duration should be persisted or derived; interaction with Objective 10's beyond-end behavior; and impact on seeking and transport UI.
- **Production engine replacement:** a future linked-FFmpeg or Qt engine may replace the subprocess seam behind the same boundary when a real desktop/deployment target demands it (replaceable seam, reuses Objective 10 contract and Objective 15 fixtures).

## Intentionally NOT decisions (implementation-detail planning for later feasibility/implementation objectives)

Exact FFmpeg command line; rawvideo vs image2pipe transport; frame-pump implementation; clock implementation; buffering strategy; cache design; threading model; exact playback-rate implementation; audio architecture.

## Consequences

- Phase 3 is marked OPEN with the approved opening architecture; no media-engine/playback/duration/audio implementation exists and none is authorized by this decision.
- No source, test, build, schema, dependency, or Objective 10 behavior changes result from this decision; it is a scope/boundary and dependency-stance record.


# Decision 018 — 360 Reframing Engine: Structured Plan Boundary, Replaceable Frame Provider, Deterministic Renderer

**Status:** Accepted (2026-09-17, 360 Reframing Objective 1)

## Context

The human-approved product priority is a working AI-native 360 editing pipeline: 360 source -> scene/target understanding -> request interpretation -> virtual-camera decision -> deterministic reframing -> flat video output. The project already had (Phase 2/3) the established camera conventions (`ViewportState`/`ViewerProjection`/`EquirectView`), a single-frame FFmpeg decode seam (`FrameExtractor`), a streaming frame seam (`FrameSource`/`FfmpegFrameSource`), and a player/timing foundation, but no structured reframing representation and no deterministic execution from decisions to rendered output. `AI_EDIT_CONTRACT.md` and `ARCHITECTURE.md` mandate a structured AI-to-media boundary, but no concrete reframing schema or engine existed.

## Decisions

- A **structured, JSON-serializable, validated `ReframePlan`** (with ordered `CameraKeyframe` values) is the boundary between reframing decisions and media execution. It is versioned, inspectable, modifiable, and always validated before execution.
- The **camera path is a pure deterministic function** (`CameraPath::stateAt`): shortest-path yaw interpolation, bounded pitch/roll/FOV, hold/linear interpolation, and deterministic hold outside the keyframe range. It reads no clock, media, or model.
- Media decoding is behind a **replaceable `ReframeFrameProvider` seam**. The first production implementation uses the existing FFmpeg-CLI single-frame seam (`FfmpegSeekFrameProvider`); a future streaming/linked-FFmpeg provider can replace it without changing the engine.
- Rendering is a **deterministic executor** (`ReframeRenderer`) composing the provider, `CameraPath`, and the existing `EquirectView`; the encoder is a thin FFmpeg image-sequence step. The same plan and source produce the same frames.
- Natural-language requests are interpreted by a **replaceable, deterministic `ReframeIntent` boundary** (`ReframeIntentParser`). It recognizes aspect/platform, time ranges, camera directions, and subject references. **Unresolved subject references are reported, never fabricated**; subject resolution (detection/tracking) is a separate future capability. An AI provider can later produce the same structured intent.
- Source media remains read-only; rendering writes only derived outputs.

## Consequences

- The decision layer and deterministic executor are separable and independently testable: 26 reframing tests plus a real FFmpeg end-to-end pipeline test; full suite 180 passed / 0 failed / 0 skipped.
- `EquirectView::MaxOutputWidth` remains a viewer-presentation cap only; the renderer supports arbitrary output dimensions.
- Target detection/tracking, speaker/dialogue analysis, content-based cut selection, reframe-plan persistence in the project schema, UI integration, and AI-provider integration remain future objectives; the reframe engine is deliberately not yet wired into `Application`/`MainWindow`.
- This decision does not change Phase 3's media-engine boundary (Decision 017); it adds the editing/reframing layer above the existing media seams.


# Decision 019 — Target Resolution: Tangent-View Detection, Replaceable Detector, Deterministic Spherical Tracker

**Status:** Accepted (2026-09-17, 360 Reframing Objective 2)

## Context

360 Reframing Objective 1 delivered the deterministic reframing engine but no way to determine where a target is in the footage. A normal 2D detector run on a full equirectangular frame is unreliable near the poles and the 0/360 degree seam because of projection distortion. The objective required "eyes" that map detections into the existing spherical camera contract while keeping the computer-vision layer replaceable and licensing-clean.

## Decisions

- **Detect in overlapping perspective (tangent) views, then reproject.** `EquirectViewPlan` deterministically covers the sphere with pinhole views rendered by the existing `EquirectView`; `EquirectProjection` maps detections back to spherical directions using the exact inverse camera model; spherical non-maximum suppression merges overlapping detections. This avoids direct equirectangular distortion and reuses already-tested projection primitives (evidence and candidate analysis in `docs/TARGET_RESOLUTION_TECHNOLOGY.md`).
- **The detector is a replaceable seam** (`TargetDetector`) that receives a perspective view and a query. A production-capable adapter (`ProcessTargetDetector`) talks to an external helper over a file/JSON protocol, so Reelcraft links no computer-vision library and model/runtime/license choices stay independent.
- **The tracker is deterministic and replaceable** (`SphericalTargetTracker`): spherical NMS within a frame, then greedy nearest-neighbour association on great-circle distance with a gate and miss counting. No appearance or re-identification model is claimed.
- **No fabricated targets.** When nothing matches a query, the resolver records an unresolved note and returns no observations.
- **Detected targets become `ReframeTarget`s** (`TargetResolver::resolvedTargets`) usable by the existing `ReframePlanBuilder`, and whole tracks become `ReframePlan`s (`TargetTrackPlanner`) consumed by `CameraPath`/`ReframeRenderer`.
- **Dependency/licensing stance:** no linked CV dependency. Ultralytics YOLO (AGPL-3.0; enterprise license for proprietary use) is rejected as a default; permissively licensed candidates (YOLOX, RT-DETR, OpenCV/OpenCV Zoo, MediaPipe — Apache-2.0; ONNX Runtime — MIT) are recommended and recorded.

## Consequences

- 39 new tests cover geometry, view coverage, seam and pitch boundaries, tracking, resolution, the subprocess protocol, the planner, and a model-free end-to-end detection -> direction -> ReframeTarget -> plan -> render path. Full suite: 219 passed / 0 failed / 0 skipped.
- Real model integration is not present in this environment; the seam and protocol are tested with a synthetic detector and a shell helper. No model weights are bundled.
- "Me" identity, speaker localization, semantic understanding, and production tracking quality remain future objectives.
- This decision does not change Decisions 017 or 018; it adds a target-resolution layer above the reframing engine.


# Decision 020 — Real Detector Integration: OpenCV Zoo YOLOX via the Existing Subprocess Seam

**Status:** Accepted (2026-09-17, 360 Reframing Objective 3)

## Context

Decision 019 established the replaceable `TargetDetector`/`ProcessTargetDetector` boundary but bundled no real model, so real-world detection was not yet verified. Reelcraft needed to prove it can actually detect a real person in real 360 footage without coupling the C++ application to Python, OpenCV, ONNX, YOLOX, or a GPU runtime.

## Decisions

- **First real detector:** OpenCV Zoo `object_detection_yolox` (2022nov), `object_detection_yolox_2022nov.onnx` (~35.8 MB), COCO 2017 (80 classes, `person` = class 0). Code and weights are **Apache-2.0** (verified from the model directory `LICENSE`); commercial use permitted. Ultralytics YOLO (AGPL-3.0) remains rejected as a default (Decision 019).
- **Runtime:** Python 3 + OpenCV DNN 4.10 (`python3-opencv`), CPU target, installed only as an optional external helper — **not linked into the Reelcraft build**.
- **Boundary preserved:** the helper implements the existing `ProcessTargetDetector` file/JSON protocol. Reelcraft stays runtime-independent; swapping to RT-DETR, ONNX Runtime, a GPU service, or another model requires no C++ change.
- **Equirect handling unchanged:** detection runs on perspective/tangent views generated by `EquirectView`; detections are reprojected by `EquirectProjection`. No second coordinate system.
- **Testing posture:** the normal unit suite stays model-free. The real detector is exercised by a separate integration test (`realDetectorIntegration`) that is skipped unless the helper/model/clip are configured. `tools/detector_helper/` ships the optional helper plus its protocol and licensing documentation.

## Consequences

- Verified on a real 3840x1920 equirectangular clip (a lightweight proxy was used for speed; the original media was untouched): real YOLOX detected people in tangent views; a target track held a stable identity across five sampled frames (mean confidence ~0.92); the track produced a `ReframePlan`; and `ReframeRenderer` produced a 640x360 H.264 clip centered on the detected person.
- The helper is optional and environment-specific; model weights (~36 MB) are downloaded, not committed, and are not required by the build or the normal test suite.
- GPU inference (CUDA) and heavier models remain possible on RunPod; this objective used lightweight CPU inference because no RunPod credentials were available in the environment.
- "Me" identity, speaker localization, and production tracking quality remain future objectives.


# Decision 021 — Target Identity and Deterministic Selection

**Status:** Accepted (2026-09-17, 360 Reframing Objective 4)

## Context

Objective 3 proved real detection and tracking, but there was no structured creator identity: "me" and multi-person references were resolved only by matching a track id or label, and selection could depend on detector/track ordering. The objective was to distinguish a creator-selected target from other detected people deterministically, without adding appearance/biometric re-identification, speaker association, or GPU work.

## Decisions

- **Identity is a structured binding** between a creator-meaningful identity key (canonical "me") and a tracker track id: `CreatorTargetSelection` and `IdentityBinding` in `app/target/TargetIdentity.h`. It is JSON-serializable and inspectable, and is explicitly **not** biometric identity.
- **Creator selection is structured data**, not renderer/UI state: a direction (yaw/pitch) at a time, or an explicit track id, plus optional label/evidence. `TargetIdentityRegistry::bindFromSelection` binds deterministically to the nearest in-window track (ties: confidence desc, first observation time, id) and refuses when nothing is within the gate.
- **Identity persistence is geometric, not appearance-based:** the registry refreshes against live tracks and re-binds a lost identity only when exactly one active, unclaimed, label-compatible track continues its predicted trajectory; otherwise it reports the identity unresolved or ambiguous. The tracker adds bounded constant-velocity prediction (crossing trajectories) and a bounded re-entry gate (temporary loss/occlusion/re-entry).
- **Deterministic selection** (`app/target/TargetSelector.{h,cpp}`) resolves a documented vocabulary ("me"/selected aliases, "the other person", "person N"/ordinals, left/right, exact track id, unique label) using the canonical order (first observation time, numeric track id, id string) — never detector output order. Genuinely ambiguous references return an ambiguous result with candidates rather than a guess.
- **Future seam:** appearance/embedding re-identification and audio-visual speaker association can extend the same binding structure later, without changing the detector, geometry, planner, or renderer.

## Consequences

- 21 new model-free tests cover crossing, re-entry, prediction determinism, seed binding, claim conflicts, active-state resolution, unique vs ambiguous continuity re-binding, JSON round-trip, creator aliases, other-person ambiguity, ordinal determinism regardless of input order, left/right, track-id/label, and identity -> `ReframePlanBuilder` -> `CameraPath`. Full model-free suite: 240 passed / 0 failed / 1 skipped.
- Real-footage integration selects a presenter by a structured seed, binds "me", keeps the identity across the sampled frames, and renders a flat output centered on the selected presenter.
- "Me" currently means "the track the creator selected (or its unique geometric continuation)". It is not biometric identity and cannot re-identify a person after a long absence or among similar people without a future appearance model.
- No detector, model/runtime, renderer, or geometry changes; Decisions 017-020 preserved.


# Decision 022 — Appearance-Based Re-Identification: Optional External Embedding Seam

**Status:** Accepted (2026-09-17, 360 Reframing Objective 5)

## Context

Decision 021 made identity geometric and creator-selected, but it cannot re-identify a person after a long absence or among similar people. The objective was to add an optional, replaceable visual-appearance layer that strengthens (never overrides) the creator identity, without coupling the C++ core to a model runtime and without claiming biometric identity.

## Decisions

- **Optional appearance seam:** `AppearanceProvider` (interface) + `ProcessAppearanceProvider` (external helper over a file/JSON protocol). The C++ core links no Python, OpenCV, ONNX Runtime, or model.
- **Structured evidence, not a magic score:** `AppearanceEmbedding` (L2-normalized vector), `AppearanceProfile`, and `AppearanceEvidence` with an explicit `AppearanceVerdict` (Unavailable / Agree / Disagree / Weak / Ambiguous), cosine similarity, explicit accept (0.75) and reject (0.55) thresholds, and deterministic temporal aggregation (element-wise mean, then normalize).
- **Deterministic orchestration:** `IdentityReidentifier` maintains a profile for a bound identity and applies a documented precedence:
  1. explicit creator selection / active bound track is authoritative — appearance never overrides it;
  2. valid tracker continuity needs no re-identification;
  3. a unique geometric continuation is accepted and verified; strong appearance disagreement vetoes it and the identity becomes unresolved (recorded, no silent swap);
  4. appearance may re-acquire exactly one strong candidate when geometry provides none; multiple strong candidates are ambiguous;
  5. otherwise the identity remains unresolved.
- **Registry stays media/model free:** `TargetIdentityRegistry` only stores profiles and applies structured decisions (`setAppearanceProfile`, `rebindWithAppearance`, `annotateAppearance`, `markUnresolved`, `rejectTarget`). Vetoed targets are recorded so geometry cannot silently re-accept them.
- **Crop extraction:** `TargetCropExtractor` reuses `EquirectView` to render a deterministic crop from the target direction and angular extent.
- **Model selected for real validation:** OpenVINO Open Model Zoo `person-reidentification-retail-0277` (ONNX `person-reidentification-retail-0265.onnx`, 256-d, input 256x128 BGR 0-255); code and weights are **Apache-2.0** and it is trained on an internal dataset. Runtime: ONNX Runtime (MIT) in an external helper. Rejected: OpenCV Zoo YoutuReID (weights from an unlicensed source trained on Market1501/DukeMTMC/MSMT17), OSNet/torchreid weights (research-dataset provenance), and Ultralytics (AGPL-3.0).

## Consequences

- 24 model-free tests cover serialization, normalization/similarity, thresholds, strong/weak/ambiguous/wrong-person behavior, geometry/appearance conflict, explicit-selection precedence, missing/failed provider, deterministic ordering, and geometry confirmation. Full model-free suite: 264 passed / 0 failed / 1 skipped.
- Real-footage integration verifies real embeddings (same-person cosine ~0.97, different-person ~0.51) and a controlled re-acquisition through a simulated tracker gap, feeding the existing selection/plan/render path.
- Appearance is not biometric certainty; "me" remains the creator-selected identity, and appearance only supports bounded re-acquisition or vetoes an unjustified geometric transfer.
- No detector, geometry, planner, or renderer changes; Decisions 017-021 preserved.


# Decision 023 — Optional Audio/Speaker Evidence: Replaceable Provider, Evidence-Only Identity Integration

**Status:** Accepted (2026-09-17, 360 Reframing Objective 6)

## Context

Identity is geometric with optional appearance re-identification (Decisions 021/022). The product capability "follow whoever is speaking" needs audio evidence about which visible tracked person is speaking. Audio must not become the authoritative identity source and must not silently override the creator's explicit selection.

## Decisions

- **Optional audio seam:** `SpeakerEvidenceProvider` + `ProcessSpeakerProvider` (external file/JSON helper). The C++ core links no audio or ML runtime.
- **Structured evidence:** `SpeakerInterval`, `SpeakerAnalysis`, `SpeakerEvidence`, `SpeakerSegment`, and an explicit `SpeakerVerdict` (Unavailable / Active / Ambiguous / Overlap / Silence / Unassociated). JSON-serializable; provider-local `speakerId`; optional direction-of-arrival.
- **Deterministic association:** `SpeakerTargetAssociator` maps a speakerId to an existing target track in this order: explicit creator/structured binding > optional spatial (direction of arrival) nearest within a gate > single-visible-person > unassociated/ambiguous. It never creates a target and reports ambiguity rather than guessing.
- **Temporal hysteresis:** `SpeakerTimeline` merges same-speaker intervals across short pauses (`holdMs`), requires a different speaker to persist `switchConfirmMs` before taking over, ignores speech shorter than `minSpeechMs`, and represents simultaneous speech as an Overlap segment. This prevents camera thrashing.
- **Evidence-only identity integration:** audio never rebinds identity. `TargetIdentityRegistry::annotateSpeaker` records `speakerId`, confidence, and verdict on the binding without changing resolution. Audio drives deterministic **selection** for "follow the speaker" through `SpeakerReframePlanner`, not identity mutation.
- **Provider selected for real validation:** Silero VAD (`silero_vad.onnx`), **MIT** code and weights, local CPU via ONNX Runtime (MIT) with FFmpeg audio decode. VAD provides speech activity, not speaker identity; association uses an explicit creator binding (or the single-visible-person/DoA rules).
- **Rejected/deferred:** pyannote diarization (gated models plus a PyTorch stack), SpeechBrain/torchreid speaker embeddings (heavy runtime, VoxCeleb weight provenance), cloud speaker APIs (no cloud dependence), and audio-visual active-speaker models (research-grade or unclear weight licensing).

## Rationale for audio's position in the precedence hierarchy

Audio is placed below appearance and is deliberately **not allowed to trigger an identity rebind**. The objective explicitly warns against identity swaps driven by a model's numerical confidence; audio provides evidence and a selection signal, while geometry/appearance continue to own re-identification. This is a conservative choice and is flagged for checkpoint review.

## Consequences

- 31 new model-free tests; full model-free suite: 295 passed / 0 failed / 1 skipped.
- Real footage (audio+video proxy; original untouched): Silero VAD detects speech in the real 360 clip; with a creator speaker→target binding, the evidence associates to the visible presenter, and `SpeakerReframePlanner` produces a `ReframePlan` consumed by the existing deterministic renderer.
- "Follow whoever is speaking" now works through the structured selection layer. Automatic audio-visual speaker attribution without an explicit binding (diarization / active-speaker models) remains future work.
- No changes to the detector, geometry, identity resolution, or renderer; Decisions 017-022 preserved.


# Decision 024 — Audio-Visual Provider Attribution Seam and the Feasibility Boundary for Automatic Speaker Attribution

**Status:** Accepted (2026-09-17, 360 Reframing Objective 7)

## Context

Decision 023 gave Reelcraft optional audio/speaker evidence, but with a VAD-only provider the audio layer can say *when* speech occurs, not *who* is speaking. Objective 7 asked for automatic audio-visual speaker attribution — diarization or an audio-visual active-speaker model attributing speech to visible tracks without a creator binding — behind the existing replaceable seam, under an explicit efficiency guardrail: deliver the *minimum reliable capability* for "follow the person who is speaking", not maximum perception sophistication.

Feasibility was investigated on the project's real 360 footage before building anything:

- **The audio is mono** in both the original `360_TEST_4K.mp4` and the derived A/V proxy, so direction-of-arrival / spatial (beamforming) attribution is impossible.
- A lightweight **face-detection + mouth-region-motion vs audio-envelope correlation** probe was run at 12 fps (48 frames, 4 s, lags −3..+3) on the two visible presenters (OpenCV YuNet face detector, MIT). The presenter identified as speaking reached max correlation 0.250; the other presenter reached 0.192 — a weak separation well inside noise, and the likely listener had *higher* motion energy. Naive motion/audio correlation is not a reliable discriminator on this footage.
- **Licensing:** no permissively licensed, clearly commercial audio-visual active-speaker model was identified. TalkNet, LoCoNet, and AV-HuBERT are research-grade and/or their weights' licensing is unclear; pyannote diarization models are gated behind access conditions and add a PyTorch stack; SpeechBrain/torchreid speaker embeddings are heavy with VoxCeleb weight provenance. This confirms the licensing finding of Decision 023.

## Decisions

- **Complete the seam, not a speculative perception subsystem.** A provider may now attach an optional **attribution hint** (`targetIdHint`) to a `SpeakerInterval`: a specific existing target track id that the provider believes is speaking. This is the minimal protocol extension that lets a future diarization or audio-visual provider attribute directly through the existing `SpeakerEvidenceProvider` boundary; no new C++ component, runtime, or dependency is added.
- **Deterministic consumption with a safe precedence.** `SpeakerTargetAssociator` honours the hint only when the hinted target is visible, and only after an explicit creator binding: explicit creator/structured binding > visible provider hint > spatial direction-of-arrival > single-visible-person > unassociated/ambiguous. A hint for a non-visible target is ignored and falls through; a hint never invents a target and never overrides the creator. The association method is recorded as `provider-hint`.
- **The hint is carried through the timeline.** `SpeakerTimeline` propagates the hint from the originating intervals onto the coalesced `SpeakerSegment` (first non-empty hint wins; cleared for overlap and silence segments), so `SpeakerEvidenceAnalyzer` can honour it when it associates the stable segment. The existing hysteresis, verdicts, and evidence-only identity rule are unchanged.
- **No model-backed provider is shipped, and this is deliberate.** Reelcraft does not ship a motion-correlation or diarization provider because none available in this environment is both reliably accurate and permissively licensed. Shipping one would violate the objective's "minimum reliable capability" guardrail; the objective's own contingency (complete the seam and tests, document exactly what prevented real attribution) is followed instead.
- **Objective 7 outcome.** The reliable "follow the speaker" capability remains the Objective 6 path (an explicit one-time creator speaker→target binding, or the single-visible-person rule, on real 360 footage). Automatic attribution among multiple visible people is available to any future provider that supplies a `targetIdHint`; without one it is honestly reported as ambiguous/unassociated rather than guessed.

## Consequences

- 5 new model-free tests: hint JSON round-trip and backward compatibility; a visible hint attributing despite several visible people; a non-visible hint falling through to the spatial rule; an explicit binding defeating a hint; and the analyzer/timeline path carrying the hint end to end. Full model-free suite: 300 passed / 0 failed / 1 skipped.
- No detector, geometry, identity-resolution, or renderer changes; audio remains evidence-only and below appearance in the precedence hierarchy; Decisions 017–023 preserved.
- Per the human-approved priority, the next work is end-to-end 360 user-command testing rather than further perception subsystems. A real audio-visual/diarization provider and GPU optimization are recorded as future work behind the same seam.


# Decision 025 — End-to-End 360 User-Command Execution Boundary

**Status:** Accepted (2026-09-17, 360 Reframing Objective 8)

## Context

Objectives 1–7 built the 360 pipeline as separate layers: a structured `ReframePlan` + deterministic renderer (Decision 018), a replaceable target-resolution layer (019), a real detector helper (020), structured target identity/selection (021), optional appearance re-identification (022), optional audio/speaker evidence (023), and an audio-visual provider-attribution seam (024). `ReframePipeline` could parse an instruction and render, but it accepted `resolvedTargets` as a caller input: there was no single entry point that turned a user command containing a subject reference into resolved directions. End-to-end command testing therefore required composing the decision layers with the deterministic engine.

## Decisions

- **One composition entry point:** `app/reframe/ReframeCommandRunner.{h,cpp}` turns a user instruction plus a 360 source into a validated plan and, optionally, a rendered flat video. It is the top of the 360 command path:
  `instruction -> parsed intent -> subject references -> target resolution (replaceable detector + tracker) -> identity/selection -> resolved directions -> validated ReframePlan -> deterministic render`.
- **Two explicit stages:** `prepare()` is the decision stage (parse + resolve + plan); it is deterministic and model-free when an in-memory detector/provider is injected. `run()` adds the deterministic execution stage through the existing `ReframePipeline`. The AI/decision side and the deterministic executor stay separate; `ReframePipeline` itself is unchanged.
- **The runner adds no perception of its own.** It reuses `TargetResolver` (detector + tracker), `TargetIdentityRegistry` + `TargetSelector` (identity/selection), `ReframePlanBuilder` (validated plan), and `ReframePipeline`/`ReframeRenderer` (execution). Detectors remain replaceable and optional.
- **No fabricated direction:** an unresolved or ambiguous subject reference produces an error naming the reference and no plan; a direction-only instruction needs no detector; a creator seed that fails to bind is recorded in notes and does not invent a target.
- **Identity precedence is unchanged:** explicit creator selection > tracker continuity > geometry > appearance > audio > unresolved. Because the runner uses `TargetSelector`, it inherits the documented reference vocabulary ("me", "the other person", "person N", left/right, exact track id, unique label) and reports ambiguity rather than guessing.
- **Source media is read-only:** the runner and the pipeline only read the source.

## Consequences

- 8 new model-free tests cover subject resolution + plan, direction-only commands without a detector, unresolved/ambiguous honesty, creator-identity resolution ("follow me"), missing-detector and invalid-range errors, and determinism. Full model-free suite: 308 passed / 0 failed / 2 skipped.
- A new env-gated `realUserCommandIntegration` test exercises the full command path on real 360 footage with the Apache-2.0 YOLOX detector (command -> resolve -> plan -> render); the normal suite stays model-free. Real-run evidence is recorded in `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md`.
- This is the composition layer future work builds on (speaker-aware commands, Application/UI wiring); it does not change the detector, geometry, identity, appearance, speaker, planner, or renderer internals. Decisions 017–024 preserved.

# Decision 026 — Application-Level 360 Command Orchestration Boundary

**Status:** Accepted (2026-09-17, 360 Reframing Objective 9)

## Context

Objective 8 delivered `ReframeCommandRunner`, a library-level composition entry point that turns an instruction plus a 360 source into a validated `ReframePlan` and a rendered flat video. It was not reachable from the product: `Application` owned project/media/viewport and `MainWindow` exposed media/preview/viewport controls, but nothing accepted a 360 editing command or surfaced its result. Objective 9 wires the command path into the application without duplicating the runner's logic and without expanding into general UI work.

## Decisions

- **The application owns orchestration, not interpretation.** `Application::runReframeCommand()` (and `runReframeCommandTo()` for an explicit output path) resolves the active media, validates application state and the output location, builds a `ReframeCommandRequest`, delegates to the command executor, and maps the `ReframeCommandResult` into an application-visible `ReframeCommandOutcome`. Parsing, subject resolution, identity/selection, planning, and execution stay in `ReframeCommandRunner`; no logic is duplicated.
- **Structured, application-visible result.** `ReframeCommandOutcome` (`app/application/ReframeCommandOutcome.h`) records success/failure, the source media reference, the instruction, the effective time range, the output specification and path, the frame count, notes, unresolved references, and resolved target directions. It is JSON-serializable and emitted through `reframeCommandFinished()` for success and failure alike. Runner errors propagate verbatim; nothing is silently substituted or fabricated.
- **Injectable, optional inputs.** The target detector and command frame provider are non-owned, replaceable inputs (`setTargetDetector`, `setCommandFrameProvider`); the application does not own their lifetime. `main.cpp` builds a `ProcessTargetDetector` from `REELCRAFT_TARGET_DETECTOR_PY`/`_SCRIPT`/`REELCRAFT_TARGET_YOLOX_MODEL` when configured, so subject-referencing commands work in the product without linking an ML runtime; otherwise they report a clear detector error.
- **Test/DI execution seam.** The command executor is a `std::function` defaulting to `ReframeCommandRunner::run`; tests inject a fake or a prepare-only executor, keeping application tests model-free and deterministic. This is the only new application boundary.
- **Minimal UI, no redesign.** `MainWindow` gained one command input, a start/end seconds pair, a run button, and a result label, emitting `reframeCommandRequested(instruction, startMs, endMs)`; `main.cpp` connects it to `Application::runReframeCommand` and connects `reframeCommandFinished` to `MainWindow::showReframeCommandResult`.
- **Outputs are session state; the source is never modified.** Each outcome records the output path; generated renders are not persisted in the project schema (the project has no output section and duration/metadata probing remains deferred). The application refuses to run when the output path equals the source path and never writes to the source.
- **Range-less commands need a caller default.** Because no duration/ffprobe metadata exists, the UI supplies a fallback start/end range; a command that contains its own time range overrides it (the runner decides). A range-less command with an invalid fallback range fails honestly.

## Consequences

- 17 new model-free application tests cover project/active-media/empty-command validation, missing source, invalid output directory, source-equals-output, request delegation and outcome mapping, model-free subject resolution, unresolved/ambiguous honesty, missing detector, invalid range, render-failure propagation, determinism, source non-modification, the UI request signal, and result presentation; a resolver-robustness test proves an undecodable sample no longer aborts sequence resolution. Full model-free suite: 326 passed / 0 failed / 3 skipped.
- A new env-gated `realApplicationCommandIntegration` exercises the application command path on real 360 footage with the Apache-2.0 YOLOX detector; the normal suite stays model-free. Real-run evidence is recorded in `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md`.
- `ReframeCommandRunner` and everything below it are unchanged; Decisions 017–025 preserved. Persisting rendered outputs and full-clip (duration-aware) ranges remain future work.

# Decision 027 — Duration Probing and Persisted Render Records

**Status:** Accepted (2026-09-17, 360 Reframing Objective 10)

## Context

Decision 017 deferred ffprobe duration metadata to "its own decision/objective", and Objective 9 recorded two gaps (see `KNOWN_ISSUES.md`): generated renders were session state (not persisted in the project), and range-less commands used a caller-supplied fallback range because the application could not know the clip duration. Objective 10 explicitly scopes both: "Persist generated renders (or add a project output section) and add duration-aware full-clip ranges via the deferred ffprobe duration metadata." This decision satisfies that deferred gate for the 360 command path rather than silently reopening it.

## Decisions

- **Duration probing is a replaceable, external, fail-safe media-engine seam.** `app/media/MediaDurationProbe.h` defines `durationMs(filePath, ...)`; `FfprobeDurationProbe` implements it with the external `ffprobe` executable (never linked), resolved via `REELCRAFT_FFPROBE`, then a sibling `ffprobe` next to the resolved ffmpeg executable, then `PATH`. It reads the file only, validates existence/regularity, and fails deterministically (unavailable executable, missing file, non-zero exit, timeout, unparsable or non-positive duration). The application owns no codec dependency.
- **Whole-clip sentinel.** A zero start/end range means "the whole clip". `Application::runReframeCommandTo()` resolves it through the probe to `[0, durationMs]`. If the probe is unavailable the range stays invalid and the existing `ReframeCommandRunner` honestly reports an invalid range (or uses an explicit range parsed from the command). `MainWindow`'s range controls default to 0/0 with a "(0=whole)" hint. Probing happens only for the whole-clip sentinel, never for an explicit range.
- **Render records are the existing structured outcome, persisted additively.** Every command that reaches an output target appends its `ReframeCommandOutcome` (success or failure, with the error) to the application's authoritative record list. `Project` gains an additive `reframeOutputs` JSON array and `CurrentSchemaVersion` becomes 3; schema-2 projects load with an empty list and future schemas are still rejected. `Application` emits `reframeOutputsChanged` and `MainWindow` re-lists the records.
- **No parallel architecture.** The probe is a leaf seam in the media layer mirroring `FrameExtractor`; the record is the existing `ReframeCommandOutcome`; persistence reuses the project's additive-section pattern. `ReframeCommandRunner`, the deterministic pipeline/renderer, the perception/identity/appearance/speaker layers, no-fabrication behavior, and original-media protection are unchanged.

## Consequences

- 13 new model-free tests cover duration-output parsing, whole-clip range defaulting, explicit-range probe skipping, probe-failure honesty, record creation (success and failure), save/open persistence, new-project clearing, the project-section round-trip, outcome JSON round-trip, the UI record list, and the whole-clip UI default; one ffmpeg/ffprobe-gated test probes a generated clip. Full model-free suite: 339 passed / 0 failed / 3 skipped.
- Real-footage validation at completion: `realApplicationCommandIntegration` now uses the whole-clip range (probed from the ~12 s proxy) and verifies the render record survives save/open; the result is recorded in `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md`.
- Decisions 017–026 preserved. The deferred Phase 3 ffprobe gate is satisfied for the 360 command path; general playback duration metadata remains future work.

# Decision 028 — Speaker-Aware Command Path: Audio as Evidence, Explicit Binding First

**Status:** Accepted (2026-09-17, 360 Reframing Objective 11)

## Context

Objectives 6/7 delivered an optional audio/speaker evidence layer (`SpeakerEvidenceAnalyzer`, `SpeakerTimeline`, `SpeakerTargetAssociator`, `SpeakerReframePlanner`), and Objectives 8/9 wired a general 360 command path (`ReframeCommandRunner`, `Application`). That command path resolved subject references through a detector, but a "speaker" reference was unrecognized, so audio could not drive a command. Objective 11 scopes exactly that: "Reuse the Objective 6/7 speaker evidence layer inside the application command path so commands such as 'follow the speaker' select the active speaking target, keeping audio as evidence and never overriding an explicit creator selection."

## Decisions

- **The speaker layer is reused, not duplicated.** `ReframeCommandRunner` recognizes speaker references (`speaker`, `active speaker`, `person speaking`, `whoever is speaking`, plus the new `keep <subject> centered` / `center <subject>` phrasings) and routes them through the existing `SpeakerEvidenceAnalyzer` (provider -> timeline -> association) and `SpeakerReframePlanner` (associated timeline -> `ReframePlan` with cuts at speaker changes). No new perception and no parallel command system.
- **Audio is evidence, never authority.** The speaker path requires an optional, replaceable `SpeakerEvidenceProvider`; without one it reports honestly. Explicit creator `speakerId -> targetId` bindings are applied to the associator and are honoured first (explicit > visible provider hint > spatial > single visible > ambiguous). An explicit creator identity is never rebound by audio, and no speaker direction is fabricated.
- **No silent reinterpretation.** A command that mixes a speaker reference with another subject reference or with an explicit camera direction is reported as unsupported rather than partially executed. An unassociated or ambiguous speaker produces an error and no plan.
- **Plans are rendered directly.** `ReframePipeline::renderPlan()` renders an already-validated plan with the same deterministic renderer/encoder, and `ReframePipeline::run()` now delegates to it. `ReframeCommandRunner::run()` renders the prepared plan, so a `SpeakerReframePlanner` plan is not re-derived by `ReframePlanBuilder`.
- **Application plumbing.** `Application` holds a non-owned `SpeakerEvidenceProvider` and optional `speakerBindings`, copies them into each `ReframeCommandRequest`, and `main.cpp` builds a `ProcessSpeakerProvider` from `REELCRAFT_SPEAKER_PY`/`_SCRIPT`/`REELCRAFT_SILERO_MODEL` when configured. `ReframeCommandRequest` also accepts optional pre-resolved tracks, so a caller that already resolved targets does not resolve twice.
- **Preserved invariants.** Original media remains read-only; detector/identity/appearance/speaker boundaries remain replaceable; the deterministic renderer executes, never perceives.

## Consequences

- 9 new model-free tests: speaker phrasing parsing, speaker-follow with a scripted provider, honest failure without a provider, honest ambiguity/unassociated behavior, explicit-binding precedence, unsupported mixing (subject and direction), `renderPlan` input validation, and application plumbing. Full model-free suite: 348 passed / 0 failed / 4 skipped.
- A new env-gated `realSpeakerCommandIntegration` resolves real presenter tracks once with the Apache-2.0 YOLOX detector, then runs "follow the speaker" with the real Silero VAD provider and an explicit creator speaker binding through the deterministic renderer. Result recorded in `CURRENT_STATE.md` and `DEVELOPMENT_LOG.md`.
- Decisions 017–027 preserved. GPU optimization and the deferred Phase 3 player-lifecycle objective remain future work.

# Decision 029 — Objective 12 Scope: 360 Command UI and Rendered-Result Preview

**Status:** Accepted (2026-09-17, 360 Reframing Objective 12)

## Context

Objectives 1–11 completed the 360 command pipeline (source -> scene/target understanding -> English command -> resolve -> plan -> deterministic render -> persisted record). Objective 11's `NEXT_TASK.md` next-objective entry named only candidates — GPU optimization of the ML helpers, the deferred Phase 3 Application-level player-lifecycle objective, and richer UI for the persisted render records — and stated that the next objective "requires its own scoped objective; do not begin automatically." No scoped Objective 12 existed in the repository. The human selected the candidate that most directly serves the 360-first priority: "360 command UI + preview rendered results."

## Decisions

- **Objective 12 is "360 Command UI and Rendered-Result Preview."** It makes the existing command path usable from the application and lets the creator see a generated reframe, without building a general player/timeline or a parallel command system.
- **Creator "me" selection from the application.** `Application::selectCreatorTargetFromViewport()` seeds the canonical "me" identity from the current viewport yaw/pitch and the current preview time; `clearCreatorSelection()` clears it. The selection is session state (not persisted), is cleared on new/open project, and is passed into every `ReframeCommandRequest` (`hasCreatorSelection`/`creatorSelection`), so "follow me" / "keep me centered" can resolve without a known track id.
- **Rendered-result preview.** `Application::previewReframeOutput(index)` decodes the first frame of a persisted render record and emits `reframeOutputPreviewReady`. It fails honestly for an invalid index, a missing output file, or a decode failure. The decoder is an injectable seam (`ReframePreviewDecoder`) defaulting to the existing external-FFmpeg `FrameExtractor`; tests inject a fake so orchestration stays model-free.
- **Flat presentation.** A reframed render is a flat video, so `MainWindow::showReframeOutputPreview` presents the decoded frame through the viewer's existing flat mode (`setFlatSourceMode(true)` + `setSourceImage`), never the equirectangular camera transform. No viewer architecture change.
- **Minimal UI, no redesign.** `MainWindow` gained "Select Center as Me" / "Clear Me" buttons and a selection readout, a "Preview Selected Render" action on the existing render list, and a provider status line. `main.cpp` wires them; external detector/speaker providers remain configured through `REELCRAFT_*` and are reported as configured/not configured.
- **Preserved invariants.** Original media and generated outputs are read-only; the deterministic renderer and the replaceable ML/provider boundaries are unchanged; the deferred Phase 3 player-lifecycle objective is not started (only the single-frame preview, which reuses the existing frame seam, is in scope). Continuous playback and general timeline/UI work remain future objectives.

## Consequences

- 11 new model-free tests cover creator selection (set, require context, pass-through, clear), render preview (decode, reject bad inputs), and the UI (buttons, selection readout, preview request, flat presentation, provider status). Full model-free suite: 359 passed / 0 failed / 4 skipped.
- No expensive real-media validation is required: the preview decode reuses the already-verified external-FFmpeg `FrameExtractor` seam, and the existing env-gated real integrations were not re-run.
- Decisions 017–028 preserved. GPU optimization and the Phase 3 player lifecycle remain future work.


# Decision 030 — Objective 13 Scope: 360 Rendered-Result Playback

**Status:** Accepted (2026-09-17, 360 Reframing Objective 13; human-selected scope)

## Context

Objective 12's next-objective entry named three un-scoped candidates (GPU optimization, the deferred Phase 3 Application-level player lifecycle, and persisting the creator selection). The human selected the Phase 3 player lifecycle, scoped specifically to serving the 360 workflow: continuous deterministic playback of persisted 360 -> flat rendered results. Decision 017 already fixed the boundary — 'decode/media-source seam -> player/timing -> Application -> Viewer' — and deferred the Application-level player lifecycle (Phase 3 Objective 5). Objective 13 resumes exactly that piece, and only that piece, for rendered-result playback. The existing media/player seams (`FrameSource`, `FfmpegFrameSource`, `FramePump`, `Player`, `Playhead`, `Clock`, `PacingPolicy`) already exist and are tested; only Application-level orchestration was missing.

## Scope (human-selected)

In scope:
- Application owns/creates/replaces/disposes the playback source (`FrameSource`), frame pump (`FramePump`), and player (`Player`) for the selected persisted rendered result.
- Reuses the existing `FrameSource`/`FfmpegFrameSource`/`FramePump`/`Player`/`Playhead`/`Clock`/`PacingPolicy` seams; no parallel playback architecture.
- An Application-level event-loop driver (an owned `QTimer`) invokes `Player::tick()`; the Player never owns the event loop.
- Deterministic play/pause/stop of an already-rendered result; the frame interval is derived from the record's output fps.
- The Objective 10 preview-time contract and the existing single-frame render preview are preserved.
- The Viewer stays presentation-only: the Application emits frames and the UI presents them flat.

Out of scope (explicit): QtMultimedia; audio; general-purpose media playback; timeline editing; duration/ffprobe work; scrubbing/timeline UI beyond the basic controls; GPU optimization; new ML/perception; changing the deterministic renderer; replacing the existing media/player abstractions; any subsequent objective.

## Definition of Done

1. `Application::startReframeOutputPlayback(index)` opens (or replaces) the selected rendered result through the existing seam, starts the `Player`, and begins ticking; `pauseReframeOutputPlayback()`, `resumeReframeOutputPlayback()`, `stopReframeOutputPlayback()`, and `tickReframeOutputPlayback()` exist and are deterministic.
2. The owned timer drives ticks; the `Player` owns no timer or thread; pausing/stopping halts advancement.
3. Playback frames are surfaced (`reframePlaybackFrameReady`), with `reframePlaybackStateChanged` and `reframePlaybackEnded`; the viewer presents them flat.
4. Lifecycle: selecting a new result replaces the previous playback; new/open project disposes it; the source is always closed (no leaked subprocess).
5. Honest failures: invalid index, missing output file, unknown dimensions, source-open failure, decode error, and end-of-stream are reported deterministically; the source media and rendered outputs are read-only.
6. The Objective 10 preview-time contract is unchanged, and the single-frame render preview still works.
7. Model-free tests (injected `FrameSource`, `ManualClock`, `PacingPolicy`) cover start/play/pause/resume/stop/replace/end/error/position and the UI controls; the full model-free suite is green.
8. One real-media completion validation renders a 360 clip to a flat result through the existing pipeline and plays it back through the Application (gated on clip + ffmpeg; no ML).
9. Documentation is updated (`CURRENT_STATE`, `NEXT_TASK`, `DEVELOPMENT_LOG`, `PROJECT_HISTORY`, `AI_HANDOFF`, `CHANGELOG`, `KNOWN_ISSUES`) and a dedicated checkpoint commit leaves a clean tree.

## Decisions

- Reuse the Decision 017 boundary unchanged. Playback is limited to persisted rendered results; general media playback remains behind the same seam for a future objective.
- The Application owns the playback objects and the event-loop driver; the `Player` stays passive and deterministic.
- Injectable seams keep tests model-free: a `PlaybackSourceFactory` (default: `FfmpegFrameSource` opened with the record's geometry) and injected `Clock`/`PacingPolicy` (defaults owned by the `Player`).
- The frame interval derives from `ReframeCommandOutcome::outputFps` (fallback 40 ms), and the driver timer interval is clamped for sanity.

## Consequences

- 9 new model-free tests cover start/play/pause/resume/stop/replace/end/error/position and the UI controls. Full model-free suite: 368 passed / 0 failed / 5 skipped.
- Real-media completion validation renders a real 360 clip to a flat result through the existing pipeline and plays it back; it surfaced and drove a fix for a **pre-existing media-seam bug**: `FfmpegFrameSource::readNextFrame` reported end-of-stream as soon as the ffmpeg process exited, discarding output still buffered in the pipe, so a slow/paced consumer lost the tail (about 10 of 20 frames). The read now drains all buffered output before deciding EOF, and the validation plays all 20 rendered frames. This is a correctness fix inside the existing media seam, not a new architecture.
- Decisions 017–029 preserved. General/active-media playback, audio, timeline editing, duration metadata, creator-selection persistence, and GPU optimization remain future work.


# Decision 031 — Objective 14 Scope: 360 Temporal Editing Operations

**Status:** Accepted (2026-09-17, 360 Reframing Objective 14; human-selected scope)

## Context

The 360 pipeline resolves targets, plans a camera, renders deterministically, persists records, and (Objective 13) plays rendered results. It has no notion of *temporal* editing: a plan renders one contiguous source range. Objective 14 adds a deterministic temporal-editing foundation — retain / remove / target-duration — that composes with the existing reframing/target/speaker layers and reuses the existing execution, persistence, and playback paths.

## Scope (human-selected)

In scope:
- A deterministic, JSON-serializable temporal representation, independent of natural-language parsing: `TemporalEditPlan` with Keep/Remove/TargetDuration operations, ordered multi-range support, validation, deterministic normalization, and rejection of invalid/ambiguous/contradictory input.
- Command integration through the existing `ReframeIntentParser`/`ReframeIntent` (no parallel parser): parse 'cut from X to Y', 'remove X to Y', 'keep X and Y', 'make a N-second version', and compose with existing target/reframe clauses.
- Composition with 360 reframing: a temporal selection is expressed as ordered retained source ranges on the existing `ReframePlan`; the camera path is still evaluated at absolute source time.
- Deterministic execution: the existing `ReframeRenderer`/`ReframePipeline` render each retained range in order, concatenating frames; the source media stays read-only and a new persisted result is produced.
- Persisted records and Objective 13 playback: the resulting flat output is a normal render record and plays back through the existing playback path.

Out of scope (explicit): full timeline editor; drag-and-drop/scrubbing UI; captions; transitions; effects; color grading; audio editing/mixing; general media playback; QtMultimedia; GPU optimization; new ML/perception/detection/re-identification/speaker systems; changing the 360 projection/reframing architecture; replacing the playback architecture; creator-'me' persistence; any later objective.

## Definition of Done

1. `TemporalEditPlan` (app/reframe) is a validated, JSON-serializable representation independent of the parser: Keep/Remove/TargetDuration, ordered ranges, `isValid`, deterministic `normalize`, `resolve(durationMs, defaultStartMs)`, and rejection of reversed/zero-length/negative/out-of-bounds ranges and empty or contradictory results.
2. `ReframeIntentParser` produces `ReframeIntent::temporalEdit` from the documented temporal phrases, reusing the existing intent architecture; invalid time ranges, out-of-bounds ranges, contradictory operations, unsupported operations, and ambiguous requests are explicit and honest.
3. `ReframePlan` gains an additive, ordered `segments` list (empty = the existing single source range) that changes only frame timing; `frameCount()`/`frameTimeMs()` and `isValid()` honor it, and the `ReframeRenderer` needs no new architecture.
4. `ReframeCommandRunner` composes a resolved temporal selection with the existing target/identity/speaker resolution and builds a plan whose segments are the retained ranges.
5. `Application` resolves the temporal edit against the probed source duration (whole-clip probing already exists) and reports out-of-bounds/contradictory cases honestly before execution.
6. `ReframeCommandOutcome` persists the retained segments so the record represents the resulting output.
7. The produced render plays through the existing Objective 13 playback with no new playback system.
8. Tests cover valid single/multiple ranges, keep, remove, overlapping, adjacent, reversed, zero-length, negative/out-of-bounds, deterministic normalization, contradictory operations, ambiguous commands, temporal+target, temporal+'me', temporal+speaker (where the architecture permits), rendering a temporally edited result, and playback of it.
9. Full model-free regression is green; one real-media completion validation renders a temporally edited 360 result and plays it back.
10. Documentation updated (DECISIONS, CURRENT_STATE, NEXT_TASK, CHANGELOG, DEVELOPMENT_LOG, PROJECT_HISTORY, AI_HANDOFF, KNOWN_ISSUES where applicable); dedicated checkpoint commit; clean tree; Decisions 017–030 preserved.

## Decisions

- The temporal representation is a first-class structured value (not parser state); the parser only produces it.
- Temporal selection is expressed as ordered retained ranges on the existing `ReframePlan`; the deterministic renderer concatenates per-range frames and the camera path is evaluated at absolute source time. No new rendering architecture.
- Remove and target-duration operations are resolved to retained ranges against the known source duration; without a duration they fail honestly.
- The command layer distinguishes valid / invalid-range / out-of-bounds / contradictory / unsupported / ambiguous outcomes and never invents timestamps.

---

# Decision 032 — Objective 15 Scope: 360 Compound Natural-Language Editing

**Status:** Accepted (2026-09-17, 360 Reframing Objective 15; human-selected scope)

## Context

The Objective 14 parser treats a clause containing a temporal operation plus time tokens as a temporal clause and skips it in the camera-subject parser, so a single compound command that joins a temporal edit and a camera/target instruction with "and" loses the camera/target half. The documented example "Keep 0:00 to 0:30 and follow me." produces only the temporal edit. Objective 15 fixes that composition without adding a second parser, a second temporal representation, a new renderer, or a new playback path.

## Scope (human-selected)

In scope — only unambiguous compositions:

- Class A — temporal + target: "Keep 0:00 to 0:30 and follow me." -> KEEP [0:00, 0:30] + target "me" (existing follow/center behavior applied to the entire retained range).
- Class B — temporal + explicit target: "From 0:35 to 1:10, keep the person I selected centered." -> KEEP [0:35, 1:10] + the existing creator-selected target vocabulary.
- Class C — temporal + speaker: "Keep 0:35 to 1:10 and follow whoever is speaking." -> KEEP [0:35, 1:10] + the existing active-speaker path.
- Class D — target duration + target: "Make a 30-second version and keep me centered." -> the existing TargetDuration operation + the existing "me" target applied to the resulting effective range.

The deterministic semantic rule: when a temporal edit and a camera/target instruction occur in the same unambiguous command and no separate applicability interval is specified, the camera/target instruction applies to the entire resulting temporal edit. Multiple camera moves continue to use the existing camera-path interpolation.

Explicitly out of scope: new detection/tracking/re-identification/speaker systems; GPU optimization; creator-"me" persistence; a timeline editor, drag/drop, or scrubbing; captions, transitions, effects, color grading, audio editing; general-purpose playback or QtMultimedia; renderer/playback replacement; a second temporal representation; a parallel natural-language parser; a generalized editing DSL; arbitrary sequential temporal applicability (for example "follow me for the first 20 seconds, then follow the other person"); and any later objective.

## Definition of Done

1. The compound intent is explicitly represented on the existing ReframeIntent (a temporal edit plus camera moves) and documented; no new intent type or workflow engine.
2. The existing ReframeIntentParser parses the supported compound phrases; temporal clauses are still not interpreted as target subjects.
3. Temporal + target/reframe commands preserve both operations and the camera/target resolves through the existing target/identity system.
4. Temporal + speaker commands preserve both operations through the existing speaker system.
5. Target-duration + target commands work through the existing TargetDuration operation.
6. Ambiguous, contradictory, invalid, and unsupported compositions fail honestly (including a camera instruction with its own separate time interval), and no timestamp or target identity is invented.
7. TemporalEditPlan, ReframePlan, the renderer, and Objective 13 playback are reused unchanged in architecture.
8. Focused Objective 15 tests pass; the full model-free regression is green; one real-media compound-command validation renders a compound result and plays it back.
9. Documentation updated (DECISIONS, CURRENT_STATE, NEXT_TASK, CHANGELOG, DEVELOPMENT_LOG, PROJECT_HISTORY, AI_HANDOFF, KNOWN_ISSUES where applicable); dedicated checkpoint commit; clean tree; Decisions 017-031 preserved.

## Decisions

- This is a composition fix in the existing intent parser, not a new natural-language architecture.
- A temporal clause's time ranges are stripped before camera/target extraction, so both halves of a compound "and" command survive; the operation keyword is retained so "keep <subject> centered" still parses.
- The default applicability rule is whole-retained-range; a camera instruction that supplies its own separate time interval is unsupported and fails honestly rather than being silently dropped.

---

# Decision 033 — Persisted, Versioned Edit Decisions (EditDecision)

**Status:** Accepted (2026-09-17, 360 Reframing Objective 16; human-selected scope)

## Context

The 360 command path produces a render from a natural-language instruction, and Objective 10 persists each attempt as a structured `ReframeCommandOutcome` record. That record is an *audit* record, not a *reproduction* record: it stores the instruction, the source reference, the effective ranges, the output specification and the frame count, but it does **not** store the resolved `ReframePlan` (camera keyframes, retained segments). The plan is computed inside `ReframeCommandRunner::prepare()`, used for the render, and then discarded by the application.

The consequence is that a persisted render can only be reproduced by re-running the original command, which requires re-parsing the natural-language instruction and, for any subject reference ("follow me", "follow the speaker"), re-running the whole perception stack — detector, tracker, identity/selection, appearance, speaker providers. A later render is therefore only as reproducible as those replaceable providers are stable, and it cannot be reproduced at all if a provider is unavailable.

Objective 16 closes that gap with a persisted, versioned `EditDecision` artifact that is sufficient on its own to reproduce a render.

## Decisions

- **The artifact is the resolved decision, not the instruction.** `app/reframe/EditDecision.{h,cpp}` stores the media the decision was made against (id, path, fingerprint), the already-resolved `ReframePlan` verbatim, the originating instruction (provenance only), and a creation timestamp. Because the plan already contains concrete camera keyframes, retained segments and the output specification, replay requires no natural-language parsing and no perception provider. The instruction is never re-parsed.

- **It embeds in the existing outcome record; no parallel pipeline, store, or file format.** The decision is written as an additive `editDecision` object inside each existing `reframeOutputs` record, so it reuses the project's existing additive-section persistence, the existing `Application` save/open path, and the existing `ReframeCommandOutcome` JSON round trip. Replay reuses the existing renderer through `ReframePipeline::renderPlan()` — the entry point that already renders an already-validated plan. Nothing new renders, stores, or migrates data.

- **Original media is referenced, never touched.** A decision stores only `mediaId` (the deterministic id `MediaItem` derives from the canonical path), the path, and a fingerprint. Replay opens the source read-only; no code path in this feature writes to source media.

- **`Project::CurrentSchemaVersion` stays 3; the artifact's own `schemaVersion` is the compatibility gate.** Adding a field inside the already-opaque `reframeOutputs` array does not change which top-level project sections exist, so a project schema bump would signal a change that has not occurred and would make older builds refuse a project they can read perfectly well (they simply ignore `editDecision`). `EditDecision::CurrentSchemaVersion` (1) is the real gate, checked by the artifact loader. Precedent: `activeMediaId` was added additively without a bump (Decision 027 notes `reframeOutputs` "becomes 3"; the `media` section had produced 2).

- **The loader refuses to mis-parse.** `EditDecision::readFromJsonObject()` fails with a descriptive error when `schemaVersion` is missing, non-numeric, `<= 0`, or greater than `CurrentSchemaVersion`; when the creation time or source reference is missing/malformed; when the plan is absent or fails `ReframePlan::isValid()`; or when a present `decisionHash` disagrees with the recomputed digest. A refusal never writes a partially-parsed artifact into the caller's output object. An unknown version is refused outright rather than parsed on the assumption that the layout is compatible.

- **The digest rule is fixed and precise.** `decisionHash` is the lowercase-hex SHA-256 over the **compact** JSON encoding of the decision payload *with the `decisionHash` key removed*; `createdUtc` **is** included. Key order is Qt's deterministic sorted `QJsonObject` order, so the digest is stable across processes, which is what makes it usable for diffs, integrity checks on load, and replay comparison.

- **An absent `decisionHash` is tolerated; a wrong one is fatal.** The digest is derived data, not authoritative: recomputing it is always possible and costs nothing, so its absence is not a compatibility signal and does not justify rejecting an otherwise well-formed decision, while a digest that is *present and disagreeing* is positive evidence of tampering or corruption and must fail.

- **Timestamp quantization is applied in three places, and all three must be preserved.** `MediaItem` holds the source modification time in memory at full filesystem resolution (typically microseconds) but serializes it as ISO-8601 **with milliseconds**; `QFileInfo` reports full resolution. Comparing a stored value against a live one therefore reports a spurious *"the source file has changed"* after every save/load cycle, and replay would refuse valid decisions. `EditDecision` therefore quantizes the modification time to **UTC milliseconds in all three places**: on record (`fromPlan`), on load (`readFromJsonObject`), and on comparison (`checkSource`). Any future change that touches `EditDecision` or `MediaItem` serialization **must preserve this triple application**; dropping or partially applying it silently breaks reproducibility rather than failing loudly.

- **Record-load failure policy: LENIENT-RECORD (decided explicitly).** When `ReframeCommandOutcome::readFromJsonObject()` encounters an `editDecision` it cannot load, the **record still loads**; the decision is simply not available. The record reports `hasEditDecision() == false` and a non-empty `editDecisionError()` explaining why, and the unreadable `editDecision` JSON is **preserved verbatim** so that re-saving the project cannot destroy data this build does not understand (a newer build may still interpret it).
  - *Reasoning.* The two objects have different truth status. The record is a **historical fact**: that render was produced, and its output file exists on disk. Failing the whole record would delete a completed render from the creator's list, and would make opening a project in an older build silently lose entries — the same silent-loss failure the strict loader exists to prevent. The decision, by contrast, is a **derived instruction**: if it cannot be trusted it must not be guessed, and replay must refuse honestly. Lenient-record + strict-decision satisfies both, and is consistent with the project's existing stance that records are preserved and degradation is surfaced (media records are never removed when a file becomes unavailable).
  - *Surfacing.* Because the record is kept, the failure must not be silent: `Application::restoreReframeOutputsFromJson()` counts records whose decision failed to load and reports them through the existing `backgroundCompleted` status channel on project open.

- **Known limitation: a decision is only persisted where a record is.** `Application::runReframeCommandTo()` only appends an outcome (and therefore only persists a decision) when the command reached a **non-empty output path**. Early application-state and validation failures — no project, no active media, unavailable media file, empty instruction, missing output directory, output path equal to the source — produce a user-visible failure outcome but **no persisted record and no decision**. Such a command cannot be replayed from the project, because nothing about it was stored.

- **Forward constraint for Objective 17+: persisted decisions are immutable.** A persisted `EditDecision` is a historical record of a decision that was actually executed and must never be mutated in place. A revision — a different camera, a different range, a different output specification — is expressed as a **new decision that references the prior one** (lineage), preserving both the original artifact and the fact that it was superseded. Any future editing, revision, or undo/redo feature must be built on new-decision-with-lineage rather than mutation, or the reproducibility guarantee this artifact exists to provide is lost.
- **Reproducibility claim:** same decision + same source -> same decoded frames,
  independent of ffmpeg build. Byte-identical container equality is asserted in
  this environment and may differ across ffmpeg builds; that is a delivery-layer
  concern (Obj22), not a decision-layer one.

- **Enforced output-path refusals, all before any render.** Replay validates the
  output path in full before the encoder can be invoked, and refuses three cases:
  an empty path; a path equal to the decision's source media (so replay can never
  overwrite the original media); and a path that already exists. Each refusal
  returns `ReplayResult{ ok = false }` with a descriptive error, appends no
  record, emits no `reframeOutputsChanged`, leaves any existing file
  byte-identical, and never reaches the renderer.

- **Forward extension, NOT implemented: an explicit `allowOverwrite` option.**
  Replay currently refuses any output path that already exists, with the error
  `"output path already exists: <path>; delete it or choose a fresh path"`, and
  the existing file is left untouched (not truncated, not partially written) with
  no record appended. There is no override. A future `allowOverwrite` flag --
  for callers that explicitly intend to replace an existing output -- is recorded
  here as a possible extension, so that the refusal reads as a deliberate default
  rather than an unfinished capability. It is **not** built in Objective 16.


- Verified results at completion (2026-09-17): the model-free artifact tests, the
  record-integration and back-compat tests, the decision-attachment tests, the
  same-process replay equivalence test, and the fresh-process replay test all pass.
  Full suite: **401 passed / 0 failed / 8 skipped**. The 8th skip is the child-only
  slot `replayFreshProcessChild`, which is driven by its parent
  (`replayFreshProcessReproducesRender`) and intentionally skips when run
  standalone; the other 7 are the env-gated real-media integration tests.
- Verification toolchain: the replay equivalence results above were obtained with
  the **Debian** FFmpeg build inside the proot container, after the FFmpeg PATH leak
  described in `docs/DEVELOPMENT_ENVIRONMENT.md` was diagnosed and fixed. The
  frame-level reproducibility claim holds across FFmpeg builds; the container
  byte-equality assertion is environment-specific, exactly as scoped in the
  reproducibility claim above.
- The replay path is **perception-free by construction and verified at object-code
  level**: `EditDecision.o`'s external surface is Qt plus
  `ReframePlan::readFromJsonObject`, and the disassembled call graph of both
  `ReframePipeline::renderPlan` and the fresh-process child function contains no
  `ReframeIntentParser`, `TargetDetector`, or provider symbol.

## Consequences

- A render produced by the 360 command path can be reproduced from persisted data alone: load the decision, verify the source fingerprint, render the stored plan. No natural-language parsing and no perception provider are on that path.
- Source drift is detected rather than ignored: a missing file and a changed file are reported as **separate failure classes with separate messages**, never collapsed into one.
- Model-free tests cover the artifact in isolation (round-trip determinism, digest rule and stability, schema rejection, invalid-plan rejection, tamper rejection, fingerprint classes, file save/load); the record-level tests cover a valid decision, a corrupt decision under the lenient-record policy, and pre-Objective-16 records that carry no decision at all.
- Replay is verified end to end, including a genuinely fresh OS process that loads the artifact from disk and re-renders, confirming that reproduction does not depend on in-process state.
- Decisions 017–032 are preserved. No new database, ORM, storage format, renderer, or parallel pipeline was introduced; `Project::CurrentSchemaVersion` remains 3.


---

# Decision 034 — Creator Decision Provenance and Immutable Revision

**Status:** Accepted (2026-09-18, 360 Reframing Objective 17; human-locked scope)

## Context

Decision 033 made a render reproducible from a persisted `EditDecision` and stated a forward constraint: persisted decisions are immutable, and a revision must be expressed as a **new** decision with lineage rather than a mutation. Objective 17 implements that constraint and answers creator-control requirements already recorded in `AI_EDIT_CONTRACT.md` §10 (the creator may accept, reject, modify, or request a revision) and Decision 007 (creators retain final authority, and the edit model must distinguish AI-generated decisions from creator modifications).

## Decisions

- **The artifact advances v1 -> v2; existing v1 decisions stay valid.** `EditDecision::CurrentSchemaVersion` becomes 2. The gate accepts every version in `1..CurrentSchemaVersion`, so **no legacy-read shim was required**.
- **A loaded decision RETAINS its own version.** The loader stores the version it read and the serializer writes that stored value, so a v1 decision re-serializes as v1 instead of being silently upgraded in place. This was determined from the actual implementation and is locked by a regression test: a v1 payload loads, re-serializes byte-identically, and its recorded digest still verifies. Upgrading it to v2 during load would change the payload, invalidate the digest, and cause the strict loader to refuse a decision that was previously valid.
- **`origin` and `parentDecisionHash` are OMITTED when unset.** This is a correctness requirement, not a style choice. Writing them unconditionally (even as empty strings) would add payload keys to every legacy decision, changing its recomputed digest so that it no longer matched the stored `decisionHash` — and the strict loader would then refuse a previously-valid decision. When present, both fields ride the canonical payload and are therefore covered by `decisionHash`.
- **`origin` describes how a DECISION was formed, not how a record was produced.** The vocabulary is `command` and `creator-revision`. Replay deliberately does **not** stamp a new origin: `replayEditDecision()` re-uses the same decision so that Objective 16's identical-`decisionHash` invariant holds, and re-stamping an origin would break it.
- **Lineage is a single parent: a chain, never a graph.** A revision is created by `EditDecision::revisedFrom(parent, plan, media, instruction, createdUtc)`, which reads the parent and constructs a new artifact whose `parentDecisionHash` is the parent's digest. There are no multiple parents, no back-pointers, no traversal infrastructure and no topological sorting, and **no mutator exists that changes a stored decision**.
- **Lineage is validated in two distinct places.** At load, `origin` must be in the known vocabulary and `parentDecisionHash` must be exactly 64 lowercase hex characters, or the artifact is refused as malformed. At consumption, `Application::decisionProvenance()` resolves the parent against the records the application actually holds and reports `parentResolved` honestly — a syntactically valid hash is not proof of a valid lineage relationship.
- **Revision reuses the existing pipeline unchanged.** A revision is free text and travels the same path as a command: `ReframeIntentParser` -> `ReframePlanBuilder` -> `ReframePlan` -> render -> record. `runReframeCommandTo()` and `reviseEditDecision()` share one internal implementation carrying an optional `parentDecision`, so there is a single command path and the single append gate is preserved. A revision refuses to overwrite the output of the record it revises, and refuses when the source fingerprint no longer matches.
- **Accept/Reject remains session-only and is not persisted.** No decision-status state machine, and no persisted Superseded/Approved/Modified field. `AI_EDIT_CONTRACT.md` §9's status vocabulary remains conceptual.
- **The silent restore failure is fixed.** `restoreReframeOutputsFromJson()` previously discarded unparseable or non-object render records with no message at all; it now counts them and reports through the existing `backgroundCompleted` status channel, with wording distinct from the unreadable-decision message.
- **Structured logging via `QLoggingCategory`** (category `reelcraft.decision`) for created, loaded, refused and revised. `QLoggingCategory` ships with Qt, so there is no new dependency, and there is no remote telemetry.

## Explicitly out of scope

Target-identity persistence (`TargetIdentityRegistry`, `CreatorTargetSelection`) is a separate future objective. Also out: structured operation-level or timeline editing; direct keyframe editing; the deterministic intent->plan completeness checker (its own objective); an LLM auditor; a `Project` schema bump (`CurrentSchemaVersion` stays 3); retention/compaction of accumulating decisions; remote telemetry; and any new third-party dependency.

## Consequences

- 9 new tests cover v1 compatibility and version retention, hash participation of the new fields, malformed origin/parent rejection, immutable child creation, revision input refusal, replay origin preservation, provenance/lineage reporting, restore error-reporting, and refusal logging.
- Targeted Objective 17 run: 9 focused tests pass. Targeted regression of the affected areas (edit-decision, record persistence/restore, replay, command path, including both ffmpeg-gated replay tests): **31 passed / 0 failed / 0 skipped**.
- The replay path remains perception-free and no existing invariant was weakened: original media stays read-only, decisions stay immutable, creator authority and non-fabrication are unchanged, the replaceable seams and the single append gate are untouched, and serialization/hash determinism is preserved.
- Known limitation: an **unparseable render record** is now reported but is still discarded — unlike an unreadable decision, it is not preserved verbatim. Recorded in `KNOWN_ISSUES.md`.
- Decisions 017-033 are preserved. No new database, ORM, storage format, renderer, parser or parallel pipeline was introduced.


---

# Decision 035 — Objective 18 Scope: Deterministic Intent -> Plan Contract Checker

**Status:** Implemented (2026-09-18). Scoped by human authorization and implemented within the same objective; the scope text below is unchanged.

## Context

The 360 command path turns a natural-language instruction into a structured `ReframeIntent`, then into a validated `ReframePlan`, then into a render. Every stage validates its own invariants, and Objective 14/15 added temporal composition. What no stage does is verify, after the fact, that the **final** executable plan still honours the specific portions of the intent that the architecture defines as executable requirements.

This gap is not hypothetical: Objective 15 existed precisely because a compound command silently lost its camera half. Nothing structural would have caught that; a test did. Objectives 16 and 17 considered a completeness checker and deferred it both times, on the grounds that its rule set needed to be defined against the actual intent->plan mapping rather than against a plausible-sounding idea of one.

Architectural discovery (read-only, recorded in `DEVELOPMENT_LOG.md`) established the mapping, the false-positive hazards, and the one boundary that decides which rules are safe. This decision formally scopes the objective. **It authorizes no implementation.**

## Objective

Add a deterministic, pure contract checker that verifies that the final executable `ReframePlan` honours the specific portions of `ReframeIntent` that the existing architecture defines as executable requirements.

The checker is **not** an AI auditor and does **not** attempt to determine semantic understanding of user intent. It performs no inference, calls no model, and reads no media.

## In-scope rules

### IPC-1 — Output specification fidelity

- **Predicate:** if `intent.hasOutput` is true, `plan.output()` must exactly equal `{intent.outputWidth, intent.outputHeight, intent.outputFps}`.
- **Intent fields:** `hasOutput`, `outputWidth`, `outputHeight`, `outputFps`. **Plan fields:** `output()`.
- **Checkability:** directly checkable (exact comparison, no transformation).
- **NotApplicable:** when `intent.hasOutput` is false, because the caller's default output is intentionally used.
- **Severity:** FATAL.
- **Existing guarantee:** both planners apply the same rule (`ReframePlanBuilder.cpp:32-41`; `ReframeCommandRunner.cpp:289-294` for the speaker path) and `OutputSpec::isValid()` bounds the values, but nothing compares the plan's output back to the intent. `ReframePlan::isValid()` checks bounds only, so this is not a duplicate of it.

### IPC-2 — Requested time-range containment

- **Predicate:** if `intent.hasTimeRange` is true, then without a temporal request `plan.sourceRange()` must **equal** the requested range, and with a temporal request `plan.sourceRange()` must **contain** the requested range.
- **Intent fields:** `hasTimeRange`, `startMs`, `endMs`, `hasTemporalRequest`. **Plan fields:** `sourceRange()`.
- **Checkability:** directly checkable in both forms (the containment form is an inequality over two existing fields; it does not re-derive `applyTemporal`'s arithmetic).
- **NotApplicable:** when `intent.hasTimeRange` is false, because the caller's default range is intentionally used.
- **Severity:** FATAL. A plan narrower than requested silently omits footage the creator asked for.
- **Existing guarantee:** none. `ReframePlan::isValid()` guarantees keyframes and segments lie *within* `sourceRange`, and that segments are ordered and non-overlapping — it never checks that the *requested* range is contained.

### IPC-3 — Temporal edit materialisation

- **Predicate:** if `intent.hasTemporalRequest` is true **and** `intent.temporalError` is empty, the final plan must contain at least one retained segment (`!plan.segments().isEmpty()`).
- **Intent fields:** `hasTemporalRequest`, `temporalError`. **Plan fields:** `segments()`.
- **Checkability:** directly checkable.
- **Severity:** FATAL.
- **Premise verified before recording this scope.** The required verification was carried out: `TemporalEditPlan::resolve()` *can* return an empty list, but `ReframeCommandRunner::prepare()` converts an empty resolution into a hard error (`:113-121`) before any plan is built, and `applyTemporal` copies **every** resolved segment into `plan.segments()` (`:126-133`). A plan reaching the seam with `hasTemporalRequest` therefore always carries at least one segment. The premise holds and the rule is sound.
- **Recorded honestly:** IPC-3 is consequently **already guaranteed by preparation**, not only by `ReframePlan::isValid()`. It is recorded as a **contract assertion at the checker boundary** — guarding against future divergence in `applyTemporal` or a future planner that bypasses temporal resolution — and **not** as new coverage. This is stated so the rule is not later mistaken for closing a gap that already exists.

## Checker contract

The checker must conceptually be a pure function over `(const ReframeIntent &, const ReframePlan &)` returning a deterministic structured report containing an overall verdict, stable rule ID(s), and human-readable violation detail.

No I/O. No global state. No dependencies. No persistence. No mutation. No planner provenance input.

## Integration boundary

The checker will eventually be invoked at **both** existing final-plan points in `ReframeCommandRunner::prepare()`:

1. main path, after `applyTemporal` (`:419`);
2. speaker path, after `applyTemporal` (`:340`).

The command runner must **not** be refactored merely to create a shared convergence point: the surviving rules are path-independent (verified — both paths apply the identical output rule and both call `setSourceRange`/`setOutput`), so a convergence refactor would restructure a tested composition boundary (Decision 025) for no additional coverage. Two call sites are accepted, with the drift risk mitigated by a test that exercises both paths.

## Fatal semantics

A violation is eventually fatal through the **existing** preparation error path: `result.ok == false`, a non-empty `result.error`, no executable plan returned, and nothing persisted. It must never be a silent skip, and it must not introduce a new error channel.

## Intentional exceptions (must never become violations)

1. Temporal edits can intentionally **expand** the final source range (`ReframeCommandRunner.cpp:139-148`).
2. Empty `moves` can intentionally produce a **synthesized centered-forward keyframe** (`ReframePlanBuilder.cpp:42-51`).
3. Missing output intentionally uses the **caller-provided default**.
4. Missing time range intentionally uses the **caller-provided default**.
5. Target references are intentionally **resolved into camera coordinates and not retained** in the executable plan.
6. **Labels and notes are descriptive information**, not executable requirements.
7. **Speaker-path keyframes are planner-owned** (`ReframeCommandRunner.cpp:308`) and therefore cannot be validated against `intent.moves`.

## Excluded rules and why they cannot safely be checked

- **Keyframe-count <-> move-count.** Not decidable from `(intent, plan)`: the speaker planner derives keyframes from speaker-change cuts, so a count rule would false-positive on the speaker path. Checking it would require planner provenance, an input the pair does not carry.
- **Per-move direction correspondence.** Same planner problem, plus moves carrying a `targetRef` have no knowable direction in the pair, because the reference is resolved and discarded. It would also freeze the builder's even time distribution as a contract.
- **Target identity / subject correctness.** The plan stores yaw/pitch only; which target produced a keyframe is not recorded. Not provable from the pair; already governed by the precedence in Decisions 021/022.
- **Media identity.** `ReframeIntent` has no media field; `plan.sourceMediaId` is set from `ReframeCommandRequest` (`:420`). Not a property of the pair.
- **Validation already provided by `ReframePlan::isValid()`:** keyframes within range, segment ordering and non-overlap, bounded and finite camera values, output bounds, minimum frame count. Including any of these would duplicate an existing authoritative validator.
- **Unresolved-target checking.** Already handled by preparation, which makes a non-empty unresolved set a hard error (`:398-401`).
- **Labels and notes.** Descriptive metadata; `label` is never read by planning and `notes` are copied to `ReframeBuildResult.notes` only.
- **"Exactly one keyframe when `moves` is empty".** Over-specification of documented behaviour; asserting it would freeze an implementation detail.

## Explicit exclusions

The following are **not** Objective 18: parser changes; changes to `ReframeIntent`, `ReframePlan`, `CameraKeyframe` or `EditDecision`; persisted schema changes; target identity verification; media identity verification; keyframe-count <-> move-count verification; per-move direction correspondence; labels or notes; unresolved-target checking already handled by preparation; validation already provided by `ReframePlan::isValid()`; parser redesign; target-selection persistence; Accept/Reject persistence; report persistence; telemetry; UI; playback; renderer/executor redesign; LLM/AI auditor; new dependencies; command-runner convergence refactor; timeline/editor functionality.

## Relationship to existing decisions

- **Decision 018** (structured plan boundary, deterministic renderer): the checker consumes that boundary's outputs and changes neither side.
- **Decision 025** (end-to-end command execution boundary): the checker is invoked inside that composition boundary without restructuring it.
- **Decision 031** (temporal editing): the source of the intentional range widening that IPC-2 must permit.
- **Decision 032** (compound commands): the regression that motivates a structural completeness check.
- **Decision 033** (persisted decisions immutable): unchanged; no rule requires touching the artifact.
- **Decision 034** (provenance and immutable revision): unchanged; the checker result is **not** persisted and adds no field to the decision.

## Constraints

- No schema bump is authorized for Objective 18.
- No stored checker result is authorized.
- Persisted `EditDecision` records remain immutable; revision remains new-decision-with-single-parent-lineage.
- Existing validators remain authoritative for the invariants they already enforce; the checker duplicates none of them.

## Implementation definition of done (for when implementation is authorized)

1. IPC-1, IPC-2 and IPC-3 implemented with stable IDs.
2. Pure deterministic checker with no I/O, global state or dependencies.
3. Invoked after final temporal application on **both** the main and speaker paths.
4. Violations are fatal through the existing preparation error mechanism.
5. No persistence occurs for rejected plans.
6. Positive tests cover each rule.
7. Anti-false-positive tests cover: temporal range widening; the empty-moves synthesized keyframe; default output; default range; normal builder plans; the main command path; the speaker path.
8. Determinism is tested.
9. Existing relevant regression tests remain green.
10. The full model-free test suite is run once at the final checkpoint.
11. Documentation is updated consistently.
12. No parser, plan schema, `EditDecision` schema, renderer, playback or dependency changes occur.

## Consequences

- Objective 18 is formally bounded to three path-independent predicates, a pure two-argument function, two call sites, and zero schema, persistence or dependency change.
- The two false-positive hazards (temporal widening; empty-moves synthesis) and the excluded rule families are recorded here so they are not rediscovered later as additions.
- IPC-3 is recorded as a contract assertion whose premise was verified, with its existing coverage stated plainly rather than overstated.
- No implementation is authorized by this decision. Decisions 017-034 are preserved unchanged.


## Implementation outcome (2026-09-18)

**Status: implemented and verified.** The scope above is unchanged; this section records what was built against it.

### What was added

- `app/reframe/ReframeContract.{h,cpp}` (new): `ContractViolation` (stable rule id + deterministic detail), `ContractReport` (`isConsistent()`, `summary()`), and `ReframeContract::check(const ReframeIntent &, const ReframePlan &)`.
- Both `.pro` files gained the new source and header. No dependency, flag or configuration change.
- Two integration call sites in `ReframeCommandRunner::prepare()`, each immediately after `applyTemporal`: the speaker path and the main path. Both use the existing error mechanism: `result.error = contract.summary(); return result;` before `result.ok` is set and before any plan is returned. No convergence refactor was performed.

### Verified behaviour

- **IPC-1** applies only when `intent.hasOutput` is true; compares width, height and fps exactly; NotApplicable otherwise.
- **IPC-2** requires exact equality without a temporal request, and containment with one, so the intentional widening by `applyTemporal` cannot false-positive. NotApplicable without `hasTimeRange`.
- **IPC-3** fires only for a request that is present and error-free with an empty `plan.segments()`; NotApplicable otherwise, including when `temporalError` is non-empty.
- Violations are appended in fixed rule order (IPC-1, IPC-2, IPC-3) so the report and its summary are deterministic; numbers are formatted with `QString::number(value, 'g', 10)` for locale independence.

### Tests

- 6 new tests: `reframeContractOutputFidelity`, `reframeContractTimeRange`, `reframeContractTemporalMaterialisation`, `reframeContractReportsAreDeterministic`, `reframeContractAcceptsRealPipelinePlans`, `reframeContractAcceptsSpeakerPathPlans`.
- Focused run: 8 passed / 0 failed / 0 skipped (6 new plus init/cleanup).
- Targeted regression across the affected area (runner, builder, intent, plan, temporal, speaker, pipeline and application command paths, plus the new tests): **55 passed / 0 failed / 0 skipped**.
- Anti-false-positive coverage exercises the real pipeline: direction-only (default output and range), output-requesting with the synthesized centered-forward keyframe, an explicit requested range, a resolved temporal edit, a resolved-subject plan, and the speaker path with planner-owned keyframes.

### Recorded limitation (honest)

The rules are currently satisfied **by construction** on both paths: the builder and the speaker planner both derive output and range from the same intent-or-default rule, `applyTemporal` only ever widens, and preparation already refuses an empty temporal resolution. **No reachable input today can produce a violation**, which is precisely why the checker exists: it is an executable contract that fails loudly if those properties are broken by a future change. Consequently the FATAL wiring is verified by the rule-level tests plus inspection of the two call sites, and is **not** covered by an end-to-end integration test — triggering it would require injecting a plan that the current planners cannot produce, which would mean adding a seam outside this objective's scope.

### Constraints honoured

No parser, `ReframeIntent`, `ReframePlan`, `CameraKeyframe` or `EditDecision` change; no schema bump; no persisted checker result; no persistence on rejection; no new dependency; no LLM; no renderer or playback change; no convergence refactor. Decisions 017-034 are preserved.


---

# Decision 036 — Source-Media Playback Architecture

**Status:** Accepted (2026-09-18, 360 Reframing Objective 19)

## Context

The product direction is that Reelcraft must first become a working 360 video editor, and the prerequisite for improving automatic reframing is that real 360 footage is *observable*. Until this objective the application could show a single decoded frame of the active media and step through time one frame at a time, while continuous playback existed only for rendered results (Objective 13, Decision 030). Watching 360 footage, scrubbing it, and looking around while it plays were not possible.

## Decisions

- **Source playback is a separate pipeline that reuses the existing seams, not a generalisation of the rendered-result path.** It owns its own `FrameSource`/`FramePump`/`Player`; the two playbacks share the single event-loop timer, which dispatches by which player exists, and they are mutually exclusive (starting either stops the other). The tested Objective 13 path is deliberately left unperturbed; unifying the two was rejected as a change to a tested boundary for no functional gain.
- **Continuous decoding, never a process per displayed frame.** Playback uses the existing persistent FFmpeg subprocess seam (`FfmpegFrameSource` + `FramePump` + `Player`). The requirement that playback must not regress into decode-one-frame-per-process is therefore structural: one decoding process serves the whole session.
- **The decoded stream is a bounded 2:1 proxy chosen by the application (1024x512), not the native source resolution.** `FfmpegFrameSource` already scales inside FFmpeg, so the caller supplies the geometry and pays one decode per presented frame whose cost is independent of the source resolution. For a 4K equirect source this is what makes viewing viable at all on the development device, where a single native frame decode measured seconds. **The original media is only ever read.**
- **Aspect ratio is preserved by letterboxing when the media aspect differs from the proxy.** `FfmpegFrameSource::open` gained an optional aspect-preserving scale mode; stretching a declared-equirect frame would silently corrupt the 360 projection. The option is additive and defaults to the previous behaviour, so the rendered-result path is byte-for-byte unchanged.
- **Seeking reopens the continuous stream with an input seek.** `FfmpegFrameSource::open` gained an optional start offset (`-ss` before `-i`), and the application carries the offset so the reported position is absolute source time. A seek preserves whether playback was running, and a seek performed while paused leaves the stream paused at the new position and resumable.
- **The source frame rate is learned once per open, through the existing ffprobe seam.** `MediaDurationProbe` gained `frameRate()` with a default implementation that honestly reports *unknown*, so existing probes and test doubles stayed valid; `FfprobeDurationProbe` overrides it by parsing `r_frame_rate`. Playback is paced at the source rate when known and at a documented default otherwise. The interval is a playback pacing parameter, never media metadata, and is never written anywhere.
- **Presentation reuses the existing viewer path.** Source frames are emitted and routed through the same projection-aware preview path as the single-frame preview, so 360 footage uses the equirectangular camera and the authoritative `ViewportState` keeps working during playback: looking around while the footage plays needs no new viewer code.
- **Audio is DEFERRED, deliberately and explicitly.** Reelcraft has no audio output: rendered results are silent by design, `QtMultimedia` was explicitly deferred by Decision 017, and the media seams decode with `-an`. Adding synchronized audio playback would introduce a new output subsystem, a new dependency and a seek/pause synchronization model — a disproportionate expansion of a video-playback objective. Source playback is therefore **video-only**, and it is kept extensible: the frame/timestamp emission point is the natural place for an audio scheduler to attach, and the reopen-on-seek model already defines what a seek means.

## Consequences

- Real 360 footage is now watchable in Reelcraft: play, pause, seek, look around, resume, and clean end-of-media handling, all without a process per frame.
- The emitted proxy frames and the reported position are the intended feed for the future automatic-reframing work, so perception can consume the same stream instead of duplicating the decoding pipeline. **Caveat recorded for that objective:** the 1024x512 proxy is a *viewing* budget, and detection may require a larger or separate decode; the proxy size is an application constant, not a contract.
- Known limitation: playback is silent. The smallest viable follow-up is an audio output seam driven by the same position/seek model (a `QAudioSink`-based or external-player implementation behind an injectable interface), which should be scoped as its own objective.
- Known limitation: the source frame rate is discovered by an ffprobe call at each open (start and each seek). An unknown rate falls back to the default pacing, so playback speed can differ from the source rate when ffprobe is unavailable or reports nothing.
- No parser, plan, decision-artifact, renderer, export or perception change. Decisions 017-035 are preserved.


---

# Decision 037 — Persistent Render Decoding Architecture

**Status:** Accepted (2026-09-18, 360 Reframing Objective 20)

## Context

Reelcraft must first become a working 360 video editor, and the reframing engine is the part that does the work. Rendering a reframe means decoding one source frame per output frame, and the provider the pipeline used (`FfmpegSeekFrameProvider`) satisfies every request by spawning one FFmpeg process with an input seek. A render of N frames therefore created O(N) decoder processes. On the development device this dominated the cost of every real render and made longer renders progressively worse, and it is the reason Objective 20 was scoped with a single measurable requirement rather than a general "make rendering faster".

The constraint that shapes every choice below is that this must be a **throughput** change and not a **behaviour** change. The deterministic render contract (Decision 018) says the renderer executes an already-validated plan; a render that is faster but produces different frames would silently invalidate every persisted decision and every equivalence guarantee Reelcraft has accumulated.

## Decisions

- **Continuity is exploited where the work actually is.** `ReframeRenderer::render()` requests timestamps monotonically non-decreasing because `plan.frameTimeMs()` walks the plan's ordered segments, so consecutive requests are normally adjacent frames of the same stream. The provider therefore keeps one `FfmpegFrameSource` open at an **anchor** and answers subsequent requests by reading forward from it. Process creation follows anchors, not frames.
- **The forward replay window is bounded (3000 ms), and a jump beyond it re-anchors.** Unbounded replay would let a single far-forward request decode everything in between, which is worse than the old behaviour for exactly the request pattern that motivated streaming. Re-anchoring also re-aligns the cursor with the true source timeline, which is what makes frame selection stable after a jump. Anchoring stays an O(anchors) cost, never O(frames).
- **The positioned seek path is retained as the fallback and as the geometry-discovery path, and the provider never guesses.** An unknown frame rate, invalid input, an unreadable source, the end of the media, or any stream failure falls back to the seek path, so the worst case is the previous cost and never a different frame. Geometry is learned from the first anchor's seeked frame because a rawvideo stream carries no dimensions.
- **Frame identity with the previous path is a contract, not a coincidence.** The persistent stream delivers `Format_RGB888` while the seek path delivers the PNG-decoded format, and a mismatch here changed rendered output during development; frames are therefore normalised to the seek path's format before delivery. The far-forward anchor additionally depends on an input-seeked stream's first frame being exactly the frame the positioned seek returns at that timestamp — verified directly against FFmpeg and locked by a test.
- **The source frame rate is read once per source through the existing ffprobe seam (Decision 027), and an unknown rate disables streaming entirely.** Pacing the sequential cursor requires a frame step; inventing one would shift frame selection. With no rate the provider behaves exactly as it did before.
- **The provider is the only thing that changed.** `ReframePlan`, `CameraPath`, `ReframeRenderer`, `ReframePipeline::renderPlan`, the decision artifact and every persisted schema are untouched, so persisted decisions remain replayable and byte-comparable.

## Consequences

- The core requirement is met and measured: on a real 360 render of 6 output frames, FFmpeg process creations fell from 9 to 5, and no decoder process scales with the frame count. The structural assertion is tested directly (60 consecutive requests => at most 3 stream opens, exactly 1 geometry decode).
- Output equivalence is proven, not assumed: streaming and seek rendering produce identical decoded frames and byte-identical MP4 containers for continuous, trimmed and multiple disjoint temporal segments.
- The streaming path is an optimisation with a conservative failure mode. Any source it cannot stream — an unknown frame rate above all — keeps the previous behaviour exactly, which is why the fallback is stated as part of the architecture rather than as an implementation detail.
- Suite cost is recorded honestly: the focused real-FFmpeg provider tests add roughly 280 s and the full suite now runs about 563 s.
- A build-integrity defect surfaced while debugging this objective and is fixed and recorded: a generated makefile predated the new header and did not declare it as a dependency of the translation unit that instantiates the provider on the stack, so a header change never recompiled it and the binary mixed two revisions of the class. The failure mode (stack-canary abort plus a spurious frame mismatch) is recorded in `KNOWN_ISSUES.md`, and the operational requirement in `DEVELOPMENT_ENVIRONMENT.md`.
- Not in this objective: audio, perception/detection/tracking, camera-path generation, auto-reframing, export presets, equirect export, UI, `EditDecision`/`ReframeIntent` schema changes, `ReframePlan` redesign, and any change to the Objective 19 1024x512 playback proxy. Decisions 017-036 are preserved.


---

# Decision 038 — Media Analysis Is a Versioned, Layered, Provider-Neutral Artifact Referenced by the Project

**Status:** Accepted (2026-09-18, Objective 21)

## Context

The documented workflow (MASTER_GUIDE §3, AI_EDIT_CONTRACT §2) places **Media Analysis** between source media and creator intent, and PROJECT_MODEL §6 defines analysis as *derived data* that must stay distinguishable from creator decisions. Nothing implemented that stage: perception ran per command, over a command-scoped range, evenly sampled, and was discarded immediately, and the Project schema (v3) had no place to record what had been learned.

## Decision

- **Analysis is a persisted artifact with its own compatibility gate.** `MediaAnalysis` owns a `schemaVersion` independent of the Project schema, following the `EditDecision` artifact conventions exactly: a strict envelope loader, version-retaining serialization, and preservation of anything it cannot interpret.
- **The artifact is a small closed ENVELOPE containing named CAPABILITY LAYERS.** The envelope carries only addressing, provenance, lifecycle, coverage and confidence. Capability-specific observations live inside a typed layer. A new capability is a new layer kind, never a new envelope field — this is what prevents the catch-all schema the design explicitly avoids.
- **Layers are independently versioned** (`layerVersion`) and independently available, so one capability can evolve, or fail, without invalidating the others.
- **The Project stores REFERENCES, never analysis.** An additive `analysisRefs` section holds a small record per media (mediaId, artifactId, artifactPath, source fingerprint). `Project::CurrentSchemaVersion` stays **3**: the section is additive and its absence reads back as an empty list, exactly like `viewerState`/`activeMediaId`/`reframeOutputs`.
- **Analysis is a performance and review optimization, never a correctness dependency.** A project must open, render, replay and review with every analysis artifact deleted, moved, stale, unreadable, or referring to missing media. Each of those is a reported STATUS, never a load failure and never corruption.
- **Provider-native output never enters the core model.** Tier-1 normalized observations are Reelcraft types; Tier-2 provider output is retained opaquely and is never interpreted by core. The provider adapter owns the mapping.
- **Source-reference vocabulary is shared, not duplicated.** `core/MediaSourceReference` now defines the source reference and the `Matches`/`FileMissing`/`FingerprintMismatch` status used by both `EditDecision` and `MediaAnalysis`, so the two artifacts cannot drift apart in field names, fingerprint semantics or JSON shape.

## Consequences

- Objective 21 delivers the first two capabilities — `technical` (deterministic, no model, from the existing ffprobe seam) and `targets` (the existing 360 resolver, tracker and equirect view coverage, unchanged).
- Every future capability (transcript, scene boundaries, quality, salience, B-roll) plugs into the same envelope without new persistence, lifecycle, invalidation or provider machinery.
- A damaged or missing reference is reported through the existing status channel and skipped; analysis can never make a project unopenable.
- Analysis for large media does not bloat the project file, and personal/derived data stays separable from the shareable project.
- No new dependency, no ML runtime, no LLM. Decisions 017-037 preserved.

---

# Decision 039 — Analysis Is Evidence, Never Editorial Decision

**Status:** Accepted (2026-09-18, Objective 21)

## Context

The whole point of separating AI reasoning from deterministic execution (Decision 001) is that the decision layer and the execution layer have different trust properties. A media-analysis artifact that quietly accumulates editorial judgements would reintroduce the coupling that boundary exists to prevent, and would make "what did the footage show?" indistinguishable from "what did the system decide?".

## Decision

- **Analysis may assert what is observable; it may never assert what to do.** There is no `ReframePlan`, no camera keyframe, no cut list, and no A-roll/B-roll label anywhere in the analysis model.
- **The test to apply:** if two creators with different intents could disagree about a statement, it is reasoning, not analysis. Positions, times, labels, metrics, associations and confidence are analysis; "this is a usable A-roll candidate", "this is boring", "cut here" are reasoning.
- **Salience and quality are split deliberately:** the METRICS are analysis, the RANKING and SELECTION are reasoning.
- **Reasoning consumes `(MediaAnalysis, ProjectContext, ReframeIntent)` and produces only the existing validated `ReframePlan`.** No new plan type is introduced; whatever reasoner appears later — deterministic rules today, a model later — must emit that structure, exactly as `ReframeIntent` was designed to be produced by a future provider.
- **Analysis may be absent and reasoning must still work**, degrading honestly rather than assuming silence.

## Consequences

- The deterministic engine, the contract checker (Decision 035) and replay (Decision 033) are untouched by any future analysis capability.
- The evidence→plan boundary is now explicit and testable at the model level: a plan cannot be smuggled into an artifact that has no field for it.
- Explainability and grounding belong to the decision, not the analysis. A future plan-level explanation field is a separate, additive decision and is NOT part of Objective 21.

---

# Decision 040 — Analysis Is Never on the Deterministic Replay Path

**Status:** Accepted (2026-09-18, Objective 21)

## Context

Decision 033 guarantees that a persisted `EditDecision` reproduces a render byte-for-byte with no parser and no perception provider on the replay path, verified at object-code level. Analysis is produced by models and is therefore non-deterministic. Letting it influence replay would silently destroy the strongest reproducibility guarantee the project has.

## Decision

- **Replay consumes `EditDecision::plan()` and nothing else.** `replayEditDecision()` and every deterministic execution path must remain free of `MediaAnalysis`.
- **Analysis is deletable without consequence.** Deleting every analysis artifact must leave rendering, replay and project loading fully functional; the worst case is the previous behaviour of resolving perception on demand.
- **A stored decision is never rewritten or supplemented by analysis.** Learning, preference and analysis may influence FUTURE reasoning only.
- **The invariant is verified behaviourally and must stay verified.** `replayIsIndependentOfMediaAnalysis` renders and replays with no analysis present, then with a resolving artifact plus a dangling reference, then after deleting the artifact, and asserts the decoded frames and the stored decision hash are identical in every case.

## Consequences

- Non-deterministic perception can never leak into a reproducible render.
- The test is a permanent regression guard: a future change that reaches for analysis during replay fails it.
- Combined with Decision 036 (silent renders) and Decision 033 (perception-free replay), the reproducibility contract now covers the whole analysis stage.

---

# Decision 041 — Capability Layers Are Independently Available, Coverage-Aware, and Explicitly Unavailable Rather Than Empty

**Status:** Accepted (2026-09-18, Objective 21)

## Context

A video may have person tracks and timings available while transcription or scene understanding is unavailable. The dangerous failure mode is not a missing capability but an ambiguous one: an empty result list is indistinguishable from "nothing was there", and a silent absence is indistinguishable from "we never looked".

## Decision

- **Seven explicit layer states:** `NotStarted`, `InProgress`, `Partial`, `Complete`, `Failed`, `Unavailable`, `Stale`.
- **`Unavailable` (this environment cannot produce the capability) and `Failed` (it was attempted and errored) are different from each other AND from an empty successful result.** Both non-results must carry a deterministic explanatory reason. This follows the precedent already set by `SpeakerAnalysis::available`, `SpeakerVerdict::Unavailable` and the probe seam's "unknown" defaults.
- **Coverage is mandatory for any state that carries evidence.** A layer records the time ranges it actually covers, so "no observation at 07:30" is distinguishable from "we never looked at 07:30". Coverage means *the span the sampling covers*, not a claim that every frame in it was examined; the sampling interval is persisted in the layer spec so the difference is explicit.
- **Source validity and analysis freshness are separate axes and are never collapsed.** Source validity is `Matches`/`FileMissing`/`FingerprintMismatch`; freshness is whether the artifact was produced by the specification currently in use. An artifact can be fresh about a file that has moved, or stale about an unchanged file.
- **Everything the build cannot interpret is preserved verbatim**, so an older build can never destroy a newer capability's data; a re-saved artifact re-emits unknown layers byte-for-byte.

## Consequences

- A partial analysis is usable and honestly labelled; truncation by budget or a skipped sample produces `Partial`, never a silent claim of the whole span.
- The UI/status layer can explain *why* something is missing instead of showing a blank.
- Objective 21's implementation deliberately exercises this: with no detector configured the `targets` layer is recorded `Unavailable` with a reason, and opens no decoder at all, and this is asserted by test.

---

# Decision 042 — One Decode Pass Per Analysis Run at a Recorded Perception Resolution

**Status:** Accepted (2026-09-18, Objective 21)

## Context

A single 4K frame decode costs seconds on the development device, so whole-video analysis at source resolution is impossible rather than merely slow. The viewer's 1024x512 display proxy exists for display latency and is not a perception budget. Objective 20 established that decoder processes must not scale with the amount of work.

## Decision

- **Exactly one persistent visual decoder per analysis run**, opened at the configured **perception resolution**, decoding strictly sequentially and sampling on a configured interval. Never one process per sampled frame; never source resolution by default.
- **The perception resolution is its own concept.** It is not the viewer display proxy and not the source resolution. It is explicit, configurable, and **persisted in the layer specification**, because an observation is only comparable with another made at the same settings.
- **The sampling interval is persisted too**, because it bounds the temporal precision of every conclusion drawn from the layer.
- **Sampling never invents a timestamp.** The probed duration is the END of the last frame, so the analysis scope is clamped to the last decodable timestamp rather than requesting a frame that was never encoded.
- **The specification identity is a deterministic hash** over perception resolution, sampling, participating capabilities and provider identities, so swapping a model or changing a resolution invalidates a stored artifact instead of silently reusing it.
- Audio-only capabilities never open the visual decoder.

## Consequences

- Whole-video analysis is bounded by the scope and the sample budget, not by the number of capabilities; adding a second visual capability costs no extra decode.
- `decoderOpens` is surfaced on the run result, so a violation of the single-decoder contract would be caught rather than hidden.
- Suite cost is recorded honestly: the real-media analysis tests add roughly 100 s on this device.
- Sampling precision is a recorded limitation: observations on a 1 s grid cannot justify frame-accurate edits. Fine-grained placement requires targeted re-analysis, which the layer spec and coverage model make decidable.


---

# Decision 043 — Subject-Follow Instructions Execute as a Camera Path Through the Resolved Track

**Status:** Accepted (2026-09-18, 360 Reframing Objective 23)

## Context

By Objective 21 the pipeline could already carry a natural-language instruction to a deterministic render: the parser produced a `ReframeIntent`, `ReframeCommandRunner::prepare()` resolved subject references through the replaceable detector/tracker, `ReframePlanBuilder` produced a validated `ReframePlan`, and `ReframePipeline` rendered it. What it could not do was *follow*.

`ReframePlanBuilder` emits exactly one keyframe per camera move, using a single resolved direction per subject. A follow instruction ("follow me", "keep me centered") is one move, so it produced **one keyframe** — a locked-off shot aimed at wherever the subject happened to be, for the whole range. The flagship instruction class therefore rendered as a static camera.

Meanwhile `TargetTrackPlanner::planTrack()` — which converts a resolved track's time-ordered observations into up to 40 camera keyframes, and is unit-tested — was **never reached by any production code path**. The trajectory existed; nothing consumed it.

## Decision

- **The intent records whether a clause asks for continuous framing or a one-shot aim.** `ReframeCameraMove` gains an in-memory `followSubject` flag. The parser sets it for follow-class clauses ("follow X", "keep me centered", "keep X centered") and leaves it clear for aim-class clauses ("look at X", "move to X", "centered on X", "center X") and for explicit directions. `ReframeIntent` is never persisted (EditDecision stores the resolved plan), so this carries no schema or compatibility impact.
- **A follow instruction is executed as a camera path through the resolved track.** For a single target-referencing follow move, `ReframeCommandRunner::prepare()` builds the plan with the existing `TargetTrackPlanner` from the subject's track and uses it in place of the builder's single-direction plan.
- **Scope is narrow by construction.** Only a single target-referencing follow move takes this path. Explicit directions, multi-move camera paths, speaker commands, direction-only commands and every existing test keep the previous behaviour. Aim instructions deliberately remain a single fixed direction.
- **The existing builder stays the validity gate and the fallback.** It runs first and unchanged; the follow plan replaces it only when the track has usable observations inside the range. Otherwise the command degrades to exactly the previous fixed camera and adds a note saying why, rather than failing.
- **No new seam, no new type, no new dependency.** The plan, camera path, renderer, playback, replay and persisted-decision semantics are untouched, so determinism, immutability and replay isolation are unaffected.

## Consequences

- "Follow me while I'm walking" and "keep me centered" now render a real tracking camera instead of a locked-off shot, demonstrated end to end on a real encoded 360 equirect clip (raw footage → English instruction → validated plan → rendered MP4).
- The trajectory-to-camera-path stage listed as a next candidate in `NEXT_TASK.md` has its first step done: the resolved trajectory reaches the camera path. What remains of that candidate is *quality* — denser tracking and a smoothing/framing layer — not connectivity.
- **Recorded limitation:** the follow path is only as good as the sampling. The resolver samples the instruction range (five timestamps by default), so a follow path has at most that many keyframes; smoother following needs denser resolution sampling, which is already a caller/config concern (`resolveTimestamps`, `maxResolveSamples`) and was deliberately not changed here.
- **Recorded finding (not changed):** on real footage the covering-view detector can report the same subject from two overlapping cover views with centroids further apart than the tracker's default 8-degree merge distance. Observed at 9.6 degrees for a synthetic subject much larger than a person; the tracker then keeps two identities and the command honestly refuses as *ambiguous* rather than guessing. A fixture-sized subject needed a wider `mergeDistanceDeg` to keep one identity. This is a perception-tuning question in Decision 019 territory, deliberately left alone, and is the natural next quality item for follow accuracy.
- Creator Memory remains an unrecorded future topic; the number 043 was previously referenced prospectively for it in the Objective 21 development log, but no decision was ever recorded under it.


---

# Decision 044 — Follow Instructions Sample the Trajectory at Their Own Temporal Resolution

**Status:** Accepted (2026-09-18, 360 Reframing Objective 24)

## Context

Decision 043 connected the resolved subject trajectory to the camera path, but the trajectory itself was still sampled with **one budget shared by every subject command**: `ReframeCommandRequest::maxResolveSamples`, default 5, applied to the whole instruction range. Five samples is ample for *aiming* at a subject — an aim uses a single representative direction — and far too sparse for *tracking* one, where it yields a camera path that moves in five steps across the entire instruction and, measured, can move further per step than the tracker's association gate, splitting one subject into several identities.

## Decision

- **The trajectory's temporal resolution becomes a follow-specific property.** `ReframeCommandRequest` gains `followSampleIntervalMs` (default **250 ms**) and `followResolveSamplesMax` (default **24**), used **instead of** `maxResolveSamples` when the instruction follows a subject.
- **Expressed as an interval, not a count.** A fixed count degrades with duration — 25 samples over a 60-second clip is a 2.5-second step, which is the same coarseness the objective exists to remove — whereas an interval holds temporal resolution constant and lets the maximum bound the decode and detection cost regardless of clip length.
- **Scope is the follow path and nothing else.** Aim, direction and speaker commands keep `maxResolveSamples` and their existing behaviour and cost. An explicit caller-supplied `resolveTimestamps` list still wins over both budgets.
- **The density is evidence-based, not chosen to maximise keyframes.** Measured on a fixed 4-second fixture whose subject sweeps 80 degrees:

  | interval | cap | samples | observations | keyframes | spacing | angular span |
  |---|---|---|---|---|---|---|
  | 1000 ms | 40 | 5 (the old budget) | 5 | 5 | 1000.0 ms | 78.7° |
  | 500 ms | 40 | 9 | 9 | 9 | 500.0 ms | 78.7° |
  | **250 ms** | **24 (default)** | **17** | **17** | **17** | **250.0 ms** | **78.7°** |
  | 100 ms | 40 | 40 | 40 | 40 | 102.6 ms | 78.7° |
  | 100 ms | 12 | 12 | 12 | 12 | 363.6 ms | 78.7° |

  Density translates one-for-one into camera keyframes, spacing follows the interval exactly, and **the angular span is identical at every density** — denser sampling changes temporal resolution and not the trajectory. The cap binds as designed, and 250 ms is chosen because it makes the segment length shorter than the planner's own 40-keyframe ceiling would matter for realistic ranges while bounding a follow command to at most 24 decodes.

## Consequences

- A follow camera path now has roughly 250 ms segments instead of roughly 1000 ms, i.e. about 3.4x the keyframes on the measured fixture, without any change to the trajectory, the planner, the renderer or the persisted plan.
- Denser sampling also **improves association**: smaller per-step angular motion keeps a moving subject inside the tracker's association gate, so one subject stays one identity. (A related measured fact: with steps of 40 degrees the tracker splits the subject, which is why an explicit three-timestamp list is not a usable override for a fast-moving subject.)
- Cost is one frame decode plus one detection pass per sample, bounded at 24 per follow command. Where no frame provider is injected, each sample is still its own decoder process — the recorded follow-up, and a resolution-seam change that belongs to its own objective rather than this one.
- No perception threshold, detector, cover-view association, identity semantic, planner, renderer or persisted-schema change. The Objective 23 duplicate-identity finding (merge distance) is untouched and still awaits its own Decision 019 investigation.


---

# Decision 045 — Trajectory Resolution Reuses the Persistent Decoding Stream

**Status:** Accepted (2026-09-18, 360 Reframing Objective 25)

## Context

Decision 044 raised follow-command sampling to as many as 24 trajectory samples. Resolution then opened **one FFmpeg process per sample** (`FfmpegSeekFrameProvider` → `FrameExtractor::extractFrameAt`), so the cost of a follow command scaled with its sampling density at a fixed cost per sample.

That fixed cost is not decoding. Measured on this device, a single-frame decode of a 360x180 clip costs about the same as one of a 4K source (~2.5 s), because the dominant term is process start-up and the syscall cost of the proot container (~1 ms for `getpid()`, ~3.4 ms for a `stat()`), not the pixel work. Reducing the number of invocations is therefore the lever, exactly as Objective 20 found for rendering.

## Decision

- **The default trajectory-resolution provider becomes `ReframeStreamFrameProvider`** — the provider the render path already uses (Decision 037) — constructed with a source frame rate read once through the existing `FfprobeDurationProbe` seam. This mirrors `ReframePipeline::renderPlan`, which has used the same pattern since Objective 20.
- **No new seam, type, interface or dependency.** `ReframeStreamFrameProvider` is already a `ReframeFrameProvider`, so the replaceable resolution seam is unchanged; only the default implementation behind it changes.
- **Correctness is inherited, not assumed.** Objective 20 proved the streaming provider's frames byte-identical to the positioned seek path's, and its bounded-window and re-anchor logic is what keeps frame selection aligned with the source timeline. Detection therefore sees the same pixels it saw before.
- **Failure degrades to the previous behaviour.** The provider falls back to the positioned seek whenever it cannot stream — an unknown frame rate, a far-forward or backwards request, an unreadable or ended source, a failed stream open. An unknown frame rate means nothing is streamed and every request is served exactly as before, so the worst case is the previous cost, never a different frame.
- **The renderer and playback are not involved.** Resolution keeps its own provider instance; nothing about the render path changes.

## Measured

Same source, same timestamps, same follow command, six samples, counting every FFmpeg invocation through a wrapper:

| | decoder processes | resolution time | observations | keyframes |
|---|---|---|---|---|
| positioned seek (previous default) | 6 | 15.2–16.0 s | 6 | 6 |
| persistent stream (new default) | **3** | **9.9–10.1 s** | 6 | 6 |

Observations, keyframe count, keyframe timestamps and every keyframe's yaw and pitch are **exactly equal** between the two; the assertions compare them exactly rather than approximately. The real-media follow test (eight samples) fell from 37.6 s to 26.8 s.

The persistent path's process count is bounded by *anchors* — one geometry decode, plus one stream open per bounded window — rather than by sample count, so the saving grows with sampling density; the six-sample measurement understates it, and at the shipped maximum of 24 samples the ratio is roughly six to one.

## Consequences

- A dense follow command no longer pays a process per sample. The fixed costs are one ffprobe call for the frame rate and one geometry decode; both amortise across the samples.
- **Recorded caveat:** within a bounded window the stream decodes the frames between samples at native resolution. That is cheaper than a separate invocation because the per-invocation cost is start-up rather than decoding — the 360x180 and 4K figures above are the evidence for that — but no direct measurement on a 4K 360 source was possible here, because the suite deliberately uses a small proxy rather than the 779 MB original.
- **Recorded cost:** trajectory resolution now performs one extra ffprobe call per pass.
- Nothing about perception, thresholds, cover-view association, identity semantics, the planner, camera-path semantics, the renderer, the parser or any persisted schema changed, and the Objective 23 duplicate-identity finding still awaits its own Decision 019 investigation.


---

# Decision 046 — Follow Trajectories Are Smoothed by a Centred Circular Average Inside the Planner

**Status:** Accepted (2026-09-18, 360 Reframing Objective 26)

## Context

Objectives 23-25 made the follow path connected, densely sampled and cheap to resolve. What remained was its quality as camera movement. `TargetTrackPlanner` emitted one keyframe per resolved observation, carrying the detected direction verbatim, and `CameraPath::stateAt` interpolates linearly between keyframes along the shortest yaw path. The follow path was therefore **piecewise linear through the raw detections**: its velocity is discontinuous at every keyframe, and every sample-to-sample wobble of the detector appears directly in the camera movement. Denser sampling (Decision 044) reduced the size of each wobble but not its presence.

Inspection also settled two placement questions. `TargetTrackPlanner::planTrack` is called **only** by the follow path in `ReframeCommandRunner` — the speaker planner builds its own keyframes — so smoothing there cannot affect any other command. And the planner's stated job is exactly "convert a resolved target track into the existing deterministic reframing representation", which is where trajectory shaping belongs; a separate layer would have added an abstraction with a single caller.

## Decision

- **Smoothing happens inside `TargetTrackPlanner`**, controlled by a new `Config::smoothingWindow` (default 5; 1 disables). It is deterministic, model-free, uses only the already-resolved trajectory, and depends on nothing from the detector, identity resolution, decoding or the renderer.
- **The window is centred and shrinks SYMMETRICALLY at the ends** rather than being clipped one-sided. Three properties follow, and each is asserted:
  - the average is never taken over a one-sided neighbourhood, so the path is **not delayed**, and a constant-velocity (linear) trajectory is reproduced **exactly** — smoothing removes discrete jitter without flattening genuine motion. The pre-existing `targetTrackPlannerBuildsFollowPlan` test, whose observations are a linear ramp, passes unchanged and is the independent evidence for this;
  - every smoothed value lies inside the range of the raw values it averaged, so smoothing **cannot overshoot**; and
  - keyframe **times, count and ordering are never altered**, so start/end timing, trajectory direction and total angular span survive by construction.
- **Yaw is averaged on the circle.** The sequence is unwrapped into a continuous angle before averaging and re-normalised afterwards, so a transition across the ±180° boundary is the short way round. Averaging the raw sawtooth would fold 178° and −178° to 0° — a 180° error — which is the failure this requirement exists to prevent.
- **Framing stays centred; no offset policy is introduced.** The follow path aims directly at the subject, and that is what the command semantics say: "follow the person", "keep me centered" and "keep X centered" all mean centred framing. Inventing a lead-room or rule-of-thirds offset would change the meaning of those commands, so the framing envelope remains the keyframe field of view and the subject stays inside it because a centred average never moves the camera further from the subject than the raw extremes did.

## Consequences

- Follow camera movement is smoothed without any change of shape downstream: the same number of keyframes at the same times, with directions that no longer carry sample-to-sample jitter.
- Measured on a synthetic wobble of ±20° per sample: the largest angular step between consecutive keyframes falls from **40.0° to 4.0°**, while a linear ramp comes through unchanged to within 1e-9 and a stationary subject stays exactly still.
- Yaw wraparound is handled: a trajectory crossing the boundary keeps every step at 8.0° and every direction beyond 150°, instead of folding toward zero.
- **Contract note:** `planTrack`'s output is now a *smoothed* trajectory rather than one passing exactly through each observation. The change is small, bounded, and cannot move a direction outside the raw envelope, but it is a genuine change to what that function returns and is recorded as such here.
- No perception, threshold, identity, sampling-density, decode, parser, renderer, camera-path or persisted-schema change. Aim, direction, speaker and multi-move commands never reach this planner and are untouched.
- Cost is negligible: it operates on already-resolved keyframes and adds no decode, detection or allocation beyond two small lists.



---

# Decision 047 — Covering-View Duplicates Are Consolidated by Overlapping Angular Footprints, Not by a Larger Merge Distance

**Status:** Accepted (2026-09-18, 360 Reframing Objective 27)

## Context

Decision 019 gave the 360 tracker a deterministic duplicate-consolidation step: observations whose spherical centroids lie within a flat, person-tuned `mergeDistanceDeg` (default **8°**) are treated as one identity. Objective 23 found the case that rule cannot handle and recorded it as a finding rather than acting on it: **one real subject reported by two overlapping covering views with centroids further apart than the merge distance**, which left two identities and made every reference to that subject resolve honestly as *ambiguous*. Objectives 24, 25 and 26 each recorded that the finding was deliberately untouched and awaited its own Decision 019 investigation.

The covering-view plan (`EquirectViewPlan::coveringViews`) renders overlapping tangent views so that nothing on the sphere falls outside every view — that is what "covering" means — so **a subject near a view boundary is necessarily seen twice**: once by the view that owns its centre, and once as a partial silhouette at the edge of its neighbour. `EquirectProjection::detectionToDirection` maps a detection box centre to a direction, so a **clipped** box reports a centre pulled toward the clipping view's own axis. Where that happens the two reports of a single subject can sit further apart than a person-sized threshold while being, geometrically, the same person.

## Investigation

The failure was reproduced from the real geometry rather than from an assumption, by driving the production covering-view plan, renderer, detector and projection by hand and reading the **raw per-view detections** for a single synthetic subject that straddles a boundary:

| report | yaw | pitch | yawRadius | pitchRadius | view |
|---|---|---|---|---|---|
| complete | 6.20° | 0.00° | 11.68° | 11.86° | yaw 0° |
| clipped sliver | 15.82° | 0.26° | **1.35°** | 6.95° | yaw 60° |

Duplicate separation **9.61°** against a person-tuned merge distance of **8.0°** — exactly the Objective 23 measurement, now explained: the sliver's yaw half-extent has collapsed (1.35° against 11.68°), because the clipping view sees only a thin edge of the subject, and its truncated centroid is displaced toward that view's axis. **The pair is one subject, but the displacement is a property of where the view boundary cut the box, not of where the subject is.**

The decisive observation is that `detectionToDirection` **already reports each detection's angular half-extents** (`yawRadiusDeg` / `pitchRadiusDeg`). A truthful detector states how big the thing looks; a clipped report states a small footprint, and a small footprint far away is what a *different* object looks like. The information needed to tell "one subject, seen twice" from "two subjects" was therefore already on the observation; only the merge test was ignoring it.

## Decision

- **The duplicate-consolidation test becomes: same target if the centroids are within `mergeDistanceDeg` *or* if their yaw footprints reach each other** — that is, if `separation <= a.yawRadiusDeg + b.yawRadiusDeg`. It is implemented as one named predicate, `sameTargetAcrossCoveringViews` in `app/target/SphericalTargetTracker.cpp`, with all other merge behaviour unchanged: footprint size remains the survivor tie-breaker, and association, ordering, gating and track assignment are exactly as before.
- **The global merge distance is NOT raised.** Tuning `mergeDistanceDeg` from 8° to 24° was the Objective 23 workaround; it declares that people are never more than 24° apart, which is false, and it merges genuinely separate people everywhere on the sphere to repair a displacement that only occurs at a view boundary. The default stays **8.0°**, and the suite workarounds that had set 24° were **removed**.
- **The extension cannot fire on an arbitrary threshold.** It is conditioned on a *reported size*: at least one observation must be large enough that its own angular extent reaches the other's centre. Two observations of zero radius (injected doubles, unit-style observations) satisfy only the original distance test, so no existing synthetic behaviour changes by accident.
- **Scope is deliberately narrow.** Only the **yaw** footprint is used, because yaw is the axis along which covering views are laid out and along which the measured clipping occurs (pitch rows are 62.5° apart and no pitch-axis duplicate was demonstrated). The smallest rule the evidence supports was chosen over the more general "combined 2D footprint" rule, because a wider rule merges more and the evidence does not require it.
- **Nothing else in perception is touched.** No change to `EquirectViewPlan` or its coverage, to `EquirectProjection` or its geometry, to the detector or its boxes, to view counts or fields of view, to the association gate, to identity selection or reference semantics, to sampling density, to planning or smoothing, to the renderer, to FFmpeg or decoding, or to any persisted schema. No new dependency and no model.
- **The old workaround is deleted, not preserved.** Removing the `mergeDistanceDeg = 24.0` overrides means the Objective 23 follow tests, the Objective 24 density test, the Objective 25 decoder-reuse test and the Objective 26 smoothing tests now run at the **shipped default**, so the suite no longer depends on a threshold the product does not use.

## Rationale

The two candidate fixes were "widen the threshold" and "use the footprint the observation already carries". The first is wrong on its own terms: the required distance is unbounded — it grows with the subject's apparent size and shrinks with the subject's distance from the view boundary — so any constant is either insufficient for a large nearby subject or destructive for small distant ones. The second is the property that actually distinguishes the cases: *a clipped report is small*, and two independent objects whose reported extents overlap are not separable by centroid distance in the first place.

It was also verified that the fix is not a disguised threshold increase: a contrast block in the new test raises `mergeDistanceDeg` to 24° and shows it merging two **separate** subjects that the default now keeps apart — the behaviour that motivated rejecting the threshold route.

## Consequences

- **Genuine multi-person ambiguity is preserved, and is asserted.** Two 3°-radius subjects at yaw −6° and +6° (12° apart, distinct colours, both labelled a person) still resolve as **2 identities** at the default: the reported extents (3° + 3° = 6°) do not reach across the 12° separation. Two subjects 80° apart still resolve as **2**. A single subject still resolves as **1**.
- **One subject seen by two covering views is now one identity**: the reproduced case yields **2 raw per-view detections** and exactly **1 identity** at the shipped 8° default, at yaw ≈ 6.20° with a yaw footprint above 10° — the complete detection, not the sliver.
- Behaviour is deterministic and repeatable: the merge is re-run on a freshly constructed resolver inside the same test and produces identical identities.
- **Contract change, recorded:** `SphericalTargetTracker::mergeNearDuplicates` no longer decides duplicates by distance alone; its decision is now *distance or overlapping yaw footprints*. Anything asserting the old distance-only semantics must be read against this decision.
- Four suite-level workarounds were deleted rather than left in place, so the default path — not a test-only threshold — is what the follow, density, decoder-reuse, smoothing and real-media pipeline tests now exercise.
- The Objective 23 duplicate-identity finding is **closed**, and the notes in Decisions 043, 044, 045 and 046 that leave it open are superseded by this decision; their historical text is preserved unchanged.

## Verification

- 3 new tests: the exact failure mode reproduced from raw per-view detections (2 raw reports, one complete at an 11.68° yaw half-extent and one sliver at 1.35°, separation 9.61° > 8.0°; then 1 identity after consolidation, with a determinism repeat); **non-over-merging** (nearby distinctly-coloured subjects stay separate, the 24° threshold contrast merges them, a single subject stays single, distant subjects stay separate); and follow resolution at the **default** merge distance on a moving subject.
- Targeted regression over the perception, tracking, planning, camera-path, command-runner, resolution, streaming-provider and real-media pipeline tests: **73 passed / 0 failed / 0 skipped** (240 s).
- The real-media follow pipeline and the decoder-reuse test both pass **with the 24° overrides removed**, which is the evidence that the fix, not a test setting, is doing the work.


---

# Decision 048 — The Rendered 360 Output Preserves the Source Audio of the Plan's Retained Spans

**Status:** Accepted (2026-09-18, 360 Reframing Objective 28)

## Context

The priority pipeline is *360 source -> understanding -> request -> structured edit/reframe plan -> virtual-camera decisions -> deterministic execution -> flat video output*. Every stage of it was implemented and tested except the last one's completeness: **the artifact the engine produced was silent**. `ReframeRenderer::encodeVideo` accepted only a rendered-PNG pattern and wrote H.264 video with no audio input, no audio map and no audio codec at all, `ReframePipeline::renderPlan` never handed the source path to the encoder, and the decode seams explicitly discard audio (`-an`). Nothing in the suite observed the absence, because every generated fixture is video-only.

Objective 14 changed what silence costs. `ReframePlan` gained ordered retained `segments`, and `frameTimeMs()` walks them to build the **output** timeline, so a retimed render no longer shares the source's timeline. Any audio an output carries therefore has to be mapped through the same spans the picture was built from — which makes this deterministic-execution work driven by the plan, not a container-level toggle.

## Decision

- **The rendered output carries the source audio of the plan's retained source spans**: `ReframePlan::segments()` in order when the plan has them, otherwise `sourceRange()`. Each span is trimmed out of the source, its timestamps are reset, and the spans are concatenated in order, so the audio begins at output time zero and corresponds exactly to the retained picture.
- **Audio is an execution policy, not a plan field.** The plan continues to describe only *what* (source spans, camera keyframes, output specification); carrying the audio of those spans is a documented default behaviour of the deterministic engine, exactly as the pixel format and codec are. No field is added to `ReframePlan`, `CameraKeyframe`, `EditDecision` or `Project`, no schema version changes, and every stored decision's payload and digest are untouched. Because audio is a pure function of `(source, plan)`, replay reproduces it without consulting anything new.
- **Two passes, and the picture keeps its existing encoder.** Pass 1 renders the PNG sequence into a temporary file through the **unchanged** `encodeVideo`; pass 2 remuxes that file with `-c:v copy` and adds the audio stream. The video elementary stream of an output that carries audio is therefore byte-identical to the video-only output of the same frames — the change cannot alter a pixel, and the Objective 16/20 equivalence guarantees apply to the picture unchanged.
- **The audio is always re-encoded, never stream-copied.** Stream copy cannot be trimmed to an arbitrary source span (it is keyframe-bound), and it would make the output depend on the source's codec rather than on this command. Fixed parameters (AAC, 192 kbit/s) make the result a deterministic function of the input.
- **The concatenated audio is bounded to the rendered picture.** `framesForRange` rounds a span's frame count **down**, so a retained span can be longer than the frames it produced; the output duration is `frameCount / fps` and the audio is trimmed to it. Without that bound a rounded span leaves an audio-only tail and extends the container past the last frame (measured: a 2333 ms span at 2 fps renders 2000 ms of picture and produced a 3000 ms container when left unbounded).
- **Source audio format is preserved from a reported fact, never assumed.** Sample rate is passed through, and the channel count is forced only for the unambiguous mono/stereo layouts; anything else is left to FFmpeg so an uncommon layout is preserved by the muxer rather than approximated. **Spatial audio is carried as-is and is not rotated** — a virtual-camera rotation does not rotate the recorded sound field. Recorded as a limitation, not a silent approximation.
- **Whether the source has audio is a FACT from the probe seam** (`MediaDurationProbe::streamSummary`, the same seam the media-analysis technical layer uses), never a guess:
  - no audio track -> the previous silent output, **no error, nothing reported**;
  - the probe cannot answer -> the previous silent output plus an **explicit recorded reason** in the result notes;
  - audio exists but cannot be mapped or encoded -> an **honest failure**, never a silent file.
- **The audio map is required, not optional.** The graph maps `[aout]` rather than `1:a?`, so a source whose audio cannot be produced fails loudly instead of quietly degenerating into a silent success.
- **The original media is only ever read.** No source file is opened for writing anywhere in this path, verified by the project's own fingerprint vocabulary and by a content digest before and after.

## Rationale

The alternative — an audio field on the plan — was rejected for the reason the plan's design already implies: the plan describes a *decision*, and "keep the audio of the footage you kept" contradicts no decision anyone can currently express. No command asks for a silent output, so a field would exist to encode a default, while changing the payload and therefore the digest of every already-persisted decision and requiring a migration. The smallest correct change was to let the plan keep deciding the spans and let execution carry the audio those spans contain.

Making the picture pass through the existing encoder first was chosen over a single-pass two-input encode because it converts a behavioural question into a structural one: the video stream is not merely *equal* to the previous one, it *is* the previous encoder's output remuxed. That is why the video-only container remains byte-identical and why the existing equivalence tests needed no relaxation.

## Consequences

- A render of a source that has audio is now a complete deliverable: picture and sound, correctly retimed when the plan retains disjoint spans.
- **No existing guarantee is relaxed.** A video-only source renders a byte-identical container to the standalone video-only encoder (asserted directly, not inferred), and the Objective 20 streaming-vs-seek container-equality test still passes untouched.
- **Reproducibility is extended, not weakened.** Where container equality is meaningful it is still asserted; where it is not (a container that now carries a stream is a different container) the guarantees asserted are equal **decoded video** and equal **decoded PCM audio**, for repeated renders and for replay.
- Replay of a stored `EditDecision` reproduces the audio as well as the picture, with the same perception-free path (no parser, no detector, no analysis).
- **Recorded limitation:** Reelcraft still cannot *play* audio. In-app source playback and rendered-result playback remain video-only (Decision 036), and the rendered file is where the audio lives. Adding an audio output subsystem remains its own objective with its own dependency decision, and is deliberately not a side effect of this one.
- The limitation is also real for real footage: the project's real 360 clip is mono, so channel-layout preservation beyond mono/stereo is exercised only by generated fixtures in this environment.
- Not in this objective: audio editing (mute, volume, fades, mixing, music), any parser keyword for them, transcription/diarization/audio analysis, spatial or ambisonic rendering, and any change to `ReframePlan`, `CameraKeyframe`, `EditDecision`, the `Project` schema, `ReframeIntent`, the parser, the contract checker, perception, target tracking, camera-path generation, analysis, reasoning or the UI. Decisions 017-047 are preserved.

## Verification

- 7 new model-free tests: audio preserved on a continuous range (stream present, channel count and sample rate preserved, container duration matches the picture, tone/silence content verified where the fixture puts it, the **video elementary stream byte-identical** to the same plan rendered without audio while the container differs, repeated renders equal in decoded video and decoded PCM, source bytes and modification time unchanged); audio follows disjoint retained segments in order (both retained spans sound, the dropped silent middle does not, container is the picture's length); a late-starting range keeps only its own audio and a range whose frame count rounds down leaves no audio tail; a video-only source yields **no** audio stream and a container **byte-identical** to the standalone video-only encoder, with no note recorded; an unusable probe degrades to the silent output with the reason recorded; a stereo source round-trips with its layout preserved and its fingerprint and content digest unchanged; and an application-level command followed by a replay reproduces both the decoded picture and the decoded audio.
- Targeted regression over the renderer, pipeline, replay, command-runner and real-media follow tests: **20 passed / 0 failed / 0 skipped** (14 s), including `reframeRenderEquivalenceStreamingVersusSeek` unchanged.
- Full model-free suite run at the checkpoint.

---

*Decisions 001-047 are preserved verbatim; this decision adds to them and supersedes none of them.*


---

# Decision 049 — Natural-Language Framing Is a Lens on the Existing Plan, Not a New Instruction Type

**Status:** Accepted (2026-09-18, 360 Reframing Objective 29)

## Context

The 360 pipeline could aim the virtual camera (`pan right`, `look at the car`), follow a resolved subject over time (Objectives 23-27), cut and re-time the source (Objective 14), and — since Objective 28 — deliver the retained audio with the picture. It could not change the **lens**. `ReframePlanBuilder` wrote `frame.fieldOfViewDeg = 90.0` for every keyframe it produced, and the other two planners carried their own constants (`TargetTrackPlanner::Config::fieldOfViewDeg = 90`, `SpeakerReframePlanner::Config::fieldOfViewDeg = 75`). No instruction, in any phrasing, could reach any of them — "zoom in on me" was not even *recognized*: the clause classifier looked for camera verbs, and "zoom" was not one, so the instruction was dropped with "Instruction was not recognized by the current grammar."

This is a framing capability, not a direction capability. For automated reframing from natural language, tightness is half of what a creator asks for: a wide establishing view and a close-up on the speaker are the same camera *position* and a different *lens*.

## Decision

- **Framing is a property of a camera move, and the move already exists.** `ReframeCameraMove` gains `hasFieldOfView` / `fieldOfViewDeg` (in-memory only, exactly like `followSubject`), the deterministic parser sets them from a framing clause, and `ReframePlanBuilder` puts the value on the keyframe it was already producing. The executable representation does not change at all: `CameraKeyframe::fieldOfViewDeg` already existed, was already serialized, was already interpolated by `CameraPath`, was already rendered by `EquirectView`, and was already validated to `[20, 140]`.
- **No new plan type, no schema change, no new field anywhere persisted.** `ReframeIntent` is never persisted; `ReframePlan`, `CameraKeyframe`, `EditDecision` and the `Project` schema are untouched, so existing decisions keep their payloads and digests, and a framing instruction is replayable through the existing perception-free path.
- **A framing clause is a lens request, not a direction.** The vocabulary is a documented, deterministic ladder of absolute vertical fields of view — `zoom in` 60, `close-up` 60, `extreme close-up`/`zoom in a lot` 40, `wide`/`zoom out`/`pull back` 120, `very wide`/`zoom all the way out` 140 — plus explicit numbers (`field of view 60`, `60 degree field of view`, `fov=75`), with `slightly` / `a bit` / `a little` halving the step toward 90. Matching is whole-word, so `wide` never matches `widescreen` (an output aspect) and `closer` never matches a word it merely appears inside. Named levels are checked most specific first. There is no multiplier form ("2x") and no continuous dial from language; those are recorded limitations, not silent approximations.
- **The consumed framing phrase is removed before direction detection.** `pull back` and `back up` contain direction words ("back"), and reading them as a 180° turn while also widening the lens would be two conflicting instructions from one phrase. The phrase is consumed by the framing parser and removed from the text the direction parser sees — the same technique Objective 15 already uses for temporal ranges.
- **A clause that carries framing AND a direction or subject is ONE move carrying both** — "pan right and zoom in" is a pan at 60°, and "zoom in on the presenter" is an aim at the presenter at 60°. Because plain "and" (and a bare comma) do not separate clauses, the subject capture could otherwise swallow the framing half ("follow me and zoom in" captured `me and zoom in`); a trailing framing clause is therefore stripped from a subject reference exactly as a trailing temporal word already was.
- **A framing-only clause is a real move.** It changes the lens without moving the camera, holding the direction the camera already has (centered forward when the instruction opens with a lens change). That is what makes `start wide, then push in on me` a genuine two-keyframe move — wide forward at t=0, tight on the subject at the end — using the interpolation `CameraPath` already performs.
- **The lens persists until it is changed again**, in the builder, exactly as a zoom ring does. An instruction with no framing clause renders at the same default lens it always did, so nothing that does not mention framing changes behaviour.
- **Every execution path is wired explicitly rather than assumed.** The ordinary builder carries the lens per move; the follow path passes it into `TargetTrackPlanner::Config` (its keyframes are its own); the speaker path passes a single requested lens into `SpeakerReframePlanner::Config`. A **lens change** cannot be expressed by the speaker planner, and rendering one fixed lens instead would drop half the request, so it is refused honestly through the existing preparation error path.
- **IPC-4 guards the result.** The contract checker (Decision 035) gains a fourth rule: when the instruction requested one or more fields of view, the executable plan must actually **reach** each of them. It is deliberately not "every keyframe carries a requested value", because a lens change legitimately starts from the lens the camera already had. NotApplicable when no framing was requested.
- **An unsatisfiable lens is reported, never clamped.** `field of view 200` is recorded in the intent's notes as outside the supported range and passed through, so the existing plan validator refuses the plan and the command fails with both messages.

## Consequences

- Reframing instructions can now set the framing: `zoom in`, `go wide`, `use a close-up`, `zoom in on the presenter`, `follow me and zoom in`, `keep me centered and slightly closer`, `start wide, then push in on me`, `field of view 45`, `keep 0:00 to 0:30 and zoom in`.
- The framing that was applied is reported deterministically (`Framing: field of view 60 degrees.`) and travels with the command result into the application outcome, so a creator sees what was applied instead of inferring it.
- **Nothing that does not mention framing changes.** The default lens stays 90° in the builder and the target planner and 75° in the speaker planner; directions, aim/follow classification, temporal editing, compound composition, source-audio preservation, replay and every persisted artifact are untouched.
- A requested lens reaches the **pixels**: the same camera direction at a tighter field of view renders a different picture, and a stored decision carrying the lens replays identically — both asserted.
- Recorded limitations: the vocabulary is a fixed ladder plus explicit numbers (no "2x", no arrow-key ramp); framing offsets (rule-of-thirds, lead room) remain deliberately absent (Decision 046); a lens change is not expressible on the speaker path and is refused rather than approximated.
- Not in this objective: framing offsets, per-subject framing, multi-subject framing, zoom by multiplier, any UI control, any perception, analysis, reasoning or persisted-schema change. Decisions 017-048 are preserved.

## Verification

- 6 new model-free tests: the parser (`reframeIntentParsesFraming` — the ladder, explicit numbers, the softener, word boundaries, the "pull back" direction trap, subject capture with `and` and with a comma, compound temporal+framing, an unsatisfiable value reported, and a two-state lens change); the builder (`reframeBuilderAppliesRequestedFraming` — framing-only, direction+framing, the wide-to-tight path with interpolation checked mid-way, lens persistence, the unchanged default, and the honest refusal of an unsatisfiable lens); the contract (`reframeContractFieldOfViewFidelity` — NotApplicable, consistent, IPC-4 on a dropped lens, and the containment rule for a lens change); the follow path (`reframeCommandRunnerFollowsAtRequestedFraming` — the same trajectory at the requested lens, timestamps and yaw identical, plus a detector-free direction+framing command); the speaker path (`reframeCommandRunnerSpeakerFramingIsHonest` — a single lens honoured, a lens change refused); and end to end (`reframePipelineRendersRequestedFraming` — two FFmpeg renders whose pictures must differ, the note reported, and an Application command whose stored decision carries the lens and whose replay reproduces the frames byte-for-byte at decode level).
- Targeted regression over the parser, builder, contract, command runner, camera path, follow/sampling/smoothing, planners, pipeline and render-equivalence, replay and application-command tests: **73 passed / 0 failed / 0 skipped** (29.6 s).
- Full model-free suite run at the checkpoint.

---

*Decisions 001-048 are preserved verbatim; this decision adds to them and supersedes none of them.*


---

# Decision 050 — Multi-Subject Framing Is One Enclosing Framing Decision on the Existing Plan

**Status:** Accepted (2026-09-18, 360 Reframing Objective 30)

## Context

Every reframing decision the engine could express until this objective aimed the camera at ONE subject: `ReframePlanBuilder` resolved one direction per camera move, and the follow path built a trajectory through one track. "keep both of us in frame" had no representation at all — the parser found no direction and no subject, and the instruction was dropped unrecognized.

Three questions had to be answered before implementing it, and inspection answered all three:

- **Can the executable representation express it?** Yes, unchanged. A camera keyframe already carries yaw, pitch, roll and a vertical field of view, and `CameraPath` already interpolates all of them. A framing that contains two subjects is a keyframe aimed between them at a lens wide enough for both; a pair that moves is a sequence of such keyframes. **No plan type, no keyframe field and no schema change is required.**
- **Can perception supply two identities at once?** Yes. `TargetResolver` already returns every track it resolved, `TargetTrack` exposes observations with their reported angular footprints and an interpolating `sampleAt`, and `TargetSelector` resolves "me", "the other person" and the canonical "person 1"/"person 2" order deterministically, reporting ambiguity rather than choosing. No new detector, provider or model is needed.
- **What is the smallest defensible framing rule?** The one that is the exact inverse of the renderer's own camera basis.

## Decision

- **A plural request is a GROUP, resolved at command time.** `ReframeCameraMove` gains `subjectGroup` (`None` / `CreatorAndOther` / `TwoPeople`), an in-memory field exactly like `followSubject` and `hasFieldOfView`. The parser records WHICH group was asked for and never which tracks, so nothing is fabricated at parse time, nothing is persisted, and the same instruction resolves against whatever the current tracks and identity state are.
  - "both of us", "us both", "the two of us" → the creator and the one other visible person, through the existing identity rules (an unselected creator, or more than one other visible person, is refused honestly).
  - "both people", "both of them", "the two people" → EXACTLY two visible people in the selector's canonical order; more or fewer is ambiguous and is reported with its candidates rather than silently choosing two.
  - A framing verb (or an explicit "in frame"/"in shot"/"in view") must be present, so a passing mention of two people is not turned into a framing instruction.
- **The framing rule is the exact inversion of the renderer's camera basis.** A ray at yaw offset t and pitch p is inside a vertical-FOV v view when `|sin t · cos p| ≤ tan(v/2) · aspect` and `|sin p| ≤ tan(v/2)`. Since `tan x ≥ sin x`, requiring `tan(span/2) ≤ tan(v/2)` for both axes is **conservative** — it can never under-frame — and every subject's own reported footprint contributes to the spans. The aim is the midpoint of the two footprints (the only preference-free choice: no subject is privileged, as instructed), yaw is unwrapped first so a pair straddling ±180 is framed the short way round, and the lens is the tightest one that contains them for the WHOLE instruction. The renderer's own limits (20°-140°) bound the result; a tighter requirement is raised to the renderable minimum, which still contains both, so it is not a clamp that could hide one.
- **It is ONE framing decision, executed as a camera path.** Framing is computed only at the timestamps where EVERY requested subject was actually observed — nothing is interpolated or invented — and between them the existing `CameraPath` interpolation holds the framing. Keyframe times come from those joint observations, which for a command path are the resolver's own follow-resolution samples (Objective 24), so the pair path is as dense as a follow path and costs the same to resolve.
- **Objective 29 composes.** A named lens is honoured exactly when it can contain both subjects ("keep both of us in frame, wide" frames at 120°); a lens too narrow to contain them is **refused with the measured requirement**, never widened behind the creator's back. Because one framing decision has one lens, a request that names two lenses is refused as a combination of camera instructions rather than rendered at one of them.
- **Nothing is dropped, clamped or substituted.** A subject with no usable observation in the range, subjects never observed together, a group that cannot fit inside the renderable field of view, an unresolved or ambiguous reference, or a plural request mixed with a direction or another camera instruction each fail the command with a specific reason.

## Why the contract checker does NOT verify this (recorded explicitly)

`ReframeContract` checks the final `(ReframeIntent, ReframePlan)` pair, and Decision 035 already established that **the plan retains camera coordinates only** — it carries no subject identity and no subject geometry. A rule asserting "both subjects are inside the frame" is therefore **not decidable** from those two values, and a rule that merely asserted "two ids were resolved" would prove nothing about the framing. Rather than add a cosmetic IPC-5, the guarantee is enforced where the identities and footprints actually exist — in `ReframeCommandRunner::prepare()` before any plan is built — and is asserted by tests that check the *exact* containment condition at every keyframe with the subjects' real positions. The existing contract rules (IPC-1..IPC-4) continue to hold and to be checked on the resulting plan, including the requested lens.

## Consequences

- "keep both of us in frame", "keep us both in frame", "follow both of us", "keep both people in frame", "keep both of them in frame" now produce a real framing; "…, wide" and "…, close-up" compose with the Objective 29 lens vocabulary, and compound temporal commands ("keep both of us in frame and keep 0:00 to 0:30") work through the existing composition rule.
- **Single-target behaviour is untouched**: the ordinary builder, the single-track follow path (including its smoothing), the speaker path, aim instructions, temporal editing, audio preservation and replay are all unchanged, and the parser only classifies a clause as plural when a plural phrase AND a framing verb are present.
- **Replay is unchanged and perception-free**: the multi-subject framing lives entirely in the persisted `ReframePlan` (yaw/pitch/FOV keyframes), so a stored decision replays to the same frames, verified by decoding.
- **No schema, no plan type, no dependency, no model.** `ReframeIntent` is never persisted; `ReframePlan`, `CameraKeyframe`, `EditDecision` and the `Project` schema are untouched.
- Recorded limitations: only TWO subjects are supported (a group is `CreatorAndOther` or `TwoPeople`); the pair path is not smoothed (the existing symmetric smoothing is defined for one direction sequence, and smoothing a per-sample enclosure could break the containment guarantee — a follow-up needs a containment-preserving smoothing); framing offsets and composition rules remain deliberately absent (Decision 046); a pair that is only observed together once produces a single, static enclosing framing; and the joint-framing rule requires exact timestamp agreement between the two tracks, which is what one resolver pass produces.
- Not in this objective: arbitrary multi-person detection, new providers, face recognition, identity guessing, cinematic composition, offsets or lead room, automatic subject preference, more than two subjects, creator UI, Analysis→Reasoning, speaker/dialogue automation, in-app audio playback. Decisions 001-049 are preserved.

## Verification

- 6 new model-free tests: the parser (both groups, all phrasings, single-subject non-regression, a passing mention not becoming an instruction, a plural clause that also carries a direction keeping both, and composition with a named lens); the pure framing mathematics (midpoint aim, footprint- and aspect-driven lens, containment verified with the exact camera-basis condition, a pitch-dominated pair, a vertical output, ±180 wraparound, an impossible pair refused with the measured requirement, and fewer than two observations rejected); the runner with two injected tracks (two distinct resolved targets, one keyframe per joint observation, one lens, containment at every keyframe, determinism, a requested wide lens honoured, and single-target follow unchanged); the real detector/tracker path (two detected people framed together, and the flagship "keep both of us in frame" resolved through a creator selection plus "the other person"); every honest refusal (cannot fit, requested lens too narrow, combination with another camera instruction, three people for "both people", never observed together, one subject unusable in range, unselected creator, plural plus direction); and a moving pair through the real pipeline (rendered twice with equal frames, the whole plan persisted in an `EditDecision` and replayed to identical frames, with the source untouched).
- Targeted regression over the parser, builder, contract, command runner, planners, camera path, target selector/resolver/tracker, pipeline, render equivalence, replay, application commands and the Objective 28/29 behaviours: **102 passed / 0 failed / 0 skipped** (39.7 s).
- Official `scripts/build_and_test.sh` run at the checkpoint.

---

*Decisions 001-049 are preserved verbatim; this decision adds to them and supersedes none of them.*


---

# Decision 051 — Development Management: One Home Per Fact, a Capability Register, and Controlled Batches

**Status:** Accepted (2026-09-18, Process Objective P1)

## Context

Thirty objectives produced 478 passing tests, clean checkpoints and additive history, but the
management layer around that work had grown faster than the product. Measured at the Objective 30
checkpoint: **11,518 lines / ~900 KB of documentation**; each objective wrote **~200 new doc lines
into 5-8 documents**, so every milestone was narrated four to eight times in different words;
`CURRENT_STATE.md` carried **74 objective narrative sections** that duplicate the 74 dated
`DEVELOPMENT_LOG.md` entries one-for-one; `NEXT_TASK.md` mixed a stale candidate prose list with
per-objective history; the objective prompts restated rules that `AGENT_WORKFLOW.md` already
defines as canonical; validation debt (Objectives 28-30 are fixture-only) was invisible without
reading thousands of lines; and a documented configuration value had already drifted from its script
(`timeout 240` in `DEVELOPMENT_ENVIRONMENT.md` vs 900 in `scripts/build_and_test.sh`).

## Decision

- **One home per fact.** `DEVELOPMENT_LOG.md` is the single detailed chronological record (one entry
  per objective, append-only). `DECISIONS.md` carries decisions only. `CURRENT_STATE.md` describes
  the system as it is now. `NEXT_TASK.md` is the capability register. `ARCHITECTURE.md` carries the
  architecture plus indexes. `CHANGELOG.md` is user-facing only. `PROJECT_HISTORY.md` is
  milestone-level narrative. `AGENT_WORKFLOW.md` is the canonical policy. `KNOWN_ISSUES.md` carries
  open limitations with reasons. An objective must not be narrated in several of them.
- **`NEXT_TASK.md` becomes the operational register**: one row per capability with status
  (done/partial/missing/blocked/deferred), implementation seam, dependency, **validation level**,
  verification commit, plus an explicit **validation debt** section, a candidates table with stated
  prerequisites, and the human-approved priority. It is a register, not a history.
- **Living indexes, kept cheap.** `ARCHITECTURE.md` §23 maps stable seams to implementation,
  guarantee and consumers; §24 maps behavioural guarantees to the tests that verify them, so a
  regression set is looked up rather than re-derived. Both are indexes, not explanations.
- **Controlled batches with a mandatory gate** (`AGENT_WORKFLOW.md` §12). A human approves a
  capability family and 2-3 dependency-adjacent objectives; each objective still gets its own
  implementation, tests, documentation and commit; between objectives the agent re-verifies the next
  objective's prerequisites and stops if anything in the stop list has changed (architecture,
  semantics, schema, dependency/licence, source-media risk, unresolved failure, scope growth, or
  anything that invalidates the batch). Autonomy grows for routine engineering; selecting a new
  product direction remains a human act.
- **Lightweight checkpoint hygiene** (`AGENT_WORKFLOW.md` §13, `scripts/checkpoint_check.sh`):
  build-tree freshness, present-tense script/document value drift, and documented-vs-actual test
  totals. No documentation-validation framework.

## Consequences

- No product behaviour, persisted schema, plan/keyframe semantics, rendering, perception, decision or
  replay semantics change; Decisions 001-050 are untouched and remain the architectural record.
- The engineering loop itself is unchanged: smallest coherent change, incremental compile, focused
  tests, targeted regression, one official full-suite run, one commit, clean tree.
- Per-objective documentation shrinks sharply and the reading side of an objective becomes the
  register plus the indexes plus the files to be touched, instead of thousands of narrative lines.
- New duties, deliberately small: keep the register and the two indexes accurate at each checkpoint,
  and run the checkpoint script before declaring completion. A stale index is a defect of the same
  kind as a stale test.
- Deferred detail remains retrievable: git history holds every previous document version, and
  `DEVELOPMENT_LOG.md` holds the per-objective record.

## Verification

- Documentation-only diff; the test suite is unchanged (478 passed / 0 failed / 9 skipped) and
  `scripts/checkpoint_check.sh` passes. The trimmed `CURRENT_STATE.md` narrative was verified to be
  preserved: its 74 objective headings correspond one-to-one with the 74 dated `DEVELOPMENT_LOG.md`
  entries, and the file states where the detail lives.

---

*Decisions 001-050 are preserved verbatim; this decision adds to them and supersedes none of them.*


---

# Decision 052 — N-Way Group Framing, and the Enclosure Rule Computed Exactly in the Renderer's Basis

**Status:** Accepted (2026-09-18, 360 Reframing Objective 31)

## Context

Objective 30 framed **exactly two** subjects. Inspection before this objective confirmed that the
*engine* was already general — `enclosingFramingDeg` and `planTracks` accept any number of
observations/tracks — while the *decision* layer was not: the group enum knew two groups, the
vocabulary knew "both", and the runner hard-coded a pair.

Objective 31 was asked to extend that to N **and** to test the *real* spherical camera basis against
every target, without arbitrary thresholds. That second requirement exposed a defect in what
Objective 30 had shipped:

- **The enclosure rule was an approximation.** It derived the required vertical field of view from the
  group's yaw span and pitch span (`2·atan(tan(yawSpan/2)/aspect)` and `pitchSpan`). The renderer
  builds a pixel's ray as `forward + right·(ndcX·tanHalf·aspect) + up·(ndcY·tanHalf)`, so
  containment is the **tangent** condition `|lateral/forward| ≤ tanHalf·aspect` and
  `|vertical/forward| ≤ tanHalf`. Span-derived requirements coincide with it only when a footprint
  occupies one axis at a time: the property sweep found a six-subject, high-pitch case
  (±13° yaw, ±11° pitch) that escaped a 22° lens because a diagonally opposite corner's true vertical
  component exceeded `sin(Δpitch)`.
- **The test helper was too permissive.** It compared direction *cosines*
  (`|lateral| ≤ tanHalf·aspect`) where the condition requires tangents, so Objective 30's
  containment evidence was weaker than its decision text claimed. The sweep caught both.

## Decision

- **Group framing is N-way, and the size travels with the group.** `ReframeSubjectGroup` becomes
  `CreatorAndOthers` / `VisiblePeople` with `ReframeCameraMove::subjectCount` (0 = count-free).
  Semantics, all deterministic and preference-free:
  - "the three of us" / "all three of us" / "the 5 of us" → creator + exactly (n-1) other visible
    people; a different count is refused with its candidates.
  - "all of us" / "us all" → creator + every other visible person (at least one).
  - "the three people" / "three of them" / "4 people" → exactly n visible people.
  - "everyone" / "everybody" / "all of them" / "all of the people" → every resolved visible person
    (at least two; a single subject is not a group framing).
  Sizes are read as words (two..ten) or digits (2..10); a size below two is not a group request. The
  more specific phrase always wins ("everyone of us" is the creator family, not "everyone").
- **Which tracks is still resolved at command time**, through the selector's canonical order (first
  observation, then numeric id, then id) — never detector or container order — and the creator leads
  a creator-inclusive group. A group is never completed by substituting, dropping or inventing a
  subject.
- **The lens is computed exactly in the renderer's own basis.** The aim is the centre of the group's
  unwrapped yaw/pitch span (the only preference-free choice), the basis is built exactly as
  `EquirectView` builds it (including its degenerate fallback), and the required half-tangent is the
  maximum over **every footprint corner** of `|lateral/forward|/aspect` and `|vertical/forward|`.
  Containment therefore holds for the whole footprint, not just its centre, and no arbitrary
  constant, margin or merge threshold is involved.
- **Honest limits are unchanged and now exact.** A corner at or behind the view plane, or a
  requirement above the renderable maximum (140°), refuses with the measured requirement; a named
  lens narrower than the requirement is refused rather than widened; a requirement below the
  renderable minimum (20°) is raised to it, which still contains every subject.
- **Containment is asserted in the tangent form against the actual reported footprints** — including
  in the Objective 30 tests, which used assumed footprint sizes. The contract checker still does not
  (and cannot) verify containment; Decision 050 stands.

## Consequences

- Natural language now reaches group framing: "keep the three of us in frame", "keep all of us in
  frame", "keep the three people in frame", "keep everyone in frame", with sizes up to ten.
- **Objective 30's plan numbers change slightly**: the exact rule is tighter than the approximation
  where the approximation over-estimated, and it refuses where the approximation would have
  under-framed. Two Objective 30 assertions were updated for that reason (an assumed footprint size
  replaced by the detector's own reported one, and the pitch-dominated expectation), and its
  containment assertions were strengthened rather than relaxed.
- No schema, persisted artifact, dependency or plan-type change; single-subject behaviour is
  untouched; replay remains perception-free.
- Recorded limitations: the vocabulary is bounded at ten named sizes; group membership is fixed for
  the instruction (no dynamic membership); the group path is not smoothed; framing offsets remain
  absent (Decision 046). Objective 32 (explicit subject-reference sets) builds on this resolution.

## Verification

- 5 new model-free tests plus the strengthened Objective 30 tests: vocabulary and sizes
  (`reframeIntentParsesGroupFraming`), canonical N-way resolution with creator leadership and
  order-independence (`reframeGroupFramingResolvesCanonicalSets`), every honest refusal
  (`reframeGroupFramingRefusesHonestly`), a deterministic geometry sweep over 3-6 subjects, ±180
  wraparound, high pitch, non-uniform footprints and four output aspects with both outcomes exercised
  (`reframeGroupFramingGeometrySweep`), and render + replay equivalence with source immutability
  (`reframeGroupFramingRendersAndReplays`).
- Targeted regression across parser, builder, contract, runner, planner, camera path, selector,
  resolver, pipeline, audio, replay and application tests: **101 passed / 0 failed / 0 skipped**.
- Full model-free suite run at the checkpoint.

---

*Decisions 001-051 are preserved verbatim; this decision adds to them and supersedes none of them.*


---

# Decision 053 — Explicit Subject Sets Are References Resolved at Command Time, Split Only Inside a Framing Construction

**Status:** Accepted (2026-09-18, 360 Reframing Objective 32)

## Context

Objective 30 framed "both of us"; Objective 31 generalised that to group phrases with sizes ("the
three of us", "everyone"). Neither could name *which* subjects: "keep me and person 2 in frame" had no
representation, because the intent carried at most one subject reference and the singular capture took
the first reference while ignoring the rest. Identity already existed — the selector resolves the
creator alias, ordinals ("person 2", "the second person"), left/right, exact track ids and unique
labels — so nothing new was needed except a way to ask for several of them at once.

The hazard is the word "and". The parser already handles three things joined by "and": a temporal
clause ("keep 0:00 to 0:30 and follow me"), a lens clause ("follow me and zoom in"), and a group
phrase ("keep both of us in frame"). Objective 31's gate fixed the detection order — framing phrase →
group detection → direction → singular capture → filler stripping — and any new splitting had to fit
inside it without disturbing those cases.

## Decision

- **An explicit set is a third kind of group** (`ReframeSubjectGroup::ExplicitSet`) carrying
  `ReframeCameraMove::subjectReferences` in the order they were written. The parser records
  references only; **which tracks they are is decided at command time** by the existing selector, so
  no identity system is invented and nothing is persisted.
- **"and" is split only inside a construction that is already a framing instruction.** The detector
  requires a framing verb (or an explicit "in frame"), consumes the lens phrase and the trailing
  framing words first, removes the framing verbs, and only then splits on "and"/"&"/","/: each
  fragment must survive as a reference. A fragment made only of instruction or filler words ("From",
  the residue of a temporal phrase) is not a reference. Fewer than two references means the clause is
  not a set at all and falls through to the existing single-subject path — which is what keeps
  "follow me and zoom in", "keep me centered and zoom in", "keep 0:00 to 0:30 and follow me" and
  "Make a 30-second version and keep me centered." parsing exactly as before.
- **Every reference must resolve.** An unresolvable or ambiguous reference fails the command with the
  reference named; a name is never reinterpreted as a different subject, and a group is never
  completed by substitution.
- **Duplicates are resolved, not collapsed early.** References are kept as written, resolved
  individually, and de-duplicated by track id at command time; two references to one subject are ONE
  subject, and a set with fewer than two *distinct* subjects is refused. Collapsing duplicates in the
  parser was rejected explicitly: it turned "person 1 and person 1" into a single reference, the
  clause stopped being a set, and the command silently fell back to a centered camera — a silent
  reinterpretation of a multi-subject request.
- **Canonical order decides the set**, with the creator leading when it is part of it (the same rule
  as "…of us"). Neither the written order nor the detector/container order may change the resolved
  set or the plan.
- **The framing path is reused, not reimplemented.** An explicit set populates the same resolved
  track set that group phrases produce, so `TargetTrackPlanner::planTracks` and the exact
  tangent-containment enclosure of Decision 052 apply unchanged. The enclosure mathematics is not
  touched by this objective.
- **Group phrases keep precedence in the same clause**, and a direction mixed with an explicit set is
  refused by the existing rule.

## Consequences

- "keep me and person 2 in frame", "keep person 1 and person 3 in frame", "keep me and person 2 and
  person 4 in frame" and "frame the presenter and the guest" (unique labels) now work, with a named
  lens ("…, close-up", "…, wide") composing through Objective 29.
- Refusals are specific: an unresolvable name, an ambiguous name, an unselected creator, a set of one
  distinct subject, a set that cannot fit inside 140°, or a named lens narrower than the requirement.
- Recorded limits: the reference vocabulary is the selector's existing one (creator aliases, ordinals,
  left/right, track ids, unique labels) — a name that matches several tracks at once is ambiguous and
  refused; a reference fragment longer than a phrase (for example a whole clause) is not treated as a
  set and falls back to the single-subject path; group sizes above ten from Objective 31 are
  unchanged; membership is still fixed for the instruction.
- No schema, persisted artifact, dependency, plan type or rendering change; single-subject and group
  behaviour are unchanged; replay remains perception-free.

## Verification

- 4 new model-free tests: `reframeIntentParsesExplicitSubjectSets` (the four reference forms, three
  or more references, a lens clause inside a set, and the guards that keep "follow me and zoom in",
  "keep me centered and zoom in", the temporal-residue clause, the target-duration clause, "pan right
  and zoom in" and the Objective 31 group vocabulary unchanged);
  `reframeCommandRunnerResolvesExplicitSubjects` (creator + numbered subject, three explicit
  references, order independence in text and in the input container, deterministic repeats, unique
  labels resolving through the existing label identity, and a pair straddling ±180 with containment
  asserted for every subject's real footprint at every keyframe);
  `reframeCommandRunnerExplicitSubjectsRefuseHonestly` (unresolvable reference, duplicate-only set,
  duplicate plus a genuine second subject, unselected creator, ambiguous name, a set beyond the
  renderable maximum, a named lens too narrow and the same geometry accepted when wide, plus group and
  mixed-direction non-regression); `reframeExplicitSubjectsRenderAndReplay` (deterministic execution,
  decision round-trip, perception-free replay to identical frames, source untouched).
- Targeted regression across parser, builder, contract, runner, planner, camera path, selector,
  resolver, pipeline, audio, replay and application tests: **105 passed / 0 failed / 0 skipped**.
- Full model-free suite run at the checkpoint.

---

*Decisions 001-052 are preserved verbatim; this decision adds to them and supersedes none of them.*

---

# Decision 054 — Creator Review Is a Read-Only View of the Canonical Plan, and Accept Executes Exactly That Plan

**Status:** Accepted (2026-09-18, 360 Reframing Objective 34)

## Context

The documented creator workflow places a review step between the system's editing decision and its
execution ("the creator remains in control and can review, modify, or reject any AI decision"). What
existed instead was all-or-nothing: an instruction was parsed, resolved, planned and **rendered** in
one call (`Application::runReframeCommand` → `ReframeCommandRunner::run`), so the creator's first sight of
the edit was the finished file. The structured artifacts that make inspection possible were already
there and already authoritative — the validated `ReframePlan`, and once a render is recorded, the
immutable `EditDecision` that carries it — but nothing surfaced a plan before committing to it.

The objective was scoped to a *foundation*: inspect the plan, then accept or reject it. Explicitly out
of scope: revising the plan, editing a timeline, undo/redo, and any new plan or persistence schema.

## Decision

- **Review is a view, not a second editor.** `ReframePlanReview` (`app/application/ReframePlanReview.*`)
  is a plain value type whose every displayed fact is either the plan's own value, a pure derivation
  from it, or context the decision stage reported (the instruction, the resolved subject ids, the
  notes). It carries the canonical plan by value plus a SHA-256 digest of that plan's canonical JSON,
  which is the review's fingerprint of the exact plan it will execute. It is **never serialized, never
  persisted and never a second plan schema**.
- **Preparation is the decision stage only, through the same request the command path builds.** The
  validation and `ReframeCommandRequest` construction that `runReframeCommandInternal` had always
  performed were **extracted into one shared `Application::buildReframeCommandContext`**, and both paths
  now use it: the direct command path, the revision path and review preparation. That is what makes
  "the reviewed plan is the plan that would have run" a structural property rather than an inspection
  claim. Preparation uses the new `m_commandPreparer` seam (default
  `ReframeCommandRunner::prepare`, same signature as the executor) so no perception or decode is added
  and tests stay model-free. It renders nothing, appends nothing and writes nothing.
- **Accept renders EXACTLY the reviewed plan.** `acceptReframeReview()` calls the existing
  `ReframePipeline::renderPlan` seam — the same one `replayEditDecision` uses, through the same
  `m_replayRenderer` injection point — with the reviewed plan, its source and the reviewed
  destination. Nothing is re-parsed, re-resolved or re-planned, and the plan is copied rather than
  mutated, so accepting cannot change what the creator saw. The result is recorded through the
  **single existing append gate** (`appendReframeOutput`) as an ordinary render record carrying an
  `EditDecision::fromPlan` of that same plan, and the review's digest identifies it.
- **Reject renders nothing.** Rejection is a pure state change: no render call, no record, no file
  written, and the source media untouched.
- **Review is session state with an explicit lifetime.** It is cleared on accept, reject, new project,
  project open, active-media change and removal of the active media, and a new preparation replaces a
  previous one, so a stale review can never be accepted. Nothing about it enters the project file.
- **Honest failure.** A preparation that produced no plan appends no render record (it never reached
  an output target) and is reported with its reason and an empty output path. A failed render is
  recorded exactly as the command path records one — with its error and with the plan that was
  attempted — and the review is consumed either way, so it cannot be re-accepted against stale state.
- **Destination policy is the command path's.** Accept keeps the destination the review was prepared
  with unless the caller names another one, which is validated identically (the directory must exist,
  and the path must differ from the source). Accept does **not** adopt replay's never-overwrite rule:
  accept is the continuation of a command, not a replay of a historical record.
- **The direct command path is unchanged**, including its behaviour of recording a failed attempt that
  reached an output target. Review is an additional path, not a replacement.
- **No revision, no timeline, no undo.** There is deliberately no way to edit the reviewed plan; the
  creator accepts it or goes back and runs a different command. `reviseEditDecision` remains an API
  without a review-surface integration, and the plan has no mutator.

## Consequences

- A creator can now see what Reelcraft understood and what it intends to do — instruction and
  understanding, framing, camera movement, lens, retained time spans, output geometry and the audio
  policy — together with the plan's keyframe count and digest, **before** any render cost or output
  file exists, and then accept or reject it. The UI panel is four widgets plus one signal pair; all
  wiring stays in `main.cpp`.
- The reviewed plan's identity is checkable after the fact: the review's digest equals the digest of
  the plan handed to the renderer, which equals the plan stored in the record's `EditDecision`.
- Recorded limits: the review cannot be revised (only accepted or rejected); it is not persisted, so
  closing and reopening a project leaves nothing to accept; it shows the plan and not a preview image
  (the rendered-result preview of Objective 12 operates on completed records); and the destination is
  shown through the record rather than as an editable field.
- No perception, parser, resolver, planner, camera-path, renderer, encoder, audio, decision-artifact
  or schema change. No new dependency, provider or plan type. Replay remains perception-free, and the
  Objective 33 real-media harness is untouched.

## Verification

- 13 new model-free tests: `creatorReviewPrepareBuildsTheReviewedPlan` (every displayed fact derived
  from the plan, the digest, the summary lines, no render, no record, no file, one review signal);
  `creatorReviewDescribesTemporalEditAndAudioPolicy` (retained spans and the execution-policy audio
  line stated from the plan's own segments);
  `creatorReviewRejectRendersNothing` (no render, no record, no file, source byte-identical, and a
  rejected review cannot be accepted); `creatorReviewAcceptRendersTheExactReviewedPlan` (the plan
  handed to the renderer is byte-equal in canonical JSON and equal by digest to the reviewed plan, from
  the reviewed source, to the derived destination, recorded once with a decision built from that same
  plan); `creatorReviewAcceptValidatesDestination` (missing directory, source-as-destination, and an
  explicit alternative); `creatorReviewAcceptReportsRenderFailure` (honest failure, the attempt
  recorded with its plan, no re-accept); `creatorReviewPrepareFailureLeavesNoReview` (no record, error
  reported with an empty output path, no stale review); `creatorReviewPrepareReusesCommandValidation`
  (no project / no active media / empty instruction / missing file report the **same** reason as the
  command path); `creatorReviewInvalidatedByContextChange` (active-media switch, media removal, new
  project, project open, reject — each with exactly one clearing signal); and
  `creatorReviewIsNotASecondPlanRepresentation` (an invalid plan yields an invalid review, the digest is
  a pure function of the plan, the review holds the plan by value, and saving a project before and
  after a preparation produces an identical file with no review key);
  `creatorReviewDoesNotChangeTheDirectCommandPath`, `creatorReviewPersistsOnlyThroughTheRenderRecord`
  and `creatorReviewPanelPresentsPlanAndRequestsDecisions`.
- Targeted regression over the command path, records, lifecycle, creator selection, playback, replay,
  revision, provenance and the touched UI surfaces: **87 passed / 0 failed / 0 skipped**.
- Official `scripts/build_and_test.sh`: exit 0, **500 passed / 0 failed / 14 skipped** (487 passes plus
  the 13 new tests; the 14 skips are unchanged, because none of the new tests needs real media).

---

---

# Decision 055 — Creator Revision Semantics: Fresh Sibling Destination, Derived Supersession, Existing Lineage

**Status:** Accepted (2026-09-20, 360 Reframing Objective 35)

## Context

Objectives 16 and 17 made a render reproducible from a persisted `EditDecision` and made that artifact
immutable: a revision is expressed as a **new** decision carrying the prior one's hash as its single
parent (Decision 034). Objective 34 then added Creator Review — inspect the prepared plan, accept it or
reject it — and deliberately stopped there.

What existed after Objective 34 was an asymmetry: the revision *mechanism* was implemented and tested
(`EditDecision::revisedFrom`, `Application::reviseEditDecision`), but it was unreachable from the
product, and the decisions a creator acts on are renders they have seen. Bringing the mechanism to a
surface raises four questions that the mechanism itself does not answer, and each of them is a
semantic choice rather than an implementation detail:

- **Where does a revised render go?** The command path accepts any output path and will overwrite an
  existing file; replay refuses to overwrite.`reviseEditDecision` refuses only the *parent record's*
  own output path, so a caller can still name any other existing file as the destination.
- **What does "superseded" mean?** `AI_EDIT_CONTRACT.md` §9 lists a conceptual status vocabulary
  (Approved, Superseded, …) that has never been implemented, and decisions are immutable — a stored
  "superseded" flag would be a mutable field on an immutable artifact.
- **How is a revised render attributed?** `origin` already has exactly two values, `command` and
  `creator-revision`, and Decision 007's consequence requires the model to distinguish AI-generated
  decisions from creator modifications.

## Decision

Four semantics, and nothing else:

1. **A revision initiated through the creator revision surface writes to a deterministic fresh sibling
   destination: `<base>_reframe_rev<N>.mp4`**, where `<base>` is the source media's complete base name
   and `N` is **the smallest positive integer whose path does not already exist**. The directory is the
   directory of the record being revised (the revision is a sibling of the render it revises), falling
   back to the source media's directory when the record carries no usable output path. The name is
   derived from the media and not from the parent's file name, so a revision of a revision continues
   the same sequence (`…_rev1`, `…_rev2`, `…_rev3`) instead of nesting suffixes.
2. **A revision through this surface never overwrites its parent render, and never overwrites any other
   existing file.** The derivation in (1) establishes this by construction: a candidate path is chosen
   only when no file exists there. This is deliberately stricter than the general command policy and
   does not change it — the explicit-path form `reviseEditDecision(index, instruction, path)` keeps its
   existing behaviour, and its Objective 17 tests are unchanged.
3. **Supersession is DERIVED from lineage at read time.** No `superseded`, `approved` or status field is
   added to `EditDecision` (or anywhere else), no decision-status state machine is introduced, and no
   persisted artifact gains a mutable field: a record is superseded exactly when another record this
   application holds names its decision hash as that record's `parentDecisionHash`. The relationship
   is computed on demand, is honest about what the application currently holds, and disappears with
   the records it describes. `AI_EDIT_CONTRACT.md` §9 therefore remains conceptual.
4. **A revision is attributed as `creator-revision` and carries the existing parent decision hash** —
   unchanged Objective 17 vocabulary and persistence. The revision is a NEW immutable decision; the
   plan it carries is built by the same command pipeline as any other revision, and the parent artifact
   is only ever read.

## Explicitly not decided here

This decision changes **no** persistence semantics: no `Project` schema bump, no new persisted
artifact, no change to `decisionHash`, to version retention, to the loader's strictness, to
`origin`/`parentDecisionHash` validation, to replay (still perception-free and still re-using the same
decision), or to the single append gate. It also does not decide: retention or compaction of
accumulating decisions; persisting accept/reject status; structured, operation-level, keyframe or
timeline editing; pre-render revision of a reviewed-but-unrendered plan (which has no persisted parent
to point at and would need its own semantic decision); or any unification of output-path policy across
command, revision and replay.

## Implementation note (recorded during the same objective, before its commit)

Point (1) above says "the smallest positive integer whose path does not already exist". Implementing it
exposed that the literal rule is insufficient, and the refinement is recorded rather than quietly
applied: **a candidate is fresh when no file exists there AND no record the application holds already
claims that output path.**

The case that requires it: a record's output file can legitimately be absent — the creator deleted the
render, or (in the test suite) the render was produced by an injected executor that writes nothing. On
the literal rule, revising such a record derives `…_rev1.mp4`, which *is* that record's own output path,
so the revision path correctly refuses ("must not overwrite the record it revises") and **every later
revision of that record stalls on the same refusal**. Two focused tests failed exactly this way before
the refinement. With the second condition the sequence advances (`…_rev1`, `…_rev2`, `…_rev3`) whether
or not the earlier files still exist, and "never overwrites" remains true by construction for both files
and records.

## Consequences

- A revision is additive and non-destructive by construction: the parent render stays byte-identical on
  disk and in the project file, and the creator's revised render appears beside it under a name that
  cannot collide.
- The absence of a stored status keeps every immutable artifact immutable; the cost is that supersession
  is only visible while the records that express it are held, which is exactly the honesty the read-time
  derivation provides.
- No perception, parser, planner, geometry, renderer, encoder, audio or decision-persistence behaviour
  is affected, and the Objective 34 review path is untouched.

## Verification

- Recorded before implementation; verified by the Objective 35 tests: the derived path is a fresh
  sibling (`_rev1`, then `_rev2` when `_rev1` exists), an existing file is never overwritten (including
  the parent render and a pre-existing `_rev1`), the child record carries `origin=creator-revision`
  with the parent's hash, a two-step chain resolves through `decisionProvenance`, the parent record is
  byte-identical afterwards, supersession is derived at read time, and every refusal (unknown index,
  record without a decision, empty instruction, drifted or missing source) renders nothing and appends
  nothing.

---

---

# Decision 056 — A Revision Never Targets the Output Path of a Render Record the Application Holds

**Status:** Accepted (2026-09-20, 360 Reframing Objective 36)

## Context

Decision 055 point (2) fixed the destination semantics of the *derived* revision path and explicitly
left the explicit-path form alone: "the explicit-path form `reviseEditDecision(index, instruction,
path)` keeps its existing behaviour". Objective 36 tested what that behaviour actually permits, and
found a reachable data-loss path: `reviseEditDecision` refused only **the parent record's** output path
(`Application.cpp`, the "must not overwrite the record it revises" check), so a caller could revise
record 0 and name **record 1's** output path. The revision then rendered over record 1's file, and the
project ended up holding **two records claiming the same output path**, one of which no longer contained
what its own decision says it contains — silently destroying a render the creator could previously
replay.

This was reproduced before any change was made: a focused test rendered two distinguishable records and
then revised the first onto the second's path. The revision succeeded, the second record's file content
changed, and a third record was appended claiming that same path.

## Decision

- **A revision refuses any output path that is already claimed by a render record the application
  holds**, not only the path of the record being revised. The refusal is a distinct message naming the
  record that owns the path ("must not overwrite another recorded render: record N already writes to
  …"), so it remains distinguishable from the parent-specific refusal, which keeps its existing wording.
- **This supersedes Decision 055's statement that the explicit-path revision form's behaviour is
  unchanged — in this one respect only.** Everything else about 055 stands: the derived path, the
  never-overwrite rule for the surface, derived supersession, and `creator-revision` attribution with
  the existing parent hash are unchanged.
- **The check is on RECORDS, not on the filesystem.** A path holding an unrelated file that no record
  claims is still the caller's business (the general command policy, deliberately unchanged); a path
  that a *record* claims is never a valid revision target even if that record's file is currently
  missing, because the record is a historical fact about a render and replay must be able to reproduce
  it.
- **The general command path is deliberately unchanged.** `runReframeCommandTo` still accepts any path,
  including one a record claims. This decision tightens the *revision* surface, whose stated purpose is
  to add to the history rather than replace it; widening it to every command is a separate question that
  this decision does not answer.
- **Derived supersession becomes visible, not just computed.** The provenance readout now states whether
  a record has been revised and by which held records. This is presentation of Decision 055 point (3):
  nothing is stored, and no artifact gains a status field.

## Consequences

- A revision can no longer destroy another render, whether the caller uses the derived destination or
  names one explicitly.
- The refusal set of the explicit-path form grows by one case. Callers that name a genuinely fresh path
  are unaffected, which is verified by an existing Objective 17 test and by a new one.
- The revision surface's guarantee is now stated positively: **a revision only ever adds a record and
  writes a file no record claims.**
- No schema, artifact, hash, loader, replay, renderer or command-path behaviour changes; no new
  dependency; the Objective 34 review path and Objective 35's derived path are untouched.

## Verification

- The reproduction is kept as a regression: `applicationRevisionRefusesAnotherRecordsOutputPath`
  asserts the refusal, that it names the owning record, that the other record's file bytes are
  unchanged, and that no record was appended.
- `applicationRevisionStillAllowsAFreshExplicitPath` pins the API's shape (a fresh explicit path still
  works, and the parent-specific refusal keeps its own message).
- `mainWindowProvenanceShowsDerivedSupersession` pins the readout: a revised record names the records
  that revise it, a leaf record says it has no later revision, and a doubly-revised record lists both.

---

---

# Decision 057 — No Render May Write to a Path a Render Record Owns, and the Default Destination Is Derived Fresh

**Status:** Accepted (2026-09-20, 360 Reframing Objective 37)

## Context

Decision 056 established, for the revision surface, that a path a render record owns is never a valid
destination: a record is a historical fact about a render, and replay must reproduce it. Decision 056
explicitly declined to answer the wider question it raised — "the general command path is deliberately
unchanged … widening it to every command is a separate question that this decision does not answer".

Objective 37 answered it with evidence, found while writing the creator-workflow end-to-end test. The
test ran the documented sequence (command → review → accept → revise → replay) and asserted that record
0's rendered file still contained record 0's render. It did not: the reviewed plan had been **accepted
onto the same default destination**, `<base>_reframe.mp4`, silently overwriting the earlier render while
both records claimed that path. The same hazard applies to the plain command path, which is worse because
it is the everyday action: running the same command twice writes the second render over the first, and the
project then holds two records (and two decisions) pointing at one file whose content matches only the
newer one. Any later replay of the older decision writes a file that disagrees with what the project says
that record was.

## Decision

- **No render may write to a path that a render record the application holds already owns.** The rule is
  enforced for the direct command path and for review acceptance (the revision surface already enforces
  it since Decision 056), through the single shared definition `recordHoldingOutputPath()`.
- **An explicitly supplied destination that a record owns is REFUSED**, with a message naming the owning
  record. Refusal, not silent redirection: a caller who names a path must be told it is unavailable.
- **A destination that is not supplied is derived FRESH**: `<base>_reframe.mp4` for the first render of a
  clip — the documented default is unchanged in the ordinary case — and `<base>_reframe_2.mp4`,
  `<base>_reframe_3.mp4`, … for later renders, taking the first name that neither exists as a file nor is
  owned by a record. A second render therefore lands beside the first instead of on top of it, and a
  pre-existing unrelated file with the default name is left alone.
- **Review acceptance derives its destination at ACCEPT time.** The destination is a filesystem fact, not
  part of the reviewed plan; deriving it when the creator commits removes the whole class of "the
  destination was taken while the plan was waiting for review" and never forces a re-review (which would
  re-run perception for a filesystem accident). Accepting with an explicit path keeps the refusal rule
  above.
- **This answers the question Decision 056 left open and supersedes that one sentence of it.** Nothing
  else in Decision 056 changes, and Decisions 033 (replay) and 055 (revision destinations) are unchanged:
  replay still refuses an existing file outright, and revisions still use their own `_rev<N>` sequence.

## Consequences

- Renders accumulate instead of overwriting each other, which is what the rest of the architecture already
  assumes: records are append-only, decisions are immutable, and replay is expected to reproduce a
  historical render. Silent loss of a rendered result requires a deliberate filesystem action by the
  creator, not an ordinary second render.
- Behaviour changes in one visible way: running a command twice into the same directory now produces
  `…_reframe.mp4` and `…_reframe_2.mp4` rather than one file written twice. The first render of a clip is
  unaffected, which is why the existing suite is largely untouched by this decision.
- Callers that name an occupied destination now get a refusal where they previously got an overwrite. The
  general media-import, preview, playback and replay paths are unaffected; the original media is still
  never a valid destination.
- Cost: a render can no longer be "redone in place" through the same API call. That is deliberate — the
  project has no destructive-operation surface, and re-doing an edit produces a new record either way.
- No schema, artifact, hash, loader, renderer, encoder or replay change; no new dependency, provider or
  plan type.

## Verification

- The end-to-end creator-workflow test is the reproduction and the regression: it asserts, after the full
  sequence, that record 0's file still contains record 0's render.
- `applicationRenderDestinationsNeverOverwriteARecordedRender` pins the derived sequence
  (`clip_reframe.mp4`, `clip_reframe_2.mp4`, `clip_reframe_3.mp4`) and that an unrelated existing file
  with the default name is skipped rather than overwritten.
- `applicationCommandRefusesAnExplicitPathHeldByARecord` pins the refusal, that it names the owning
  record, that nothing is appended, and that the record's file is unchanged.
- `applicationReviewAcceptRefusesAClaimedDestination` pins the explicit-path case on the review surface.

---

---

# Decision 058 — Constrained Plan-Level Creator Adjustment: Monotone Lens Widening Only

**Status:** Accepted (2026-09-20, 360 Reframing Objective 40; human-approved scope)

## Context

Creator control enters the pipeline in exactly three places today: the instruction text (the command
path), accept/reject of a prepared plan (Objective 34), and a record-level revision expressed as a NEW
instruction (Objective 35), which re-runs perception through the shared request builder. None of them can
express *"keep this exact framing, just show me more of the scene"*: the revision path pays full
perception cost (decode + detection per sample) and its result can settle on a different aim or a
different subject set, because detection is not bit-identical between runs.

The renderer's own geometry makes exactly one class of plan change provably safe. In
`EquirectView::render`, a pixel's ray is `forward + right·lateral + up·vertical` with
`lateral = tan(FOV/2)·(ndcX·aspect·cosRoll + ndcY·sinRoll)` and
`vertical = tan(FOV/2)·(−ndcX·aspect·sinRoll + ndcY·cosRoll)`, over an **FOV-independent** orthonormal
basis. Inverting that mapping gives the containment predicate

```
contained(direction, FOV)  ⟺  |a| ≤ tan(FOV/2)·aspect  ∧  |b| ≤ tan(FOV/2)
```

(where `a`, `b` are the direction's roll-rotated lateral/vertical components over its forward
component) — the same tangent condition the planner uses to *choose* the lens
(`TargetTrackPlanner::enclosingFramingDeg`) and the same predicate the tests assert
(`subjectInsideFrame`). Because `tan` is strictly increasing on `(0°, 70°]` and the supported maximum
is 140°, **increasing the field of view with everything else held fixed only ever weakens both
inequalities**: the set of directions a frame covers is nested in `tan(FOV/2)`. Verified analytically
against the implementation and numerically over 200,000 randomized cases (aims ±180° yaw / ±85° pitch,
rolls including 90°, five aspect ratios, FOV ∈ [20,140]): of 9,381 directions contained at the original
field of view, **0** fell outside at a wider one.

The corresponding requirement for a *narrower* lens is computed by the planner from subject footprint
corners and is **not stored anywhere**: `ReframePlan` holds only schema version, source media id, source
range, retained segments, output specification and keyframes, and `CameraKeyframe` holds only time,
yaw, pitch, roll, field of view and interpolation. Neither the review session (which keeps only resolved
subject ids and directions) nor the decision artifact carries footprint sizes, and the doctrine forbids
re-parsing the stored instruction. Narrowing therefore **cannot be validated from a plan**, and remains a
re-planning operation.

## Decision

1. **Plan-level creator adjustment is permitted for exactly one operation: monotone lens widening** —
   `FOV_new,i = max(FOV_old,i, T)` for every keyframe of an already-valid plan, with `T` an absolute
   target in `(max FOV_old, 140]`.
2. **The first implementation is post-render only**: it operates on the plan already stored in a
   persisted render record's `EditDecision`. Pre-render adjustment of a reviewed-but-unrendered plan is
   **explicitly deferred**: such a plan has no persisted parent decision to point at, so honest
   attribution would need a new lineage semantic that this decision does not create.
3. **The transformation is pure and deterministic**, over a plan value: no media, no perception, no
   parser, no clock, no randomness, no locale.
4. **Only keyframe `fieldOfViewDeg` may change.** Camera aim (yaw, pitch), roll, keyframe times, keyframe
   count and order, interpolation, source range, retained segments, output specification, source media
   identity, schema version and every other plan field remain byte-identical. The input plan is never
   mutated.
5. **Field of view may only increase.** A target that does not actually widen any keyframe is refused, as
   is a target above the renderable maximum. **Narrowing is refused** and the refusal says why (the plan
   does not store what it must contain).
6. **The result must remain within the existing `[20, 140]` bounds** and must pass the existing
   `ReframePlan::isValid()`. The bounds are refused, never clamped: clamping would render a different
   lens than the stored plan claims.
7. **The transformation must be verified, not trusted.** A pure adjustment validator re-derives its
   invariants from the input and output plans and asserts, field by field, that the field of view is the
   only thing that changed and only upward. Exact equality is used deliberately: the transformation must
   *copy* fields, not recompute them.
8. **This is not a second intent→plan construction path.** It cannot choose an aim, a time, a segment, a
   subject or an output; it relaxes one scalar. `ReframeContract` IPC-1..4 remains a **build-time check
   over the transient original `(intent, plan)` pair**, run only inside `ReframeCommandRunner::prepare`,
   and is not re-applied — no intent exists to check against, exactly as for every replayed plan. IPC-1,
   IPC-2 and IPC-3 remain true because the fields they constrain are untouched; **IPC-4 deliberately
   ceases to describe the adjusted plan** (the plan no longer reaches the originally requested lens).
9. **Attribution must be auditable, not merely explained.** The adjusted plan becomes part of a NEW
   immutable `EditDecision` created with the existing `EditDecision::revisedFrom(parent, plan, media,
   instruction)`, so `origin=creator-revision` and `parentDecisionHash` — both **inside the hashed
   payload** — record the modification. The instruction carried is the parent's instruction verbatim
   (the adjusted plan *descends from* it; it does not honour it), and the human-readable parameter of the
   change is recorded in the record's existing `notes`. Attribution never rests on an unhashed note.
10. **No new persisted artifact, no schema change, no new origin value.** The plan is stored inside the
    decision, as every plan already is.
11. **The existing append gate and the existing render seam are used unchanged**: the adjusted plan is
    rendered through the same deterministic render entry point every render uses, and recorded through
    `appendReframeOutput`. No second rendering path exists.
12. **Decisions 055-057 apply unchanged**: the destination is a fresh sibling derived from the record
    being adjusted (`<base>_reframe_rev<N>.mp4`, the smallest free name), no held record's render may be
    overwritten, and the parent record and its file are never modified.
13. **Replay is unchanged and perception-free**: it executes the persisted adjusted plan, re-runs neither
    perception nor parsing, and never re-applies the adjustment.
14. **This decision does not generalize.** Arbitrary plan mutation, aim changes, timeline or keyframe
    editing, subject changes, output-geometry changes, narrowing and pre-render adjustment remain outside
    it and each would need its own decision.

## Consequences

- A creator can hold a framing they have seen and simply see more of the scene, instantly, with
  containment preserved by construction, full lineage, and a replayable record.
- The integrity of "plan ⇄ instruction" is now *descent plus attribution* rather than a re-verified
  fidelity claim. A reader comparing a stored instruction ("…, close-up") with a stored plan at 110° will
  find that the record says `origin=creator-revision` and names its parent, and that the note explains
  the parameter — the fact is hashed, the explanation is not.
- Each adjustment adds a record carrying a full plan copy, which sharpens the documented absence of a
  retention/compaction policy. That remains its own future decision.
- The widening ladder offered to the creator is derived from the existing framing vocabulary (the
  established default lens plus every wider value the vocabulary can request), so no new lens vocabulary
  is invented.

## Verification

Recorded before implementation; verified by the Objective 40 tests: the pure transformation and its
refusals (non-widening target, target above 140, non-finite target, invalid input plan), input-plan
immutability, the field-by-field "only FOV changed and only upward" validator, idempotence, plan validity,
containment preservation asserted with the existing tangent predicate at every keyframe **and at
intermediate camera times**, attributed child records with the correct parent hash, parent record and
parent file unchanged, fresh destinations that never overwrite a held render, replay of the widened
decision with no perception consulted, deterministic repeated execution, and the UI request and refusal
behaviour.

---

*Decisions 001-057 are preserved verbatim; this decision adds to them and supersedes none of them.*

---

# Decision 059 — A Browser Presentation/Control Layer Over the Existing Application, With a Job Boundary for Long-Running Work

**Status:** Accepted (2026-09-20, Architecture Obj A1; human-approved scope). This decision authorises a
**boundary**, not an implementation: no server, route, upload handler, preview endpoint, browser asset or
new dependency is created by it.

## Context

Reelcraft's 360 reframing engine is complete enough to be driven by a creator (41 objectives), but it has
exactly **one** presentation consumer: the Qt desktop shell, wired in `app/main.cpp` by 40
`QObject::connect` calls that join `Application` to `MainWindow` and contain no logic of their own. A
repository-wide search finds **no** HTTP client or server, socket, upload, byte-range, MIME or browser-asset
code anywhere, and `QtNetwork` is not linked (`reelcraft.pro:1` is `QT += widgets concurrent`).

A browser-based presentation and control surface is required. The risk this decision exists to foreclose is
the obvious one: a browser front end that re-implemented editing, planning, rendering, review, revision,
lineage, destination policy or media identity would be a second Reelcraft — and every guarantee the last
twenty objectives established would then hold only in the desktop half.

Three verified properties of the existing engine determine the shape of the boundary.

**1. Planning and rendering are synchronous and blocking.** `Application::runReframeCommandInternal`
(`app/application/Application.cpp:707`) calls its injected executor (`:766`, defaulting to
`ReframeCommandRunner::run`) and returns only when the work is finished; execution continues through
`ReframePipeline::renderPlan` (`app/reframe/ReframePipeline.cpp:70`) into `ReframeRenderer` and out to
an FFmpeg `QProcess`. There is no job, operation, progress or cancellation abstraction anywhere in the
repository.

**2. `Application` is single-threaded and holds no synchronisation primitive.** There is no mutex,
read-write lock, semaphore or atomic in `app/application/`. Serialisation today is a consequence of "one
caller, one thread", not a mechanism. An HTTP listener embedded in the same process would therefore have to
choose between blocking its own event loop for the whole operation and mutating application state from a
second thread — and the second option is not available at all.

**3. Plan preparation is not the cheap step it appears to be.** Target resolution runs one detector
subprocess **per covering view** (`app/target/TargetResolver.cpp:93-107`; the default view plan is 6 yaw ×
3 pitch, `app/target/EquirectViewPlan.h:20-25`) for **each** resolved timestamp, and a follow instruction
resolves up to 24 timestamps (`app/reframe/ReframeCommandRunner.h`, `followResolveSamplesMax`).
`Application::prepareReframeCommand` (`Application.cpp:933`) is therefore a long-running operation in its
own right, not only `acceptReframeReview` (`:989`).

The boundary drawn here is the one `ARCHITECTURE.md` already required. Its **Platform Boundary Constraint**
states that project state and application orchestration must be kept independent of QtWidgets, and that the
boundary must be maintained *"where the UI supplies paths/data rather than core code invoking platform
dialogs directly."* `Application` honours that — `app/application/` contains no QtWidgets symbol — and
`MainWindow::chooseMediaFilePath()` is `virtual` precisely so a different presentation consumer can supply
its paths differently. Decision 013 selected Qt 6 for the Phase 1 desktop foundation after evaluating
Electron and Tauri 2; **that decision is preserved, not superseded** — the desktop UI remains a first-class
consumer. `ARCHITECTURE.md` §19 lists web-based UI technologies as a candidate and §20 leaves the final UI
framework open; this decision settles the **boundary** and deliberately leaves the final framework question
open.

## Decision

### Authority and non-duplication

1. **`Application` remains the sole authoritative orchestration seam.** The browser is a **second
   presentation/control consumer**, a peer of `MainWindow`, and it reaches Reelcraft exclusively through
   `Application`'s public API and signals. A backend must not call `ReframeCommandRunner`,
   `ReframePipeline`, `ReframeRenderer`, `ReframePlanBuilder`, `ReframePlanAdjustment`,
   `ReframeContract`, `TargetResolver` or `EditDecision` directly; those sit behind `Application` for
   exactly the guarantees listed in item 4.
2. **The browser contains no Reelcraft logic.** It must not implement, in any form: an editing engine; a
   planner or intent parser; a renderer or encoder; a reframe-plan representation; subject or target
   resolution; a creator review system; a revision or lineage system; a destination or no-overwrite policy;
   or a media-identity/fingerprint implementation. Presentation, intent capture and request issuance only.
3. **No second domain model.** Transport DTOs may exist, but every field must map to an existing
   authoritative Reelcraft value type (`ReframeCommandOutcome`, `ReframePlan`, `EditDecision`,
   `ReframePlanReview`, `DecisionProvenance`, `MediaItem`, `Project`), and none may become a second
   source of truth. Where an existing type already serialises itself — `ReframeCommandOutcome::toJsonObject()`,
   `ReframePlanReview::summaryLines()`, `Project` — that serialisation is used rather than a parallel one.
4. **These existing invariants are preserved unchanged and may not be weakened, duplicated or re-derived at
   the boundary:** sacred read-only original media; the deterministic plan as the single edit representation;
   deterministic rendering and replay; creator review as a read-only view over the authoritative plan;
   creator revision lineage (single-parent, hashed, immutable decisions); the no-overwrite destination
   policy; unreadable render-record and unreadable-decision preservation; creator selection semantics;
   lens-widening revision semantics; provenance and decision lineage; and the `Application` orchestration
   boundary itself.

### Process, ownership and the job boundary

5. **A separate headless Reelcraft backend process owns the authoritative `Application` instance.** The
   HTTP listener is **not** embedded in the existing GUI process. The reason is Context item 2: an embedded
   server has no safe concurrency model over a synchronous, non-thread-safe `Application`, whereas a
   separate process obtains a single owner and serialised mutation for free.
6. **One owner and one thread mutate `Application` state.** Mutations that touch application state are
   serialised through that single owner. No concurrent mutation is permitted unless the application layer is
   explicitly redesigned for it, which this decision does not authorise.
7. **Long-running work must not require holding an HTTP request open.** Because the pipeline is synchronous
   and blocking (Context item 1), a request that waits for a render or a plan preparation is architecturally
   forbidden. The required shape is:

   ```
   HTTP request
     -> validate / accept the command
     -> create or identify an operation
     -> return promptly
     -> the authoritative backend executes through the existing Application/engine
     -> the operation reaches a terminal state (completed / failed / ...)
     -> the browser observes operation state
   ```

8. **Plan preparation is a long-running operation, not a fast call**, and must be modelled as one for the
   same reason as rendering (Context item 3).
9. **The job/operation boundary is a bookkeeping boundary, never a second engine.** It identifies work,
   reports its state and holds its terminal outcome. It must not plan, render, re-plan, retry by re-deriving
   an edit, or hold a shadow copy of application state. Its terminal state derives from the authoritative
   `Application` result (`reframeCommandFinished`, `reframeOutputsChanged`, `reframeReviewChanged`) —
   never from a parallel execution path.
10. **The exact worker implementation, progress granularity, cancellation semantics and event transport are
    deliberately left open by this decision.** They are implementation decisions for the objective that
    builds the boundary, subject to items 6, 7 and 9.

### Media input

11. **Bytes arriving over HTTP do not make canonical Reelcraft media.** Uploaded content becomes a
    `MediaItem` only after the existing import path accepts it:
    - uploads are **streamed**, not accumulated in memory;
    - bytes are written to a **controlled staging location**;
    - the content is **validated and finalised before** it is exposed as canonical media;
    - the **stable canonical path is established before** any identity or fingerprint is derived from it —
      `MediaItem::id` is the SHA-256 of the canonical file path (`app/core/MediaItem.cpp:59-60`) and an
      `EditDecision` fingerprints size and modification time, so a path that later changed would silently
      invalidate every decision made against it;
    - a **browser-supplied path, filename or URL may never escape the application's controlled media storage
      boundary**;
    - the sacred/read-only original-media invariant applies unchanged to the finalised file.
12. **No media storage database, retention policy, cleanup policy, resumable-upload protocol or multi-user
    storage model is chosen here.** Those remain open architecture questions (Open Questions 4 and 10).

### Media output and its serving

13. **Serving rendered media to a browser is a transport concern, not a rendering concern.** It introduces no
    new render path, no new encode and no new artifact: the bytes served are the file the existing
    deterministic renderer already produced and that a render record already names.
14. Any such serving must account for normal browser media behaviour — **correct MIME type,
    `Content-Length`, byte-range requests, `206 Partial Content`, invalid-range handling, stable file
    identity while the media is being served, and no serving of arbitrary filesystem paths**. This decision
    records the requirement and implements nothing.

### 360 camera and projection authority

15. **There is exactly ONE authoritative Reelcraft camera/projection model and it stays in C++.**
    `ViewportState` (`app/viewer/ViewportState.h`) owns the camera state and `EquirectView`
    (`app/viewer/EquirectView.h`, implementation `EquirectView.cpp:127`) is the single image-projection
    primitive — already shared by the viewer, the renderer, the target resolver and the crop extractor. A
    browser preview must not introduce a divergent convention: JavaScript that recomputed yaw/pitch/roll/FOV
    projection would be a second camera model, and would disagree with the model that seeds the creator's
    "me" identity from the viewport (`Application::selectCreatorTargetFromViewport`).
16. **This decision does not choose the preview mechanism.** WebGL, server-rendered PNG/JPEG frames, canvas
    rendering, a video-streaming format, a preview frame rate and a preview encoding are all explicitly
    **not decided here**; each is an implementation decision for a later objective. What is fixed is item 15.

### API boundary

17. **The HTTP layer is a thin transport/control boundary and is conceptually versioned** (`/api/v1/...`).
    Route names are illustrative and not immutable. The capabilities it may expose are project/session
    establishment, media upload/import, active-media selection, media listing, 360 preview access, viewport
    interaction, natural-language plan preparation, creator review, accept/reject, render execution, render
    status, render listing, rendered-media playback, creator lens widening/revision, and operation/event
    status. Any route that would require the browser to hold domain state is out of bounds by item 2.

### Errors

18. **A structured, machine-distinguishable error model is an architectural requirement.** The browser must
    be able to distinguish at least: malformed request; invalid Reelcraft command; unavailable capability;
    failed operation; missing media; missing render; conflict/state violation; unsupported operation; and
    internal/unexpected failure. **The browser must never depend on parsing human-readable error text.** The
    concrete schema is an implementation decision.

### Status and events

19. **Whatever mechanism reports operation state must survive disconnection.** It must account for reconnecting
    clients, events missed during a temporary disconnection, operation identity, current-state recovery after
    reconnect, and must not depend on one permanently open connection. **This decision does not select the
    transport**: polling is an acceptable implementation fallback if the architecture stays clean, and SSE (or
    anything else) is not assumed correct.

### Security and network scope

20. **The non-negotiable invariant: the HTTP boundary must never expose arbitrary filesystem access or
    arbitrary command execution merely because a browser supplied a path, filename, URL, executable or
    argument.** Every path a request influences is resolved inside application-controlled storage, and every
    path comparison the boundary relies on must be identity-based rather than string-based.
21. **The following remain explicitly UNRESOLVED and are not decided here:** local-only vs LAN/remote access;
    authentication; authorization; TLS; multi-user support; project/session ownership; cross-origin policy;
    and CSRF considerations if browser credentials are ever used. `ARCHITECTURE.md` §15 already makes access
    control, project isolation, secure media handling and temporary processing artifacts architectural
    requirements; this decision does not narrow that to a mechanism.

### Concurrency, ownership and dependencies

22. **One authoritative backend process owns application state; the browser maintains no independent editing
    state; and simultaneous desktop and backend ownership of one project remains an unresolved
    deployment/product decision.** Reelcraft's no-overwrite guarantee is enforced *within one `Application`
    instance* (`Application::recordHoldingOutputPath`), so two live owners of one project would break it.
    Until that is decided, one writer is assumed.
23. **No database, distributed lock, multi-user architecture, message broker or service mesh is introduced.**
24. **No new runtime dependency is authorised by this decision.** React, Vue, Node/npm, TypeScript, Electron,
    Tauri, Qt WebEngine and any third-party HTTP framework are not authorised. A plain static
    HTML/CSS/JavaScript presentation layer served as files, plus the smallest appropriate existing
    Qt/network mechanism, remain the default direction. **If a new dependency proves necessary it is an
    explicit dependency gate requiring human authorisation** — it may not be introduced silently as an
    implementation detail.

## Consequences

- The browser becomes a first-class way to drive the existing engine without any part of the engine being
  reimplemented, and the desktop UI keeps working unchanged as a peer consumer.
- `Application`'s seam is now load-bearing for two presentation consumers. Its contract — established by the
  Platform Boundary Constraint and by the injection seams the model-free tests already use — becomes the de
  facto public API of Reelcraft, and a change to it now has two consumers to consider.
- Long-running work acquires an identity it does not have today. `Application` reports a command only when it
  finishes (`reframeCommandFinished`), and `m_lastReframeOutcome` is a single slot; a browser needs to ask
  "what is running, and how did the thing I started end?" That is new bookkeeping, and item 9 is what keeps it
  from becoming a second engine.
- Uploads introduce a media lifecycle Reelcraft does not have today. `Application` has no project path, no
  staging area and no storage root, and `importMediaFile` accepts only a path to a file that already exists.
  Where uploaded bytes live is therefore a foundational decision this one deliberately does not make (Open
  Question 4), because `MediaItem` identity is path-derived and a cleanup policy that deleted a referenced
  file would invalidate decisions.
- Two pre-existing properties become **contractual** rather than incidental once bytes are served over HTTP:
  render output is currently written directly to its final path by FFmpeg with no atomic publication, and the
  destination guards compare non-canonical `QFileInfo::absoluteFilePath()` against canonical media paths.
  Serving a partially written file, or resolving a destination through a symlink, are boundary-reachable
  versions of those properties. Item 20 states the invariant; the concrete hardening is implementation work
  this decision identifies but does not perform.
- The final UI framework question (`ARCHITECTURE.md` §20) stays open, and the deployment/packaging strategy is
  untouched.

## Open questions (deliberately unresolved)

1. Local-only vs LAN/remote access (item 21).
2. Authentication and authorization (item 21).
3. Project/session identity and persistence — `Application` has no project path and `Project` stores no
   self-path (item 21; see Consequences).
4. Media storage location, retention, lifecycle and cleanup (items 11, 12).
5. Concurrent desktop/backend ownership and simultaneous access (item 22).
6. Multi-user scope (items 12, 21, 22).
7. Review persistence — creator review is session state today (Decision 054) (item 19).
8. Event transport details (item 19).
9. Preview transport and rendering strategy (item 16).
10. Upload resumability (item 12).
11. Render progress and cancellation semantics (item 10).
12. Audio playback and dependency architecture (Decision 036 follow-up), where applicable to a browser client.
13. Any new dependency requiring approval (item 24).

## Verification

Documentation-level, as this decision authorises no code. Verified during inspection at
`a1386f36ca6b7c05fe51cd5c196ccbf40d95deb0`:

- `Application` (`app/application/Application.h:94`) is the sole orchestration seam and contains no
  QtWidgets symbol; `app/main.cpp` wires it to `MainWindow` with no intervening logic.
- No HTTP, socket, server, upload, byte-range, MIME or browser-asset code exists anywhere in the repository,
  and `QtNetwork`/`QtWebEngine` are not linked.
- The render and planning paths are synchronous and blocking, with no job, progress or cancellation
  abstraction.
- No synchronisation primitive exists in `app/application/`.
- `MediaItem` identity is the SHA-256 of the canonical path; `MediaItem` never probes content.
- The destination guards use `absoluteFilePath()`; `MediaItem::path()` is canonical.
- `ViewportState`/`EquirectView` are the single camera/projection authority, with four consumers and one
  implementation.
- Decision 013 (Qt 6 desktop foundation), Decision 014 (platform strategy), Decisions 054-058 (review,
  revision, destinations, lens widening) and `ARCHITECTURE.md`'s Platform Boundary Constraint are all
  preserved by this decision, and none is superseded.

---

*Decisions 001-058 are preserved verbatim; this decision adds to them and supersedes none of them.*




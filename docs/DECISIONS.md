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

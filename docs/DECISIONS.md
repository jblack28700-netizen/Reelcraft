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


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


# Reelcraft — Known Issues

## Purpose

This document records known limitations, unresolved problems, risks, and areas requiring future investigation.

Known issues must be described accurately and should not be used to hide incomplete implementation.

Issues should be removed or marked resolved only after verification.

---

# Current Project State

Reelcraft is currently in the documentation and architecture foundation stage.

There is no application implementation yet.

Therefore, most current limitations are architectural or implementation-planning gaps rather than software defects.

---

## KI-001 — No Application Implementation

### Status

Open

### Description

The Reelcraft application itself has not yet been implemented.

The current repository primarily contains project documentation and architecture planning.

### Impact

No end-user editing workflow is currently available.

### Planned Resolution

Begin Phase 1 — Foundation with small, independently verifiable implementation tasks.

---

## KI-002 — Project/Edit Schema Not Finalized

### Status

Open

### Description

The final persistent project and edit-model schema has not yet been defined.

### Impact

Implementation of project persistence and structured editing cannot be considered finalized until the schema is designed and validated.

### Planned Resolution

Define the minimum viable project/edit model during the appropriate Phase 1 implementation task.

---

## KI-003 — Media Engine Architecture Not Implemented

### Status

Open

### Description

The deterministic media engine is currently an architectural concept rather than an implemented subsystem.

### Impact

No actual media processing pipeline exists yet.

### Planned Resolution

Implement the media-engine foundation incrementally after the core project foundation is established.

---

## KI-004 — AI Provider Strategy Not Finalized

### Status

Open

### Description

No permanent AI provider or model has been selected.

### Impact

AI functionality cannot yet be optimized around a specific provider or model.

### Planned Resolution

Maintain provider abstraction and evaluate models according to actual Reelcraft requirements during implementation.

---

## KI-005 — Local/Cloud Processing Allocation Not Finalized

### Status

Open

### Description

The exact division of workloads between local, cloud, and hybrid processing remains undecided.

### Impact

Infrastructure and deployment architecture cannot yet be finalized.

### Planned Resolution

Use measured performance, cost, privacy, hardware, and model requirements to guide future decisions.

---

## KI-006 — GPU Acceleration Strategy Not Finalized

### Status

Open

### Description

The long-term GPU acceleration strategy has not yet been determined.

### Impact

Performance architecture remains provisional.

### Planned Resolution

Evaluate acceleration options when real media-processing workloads exist and performance measurements can be collected.

---

## KI-007 — 360° Processing Pipeline Not Implemented

### Status

Open

### Description

360° video is a first-class architectural requirement, but the actual 360° processing, viewing, and reframing pipeline has not yet been implemented.

### Impact

The core 360° creator workflow is not yet functional.

### Planned Resolution

Implement 360° support incrementally, beginning with the foundation required for reliable media representation and viewing.

---

## KI-008 — Camera Adapter System Not Implemented

### Status

Open

### Description

Camera-specific behavior is intended to be isolated behind adapters, but the adapter system has not yet been implemented.

### Impact

Additional camera support cannot yet be added through a formal adapter interface.

### Planned Resolution

Define and implement the adapter boundary before adding substantial camera-specific functionality.

---

## KI-009 — Automated Test Infrastructure Not Implemented

### Status

Open

### Description

The project does not yet contain an application test suite or automated regression framework.

### Impact

Implementation work cannot yet rely on automated application-level regression testing.

### Planned Resolution

Establish testing infrastructure as part of the implementation foundation before substantial feature development.

---

## KI-010 — Final Technology Stack Not Finalized

### Status

Open

### Description

The final UI framework, runtime, storage system, rendering architecture, and deployment strategy remain intentionally undecided.

### Impact

Implementation technology choices must still be evaluated rather than assumed.

### Planned Resolution

Make technology decisions incrementally based on documented requirements, testing, and measurable constraints.

---

# Issue — Current aarch64 proot development environment: GCC driver prefix and unavailable bash sandbox (2026-09-17)

### Status

Open — environment limitation, not a product defect.

### Description

On the current aarch64 proot/Termux development device:

- The Debian GCC 15 driver could not locate `cc1`/`cc1plus` (installed under `/usr/libexec/gcc/...`) or `ld` when invoked as bare `g++`, because it computed an empty/relative install prefix from `argv[0]`. It was repaired non-destructively by symlinking `cc1`, `cc1plus`, and `ld` into `/usr/lib/gcc/aarch64-linux-gnu/15/` and invoking `/usr/bin/g++` (absolute path). Builds must pass `QMAKE_CC=/usr/bin/gcc QMAKE_CXX=/usr/bin/g++`.
- FFmpeg is not on the Debian PATH; the Termux build is used via `REELCRAFT_FFMPEG=/data/data/com.termux/files/usr/bin/ffmpeg`.
- The `bash` tool is unavailable: the workspace-write sandbox backend (bwrap) cannot start on this host, and escalation to `danger-full-access` was declined. Inspection, builds, and tests were performed through the in-process code runtime and detached background processes only.

### Impact

Build/test commands must set the compiler and FFmpeg environment explicitly. Shell-based workflows are not directly available. This does not affect product architecture or source.

### Planned Resolution

Re-verify the standard `scripts/build_and_test.sh` workflow on a host with a working sandbox and a correctly installed GCC; record the result in `DEVELOPMENT_ENVIRONMENT.md`. The GCC symlinks are reversible.

---

# Issue — No real detection model is integrated (360 target resolution) (2026-09-17)

### Status

Partially resolved (2026-09-17) — real detection is now verified on real 360 footage; the helper/model remain external and optional by design.

### Description

The target-resolution layer (`app/target/`) implements a replaceable detector seam (`TargetDetector`) and a dependency-free subprocess adapter (`ProcessTargetDetector`). Historically it bundled no real detector because the environment had no CV runtime; that gap is now closed for verification: an optional external helper using OpenCV Zoo YOLOX (Apache-2.0) + OpenCV DNN (CPU) runs through the existing protocol and detected real people in the project's real 3840x1920 equirect clip. Reelcraft still links no CV library, and no model weights are committed.

Consequently, "real-world detection verified" is now claimed with the caveat that a detector helper + model + suitable footage must be provided; the normal unit suite stays model-free (the real test skips when unconfigured).

### Impact

Requests such as "follow me", "keep me centered", "follow the person speaking", and "look at the car" still require a resolved target direction (from a detector helper, a manual/creator selection, or a future model). Unresolved targets fail deterministically and are reported rather than fabricated.

### Planned Resolution

Implemented for the first slice (Decision 020): an optional permissively licensed helper behind the existing `ProcessTargetDetector` protocol (`tools/detector_helper/`), with a separate integration test. Remaining work: GPU/RunPod inference, additional models, and identity/speaker resolution. Do not adopt AGPL Ultralytics models without an explicit product/licensing decision.

---

# Issue — "me" identity is geometric, not biometric (360 target identity) (2026-09-17)

### Status

Open — intentional first-slice boundary; not a defect.

### Description

Creator identity is a deterministic binding between the canonical key "me" and a tracker track id, established from a structured creator seed (direction/time or track id) and maintained through geometric trajectory continuity (bounded velocity prediction and re-entry). It is not appearance-based or biometric.

Consequently, identity survives normal movement, crossing trajectories, temporary detection loss/occlusion, and short re-entry with a unique continuation, but it cannot reliably re-identify a person after a long absence, among visually similar people, or when a unique continuation cannot be established. In those cases the registry reports the identity unresolved or ambiguous rather than guessing. On crowded real footage, "the other person" is reported ambiguous when more than one other person is visible.

### Impact

References to "me" require an explicit creator selection first. Multi-person references that are genuinely ambiguous return no target and a candidate list; callers must disambiguate (for example with "person 1"/"person 2").

### Planned Resolution

Add an optional, replaceable appearance/embedding re-identification seam behind `IdentityBinding`, plus audio-visual speaker association, as a future objective. Keep any model optional so the unit suite stays model-free.

---

# Issue — Appearance re-identification is similarity-based, not biometric (2026-09-17)

### Status

Open — intentional boundary; not a defect.

### Description

The appearance layer uses a person re-identification embedding (OpenVINO OMZ `person-reidentification-retail-0277`) and cosine similarity with configurable accept (0.75) and reject (0.55) thresholds. It supports bounded re-acquisition and can veto an unjustified geometric identity transfer, but it is not face recognition and does not provide biometric certainty. Thresholds are domain dependent and may need retuning for different footage.

Weight licensing was checked explicitly: the OMZ model is Apache-2.0 for code and weights and trained on an internal dataset. OpenCV Zoo's `person_reid_youtureid` weights come from an unlicensed source trained on Market1501/DukeMTMC/MSMT17 and were rejected; OSNet/torchreid pretrained weights have the same research-dataset concern.

### Impact

After a long absence, a strong appearance match is required to re-acquire "me"; weak or conflicting evidence (including geometry/appearance disagreement) leaves the identity unresolved rather than guessing. In crowded or appearance-similar scenes, re-acquisition may remain unresolved.

### Planned Resolution

Keep the appearance provider replaceable; add active-speaker/audio-visual association and, separately, evaluate stronger appearance models and GPU inference behind the same interface.

---

# Issue — Audio provides evidence, not speaker identity (2026-09-17)

### Status

Open — attribution seam complete; no reliable, permissively licensed model-backed provider available (updated 2026-09-17, Objective 7).

### Description

The audio layer uses Silero VAD (voice activity detection), which reports *when* speech occurs, not *who* is speaking. A provider-local `speakerId` ("spk1" for VAD-only) is associated with a visible track by an explicit creator binding, an optional provider-supplied direction of arrival, or the single-visible-person rule. With several visible people and no explicit binding, the result is correctly reported as ambiguous/unassociated rather than guessed.

Audio never changes identity resolution: `TargetIdentityRegistry::annotateSpeaker` records evidence only, and identity precedence remains explicit selection > tracker continuity > geometry > appearance > audio > unresolved.

Weight licensing was checked: Silero VAD is MIT (code and the ONNX weights). pyannote diarization models are gated and the runtime is heavy; SpeechBrain/torchreid speaker embeddings carry VoxCeleb weight provenance; cloud APIs and audio-visual active-speaker models were deferred.

### Objective 7 investigation (2026-09-17)

The provider-attribution seam is now complete: an `AudioVisual`/`Diarization` provider can attach a `targetIdHint` (an existing target track id) to a speech interval, and `SpeakerTargetAssociator` honours it deterministically (explicit binding > visible hint > spatial DoA > single visible > ambiguous/unassociated). No model-backed provider is shipped, because the available evidence does not support a reliable one in this environment:

- The real 360 footage audio is **mono** in both the original and the derived proxy, so direction-of-arrival attribution is impossible.
- A face-detection + mouth-motion/audio-envelope correlation probe (12 fps, 48 frames, lags −3..+3, OpenCV YuNet) gave max correlation 0.250 for the speaker vs 0.192 for the listener — a weak separation that is not a reliable discriminator.
- No permissively licensed, clearly commercial audio-visual active-speaker model was identified (TalkNet/LoCoNet/AV-HuBERT research-grade/unclear weights; pyannote gated and PyTorch-heavy; SpeechBrain/torchreid VoxCeleb provenance).

### Impact

"Follow whoever is speaking" works when the creator identifies the speaker once (or when only one person is visible). Fully automatic attribution among multiple people requires a diarization or audio-visual active-speaker provider; the seam is ready for one, and until one is supplied the result is honestly reported as ambiguous/unassociated rather than guessed.

### Planned Resolution

Add an optional diarization/audio-visual provider behind the existing `SpeakerEvidenceProvider` seam that supplies `targetIdHint`; keep models optional and the unit suite model-free. Re-evaluate licensed audio-visual models and (if ever multi-channel audio is available) direction-of-arrival association.

---

# Issue — Generated renders are session state (2026-09-17)

### Status

Resolved (2026-09-17, Objective 10) — see Resolution.

### Description

The application-level command path (Objective 9) records each command in a structured `ReframeCommandOutcome` (source, instruction, effective range, output path, dimensions, frame count, errors), but that record and the generated output file are session state: they are not persisted in the project schema, and the application does not re-list prior renders after restarting or reopening a project. There is also no duration/ffprobe metadata, so a command without its own time range uses the caller-supplied fallback range rather than the whole clip.

### Impact

After restarting or reopening a project, previously generated renders are not shown in the application. Long clips require the user to set the range explicitly (or use the UI fallback) until duration probing exists.

### Planned Resolution

Add a small project output section (or reuse the existing media/attributes pattern) to persist render records, and implement the deferred ffprobe duration metadata so range-less commands can default to the whole clip. Do not build a large persistence system before the project schema supports it.

### Resolution

Implemented in Objective 10 (Decision 027). `Project` gained an additive `reframeOutputs` section (`CurrentSchemaVersion` 3); the application records every command that reaches an output target and re-lists records through `reframeOutputsChanged`; and a replaceable `MediaDurationProbe`/`FfprobeDurationProbe` seam resolves a zero ("whole clip") range to `[0, durationMs]`. Model-free tests cover parsing, range defaulting, record creation, and save/open persistence; `realApplicationCommandIntegration` verifies whole-clip probing and persistence on real 360 footage. Remaining limitation: records are not revalidated against a now-missing output file, and a missing ffprobe still requires an explicit range.

---

# Issue Management Rules

For each future issue:

1. Assign a unique issue identifier.
2. Record its status.
3. Describe the problem clearly.
4. Record its impact.
5. Record the intended resolution or investigation.
6. Verify the resolution before marking the issue resolved.
7. Update related documentation when an issue changes an architectural decision.

Do not silently remove historical issues.

When an issue is resolved, preserve its record and mark it resolved with the verification date and relevant checkpoint when appropriate.

## 2026-09-03 — Environment/Observed

- Offscreen QPA plugin reports `This plugin does not support propagateSizeHints()` during headless smoke test; not observed under a normal windowing platform.
- `qmake` is not on the default PATH in the current environment; use `/usr/lib/qt6/bin/qmake`.

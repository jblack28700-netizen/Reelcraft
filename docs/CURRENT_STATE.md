# Reelcraft — Current State

**This document describes the system as it is NOW.** Per-objective implementation detail is not
repeated here: it lives in `DEVELOPMENT_LOG.md` (chronological record), `DECISIONS.md`
(architectural decisions), `NEXT_TASK.md` (capability register and candidates) and
`PROJECT_HISTORY.md` (milestone narrative). No product behaviour is changed by editing this file.

## Version and branch

- Version: **0.2.67** (user-facing history: `CHANGELOG.md`).
- Branch: `main`. Current checkpoint: `5592118074624348033185afcb462069284e6c13` (Objective 30).

## Priority and stage

- **Human-approved priority:** a working **360 reframing / editing capability**.
  Pipeline: 360 source -> scene/subject understanding -> natural-language or structured request ->
  structured edit/reframe plan -> virtual-camera decisions -> deterministic execution -> flat output.
- **Stage:** Phase 4 (360 reframing engine) — 30 objectives complete and verified; Phase 3 media
  engine is open, its player-lifecycle objective deferred behind the 360 priority.
- **Current objective:** none selected. The approved multi-subject framing batch is complete —
  Objective 31 (N-way group framing, Decision 052) and Objective 32 (explicit subject sets,
  Decision 053) — and the next capability is chosen by the human from `NEXT_TASK.md` §5.

## Subsystem status

| Subsystem | Status | Where |
|---|---|---|
| Desktop shell, project/media library, additive schema v3 | implemented, verified | `app/core/`, `app/ui/` |
| Decode seam, frame pump, player/timing | implemented, verified (no Application lifecycle wiring) | `app/media/`, `app/playback/` |
| Equirectangular viewer presentation + look-around | implemented, verified | `app/viewer/`, `app/ui/ViewerWidget.*` |
| 360 source playback (proxy stream, seek, viewpoint) | implemented, verified; **video-only** | `app/application/Application.*` |
| Reframing engine (plan, camera path, render, encode) | implemented, verified | `app/reframe/` |
| Subject framing: follow path, sampling density, decoder reuse, smoothing; **group framing (2-10 subjects) and explicit subject sets** | implemented, verified (fixtures) | `TargetTrackPlanner`, `ReframeCommandRunner` |
| Natural-language intent (directions, subjects, temporal, compound, lens, group sizes) | implemented, verified | `app/reframe/ReframeIntent.*` |
| Target resolution, tracking, identity, selection | implemented, verified (real detector via external helper) | `app/target/` |
| Appearance re-identification, speaker evidence | implemented as **optional seams**; real inference needs external helpers | `app/target/` |
| Persisted reproducible decisions + provenance/revision | implemented, verified (fresh-process replay) | `app/reframe/EditDecision.*` |
| Intent -> plan contract checker (IPC-1..4) | implemented, verified | `app/reframe/ReframeContract.*` |
| Rendered-output audio preservation | implemented, verified **on fixtures only** | `ReframeRenderer`, `ReframePipeline` |
| Persistent media analysis (artifact + one-pass runner) | implemented; **no consumer yet** | `app/analysis/` |
| Creator review/revision UI | **missing** (APIs exist) | — |
| In-app audio playback | **missing** (Decision 036 follow-up) | — |

## Test baseline

- Current result: **487 passed, 0 failed, 14 skipped** (Objective 33 checkpoint; this is the
  canonical form `scripts/checkpoint_check.sh` reads). Full suite with
  `scripts/build_and_test.sh`; a focused set by passing QtTest function names to
  `tests/reelcraft_tests`.
- Measured wall time: **~66 s** on the current RunPod container; ~730 s on the proot development
  device, which the script's **900 s** ceiling accommodates. The FFmpeg render/decode/analysis tests
  dominate; a 25-test logic subset runs in ~1.7 s.
- The **14 skips are by design**: 13 environment-gated real-media/model integrations (8 from
  Objectives 3-19 plus the 5 Objective 28-32 harness tests added by Objective 33) and the child-only
  fresh-process replay slot. Which capabilities are therefore fixture-only is tracked in
  `NEXT_TASK.md` §4 (validation debt).
- The normal suite is model-free and requires no detector, model, network or real footage.

## Validation status

Fixture-verified: the whole model-free suite. Real-media-verified at some earlier checkpoint: source
playback, target detection, follow pipelines, temporal edits, rendered-result playback, application
commands (when the `REELCRAFT_*` inputs are configured).

**Not real-media-verified: rendered-output audio (Obj 28), lens control (Obj 29), two-subject framing
(Obj 30), N-way group framing (Obj 31) and explicit subject sets (Obj 32).** A readiness assessment on
2026-09-18 found this container cannot run that sweep (no real 360 clip, no `numpy`/`cv2`/
`onnxruntime`, no model weights), and Objective 33 built the five-test harness that will run it —
`realMediaAudioPreservation`, `realMediaLensRequestReachesOutput`,
`realMediaMultiSubjectContainment`, `realMediaGroupInfeasibilityIsHonest`,
`realMediaExplicitReferencesResolve` — which currently skip with precise prerequisite messages.
Exact inventory, probes and the invocation contract are in `DEVELOPMENT_ENVIRONMENT.md`; the debt and
its minimum unblocking action are in `NEXT_TASK.md` §4.

## Known limitations (current, not historical)

- **No audio output in the application**: rendered files carry audio (Obj 28), but source and
  rendered-result playback are video-only (Decision 036).
- **No creator review/revision UI**: `decisionProvenance()`, `reviseEditDecision()` and
  `replayEditDecision()` exist as APIs only; accept/reject status is session-only.
- **Media analysis has no consumer**: artifacts are produced and referenced, never yet used to make
  a decision; sampling defaults to a 1 s grid.
- **Perception needs external helpers**: detection, appearance and speaker evidence are optional
  subprocess helpers; Reelcraft links no CV/ML runtime. Identity is geometric (+ optional
  appearance), not biometric; automatic speaker attribution remains provider-gated.
- **Framing limits**: group framing covers two to ten subjects by group phrase ("the three of us",
  "everyone") or by explicitly named references ("me and person 2", "the presenter and the guest"),
  resolved through the existing selector vocabulary; a group is refused rather than trimmed when it
  cannot fit the renderable field of view, when its size cannot be satisfied, when a named reference
  does not resolve or is ambiguous, or when the set reduces to one distinct subject. Sizes above ten,
  dynamic membership and references longer than a phrase are not supported; the group path is not
  smoothed; framing offsets/lead room are deliberately absent (Decision 046).
- **No timeline editor, multi-source editing, transitions, captions or colour work**; output is flat
  video only (no 360/equirect export).
- **No retention policy** for accumulated edit decisions or analysis artifacts.
- Technology choices that remain open: final AI provider/model selection, local/cloud split, GPU
  acceleration, packaging/deployment (see `ARCHITECTURE.md` §20).

## Development environment

Development runs on a Linux RunPod container (`/workspace/Reelcraft`) and historically on an
Android/Termux + proot Ubuntu device (`/root/reelcraft`) with a 12x slower wall clock. Build and
test commands, the FFmpeg configuration, helper runtimes and the `REELCRAFT_*` variables are all
documented in `DEVELOPMENT_ENVIRONMENT.md`. Verify build-tree freshness with
`scripts/checkpoint_check.sh` before claiming a checkpoint.

## Development rules

The canonical operating policy is `AGENT_WORKFLOW.md` (autonomy, validation tiers, scope control,
documentation, Git checkpoints, controlled batches and gates). Project identity, authority order and
stop-and-ask conditions are in `AI_HANDOFF.md`; product requirements in `REQUIREMENTS.md` and
`PROJECT_MODEL.md`; the AI/deterministic boundary in `AI_EDIT_CONTRACT.md`.

## Where information lives (one home per fact)

| Document | Owns |
|---|---|
| `DEVELOPMENT_LOG.md` | detailed chronological engineering record (one entry per objective) |
| `DECISIONS.md` | immutable architectural decisions (append-only, never rewritten) |
| `CURRENT_STATE.md` | current system state (this file) |
| `NEXT_TASK.md` | capability/dependency register, validation debt, candidates |
| `ARCHITECTURE.md` | architecture, seam index, behaviour -> test index |
| `CHANGELOG.md` | user-facing changes only |
| `PROJECT_HISTORY.md` | milestone narrative (phase/milestone level) |
| `AGENT_WORKFLOW.md` | canonical development policy |
| `KNOWN_ISSUES.md` | open limitations and deferred work, with reasons |
| `DEVELOPMENT_ENVIRONMENT.md` | build/run commands and environment constraints |

## Completed objectives index

Detailed records live in `DEVELOPMENT_LOG.md`; the commit column is the checkpoint that delivered
each objective.

| Objectives | Delivered | Commits |
|---|---|---|
| Phase 1 (foundation) | Qt 6 shell, project lifecycle, offscreen test suite | `f4ad631`^, Phase 1 series |
| Phase 2 Obj 1-15 | viewer state/presentation/camera, media records, equirect preview, time navigation, pointer look-around, flat path, state consistency, bilinear rendering, real-media review validation | Phase 2 series; closed at `dcd876b` |
| Phase 3 Obj 2-4 | persistent-stream feasibility, replaceable media-source seam + frame pump, player/timing foundation | `706ef08`, `f50a52a`, `28f65b8` |
| Phase 3 Obj 5 | application-level player lifecycle — **not started** (deferred behind the 360 priority) | — |
| Phase 4 Obj 1-6 | reframing vertical slice, target/subject resolution, real detector integration, identity + selection, appearance re-identification, audio/speaker evidence | `bb17a1c`, `37703fa`, `c75d979`, `b2ca847`, `b7ee7fa`, `8ecc17b` |
| Phase 4 Obj 7-15 | provider-attribution seam, end-to-end command execution, application orchestration, persisted outputs + duration, speaker-aware commands, command UI + preview, rendered-result playback, temporal editing, compound commands | `f4ad631`, `c581f95`, `ec641a7`, `1bc7324`, `b5b5120`, `41ab5f9`, `766e3ef`, `aaae143`, `98b0413` |
| Phase 4 Obj 16-21 | reproducible edit decisions, provenance/revision, contract checker, source playback, persistent render decoding, persistent media analysis | `bed4f97`, `0902a16`, `dc42d82`, `94d8bb4`, `56f7ac2`, `90f671c` |
| Phase 4 Obj 23-27 | follow camera paths, follow sampling density, decoder reuse, trajectory smoothing, covering-view duplicate consolidation (no Objective 22 was scoped) | `39668ca`, `99a7282`, `c872dc3`, `0e90fc3`, `0144bfa` |
| Phase 4 Obj 28-32 | source-audio preservation, lens/FOV control, two-subject framing, N-way group framing (with the enclosure rule recomputed exactly in the renderer's basis), explicit subject sets | `7adf433`, `d971009`, `5592118`, `414c33d`, `this checkpoint` |
| Process Obj P1 | development-management consolidation (register, seam/test indexes, batch+gate policy, checkpoint hygiene) | this checkpoint |

^ `f4ad631` also carries the canonical workflow policy (Objective 7's precedent).

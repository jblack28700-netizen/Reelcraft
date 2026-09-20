# Reelcraft — Next Task & Capability Register

**Purpose.** This is the **operational register**: what exists, what is missing, what blocks what,
how well each capability is validated, and which objectives are candidates. It is a register, not a
history — per-objective implementation detail belongs in `DEVELOPMENT_LOG.md`, architectural
decisions in `DECISIONS.md`, and the current system state in `CURRENT_STATE.md`.
Execution policy (including controlled batches and gates) is `AGENT_WORKFLOW.md` §12.

**Status key:** `done` · `partial` · `missing` · `blocked` · `deferred`
**Validation key:** `fixture` = generated media + model-free tests · `real-media` = env-gated
integration verified on real footage/model/app · `—` = none yet.

---

## 1. Current priority (human-approved)

**360 Reframing / Editing capability.**
Pipeline: 360 source -> scene/subject understanding -> natural-language or structured request ->
structured edit/reframe plan -> virtual-camera decisions -> deterministic execution -> flat output.

## 2. Current objective

**None selected.** Architecture Obj A1 (**Decision 059** — the browser presentation/control boundary
over the existing application) is complete; it authorises a **boundary only** and created no server, route,
browser asset or dependency. It follows Objective 41 (creator modifications visible in the render list and
the provenance readout), Objective 40 (creator lens widening, Decision 058), Objective 39 (render-list
selection), Objective 38 (record preservation) and Objective 37 (render destinations, Decision 057). Each
delivered objective has its own commit.

**The next capability is chosen by the human from §5** after the post-objective gate; nothing in this
file authorises starting one automatically.

---

## 3. Capability register

| Capability | Status | Seam / location | Depends on | Validation | Verified at |
|---|---|---|---|---|---|
| Project/edit model, media library, additive schema v3 | done | `app/core/Project.*`, `MediaItem.*` | — | fixture | Phase 2 |
| Source playback (360 proxy, seek, viewpoint) | done | `app/application/Application.*` + `media/FfmpegFrameSource.*` | ffprobe frame rate | fixture + real-media | `94d8bb4` |
| Reframe plan / keyframe model | done | `app/reframe/ReframePlan.*`, `CameraKeyframe.*` | — | fixture | `bb17a1c` |
| Camera path (shortest-yaw, hold, FOV interpolation) | done | `app/reframe/CameraPath.*` | plan model | fixture | `bb17a1c` |
| Deterministic render + H.264 encode | done | `app/reframe/ReframeRenderer.*`, `ReframePipeline.*` | plan, decode | fixture + real-media | `bb17a1c` |
| Persistent render decoding (process per anchor) | done | `ReframeStreamFrameProvider.*` | ffprobe frame rate | fixture (byte-equal vs seek) | `56f7ac2` |
| NL intent grammar (directions, ranges, subjects, temporal, compound) | done | `app/reframe/ReframeIntent.*` | — | fixture | `aaae143`, `98b0413` |
| Target resolution (equirect geometry, views, tracker) | done | `app/target/{EquirectProjection,EquirectViewPlan,TargetResolver,SphericalTargetTracker}.*` | detector seam | fixture + real detector | `37703fa`, `c75d979` |
| Identity + deterministic selection ("me", ordinals, left/right) | done | `app/target/{TargetIdentity,TargetSelector}.*` | tracker | fixture | `b2ca847` |
| Appearance re-identification (optional seam) | done | `app/target/{Appearance*,IdentityReidentifier}.*` | external ReID helper | fixture (helper-gated) | `b7ee7fa` |
| Speaker evidence + association (optional seam) | done | `app/target/Speaker*` | external VAD helper | fixture (helper-gated) | `8ecc17b` |
| Follow-subject camera path | done | `TargetTrackPlanner::planTrack` | tracker, decoder | fixture + real-media follow | `39668ca` |
| Follow sampling density (250 ms / 24 cap) | done | `ReframeCommandRequest` follow budget | follow path | fixture | `99a7282` |
| Decoder reuse for trajectory resolution | done | `ReframeStreamFrameProvider` as default | follow path | fixture (process counting) | `c872dc3` |
| Follow trajectory smoothing | done | `TargetTrackPlanner` `smoothingWindow` | follow path | fixture | `0e90fc3` |
| Covering-view duplicate consolidation | done | `SphericalTargetTracker` footprint rule | tracker | fixture | `0144bfa` |
| Temporal editing (retain/remove/target-duration) | done | `TemporalEditPlan.*` + plan `segments` | probed duration | fixture + real-media | `aaae143` |
| Persisted, reproducible edit decisions | done | `app/reframe/EditDecision.*` | plan, source fingerprint | fixture + fresh-process replay | `bed4f97` |
| Decision provenance + immutable revision | done | `EditDecision` v2 + `Application` | decisions | fixture | `0902a16` |
| Intent -> plan contract checker (IPC-1..4) | done | `app/reframe/ReframeContract.*` | intent, plan | fixture | `dc42d82`, `d971009` |
| **Audio in rendered output** | done | `ReframeRenderer::encodeVideoWithAudio`, pipeline audio decision | plan segments, probe facts | **fixture only** | `7adf433` |
| **Lens / field-of-view control** | done | intent framing + builder/planner wiring + IPC-4 | intent, plan | **fixture only** | `d971009` |
| **Group framing (2-10 subjects)** | done | `TargetTrackPlanner::{planTracks,enclosingFramingDeg}` (exact basis), `ReframeSubjectGroup`/`subjectCount`, runner group resolution | two or more resolved tracks | **fixture only** | `5592118`, this checkpoint (Obj 31) |
| Persistent media analysis (artifact + runner) | partial | `app/analysis/{MediaAnalysis,MediaAnalysisRunner}.*` | detector / ffprobe | fixture; **no consumer** | `90f671c` |
| **Creator review (inspect the plan, accept or reject)** | done | `app/application/ReframePlanReview.*`, `Application` review API + preparer seam, `MainWindow` review panel | — | **fixture only** | this checkpoint (Obj 34) |
| **Creator modification visibility** (lineage marker in the render list, recorded notes in the provenance readout) | done | `MainWindow` list/readout, `DecisionProvenance::notes` | — | **fixture only** | this checkpoint (Obj 41) |
| **Creator lens widening** (constrained plan adjustment: monotone FOV increase only, no perception) | done | `app/reframe/ReframePlanAdjustment.*`, `Application::{widenRenderedLens,nextWiderLensFor}` | — | **fixture only** | this checkpoint (Obj 40) |
| **Project round-trip preservation** (an unreadable render record or edit decision is kept verbatim, never destroyed by open+save) | done | `Application::{reframeOutputsJson,restoreReframeOutputsFromJson}`, `ReframeCommandOutcome::rawRecord` | — | **fixture only** | this checkpoint (Obj 38) |
| **Non-destructive render destinations** (no render writes to a path a render record owns; derived names are fresh) | done | `Application::{defaultReframeOutputPath,recordHoldingOutputPath}` + `buildReframeCommandContext`/`acceptReframeReview` guards | — | **fixture only** | this checkpoint (Obj 37) |
| **Creator revision v1 (revise a rendered edit) + provenance/supersession readout** | done | `Application::{revisionOutputPath,reviseReframeOutput,revisionsOf,recordHoldingOutputPath}` over the Objective 17 mechanism, `MainWindow` revision/provenance controls | — | **fixture only** | `6cf1d1e` (Obj 35), this checkpoint (Obj 36) |
| Pre-render revision (revise a reviewed-but-unrendered plan) | blocked | `ReframePlanReview` + review API exist | a semantic decision: an unrendered plan has no persisted parent decision to point at (parentless `creator-revision` origin / plan-level parentage) | — | — |
| Structured / operation-level plan editing | deferred | — | own decision(s): Decision 058 permits ONLY monotone lens widening on a persisted plan; narrowing, aim/timing/segment/output edits and pre-render adjustment would each need their own proof or a re-plan | — | — |
| In-app audio playback | missing | — | audio-output subsystem decision (Decision 036) | — | — |
| Explicit subject sets ("keep me and person 2 in frame", "frame the presenter and the guest") | done | `ReframeSubjectGroup::ExplicitSet` + `subjectReferences` + runner reference resolution | the existing selector vocabulary | **fixture only** | this checkpoint (Obj 32) |
| Pair-path smoothing (containment-preserving) | missing | `TargetTrackPlanner::planTracks` | measured real-media jitter evidence | — | — |
| Framing offsets / composition (lead room, thirds) | deferred | — | decision redefining "centered" (Decision 046) | — | — |
| Automatic speaker / dialogue attribution | blocked | `SpeakerEvidenceProvider` seam ready | permissively licensed AV/diarization provider | — | — |
| Analysis -> Reasoning seam | blocked | Decision 039 defines the shape | app-level analysis action + defined reasoning semantics + evidence granularity | — | — |
| Phase 3 Obj 5 player lifecycle orchestration | deferred | `Player`/`FramePump` exist | 360 priority; own scoped objective | partial (fixture) | — |
| **Browser presentation/control layer** (headless backend process + thin HTTP control boundary) | missing | Decision 059 authorises the boundary; nothing exists — no server, socket, upload, serving, event or browser-asset code, and `QtNetwork` is not linked | the Decision 059 open questions that gate its shape (exposure boundary, authentication, project/session identity, media storage lifecycle, dependency authorisation) | — | — |
| **Render-output publication atomicity + identity-based destination comparison** | missing | `ReframeRenderer::encodeVideo`/`encodeVideoWithAudio` write to the final output path; `Application::recordHoldingOutputPath` compares `absoluteFilePath()` against canonical media paths | a decision (the first changes observable render behaviour; the second tightens Decisions 056/057) | — | — |

---

## 4. Validation debt (deliberately visible)

- **Objectives 28, 29, 30, 31 and 32 are fixture-validated only.** Their env-gated real-media counterparts have
  never run, because no real 360 clip / detector / speaker helper is configured in this environment.
  The suite reports **14 skips**: 8 environment-gated integrations from Objectives 3-19
  (`realDetectorIntegration`, `realSpeakerCommandIntegration`, `realSourcePlaybackIntegration`,
  `realReframePlaybackIntegration`, `realTemporalEditIntegration`, `realCompoundCommandIntegration`,
  `realUserCommandIntegration`, `realApplicationCommandIntegration`), the 5 Objective 28-32 harness
  tests below, and the child-only `replayFreshProcessChild` slot.
- **Objective 41 adds no validation debt either.** It presents stored facts in the creator surfaces and adds
  no perception, geometry or rendering behaviour.
- **Objective 40 adds no validation debt either.** The lens adjustment is a pure transformation over a plan
  plus application orchestration; it adds no perception, geometry or rendering behaviour, and its containment
  guarantee is asserted with the existing renderer-equivalent predicate.
- **Objective 38 adds no validation debt either.** Preservation is a persistence-path property verified by
  model-free project round-trip tests; it involves no perception or rendering behaviour.
- **Objective 37 adds no validation debt either.** The destination rule and the creator-workflow
  end-to-end coverage are application-level behaviour over injected seams; they add no perception or
  rendering behaviour, and the one behaviour change (repeated renders no longer overwrite) is fully covered
  by models-free tests.
- **Objective 36 adds no validation debt either.** It tightens the revision surface's refusals (a
  revision never targets the output path of a render record the application holds) and presents derived
  supersession; both are covered by model-free tests over injected executors, and neither adds perception
  or rendering behaviour.
- **Objective 35 adds no validation debt either.** Creator revision composes the already-validated
  Objective 17 mechanism (immutable decisions, lineage, replay) with the existing command pipeline, and
  the provenance readout only presents persisted facts; its validation level is fixture by design and it
  has no real-media counterpart to run.
- **Objective 34 adds no validation debt.** Creator review is application/UI orchestration over
  artifacts that are already validated (the plan, the decision, the render seam): it introduces no
  perception, no geometry and no rendering behaviour, so its validation level is fixture by design and
  it has no real-media counterpart to run.
- Specifically unverified on real footage: audio muxing against a real (mono) source track; lens
  control under real detector noise; group framing with real detection footprints, occlusion, dropped
  observations and crowded scenes (where an honest "cannot fit" refusal is expected but unmeasured).
- **Readiness assessment (2026-09-18, measured in this container): BLOCKED on two independent
  prerequisites.**
  1. **No media or runtimes exist here.** The documented assets are absent — `~/360_TEST_4K.mp4`,
     `~/.cache/reelcraft/media/` and `~/.cache/reelcraft/models/` do not exist — and a filesystem
     search found no video file larger than 20 MB anywhere. `numpy`, `cv2` and `onnxruntime` all fail
     to import (`ModuleNotFoundError`), and no model weights are present. Method and exact probes:
     `DEVELOPMENT_ENVIRONMENT.md` → "Real-media validation prerequisites in this container".
  2. **A harness for Objectives 28-32 now exists (Objective 33) but has never run.** The
     Objective 3-19 gated tests assert that era's behaviour only (rendered playback, temporal
     segments, a single-subject "follow person 1", source playback, project reopen); the new tests
     `realMediaAudioPreservation`, `realMediaLensRequestReachesOutput`,
     `realMediaMultiSubjectContainment`, `realMediaGroupInfeasibilityIsHonest` and
     `realMediaExplicitReferencesResolve` cover Objectives 28-32 and skip with a precise
     prerequisite message until the assets below are supplied. See
     `DEVELOPMENT_ENVIRONMENT.md` → "Invocation contract for the Objective 28-32 harness".
- **Minimum human action to unblock (the harness is now in place, so this is assets only):**
  (i) supply one real equirectangular 360 clip with audio and export `REELCRAFT_TARGET_CLIP` (a short
  1280x640 proxy extracted read-only from the original is sufficient and fast); (ii) install the
  detector runtime and weights (`python3-opencv` + `python3-numpy`; plus `yolox_2022nov.onnx` in
  `~/.cache/reelcraft/models/`; `onnxruntime` and `silero_vad.onnx` only for the speaker/appearance
  helpers, which this sweep does not need); (iii) run the five harness tests. Until then every one of
  them skips, and no validation claim may be made. Environment capability is otherwise adequate: ffmpeg 6.1.1
  with libx264/aac, 8 CPUs, ~1 TB RAM, ~9 GB free disk, no GPU (CPU inference is what the helpers
  expect).

---

## 5. Candidate next objectives (human selects)

| Candidate | Readiness | Prerequisite | Why it is a candidate |
|---|---|---|---|
| **Real-media validation sweep for Objectives 28-30** | ready (blocked on media) | a real 360 clip + optional detector/speaker helpers | closes the only validation debt in the current capability set |
| **Real-media validation sweep** (see §4) | ready once media exists | a real 360 clip + optional helpers | closes the only validation debt in the framing capability |
| **Containment-preserving group-path smoothing** | candidate | measured jitter evidence from the sweep above | quality refinement; evidence-poor today |
| **Structured plan adjustment (lens/time/aim on a reviewed plan, no perception)** | ready (needs scope + decision) | a decision authorising a plan-level editing path (the intent is not persisted, so only the plan can be adjusted) | the documented §10 "modify individual operations" control; makes revision instant instead of a full re-run |
| **Pre-render revision of a reviewed plan** | blocked on a decision | parentless creator-revision lineage semantics | continues the Objective 34 review loop; thin value until the lineage semantic exists |
| **Real-media validation of the creator-review path** | not applicable | — | not a candidate: Obj 34 adds no perception or rendering behaviour (see §4) |
| **Containment-preserving pair smoothing** | candidate | measured jitter evidence from the validation sweep | quality refinement; weak evidence today (see §6) |
| **Analysis -> Reasoning seam** | blocked | app-level analysis action, reasoning semantics, evidence granularity | the documented successor stage (Decision 039) |
| **In-app audio playback** | blocked | dependency decision (Decision 036 follow-up) | creator review of rendered audio |
| **Render-output publication atomicity + identity-based destination comparison** | ready (needs a decision) | a decision authorising a change to observable render behaviour and a tightening of Decisions 056/057 | makes "serve the rendered file" safe and closes a path-string-reachable route to overwriting a recorded render or the source media (Decision 059, Consequences) |
| **Web/backend boundary v1** (headless backend + thin HTTP control boundary) | blocked on decisions | the gating Decision 059 open questions: exposure boundary (local-only vs LAN/remote), authentication/authorization, project and session identity, media storage location/retention/lifecycle, and any new dependency | the authorised browser presentation/control layer — the largest remaining gap between the engine and a creator |

## 6. Deferred / blocked, with the missing prerequisite stated

- **Framing offsets** — needs a decision that changes what "centered" means (Decision 046). Not a
  tuning change.
- **Automatic speaker attribution** — needs a permissively licensed AV/diarization provider; real
  footage audio is mono, so spatial association is unavailable (Decision 024).
- **Analysis -> Reasoning** — needs (i) an application-level way to produce an analysis, (ii) an
  agreed statement of what the reasoner decides, (iii) evidence finer than the current 1 s sampling
  grid if it is to drive 250 ms follow commands.
- **Group sizes above ten, and dynamic group membership** — the vocabulary stops at ten named
  subjects and a group's membership is fixed for the instruction; "whoever is in frame at the time"
  would need its own semantics and perception evidence.
- **Explicit references beyond a phrase** — an explicit set resolves through the selector's existing
  vocabulary (creator aliases, ordinals, left/right, track ids, unique labels); a name matching several
  tracks is ambiguous and refused, and a reference longer than a phrase is not treated as a set.
- **Pair/group-path smoothing** — needs evidence. The pair aim is an average of two detections and the
  lens is constant, so the jitter profile is milder than the case that motivated Objective 26, and
  the pair path has never run on real footage. Measured 40°->4° step reduction justified Objective
  26; nothing equivalent exists here yet.
- **Browser presentation/control layer** — Decision 059 authorises the boundary and deliberately leaves
  thirteen questions open rather than settling them by default. Implementation may not begin until the
  gating ones are decided: exposure boundary (local-only vs LAN/remote), authentication/authorization,
  project and session identity, media storage location/retention/lifecycle, and any new dependency. The
  remaining open questions (event transport, preview mechanism, upload resumability, render
  progress/cancellation, review persistence, multi-user scope, concurrent desktop/backend ownership, audio
  playback) are listed in Decision 059 itself and do not all gate the first slice.
- **Render-output publication atomicity and identity-based destination comparison** — renders are currently
  written directly to their final path by FFmpeg with no atomic publication, and destination comparisons are
  path-string based rather than identity-based. Decision 059 makes both **contractual** once bytes are served
  over HTTP, but fixing them requires a decision first: the first changes observable render behaviour and the
  second tightens Decisions 056/057.

## 7. How work is authorised

- One **capability family + batch** (2-3 dependency-adjacent objectives) is approved by the human;
  see `AGENT_WORKFLOW.md` §12 for the gate that runs between objectives.
- Each objective keeps its own implementation, tests, documentation and **one Git commit**.
- The agent may make routine engineering decisions autonomously and must **not** silently select a
  new product direction, capability family, dependency or architectural change.

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

**Process Objective P1 — Development-management consolidation** (this checkpoint; documentation and
policy only, no product change).

**Objective 31 — N-way group framing** is complete at this checkpoint (Decision 052).

**Objective 32 — explicit multi-subject references** is the next objective of the currently approved
batch. It starts only after the batch gate (`AGENT_WORKFLOW.md` §12) confirms its prerequisites; if
the gate fails, work stops and the batch is re-authorised by the human.

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
| Creator review / revision UI | missing | Application APIs exist (`decisionProvenance`, `reviseEditDecision`) | — | — | — |
| In-app audio playback | missing | — | audio-output subsystem decision (Decision 036) | — | — |
| Explicit multi-subject references ("keep me and person 2 in frame") | missing | parser subject-set capture + the Objective 31 N-way resolution | Objective 31 (done) | — | — |
| Pair-path smoothing (containment-preserving) | missing | `TargetTrackPlanner::planTracks` | measured real-media jitter evidence | — | — |
| Framing offsets / composition (lead room, thirds) | deferred | — | decision redefining "centered" (Decision 046) | — | — |
| Automatic speaker / dialogue attribution | blocked | `SpeakerEvidenceProvider` seam ready | permissively licensed AV/diarization provider | — | — |
| Analysis -> Reasoning seam | blocked | Decision 039 defines the shape | app-level analysis action + defined reasoning semantics + evidence granularity | — | — |
| Phase 3 Obj 5 player lifecycle orchestration | deferred | `Player`/`FramePump` exist | 360 priority; own scoped objective | partial (fixture) | — |

---

## 4. Validation debt (deliberately visible)

- **Objectives 28, 29, 30 and 31 are fixture-validated only.** Their env-gated real-media counterparts have
  never run, because no real 360 clip / detector / speaker helper is configured in this environment.
  The suite reports **9 skips**: 8 environment-gated integrations
  (`realDetectorIntegration`, `realSpeakerCommandIntegration`, `realSourcePlaybackIntegration`,
  `realReframePlaybackIntegration`, `realTemporalEditIntegration`, `realCompoundCommandIntegration`,
  `realUserCommandIntegration`, `realApplicationCommandIntegration`) plus `replayFreshProcessChild`.
- Specifically unverified on real footage: audio muxing against a real (mono) source track; lens
  control under real detector noise; group framing with real detection footprints, occlusion, dropped
  observations and crowded scenes (where an honest "cannot fit" refusal is expected but unmeasured).
- Clearing it needs media/helpers, not code: set the `REELCRAFT_*` variables listed in
  `DEVELOPMENT_ENVIRONMENT.md` and run the gated tests. Treat this as the first validation step of
  the next product batch, not as a feature objective.

---

## 5. Candidate next objectives (human selects)

| Candidate | Readiness | Prerequisite | Why it is a candidate |
|---|---|---|---|
| **Real-media validation sweep for Objectives 28-30** | ready (blocked on media) | a real 360 clip + optional detector/speaker helpers | closes the only validation debt in the current capability set |
| **Explicit multi-subject references (Objective 32)** | ready (approved) | Objective 31's N-way resolution — done | names the subjects precisely instead of by group phrase |
| **Creator review / revision UI** | ready | — | makes persisted decisions and revisions usable without new pipeline work |
| **Containment-preserving pair smoothing** | candidate | measured jitter evidence from the validation sweep | quality refinement; weak evidence today (see §6) |
| **Analysis -> Reasoning seam** | blocked | app-level analysis action, reasoning semantics, evidence granularity | the documented successor stage (Decision 039) |
| **In-app audio playback** | blocked | dependency decision (Decision 036 follow-up) | creator review of rendered audio |

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
- **Pair/group-path smoothing** — needs evidence. The pair aim is an average of two detections and the
  lens is constant, so the jitter profile is milder than the case that motivated Objective 26, and
  the pair path has never run on real footage. Measured 40°->4° step reduction justified Objective
  26; nothing equivalent exists here yet.

## 7. How work is authorised

- One **capability family + batch** (2-3 dependency-adjacent objectives) is approved by the human;
  see `AGENT_WORKFLOW.md` §12 for the gate that runs between objectives.
- Each objective keeps its own implementation, tests, documentation and **one Git commit**.
- The agent may make routine engineering decisions autonomously and must **not** silently select a
  new product direction, capability family, dependency or architectural change.

# Reelcraft — Project History

## Purpose

This document records major milestones in the development of Reelcraft.

It provides chronological context for the project so future developers and AI agents do not need to reconstruct history from chat conversations.

---

# 2026-09-02 — Project Foundation

Reelcraft was established as an AI-native video editing platform focused on content creators, vloggers, and especially 360° video creators.

The long-term direction is to allow creators to provide footage and describe editing intent naturally while AI analyzes the media and produces structured editing decisions that the creator can review and control.

---

# 2026-09-02 — Documentation System Established

A persistent Markdown documentation system was established as the project's long-term source of truth.

The system is designed so future AI agents can continue development without depending on chat history.

Core documentation areas include:

- Master product vision
- Current project state
- Next development task
- AI handoff
- Architecture
- Architectural decisions
- Project history
- Development log
- Known issues
- Changelog
- Development environment

---

# 2026-09-02 — Initial Architecture Defined

The first formal technical architecture was documented.

Major subsystem boundaries were established for:

- User Interface
- Project / Edit Model
- Media Ingestion
- Camera / Media Adapters
- AI Orchestration
- AI Provider / Model Abstraction
- Deterministic Media Engine
- Rendering / Export
- Storage / Project Management

The core AI-to-media boundary was established as:

AI reasoning
 structured edit plan
 validation
 deterministic media execution

No implementation technology was permanently locked at this stage.

---

# 2026-09-02 — Controlled Development Workflow Established

The project adopted a controlled incremental development methodology.

The workflow requires:

1. One active development objective at a time.
2. Explicit scope.
3. Explicit non-goals.
4. Definition of Done.
5. Inspect-before-changing.
6. Incremental implementation.
7. Testing and regression verification.
8. Documentation updates.
9. Git review and checkpointing.
10. Stop-and-ask behavior when requirements or architecture conflict.

---

# 2026-09-02 — Git Repository Established

Git was initialized for the Reelcraft project.

The primary development branch was established as:

`main`

The first documentation foundation was committed as:

`b317aa1`

Commit message:

`docs: establish Reelcraft project foundation`

---

# 2026-09-02 — Architecture and Task Foundation Checkpointed

The following files were added and verified:

- `docs/ARCHITECTURE.md`
- `docs/NEXT_TASK.md`

They were committed with:

`docs: formalize architecture and development workflow`

This checkpoint established the formal architecture and active documentation objective.

---

# 2026-09-02 — Architectural Decision Record Established

`docs/DECISIONS.md` was created to preserve the reasoning behind significant architectural choices.

Initial decisions cover:

- AI/media execution separation
- Non-destructive editing
- First-class 360° support
- Camera adapters
- Replaceable AI providers
- Portable project state
- Human review authority
- Incremental development
- Inspect-before-changing
- Documentation as source of truth
- Evidence-based technology selection
- Controlled architecture evolution

Architectural changes must be documented rather than silently replacing previous decisions.

---

# 2026-09-02 — AI Handoff System Established

`docs/AI_HANDOFF.md` was created as the entry point for future AI development agents.

It establishes:

- Repository inspection requirements
- Documentation reading order
- Scope protection
- Testing expectations
- Git checkpoint requirements
- Architecture protection
- Stop-and-ask conditions
- Project-state verification
- Session handoff requirements

The goal is for a future AI agent to understand the project and continue work without reconstructing the previous conversation.

---

# Current Historical Position

Reelcraft has completed its initial project identity, architecture, documentation foundation, architectural decision record, and AI handoff system.

The application implementation has not yet begun.

The next stage is to complete the remaining project-control documentation and then define the first small implementation objective for Phase 1 — Foundation.

---

# Historical Recording Rule

Future meaningful milestones should be added chronologically.

Each entry should describe:

- What happened
- Why it mattered
- What changed
- What was verified
- What checkpoint or commit resulted

Do not rewrite history to make previous decisions appear as though they were always known.

If a major decision changes, preserve the previous historical record and document the reason for the change.

## 2026-09-02 — Product, Project, and AI Contracts Established

The conceptual foundation was expanded with three supporting contracts:

- `REQUIREMENTS.md` defines the product and system requirements while preserving technology flexibility.
- `PROJECT_MODEL.md` defines the conceptual structure and ownership of project state.
- `AI_EDIT_CONTRACT.md` defines the boundary between AI reasoning and deterministic media execution.

These documents reinforce the core architectural rule that AI determines intended edits while deterministic systems execute validated structured instructions.

The project also explicitly deferred creation of a substantial production application skeleton. Technology and runtime choices remain open and will be evaluated before implementation to reduce premature lock-in.

`NEXT_TASK.md` was updated to establish a focused technology/runtime evaluation as the next objective, and `CURRENT_STATE.md` / `DEVELOPMENT_LOG.md` were updated to reflect this transition.

This milestone does not represent production feature implementation.

## 2026-09-02 — Technology and Runtime Evaluation Completed

The focused technology and runtime evaluation was completed sufficiently to establish a validated technical direction for the first implementation stage.

The evaluation covered:

- Desktop runtime and UI approaches
- Deterministic media-processing integration
- AI integration and replaceable provider abstraction
- Voice interaction using the same intent architecture as text
- First-class 360° video requirements
- Local, cloud, and hybrid processing
- Large 4K/8K/360° media workloads
- Performance and GPU requirements
- Project state and storage
- Rendering and export
- Testing
- Packaging and deployment
- Integration, portability, dependency, licensing, vendor, and platform lock-in risks

The evaluation established several technology-agnostic requirements:

- Reelcraft requires a GPU-capable desktop application rendering layer capable of interactive 2D/3D content and future 360° presentation.
- The UI/application layer must remain separate from deterministic media processing.
- AI reasoning must produce structured, validated instructions rather than directly manipulating media.
- Original media must remain untouched.
- AI providers must remain replaceable.
- Voice and text should enter the same underlying intent architecture.
- Local, cloud, and hybrid processing must remain possible.
- Background processing must not unnecessarily block the interactive application.
- Project state must remain separate from original media and remain portable/recoverable.
- Automated testing and regression protection are required.

Qt 6, Electron, and Tauri 2 remain evaluated candidate approaches rather than permanent downstream commitments. Wails v3 remains a lower-priority/watchlist option because its current v3 status requires additional caution.

No production media engine, 360° editing pipeline, AI provider integration, voice editing system, or major application feature was implemented as part of this evaluation.

`DEVELOPMENT_LOG.md` records the detailed evaluation outcome. `CURRENT_STATE.md` now identifies validated technical direction as the current stage, and `NEXT_TASK.md` now defines the smallest Phase 1 application-foundation objective.

This milestone represents completion of the evaluation stage and transition into controlled Phase 1 foundation implementation planning. It does not represent completion of the Reelcraft application.

## 2026-09-06 — Phase 2 (360 Viewer Review & Navigation) Complete

Phase 2 was re-scoped by Decision 016 to "360 Viewer Review & Navigation": import and manage real media, select active media, deterministic single-frame preview, time stepping/seek, look-around orientation (keyboard and pointer), correct flat/equirectangular presentation, and consistent viewer/project state. Continuous/paced playback, duration-aware transport, and audio are explicitly deferred to a future playback/media-engine phase.

Objectives 1-15 were implemented and verified incrementally: viewer state/camera foundations, synthetic scene and equirectangular presentation (bilinear), media import/availability/library/active-media contracts, FFmpeg-CLI single-frame decode seam, time navigation, pointer orientation controls, projection declaration and flat routing, viewer-state consistency, and end-to-end real-media review-path validation.

Verification at closeout: 136 passed / 0 failed / 0 skipped; application and test builds succeed; offscreen smoke SMOKE_EXIT=124; clean-checkout verification of the closeout commit passes; working tree clean. Phase 2 formally closed 2026-09-06 (closeout commit; prior HEAD ae317f5). Phase 3 (Media Engine / playback decision) has not started.

## 2026-09-06 — Phase 3 Opening Decisions Recorded (Media Engine)

Phase 3 (Media Engine / continuous playback) was opened with a documentation-only decision objective after a read-only feasibility discovery was approved. Decision 017 records: opening architecture = persistent FFmpeg streaming subprocess behind a replaceable media/player seam (currently feasible path); FrameExtractor remains the deterministic single-frame preview/validation seam; linked FFmpeg libraries and QtMultimedia are deferred (no installation; revisit at a real desktop/deployment dependency decision); ffprobe duration metadata is reopened as its own future decision gate; the Objective 10 preview-time contract is preserved (playback extends it); and the architecture boundary is decode seam → player/timing → Application → Viewer, with timeline/editor/AI above Application. No media-engine/playback/duration/audio implementation exists. Phase 2 remains formally closed. Changes were documentation-only.

## 2026-09-16 — Phase 3 Objective 3: Media-Engine Foundation (Replaceable Source + Frame Pump)

Phase 3 continued with its first production media-engine foundation objective. An abstract, transport-agnostic decode/media-source seam (`FrameSource`) was introduced along with a persistent FFmpeg-subprocess implementation (`FfmpegFrameSource`: rawvideo/rgb24, caller-supplied geometry, deterministic bounded reads and process cleanup) and a synchronous, caller-driven `FramePump` that requests one frame per `advance()` and emits `frameReady`/`streamEnded`/`streamFailed`. No Application/UI playback, continuous/paced playback, duration metadata/ffprobe, audio, timeline, schema, or dependency changes were made; the Objective 10 preview-time contract is unchanged and `FrameExtractor` remains the single-frame preview/validation seam. The concrete interface, rawvideo transport, and pump mechanics remain implementation details under Decision 017 rather than new decisions. Verification: 144 passed / 0 failed / 0 skipped; application and test builds succeed; official build-and-test script green; offscreen smoke SMOKE_EXIT=124. Implemented and verified 2026-09-16.

## 2026-09-16 — Phase 3 Objective 4: Player/Timing Subsystem Foundation

Phase 3 added its second production foundation: the deterministic player/timing subsystem above the decode/media-source seam. `Playhead` provides clock-free position state (frame count, current index, presentation timestamp from an explicit frame interval); `Clock`/`SystemClock` and `PacingPolicy`/`DefaultPacingPolicy` are replaceable, injectable abstractions so playback is deterministic and tested without wall-clock delays; and `Player` is a `Stopped`/`Playing`/`Paused` state machine with `play()`/`pause()`/`stop()`, clock/pacing-driven `tick()`, explicit `stepOnce()`, and state/position/frame/end/error signals. The `FramePump` remains a passive, caller-driven decoder and is consumed only through its existing signals; the Player owns no timer or thread. No Application/UI playback wiring, continuous playback UX, duration metadata/ffprobe, audio, timeline, schema, or dependency changes were made, and the Objective 10 preview-time contract is unchanged. The design follows Decision 017, which already fixes the player/timing boundary and treats clock/pacing implementation as an implementation detail, so no new decision was recorded. Verification: clean application and test builds; 154 passed / 0 failed / 0 skipped (144 prior + 10 player/timing tests); official build-and-test script green; offscreen smoke SMOKE_EXIT=124. Implemented and verified 2026-09-16.



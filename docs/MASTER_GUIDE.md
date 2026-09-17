# Reelcraft — Master Guide

## 1. Project Identity
Reelcraft is an AI‑native video editing platform designed primarily for content creators, vloggers, and especially creators working with 360° video. The current development camera is an Insta360 X5, but the architecture must remain camera‑agnostic.

## 2. Ultimate Vision
The creator should eventually be able to provide raw footage and instruct Reelcraft in natural language. Examples:
- "Remove the boring parts."
- "Make a 60‑second TikTok from this."
- "Keep me centered."
- "Switch cameras when she starts talking."
- "Reframe this 360 footage toward the car."

The AI will understand the footage, understand the intent, create an editing plan, and execute it through deterministic media systems. The creator remains in control and can review, modify, or reject any AI decision.

## 3. Primary Creator Workflow
1. Creator imports raw footage (360°, multi‑cam, audio, etc.).
2. Reelcraft analyzes media: video, audio, dialogue, scenes, speakers, context.
3. Creator gives a natural‑language edit instruction (or uses manual tools).
4. AI interprets the instruction, creates a structured editing plan.
5. Deterministic media engine executes the plan (cuts, reframes, color, captions, etc.).
6. Creator reviews the result, accepts/rejects/modifies edits.
7. AI revises if needed.
8. Final render is produced in requested formats (16:9, 9:16, 360°, etc.).

## 4. Core Principles
1. **AI = decision maker; deterministic media engine = executor.**  
   AI determines *what* should happen; the media engine performs *how* to do it reliably.
2. **Non‑destructive editing.** Original footage is never altered. All edits are instructions.
3. **Structured editing model.** Edit decisions are represented as data (e.g., JSON), not hardcoded changes.
4. **360° is a first‑class capability.** Not treated as flat video.
5. **Camera‑agnostic.** Camera‑specific logic is isolated behind adapters.
6. **AI provider/model abstraction.** No permanent lock‑in to a single AI vendor.
7. **Local/cloud/hybrid processing.** Balance cost, performance, and privacy.
8. **Performance matters from day one.** 4K/8K/360° require proxy workflows and efficient rendering.
9. **Portability.** Project state is separate from the development environment.
10. **Privacy and security are architectural concerns.** User data must be protected.

## 5. High‑Level Architecture (Conceptual)
Reelcraft
├── User Interface (creator controls, timeline)
├── AI Orchestration (vision, language, transcription, etc.)
├── Project / Edit Model (structured edit data)
├── Media Engine (video/audio processing, 360° rendering)
└── Export / Rendering

The AI orchestration layer uses multiple specialized models (vision, speech, language) but is abstracted so providers can be swapped. The media engine uses tools like FFmpeg but hides low‑level details behind an abstraction.

## 6. Documentation System (AI Memory)
The project's persistent memory lives in Markdown files, not chat history. The core files are:
- `MASTER_GUIDE.md` – this file.
- `CURRENT_STATE.md` – what exists right now.
- `NEXT_TASK.md` – the exact next development objective.
- `AI_HANDOFF.md` – information for the next AI agent.
- `PROJECT_HISTORY.md` – major milestones and decisions.
- `DEVELOPMENT_LOG.md` – chronological engineering journal.
- `KNOWN_ISSUES.md` – bugs and limitations.
- `DECISIONS.md` – important architectural decisions and reasons.
- `CHANGELOG.md` – user‑facing changes.
- `ARCHITECTURE.md` – formal technical architecture.
- `DEVELOPMENT_ENVIRONMENT.md` – how to set up the dev environment.
- Subsystem docs under `docs/systems/`.

**Golden Rule:** Never rely solely on AI chat history. Documentation is the source of truth.

## 7. Development Rules for AI Agents
1. **Inspect before changing.** Never assume a feature exists.
2. **Protect existing work.** Don't delete or rewrite working code without reason.
3. **Create a recoverable checkpoint before risky changes.**
4. **Make small, incremental changes.** Test after each step.
5. **Document successes AND failures.** Failed attempts are valuable.
6. **Never claim completion without verification.**
7. **Stop and ask when encountering major ambiguity or architectural conflict.**
8. **Maintain the documentation after every session.**

## 8. Definition of Done
A development task is considered **done** only when:
- The code has been implemented according to the agreed scope.
- The change has been tested (unit, integration, and manual where appropriate).
- Existing functionality has not regressed (regression tests pass).
- Documentation has been updated (if applicable).
- A checkpoint/commit has been created.
- The result has been verified by the developer or by an automated verification step.

## 9. Roadmap (Phased Development)
- **Phase 1 – Foundation:** Basic app architecture, project structure.
- **Phase 2 – 360 Viewer Review & Navigation:** import and manage real media, select active media, deterministic single-frame preview, time stepping/seek, look-around orientation (keyboard and pointer controls), correct flat/equirectangular presentation, and consistent viewer/project state. *(Scope redefined 2026-09-06 — continuous/paced playback, duration-aware playback transport, and audio are explicitly deferred to a future playback/media-engine phase; see DECISIONS.md Decision 016.)*
- **Phase 3 – Media Engine:** Reliable video processing/rendering.
- **Phase 4 – Timeline Intelligence:** Transcript and timeline understanding.
- **Phase 5 – AI Analysis:** Scene, speaker, object, and content analysis.
- **Phase 6 – AI Editing:** Automated cutting and edit decisions.
- **Phase 7 – B‑roll and Dialogue Intelligence:** Semantic B‑roll and speaker‑aware editing.
- **Phase 8 – AI 360° Reframing:** Automatic virtual‑camera control. *(A deterministic 360 reframing engine — structured plan, camera path, renderer, and natural-language intent boundary — was implemented and verified 2026-09-17; automatic scene/target understanding is the next step.)*
- **Phase 9 – AI Finishing:** Audio, color, captions, graphics, animations.
- **Phase 10 – Conversational AI Editor:** Natural‑language editing and personalization.

*The roadmap may change as development reveals better approaches.*

## 10. Golden Rules Summary
- The actual project is the source of truth, not assumptions.
- AI is a decision maker, not the entire media engine.
- Editing is structured and non‑destructive.
- Original media is sacred.
- Human control is final.
- Documentation is persistent memory.
- Build incrementally, test constantly.

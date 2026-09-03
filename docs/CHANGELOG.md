# Reelcraft — Changelog

All notable Reelcraft project changes should be recorded here.

This changelog focuses on meaningful project-level changes and user-visible capabilities. Internal development details belong in DEVELOPMENT_LOG.md.

---

# [0.1.0] — 2026-09-02

## Added

- Established the Reelcraft project identity and product vision.
- Established the persistent project documentation system.
- Established the controlled AI development workflow.
- Established the initial technical architecture.
- Established architectural decision tracking.
- Established AI handoff documentation.
- Established project history tracking.
- Established development logging.
- Established known-issue tracking.

## Project Status

Reelcraft is currently in the documentation and architecture foundation stage.

No production application functionality has been released yet.

---

# Changelog Rules

Future entries should:

- Use the appropriate project version.
- Include the date of the change.
- Describe meaningful user-facing or project-level changes.
- Avoid claiming functionality that has not been implemented and verified.
- Keep internal implementation details primarily in DEVELOPMENT_LOG.md.
- Preserve historical entries rather than rewriting them.

## 0.1.1 — Product and AI Architecture Contracts

### Added

- Product requirements contract in `REQUIREMENTS.md`.
- Conceptual project model in `PROJECT_MODEL.md`.
- AI-to-deterministic editing contract in `AI_EDIT_CONTRACT.md`.
- Technology/runtime evaluation task in `NEXT_TASK.md`.

### Updated

- `CURRENT_STATE.md` now reflects the transition toward validated technical direction.
- `DEVELOPMENT_LOG.md` records the new product and AI architecture foundation.
- `PROJECT_HISTORY.md` records the milestone chronologically.

### Architectural Clarification

The project now explicitly preserves the boundary:

    AI reasoning → Structured Edit Plan → Validation → Deterministic Media Execution

AI output is untrusted until validated. Original media remains protected, 360° remains a first-class capability, camera-specific behavior remains behind adapters, and voice/text share the same intent architecture.

### Deferred

A substantial production application skeleton remains intentionally deferred until the technology/runtime evaluation provides sufficient evidence for a safe implementation direction.

No production video editor, media engine, AI provider integration, voice system, or 360° editing system was implemented in this milestone.

## v0.1.2 — Technology Direction Validated

Date: 2026-09-02

- Completed the focused technology and runtime evaluation.
- Established validated requirements for the desktop application rendering layer.
- Confirmed the separation between UI/application systems, AI reasoning, and deterministic media execution.
- Confirmed first-class 360° requirements without prematurely implementing the 360° pipeline.
- Preserved replaceable AI providers and local/cloud/hybrid processing options.
- Preserved non-destructive media handling and portable project-state requirements.
- Established automated testing and regression protection as Phase 1 requirements.
- Kept Qt 6, Electron, and Tauri 2 as evaluated candidates rather than permanent technology commitments.
- Kept Wails v3 as a lower-priority/watchlist option.
- Transitioned the project from technology-direction evaluation to controlled Phase 1 application-foundation planning.
- No major editor feature or production media pipeline was implemented in this milestone.

## v0.1.3 — Phase 1 Desktop Application Foundation Complete
Date: 2026-09-03

- Added minimal Qt 6 desktop application shell with UI/application-core separation
- Added Project create/identify/persist/reopen/close lifecycle
- Added non-destructive JSON project storage
- Added background task demonstration with QtConcurrent
- Added automated Qt Test suite (9 tests)
- Added error handling for invalid project save/open paths
- Preserved architecture boundaries; no production editor/media/AI systems implemented

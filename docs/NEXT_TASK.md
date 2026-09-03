# Reelcraft — Next Task

## Objective

Establish the smallest working, testable Reelcraft desktop application foundation using the validated technical direction, while preserving strict separation between the UI/application layer, project state, future AI orchestration, and deterministic media execution.

This is a foundation task, not a major editor-feature implementation.

## Why This Is the Next Task

The technology/runtime evaluation is complete enough to establish a validated technical direction for Phase 1.

The project now needs a minimal executable foundation that proves the core application boundaries and development workflow before larger editor capabilities are introduced.

## In Scope

- Select and establish the desktop runtime/UI foundation for Phase 1 based on the validated technical direction.
- Create a minimal Reelcraft application shell.
- Establish a clear UI/application-core boundary.
- Establish minimal project lifecycle: create, identify, persist, reopen, and close.
- Establish a safe boundary for future deterministic media processing.
- Establish background work that does not block the UI.
- Establish basic cross-boundary error handling.
- Establish the initial automated test foundation.
- Establish a reproducible build and run workflow.
- Verify alignment with ARCHITECTURE.md and AI_EDIT_CONTRACT.md.

## Explicitly Out of Scope

- Full video editor
- Full timeline
- Production media engine
- Production video playback
- 360° viewer or reframing
- AI editing
- Production AI provider integration
- Voice editing
- B-roll intelligence
- Scene detection
- Speaker detection
- Captions
- Color grading
- Effects system
- Production rendering/export
- Cloud infrastructure
- Complete production project schema
- Insta360 X5-specific implementation
- Camera-specific media adapters
- GPU-optimized production video pipeline
- Plugin architecture
- Production packaging/distribution

## Implementation Constraints

- Maintain one active development objective.
- Inspect before changing.
- Protect existing work.
- Make small logical changes.
- Test after meaningful changes.
- Perform regression testing.
- Use Git checkpoints before and after meaningful or risky changes.
- Keep experiments separated from stable work.
- Update persistent documentation.
- Never claim completion without verification.
- Stop when requirements or architecture conflict.
- AI reasoning and deterministic media execution remain separate.
- Original media must never be modified by the application foundation.
- Project state remains separate from original media.
- 360° remains first-class architecturally but is not implemented in Phase 1.
- Camera-specific behavior remains behind adapters.
- AI providers remain replaceable.
- Voice and text remain compatible with the same intent architecture.
- Avoid premature full-editor schema design.
- Avoid unnecessary dependencies.
- Keep the UI responsive and background work nonblocking.
- Material technology decisions must be recorded in DECISIONS.md.

## Definition of Done

Phase 1 is complete only when:

1. The selected Phase 1 desktop runtime/UI foundation builds successfully.
2. Reelcraft launches successfully in the supported test environment.
3. A basic UI shell renders reliably.
4. UI-to-application/core communication is demonstrated.
5. A minimal project can be created.
6. Minimal project state can be persisted.
7. A persisted project can be reopened successfully.
8. Original media is never modified.
9. Background work executes without freezing the UI.
10. At least one expected failure/error path is handled safely.
11. Automated tests execute successfully.
12. Regression checks pass.
13. The implementation respects ARCHITECTURE.md and AI_EDIT_CONTRACT.md.
14. The build/run workflow is reproducible.
15. Relevant documentation is updated.
16. A verified Git checkpoint exists.
17. git status is clean.
18. Completion is supported by actual verification.

## Verification Plan

- Build the application from the documented development environment.
- Launch it and verify the UI shell.
- Create, persist, close, and reopen a minimal project.
- Exercise UI-to-core communication.
- Exercise background work and verify UI responsiveness.
- Exercise an invalid or failure condition.
- If a media fixture is used, verify that original media is not modified.
- Run automated tests.
- Run regression checks.
- Inspect the final diff.
- Run git diff --check.
- Create the Git checkpoint and verify a clean working tree.

## Success Criteria

Move the project from:

Documentation + validated technical direction

to:

Validated technical direction + minimal executable application foundation

without implementing major editing features or prematurely locking downstream architecture.

## Stop Conditions

Stop and ask for clarification if:

- Requirements conflict with architecture.
- A technology choice requires an unsupported major architectural commitment.
- Phase 1 expands into a major editor feature.
- A dependency introduces significant unnecessary lock-in.
- A change could modify or overwrite original media.
- The AI-to-deterministic boundary would be bypassed.
- Verification cannot establish whether the implementation works.
- The current environment cannot support required verification and no safe alternative exists.

## Next Task After Completion

After Phase 1 is fully implemented, tested, verified, documented, and checkpointed, define the next smallest logical development objective.

Do not automatically begin a major feature.

The next objective must be established from the evidence produced by Phase 1 and documented before implementation begins.

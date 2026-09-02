# Reelcraft — Next Task

## Objective

Complete the initial Reelcraft documentation and architecture foundation so that development can proceed from a clear, persistent, version-controlled technical plan.

## Current Context

Reelcraft is currently at version 0.1.0.

The project contains no application implementation yet. The existing documentation establishes the product vision, core principles, conceptual architecture, documentation system, development rules, definition of done, and phased roadmap.

Git has been initialized on the `main` branch and the initial documentation foundation has been committed.

## Scope

This task is limited to establishing the next layer of project documentation and architecture planning.

The work should:

1. Formalize the technical architecture described conceptually in `MASTER_GUIDE.md`.
2. Establish the persistent documentation files required by the Master Guide.
3. Define major subsystems and their responsibilities.
4. Record important architectural decisions and their reasoning.
5. Establish the development environment requirements.
6. Ensure another AI agent can understand the project state and continue development without relying on chat history.

## Non-Goals

This task must NOT:

- Implement application features.
- Build the user interface.
- Implement the media engine.
- Install unnecessary application dependencies.
- Process real video.
- Modify or delete original media.
- Perform large-scale refactoring.
- Lock Reelcraft to a specific AI provider.
- Treat any specific camera as the permanent architecture.
- Skip testing or verification.

## Definition of Done

This task is complete only when:

- The required documentation structure exists.
- `ARCHITECTURE.md` formally describes the major system components and their relationships.
- Subsystem documentation exists where appropriate.
- `DECISIONS.md` records major architectural decisions.
- `DEVELOPMENT_ENVIRONMENT.md` documents the development environment.
- `AI_HANDOFF.md` accurately describes how another AI should continue the project.
- `PROJECT_HISTORY.md` records the project's major milestones.
- `DEVELOPMENT_LOG.md` records the work performed.
- `KNOWN_ISSUES.md` accurately records known limitations.
- `CHANGELOG.md` records user-facing project changes.
- `CURRENT_STATE.md` accurately reflects the resulting state.
- `NEXT_TASK.md` identifies the next development objective.
- Documentation is internally consistent.
- Git status is clean after the completed checkpoint.
- A Git checkpoint is created after verification.

## Verification

Before declaring this task complete:

1. Review all created documentation.
2. Check for contradictions between documentation files.
3. Verify file paths and names.
4. Verify that no application source code was unintentionally created.
5. Run appropriate documentation/file-structure checks.
6. Review the Git diff.
7. Commit the verified changes.

## Development Rule

Work incrementally.

Do not create the entire architecture blindly in one uncontrolled operation. Establish one logical documentation component at a time, inspect the result, and update the project state before moving to the next component.

## Next Step After This Task

Once the documentation and architecture foundation is verified and checkpointed, define the first implementation task for Phase 1 — Foundation.

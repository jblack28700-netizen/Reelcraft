# Reelcraft — Next Task

## Objective

Establish the minimal technical foundation for Reelcraft application development without implementing user-facing editing features.

The first implementation objective is to create a minimal, testable application skeleton and establish the project's initial validation structure.

## Why This Is the Next Task

The documentation and architecture foundation is now established.

Before building major Reelcraft features such as the 360° viewer, timeline, AI analysis, or editing engine, the project needs a small executable foundation that can be tested and expanded safely.

This task establishes that foundation without prematurely locking the project into a large application architecture.

## Scope

The task should establish:

1. A minimal application entry point.
2. A minimal project/runtime structure consistent with ARCHITECTURE.md.
3. A basic health/startup path that proves the application can initialize.
4. A minimal automated test structure.
5. Basic project validation.
6. Documentation describing the implemented foundation.
7. Git checkpointing before and after the implementation.

## Non-Goals

This task must NOT:

- Build the full user interface.
- Build the timeline editor.
- Implement video editing.
- Implement the media engine.
- Process real video.
- Implement AI editing.
- Implement AI providers.
- Implement 360° viewing.
- Implement 360° reframing.
- Lock the project to a specific AI provider.
- Lock the project to a specific camera.
- Add unnecessary dependencies.
- Modify original media.
- Perform large-scale refactoring.
- Combine multiple major features into one task.

## Required Process

Before making implementation changes:

1. Inspect the existing repository.
2. Read MASTER_GUIDE.md.
3. Read ARCHITECTURE.md.
4. Read AI_HANDOFF.md.
5. Read DEVELOPMENT_ENVIRONMENT.md.
6. Check Git status.
7. Create or confirm a safe checkpoint.
8. Identify the smallest implementation step.

During implementation:

1. Make one logical change at a time.
2. Verify each change.
3. Run automated tests where available.
4. Run regression checks.
5. Keep the implementation within this task's scope.
6. Stop if a major architectural decision becomes necessary.

After implementation:

1. Run the complete available test suite.
2. Perform basic application startup/health verification.
3. Review the resulting files.
4. Update CURRENT_STATE.md.
5. Update DEVELOPMENT_LOG.md.
6. Update CHANGELOG.md if the change is user-visible.
7. Update KNOWN_ISSUES.md if a new limitation is discovered.
8. Review the Git diff.
9. Create a verified Git checkpoint.
10. Confirm the working tree is clean.

## Definition of Done

This task is complete only when:

- A minimal application entry point exists.
- The application can initialize successfully.
- A basic health/startup verification succeeds.
- Automated test infrastructure exists at a minimal useful level.
- Tests pass.
- No existing project functionality is regressed.
- No original media is modified.
- No unnecessary dependencies are introduced.
- Documentation accurately describes the implemented foundation.
- CURRENT_STATE.md reflects the new state.
- DEVELOPMENT_LOG.md records the work.
- Git contains a verified checkpoint.
- `git status` is clean.

## Stop-and-Ask Conditions

Stop before implementation and request clarification if:

- The chosen application runtime conflicts with the documented architecture.
- A technology decision would permanently constrain future architecture.
- A dependency is required whose purpose is unclear.
- The task requires implementing a major subsystem.
- Requirements conflict between project documents.
- Existing files contain unexpected implementation that changes the scope.
- The implementation would require modifying original media.
- A test failure cannot be explained safely.
- The correct architecture cannot be determined from the existing documentation.

## Success Criteria

The project should move from:

    Documentation-only foundation

to:

    Minimal executable and testable foundation

while preserving the architectural flexibility required for future Reelcraft development.

## Next Task After Completion

After this objective is verified and checkpointed, define the next small Phase 1 task based on what was learned from the foundation implementation.

Do not automatically begin the next feature.

The project must return to the controlled workflow:

    Inspect
      ↓
    Define Objective
      ↓
    Define Scope
      ↓
    Define Definition of Done
      ↓
    Implement Small Change
      ↓
    Test
      ↓
    Verify
      ↓
    Document
      ↓
    Git Checkpoint
      ↓
    Define Next Objective

# Reelcraft — AI Development Handoff

## Purpose

This document is the entry point for any AI agent continuing development of Reelcraft.

An AI agent must read this document before making project changes.

This document does not replace the other project documentation. It tells the agent which documents establish the project's source of truth and how they should be used.

---

# 1. First Rule

Do not assume the current state of Reelcraft from chat history.

The repository documentation and actual project state are authoritative.

Before making changes:

1. Inspect the current Git status.
2. Read `MASTER_GUIDE.md`.
3. Read `CURRENT_STATE.md`.
4. Read `NEXT_TASK.md`.
5. Read `ARCHITECTURE.md`.
6. Read relevant decisions in `DECISIONS.md`.
7. Inspect the actual files relevant to the task.
8. Confirm the active development objective.

---

# 2. Project Identity

Reelcraft is an AI-native video editing platform designed primarily for:

- Content creators
- Vloggers
- 360° video creators
- Social-video creators
- Other creators who want AI-assisted editing

The long-term goal is to allow creators to provide footage and describe what they want in natural language while retaining meaningful control over the resulting edit.

---

# 3. Current Development Stage

The project has moved well past the documentation-foundation stage. A working Qt 6 desktop shell, project/media model, deterministic single-frame review path, and a Phase 3 media-engine foundation (replaceable media-source seam, frame pump, player/timing subsystem) exist and are verified.

The **current human-approved priority is the 360 editing/reframing capability**: 360 source -> scene/target understanding -> request interpretation -> structured reframe plan -> virtual-camera decisions -> deterministic execution -> flat video output.

A deterministic 360 reframing vertical slice is implemented and verified under `app/reframe/` (Decision 018): structured `ReframePlan`/`CameraKeyframe`, pure `CameraPath`, replaceable `ReframeFrameProvider`, deterministic `ReframeRenderer`, a deterministic natural-language `ReframeIntent` boundary, and an end-to-end `ReframePipeline`. The automated suite is 180 passed / 0 failed / 0 skipped.

The Phase 3 Objective 5 (Application-level player lifecycle) entry in `NEXT_TASK.md` remains valid history but must not be started mechanically while the 360 priority is active.

Target/subject resolution is also implemented under `app/target/` (Decision 019): a replaceable `TargetDetector` seam (with a dependency-free subprocess adapter), pure 360 geometry, deterministic tangent-view coverage, a deterministic spherical tracker, and a resolver/track-planner that produce `ReframeTarget`s and `ReframePlan`s for the existing engine. A real detector now runs through that seam: `tools/detector_helper/` implements the `ProcessTargetDetector` protocol with OpenCV Zoo YOLOX (Apache-2.0) + OpenCV DNN on CPU, and real detection -> tracking -> plan -> flat render was verified on real 360 footage (Decision 020). The helper/model are optional and external; the normal unit suite is model-free and skips the real integration test unless configured. Structured creator identity and deterministic selection are implemented (Decision 021): `TargetIdentityRegistry` binds "me" to a creator-selected track and `TargetSelector` resolves multi-person references deterministically, feeding the existing planner/renderer. "Me" is geometric identity, not biometric: it does not re-identify after long absence or among similar people, and it reports unresolved/ambiguous rather than guessing. Speaker/active-speaker association, appearance re-identification, GPU inference, and UI integration remain future objectives.

Do not assume that planned architecture has already been implemented; read `CURRENT_STATE.md` and `NEXT_TASK.md` for exact status.

---

# 4. Current Development Camera

The current development camera is the Insta360 X5.

This is a development target, not a permanent architectural dependency.

Reelcraft must remain camera-agnostic through adapter boundaries.

Do not redesign the core architecture around the X5 alone.

---

# 5. Core Architectural Rule

The most important system boundary is:

AI reasoning
 structured edit plan
 validation
 deterministic media execution

AI decides what should happen.

The deterministic media system performs the actual media operations.

AI must not directly modify source media.

---

# 6. Source Media Protection

Original media is sacred.

Normal Reelcraft editing operations must be non-destructive.

Do not:

- Overwrite source footage
- Delete source footage
- Modify original media as part of an edit
- Assume generated files are safe replacements for originals

Use project/edit instructions, derived media, proxies, and renders as separate artifacts.

If a task could affect original media, stop and verify the requirement before proceeding.

---

# 7. One Active Objective

Only one meaningful development objective should be active at a time.

Every task must define:

- Objective
- Scope
- Non-goals
- Definition of Done
- Verification requirements

Do not combine unrelated features or refactors into the same task.

If the requested change conflicts with the active objective, stop and resolve the conflict before proceeding.

---

# 8. Inspect Before Changing

Before modifying an existing file:

1. Read the relevant file.
2. Inspect related files when necessary.
3. Inspect Git status.
4. Determine whether the file is already part of working functionality.
5. Understand the current behavior.
6. Identify the smallest safe change.

Never overwrite existing work based solely on assumptions.

---

# 9. Scope Protection

Do not make unrelated changes.

If a task says to implement one component, do not automatically:

- Redesign other components
- Upgrade unrelated dependencies
- Reformat unrelated files
- Refactor unrelated code
- Replace working technology
- Add speculative features

If an unrelated problem blocks the active task, document it and determine whether it should become a separate task.

---

# 10. Definition of Done

Do not claim a task is complete merely because code was written.

A task is complete only when:

- The requested implementation exists.
- The implementation matches the defined scope.
- Relevant tests pass.
- Existing functionality has not regressed.
- Appropriate manual verification has been performed.
- Documentation is updated.
- Git state has been reviewed.
- A checkpoint exists when required.

---

# 11. Testing Requirements

Testing should be proportional to the change.

Where applicable, perform:

- Unit testing
- Integration testing
- Regression testing
- Manual functional testing
- Error-path testing
- Performance testing

Media-related changes should be tested with representative media rather than assumed to work.

AI-related changes should be tested for malformed, unexpected, and incomplete model output.

---

# 12. Git and Checkpoints

Git is part of the development safety system.

Before meaningful or risky changes:

- Inspect Git status.
- Create or confirm a recoverable checkpoint when appropriate.

After meaningful changes:

- Review the diff.
- Verify the result.
- Commit the verified state.

Do not create a commit merely to hide an unverified or broken state.

Keep commits understandable and logically scoped.

---

# 13. Documentation Requirements

The following documents form the persistent project-control system:

- `MASTER_GUIDE.md`
- `CURRENT_STATE.md`
- `NEXT_TASK.md`
- `AI_HANDOFF.md`
- `PROJECT_HISTORY.md`
- `DEVELOPMENT_LOG.md`
- `KNOWN_ISSUES.md`
- `DECISIONS.md`
- `CHANGELOG.md`
- `ARCHITECTURE.md`
- `DEVELOPMENT_ENVIRONMENT.md`

Relevant documentation must be updated when project state changes.

---

# 14. Architecture Protection

Do not make major architectural changes casually.

If implementation reveals that the architecture needs to change:

1. Identify the existing decision.
2. Explain why it needs reconsideration.
3. Document the evidence.
4. Record the new decision.
5. Identify affected systems.
6. Update architecture documentation.
7. Test affected functionality when implementation exists.
8. Record the change in project history.
9. Create a Git checkpoint.

Never silently overwrite architectural history.

---

# 15. Technology Selection

Do not treat technology candidates in `ARCHITECTURE.md` as permanent commitments.

The project currently intentionally leaves several choices open, including:

- UI framework
- Application runtime
- Project file format
- Storage implementation
- AI providers
- AI models
- Local/cloud workload allocation
- Rendering architecture
- GPU strategy
- Database requirements
- Plugin architecture
- Packaging/deployment

Technology should be selected based on actual requirements, testing, performance, compatibility, maintainability, privacy, and cost.

---

# 16. AI Provider Independence

Do not build core project logic directly around one AI provider.

Provider-specific code should remain behind an abstraction boundary.

The project should remain capable of supporting different models and providers where practical.

---

# 17. 360° Requirements

360° video is a first-class capability.

Do not implement 360° functionality as an afterthought if the affected subsystem establishes foundational media behavior.

The architecture must eventually support concepts including:

- Equirectangular media
- 360° metadata
- Orientation
- Viewer orientation
- Virtual-camera positioning
- Reframing
- Keyframed viewpoints
- 360° export
- Flat-video reframing

---

# 18. Media Engine Boundary

The deterministic media engine is responsible for executing validated edit instructions.

AI agents should not bypass this boundary by directly manipulating media as a substitute for implementing the proper edit pipeline.

Tools such as FFmpeg may eventually be used internally, but higher-level Reelcraft components should communicate through defined media-engine interfaces.

---

# 19. Error Handling

AI output must be treated as untrusted structured input.

Before execution, validate:

- Schema
- Media references
- Timeline ranges
- Supported operations
- Required dependencies
- 360° parameters
- Rendering requirements

Invalid instructions should be rejected or corrected before execution.

---

# 20. Performance

Do not optimize blindly.

When performance becomes relevant:

1. Measure the current behavior.
2. Identify the bottleneck.
3. Make the smallest justified change.
4. Measure again.
5. Verify that correctness has not regressed.

Proxy workflows, GPU acceleration, caching, parallelism, and cloud processing should be introduced based on evidence and requirements.

---

# 21. Privacy and Security

Treat creator media and project information as potentially sensitive.

Do not expose source media, credentials, tokens, or private project information unnecessarily.

When implementing cloud AI or remote processing, explicitly consider:

- What data leaves the device
- Where it is processed
- How long it is retained
- What provider receives it
- What credentials are required
- What artifacts remain after processing

---

# 22. Current Task Procedure

When beginning a new task:

1. Read the project state.
2. Identify `NEXT_TASK.md`.
3. Confirm the objective.
4. Define or confirm the Definition of Done.
5. Inspect relevant implementation.
6. Plan the smallest logical change.
7. Implement incrementally.
8. Test after meaningful steps.
9. Perform regression checks.
10. Update documentation.
11. Review Git diff/status.
12. Create the appropriate checkpoint.

---

# 23. Stop-and-Ask Conditions

An AI agent must stop rather than guess when:

- Requirements are contradictory.
- A requested change conflicts with the architecture.
- The correct behavior is materially ambiguous.
- A change could destroy or overwrite original media.
- A change would require a major architectural decision that has not been evaluated.
- Existing functionality would need to be removed without explicit justification.
- A dependency or technology choice would create significant lock-in.
- Tests reveal an unexpected regression that cannot be safely resolved within scope.
- The task would expand substantially beyond its defined scope.

When stopping, clearly explain the conflict and identify the decision that needs to be made.

---

# 24. How to Determine Where to Continue

The AI agent should determine the current position using:

1. `CURRENT_STATE.md`
2. `NEXT_TASK.md`
3. Git status/history
4. Relevant architecture and decision documents
5. Actual project files

Do not infer progress from the existence of a planned document or feature.

A feature is implemented only when the repository contains the implementation and verification supports that conclusion.

---

# 25. Handoff Completion

Before ending a development session, update the persistent state so another agent can continue.

At minimum, ensure:

- Current task is accurate.
- Current state is accurate.
- Recent work is documented.
- Known problems are recorded.
- Relevant architectural decisions are recorded.
- Git state is understandable.
- The next logical objective is identified.

The next AI should be able to begin by reading the repository documentation rather than reconstructing the previous conversation.

---

# 26. Authority Order

When information conflicts, use this general order of authority:

1. Actual repository state
2. Verified test results
3. Current project documentation
4. Recorded architectural decisions
5. Current task definition
6. Previous development history
7. Chat instructions or assumptions

If the conflict cannot be resolved safely, stop and ask.

---

# 27. Final Principle

Reelcraft should be built deliberately.

The objective is not merely to produce code quickly.

The objective is to create a reliable, extensible, AI-native editing platform whose architecture, project state, and development history remain understandable to both humans and future AI agents.

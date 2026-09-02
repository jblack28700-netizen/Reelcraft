# Reelcraft — Next Task

## Objective

Evaluate and establish a safe technical direction for Reelcraft application development before beginning production application implementation.

The purpose of this task is to evaluate the runtime, UI, media-processing, AI, voice, 360°, storage, performance, packaging, and deployment approaches required by the documented product requirements and architecture.

This task is an evaluation task, not a production implementation task.

---

## Why This Is the Next Task

The documentation and conceptual architecture foundation is established.

The project now has:

- Product requirements
- Conceptual project model
- AI-to-deterministic editing contract
- High-level architecture
- Controlled AI development workflow
- Persistent project state and history
- Development environment documentation

However, several implementation-level technology decisions intentionally remain unresolved.

The project should therefore evaluate those decisions before creating a production application skeleton. This prevents the initial implementation from prematurely locking Reelcraft into a runtime, UI framework, media architecture, AI provider, rendering strategy, or deployment model that later conflicts with the product requirements.

---

## Scope

The evaluation should examine the technical approaches needed to support:

1. Desktop-first application development.
2. Creator-facing UI and interaction.
3. AI orchestration and provider abstraction.
4. Voice interaction using the same intent architecture as text.
5. Deterministic media processing.
6. 360° playback, processing, reframing, and export.
7. Camera/media adapters.
8. Local, cloud, and hybrid processing.
9. Performance and proxy workflows for large media.
10. GPU acceleration.
11. Project storage and persistence.
12. Project portability and recovery.
13. Rendering and export.
14. Automated testing.
15. Security and privacy.
16. Plugin/extension architecture.
17. Packaging and deployment.
18. Future extensibility.

Candidate technologies and approaches must be compared against the actual Reelcraft requirements rather than selected merely because they are familiar, popular, or convenient.

---

## Evaluation Rules

The evaluation must:

- Use REQUIREMENTS.md as the product requirements source.
- Use ARCHITECTURE.md as the conceptual architecture source.
- Use PROJECT_MODEL.md for project-state requirements.
- Use AI_EDIT_CONTRACT.md for the AI-to-deterministic boundary.
- Use MASTER_GUIDE.md and AI_HANDOFF.md for project principles and development rules.
- Separate verified facts from assumptions.
- Identify integration risks.
- Identify performance risks.
- Identify portability risks.
- Identify packaging and deployment risks.
- Identify dependency and licensing risks where relevant.
- Identify vendor or platform lock-in risks.
- Consider the needs of 360° media as a first-class capability.
- Consider online AI capabilities and local/cloud/hybrid processing.
- Consider voice interaction.
- Consider large 4K/8K/360° media workloads.
- Preserve the separation between AI reasoning and deterministic media execution.
- Avoid making technology choices that unnecessarily constrain future architecture.

No technology should become a permanent architectural commitment merely because it is evaluated or prototyped.

---

## Required Process

Before beginning the evaluation:

1. Inspect the repository.
2. Read MASTER_GUIDE.md.
3. Read REQUIREMENTS.md.
4. Read PROJECT_MODEL.md.
5. Read AI_EDIT_CONTRACT.md.
6. Read ARCHITECTURE.md.
7. Read DECISIONS.md.
8. Read AI_HANDOFF.md.
9. Read DEVELOPMENT_ENVIRONMENT.md.
10. Check Git status.
11. Confirm a safe checkpoint exists before making meaningful changes.

During evaluation:

1. Evaluate one technical area at a time.
2. Compare relevant candidate approaches.
3. Record evidence and assumptions.
4. Identify advantages and limitations.
5. Identify architectural consequences.
6. Avoid unnecessary dependencies or production implementation.
7. Stop if requirements conflict or evidence is insufficient.
8. Record material architectural conclusions in DECISIONS.md.

After evaluation:

1. Review the evaluation conclusions.
2. Identify which decisions are safe to establish.
3. Identify which decisions must remain technology-agnostic.
4. Update ARCHITECTURE.md if material architectural conclusions changed.
5. Update DECISIONS.md for significant decisions.
6. Update CURRENT_STATE.md.
7. Update DEVELOPMENT_LOG.md.
8. Update CHANGELOG.md when appropriate.
9. Update KNOWN_ISSUES.md if new limitations are discovered.
10. Review the complete Git diff.
11. Run documentation and repository verification checks.
12. Create a verified Git checkpoint.
13. Confirm the working tree is clean.

---

## Explicit Non-Goals

This task must NOT:

- Build the full UI.
- Build the timeline editor.
- Implement video editing.
- Implement the media engine.
- Process real project media.
- Implement AI editing.
- Select a production AI provider without sufficient evaluation.
- Implement production AI models.
- Implement voice editing.
- Implement the 360° viewer.
- Implement 360° reframing.
- Implement production rendering.
- Build production cloud infrastructure.
- Modify original media.
- Create a large application skeleton before the technology direction is sufficiently understood.
- Add unnecessary dependencies.
- Perform unrelated refactoring.
- Make technology commitments based only on familiarity.
- Replace the conceptual project model with a premature implementation schema.
- Bypass the AI-to-deterministic validation boundary.

---

## Evaluation Areas

The evaluation should explicitly address the following areas.

### Desktop Application Runtime

Evaluate approaches for building a desktop-first Reelcraft application.

Consider:

- UI capabilities
- Native system integration
- Performance
- Media handling
- GPU access
- Packaging
- Cross-platform portability
- Development complexity
- Long-term maintainability

### Media Processing

Evaluate how Reelcraft should integrate deterministic media processing.

Consider:

- FFmpeg or equivalent processing systems
- Process isolation
- Preview generation
- Rendering
- Error handling
- Large-media workflows
- Proxy generation
- Hardware acceleration

### AI Integration

Evaluate how Reelcraft should integrate AI capabilities.

Consider:

- Provider abstraction
- Multiple model types
- Local models
- Cloud APIs
- Structured output
- Validation
- Model replacement
- Cost
- Privacy
- Latency
- Failure handling

### Voice

Evaluate voice as an input mechanism to the same intent-processing system used by text.

Consider:

- Speech recognition
- Transcription
- Intent extraction
- Local versus cloud processing
- Privacy
- Latency
- Error correction

### 360° Media

Evaluate technical requirements for first-class 360° support.

Consider:

- Equirectangular media
- Metadata
- Orientation
- Playback
- Virtual-camera control
- Reframing
- Keyframes
- Field of view
- Flat-video output
- 360° output
- GPU requirements

### Project and Storage

Evaluate approaches for storing:

- Project metadata
- Source references
- Analysis metadata
- AI decisions
- User decisions
- Edit state
- Proxies
- Generated assets
- Render information
- History and recovery information

The project model must remain independent of the storage implementation.

### Performance and GPU

Evaluate:

- Proxy workflows
- Background processing
- Hardware acceleration
- GPU requirements
- Memory requirements
- Large project handling
- 4K/8K/360° workloads

Performance assumptions should be validated where practical rather than treated as facts.

### Packaging and Deployment

Evaluate:

- Development builds
- Production packaging
- Installation
- Updates
- Platform support
- Native dependencies
- Media-processing dependencies
- AI dependencies
- Distribution constraints

### Testing

Evaluate the testing strategy needed to support:

- Project model validation
- AI edit-plan validation
- Media-processing correctness
- Regression testing
- Performance testing
- Failure/recovery testing
- Cross-platform behavior where applicable

---

## Stop-and-Ask Conditions

Stop before making a technology commitment if:

- Requirements conflict with the candidate architecture.
- No candidate can satisfy a core requirement without an unacceptable compromise.
- A technology choice would permanently constrain a core future capability.
- A dependency has unclear purpose, licensing, security, or maintenance implications.
- Evidence is insufficient for an important decision.
- The evaluation requires implementing a major subsystem.
- A major architectural boundary would need to be abandoned.
- The decision would create significant vendor or platform lock-in without sufficient justification.

When uncertain, preserve flexibility rather than guessing.

---

## Definition of Done

This task is complete only when:

- Desktop-first application approaches have been evaluated.
- Relevant runtime/UI approaches have been evaluated.
- Media-processing approaches have been evaluated.
- AI integration approaches have been evaluated.
- Voice integration has been evaluated.
- 360° requirements have been evaluated.
- Local/cloud/hybrid processing has been evaluated.
- Performance and GPU considerations have been evaluated.
- Project/storage approaches have been evaluated.
- Rendering/export approaches have been evaluated.
- Testing requirements have been evaluated.
- Packaging/deployment approaches have been evaluated.
- Important integration and lock-in risks have been identified.
- Technology-specific decisions are explicitly documented.
- Technology-agnostic decisions are explicitly identified.
- Significant architectural decisions are recorded in DECISIONS.md.
- No premature production application implementation has been performed.
- CURRENT_STATE.md accurately reflects the resulting state.
- DEVELOPMENT_LOG.md records the evaluation.
- Relevant architecture documentation is updated.
- Repository verification succeeds.
- A verified Git checkpoint exists.
- `git status` is clean.

---

## Success Criteria

The project should move from:

    Documentation + conceptual architecture

to:

    Documentation + validated technical direction

while preserving the architectural flexibility required for future Reelcraft development.

The result should provide enough evidence to define the smallest safe Phase 1 implementation objective.

---

## Next Task After Completion

After this evaluation is verified and checkpointed, define the smallest Phase 1 implementation objective based on the technical direction and evidence discovered.

Do not automatically begin implementation of a major feature.

Return to the controlled workflow:

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

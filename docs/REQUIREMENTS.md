# Reelcraft — Product Requirements

## 1. Purpose

This document defines the product-level requirements for Reelcraft.

It describes what Reelcraft is intended to accomplish and the capabilities the finished platform must support. It does not prescribe a specific programming language, desktop framework, AI provider, database, rendering implementation, or hardware configuration unless explicitly stated as a product requirement.

Requirements in this document are part of the persistent project source of truth.

---

## 2. Product Scope

Reelcraft is an AI-native video editing platform designed primarily for content creators, vloggers, and especially creators working with 360° video.

The core product experience is:

1. Import media.
2. Understand the media through automated analysis.
3. Understand the creator's editing intent through text, voice, and direct interaction.
4. Convert that intent into a structured editing plan.
5. Validate the plan.
6. Execute the plan through deterministic media systems.
7. Present the result for creator review.
8. Allow the creator to revise the result.
9. Render/export the finished video.

Reelcraft should reduce the amount of manual editing required without removing meaningful creative control from the creator.

---

## 3. Target Platforms

### 3.1 Primary Platform

The initial product is desktop-first.

The architecture must support a capable desktop editing environment suitable for:

- Large video files
- Long-form projects
- 360° media
- Timeline-based editing
- AI-assisted workflows
- High-resolution preview and rendering
- Large project storage
- Hardware acceleration where available

### 3.2 Online Requirement

Reelcraft's full AI experience requires network connectivity because AI capabilities may depend on remote models and services.

The architecture should nevertheless permit local processing where practical.

### 3.3 Offline Capability

Offline/local operation is desirable where technically and economically practical.

The system should be designed so that local processing can be added or used without redesigning the core project and editing model.

---

## 4. Core Editing Requirements

Reelcraft must ultimately support:

- Media import
- Media organization
- Timeline editing
- Trimming
- Cutting
- Splitting
- Reordering
- Audio editing
- Transitions
- Text and titles
- Captions/subtitles
- Effects
- Color adjustments
- Speed changes
- B-roll insertion
- Dialogue-focused editing
- Scene-based editing
- Multi-format output
- Non-destructive editing
- Preview before final rendering
- Revision after AI-generated edits

Editing operations must operate on project/edit data rather than destructively modifying original source media.

---

## 5. AI Requirements

AI is a central product capability, not an optional add-on.

Reelcraft must be capable of using AI to:

- Analyze imported video
- Analyze audio
- Transcribe speech
- Identify speakers where technically possible
- Detect scenes and meaningful segments
- Understand visual context
- Understand dialogue and narrative context
- Identify potentially useful B-roll
- Suggest edits
- Generate structured editing plans
- Apply creator instructions
- Revise previous editing decisions
- Assist with captions
- Assist with color/finishing decisions
- Assist with framing and reframing
- Assist with 360° editing
- Support increasingly complex natural-language editing workflows

AI should reason about the project and desired result.

AI should not be responsible for directly performing arbitrary low-level media manipulation.

AI decisions must cross into the deterministic media system through a structured and validated interface.

---

## 6. Natural-Language Editing

Creators should be able to describe desired edits using natural language.

Examples of intended capabilities include requests such as:

- Remove pauses and unnecessary silence.
- Make the video more engaging.
- Cut repetitive sections.
- Create a short version from the long video.
- Find useful B-roll.
- Reframe the 360° footage toward the speaker.
- Create a vertical version.
- Improve pacing.
- Add captions.
- Focus the edit on the strongest moments.

The examples above are illustrative rather than a complete command list.

The system should translate natural-language intent into structured editing operations rather than relying on uncontrolled direct execution.

---

## 7. Voice Interaction

Voice interaction is a first-class product capability.

Reelcraft should allow creators to communicate editing intent through spoken commands.

Voice input should ultimately use the same underlying intent and AI orchestration system as text input.

Conceptual flow:

Voice
  |
Text / Intent Understanding
  |
AI Orchestration
  |
Structured Edit Plan
  |
Validation
  |
Deterministic Media Engine

Voice should not become a separate editing architecture.

The system should support conversational revision such as:

- Asking what was changed
- Requesting changes
- Rejecting changes
- Refining previous instructions
- Asking the system to explain an editing decision

---

## 8. 360° Video Requirements

360° video is a first-class media type.

Reelcraft must be architecturally capable of supporting:

- 360° source identification
- Equirectangular media
- 360° metadata
- 360° preview
- Interactive viewing
- Viewer orientation
- Virtual-camera positioning
- Reframing
- Keyframed viewpoints
- Subject-following reframing
- Speaker-focused reframing
- Flat-video output from 360° source
- 360° output where appropriate
- Multiple output aspect ratios

360° functionality must not be treated as a later conversion layer bolted onto a conventional flat-video editor.

---

## 9. Media and Camera Requirements

Reelcraft must be designed around media capabilities rather than one permanent camera model.

The current development camera is the Insta360 X5, but the architecture must remain camera-agnostic.

Camera-specific behavior should be isolated behind media/camera adapters.

The system should be capable of expanding support for future cameras and formats without requiring fundamental changes to the editing architecture.

Media ingestion should be capable of identifying relevant properties such as:

- Resolution
- Frame rate
- Codec
- Container
- Audio characteristics
- Color information
- Orientation
- 360° metadata
- Camera metadata
- Duration
- Media dimensions

Unsupported or partially supported media should be detected and reported clearly rather than failing unpredictably.

---

## 10. Project and File Requirements

A Reelcraft project must preserve the relationship between:

- Original media
- Imported media
- Media metadata
- Analysis results
- AI decisions
- Creator decisions
- Edit state
- Generated assets
- Proxies
- Previews
- Rendered outputs
- Export information

Original source media must remain protected.

The project must be non-destructive.

Large media assets should not unnecessarily be embedded inside project metadata.

Projects should ultimately be portable enough that the editing state can be understood and reconstructed independently of a single AI provider.

---

## 11. Creator Control and Editing History

The creator remains the final authority over the edit.

Reelcraft must support:

- Review of AI-generated changes
- Acceptance or rejection of changes
- Manual correction
- Revision of AI instructions
- Undo
- Redo
- Editing history
- Recovery from failed operations
- Preservation of meaningful project state

AI-generated decisions must not silently become irreversible modifications to original media.

The architecture should allow future support for more advanced versioning and branching of project states.

---

## 12. Export Requirements

Reelcraft must ultimately support common creator-oriented output workflows.

The export system should be capable of producing appropriate outputs for:

- Long-form video
- Vertical short-form video
- Square or other common formats where useful
- Standard flat video
- 360° video where supported
- Multiple resolutions
- Multiple frame rates where technically appropriate

Export settings should be represented as structured project/output data rather than being hard-coded into individual editing features.

---

## 13. Performance and Scalability

Reelcraft must be designed for large media workloads.

The architecture should support:

- Proxy media
- Background processing
- Incremental analysis
- Caching
- Efficient preview generation
- Hardware acceleration where available
- Long-running jobs
- Progress reporting
- Resource-aware processing
- Recovery from interrupted processing

The system should avoid requiring full-resolution processing when a lower-cost representation is sufficient for a task.

Performance targets should be established empirically as implementation progresses rather than invented without measurement.

---

## 14. Privacy and Security

Reelcraft must treat creator media and project information as sensitive user data.

The architecture should provide clear boundaries around:

- Original media
- Project metadata
- AI analysis
- Cloud processing
- Local processing
- Authentication
- Credentials
- Temporary/generated data

Users should be able to understand when media or project information is sent to remote services.

Secrets and provider credentials must not be stored in project files or source code.

---

## 15. Reliability and Recovery

Reelcraft must be designed to fail safely.

Long-running operations should be represented as recoverable jobs where appropriate.

The system should provide meaningful handling for:

- Failed imports
- Unsupported media
- Failed AI requests
- Network interruptions
- Interrupted rendering
- Missing generated assets
- Corrupted or incomplete intermediate data
- Insufficient storage
- Resource exhaustion

A failed operation should not silently corrupt the project or original media.

---

## 16. Extensibility

The architecture must allow future expansion without requiring fundamental redesign.

Areas intended for extensibility include:

- AI providers
- AI models
- Camera/media adapters
- Media formats
- Editing operations
- Export formats
- Local processing
- Cloud processing
- Hardware acceleration
- Plugins or integrations where appropriate

Specific implementation mechanisms remain open until technology evaluation.

---

## 17. Future and Long-Term Capabilities

The long-term vision may include capabilities such as:

- Fully conversational editing
- Story-aware editing
- Automatic long-form video creation
- Automatic short-form versions
- Intelligent B-roll selection
- Speaker-aware editing
- Advanced 360° reframing
- Automatic captions
- Automated finishing
- Creator-style learning and preferences
- AI-assisted titles and descriptions
- Thumbnail assistance
- Cross-platform content preparation
- More advanced local AI processing
- Hybrid local/cloud workflows

These capabilities describe the direction of the product and do not imply that they must be implemented in the first release.

---

## 18. Explicit Non-Requirements for Early Development

The following are not required during the initial foundation stages:

- Complete video editor
- Complete timeline system
- Full AI editing
- Complete 360° editor
- Production-grade cloud infrastructure
- Support for every camera
- Support for every codec
- Final AI provider selection
- Final GPU architecture
- Final packaging/distribution system
- Advanced creator analytics
- Automated social publishing
- Every future AI capability

Early development must prioritize a small, testable foundation over breadth.

---

## 19. Technology-Agnostic Requirements

The following product requirements must remain independent of specific implementation technologies:

- AI provider
- AI model
- Desktop UI framework
- Programming language
- Database
- Storage provider
- Rendering implementation
- Hardware acceleration implementation
- Cloud provider
- Local AI runtime

Technology decisions may be made later through explicit evaluation and recorded in the project decision history.

---

## 20. Requirement Change Policy

Requirements may evolve as Reelcraft is developed.

A significant requirement change must:

1. Identify the existing requirement.
2. Explain why it is changing.
3. Record the reason and evidence.
4. Identify affected architecture and systems.
5. Update related documentation.
6. Preserve historical reasoning in `DECISIONS.md` and/or `PROJECT_HISTORY.md`.
7. Verify that implementation plans remain consistent.

Requirements must not be silently changed to justify an implementation that already exists.

---

## 21. Relationship to Other Project Documents

This document defines product requirements.

Related documents have different responsibilities:

- `MASTER_GUIDE.md` — overall project identity, vision, principles, and development rules.
- `ARCHITECTURE.md` — system architecture and boundaries.
- `DECISIONS.md` — significant architectural and project decisions.
- `CURRENT_STATE.md` — current project condition.
- `NEXT_TASK.md` — the single current development objective.
- `AI_HANDOFF.md` — instructions for future AI agents continuing the project.
- `PROJECT_MODEL.md` — definition of the project's internal conceptual/data model.
- `AI_EDIT_CONTRACT.md` — contract between AI reasoning and deterministic editing systems.

This separation prevents requirements, architecture, implementation status, and temporary tasks from becoming mixed together.

---

## 22. Current Status

Status: Product requirements foundation established.

This document describes the intended product direction and required capabilities.

It does not mean those capabilities are currently implemented.

Current implementation status remains governed by `CURRENT_STATE.md`.

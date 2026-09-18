# Reelcraft — Technical Architecture

## 1. Purpose

This document defines the technical architecture of Reelcraft at the planning level.

It describes system boundaries, responsibilities, data flow, and architectural constraints without prematurely committing the implementation to specific frameworks, vendors, or infrastructure.

The architecture must support the long-term Reelcraft vision described in MASTER_GUIDE.md.

## 2. Architectural Principles

1. AI decides what should happen.
2. Deterministic systems execute the decisions.
3. Editing is represented as structured data.
4. Original media is never modified by editing operations.
5. 360° video is a first-class media type.
6. Camera-specific behavior is isolated behind adapters.
7. AI providers are replaceable.
8. Local, cloud, and hybrid processing are supported conceptually.
9. Heavy media operations should use efficient proxy workflows where appropriate.
10. Human review remains the final authority.
11. Project state must remain portable and independent of the development environment.
12. Security and privacy must be considered at every system boundary.

## 3. High-Level System

Reelcraft consists conceptually of:

- User Interface
- Project / Edit Model
- Media Ingestion
- Camera / Media Adapters
- AI Orchestration
- AI Provider / Model Abstraction
- Deterministic Media Engine
- Rendering / Export
- Storage / Project Management

The major relationship is:

Creator
  ↓
User Interface
  ↓
Project / Edit Model
  ↓
AI Orchestration
  ↓
Structured Edit Plan
  ↓
Validation
  ↓
Deterministic Media Engine
  ↓
Preview / Render / Export

AI determines what should happen. The deterministic media engine performs the actual media operations.

## 4. User Interface

The User Interface is responsible for creator interaction.

Eventually it should provide:

- Media import
- Project management
- Timeline interaction
- Playback
- 360° viewing and orientation control
- Natural-language editing instructions
- AI edit-plan review
- Manual adjustment of AI decisions
- Preview
- Export configuration
- Progress and error reporting

The UI must not contain the core media-processing implementation.

## 5. Project / Edit Model

The Project / Edit Model is the central representation of a Reelcraft project.

It should eventually represent:

- Project metadata
- Source media references
- Media metadata
- Proxy references
- Timeline structure
- Clips
- Cuts
- Transitions
- Audio decisions
- Captions
- Color adjustments
- Graphics
- 360° reframing instructions
- AI-generated decisions
- Human modifications
- Export configuration

The exact schema is intentionally not finalized yet.

Editing must be non-destructive. Source media remains unchanged while edit instructions are stored separately.

## 6. Media Ingestion

Media ingestion accepts supported media and prepares it for downstream processing.

Responsibilities include:

- Importing media
- Detecting media properties
- Identifying media type
- Extracting technical metadata
- Identifying 360° characteristics
- Creating or scheduling proxy generation
- Registering source media in the project

Camera-specific behavior must remain isolated from the core system.

The Insta360 X5 is the current development camera but is not a permanent architectural dependency.

## 7. Camera / Media Adapters

Adapters isolate camera-specific and format-specific behavior.

An adapter may eventually handle:

- Camera metadata
- Stitching-related information
- 360° metadata
- Proprietary media characteristics
- Camera-specific import requirements
- Device-specific processing requirements

Adding another camera should not require rewriting the core media system.

## 8. AI Orchestration

The AI Orchestration subsystem coordinates AI capabilities.

Potential capabilities include:

- Natural-language understanding
- Video understanding
- Scene detection
- Object detection
- Speaker identification
- Speech transcription
- Semantic search
- Content understanding
- Edit-plan generation
- Edit-plan revision

AI systems produce structured decisions.

They do not directly manipulate source media.

## 9. AI Provider / Model Abstraction

AI capabilities must be accessed through replaceable provider/model interfaces.

The architecture should support:

- Local models
- Cloud APIs
- Multiple providers
- Different models for different tasks
- Hybrid processing

No specific AI provider is permanently selected at this stage.

## 10. Deterministic Media Engine

The Media Engine executes validated structured edit instructions.

Potential responsibilities include:

- Cutting
- Joining
- Audio processing
- Transcoding
- Proxy generation
- Color processing
- Caption rendering
- Graphics rendering
- 360° processing
- Reframing
- Preview generation
- Final rendering

Tools such as FFmpeg may be used internally, but other Reelcraft components should communicate with the media engine through higher-level interfaces.

## 11. AI-to-Media Boundary

The AI-to-media boundary is a core architectural constraint.

The intended flow is:

Raw Media
  ↓
Analysis
  ↓
AI Reasoning
  ↓
Structured Edit Plan
  ↓
Validation
  ↓
Deterministic Media Engine
  ↓
Preview / Render

AI must not bypass the structured edit model to directly alter media.

## 12. 360° Video

360° video is a first-class capability.

The architecture must support:

- 360° source identification
- Equirectangular media
- Orientation metadata
- Viewer orientation
- Virtual-camera positioning
- Reframing
- Keyframed viewpoint changes
- 360° export
- Flat-video reframing from 360° sources

360° functionality must not be treated merely as ordinary flat-video processing.

## 13. Storage and Project Portability

Project state must remain separate from the development environment.

A project should eventually be portable between supported environments.

Conceptually:

Project
├── Project metadata
├── Edit model
├── Source references
├── Analysis metadata
├── AI decisions
├── User decisions
├── Proxy references
└── Render/export information

Large source and generated media may be stored separately from lightweight project metadata.

The exact storage implementation remains undecided.

## 14. Processing Model

Reelcraft should support three conceptual processing modes.

### Local

Processing occurs primarily on the creator's device or local computer.

### Cloud

Processing occurs on remote infrastructure.

### Hybrid

Different workloads are assigned to local or cloud processing according to:

- Performance
- Cost
- Privacy
- Hardware capability
- Media size
- Model availability

The project/edit model should remain independent of where processing occurs.

## 15. Security and Privacy

Security and privacy are architectural requirements.

Future implementation must consider:

- Access control
- Project isolation
- Secure media handling
- Credential protection
- AI provider data policies
- Local versus cloud processing
- Temporary processing artifacts
- Export security
- Sensitive creator data

Specific security mechanisms will be defined during implementation.

## 16. Error Handling and Validation

Structured edit plans must be validated before execution.

Validation should eventually detect:

- Invalid media references
- Invalid timeline ranges
- Unsupported operations
- Missing dependencies
- Invalid 360° parameters
- Rendering failures
- AI output schema violations

AI-generated instructions must not be trusted blindly.

Malformed or invalid instructions should be rejected or corrected before reaching the media engine.

## 17. Observability

The system should eventually make it possible to determine:

- What operation was requested
- What AI decision was produced
- What edit plan was generated
- What media operation was executed
- Whether execution succeeded
- Why an operation failed

This supports debugging, creator trust, and reproducibility.

## 18. Extensibility

The architecture should allow future expansion into:

- Additional cameras
- Additional media formats
- Multi-camera workflows
- Advanced audio workflows
- New AI providers
- New AI models
- New export formats
- Additional creator platforms
- Automated B-roll
- Advanced dialogue editing
- AI 360° cinematography

New capabilities should be introduced through defined subsystem boundaries rather than tightly coupled changes.

## 19. Technology Decisions

The following are candidates rather than permanent commitments:

- FFmpeg for media processing
- Web-based or native UI technologies
- Local AI models
- Cloud AI APIs
- Local/cloud storage systems

Specific technology choices require separate evaluation before becoming permanent dependencies.

## 20. Current Architectural Unknowns

The following remain intentionally unresolved:

- Final UI framework
- Final application runtime
- Project file format/schema
- Storage implementation
- AI provider selection
- Model selection
- Local versus cloud workload allocation
- Rendering architecture
- GPU acceleration strategy
- Database requirements
- Plugin/extension architecture
- Packaging and deployment strategy

These decisions should be evaluated as implementation requirements become clearer.

## 21. Architecture Evolution

This document is a living architectural document.

Architecture may change when testing, implementation experience, performance measurements, security requirements, or creator workflow requirements reveal a better approach.

Significant architectural changes must be recorded in DECISIONS.md and reflected in project history.

## Platform Boundary Constraint

Desktop is the first-class supported platform for the current Reelcraft stage.

Android is a planned future platform, not a current product target. Termux/Ubuntu on Android is a development environment only and does not constitute native Android support.

To preserve future portability without prematurely restructuring the Phase 1 desktop foundation:

- Keep project state and application orchestration independent of QtWidgets; QtCore is acceptable.
- Keep UI-specific concerns, including file dialogs and windowing, outside the core application and project model.
- Prefer cross-platform Qt APIs such as QFile, QStandardPaths, QJsonDocument, and QUuid.
- Do not introduce X11/Wayland-specific or Android-specific behavior into project or application layers.
- Preserve the deterministic media engine and AI orchestration as platform-independent concepts.
- Maintain the boundary where the UI supplies paths/data rather than core code invoking platform dialogs directly.


## Expanded Apple Platform Portability Constraint — 2026-09-03

Desktop remains first-class now. macOS is a planned future desktop platform. iOS and iPadOS are planned future considerations, not committed targets.

To preserve future Apple portability without premature implementation:

- Keep project state and application orchestration independent of QtWidgets.
- Keep UI file/path selection outside core application and project model.
- Keep deterministic media engine, AI orchestration, GPU strategy, storage, permissions, background processing, and hardware acceleration platform-neutral.
- Treat macOS, iOS, and iPadOS requirements as future platform-specific adapters, not core assumptions.
- Do not introduce Linux, X11, Wayland, Android, macOS, iOS, or iPadOS-specific behavior into core layers.

## 22. Media Analysis Subsystem — 2026-09-18 (Objective 21)

The analysis stage the workflow always assumed now exists. It sits **beside** the AI-to-media boundary, not inside it: it produces evidence that reasoning may consume, and it never produces an edit.

    Source Media
      |
      v
    Media Analysis  (MediaAnalysisRunner)  -->  MediaAnalysis artifact (persisted beside the project)
      |                                              |
      |  capability layers (technical, targets, ...)  |  referenced by Project::analysisRefs
      v                                              v
    Project Context --> Creator Intent --> AI Reasoning --> Structured Edit Plan --> Deterministic Execution

- **`app/analysis/MediaAnalysis.{h,cpp}`** — a versioned artifact with its own schema gate (independent of the Project schema), a source reference and fingerprint, a three-way source status, an analysis-specification identity, and a deterministic `analysisId`. Its **envelope** is deliberately small and closed: addressing, provenance, lifecycle, coverage and confidence. Capability-specific data lives in **independently versioned layers**, so a new capability is a new layer kind rather than a new envelope field.
- **`app/analysis/MediaAnalysisRunner.{h,cpp}`** — one whole-video pass: a single persistent decoder at a configured **perception resolution**, strictly sequential decoding with interval sampling, and a persisted perception/sampling specification. It composes the existing `TargetResolver`, `SphericalTargetTracker` and equirect view coverage unchanged.
- **Layering.** `core/MediaSourceReference` holds the source-reference and status vocabulary shared with `EditDecision`. `analysis/` depends on `core/`, `media/` and `target/`; nothing depends on `analysis/` except the application layer, and no deterministic execution path does.
- **Replay isolation (Decision 040).** Analysis is never on the deterministic replay path. `replayEditDecision()` consumes `EditDecision::plan()` and nothing else; deleting every analysis artifact leaves rendering, replay and project loading fully functional.
- **Degradation is a status.** Artifact missing, unreadable, mismatched, stale, source moved or gone are reported conditions, never project corruption.
- Deliberately not present: transcript, diarization, scene segmentation, quality/salience, B-roll reasoning, LLM reasoning, Creator Memory, embeddings.


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


---

## 23. Seam index (stable boundaries)

Compact map of the stable architectural boundaries: where each lives, what it guarantees, and who
consumes it. This is an index, not a replacement for the sections above or for `DECISIONS.md`.

| Seam | Implementation | Guarantee | Major consumers |
|---|---|---|---|
| Project / edit model | `app/core/Project.*`, `MediaItem.*` | deterministic project state; additive schema v3; original media never modified | Application, UI, decision/analysis artifacts |
| Project round-trip preservation | `Application::{reframeOutputsJson,restoreReframeOutputsFromJson}`, `ReframeCommandOutcome::rawRecord` | an entry this build cannot parse (record or decision) is preserved verbatim, in position, and re-emitted byte-identically, so open+save can never destroy data it does not understand; a preserved entry is never executable | project persistence |
| Media-source seam | `app/media/FrameSource.h` (abstract) | bounded reads; Ok / EndOfStream / Error / Timeout; caller-supplied geometry | `FramePump`, `Player`, playback |
| Persistent FFmpeg source | `app/media/FfmpegFrameSource.*` | one subprocess per open; input seek; aspect-preserving proxy; `-an` | source playback, analysis runner, stream provider |
| Frame pump | `app/media/FramePump.*` | synchronous, caller-driven; no timer/thread | `Player` |
| Player / timing | `app/playback/{Player,Playhead,Clock,SystemClock,PacingPolicy,DefaultPacingPolicy}.*` | explicit-tick state machine; injected clock and pacing | Application playback paths |
| Single-frame decode | `app/media/FrameExtractor.*` | external ffmpeg CLI; `-ss` seek; read-only | Objective 10 preview, provider fallback |
| Duration / stream-facts probe | `app/media/MediaDurationProbe.h`, `FfprobeDurationProbe.*` | duration, frame rate, `streamSummary` (hasAudio/sample rate/channels); additive defaults report *unknown* | whole-clip ranges, audio decision, analysis |
| Equirect -> camera renderer | `app/viewer/EquirectView.*` | vertical-FOV pinhole basis; FOV bounded [20,140]; bilinear, seam-safe | viewer presentation, reframe renderer, crops |
| Viewer camera / state | `app/viewer/{ViewerProjection,ViewportState}.*` | yaw/pitch/roll/FOV conventions shared across engine and UI | viewer, reframe math |
| 360 geometry | `app/target/EquirectProjection.*` | equirect/view pixel <-> direction; detection -> direction + angular radii; seam-safe distance | resolver, tracker, planner, crops |
| Covering views | `app/target/EquirectViewPlan.*` | deterministic overlapping tangent coverage (+polar views) | resolver, analysis |
| Detector seam | `app/target/TargetDetector.h`, `ProcessTargetDetector.*` | subprocess file/JSON protocol; no CV runtime linked; deterministic failure | resolver, analysis, command path |
| Spherical tracker | `app/target/SphericalTargetTracker.*` | deterministic NMS + gated association; duplicates consolidated by overlapping yaw footprints | resolver, identity, planner |
| Resolver | `app/target/TargetResolver.*` | views -> detections -> observations -> tracks; unresolved is never fabricated | command path, analysis |
| Identity + selection | `app/target/{TargetIdentity,TargetSelector}.*` | "me" binding, ordinals, left/right; ambiguity reported with candidates | command path, appearance re-ID |
| Appearance re-identification | `app/target/{Appearance*,IdentityReidentifier}.*` | optional, replaceable embeddings; can veto geometry, never overrides an explicit selection | identity registry |
| Speaker evidence | `app/target/Speaker*` | optional speech intervals, deterministic association and timeline; audio is evidence, never identity | speaker commands, plans |
| Intent boundary | `app/reframe/ReframeIntent.*` | deterministic parse of directions/subjects/temporal/framing/plural groups; in-memory only, never persisted | builder, command runner, contract |
| Plan model | `app/reframe/ReframePlan.*`, `CameraKeyframe.*` | validated, JSON-serializable decision; FOV [20,140]; ordered retained segments | camera path, renderer, decisions, replay |
| Constrained plan adjustment | `app/reframe/ReframePlanAdjustment.*` | the ONE permitted plan-level creator adjustment: monotone lens widening (`FOV_new,i = max(FOV_old,i, T)`), pure and deterministic, with a field-by-field validator proving only the FOV changed and only upward; widening preserves containment by construction (the renderer's covered set is nested in `tan(FOV/2)`); narrowing is refused because the planner's requirement is not stored in the plan (Decision 058) | Application widening entry point |
| Camera path | `app/reframe/CameraPath.*` | pure evaluation; shortest-yaw interpolation; hold; FOV interpolation | renderer, tests |
| Plan builder | `app/reframe/ReframePlanBuilder.*` | intent + resolved targets -> validated plan; carries the lens state; refuses unresolved plural moves | command runner, pipeline |
| Track/plan planner | `app/target/TargetTrackPlanner.*` | follow path from one track (smoothed); one enclosing framing for 2-10 tracks, computed exactly in the renderer's basis (tangent containment over every footprint corner) | command runner |
| Frame provider seam (render/resolve) | `app/reframe/ReframeFrameProvider.h` | deterministic frame per timestamp; injectable | renderer, resolution, tests |
| Persistent stream provider | `app/reframe/ReframeStreamFrameProvider.*` | one stream per anchor; bounded window; seek fallback; frames byte-identical to the seek path | renderer, resolution |
| Renderer + encoder | `app/reframe/ReframeRenderer.*` | deterministic frames from (plan, timestamp); H.264/yuv420p; optional source-audio mux | pipeline, replay |
| Pipeline | `app/reframe/ReframePipeline.*` | orchestration; audio decision from probe facts; notes surfaced | command runner, Application, replay |
| Command runner | `app/reframe/ReframeCommandRunner.*` | single composition entry point; never fabricates a direction; refuses unsupported combinations | Application |
| Render destination policy | `Application::{defaultReframeOutputPath,recordHoldingOutputPath}` | no render writes to a path a held render record owns (refused when named explicitly); an unspecified destination is derived fresh (`<base>_reframe.mp4`, then `<base>_reframe_<N>.mp4`) so renders accumulate instead of overwriting; review acceptance derives it at commit time (Decision 057) | command path, review accept, revision |
| Contract checker | `app/reframe/ReframeContract.*` | pure IPC-1..4 checks over the final (intent, plan) pair | command runner |
| Edit-decision artifact | `app/reframe/EditDecision.*` | versioned, hashed, immutable; strict loader; replay without perception | Application, replay |
| Media-analysis artifact | `app/analysis/MediaAnalysis.*`, `MediaAnalysisRunner.*` | versioned layered evidence with explicit lifecycle/coverage; never an editorial decision | project references only (no consumer yet) |
| Application orchestration | `app/application/{Application,ReframeCommandOutcome}.*` | owns project/media/selection/playback/commands; one shared command request builder and one render-record append gate; injectable seams for model-free tests | UI |
| Creator review (plan view) | `app/application/ReframePlanReview.*` | READ-ONLY derived view of a validated plan (every displayed fact is the plan's own value, a pure derivation, or decision-stage context); carries the plan by value plus a SHA-256 digest of its canonical JSON; never persisted, never a second plan schema | Application review API, UI panel |
| Review decisions | `Application::{prepareReframeCommand,acceptReframeReview,rejectReframeReview}`, `m_commandPreparer` / `m_replayRenderer` | prepare = decision stage only through the shared request builder; accept renders EXACTLY the reviewed plan via the existing render seam and the single append gate; reject renders nothing; review lifetime is explicit | `main.cpp` wiring, UI |
| Creator revision (record level) | `Application::{revisionOutputPath,reviseReframeOutput,revisionsOf,recordHoldingOutputPath}` over the Objective 17 mechanism | revision = a NEW immutable decision whose single parent is the record it revises; a revision only ever ADDS a record and writes a file no record claims — the derived destination is a fresh `<base>_reframe_rev<N>.mp4` sibling, and any path a held record owns is refused (Decision 056); supersession is DERIVED from lineage at read time and only presented, never stored | `main.cpp` wiring, UI |
| Decision provenance (the "why") | `app/application/DecisionProvenance.h`, `Application::decisionProvenance` | read-only view of a persisted decision: origin, instruction, parent hash + whether that parent is held, source status, plan summary; never executes, never modifies | UI provenance readout, tests |
| UI shell | `app/ui/{MainWindow,ViewerWidget}.*` | presentation only; no decoding or planning logic (the review panel displays a plan and emits accept/reject requests; it cannot edit it) | user |

## 24. Behaviour -> test index

Which tests verify which guarantee. Use it to size a regression run instead of re-deriving the set.
Section headers in `tests/test_project.cpp` name the objective each group belongs to.

| Behaviour / guarantee | Tests |
|---|---|
| Project + media model, schema, open/save | `project*`, `saveAndLoad*`, `mediaItem*`, `applicationImport*`, `activeMedia*`, `reopen*` |
| Viewer state, projection, equirect presentation | `viewportState*`, `viewerProjection*`, `viewerWidget*`, `equirectView*` |
| Frame source seam, pump, player/timing | `framePump*`, `fmpegFrameSource*`, `player*`, `playhead*`, `persistentStreamProbe*` |
| Duration/stream-facts probe | `ffprobeDurationProbe*`, `applicationWholeClip*` |
| Plan model + validation | `reframePlan*`, `reframeCameraKeyframe*` |
| Camera path (interpolation, wraparound, bounds) | `cameraPath*` |
| Deterministic render + encode | `reframeRenderer*`, `reframePipelineRendersRealVideoEndToEnd`, `reframePipelineRendersTemporalSegments` |
| Decode equivalence / process bounds | `reframeStreamProvider*`, `reframeRenderEquivalenceStreamingVersusSeek`, `ffmpegFrameSourceLifecycleIsSafe` |
| Intent grammar | `reframeIntent*` |
| Plan building | `reframeBuilder*` |
| Contract rules IPC-1..4 | `reframeContract*` |
| Follow path + density + smoothing | `reframeCommandRunnerFollow*`, `followResolvesWithDefaultMergeDistance`, `targetTrackPlanner*`, `followSmoothing*`, `reframePipelineFollowsMovingSubjectOnRealMedia` |
| Duplicate consolidation (covering views) | `targetResolverReproducesCoveringViewDuplicate`, `coveringViewMergeDoesNotOverMerge` |
| Target geometry / tracking / selection | `equirect*`, `targetTracker*`, `targetResolver*`, `targetSelector*`, `targetIdentity*` |
| Speaker evidence + appearance re-ID | `speaker*`, `appearance*` |
| Temporal editing + compound commands | `temporalEditPlan*`, `reframeCommandRunner*Temporal*`, `reframeCommandRunnerComposesCompoundCommands`, `applicationTemporal*` |
| Persisted decisions, revision, replay | `editDecision*`, `replay*`, `reviseEditDecision*`, `decisionProvenance*` |
| Media analysis (artifact, lifecycle, one pass) | `mediaAnalysis*`, `projectAnalysisRefs*`, `replayIsIndependentOfMediaAnalysis` |
| Rendered-output audio (Obj 28) | `reframeRenderPreservesSourceAudio`, `reframeRenderAudioFollowsRetainedSegments`, `reframeRenderAudioTrimsToSourceRange`, `reframeRenderSilentSourceStaysSilent`, `reframeRenderUnusableAudioFactsDegradesHonestly`, `reframeRenderLeavesSourceMediaUntouched`, `replayReproducesRenderedAudio` |
| Lens / FOV control (Obj 29) | `reframeIntentParsesFraming`, `reframeBuilderAppliesRequestedFraming`, `reframeContractFieldOfViewFidelity`, `reframeCommandRunnerFollowsAtRequestedFraming`, `reframeCommandRunnerSpeakerFramingIsHonest`, `reframePipelineRendersRequestedFraming` |
| Explicit subject sets (Obj 32) | `reframeIntentParsesExplicitSubjectSets`, `reframeCommandRunnerResolvesExplicitSubjects`, `reframeCommandRunnerExplicitSubjectsRefuseHonestly`, `reframeExplicitSubjectsRenderAndReplay` |
| Group framing (Obj 30/31) | `reframeIntentParsesMultiSubjectFraming`, `reframeIntentParsesGroupFraming`, `reframeMultiSubjectFramingGeometry`, `reframeGroupFramingGeometrySweep`, `reframeGroupFramingResolvesCanonicalSets`, `reframeGroupFramingRefusesHonestly`, `reframeCommandRunnerFramesTwoSubjects`, `reframeCommandRunnerResolvesTwoDetectedPeople`, `reframeCommandRunnerRejectsUnsatisfiableMultiSubject`, `reframeCommandRunnerFramesMovingSubjectsAndReplays`, `reframeGroupFramingRendersAndReplays` |
| Constrained lens widening (Obj 40) | `reframePlanWidenLensIsAPureVerifiedTransformation`, `reframePlanWidenLensPreservesContainment`, `applicationWidenRenderedLens*`, `mainWindowWidenLensSurface` |
| Unreadable persisted records preserved across reopen (Obj 38) | `applicationPreservesUnreadableRecordsAcrossReopen`, `preservedRenderRecordIsNeverUsedAsARecord`, `restoreReframeOutputsReportsUnrestorableRecord`, `mainWindowListsUnreadableRecordHonestly` |
| Render destinations never overwrite a recorded render (Obj 37) | `applicationRenderDestinationsNeverOverwriteARecordedRender`, `applicationCommandRefusesAnExplicitPathHeldByARecord`, `applicationReviewAcceptRefusesAClaimedDestination` |
| Creator workflow end to end (Obj 37) | `creatorWorkflowEndToEndPreservesInvariants`, `creatorWorkflowSupersessionIsDecisionLevel` |
| Creator revision + provenance readout (Obj 35) | `applicationRevision*`, `mainWindowRevisionAndProvenanceSurface` |
| Creator review: inspect, accept, reject (Obj 34) | `creatorReview*` |
| Source playback + rendered playback | `sourcePlayback*`, `applicationPlayback*`, `realSourcePlaybackIntegration` |
| Environment-gated real-media / model validation | `real*` (see `NEXT_TASK.md` §4) |


# Reelcraft — Architectural Decisions

## Purpose

This document records significant architectural and development decisions made for Reelcraft.

The purpose is to preserve the reasoning behind decisions so that future development can continue consistently without relying on chat history.

Decisions should be changed only when new evidence, testing, requirements, or constraints justify doing so.

---

## Decision Format

Each significant decision should document:

- Decision
- Status
- Context
- Rationale
- Consequences

Superseded decisions should remain in this document for historical traceability.

---

# Decision 013 — Qt 6 as the Phase 1 Desktop Foundation

**Status:** Accepted

## Context

Reelcraft has completed the technology/runtime evaluation and is ready to establish the smallest executable Phase 1 application foundation.

The requirements call for a desktop-first application capable of supporting responsive UI, background processing, future GPU-accelerated presentation, interactive 2D/3D content, and first-class 360° workflows.

The technology evaluation considered Qt 6, Electron, and Tauri 2. No candidate was treated as a permanent dependency solely because it was evaluated.

## Decision

Qt 6 will be used as the current desktop application foundation for Phase 1.

Qt will provide the application/UI foundation and GPU-capable presentation layer while remaining separate from:

- AI reasoning and provider integrations
- Project/edit-state ownership
- Deterministic media processing
- Rendering/export implementation
- Storage/database implementation
- Local/cloud workload allocation

The selection is an implementation decision, not a new product requirement. The architecture must preserve clear boundaries so that downstream systems are not unnecessarily coupled to Qt.

## Rationale

Qt 6 provides a strong native foundation for Reelcraft's requirements involving GPU-capable desktop presentation, interactive 2D/3D rendering, video integration, and the future first-class 360° viewport.

It also provides a mature cross-platform application framework with native multimedia and graphics capabilities, reducing the need to construct critical video/graphics integration primarily through a web-rendering layer.

The decision is supported by the current requirements and technology evaluation, while recognizing that actual implementation, compatibility, licensing, performance, and maintainability evidence may justify revisiting the choice.

## Consequences

Phase 1 implementation may introduce Qt 6 dependencies.

The project must verify Qt installation, compiler/toolchain compatibility, licensing implications, build reproducibility, application startup, UI responsiveness, and integration boundaries during implementation.

If implementation evidence demonstrates that Qt 6 is unsuitable for Reelcraft's actual requirements, the decision may be revisited through the documented architecture-evolution and technology-evaluation process.

---

# Decision 001 — AI and Media Execution Must Remain Separate

**Status:** Accepted

## Context

Reelcraft is an AI-native video editor. AI will eventually analyze footage, understand creator intent, and determine editing decisions.

AI systems should not directly manipulate source media because doing so would make behavior difficult to validate, reproduce, test, and control.

## Decision

AI systems will generate structured editing instructions.

A deterministic media-processing subsystem will validate and execute those instructions.

The intended boundary is:

AI reasoning
 structured edit plan
 validation
 deterministic execution

## Rationale

This separation provides:

- Predictability
- Testability
- Reproducibility
- Easier debugging
- Provider independence
- Human review
- Safer AI integration
- Non-destructive editing

## Consequences

AI models can change without requiring the media engine to change.

The media engine can also evolve independently of AI providers.

---

# Decision 002 — Editing Must Be Non-Destructive

**Status:** Accepted

## Context

Creators need to preserve original footage while experimenting with AI-generated and manually modified edits.

## Decision

Original source media must never be modified by normal Reelcraft editing operations.

Editing decisions will be represented separately from source media.

## Rationale

This protects creator footage and allows:

- Revisions
- Undo/redo
- Alternative edits
- AI experimentation
- Re-rendering
- Different export formats
- Project portability

## Consequences

Reelcraft requires a structured project/edit representation rather than destructive file modification.

---

# Decision 003 — 360° Video Is a First-Class Media Type

**Status:** Accepted

## Context

Reelcraft is intended to support 360° creators and immersive video workflows.

Treating 360° video as ordinary flat video would limit future capabilities.

## Decision

360° video will be represented and processed as a first-class media type.

The architecture must support:

- Equirectangular media
- 360° metadata
- Orientation
- Virtual-camera positioning
- Reframing
- Keyframed viewpoints
- 360° export
- Flat-video extraction from 360° sources

## Rationale

360° editing requires concepts that do not exist in ordinary flat-video editing.

## Consequences

360° requirements must be considered when designing the media model, viewer, timeline, analysis systems, and rendering pipeline.

---

# Decision 004 — Camera-Specific Behavior Must Use Adapters

**Status:** Accepted

## Context

The Insta360 X5 is the current development camera, but Reelcraft must eventually support additional cameras and media sources.

## Decision

Camera-specific behavior will be isolated behind camera/media adapter boundaries.

## Rationale

The core Reelcraft architecture must not become dependent on one manufacturer's metadata, file structure, or processing behavior.

## Consequences

Adding support for another camera should primarily require implementing or extending an adapter rather than rewriting the core system.

---

# Decision 005 — AI Providers Must Be Replaceable

**Status:** Accepted

## Context

AI capabilities and providers will continue to evolve.

Locking Reelcraft to one provider would create unnecessary technical and business dependency.

## Decision

AI capabilities will be accessed through provider/model abstraction boundaries.

The architecture should support:

- Multiple providers
- Multiple models
- Local models
- Cloud models
- Hybrid processing

## Rationale

This allows Reelcraft to adapt as model capabilities, pricing, availability, hardware, and privacy requirements change.

## Consequences

Provider-specific implementation details must remain outside the core project/edit model.

---

# Decision 006 — Project State Must Be Portable

**Status:** Accepted

## Context

Reelcraft may eventually run across different hardware and processing environments.

## Decision

Project state must remain conceptually independent of the machine performing the processing.

## Rationale

A project should be recoverable, transferable, and reproducible across supported environments.

## Consequences

Project metadata, edit decisions, analysis information, and processing references require a defined persistent representation.

---

# Decision 007 — Human Review Remains the Final Authority

**Status:** Accepted

## Context

AI-generated edits can be useful but may be incorrect or inconsistent with creator intent.

## Decision

Creators retain final authority over AI-generated editing decisions.

AI suggestions must remain reviewable and adjustable.

## Rationale

Reelcraft is intended to augment creators rather than remove creator control.

## Consequences

The UI and edit model must distinguish AI-generated decisions from creator modifications where appropriate.

---

# Decision 008 — Development Must Be Incremental and Checkpointed

**Status:** Accepted

## Context

Reelcraft is a complex system involving media processing, AI, UI, storage, and potentially significant computational workloads.

Large uncontrolled changes increase the risk of regressions and make failures difficult to diagnose.

## Decision

Development will proceed through small logical tasks.

Each active task must have:

- A clear objective
- Explicit scope
- Definition of Done
- Verification
- Regression testing where applicable
- Documentation updates
- Git checkpointing

## Rationale

Small verified changes make the system easier to understand, recover, and maintain.

## Consequences

Large feature requests must be decomposed into smaller development tasks before implementation.

---

# Decision 009 — Inspect Before Changing

**Status:** Accepted

## Context

AI agents and developers may work on Reelcraft at different times.

Changing files without first understanding their current state risks overwriting valid work.

## Decision

Before modifying existing project files, the current state must be inspected.

## Rationale

Persistent project state is more authoritative than assumptions or previous chat instructions.

## Consequences

AI agents must inspect relevant files, Git state, architecture, and current task state before making changes.

---

# Decision 010 — Documentation Is Part of the Project Source of Truth

**Status:** Accepted

## Context

Multiple AI agents may contribute to Reelcraft over time.

Chat history alone is not a reliable development record.

## Decision

Persistent Markdown documentation is part of the project's source of truth.

Important project state, architecture, decisions, known issues, development history, and handoff information must be recorded in the repository.

## Rationale

This allows future development to continue without reconstructing project state from conversations.

## Consequences

Documentation updates are part of the Definition of Done for meaningful work.

---

# Decision 011 — Technology Choices Must Be Evidence-Based

**Status:** Accepted

## Context

The project is currently at the architecture/planning stage.

Prematurely locking the project to frameworks, vendors, or infrastructure could create unnecessary constraints.

## Decision

Technology choices will be evaluated when implementation requirements justify them.

Candidates may be documented without becoming permanent dependencies.

## Rationale

Requirements, performance measurements, compatibility, cost, and maintainability should drive technology selection.

## Consequences

The current architecture intentionally leaves several implementation choices unresolved.

---

# Decision 012 — Architecture Is Allowed to Evolve

**Status:** Accepted

## Context

Some architectural assumptions cannot be fully validated until implementation and testing begin.

## Decision

Reelcraft architecture may evolve when implementation evidence, performance testing, security requirements, creator workflow requirements, or other significant findings justify a change.

Significant changes must be recorded in this document and reflected in project history.

## Rationale

The architecture should be durable without becoming rigid.

## Consequences

Architectural changes require explicit documentation and verification rather than silent modification.

---

# Decision Change Procedure

When a significant architectural decision changes:

1. Identify the existing decision.
2. Record why it is being reconsidered.
3. Document evidence supporting the change.
4. Record the new decision.
5. Identify affected systems and documentation.
6. Update relevant architecture documentation.
7. Test affected functionality when implementation exists.
8. Record the change in project history.
9. Create a Git checkpoint.

Never silently overwrite architectural history.

---

# Decision 014 — Platform Strategy: Desktop First-Class, Android Planned Future

**Status:** Accepted

## Context

Phase 1 uses Qt 6 as the desktop application foundation. Current development occurs in an Android/Termux/Ubuntu environment. This environment is not a native Android deployment target and must not be treated as evidence of Android support. Reelcraft is desktop-first per REQUIREMENTS.md.

## Decision

Desktop is the first-class supported platform at the current stage.

Android is a planned future platform, not a current product target.

Termux/Ubuntu on Android is a development environment only and does not constitute native Android support.

## Rationale

- Desktop-first requirements remain authoritative.
- The current development environment is a Linux userspace hosted by Android, not native Android Qt deployment.
- QtCore/QtWidgets separation already provides a useful portability boundary.
- Expanding Android support now would be scope creep and unsupported by evidence.

## Consequences

- Phase 1 remains desktop-only.
- No Android SDK/NDK, Termux:X11, or native Android packaging work is authorized.
- Future architecture work should preserve platform-independent core boundaries.
- Qt Widgets is not permanently committed as the mobile/UI implementation solely because it is used in Phase 1.
- Android/Termux is recorded as development-only and is not representative of desktop or Android performance.


## Expanded Platform Classification — 2026-09-03

The platform strategy from Decision 014 is extended without creating a new decision number.

Verified current platform classification:

- Linux desktop: first-class / currently verified
- Windows desktop: planned future desktop platform
- macOS: planned future desktop platform
- Android: planned future platform
- iOS: planned future consideration
- iPadOS: planned future consideration
- Termux/Ubuntu: development environment only

Desktop remains the current first-class product priority.

This classification does not authorize Apple SDKs, toolchains, targets, dependencies, Android implementation, UI replacement, Termux:X11 configuration, or native Apple/Android builds.

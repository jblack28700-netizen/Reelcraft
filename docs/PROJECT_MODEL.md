# Reelcraft — Project Model

## 1. Purpose

This document defines the conceptual model of a Reelcraft project.

It describes what information a project must preserve and how major project components relate to one another.

This is a planning-level model. It does not permanently prescribe a database, programming language, serialization format, or storage technology.

---

## 2. Project Definition

A Reelcraft project is the persistent representation of a creator's editing work.

A project connects:

- Source media
- Media metadata
- Analysis results
- Creator instructions
- AI decisions
- Edit decisions
- Generated assets
- Proxies
- Previews
- Project history
- Rendered outputs

The project must preserve editing intent without requiring modification of the original source media.

---

## 3. Conceptual Structure

The project can be understood as:

Project
 Project Metadata
 Source Media References
 Media Metadata
 Analysis Data
 AI Decisions
 Creator Decisions
 Edit Model
 Generated Assets
 Proxy References
 Preview References
 History / Versions
 Export Records

These are conceptual categories rather than a finalized implementation schema.

---

## 4. Project Metadata

Project metadata describes the project itself.

Potential information includes:

- Project identifier
- Project name
- Creation date
- Modification date
- Project version
- Application compatibility information
- Creator preferences where applicable
- Project-level settings
- Output preferences

Metadata must not contain secrets such as API keys or provider credentials.

---

## 5. Source Media

Source media represents the creator's original imported material.

A project should reference source media rather than destructively modifying it.

A source-media record may contain:

- Unique media identifier
- File location/reference
- Original filename
- Container
- Codec
- Resolution
- Frame rate
- Duration
- Audio properties
- Color information
- Orientation
- 360° properties
- Camera metadata
- Import status

Original media is considered protected project input.

---

## 6. Media Analysis

Analysis data describes information Reelcraft has derived from source media.

Possible analysis includes:

- Speech transcripts
- Speaker information
- Scene boundaries
- Shot information
- Objects
- Faces where appropriate
- Visual descriptions
- Audio characteristics
- Silence and pause detection
- Motion information
- 360° orientation information
- Quality/capability information
- Content embeddings or other machine-readable representations

Analysis is derived data and must remain distinguishable from creator decisions.

---

## 7. Creator Intent

Creator intent represents what the creator wants Reelcraft to accomplish.

Intent may originate from:

- Text
- Voice
- UI interaction
- Manual timeline editing
- Other supported input methods

Examples include:

- Remove pauses
- Create a short
- Focus on a speaker
- Reframe 360° footage
- Add captions
- Improve pacing
- Select useful B-roll

Creator intent should ultimately be translated into structured operations.

---

## 8. AI Decisions

AI decisions represent reasoning produced by AI systems.

Examples include:

- Recommended cuts
- Suggested clips
- B-roll selections
- Speaker-focused edits
- Scene selections
- Reframing decisions
- Caption decisions
- Narrative structure
- Finishing suggestions

AI decisions must remain distinguishable from actions that have actually been executed.

An AI suggestion is not automatically an applied edit.

---

## 9. Edit Model

The edit model represents the actual non-destructive editing state of the project.

It may contain:

- Timeline structure
- Tracks
- Clips
- Clip references
- Source ranges
- Cuts
- Trims
- Transitions
- Audio adjustments
- Effects
- Text
- Captions
- Speed changes
- Transformations
- 360° virtual-camera instructions
- Keyframes
- Output settings

The edit model should reference source media and generated assets rather than replacing originals.

---

## 10. Generated Assets

Generated assets are files produced by Reelcraft or external processing.

Examples include:

- Proxy media
- Extracted audio
- Transcoded media
- Generated thumbnails
- Temporary renders
- AI-generated assets
- Intermediate media
- Caption files
- Preview renders

Generated assets must have a clear relationship to the project and should be distinguishable from original source media.

---

## 11. Proxies and Previews

Proxies and previews are disposable or regenerable representations used to improve performance.

They must not become the authoritative source of original media.

The project should be able to determine:

- Which source produced a proxy
- Which settings produced it
- Whether it is valid
- Whether it can be regenerated
- Whether it is currently being used

Missing proxies should normally be recoverable by regeneration.

---

## 12. History and Versions

Project history records meaningful changes to the editing state.

The system should ultimately support:

- Undo
- Redo
- AI edit revisions
- Creator revisions
- Recovery points
- Project versions
- Potential future branching

History should represent changes to project state rather than creating unnecessary copies of large source media.

---

## 13. Exports

An export record describes a rendered output.

It may contain:

- Export identifier
- Source project state/version
- Output format
- Resolution
- Frame rate
- Aspect ratio
- 360° output settings
- Render status
- Output location
- Creation time
- Error information if rendering failed

Exports are outputs of the project, not replacements for the project itself.

---

## 14. State Ownership

The project model should distinguish between different kinds of information.

### Original State

Creator-provided source media and immutable source metadata.

### Derived State

Analysis, proxies, previews, caches, and other regenerable information.

### Decision State

AI recommendations and creator decisions.

### Edit State

The structured representation of the current edit.

### Output State

Information about renders and exports.

This separation is important for reliability, recovery, portability, and future AI workflows.

---

## 15. Project Portability

A Reelcraft project should not depend permanently on one AI provider.

The project should preserve enough structured information to understand:

- What media was used
- What the creator requested
- What AI decided
- What edits were applied
- What output was requested

Provider-specific information may be stored where necessary, but it should not become the fundamental definition of the project.

---

## 16. Missing Data and Recovery

The project model must tolerate missing derived data.

For example:

- A deleted proxy should be regenerable.
- A missing preview should be regenerable.
- A failed AI analysis should be retryable.
- A failed render should not destroy the edit model.
- Missing source media should be clearly identified.
- Corrupt generated data should not silently replace valid project state.

The project should distinguish between unavailable derived data and lost authoritative project data.

---

## 17. Validation

Project data must be validated before being accepted as authoritative project state.

Validation should eventually check:

- Required identifiers
- Valid media references
- Valid source ranges
- Valid timeline relationships
- Valid edit operations
- Valid output settings
- Compatible media capabilities
- Valid AI edit-plan operations

Invalid project state should not silently reach the media engine.

---

## 18. Relationship to AI

AI interacts with the project through structured information.

Conceptually:

Project State
    ↓
Analysis / Context
    ↓
Creator Intent
    ↓
AI Reasoning
    ↓
Structured Edit Plan
    ↓
Validation
    ↓
Edit Model
    ↓
Deterministic Media Processing

AI should not directly mutate arbitrary project files or source media.

---

## 19. Relationship to Media Processing

The deterministic media system consumes validated project/edit information.

It should be able to determine:

- What source media is required
- Which ranges are needed
- Which transformations are required
- Which generated assets are required
- What preview or render should be produced

The media engine should not need to understand the internal reasoning of an AI model.

---

## 20. Future Schema

The conceptual model will eventually be converted into an implementation schema.

That future schema may use:

- Structured files
- A database
- A hybrid model
- Another appropriate persistence mechanism

The implementation must preserve the conceptual distinctions established here.

The final schema should be selected only after the project's runtime, storage, and performance requirements have been evaluated.

---

## 21. Change Policy

A significant change to the project model must:

1. Identify the affected concept.
2. Explain why it must change.
3. Identify affected architecture and contracts.
4. Update related documentation.
5. Record significant decisions in `DECISIONS.md`.
6. Verify compatibility with existing project state.
7. Preserve historical reasoning.

The project model must not be silently changed to accommodate an implementation shortcut.

---

## 22. Current Status

Status: Conceptual project model established.

This document defines the intended conceptual structure of a Reelcraft project.

It is not yet an implementation schema.

No production application implementation is implied by this document.

## 6a. Media Analysis — Status (Objective 21, 2026-09-18)

This section is now backed by an implementation rather than being purely conceptual.

- **Artifact:** `app/analysis/MediaAnalysis.{h,cpp}` — a persisted, versioned record with its own `schemaVersion`, a source reference and fingerprint, a three-way source status (`Matches` / `FileMissing` / `FingerprintMismatch`), an analysis-specification identity, and a deterministic `analysisId`. It is **not** stored inside the project file: the project holds a small `analysisRefs` reference (address only), exactly as this model requires analysis to remain derived data.
- **Shape:** a small closed envelope (addressing, provenance, lifecycle, coverage, confidence) over named, independently versioned capability layers. There is deliberately no catch-all struct.
- **Analysis as derived data:** deleting every analysis artifact leaves a project loadable, renderable and replayable. Analysis is never a correctness dependency and is never on the deterministic replay path.
- **Provider normalization:** normalized Reelcraft observations are persisted; provider-native output is retained opaquely and never interpreted by core.
- **Implemented capabilities:** `technical` (duration, frame rate, resolution, aspect, audio-track presence, declared projection, 360 frame convention) and `targets` (normalized spherical observations and tracks over time).
- **Still conceptual in this section:** transcripts, speaker information beyond voice-activity intervals, scene boundaries, shot information, visual descriptions, motion information, content embeddings and quality/capability information. The model accommodates them as additional layer kinds; none of them exists yet.


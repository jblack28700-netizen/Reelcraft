# Reelcraft — AI Edit Contract

## 1. Purpose

This document defines the conceptual contract between Reelcraft's AI reasoning systems and its deterministic editing/media systems.

The central rule is:

> AI decides what should happen. Deterministic systems execute validated instructions.

AI must not directly control arbitrary low-level media operations.

---

## 2. Core Boundary

The conceptual flow is:

Source Media
    ↓
Media Analysis
    ↓
Project Context
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
Deterministic Media Engine
    ↓
Preview / Render / Export

The structured edit plan is the boundary between AI reasoning and media execution.

---

## 3. AI Responsibilities

AI may be responsible for:

- Understanding creator intent
- Understanding project context
- Interpreting transcripts
- Understanding scenes
- Identifying useful footage
- Selecting candidate clips
- Suggesting edits
- Determining narrative structure
- Selecting B-roll
- Suggesting reframing
- Suggesting captions
- Suggesting finishing operations
- Revising previous decisions
- Explaining decisions

AI is responsible for reasoning, not arbitrary file manipulation.

---

## 4. Deterministic System Responsibilities

The deterministic editing/media system is responsible for:

- Reading media
- Validating media
- Applying approved edit operations
- Generating previews
- Rendering
- Exporting
- Managing deterministic transformations
- Reporting processing progress
- Reporting processing failures

The deterministic system should produce predictable results from the same validated inputs and environment.

---

## 5. Structured Edit Plan

An AI edit plan is a structured representation of intended editing changes.

Conceptually, an edit plan contains:

- Plan identifier
- Project identifier
- Source/context references
- Creator intent
- Proposed operations
- Operation parameters
- Reasoning/explanation where useful
- Confidence where meaningful
- Dependencies
- Validation requirements
- Expected result
- Plan status

This is a conceptual contract, not a finalized JSON schema.

---

## 6. Editing Operations

The contract should support operations such as:

- Select clip
- Trim clip
- Cut segment
- Remove segment
- Reorder clips
- Insert B-roll
- Adjust audio
- Add caption
- Add text
- Apply transition
- Change speed
- Apply visual adjustment
- Reframe
- Set virtual-camera position
- Add keyframes
- Change aspect ratio
- Prepare export

Operations must use structured parameters.

AI should not emit arbitrary shell commands, FFmpeg command strings, or executable code as the normal editing interface.

---

## 7. Source References

AI operations must refer to project-managed media and logical ranges rather than arbitrary filesystem paths wherever possible.

A conceptual media reference may identify:

- Source media identifier
- Time range
- Track/context
- Analysis reference
- 360° context where applicable

This allows the project system to validate references before execution.

---

## 8. Validation

Every AI-generated edit plan must pass validation before execution.

Validation should check:

- Project existence
- Media references
- Time ranges
- Operation types
- Operation parameters
- Media capabilities
- Track relationships
- 360° requirements
- Output requirements
- Dependencies
- Resource availability where known

Invalid plans must be rejected or returned for correction.

They must not be passed directly to the media engine.

---

## 9. Plan Status

An edit plan should conceptually support states such as:

- Proposed
- Validating
- Valid
- Rejected
- Approved
- Executing
- Completed
- Failed
- Superseded

Exact implementation states may evolve.

The important distinction is between:

- What AI proposed
- What was validated
- What the creator approved
- What was executed
- What actually completed

---

## 10. Creator Approval

Reelcraft should support different levels of creator control.

The creator may:

- Accept an AI plan
- Reject an AI plan
- Modify individual operations
- Request a revision
- Approve selected changes
- Undo applied changes
- Ask why a decision was made

The product may eventually support configurable automation levels, but creator authority remains fundamental.

---

## 11. Conversational Revision

AI editing must support iterative conversation.

Conceptually:

Creator:
"Remove the boring parts."

AI:
Creates structured edit plan.

Creator:
"That's too aggressive. Keep the story about the trip."

AI:
Uses the existing project state and previous decision context to generate a revised plan.

This means AI must be able to reason about the current project state rather than treating every request as an isolated command.

---

## 12. Voice and Text

Voice and text should enter the same intent-processing architecture.

Conceptually:

Voice ─┐
       ├──→ Intent Understanding ─→ AI Reasoning ─→ Edit Plan
Text ──┘

Voice transcription is an input mechanism.

It should not create a separate editing system.

---

## 13. 360° Operations

The contract must support 360°-specific operations.

Examples include:

- Set viewing direction
- Reframe toward a subject
- Follow a speaker
- Follow an object
- Create virtual-camera keyframes
- Define field of view
- Convert 360° footage to flat output
- Maintain 360° output

360° operations must preserve the distinction between the original spherical source and the virtual camera used to create a view.

---

## 14. B-roll and Dialogue Intelligence

The contract must eventually support higher-level operations such as:

- Select supporting B-roll
- Replace visually weak sections
- Cover jump cuts
- Prioritize a speaker
- Remove irrelevant dialogue
- Preserve meaningful dialogue
- Synchronize supporting footage with narration

These operations may require analysis metadata produced by separate AI systems.

The edit contract should reference that analysis rather than embedding model-specific behavior.

---

## 15. Explainability

AI-generated editing decisions should be explainable at a useful level.

For example, Reelcraft may be able to explain that a segment was:

- Removed because it contained a long pause
- Selected because it contained a key statement
- Used as B-roll because it visually matched the spoken topic
- Reframed because a speaker was detected in that direction

Explanations should describe decisions without requiring the system to expose private model internals.

---

## 16. Confidence and Uncertainty

Where useful, AI operations may contain confidence or uncertainty information.

Low-confidence operations may be presented for stronger creator review.

Confidence must not be treated as proof of correctness.

The creator remains the final authority.

---

## 17. Provider Independence

The edit contract must remain independent of any specific AI provider or model.

A provider may produce different reasoning or analysis, but the output must ultimately map into Reelcraft's internal structured representation.

This allows providers and models to be replaced without redesigning the entire media engine.

---

## 18. Determinism Boundary

The AI layer may be probabilistic.

The execution layer should be as deterministic as practical.

The same validated edit plan should represent the same intended edit regardless of which AI model originally generated it.

This separation enables:

- Testing
- Reproducibility
- Debugging
- Provider replacement
- Safer automation
- Human review

---

## 19. Failure Handling

AI failures must not corrupt project state.

Possible failures include:

- Provider unavailable
- Network failure
- Invalid model response
- Unsupported operation
- Missing analysis
- Missing media
- Validation failure
- Resource limitations

A failed plan should remain distinguishable from a successfully executed plan.

The creator should be able to retry, revise, or abandon a failed operation.

---

## 20. Security Boundary

AI-generated output must be treated as untrusted input until validated.

The AI layer must not be allowed to:

- Execute arbitrary shell commands
- Modify original media directly
- Modify arbitrary filesystem locations
- Inject executable code into the media pipeline
- Access secrets unnecessarily
- Bypass project validation

All AI output must pass through controlled interfaces.

---

## 21. Versioning

The edit-plan contract must be versioned.

Changes to the contract should preserve the ability to understand historical project decisions where practical.

A project should be able to identify which contract version produced a stored AI decision or edit plan.

Breaking contract changes must be documented and handled deliberately.

---

## 22. Future Expansion

The contract may eventually support higher-level operations such as:

- Story restructuring
- Automatic short-form generation
- Creator-style preferences
- Multi-output generation
- Thumbnail suggestions
- Titles and descriptions
- Social-platform preparation
- Advanced 360° storytelling
- More autonomous editing workflows

These capabilities must continue to pass through the same controlled AI-to-deterministic boundary.

---

## 23. Relationship to Other Documents

- `REQUIREMENTS.md` defines what the product must ultimately support.
- `PROJECT_MODEL.md` defines what a Reelcraft project conceptually contains.
- `ARCHITECTURE.md` defines system boundaries.
- `DECISIONS.md` records significant decisions.
- `CURRENT_STATE.md` records implementation status.
- `NEXT_TASK.md` defines the current development objective.

This document specifically defines the AI-to-editing contract.

---

## 24. Current Status

Status: Conceptual AI edit contract established.

The contract is intentionally implementation-agnostic.

A concrete machine-readable schema should be designed only after the surrounding project model and runtime architecture have been evaluated.

# Reelcraft — Development Environment

## Purpose

This document records the development environment used to build and maintain Reelcraft.

It distinguishes verified environment facts from future requirements so that another developer or AI agent can reproduce the project context without relying on chat history.

---

# Current Development Environment

## Host Platform

Development is currently being performed on an Android device using Termux.

## Linux Development Environment

Reelcraft development is currently being performed inside an Ubuntu environment launched through proot-distro.

The active project path is:

    /root/reelcraft

## Shell

The current development shell is Bash.

## Version Control

Git is installed and operational.

The Reelcraft repository uses:

    main

as its primary branch.

## Current Repository State

The repository has been initialized and the documentation foundation has been checkpointed.

Application implementation has not yet begun.

---

# Verified Development Tools

The following tools have been used successfully in the current environment:

- Termux
- Ubuntu through proot-distro
- Bash
- Git
- Node.js
- npm
- FFmpeg

Tool versions may change over time and should be verified rather than assumed.

---

# Development Environment Principles

## Reproducibility

Important development dependencies and versions should eventually be documented explicitly.

## Minimal Dependencies

Do not install application dependencies merely because they might be useful.

Dependencies should be introduced only when required by an active development task.

## Environment Isolation

Development tools and application dependencies should remain distinguishable from the host Android environment.

## Verification

When a dependency becomes required:

1. Identify why it is required.
2. Install or configure it.
3. Verify that it works.
4. Record the relevant version.
5. Document the dependency when appropriate.
6. Create a checkpoint if the change is meaningful or risky.

---

# Future Environment Requirements

The final Reelcraft runtime environment has not yet been selected.

Future requirements may include:

- Media-processing libraries
- FFmpeg integration
- GPU acceleration
- Video decoding and encoding support
- 360° processing capabilities
- AI model runtimes
- AI provider APIs
- Storage systems
- Database or project-indexing systems
- UI runtime
- Testing frameworks
- Packaging and deployment tools

These are requirements to evaluate, not instructions to install everything immediately.

---

# AI Development Environment

AI agents working on Reelcraft must treat the repository documentation as the persistent source of truth.

Before making changes, an AI agent should inspect:

1. docs/MASTER_GUIDE.md
2. docs/CURRENT_STATE.md
3. docs/NEXT_TASK.md
4. docs/AI_HANDOFF.md
5. Relevant architecture and subsystem documentation
6. Git status and recent history

The AI must not assume that chat history contains the complete project state.

---

# Backup and Recovery

A verified Ubuntu Reelcraft backup exists outside the project repository.

The backup was verified using SHA-256.

Verified checksum:

    da8890d62f186c66e9afa072423e9f9dafe8457bf0c84944a1706aaaab0079c

The backup should be treated as a recovery asset and should not be modified as part of normal development.

---

# Environment Change Rules

When the development environment changes significantly:

1. Record what changed.
2. Record why it changed.
3. Verify the environment.
4. Update this document.
5. Update the development log.
6. Create a Git checkpoint when appropriate.

Do not silently change the environment in ways that could affect reproducibility.

---

# Current Environment Limitations

- The final production environment is not yet defined.
- The final application runtime is not yet selected.
- Automated application testing infrastructure does not yet exist.
- GPU acceleration strategy is not yet finalized.
- Local and cloud processing allocation is not yet finalized.
- Production packaging and deployment are not yet defined.

These limitations are expected at the current foundation stage.

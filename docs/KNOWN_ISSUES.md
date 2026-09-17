# Reelcraft — Known Issues

## Purpose

This document records known limitations, unresolved problems, risks, and areas requiring future investigation.

Known issues must be described accurately and should not be used to hide incomplete implementation.

Issues should be removed or marked resolved only after verification.

---

# Current Project State

Reelcraft is currently in the documentation and architecture foundation stage.

There is no application implementation yet.

Therefore, most current limitations are architectural or implementation-planning gaps rather than software defects.

---

## KI-001 — No Application Implementation

### Status

Open

### Description

The Reelcraft application itself has not yet been implemented.

The current repository primarily contains project documentation and architecture planning.

### Impact

No end-user editing workflow is currently available.

### Planned Resolution

Begin Phase 1 — Foundation with small, independently verifiable implementation tasks.

---

## KI-002 — Project/Edit Schema Not Finalized

### Status

Open

### Description

The final persistent project and edit-model schema has not yet been defined.

### Impact

Implementation of project persistence and structured editing cannot be considered finalized until the schema is designed and validated.

### Planned Resolution

Define the minimum viable project/edit model during the appropriate Phase 1 implementation task.

---

## KI-003 — Media Engine Architecture Not Implemented

### Status

Open

### Description

The deterministic media engine is currently an architectural concept rather than an implemented subsystem.

### Impact

No actual media processing pipeline exists yet.

### Planned Resolution

Implement the media-engine foundation incrementally after the core project foundation is established.

---

## KI-004 — AI Provider Strategy Not Finalized

### Status

Open

### Description

No permanent AI provider or model has been selected.

### Impact

AI functionality cannot yet be optimized around a specific provider or model.

### Planned Resolution

Maintain provider abstraction and evaluate models according to actual Reelcraft requirements during implementation.

---

## KI-005 — Local/Cloud Processing Allocation Not Finalized

### Status

Open

### Description

The exact division of workloads between local, cloud, and hybrid processing remains undecided.

### Impact

Infrastructure and deployment architecture cannot yet be finalized.

### Planned Resolution

Use measured performance, cost, privacy, hardware, and model requirements to guide future decisions.

---

## KI-006 — GPU Acceleration Strategy Not Finalized

### Status

Open

### Description

The long-term GPU acceleration strategy has not yet been determined.

### Impact

Performance architecture remains provisional.

### Planned Resolution

Evaluate acceleration options when real media-processing workloads exist and performance measurements can be collected.

---

## KI-007 — 360° Processing Pipeline Not Implemented

### Status

Open

### Description

360° video is a first-class architectural requirement, but the actual 360° processing, viewing, and reframing pipeline has not yet been implemented.

### Impact

The core 360° creator workflow is not yet functional.

### Planned Resolution

Implement 360° support incrementally, beginning with the foundation required for reliable media representation and viewing.

---

## KI-008 — Camera Adapter System Not Implemented

### Status

Open

### Description

Camera-specific behavior is intended to be isolated behind adapters, but the adapter system has not yet been implemented.

### Impact

Additional camera support cannot yet be added through a formal adapter interface.

### Planned Resolution

Define and implement the adapter boundary before adding substantial camera-specific functionality.

---

## KI-009 — Automated Test Infrastructure Not Implemented

### Status

Open

### Description

The project does not yet contain an application test suite or automated regression framework.

### Impact

Implementation work cannot yet rely on automated application-level regression testing.

### Planned Resolution

Establish testing infrastructure as part of the implementation foundation before substantial feature development.

---

## KI-010 — Final Technology Stack Not Finalized

### Status

Open

### Description

The final UI framework, runtime, storage system, rendering architecture, and deployment strategy remain intentionally undecided.

### Impact

Implementation technology choices must still be evaluated rather than assumed.

### Planned Resolution

Make technology decisions incrementally based on documented requirements, testing, and measurable constraints.

---

# Issue — Current aarch64 proot development environment: GCC driver prefix and unavailable bash sandbox (2026-09-17)

### Status

Open — environment limitation, not a product defect.

### Description

On the current aarch64 proot/Termux development device:

- The Debian GCC 15 driver could not locate `cc1`/`cc1plus` (installed under `/usr/libexec/gcc/...`) or `ld` when invoked as bare `g++`, because it computed an empty/relative install prefix from `argv[0]`. It was repaired non-destructively by symlinking `cc1`, `cc1plus`, and `ld` into `/usr/lib/gcc/aarch64-linux-gnu/15/` and invoking `/usr/bin/g++` (absolute path). Builds must pass `QMAKE_CC=/usr/bin/gcc QMAKE_CXX=/usr/bin/g++`.
- FFmpeg is not on the Debian PATH; the Termux build is used via `REELCRAFT_FFMPEG=/data/data/com.termux/files/usr/bin/ffmpeg`.
- The `bash` tool is unavailable: the workspace-write sandbox backend (bwrap) cannot start on this host, and escalation to `danger-full-access` was declined. Inspection, builds, and tests were performed through the in-process code runtime and detached background processes only.

### Impact

Build/test commands must set the compiler and FFmpeg environment explicitly. Shell-based workflows are not directly available. This does not affect product architecture or source.

### Planned Resolution

Re-verify the standard `scripts/build_and_test.sh` workflow on a host with a working sandbox and a correctly installed GCC; record the result in `DEVELOPMENT_ENVIRONMENT.md`. The GCC symlinks are reversible.

---

# Issue Management Rules

For each future issue:

1. Assign a unique issue identifier.
2. Record its status.
3. Describe the problem clearly.
4. Record its impact.
5. Record the intended resolution or investigation.
6. Verify the resolution before marking the issue resolved.
7. Update related documentation when an issue changes an architectural decision.

Do not silently remove historical issues.

When an issue is resolved, preserve its record and mark it resolved with the verification date and relevant checkpoint when appropriate.

## 2026-09-03 — Environment/Observed

- Offscreen QPA plugin reports `This plugin does not support propagateSizeHints()` during headless smoke test; not observed under a normal windowing platform.
- `qmake` is not on the default PATH in the current environment; use `/usr/lib/qt6/bin/qmake`.

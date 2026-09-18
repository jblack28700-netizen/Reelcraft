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

# Verified Phase 1 Build-and-Test Workflow

- Script: scripts/build_and_test.sh
- Toolchain: /usr/lib/qt6/bin/qmake
- Tests: tests/tests.pro using Qt Test
- Execution (automated): QT_QPA_PLATFORM=offscreen
Manual desktop smoke: verified via Termux:X11 / XCB
- Verified result: 15 passed, 0 failed

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
- The final production runtime is not yet selected; Phase 1 uses Qt 6 via /usr/lib/qt6/bin/qmake.
- A minimal Phase 1 Qt Test suite exists (15 tests). Run scripts/build_and_test.sh to build and run offscreen.
- GPU acceleration strategy is not yet finalized.
- Local and cloud processing allocation is not yet finalized.
- Production packaging and deployment are not yet defined.

These limitations are expected at the current foundation stage.

## Platform Classification

Desktop is the first-class supported platform at the current stage.

Android is a planned future platform, not a current product target.

Termux/Ubuntu on Android is a development environment only and does not constitute native Android support.

Consequently, performance and graphical behavior must be validated on representative desktop or native Android environments, not inferred from the current Termux/Ubuntu userspace.


## Expanded Platform Classification — 2026-09-03

- Linux desktop: first-class / currently verified
- Windows desktop: planned future desktop platform
- macOS: planned future desktop platform
- Android: planned future platform
- iOS: planned future consideration
- iPadOS: planned future consideration
- Termux/Ubuntu: development environment only

The current verified build/test workflow is Linux-only. Apple platform classification is a portability record only.

## DSH Agent Verification Environment — 2026-09-16

- Phase 3 implementation/verification (Objective 3 onward) was additionally built and tested inside the DeepSeek Harness agent container: x86_64 Ubuntu 24.04 (glibc), distinct from the Termux/proot primary development device.
- Build/test toolchain used there: Ubuntu `qt6-base-dev` / `qt6-base-dev-tools` 6.4.2 (providing `/usr/lib/qt6/bin/qmake`), GCC 13.2.0, and the system `ffmpeg` CLI. The package was installed solely to make the existing `scripts/build_and_test.sh` workflow runnable for verification.
- Verified result in that container: application and test builds succeed; full suite 144 passed / 0 failed / 0 skipped; offscreen smoke SMOKE_EXIT=124.
- This is a verification environment, not a change to the primary development environment or to product dependencies; linked FFmpeg libraries and QtMultimedia remain deferred (Decision 017).

## aarch64 proot/Termux Device — Verified Build Workaround (2026-09-17)

The primary development device is an aarch64 proot-distro Ubuntu environment. Its Debian GCC 15 driver does not locate `cc1`/`cc1plus` (installed under `/usr/libexec/gcc/...`) or `ld` when invoked as bare `g++`, because it computes an empty/relative install prefix from `argv[0]`. The following was verified for the 2026-09-17 360 reframing objective:

1. Symlink the compiler executables into the driver's expected directory (non-destructive, reversible):
   - `/usr/lib/gcc/aarch64-linux-gnu/15/cc1` -> `/usr/libexec/gcc/aarch64-linux-gnu/15/cc1`
   - `/usr/lib/gcc/aarch64-linux-gnu/15/cc1plus` -> `/usr/libexec/gcc/aarch64-linux-gnu/15/cc1plus`
   - `/usr/lib/gcc/aarch64-linux-gnu/15/ld` -> `/usr/bin/ld`
2. Invoke qmake with absolute compilers so the driver computes its prefix:
   `/usr/lib/qt6/bin/qmake QMAKE_CC=/usr/bin/gcc QMAKE_CXX=/usr/bin/g++`
3. Provide FFmpeg from the **Debian** container, never the Termux build. Either
   leave `REELCRAFT_FFMPEG` unset (PATH resolves `/usr/bin/ffmpeg`) or set it
   explicitly to `/usr/bin/ffmpeg`. Pointing it at the Termux binary
   (`/data/data/com.termux/files/usr/bin/ffmpeg`) was the original instruction
   here and is a **known-bad configuration** - see the FFmpeg PATH Leak section below.

Verified result on this device: application and test builds succeed; full suite 180 passed / 0 failed / 0 skipped; offscreen smoke SMOKE_EXIT=124.

The `bash` tool is unavailable in this environment (the workspace-write bwrap sandbox backend cannot start). Builds/tests were run through the in-process code runtime with detached background processes. See KNOWN_ISSUES.md.

## FFmpeg PATH Leak — REQUIRED Environment Setup (2026-09-17)

**This is required setup, not an optional convenience.** Without it the real-render
tests cannot complete reliably, and the failure mode looks like intermittency rather
than a configuration error.

### The leak

Inside `proot-distro login ubuntu`, `PATH` ends with the Termux bin directory:

```
/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/data/data/com.termux/files/usr/bin
```

Debian had no `ffmpeg` of its own, so `ffmpeg` resolved to the **Termux
(Android/bionic) build** at `/data/data/com.termux/files/usr/bin/ffmpeg`. Inside
proot that binary had Debian/glibc libraries bound over `/lib`, an ABI mismatch:

```
$ ldd $(which ffmpeg)
.../ffmpeg: error while loading shared libraries:
    /lib/aarch64-linux-gnu/libm.so: invalid ELF header
```

The consequence was measured, not assumed: a **single** seek+decode cost
**14,607 / 15,570 / 15,939 ms**, while `FrameExtractor` enforces a fixed
`kProcessTimeoutMs = 15000` budget (`app/media/FrameExtractor.cpp`). The work sat
exactly on the timeout boundary, so the same test passed or failed by a few hundred
milliseconds run to run. This was a specific, diagnosable PATH leak with a specific
fix — not environmental flakiness.

### The fix

```
proot-distro login ubuntu -- bash -lc 'apt-get update && apt-get install -y ffmpeg'
```

After installation `ffmpeg` resolves to `/usr/bin/ffmpeg` (Debian 8.0.1,
`built with gcc 15`), `ldd` is clean, and the same seek+decode measures
**2,590-2,640 ms** — about 17% of the frame-extraction budget rather than straddling it.

| Measurement | Termux ffmpeg (leaked) | Debian ffmpeg (fixed) |
|---|---|---|
| `which ffmpeg` | `/data/data/com.termux/files/usr/bin/ffmpeg` | `/usr/bin/ffmpeg` |
| `ldd` | `invalid ELF header` | clean |
| Single seek+decode | `14,600-15,900 ms | `2,590-2,640 ms |
| Full suite runtime | `850,000-1,021,000 ms | `221,000-241,000 ms |
| Full suite result | up to 14 failures | 0 failures |

### Why it matters beyond the tests

`scripts/build_and_test.sh` runs the suite under `timeout 240`. With the leaked
Termux ffmpeg the suite took 850-1,021 s and **could never have finished inside that
ceiling**, so the repository's own documented verification workflow was unviable for
reasons unrelated to the product. With the Debian build the suite completes in
`221-241 s and the script is viable again.

Do **not** set `REELCRAFT_FFMPEG` to the Termux binary. Leave it unset (PATH
resolves `/usr/bin/ffmpeg`) or set it explicitly to `/usr/bin/ffmpeg`.
## Optional Detector/Appearance Helper Runtimes — 2026-09-17

The real target-detection and appearance helpers are optional and external; the C++ build links none of these runtimes, and the model-free unit suite requires none of them.

- **Detection helper** (`tools/detector_helper/`): Python 3 + OpenCV 4.10 (`python3-opencv`, `python3-numpy`); YOLOX ONNX weights.
- **Appearance helper** (`tools/appearance_helper/`): Python 3 + ONNX Runtime 1.23 (`python3-onnxruntime`); ReID ONNX weights.
- **Model weights** (downloaded, not committed, under `~/.cache/reelcraft/models/`):
  - `yolox_2022nov.onnx` — OpenCV Zoo YOLOX (Apache-2.0), person/object detection.
  - `reid_0277.onnx` — OpenVINO OMZ `person-reidentification-retail-0277` (Apache-2.0), 256-d appearance embedding.
  - `silero_vad.onnx` — Silero VAD (MIT code and weights), speech-activity detection.
- **Environment variables:** `REELCRAFT_FFMPEG`; detector: `REELCRAFT_TARGET_DETECTOR_PY`, `REELCRAFT_TARGET_DETECTOR_SCRIPT`, `REELCRAFT_TARGET_YOLOX_MODEL`, `REELCRAFT_TARGET_CLIP`, `REELCRAFT_TARGET_OUTPUT`; appearance: `REELCRAFT_REID_PY`, `REELCRAFT_REID_SCRIPT`, `REELCRAFT_REID_MODEL`.
- **GPU:** inference is CPU-only in this environment; the helpers can select CUDA/other backends later (RunPod) without changing the C++ core.
- **Speaker evidence helper** (`tools/speaker_helper/`): Python 3 + ONNX Runtime; model `silero_vad.onnx` (Silero VAD, **MIT** code and weights).
- Install examples and licensing records live in `tools/detector_helper/README.md`, `tools/appearance_helper/README.md`, and `tools/speaker_helper/README.md`; the technology evaluation is in `docs/TARGET_RESOLUTION_TECHNOLOGY.md`.
- Speaker helper environment variables: `REELCRAFT_SPEAKER_PY`, `REELCRAFT_SPEAKER_SCRIPT`, `REELCRAFT_SILERO_MODEL`, and (for integration output) `REELCRAFT_SPEAKER_OUTPUT`.
- **User-command integration output (optional):** `REELCRAFT_COMMAND_OUTPUT` selects the output path for `realUserCommandIntegration`; when unset, a temporary file is used.
- **The speaker integration test requires an audio-bearing clip.** `realDetectorIntegration` reads both video and audio through `REELCRAFT_TARGET_CLIP`. The Objective 5 detector proxy `~/.cache/reelcraft/media/proxy_t115_12s.mp4` was created with `-an` (video only); with it the speaker helper correctly reports "no decodable audio stream" and the speaker assertions fail. For any run that exercises the speaker block, point `REELCRAFT_TARGET_CLIP` at the audio+video proxy `~/.cache/reelcraft/media/proxy_t115_12s_av.mp4` (H.264 + AAC mono). The original `360_TEST_4K.mp4` is never modified.


# Reelcraft — Next Task

## Objective

Perform and document the first real desktop smoke test of the Phase 1 Reelcraft application.

## Status

Not started — blocked by environment.

Current development environment has no real graphical Qt session. Android/Termux/Ubuntu is development-only and cannot provide the real desktop windowing platform required for this objective.

## Why This Is the Next Task

Phase 1 automated tests run offscreen. A real desktop smoke test is required to verify that the UI shell renders reliably and background work does not freeze the UI.

## In Scope

- Launch reelcraft with a real Qt platform plugin
- Verify the main window appears and remains open
- Exercise New Project, Save Project, Open Project, Run Background Demo through the visible UI
- Verify project lifecycle and UI responsiveness
- Document the result and create a Git checkpoint

## Explicitly Out of Scope

- Android support, SDK/NDK, Termux:X11 configuration
- Application source changes unless a defect is confirmed
- New product features
- Production media/AI/editor work
- Offscreen-only verification

## Definition of Done

- Application launches successfully against a real desktop/windowing platform
- Main window renders and remains open
- UI controls behave correctly
- Background demo completes without visibly freezing the UI
- Smoke test documented and checkpointed
- Working tree clean

## Verification Plan

- Establish real graphical Qt session
- Run ./reelcraft without QT_QPA_PLATFORM override
- Perform visible UI verification
- Record exact environment, commands, observed behavior, and warnings
- Commit documentation and verify clean status

## Stop Conditions

- No real graphical Qt display available
- Launch fails or UI cannot be verified visually
- Scope expands beyond smoke test

## Next Task After Completion

Define the next smallest development objective from the verified Phase 1 foundation.

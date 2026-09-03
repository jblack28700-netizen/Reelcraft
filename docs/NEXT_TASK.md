# Reelcraft — Next Task

## Objective

Add and verify a reproducible build-and-test workflow for the Phase 1 desktop foundation.

## Why This Is the Next Task

The Phase 1 foundation and UI integration tests are complete, but the build/run/test commands are manual and partially environment-dependent. A small script makes the workflow reproducible for future agents and contributors.

## In Scope

- Add scripts/build_and_test.sh
- Build the application using /usr/lib/qt6/bin/qmake
- Build the test binary using the same Qt toolchain
- Run the offscreen Qt Test suite
- Update DEVELOPMENT_ENVIRONMENT.md to reflect verified testing infrastructure
- Verify the script from the repository root

## Explicitly Out of Scope

- New application features
- Editor/media/AI functionality
- CI/CD infrastructure
- Packaging or distribution
- Cross-platform build matrix expansion

## Definition of Done

1. scripts/build_and_test.sh exists and is executable
2. Script builds app successfully
3. Script builds tests successfully
4. Script runs all 14 tests offscreen successfully
5. DEVELOPMENT_ENVIRONMENT.md accurately reflects the current test capability
6. Documentation updated
7. Git checkpoint created and working tree clean

## Verification Plan

- Run scripts/build_and_test.sh from the repository root
- Confirm application build succeeds
- Confirm test build succeeds
- Confirm all 14 tests pass
- Inspect git diff --check
- Commit and verify clean status

## Stop Conditions

- Script cannot locate qmake or required Qt modules
- Test run fails or hangs beyond timeout
- Scope expands beyond the reproducible workflow

## Next Task After Completion

Define the next smallest development objective from the verified Phase 1 foundation. Do not begin automatically.

## Status

Complete — 2026-09-03.

## Next Logical State

Define the next smallest development objective from the verified Phase 1 foundation. Do not begin automatically.

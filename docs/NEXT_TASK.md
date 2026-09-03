# Reelcraft — Next Task

## Objective

Complete automated UI integration verification for the Phase 1 desktop application foundation by directly testing MainWindow signal/slot behavior, display updates, and Application error paths.

## Why This Is the Next Task

Phase 1 foundation is implemented and the core test suite passes, but UI-to-application communication and expected failure handling have not yet been verified through automated UI-level tests. This objective closes that verification gap without adding new product features.

## In Scope

- Add object names to MainWindow widgets for testability
- Add automated tests for MainWindow button signals
- Add automated tests for MainWindow project/status label updates
- Add automated tests for Application save/open failure paths
- Update tests/tests.pro to link QtWidgets
- Rebuild and run the full test suite offscreen

## Explicitly Out of Scope

- Full video editor
- Timeline system
- Production media engine
- Production playback
- 360° viewer or reframing
- AI editing
- Advanced editing features
- New application features beyond verification

## Definition of Done

1. App builds successfully
2. Test binary builds successfully
3. All automated tests pass, including new UI and error-path tests
4. MainWindow button clicks verify signal emission
5. MainWindow label updates are verified
6. Application save/open error paths return false and emit backgroundCompleted with an error message
7. Documentation updated
8. Git checkpoint created and working tree clean

## Verification Plan

- Build app with /usr/lib/qt6/bin/qmake && make
- Build tests from tests/ with /usr/lib/qt6/bin/qmake && make
- Run tests with QT_QPA_PLATFORM=offscreen
- Inspect final test summary
- Run git diff --check
- Commit and verify clean status

## Stop Conditions

- Tests fail to compile or run
- Widget tests cannot run headless due to environment limitations
- Scope expands beyond verification into new features

## Next Task After Completion

Define the next smallest development objective from Phase 1 evidence. Do not begin automatically.

## Status

Complete — 2026-09-03.

## Next Logical State

Define the next smallest development objective from the verified Phase 1 foundation. Do not begin automatically.

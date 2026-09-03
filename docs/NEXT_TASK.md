# Reelcraft — Next Task

## Objective

Add and verify automated protection evidence that project lifecycle operations never modify original media.

## Why This Is the Next Task

Phase 1 foundation, UI integration, and reproducible build/test workflow are verified. The remaining Phase 1 Definition of Done gap is explicit evidence that original media remains byte-for-byte unchanged during project create, persist, and reopen operations.

## In Scope

- Add a test fixture file representing original media
- Compute SHA-256 before and after project lifecycle operations
- Verify bytes and hash remain unchanged
- Run the full offscreen test suite
- Update documentation
- Create a Git checkpoint

## Explicitly Out of Scope

- Full video editor
- Production media engine
- Media ingestion or parsing
- 360° workflows
- AI editing
- New application features

## Definition of Done

1. Automated test creates an original-media fixture
2. Test exercises project create/save/reopen without media modification
3. Test verifies fixture bytes and SHA-256 remain identical
4. All tests pass offscreen
5. Documentation updated
6. Git checkpoint created and working tree clean

## Verification Plan

- Add test using QTemporaryDir and QCryptographicHash
- Run scripts/build_and_test.sh from repository root
- Confirm all tests pass
- Inspect git diff --check
- Commit and verify clean status

## Stop Conditions

- Test cannot create or read the fixture
- Project lifecycle unexpectedly modifies media fixture
- Scope expands beyond Phase 1 verification

## Next Task After Completion

Define the next smallest development objective from the verified Phase 1 foundation. Do not begin automatically.

## Status

Complete — 2026-09-03.

## Next Logical State

Define the next smallest development objective from the verified Phase 1 foundation. Do not begin automatically.

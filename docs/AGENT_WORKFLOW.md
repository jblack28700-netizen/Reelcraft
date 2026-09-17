# Reelcraft — Agent Workflow (Canonical Operating Policy)

**Status:** Active
**Scope:** How AI coding agents execute approved objectives on Reelcraft.

This is the canonical operating-policy document for AI agents working on
Reelcraft. It defines *how* agents work: autonomy, efficiency, validation, scope
discipline, and completion. Objective-specific prompts should reference this
document instead of repeating these rules.

This document does not replace the rest of the project-control system. In case
of conflict, the authority order in `docs/AI_HANDOFF.md` (`# 26. Authority
Order`) still applies: actual repository state and verified results outrank any
policy wording here. `MASTER_GUIDE.md`, `CURRENT_STATE.md`, `NEXT_TASK.md`,
`DECISIONS.md`, and the other core documents remain authoritative for *what*
the project is and *what* the objective is; this document governs *how* the
agent carries work out.

---

## 1. Autonomous execution

Within an **approved objective**, work autonomously through the routine loop:

```
Inspect -> Understand -> Implement -> Targeted validation -> Diagnose -> Fix
        -> Regression -> Document -> Checkpoint
```

Routine implementation, testing, debugging, refactoring within scope, and
documentation do **not** require per-step Reject/Allow approval. Make the
smallest useful change, validate it, and continue.

Stop and surface a decision only for:

- **Genuine blockers** — the work cannot proceed without information or access
  that does not exist.
- **Destructive or irreversible actions** — anything that could delete or
  overwrite original media, history, or unverified work.
- **Major architectural decisions** outside the approved objective.
- **Significant licensing/legal ambiguity** — e.g. an ML model whose weights
  have unclear commercial terms (see `docs/KNOWN_ISSUES.md` for precedents).
- **Objective completion / checkpoint review.**

Do not stop merely because a task is difficult, uncertain, or requires several
iterations. Do not ask for approval for routine builds, tests, or docs edits.

## 2. Efficiency / information-per-run rule

Optimize for **useful information gained per agent run**. The repository is
persistent memory; re-deriving it wastes runs and context.

Do not:

- repeatedly deep-dive when the repository already contains enough information
  to proceed;
- repeatedly poll a long-running process (see §5);
- rerun unchanged expensive tests;
- rebuild unrelated subsystems;
- repeat research that is already documented in `docs/`;
- run full real-media/model tests after every small change.

Check `CURRENT_STATE.md`, `NEXT_TASK.md`, `DECISIONS.md`, `KNOWN_ISSUES.md`,
`DEVELOPMENT_LOG.md`, and the relevant subsystem/technology docs **before**
starting new investigation. **Make the smallest useful change first.**

## 3. Risk-based validation

Use the cheapest validation that is sufficient to establish correctness.
Default progression, from cheapest to most expensive:

1. **Compile/build** — catches structural errors immediately.
2. **Targeted tests** — the specific tests for the changed behavior.
3. **Relevant regression** — adjacent areas that could be affected.
4. **Integration test** — cross-module behavior on a controlled fixture.
5. **Expensive real-media/model validation** — the real 360 footage and/or ML
   models.

Do not skip validation that is actually necessary.

The rule is not "test less." The rule is:

> **Test intelligently, according to the changed behavior and risk.**

Edge cases, failure paths, and the "no result / unavailable / ambiguous"
branches of a change must still be covered by targeted tests.

## 4. Change-impact testing

Before running tests, identify **what behavior changed**:

- Which files, functions, data structures, and contracts were touched?
- Which existing tests exercise them?
- Which contracts could a caller observe as changed?

Run only the materially affected tests during the development loop. Reserve
broad regression suites and expensive end-to-end validation for:

- cross-cutting changes (shared types, protocol/schema changes, build changes);
- subsystem completion;
- objective completion;
- checkpoint validation.

## 5. Long-running operations

Never spend agent reasoning time repeatedly polling a long-running command.

- Launch long operations **asynchronously** (background job / detached
  process) and return immediately.
- While they run, continue useful work: repository inspection, documentation,
  diff review, test design.
- Check the result at an appropriate point, not in a tight loop.
- **Do not start a duplicate** expensive operation while the first is still
  running.

Record where the output is written so a later run can read only what it needs.

## 6. Real-media / model testing

Real footage is an **integration oracle**, not a unit-test framework.

- Use deterministic, model-free fixtures for normal development.
- Use real media and ML models when the changed behavior actually requires
  them, or to answer a specific unresolved question.
- Keep the normal unit suite model-free; gate real-media/model tests behind
  environment variables (the established pattern in `tests/test_project.cpp`).
- At objective completion, perform the appropriate comprehensive validation
  **once**.

Do not re-run unchanged real-media/model tests after small, unrelated changes.

## 7. Model / process efficiency

Avoid repeatedly loading expensive ML models or starting heavyweight processes
when the architecture permits reuse.

- Prefer the existing replaceable seams and external-helper protocol over
  bundling new runtimes into the C++ core.
- Reuse cached model files and already-running helpers where the architecture
  allows.
- If repeated model/process startup is identified as a bottleneck but is
  **outside the current objective**, document it (e.g. in `KNOWN_ISSUES.md`)
  and continue — do not expand scope prematurely. CPU/GPU optimization is its
  own objective.

## 8. Permission / sandbox failures

A permission failure is an **execution-environment problem**, not automatically
an application problem.

- Do not repeatedly retry the same blocked operation.
- Do not request unrestricted `danger-full-access` merely to perform routine
  builds or tests.
- Prefer the **narrowest available execution path** that can accomplish the
  work.
- One escalation attempt for a genuinely necessary, narrowly scoped command is
  acceptable; a rejection is final for that command.
- If no suitable execution path exists, **report the actual blocker** instead
  of burning agent time on repeated escalation attempts.

Document reproducible environment quirks in `docs/DEVELOPMENT_ENVIRONMENT.md`
rather than rediscovering them each session.

## 9. Scope control

- **One approved objective at a time.**
- Do not automatically begin the next objective.
- Do not expand scope because an interesting adjacent improvement was found.
- Document worthwhile future work (`NEXT_TASK.md`, `KNOWN_ISSUES.md`, or the
  relevant technology doc) and continue the approved objective.

If a discovered issue genuinely blocks the active objective, surface it — do not
silently fold it into the work.

## 10. Documentation and Git

The repository is the persistent source of truth; chat history is not.

- Maintain the established Reelcraft documentation system
  (`docs/MASTER_GUIDE.md` §6 and `docs/AI_HANDOFF.md` # 13).
- **Significant architectural changes must be recorded in `DECISIONS.md`** as a
  new numbered decision with context, rationale, and consequences.
- **Never rewrite architectural history.** Do not edit or reinterpret accepted
  decisions in place; add a new decision that supersedes or extends them.
- Update `CURRENT_STATE.md`, `NEXT_TASK.md`, `DEVELOPMENT_LOG.md`,
  `PROJECT_HISTORY.md`, `CHANGELOG.md`, and `KNOWN_ISSUES.md` as the change
  requires.
- Use **dedicated Git checkpoints** with understandable, logically scoped
  commits.
- **Never** `reset`, `amend`, `force-push`, or silently overwrite previous
  work. Additive history only.

## 11. Objective completion

A normal objective finishes as:

```
implementation
-> targeted validation
-> appropriate regression
-> required integration / real-media validation
-> documentation
-> Git checkpoint
-> clean working tree
-> report
```

The report states what was implemented, what was verified (with exact results),
what was deliberately not done, the decision number if any, and the commit SHA.
The working tree must be clean when the objective is declared complete.

**Do not begin the next objective automatically.** Wait for an approved
objective.

---

## Objective lifecycle at a glance

```
Approved objective
  |-- Inspect repository state + relevant docs (reuse, don't re-research)
  |-- Confirm scope, non-goals, Definition of Done, validation plan
  |-- Smallest useful change
  |-- Cheapest sufficient validation (build -> targeted -> regression)
  |-- Diagnose/fix, iterate autonomously
  |-- Comprehensive validation once, at completion
  |-- Update docs (+ DECISIONS.md if architectural)
  |-- Dedicated Git checkpoint
  |-- Clean tree + report
  '-- STOP (do not start the next objective)
```

## Relationship to other documents

- `docs/MASTER_GUIDE.md` — project identity, principles, documentation system.
- `docs/AI_HANDOFF.md` — entry point, authority order, handoff rules.
- `docs/NEXT_TASK.md` — the exact approved objective.
- `docs/DECISIONS.md` — accepted architectural decisions.
- `docs/DEVELOPMENT_ENVIRONMENT.md` — build/run commands and environment
  limitations.
- `docs/KNOWN_ISSUES.md` — known limitations and deferred work.
- `tests/test_project.cpp` — established model-free vs env-gated test pattern.

*Precedent: Objective 7 (commit `f4ad631`) is an example of this policy in
practice — small additive seam, model-free targeted tests, one broad regression
run, documented negative feasibility result, Decision 024, dedicated commit,
clean tree.*

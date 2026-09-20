#!/usr/bin/env bash
# Lightweight checkpoint hygiene checks (see docs/AGENT_WORKFLOW.md §13).
#
# Deterministic and cheap: it does NOT build and does NOT run tests. It verifies
# the things that have actually gone wrong before:
#   1. a stale build tree (a generated Makefile missing a new header dependency
#      once linked two revisions of a class -- see DEVELOPMENT_ENVIRONMENT.md);
#   2. a configuration value that drifted between a script and the docs (the
#      240 s -> 900 s suite timeout was the precedent);
#   3. a documented test baseline that no longer matches the last run.
#
# Usage:  scripts/checkpoint_check.sh [path/to/test-output.log]
# Exit:   0 = all checks passed, N = number of failed checks.

set -uo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

failures=0
ok()  { printf '  ok    %s\n' "$1"; }
bad() { printf '  FAIL  %s\n' "$1"; failures=$((failures + 1)); }

echo "checkpoint_check: build-tree freshness"
for dir in . tests; do
  if [ ! -f "$dir/Makefile" ]; then
    bad "$dir/Makefile is missing (run qmake in $dir)"
    continue
  fi
  if (cd "$dir" && make -q >/dev/null 2>&1); then
    ok "$dir tree is current"
  else
    bad "$dir tree is STALE -- rebuild before claiming a checkpoint"
  fi
done

echo "checkpoint_check: script/document drift"
script_timeout="$(grep -oE 'timeout [0-9]+' scripts/build_and_test.sh | head -1 | awk '{print $2}')"
if [ -z "$script_timeout" ]; then
  bad "could not read the suite timeout from scripts/build_and_test.sh"
else
  # Only CURRENT-tense claims are checked. Historical sentences ("ran the suite
  # under timeout 240 when this incident was diagnosed") are records, not drift.
  drift=""
  for value in $(grep -oE 'runs the suite under `timeout [0-9]+`|ceiling is now \*\*[0-9]+ s\*\*' docs/DEVELOPMENT_ENVIRONMENT.md | grep -oE '[0-9]+'); do
    if [ "$value" != "$script_timeout" ]; then
      drift="${drift}timeout $value; "
    fi
  done
  if [ -z "$drift" ]; then
    ok "suite timeout documented consistently ($script_timeout s)"
  else
    bad "DEVELOPMENT_ENVIRONMENT.md states a suite ceiling that disagrees with the script ($script_timeout s): $drift"
  fi
fi

echo "checkpoint_check: documented test baseline"
baseline="$(grep -oE 'Current result: \*\*[0-9]+ passed, [0-9]+ failed, [0-9]+ skipped\*\*' docs/CURRENT_STATE.md | head -1 | grep -oE '[0-9]+' | tr '\n' ' ')"
if [ -z "$baseline" ]; then
  bad "CURRENT_STATE.md has no 'Current result: **N passed, M failed, K skipped**' line"
else
  read -r claimed_passed claimed_failed claimed_skipped <<<"$baseline"
  log="${1:-}"
  if [ -z "$log" ] || [ ! -f "$log" ]; then
    printf '  note  documented baseline: %s passed / %s failed / %s skipped (pass a test log to compare)\n' \
      "$claimed_passed" "$claimed_failed" "$claimed_skipped"
  else
    actual="$(grep -E '^Totals:' "$log" | tail -1 | grep -oE '[0-9]+ passed, [0-9]+ failed, [0-9]+ skipped')"
    if [ "$actual" = "$claimed_passed passed, $claimed_failed failed, $claimed_skipped skipped" ]; then
      ok "baseline matches $log ($actual)"
    else
      bad "documented baseline ($claimed_passed passed, $claimed_failed failed, $claimed_skipped skipped) != last run ($actual)"
    fi
  fi
fi

echo "checkpoint_check: decision history"
decisions="$(grep -cE '^# Decision [0-9]{3} —' docs/DECISIONS.md)"
ok "$decisions decisions present (append-only: never rewrite an accepted decision)"

echo
if [ "$failures" -eq 0 ]; then
  echo "checkpoint_check: PASS"
else
  echo "checkpoint_check: $failures check(s) failed"
fi
exit "$failures"

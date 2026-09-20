#pragma once

#include <QList>
#include <QString>

#include "reframe/ReframeIntent.h"
#include "reframe/ReframePlan.h"

// ReframeContract (Objective 18, Decision 035) is a PURE, deterministic contract
// checker over the FINAL (ReframeIntent, ReframePlan) pair. It verifies that the
// executable plan honours the specific portions of the intent that the existing
// architecture defines as executable requirements.
//
// It is NOT an AI auditor and does NOT attempt semantic understanding of user
// intent. It performs no inference, reads no media, and calls no model.
//
// Properties, all deliberate:
//   - pure: no I/O, no global state, no persistence, no mutation of its inputs;
//   - no dependencies beyond Qt core types already in use;
//   - the report is a deterministic function of its two arguments.
//
// Scope, and what is deliberately NOT checked (Decision 035):
//   - keyframe count is NOT compared to move count, and per-move directions are
//     NOT compared: the speaker planner owns its keyframes, so both would
//     false-positive on the speaker path;
//   - target identity is NOT checked: the plan retains camera coordinates only;
//   - media identity is NOT checked: ReframeIntent carries no media field;
//   - nothing already guaranteed by ReframePlan::isValid() is re-checked.
//
// Intentional behaviours that must never be reported as violations:
//   - a temporal edit may widen the final source range;
//   - an empty camera instruction may synthesize a centered-forward keyframe;
//   - missing output and missing time range legitimately use caller defaults;
//   - labels and notes are descriptive only;
//   - a requested lens CHANGE legitimately starts from the previous lens, so
//     IPC-4 requires every requested field of view to be REACHED by the plan,
//     never that every keyframe carry one of them.

// One contract violation: a stable machine-readable rule id plus a deterministic
// human-readable detail.
struct ContractViolation
{
    QString ruleId;
    QString detail;
};

// The deterministic outcome of one check.
struct ContractReport
{
    QList<ContractViolation> violations;

    bool isConsistent() const { return violations.isEmpty(); }

    // Deterministic summary of every violation, in rule order. Empty when the
    // report is consistent.
    QString summary() const;
};

class ReframeContract
{
public:
    // Stable rule ids.
    static QString outputFidelityRuleId();
    static QString timeRangeRuleId();
    static QString temporalMaterialisationRuleId();
    // Objective 29.
    static QString fieldOfViewRuleId();

    // Evaluates IPC-1, IPC-2, IPC-3 and IPC-4 in a fixed order so that the
    // returned violation list is deterministic.
    static ContractReport check(const ReframeIntent &intent,
                                const ReframePlan &plan);
};

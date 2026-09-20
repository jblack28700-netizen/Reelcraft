#pragma once

#include <QList>
#include <QString>

#include "reframe/ReframePlan.h"

// ReframePlanAdjustment (Objective 40, Decision 058) is the ONE constrained
// plan-level creator adjustment Reelcraft permits: monotone lens widening.
//
// It exists because "keep this exact framing, just show me more of the scene" is
// not expressible through the instruction path without paying for perception
// again and risking a different aim. Widening the field of view with everything
// else held fixed is the one plan change whose containment safety is provable
// from the renderer's own geometry: a pixel's ray is
//
//     forward + right*(tanHalf*ndcX*aspect) + up*(tanHalf*ndcY)     (roll aside)
//
// over an FOV-independent basis, so a direction is inside exactly when
// |lateral/forward| <= tanHalf*aspect and |vertical/forward| <= tanHalf. Raising
// tanHalf only ever WEAKENS those inequalities, so every direction the original
// plan contained stays contained. Narrowing has no such proof: the requirement
// that a tighter lens would have to satisfy is computed from subject footprint
// corners at build time and is not stored in the plan, so it cannot be checked
// here at all.
//
// This is NOT a second intent->plan path. It cannot choose an aim, a time, a
// retained segment, a subject or an output; it relaxes one scalar, and
// isLensWidening() re-derives that claim field by field from the two plans.
namespace ReframePlanAdjustment {

// The widening ladder offered to a creator, ascending. It is DERIVED from the
// existing intent framing vocabulary (ReframeIntent) plus the established default
// lens: no new lens vocabulary is invented here.
QList<double> wideningLadderDegrees();

// The next ladder value strictly above currentFieldOfViewDeg, or 0.0 when there is
// none (the lens is already at the widest supported value).
double nextWiderLensDeg(double currentFieldOfViewDeg);

// The widest keyframe field of view in the plan; 0.0 for a plan with no keyframes.
double widestKeyframeFieldOfViewDeg(const ReframePlan &plan);

// Pure and deterministic: outPlan becomes `plan` with every keyframe field of view
// raised to at least targetFieldOfViewDeg, and with every other field copied
// unchanged. The input plan is never modified. Refuses (with a reason) a
// non-finite target, a target outside the supported bounds, a target that does not
// actually widen any keyframe, and an input plan that is not valid. On success the
// result is validated by isLensWidening() before it is returned.
bool widenLens(const ReframePlan &plan, double targetFieldOfViewDeg,
               ReframePlan *outPlan, QString *error = nullptr);

// Field-by-field verification that `adjusted` is `original` with only keyframe
// field-of-view values INCREASED: same keyframe count and order, same timestamps,
// yaw, pitch, roll and interpolation, same source media identity, schema version,
// source range, retained segments and output specification, every field of view
// non-decreasing, every field of view within the supported bounds, and the result
// itself valid. Comparison is EXACT on purpose: the transformation must copy
// fields, not recompute them.
bool isLensWidening(const ReframePlan &original, const ReframePlan &adjusted,
                    QString *error = nullptr);

} // namespace ReframePlanAdjustment

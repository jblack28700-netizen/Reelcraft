#pragma once

#include <QString>

#include "reframe/ReframePlan.h"
#include "target/TargetTypes.h"

// TargetTrackPlanner converts a resolved target track into the existing
// deterministic reframing representation (ReframePlan). The output is consumed
// unchanged by CameraPath and ReframeRenderer, so a "follow the target" edit
// reuses the exact same execution engine as any other reframing decision.
class TargetTrackPlanner
{
public:
    struct Config
    {
        double fieldOfViewDeg = 90.0;
        int maxKeyframes = 40;
        double minConfidence = 0.3;
        // Objective 26: number of consecutive keyframes averaged into each camera
        // direction. 1 disables smoothing.
        //
        // The window is CENTRED and shrinks SYMMETRICALLY at the ends, which has
        // three consequences the follow path depends on:
        //
        //   * the average is never taken over a one-sided neighbourhood, so the
        //     path is not delayed and a linear (constant-velocity) trajectory is
        //     reproduced EXACTLY -- smoothing removes discrete jitter without
        //     touching genuine motion, and the endpoint keyframes keep their own
        //     value because their window shrinks to themselves;
        //   * every smoothed value lies inside the range of the raw values it was
        //     averaged from, so smoothing can never overshoot; and
        //   * keyframe TIMES are never altered, so start and end timing, the
        //     trajectory direction and the total angular span survive.
        //
        // Yaw is averaged on the circle (the sequence is unwrapped first), so a
        // transition across the +/-180 degree boundary is treated as the short
        // way round. Pitch is averaged linearly and clamped. Even values behave
        // as the odd value below them.
        int smoothingWindow = 5;

        // Objective 30: a lens the instruction EXPLICITLY named (0 = none, the
        // default). The multi-subject path uses the tightest lens that contains
        // every requested subject's reported footprint, unless the instruction
        // named a lens — in which case that lens is used, and a lens too narrow
        // to contain them is refused rather than clamped. The single-track path
        // ignores this field and keeps using fieldOfViewDeg.
        double requestedFieldOfViewDeg = 0.0;
    };

    // Objective 30: the deterministic framing that contains EVERY observation's
    // reported angular footprint — the camera direction and the minimum VERTICAL
    // field of view that fits them all. Pure, stateless and model-free.
    //
    // The rule is the exact inversion of EquirectView's own camera basis: a ray
    // at yaw offset t and pitch p is inside a vertical-FOV v view when
    // |sin t · cos p| <= tan(v/2) · aspect and |sin p| <= tan(v/2). Because
    // tan x >= sin x, requiring tan(span/2) <= tan(v/2) for both axes is
    // CONSERVATIVE — it can never under-frame — and every subject's own reported
    // footprint contributes to the spans, so no arbitrary constant is involved.
    struct EnclosingFraming
    {
        bool ok = false;
        double yawDeg = 0.0;
        double pitchDeg = 0.0;
        double fieldOfViewDeg = 0.0; // required vertical field of view
        QString error;
    };

    static EnclosingFraming enclosingFramingDeg(
        const QList<TargetObservation> &observations, int outputWidth,
        int outputHeight);

    // Objective 30: a camera path that keeps SEVERAL resolved tracks inside one
    // frame. A framing is computed only at the timestamps where EVERY requested
    // subject was actually observed (nothing is interpolated or invented);
    // between them the existing CameraPath interpolation holds the framing. The
    // path uses ONE lens for the whole instruction: the requested one, or the
    // tightest lens that contains every subject throughout. An unsatisfiable
    // request fails with the measured requirement; no subject is ever dropped.
    static bool planTracks(const QList<TargetTrack> &tracks,
                           const QList<QString> &trackIds,
                           const ReframePlan::TimeRange &range,
                           const ReframePlan::OutputSpec &output,
                           const Config &config, ReframePlan *outPlan,
                           QString *error = nullptr);

    // Builds a follow plan from the observations that fall inside the requested
    // range. Returns false (with an error) when no usable observations exist or
    // the resulting plan is invalid; no target information is fabricated.
    static bool planTrack(const TargetTrack &track,
                          const ReframePlan::TimeRange &range,
                          const ReframePlan::OutputSpec &output,
                          const Config &config, ReframePlan *outPlan,
                          QString *error = nullptr);
};

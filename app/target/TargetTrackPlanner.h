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
    };

    // Builds a follow plan from the observations that fall inside the requested
    // range. Returns false (with an error) when no usable observations exist or
    // the resulting plan is invalid; no target information is fabricated.
    static bool planTrack(const TargetTrack &track,
                          const ReframePlan::TimeRange &range,
                          const ReframePlan::OutputSpec &output,
                          const Config &config, ReframePlan *outPlan,
                          QString *error = nullptr);
};

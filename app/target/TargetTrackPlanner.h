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

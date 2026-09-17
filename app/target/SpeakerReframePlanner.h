#pragma once

#include <QString>

#include "reframe/ReframePlan.h"
#include "target/SpeakerTypes.h"
#include "target/TargetTypes.h"

// SpeakerReframePlanner turns an associated speaker timeline into the existing
// deterministic ReframePlan: the camera follows the target associated with the
// active speaker. A speaker change is represented as a cut (a hold keyframe
// 1 ms before the switch followed by the new direction) so the camera does not
// pan across the whole sphere. Speaker/overlap/silence segments without an
// associated target are ignored (the previous camera is held by CameraPath).
class SpeakerReframePlanner
{
public:
    struct Config
    {
        double fieldOfViewDeg = 75.0;
    };

    static bool plan(const QList<SpeakerSegment> &segments,
                     const QList<TargetTrack> &tracks,
                     const ReframePlan::TimeRange &range,
                     const ReframePlan::OutputSpec &output,
                     const Config &config, ReframePlan *outPlan,
                     QString *error = nullptr);
};

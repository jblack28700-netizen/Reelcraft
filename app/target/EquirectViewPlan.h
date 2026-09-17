#pragma once

#include <QList>

#include "target/EquirectProjection.h"

// EquirectViewPlan deterministically covers the 360 sphere with overlapping
// perspective (tangent) views. Detecting in tangent views avoids the severe
// distortion a 2D detector suffers near the equirectangular poles and seam, and
// overlapping views make seam-crossing targets visible in at least one view.
//
// View generation depends only on its configuration: the same configuration
// always yields the same ordered view list.
class EquirectViewPlan
{
public:
    struct Config
    {
        double fieldOfViewDeg = 75.0; // vertical FOV of each view
        int yawCount = 6;
        int pitchCount = 3;
        int viewWidth = 320;
        int viewHeight = 240;
    };

    // Ordered, deterministic covering views (pitch-major, then yaw). Pole views
    // are added when the pitch band would otherwise leave a polar gap.
    static QList<PerspectiveView> coveringViews(const Config &config);

    // True when at least one view contains the given direction.
    static bool covers(const QList<PerspectiveView> &views, double yawDeg,
                       double pitchDeg);
};

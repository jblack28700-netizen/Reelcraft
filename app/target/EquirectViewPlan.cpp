#include "target/EquirectViewPlan.h"

#include <QtMath>

QList<PerspectiveView> EquirectViewPlan::coveringViews(const Config &config)
{
    QList<PerspectiveView> views;

    const double fov = qBound(EquirectProjection::MinFieldOfViewDeg,
                              config.fieldOfViewDeg,
                              EquirectProjection::MaxFieldOfViewDeg);
    const int yawCount = qMax(1, config.yawCount);
    const int pitchCount = qMax(1, config.pitchCount);
    const int viewWidth = qMax(1, config.viewWidth);
    const int viewHeight = qMax(1, config.viewHeight);

    // Keep a ~10 degree overlap between neighbouring rows so a target on a row
    // boundary is fully visible in at least one view.
    const double pitchBand = qBound(0.0, 90.0 - fov / 2.0 + 10.0, 89.0);

    const auto addRow = [&](double pitch) {
        for (int i = 0; i < yawCount; ++i) {
            const double yaw = -180.0
                + 360.0 * static_cast<double>(i) / static_cast<double>(yawCount);
            PerspectiveView view;
            view.yawDeg = EquirectProjection::normalizeYawDeg(yaw);
            view.pitchDeg = EquirectProjection::clampPitchDeg(pitch);
            view.fieldOfViewDeg = fov;
            view.width = viewWidth;
            view.height = viewHeight;
            views.append(view);
        }
    };

    if (pitchCount == 1) {
        addRow(0.0);
    } else {
        for (int i = 0; i < pitchCount; ++i) {
            addRow(-pitchBand + (2.0 * pitchBand) * static_cast<double>(i)
                                    / static_cast<double>(pitchCount - 1));
        }
    }

    // Add straight-up/down views when a row cannot reach the pole (for example
    // when a single centered row is requested).
    const double topPitch = views.isEmpty() ? 0.0 : views.last().pitchDeg;
    const double bottomPitch = views.isEmpty() ? 0.0 : views.first().pitchDeg;
    if (topPitch + fov / 2.0 < 90.0) {
        addRow(90.0);
    }
    if (bottomPitch - fov / 2.0 > -90.0) {
        addRow(-90.0);
    }

    return views;
}

bool EquirectViewPlan::covers(const QList<PerspectiveView> &views, double yawDeg,
                              double pitchDeg)
{
    for (const PerspectiveView &view : views) {
        if (EquirectProjection::isDirectionInView(view, yawDeg, pitchDeg)) {
            return true;
        }
    }
    return false;
}

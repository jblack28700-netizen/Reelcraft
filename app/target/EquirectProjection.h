#pragma once

#include <QPointF>
#include <QRectF>
#include <QString>

// EquirectProjection is the pure, deterministic 360 geometry layer for target
// resolution. It converts between:
//   - equirectangular pixels and spherical directions (yaw/pitch);
//   - a perspective (tangent) view's pixels and spherical directions;
//   - a 2D detection box in a view and a spherical centre + angular extent.
//
// It uses EXACTLY the ViewportState / ViewerProjection / EquirectView
// conventions so detections resolved here map onto the same sphere the
// reframing engine renders from:
//   - yaw 0, pitch 0 -> the camera's FRONT direction (+Y);
//   - +X is to the camera's right; +Z is up;
//   - yaw is normalized to [-180, 180); pitch is clamped to [-90, 90];
//   - vertical FOV is bounded to [20, 140].
//
// All methods are static, stateless, and free of Qt widgets or media access.
struct SphericalDirection
{
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
};

// A deterministic pinhole camera looking into the 360 sphere. This is the same
// camera model EquirectView::render uses to produce a perspective image.
struct PerspectiveView
{
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    double fieldOfViewDeg = 75.0; // vertical FOV
    int width = 320;
    int height = 240;
};

class EquirectProjection
{
public:
    static constexpr double MinFieldOfViewDeg = 20.0;
    static constexpr double MaxFieldOfViewDeg = 140.0;
    static constexpr double MaxPitchDeg = 90.0;

    // --- angle helpers -----------------------------------------------------
    static double normalizeYawDeg(double yawDeg);   // -> [-180, 180)
    static double clampPitchDeg(double pitchDeg);   // -> [-90, 90]
    static double shortestYawDeltaDeg(double fromYawDeg, double toYawDeg);
    static double angularDistanceDeg(const SphericalDirection &a,
                                     const SphericalDirection &b);
    static bool isValidDirection(double yawDeg, double pitchDeg);

    // --- equirectangular pixels <-> directions -----------------------------
    static SphericalDirection directionFromEquirectNormalized(double u, double v);
    static SphericalDirection directionFromEquirectPixel(double px, double py,
                                                         int width, int height);
    static bool equirectPixelFromDirection(double yawDeg, double pitchDeg,
                                           int width, int height, QPointF *outPixel);

    // --- perspective view pixels <-> directions ----------------------------
    static SphericalDirection directionFromViewPixel(const PerspectiveView &view,
                                                     double px, double py);
    static bool viewPixelFromDirection(const PerspectiveView &view, double yawDeg,
                                       double pitchDeg, QPointF *outPixel);
    static bool isDirectionInView(const PerspectiveView &view, double yawDeg,
                                  double pitchDeg);

    // --- detection mapping -------------------------------------------------
    // Maps a detection box in a view to a spherical centre and angular radii.
    // Returns false for an invalid box or an entirely invisible region.
    static bool detectionToDirection(const PerspectiveView &view,
                                     const QRectF &boundingBox,
                                     SphericalDirection *outCenter,
                                     double *outYawRadiusDeg,
                                     double *outPitchRadiusDeg);

private:
    EquirectProjection() = delete;
};

#include "ViewerProjection.h"

#include <QtMath>
#include <cmath>

namespace {

// Markers within this forward-component tolerance of the 90-degree sideways
// plane are treated as not visible. This guards against floating-point
// cos(90deg) ~ 6e-17 being treated as "slightly in front".
constexpr double kFrontEpsilon = 1e-9;

struct Vec3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// World-space unit direction from spherical angles. Convention:
//   - yaw 0, pitch 0 -> +Y (FRONT)
//   - yaw rotates around +Z (toward +X is a right turn when facing +Y)
//   - pitch rotates toward +Z (up) / -Z (down)
Vec3 makeDirection(double yawDeg, double pitchDeg)
{
    const double yaw = qDegreesToRadians(yawDeg);
    const double pitch = qDegreesToRadians(pitchDeg);
    const double cosPitch = std::cos(pitch);
    Vec3 direction;
    direction.x = cosPitch * std::sin(yaw);
    direction.y = cosPitch * std::cos(yaw);
    direction.z = std::sin(pitch);
    return direction;
}

double dot(const Vec3 &a, const Vec3 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 cross(const Vec3 &a, const Vec3 &b)
{
    Vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

double lengthSquared(const Vec3 &v)
{
    return dot(v, v);
}

Vec3 normalized(const Vec3 &v)
{
    const double length = std::sqrt(lengthSquared(v));
    return { v.x / length, v.y / length, v.z / length };
}

} // namespace

bool ViewerProjection::project(double markerYawDeg, double markerPitchDeg,
                               double cameraYawDeg, double cameraPitchDeg,
                               double cameraRollDeg, double fieldOfViewDeg,
                               double viewWidth, double viewHeight,
                               double *outPixelX, double *outPixelY)
{
    if (viewWidth <= 0.0 || viewHeight <= 0.0 || !outPixelX || !outPixelY) {
        return false;
    }

    const double tanHalfFieldOfView =
        std::tan(qDegreesToRadians(fieldOfViewDeg) / 2.0);
    if (tanHalfFieldOfView <= 1e-9) {
        return false;
    }

    const Vec3 worldUp{ 0.0, 0.0, 1.0 };
    const Vec3 forward = makeDirection(cameraYawDeg, cameraPitchDeg);

    // Camera basis: right x up x forward.
    Vec3 right = cross(forward, worldUp);
    if (lengthSquared(right) < 1e-12) {
        // Looking straight up or down: pick a deterministic right vector.
        right = { 1.0, 0.0, 0.0 };
    } else {
        right = normalized(right);
    }
    const Vec3 up = cross(right, forward);

    const Vec3 marker = makeDirection(markerYawDeg, markerPitchDeg);

    const double alongForward = dot(marker, forward);
    if (alongForward <= kFrontEpsilon) {
        // Behind the camera, or effectively on the 90-degree sideways plane.
        return false;
    }

    const double lateral0 = dot(marker, right);
    const double vertical0 = dot(marker, up);

    // Positive roll rotates the projected content counter-clockwise on screen.
    const double roll = qDegreesToRadians(cameraRollDeg);
    const double cosRoll = std::cos(roll);
    const double sinRoll = std::sin(roll);
    const double lateral = lateral0 * cosRoll - vertical0 * sinRoll;
    const double vertical = lateral0 * sinRoll + vertical0 * cosRoll;

    // Normalized device coordinates in a vertical-FOV basis; the horizontal
    // axis is scaled by the canvas aspect ratio.
    const double aspect = viewWidth / viewHeight;
    const double ndcX = (lateral / alongForward) / (tanHalfFieldOfView * aspect);
    const double ndcY = (vertical / alongForward) / tanHalfFieldOfView;

    *outPixelX = (0.5 + 0.5 * ndcX) * viewWidth;
    *outPixelY = (0.5 - 0.5 * ndcY) * viewHeight;
    return true;
}

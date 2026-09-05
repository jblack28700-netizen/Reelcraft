#include "EquirectView.h"

#include <QtGlobal>
#include <QtMath>
#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kMaxPitchDeg = 90.0;
constexpr double kMinFieldOfViewDeg = 20.0;
constexpr double kMaxFieldOfViewDeg = 140.0;

struct Vec3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// Same world-direction convention as ViewerProjection:
//   yaw 0, pitch 0 -> +Y; yaw rotates toward +X; pitch rotates toward +Z.
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

double clampDouble(double value, double low, double high)
{
    return value < low ? low : (value > high ? high : value);
}

} // namespace

bool EquirectView::render(const QImage &equirectSource,
                          double cameraYawDeg, double cameraPitchDeg,
                          double cameraRollDeg, double fieldOfViewDeg,
                          int outputWidth, int outputHeight,
                          QImage *outImage)
{
    if (equirectSource.isNull() || equirectSource.width() <= 0
        || equirectSource.height() <= 0) {
        return false;
    }
    if (outputWidth <= 0 || outputHeight <= 0 || !outImage) {
        return false;
    }
    if (!std::isfinite(cameraYawDeg) || !std::isfinite(cameraPitchDeg)
        || !std::isfinite(cameraRollDeg) || !std::isfinite(fieldOfViewDeg)) {
        return false;
    }
    if (cameraPitchDeg < -kMaxPitchDeg || cameraPitchDeg > kMaxPitchDeg) {
        return false;
    }
    if (fieldOfViewDeg < kMinFieldOfViewDeg || fieldOfViewDeg > kMaxFieldOfViewDeg) {
        return false;
    }

    const double tanHalfFov =
        std::tan(qDegreesToRadians(fieldOfViewDeg) / 2.0);
    if (tanHalfFov <= 1e-9) {
        return false;
    }

    // Camera basis built exactly as in ViewerProjection.
    const Vec3 worldUp{ 0.0, 0.0, 1.0 };
    const Vec3 forward = makeDirection(cameraYawDeg, cameraPitchDeg);
    Vec3 right = cross(forward, worldUp);
    if (lengthSquared(right) < 1e-12) {
        right = { 1.0, 0.0, 0.0 };
    } else {
        right = normalized(right);
    }
    const Vec3 up = cross(right, forward);

    const double roll = qDegreesToRadians(cameraRollDeg);
    const double cosRoll = std::cos(roll);
    const double sinRoll = std::sin(roll);

    const double aspect = static_cast<double>(outputWidth) / outputHeight;
    const double sourceWidth = static_cast<double>(equirectSource.width());
    const double sourceHeight = static_cast<double>(equirectSource.height());

    QImage result(outputWidth, outputHeight, QImage::Format_ARGB32);
    if (result.isNull()) {
        return false;
    }

    for (int py = 0; py < outputHeight; ++py) {
        // Normalized device coordinates with +Y up; pixel centers.
        const double ndcY = 1.0 - (py + 0.5) * 2.0 / outputHeight;
        for (int px = 0; px < outputWidth; ++px) {
            const double ndcX = (px + 0.5) * 2.0 / outputWidth - 1.0;

            // Content-space direction components (vertical-FOV basis).
            const double contentLateral = ndcX * tanHalfFov * aspect;
            const double contentVertical = ndcY * tanHalfFov;

            // Inverse of the established roll convention: positive roll
            // rotates projected content counter-clockwise on screen.
            const double lateral = contentLateral * cosRoll + contentVertical * sinRoll;
            const double vertical = -contentLateral * sinRoll + contentVertical * cosRoll;

            // World-space direction for this pixel's camera ray.
            const Vec3 direction = {
                forward.x + right.x * lateral + up.x * vertical,
                forward.y + right.y * lateral + up.y * vertical,
                forward.z + right.z * lateral + up.z * vertical
            };
            const double length = std::sqrt(lengthSquared(direction));
            const double worldYaw = std::atan2(direction.x, direction.y);
            const double worldPitch =
                std::asin(clampDouble(direction.z / length, -1.0, 1.0));

            // Equirectangular source sampling (nearest neighbor,
            // deterministic). Horizontal wrap is implicit via the u range.
            const double u = (worldYaw + kPi) / (2.0 * kPi); // [0, 1)
            const double v = (kPi / 2.0 - worldPitch) / kPi; // [0, 1]
            int sourceX = static_cast<int>(u * sourceWidth);
            if (sourceX >= equirectSource.width()) {
                sourceX = 0;
            }
            int sourceY = static_cast<int>(v * sourceHeight);
            if (sourceY >= equirectSource.height()) {
                sourceY = equirectSource.height() - 1;
            }

            result.setPixel(px, py, equirectSource.pixel(sourceX, sourceY));
        }
    }

    *outImage = result;
    return true;
}

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

struct RgbaStraight
{
    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;
    double alpha = 0.0;
};

// Reads one source pixel as straight (unpremultiplied) 0..255 components.
RgbaStraight readStraight(const QImage &source, int x, int y, bool premultiplied)
{
    const QRgb color = source.pixel(x, y);
    const double alpha = (color >> 24) & 0xFF;
    double red = (color >> 16) & 0xFF;
    double green = (color >> 8) & 0xFF;
    double blue = color & 0xFF;
    if (premultiplied && alpha > 0.0) {
        red = red * 255.0 / alpha;
        green = green * 255.0 / alpha;
        blue = blue * 255.0 / alpha;
    }
    return { red, green, blue, alpha };
}

// Deterministic bilinear sample of the four neighbors (x0/x1 with horizontal
// wrap, y0/y1 with vertical clamp). Straight-space blending; alpha is
// interpolated with the same weights.
QRgb bilinearSample(const QImage &source, int x0, int x1, int y0, int y1,
                    double fx, double fy)
{
    const bool premultiplied =
        source.format() == QImage::Format_ARGB32_Premultiplied;
    const double weight00 = (1.0 - fx) * (1.0 - fy);
    const double weight10 = fx * (1.0 - fy);
    const double weight01 = (1.0 - fx) * fy;
    const double weight11 = fx * fy;

    const RgbaStraight samples[4] = {
        readStraight(source, x0, y0, premultiplied),
        readStraight(source, x1, y0, premultiplied),
        readStraight(source, x0, y1, premultiplied),
        readStraight(source, x1, y1, premultiplied)
    };
    const double weights[4] = { weight00, weight10, weight01, weight11 };

    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;
    double alpha = 0.0;
    for (int i = 0; i < 4; ++i) {
        red += weights[i] * samples[i].red;
        green += weights[i] * samples[i].green;
        blue += weights[i] * samples[i].blue;
        alpha += weights[i] * samples[i].alpha;
    }

    return qRgba(qBound(0, qRound(red), 255), qBound(0, qRound(green), 255),
                 qBound(0, qRound(blue), 255), qBound(0, qRound(alpha), 255));
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

            // Equirectangular source sampling (bilinear, deterministic).
            // Horizontal wrap is implicit via the u range; vertical edges
            // clamp to the top/bottom source row. Straight (unpremultiplied)
            // blending is performed and alpha is interpolated with the same
            // weights.
            const double u = (worldYaw + kPi) / (2.0 * kPi); // [0, 1)
            const double v = (kPi / 2.0 - worldPitch) / kPi; // [0, 1]

            const double texX = u * sourceWidth;
            const double texY = v * sourceHeight;

            int x0 = static_cast<int>(texX);
            if (x0 >= equirectSource.width()) {
                // u exactly at the seam (1.0): wrap to the left column.
                x0 = equirectSource.width() - 1;
            }
            const int x1 = (x0 + 1) % equirectSource.width();
            double fx = texX - x0;
            if (fx >= 1.0) {
                fx = 1.0;
            }

            int y0 = static_cast<int>(texY);
            if (y0 >= equirectSource.height()) {
                // v at the bottom edge: clamp to the last source row.
                y0 = equirectSource.height() - 1;
            }
            const int y1 = y0 + 1 < equirectSource.height()
                ? y0 + 1
                : equirectSource.height() - 1;
            double fy = texY - y0;
            if (fy >= 1.0) {
                fy = 1.0;
            }

            result.setPixel(px, py, bilinearSample(equirectSource, x0, x1, y0, y1,
                                                   fx, fy));
        }
    }

    *outImage = result;
    return true;
}

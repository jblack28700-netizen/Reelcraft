#include "target/EquirectProjection.h"

#include <QtMath>

#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kFrontEpsilon = 1e-9;

struct Vec3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

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
    if (length <= 0.0) {
        return v;
    }
    return { v.x / length, v.y / length, v.z / length };
}

SphericalDirection sphericalFromVector(const Vec3 &v)
{
    const double length = std::sqrt(lengthSquared(v));
    SphericalDirection direction;
    if (length <= 0.0) {
        return direction;
    }
    direction.yawDeg = qRadiansToDegrees(std::atan2(v.x, v.y));
    direction.pitchDeg = qRadiansToDegrees(
        std::asin(qBound(-1.0, v.z / length, 1.0)));
    return direction;
}

// Camera basis exactly as ViewerProjection/EquirectView build it.
bool cameraBasis(const PerspectiveView &view, Vec3 *outForward, Vec3 *outRight,
                 Vec3 *outUp)
{
    if (view.width <= 0 || view.height <= 0
        || !std::isfinite(view.yawDeg) || !std::isfinite(view.pitchDeg)
        || !std::isfinite(view.fieldOfViewDeg)
        || view.pitchDeg < -90.0 || view.pitchDeg > 90.0
        || view.fieldOfViewDeg < 20.0 || view.fieldOfViewDeg > 140.0) {
        return false;
    }
    const Vec3 worldUp{ 0.0, 0.0, 1.0 };
    const Vec3 forward = makeDirection(view.yawDeg, view.pitchDeg);
    Vec3 right = cross(forward, worldUp);
    if (lengthSquared(right) < 1e-12) {
        right = { 1.0, 0.0, 0.0 };
    } else {
        right = normalized(right);
    }
    *outForward = forward;
    *outRight = right;
    *outUp = cross(right, forward);
    return true;
}

} // namespace

double EquirectProjection::normalizeYawDeg(double yawDeg)
{
    while (yawDeg < -180.0) {
        yawDeg += 360.0;
    }
    while (yawDeg >= 180.0) {
        yawDeg -= 360.0;
    }
    return yawDeg;
}

double EquirectProjection::clampPitchDeg(double pitchDeg)
{
    return qBound(-MaxPitchDeg, pitchDeg, MaxPitchDeg);
}

double EquirectProjection::shortestYawDeltaDeg(double fromYawDeg, double toYawDeg)
{
    double delta = normalizeYawDeg(toYawDeg) - normalizeYawDeg(fromYawDeg);
    if (delta > 180.0) {
        delta -= 360.0;
    } else if (delta <= -180.0) {
        delta += 360.0;
    }
    return delta;
}

double EquirectProjection::angularDistanceDeg(const SphericalDirection &a,
                                              const SphericalDirection &b)
{
    const double p1 = qDegreesToRadians(a.pitchDeg);
    const double p2 = qDegreesToRadians(b.pitchDeg);
    const double dy = qDegreesToRadians(b.yawDeg - a.yawDeg);
    double cosine = std::sin(p1) * std::sin(p2)
        + std::cos(p1) * std::cos(p2) * std::cos(dy);
    cosine = qBound(-1.0, cosine, 1.0);
    return qRadiansToDegrees(std::acos(cosine));
}

bool EquirectProjection::isValidDirection(double yawDeg, double pitchDeg)
{
    return std::isfinite(yawDeg) && std::isfinite(pitchDeg)
        && pitchDeg >= -MaxPitchDeg && pitchDeg <= MaxPitchDeg;
}

SphericalDirection EquirectProjection::directionFromEquirectNormalized(double u,
                                                                       double v)
{
    SphericalDirection direction;
    direction.yawDeg =
        normalizeYawDeg(qRadiansToDegrees(u * 2.0 * kPi - kPi));
    direction.pitchDeg = clampPitchDeg(qRadiansToDegrees(kPi / 2.0 - v * kPi));
    return direction;
}

SphericalDirection EquirectProjection::directionFromEquirectPixel(double px,
                                                                  double py,
                                                                  int width,
                                                                  int height)
{
    if (width <= 0 || height <= 0) {
        return SphericalDirection();
    }
    const double u = (px + 0.5) / static_cast<double>(width);
    const double v = (py + 0.5) / static_cast<double>(height);
    return directionFromEquirectNormalized(u, v);
}

bool EquirectProjection::equirectPixelFromDirection(double yawDeg, double pitchDeg,
                                                    int width, int height,
                                                    QPointF *outPixel)
{
    if (!isValidDirection(yawDeg, pitchDeg) || width <= 0 || height <= 0) {
        return false;
    }
    double u = (qDegreesToRadians(normalizeYawDeg(yawDeg)) + kPi)
        / (2.0 * kPi);
    if (u >= 1.0) {
        u -= 1.0;
    }
    if (u < 0.0) {
        u += 1.0;
    }
    const double v = (kPi / 2.0 - qDegreesToRadians(clampPitchDeg(pitchDeg))) / kPi;
    if (outPixel) {
        outPixel->setX(u * width - 0.5);
        outPixel->setY(v * height - 0.5);
    }
    return true;
}

SphericalDirection EquirectProjection::directionFromViewPixel(
    const PerspectiveView &view, double px, double py)
{
    Vec3 forward;
    Vec3 right;
    Vec3 up;
    if (!cameraBasis(view, &forward, &right, &up)) {
        return SphericalDirection();
    }
    const double tanHalf = std::tan(qDegreesToRadians(view.fieldOfViewDeg) / 2.0);
    const double aspect = static_cast<double>(view.width) / view.height;
    const double ndcX = (px + 0.5) * 2.0 / view.width - 1.0;
    const double ndcY = 1.0 - (py + 0.5) * 2.0 / view.height;
    const double lateral = ndcX * tanHalf * aspect;
    const double vertical = ndcY * tanHalf;
    const Vec3 direction = {
        forward.x + right.x * lateral + up.x * vertical,
        forward.y + right.y * lateral + up.y * vertical,
        forward.z + right.z * lateral + up.z * vertical
    };
    return sphericalFromVector(direction);
}

bool EquirectProjection::viewPixelFromDirection(const PerspectiveView &view,
                                                double yawDeg, double pitchDeg,
                                                QPointF *outPixel)
{
    if (!isValidDirection(yawDeg, pitchDeg)) {
        return false;
    }
    Vec3 forward;
    Vec3 right;
    Vec3 up;
    if (!cameraBasis(view, &forward, &right, &up)) {
        return false;
    }
    const double tanHalf = std::tan(qDegreesToRadians(view.fieldOfViewDeg) / 2.0);
    if (tanHalf <= 1e-9) {
        return false;
    }
    const double aspect = static_cast<double>(view.width) / view.height;
    const Vec3 direction = makeDirection(yawDeg, pitchDeg);
    const double alongForward = dot(direction, forward);
    if (alongForward <= kFrontEpsilon) {
        return false;
    }
    const double lateral = dot(direction, right) / alongForward;
    const double vertical = dot(direction, up) / alongForward;
    const double ndcX = lateral / (tanHalf * aspect);
    const double ndcY = vertical / tanHalf;
    const double px = (ndcX + 1.0) * view.width / 2.0 - 0.5;
    const double py = (1.0 - ndcY) * view.height / 2.0 - 0.5;
    if (outPixel) {
        outPixel->setX(px);
        outPixel->setY(py);
    }
    return true;
}

bool EquirectProjection::isDirectionInView(const PerspectiveView &view,
                                           double yawDeg, double pitchDeg)
{
    QPointF pixel;
    if (!viewPixelFromDirection(view, yawDeg, pitchDeg, &pixel)) {
        return false;
    }
    return pixel.x() >= -0.5 && pixel.x() <= view.width - 0.5
        && pixel.y() >= -0.5 && pixel.y() <= view.height - 0.5;
}

bool EquirectProjection::detectionToDirection(const PerspectiveView &view,
                                              const QRectF &boundingBox,
                                              SphericalDirection *outCenter,
                                              double *outYawRadiusDeg,
                                              double *outPitchRadiusDeg)
{
    if (!boundingBox.isValid() || boundingBox.width() <= 0.0
        || boundingBox.height() <= 0.0) {
        return false;
    }
    const SphericalDirection center =
        directionFromViewPixel(view, boundingBox.center().x(), boundingBox.center().y());
    if (!isValidDirection(center.yawDeg, center.pitchDeg)) {
        return false;
    }

    const QPointF corners[4] = {
        boundingBox.topLeft(), boundingBox.topRight(),
        boundingBox.bottomLeft(), boundingBox.bottomRight()
    };
    double yawRadius = 0.0;
    double pitchRadius = 0.0;
    bool anyCorner = false;
    for (const QPointF &corner : corners) {
        const SphericalDirection direction =
            directionFromViewPixel(view, corner.x(), corner.y());
        if (!isValidDirection(direction.yawDeg, direction.pitchDeg)) {
            continue;
        }
        anyCorner = true;
        yawRadius = qMax(yawRadius,
                         qAbs(shortestYawDeltaDeg(center.yawDeg, direction.yawDeg)));
        pitchRadius = qMax(pitchRadius, qAbs(direction.pitchDeg - center.pitchDeg));
    }
    if (!anyCorner) {
        return false;
    }
    if (outCenter) {
        *outCenter = center;
    }
    if (outYawRadiusDeg) {
        *outYawRadiusDeg = yawRadius;
    }
    if (outPitchRadiusDeg) {
        *outPitchRadiusDeg = pitchRadius;
    }
    return true;
}

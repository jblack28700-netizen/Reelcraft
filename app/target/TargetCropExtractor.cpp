#include "target/TargetCropExtractor.h"

#include <QtGlobal>

#include "viewer/EquirectView.h"

bool TargetCropExtractor::crop(const QImage &equirect, double yawDeg,
                               double pitchDeg, double yawRadiusDeg,
                               double pitchRadiusDeg, const Config &config,
                               QImage *outImage, QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!outImage) {
        return fail(QStringLiteral("Appearance crop output is null."));
    }
    if (equirect.isNull()) {
        return fail(QStringLiteral("Appearance crop requires a valid frame."));
    }
    if (!std::isfinite(yawDeg) || !std::isfinite(pitchDeg)
        || pitchDeg < -90.0 || pitchDeg > 90.0) {
        return fail(QStringLiteral("Appearance crop has an invalid direction."));
    }

    const double angularExtent = qMax(yawRadiusDeg, pitchRadiusDeg);
    double fieldOfView = config.minFieldOfViewDeg;
    if (std::isfinite(angularExtent) && angularExtent > 0.0) {
        fieldOfView = 2.0 * angularExtent * config.marginFactor;
    }
    fieldOfView = qBound(config.minFieldOfViewDeg, fieldOfView,
                         config.maxFieldOfViewDeg);
    // Keep the FOV within EquirectView's supported range.
    fieldOfView = qBound(20.0, fieldOfView, 140.0);
    const int width = qMax(1, config.width);
    const int height = qMax(1, config.height);

    return EquirectView::render(equirect, yawDeg, pitchDeg, 0.0, fieldOfView,
                                width, height, outImage);
}

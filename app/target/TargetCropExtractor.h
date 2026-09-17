#pragma once

#include <QImage>
#include <QString>

// TargetCropExtractor produces a deterministic appearance crop for a target
// direction, using the existing EquirectView projection (the same camera model
// as detection/reframing). The crop's field of view is derived from the
// target's angular extent so the whole person is included; it is bounded by the
// configured minimum/maximum.
class TargetCropExtractor
{
public:
    struct Config
    {
        double minFieldOfViewDeg = 40.0;
        double maxFieldOfViewDeg = 120.0;
        double marginFactor = 1.25;
        int width = 128;
        int height = 256;
    };

    static bool crop(const QImage &equirect, double yawDeg, double pitchDeg,
                     double yawRadiusDeg, double pitchRadiusDeg,
                     const Config &config, QImage *outImage,
                     QString *error = nullptr);
};

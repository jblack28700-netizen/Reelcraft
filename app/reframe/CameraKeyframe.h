#pragma once

#include <QJsonObject>
#include <QString>

// CameraKeyframe is one deterministic virtual-camera decision for 360
// reframing. It is pure data: no rendering, decoding, or media access.
//
// timeMs is an ABSOLUTE source-media timestamp (milliseconds). The camera
// angles use the established ViewportState conventions. interpolation
// describes how the path behaves on the segment STARTING at this keyframe
// (toward the next keyframe).
//
// Keyframes are inspectable and serializable so an AI decision, a creator
// adjustment, and a rendered result can all be traced back to the same data.
struct CameraKeyframe
{
    enum class Interpolation {
        Linear, // interpolate from this keyframe toward the next
        Hold    // hold this keyframe's camera until the next keyframe
    };

    qint64 timeMs = 0;
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    double rollDeg = 0.0;
    double fieldOfViewDeg = 90.0;
    Interpolation interpolation = Interpolation::Linear;

    QJsonObject toJsonObject() const;

    // Restores a keyframe from JSON. Requires a numeric timeMs and numeric
    // finite camera fields within the established bounds. Returns false with a
    // descriptive error otherwise.
    static bool readFromJsonObject(const QJsonObject &object, CameraKeyframe *out,
                                   QString *error = nullptr);
};

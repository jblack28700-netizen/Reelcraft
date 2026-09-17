#include "CameraKeyframe.h"

#include <QJsonValue>

#include "reframe/ReframeMath.h"

QJsonObject CameraKeyframe::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("timeMs"), static_cast<double>(timeMs));
    object.insert(QStringLiteral("yawDeg"), yawDeg);
    object.insert(QStringLiteral("pitchDeg"), pitchDeg);
    object.insert(QStringLiteral("rollDeg"), rollDeg);
    object.insert(QStringLiteral("fieldOfViewDeg"), fieldOfViewDeg);
    object.insert(QStringLiteral("interpolation"),
                  interpolation == Interpolation::Hold ? QStringLiteral("hold")
                                                       : QStringLiteral("linear"));
    return object;
}

bool CameraKeyframe::readFromJsonObject(const QJsonObject &object,
                                        CameraKeyframe *out, QString *error)
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

    if (!out) {
        return fail(QStringLiteral("Camera keyframe output is null."));
    }

    const QJsonValue timeValue = object.value(QStringLiteral("timeMs"));
    const QJsonValue yawValue = object.value(QStringLiteral("yawDeg"));
    const QJsonValue pitchValue = object.value(QStringLiteral("pitchDeg"));
    const QJsonValue rollValue = object.value(QStringLiteral("rollDeg"));
    const QJsonValue fovValue = object.value(QStringLiteral("fieldOfViewDeg"));

    if (!timeValue.isDouble() || !yawValue.isDouble() || !pitchValue.isDouble()
        || !rollValue.isDouble() || !fovValue.isDouble()) {
        return fail(QStringLiteral(
            "Camera keyframe is missing required numeric fields."));
    }

    CameraKeyframe frame;
    frame.timeMs = static_cast<qint64>(timeValue.toDouble());
    frame.yawDeg = yawValue.toDouble();
    frame.pitchDeg = pitchValue.toDouble();
    frame.rollDeg = rollValue.toDouble();
    frame.fieldOfViewDeg = fovValue.toDouble();

    if (frame.timeMs < 0) {
        return fail(QStringLiteral(
            "Camera keyframe timeMs must not be negative."));
    }
    if (!reframe::isFinite(frame.yawDeg) || !reframe::isFinite(frame.rollDeg)) {
        return fail(QStringLiteral(
            "Camera keyframe yaw/roll must be finite."));
    }
    if (!reframe::isValidPitchDeg(frame.pitchDeg)) {
        return fail(QStringLiteral(
            "Camera keyframe pitch must be finite and within [-90, 90]."));
    }
    if (!reframe::isValidFieldOfViewDeg(frame.fieldOfViewDeg)) {
        return fail(QStringLiteral(
            "Camera keyframe fieldOfView must be finite and within [20, 140]."));
    }

    const QJsonValue interpolationValue =
        object.value(QStringLiteral("interpolation"));
    if (interpolationValue.isUndefined() || interpolationValue.isNull()) {
        frame.interpolation = Interpolation::Linear;
    } else if (interpolationValue.isString()) {
        const QString text = interpolationValue.toString();
        if (text == QStringLiteral("linear")) {
            frame.interpolation = Interpolation::Linear;
        } else if (text == QStringLiteral("hold")) {
            frame.interpolation = Interpolation::Hold;
        } else {
            return fail(QStringLiteral(
                "Camera keyframe interpolation must be 'linear' or 'hold'."));
        }
    } else {
        return fail(QStringLiteral(
            "Camera keyframe interpolation must be a string."));
    }

    *out = frame;
    return true;
}

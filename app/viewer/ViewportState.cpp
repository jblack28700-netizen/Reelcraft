#include "ViewportState.h"

#include <QtGlobal>

namespace {
constexpr double kMinFieldOfView = 20.0;
constexpr double kMaxFieldOfView = 140.0;
constexpr double kPitchLimit = 90.0;

double normalizeAngle(double value)
{
    while (value < -180.0) {
        value += 360.0;
    }
    while (value >= 180.0) {
        value -= 360.0;
    }
    return value;
}

double clamp(double value, double min, double max)
{
    return qBound(min, value, max);
}

bool nearEqual(double a, double b)
{
    return qAbs(a - b) < 0.000001;
}
}

ViewportState::ViewportState(QObject *parent)
    : QObject(parent)
{
}

double ViewportState::yaw() const
{
    return m_yaw;
}

double ViewportState::pitch() const
{
    return m_pitch;
}

double ViewportState::roll() const
{
    return m_roll;
}

double ViewportState::fieldOfView() const
{
    return m_fieldOfView;
}

void ViewportState::setYaw(double value)
{
    const double normalized = normalizeAngle(value);
    if (!nearEqual(m_yaw, normalized)) {
        m_yaw = normalized;
        emit yawChanged(m_yaw);
    }
}

void ViewportState::setPitch(double value)
{
    const double clamped = clamp(value, -kPitchLimit, kPitchLimit);
    if (!nearEqual(m_pitch, clamped)) {
        m_pitch = clamped;
        emit pitchChanged(m_pitch);
    }
}

void ViewportState::setRoll(double value)
{
    const double normalized = normalizeAngle(value);
    if (!nearEqual(m_roll, normalized)) {
        m_roll = normalized;
        emit rollChanged(m_roll);
    }
}

void ViewportState::setFieldOfView(double value)
{
    const double clamped = clamp(value, kMinFieldOfView, kMaxFieldOfView);
    if (!nearEqual(m_fieldOfView, clamped)) {
        m_fieldOfView = clamped;
        emit fieldOfViewChanged(m_fieldOfView);
    }
}

QJsonObject ViewportState::toJsonObject() const
{
    QJsonObject object;
    object.insert(QStringLiteral("yaw"), m_yaw);
    object.insert(QStringLiteral("pitch"), m_pitch);
    object.insert(QStringLiteral("roll"), m_roll);
    object.insert(QStringLiteral("fieldOfView"), m_fieldOfView);
    return object;
}

bool ViewportState::readFromJsonObject(const QJsonObject &object, QString *error)
{
    if (error) {
        error->clear();
    }

    const QString yawKey = QStringLiteral("yaw");
    const QString pitchKey = QStringLiteral("pitch");
    const QString rollKey = QStringLiteral("roll");
    const QString fovKey = QStringLiteral("fieldOfView");

    const bool valid =
        object.contains(yawKey) && object.value(yawKey).isDouble() &&
        object.contains(pitchKey) && object.value(pitchKey).isDouble() &&
        object.contains(rollKey) && object.value(rollKey).isDouble() &&
        object.contains(fovKey) && object.value(fovKey).isDouble();

    if (!valid) {
        if (error) {
            *error = QStringLiteral("Viewer state JSON is missing required numeric fields.");
        }
        return false;
    }

    setYaw(object.value(yawKey).toDouble());
    setPitch(object.value(pitchKey).toDouble());
    setRoll(object.value(rollKey).toDouble());
    setFieldOfView(object.value(fovKey).toDouble());

    return true;
}

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

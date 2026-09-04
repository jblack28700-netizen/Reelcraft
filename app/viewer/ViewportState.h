#pragma once

#include <QObject>

class ViewportState : public QObject
{
    Q_OBJECT

public:
    explicit ViewportState(QObject *parent = nullptr);

    double yaw() const;
    double pitch() const;
    double roll() const;
    double fieldOfView() const;

    void setYaw(double value);
    void setPitch(double value);
    void setRoll(double value);
    void setFieldOfView(double value);

signals:
    void yawChanged(double value);
    void pitchChanged(double value);
    void rollChanged(double value);
    void fieldOfViewChanged(double value);

private:
    double m_yaw = 0.0;
    double m_pitch = 0.0;
    double m_roll = 0.0;
    double m_fieldOfView = 90.0;
};

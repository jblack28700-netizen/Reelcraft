#pragma once

#include <QElapsedTimer>

#include "playback/Clock.h"

// SystemClock is the production Clock backed by a monotonic QElapsedTimer.
// Time is measured from construction.
class SystemClock : public Clock
{
public:
    SystemClock();

    qint64 nowMillis() const override;

private:
    QElapsedTimer m_timer;
};

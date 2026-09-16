#include "SystemClock.h"

SystemClock::SystemClock()
{
    m_timer.start();
}

qint64 SystemClock::nowMillis() const
{
    return m_timer.elapsed();
}

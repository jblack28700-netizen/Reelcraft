#include "Playhead.h"

qint64 Playhead::frameCount() const
{
    return m_frameCount;
}

qint64 Playhead::currentFrameIndex() const
{
    return m_frameCount > 0 ? m_frameCount - 1 : -1;
}

qint64 Playhead::positionMs(qint64 frameIntervalMs) const
{
    if (m_frameCount <= 0 || frameIntervalMs <= 0) {
        return 0;
    }
    return (m_frameCount - 1) * frameIntervalMs;
}

void Playhead::advance()
{
    ++m_frameCount;
}

void Playhead::reset()
{
    m_frameCount = 0;
}

#pragma once

#include <QtGlobal>

// Playhead is the deterministic current-position value of the player/timing
// subsystem (Phase 3 Objective 4). It counts frames presented since the last
// reset and derives the frame index and presentation timestamp from an explicit
// playback frame interval. It performs no pacing and reads no clock.
class Playhead
{
public:
    // Number of frames presented since the last reset.
    qint64 frameCount() const;

    // Index of the most recently presented frame, or -1 before any frame.
    qint64 currentFrameIndex() const;

    // Presentation timestamp of the current frame (0 before any frame).
    qint64 positionMs(qint64 frameIntervalMs) const;

    // Marks one frame as presented / resets the playhead to the start.
    void advance();
    void reset();

private:
    qint64 m_frameCount = 0;
};

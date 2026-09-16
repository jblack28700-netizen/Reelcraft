#pragma once

#include <QtGlobal>

// PacingPolicy is the replaceable decision boundary that maps elapsed playback
// time to a number of frames to present (Phase 3 Objective 4). It is expected
// to be pure and deterministic: implementations must not read clocks, sleep,
// or touch media. Tests substitute fixed policies to verify player behavior
// without wall-clock timing.
class PacingPolicy
{
public:
    virtual ~PacingPolicy() = default;

    // Frames that should be presented for elapsedMs since the playhead's last
    // paced advancement, given the frame interval. Returns >= 0; 0 means no
    // frame is due yet. frameIntervalMs is always > 0.
    virtual int framesToAdvance(qint64 elapsedMs, qint64 frameIntervalMs) const = 0;

protected:
    PacingPolicy() = default;
};

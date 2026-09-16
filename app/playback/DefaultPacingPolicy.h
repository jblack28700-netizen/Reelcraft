#pragma once

#include "playback/PacingPolicy.h"

// DefaultPacingPolicy presents one frame per elapsed frame interval and drops
// frames when the caller falls behind, so playback stays on the elapsed-time
// cadence. It is stateless and deterministic.
class DefaultPacingPolicy : public PacingPolicy
{
public:
    int framesToAdvance(qint64 elapsedMs, qint64 frameIntervalMs) const override;
};

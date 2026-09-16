#pragma once

#include <QtGlobal>

// Clock is the replaceable timing source for the player/timing subsystem
// (Phase 3 Objective 4). It reports a monotonic time in milliseconds and is
// deliberately minimal: pacing and playhead advancement are the player's
// responsibility, not the clock's.
//
// The abstraction exists so tests can inject a deterministic manual clock and
// verify playback behavior without any wall-clock delays.
class Clock
{
public:
    virtual ~Clock() = default;

    // Monotonic time in milliseconds. Never a wall-clock date.
    virtual qint64 nowMillis() const = 0;

protected:
    Clock() = default;
};

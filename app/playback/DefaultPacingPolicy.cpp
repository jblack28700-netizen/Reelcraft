#include "DefaultPacingPolicy.h"

int DefaultPacingPolicy::framesToAdvance(qint64 elapsedMs,
                                         qint64 frameIntervalMs) const
{
    if (elapsedMs <= 0 || frameIntervalMs <= 0) {
        return 0;
    }
    return static_cast<int>(elapsedMs / frameIntervalMs);
}

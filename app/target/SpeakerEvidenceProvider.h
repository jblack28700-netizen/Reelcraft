#pragma once

#include <QString>

#include "target/SpeakerTypes.h"

// SpeakerEvidenceProvider is the replaceable audio/speaker-analysis boundary.
//
// It receives a media path and a time range and returns structured speech
// evidence (intervals with a provider-local speakerId). The C++ core never
// links a speech/audio model: an in-process implementation, an external helper,
// or a future GPU service can all implement this interface.
//
// Implementations must never modify the source media and must fail
// deterministically rather than fabricating speech.
class SpeakerEvidenceProvider
{
public:
    virtual ~SpeakerEvidenceProvider() = default;

    virtual QString name() const = 0;

    // Analyzes [startMs, endMs] of mediaPath. Returns false with a descriptive
    // error on failure; an unavailable analysis is still a valid (available ==
    // false) result.
    virtual bool analyze(const QString &mediaPath, qint64 startMs, qint64 endMs,
                         SpeakerAnalysis *out, QString *error = nullptr) = 0;

protected:
    SpeakerEvidenceProvider() = default;
};

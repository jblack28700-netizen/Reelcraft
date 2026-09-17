#pragma once

#include <QList>

#include "target/SpeakerTypes.h"

// SpeakerTimeline converts raw speech intervals into deterministic timeline
// segments with explicit hysteresis, so the camera does not thrash when speech
// alternates rapidly.
//
// Documented behavior:
//   - same-speaker intervals separated by <= holdMs are merged (short pauses
//     inside one person's turn do not end it);
//   - speech runs shorter than minSpeechMs are ignored;
//   - a segment from a different speaker must last >= switchConfirmMs to take
//     over; otherwise it is absorbed into the current speaker's segment;
//   - a silence gap shorter than holdMs is absorbed into the previous segment;
//   - simultaneous speech from two or more speakers becomes an Overlap segment
//     (speakerId empty: the system does not force a single identity).
class SpeakerTimeline
{
public:
    struct Config
    {
        qint64 minSpeechMs = 300;
        qint64 holdMs = 700;
        qint64 switchConfirmMs = 500;
        double minConfidence = 0.5;
    };

    static QList<SpeakerSegment> build(const QList<SpeakerInterval> &intervals,
                                       const Config &config);
};

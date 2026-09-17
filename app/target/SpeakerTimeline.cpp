#include "target/SpeakerTimeline.h"

#include <algorithm>

namespace {

struct Run
{
    QString speakerId;
    qint64 startMs = 0;
    qint64 endMs = 0;
    double confidence = 0.0;
    QString targetIdHint;
};

struct StateSegment
{
    qint64 startMs = 0;
    qint64 endMs = 0;
    QStringList speakers;
    QString targetIdHint;
};

double confidenceFor(const QList<Run> &runs, const QStringList &speakers,
                     qint64 startMs, qint64 endMs)
{
    double confidence = 0.0;
    for (const Run &run : runs) {
        if (!speakers.contains(run.speakerId)) {
            continue;
        }
        if (run.startMs < endMs && run.endMs > startMs) {
            confidence = qMax(confidence, run.confidence);
        }
    }
    return confidence;
}

} // namespace

QList<SpeakerSegment> SpeakerTimeline::build(const QList<SpeakerInterval> &intervals,
                                             const Config &config)
{
    // 1. Filter by confidence and validity.
    QList<SpeakerInterval> valid;
    for (const SpeakerInterval &interval : intervals) {
        QString intervalError;
        if (!interval.isValid(&intervalError)) {
            continue;
        }
        if (interval.confidence < config.minConfidence) {
            continue;
        }
        valid.append(interval);
    }
    if (valid.isEmpty()) {
        return {};
    }

    // 2. Merge same-speaker intervals separated by <= holdMs.
    QList<Run> runs;
    for (const SpeakerInterval &interval : valid) {
        bool merged = false;
        for (Run &run : runs) {
            if (run.speakerId != interval.speakerId) {
                continue;
            }
            if (interval.startMs <= run.endMs + config.holdMs
                && interval.endMs >= run.startMs) {
                run.startMs = qMin(run.startMs, interval.startMs);
                run.endMs = qMax(run.endMs, interval.endMs);
                run.confidence = qMax(run.confidence, interval.confidence);
                if (run.targetIdHint.isEmpty()) {
                    run.targetIdHint = interval.targetIdHint;
                }
                merged = true;
                break;
            }
        }
        if (!merged) {
            Run run;
            run.speakerId = interval.speakerId;
            run.startMs = interval.startMs;
            run.endMs = interval.endMs;
            run.confidence = interval.confidence;
            run.targetIdHint = interval.targetIdHint;
            runs.append(run);
        }
    }
    // Deterministic ordering.
    std::stable_sort(runs.begin(), runs.end(), [](const Run &a, const Run &b) {
        if (a.startMs != b.startMs) {
            return a.startMs < b.startMs;
        }
        if (a.speakerId != b.speakerId) {
            return a.speakerId < b.speakerId;
        }
        return a.endMs < b.endMs;
    });

    // 3. Drop runs shorter than minSpeechMs.
    QList<Run> filtered;
    for (const Run &run : runs) {
        if (run.endMs - run.startMs >= config.minSpeechMs) {
            filtered.append(run);
        }
    }
    if (filtered.isEmpty()) {
        return {};
    }

    // 4. Sweep boundaries into raw state segments (ends before starts).
    struct Event
    {
        qint64 timeMs = 0;
        int delta = 0; // +1 start, -1 end
        int runIndex = -1;
    };
    QList<Event> events;
    for (int i = 0; i < filtered.size(); ++i) {
        events.append({ filtered.at(i).startMs, 1, i });
        events.append({ filtered.at(i).endMs, -1, i });
    }
    std::stable_sort(events.begin(), events.end(), [](const Event &a, const Event &b) {
        if (a.timeMs != b.timeMs) {
            return a.timeMs < b.timeMs;
        }
        return a.delta < b.delta; // -1 (end) before +1 (start)
    });

    QList<StateSegment> raw;
    QList<int> active;
    qint64 previousTime = events.first().timeMs;
    for (const Event &event : events) {
        if (event.timeMs > previousTime && !active.isEmpty()) {
            StateSegment segment;
            segment.startMs = previousTime;
            segment.endMs = event.timeMs;
            for (int index : active) {
                if (!segment.speakers.contains(filtered.at(index).speakerId)) {
                    segment.speakers.append(filtered.at(index).speakerId);
                }
                if (segment.targetIdHint.isEmpty()) {
                    segment.targetIdHint = filtered.at(index).targetIdHint;
                }
            }
            std::stable_sort(segment.speakers.begin(), segment.speakers.end());
            if (segment.speakers.size() > 1) {
                segment.targetIdHint.clear(); // no single speaker to attribute
            }
            if (!raw.isEmpty() && raw.last().endMs == segment.startMs
                && raw.last().speakers == segment.speakers) {
                raw.last().endMs = segment.endMs;
            } else {
                raw.append(segment);
            }
        }
        previousTime = event.timeMs;
        if (event.delta > 0) {
            active.append(event.runIndex);
        } else {
            active.removeAll(event.runIndex);
        }
    }

    // 5. Hysteresis.
    QList<SpeakerSegment> output;
    for (const StateSegment &segment : raw) {
        const qint64 duration = segment.endMs - segment.startMs;
        if (segment.speakers.size() == 1) {
            const QString speaker = segment.speakers.first();
            if (!output.isEmpty()
                && output.last().verdict == SpeakerVerdict::Active
                && output.last().speakerId != speaker
                && duration < config.switchConfirmMs) {
                output.last().endMs = segment.endMs;
                continue;
            }
            SpeakerSegment result;
            result.startMs = segment.startMs;
            result.endMs = segment.endMs;
            result.speakerId = speaker;
            result.verdict = SpeakerVerdict::Active;
            result.targetIdHint = segment.targetIdHint;
            result.confidence =
                confidenceFor(filtered, segment.speakers, segment.startMs,
                              segment.endMs);
            output.append(result);
        } else if (segment.speakers.size() > 1) {
            if (!output.isEmpty() && duration < config.switchConfirmMs
                && output.last().verdict == SpeakerVerdict::Active) {
                output.last().endMs = segment.endMs;
                continue;
            }
            SpeakerSegment result;
            result.startMs = segment.startMs;
            result.endMs = segment.endMs;
            result.speakerId.clear();
            result.verdict = SpeakerVerdict::Overlap;
            result.confidence =
                confidenceFor(filtered, segment.speakers, segment.startMs,
                              segment.endMs);
            output.append(result);
        } else {
            if (!output.isEmpty() && duration < config.holdMs
                && output.last().verdict != SpeakerVerdict::Silence) {
                output.last().endMs = segment.endMs;
                continue;
            }
            SpeakerSegment result;
            result.startMs = segment.startMs;
            result.endMs = segment.endMs;
            result.verdict = SpeakerVerdict::Silence;
            output.append(result);
        }
    }

    // Coalesce adjacent segments with the same verdict and speaker so that a
    // held speaker becomes one deterministic segment.
    QList<SpeakerSegment> coalesced;
    for (const SpeakerSegment &segment : output) {
        if (!coalesced.isEmpty()
            && coalesced.last().verdict == segment.verdict
            && coalesced.last().speakerId == segment.speakerId
            && coalesced.last().endMs >= segment.startMs) {
            coalesced.last().endMs = qMax(coalesced.last().endMs, segment.endMs);
            coalesced.last().confidence =
                qMax(coalesced.last().confidence, segment.confidence);
            if (coalesced.last().targetIdHint.isEmpty()) {
                coalesced.last().targetIdHint = segment.targetIdHint;
            }
        } else {
            coalesced.append(segment);
        }
    }
    return coalesced;
}

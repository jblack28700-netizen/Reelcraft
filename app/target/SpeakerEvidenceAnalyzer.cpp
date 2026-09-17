#include "target/SpeakerEvidenceAnalyzer.h"

#include <algorithm>

namespace {

QList<TargetTrack> visiblePersonTracks(const QList<TargetTrack> &tracks,
                                       const QString &label, qint64 startMs,
                                       qint64 endMs)
{
    QList<TargetTrack> visible;
    for (const TargetTrack &track : tracks) {
        if (track.isEmpty() || !track.active()) {
            continue;
        }
        if (!label.isEmpty()
            && track.label().compare(label, Qt::CaseInsensitive) != 0) {
            continue;
        }
        if (track.lastTimeMs() < startMs || track.firstTimeMs() > endMs) {
            continue;
        }
        visible.append(track);
    }
    return visible;
}

} // namespace

SpeakerEvidenceAnalyzer::SpeakerEvidenceAnalyzer(Config config)
    : m_config(std::move(config))
{
}

SpeakerEvidenceAnalyzer::Config SpeakerEvidenceAnalyzer::config() const
{
    return m_config;
}

void SpeakerEvidenceAnalyzer::setConfig(const Config &config)
{
    m_config = config;
}

SpeakerEvidenceAnalyzer::Result SpeakerEvidenceAnalyzer::analyze(
    const QString &mediaPath, qint64 startMs, qint64 endMs,
    const QList<TargetTrack> &tracks, SpeakerEvidenceProvider *provider,
    const SpeakerTargetAssociator &associator) const
{
    Result result;
    if (!provider) {
        result.notes.append(QStringLiteral(
            "No speaker provider; audio evidence unavailable."));
        return result;
    }

    QString providerError;
    if (!provider->analyze(mediaPath, startMs, endMs, &result.analysis,
                           &providerError)) {
        result.notes.append(QStringLiteral("Speaker analysis failed: %1")
                                 .arg(providerError));
        result.analysis.available = false;
        return result;
    }

    if (!result.analysis.available) {
        result.notes.append(QStringLiteral(
            "Speaker analysis unavailable: %1")
                                .arg(result.analysis.error.isEmpty()
                                         ? QStringLiteral("no reason given")
                                         : result.analysis.error));
        return result;
    }

    result.segments =
        SpeakerTimeline::build(result.analysis.intervals, m_config.timeline);
    if (result.segments.isEmpty()) {
        result.notes.append(QStringLiteral("No speech detected in the range."));
        return result;
    }

    QList<SpeakerEvidence> perTarget;
    for (int segmentIndex = 0; segmentIndex < result.segments.size();
         ++segmentIndex) {
        const SpeakerSegment segment = result.segments.at(segmentIndex);
        if (segment.verdict != SpeakerVerdict::Active) {
            if (segment.verdict == SpeakerVerdict::Overlap) {
                SpeakerEvidence evidence;
                evidence.verdict = SpeakerVerdict::Overlap;
                evidence.confidence = segment.confidence;
                evidence.startMs = segment.startMs;
                evidence.endMs = segment.endMs;
                evidence.provider = result.analysis.provider;
                evidence.detail = QStringLiteral("overlapping speech");
                result.evidence.append(evidence);
            }
            continue;
        }

        const QList<TargetTrack> visible = visiblePersonTracks(
            tracks, m_config.association.label, segment.startMs, segment.endMs);
        QString targetId;
        bool ambiguous = false;
        QString method;
        QString associationError;
        const SpeakerInterval interval{ segment.startMs, segment.endMs,
                                        segment.speakerId, segment.confidence,
                                        false, 0.0, false };
        associator.associate(interval, visible, m_config.association, &targetId,
                             &ambiguous, &method, &associationError);

        SpeakerEvidence evidence;
        evidence.speakerId = segment.speakerId;
        evidence.confidence = segment.confidence;
        evidence.startMs = segment.startMs;
        evidence.endMs = segment.endMs;
        evidence.provider = result.analysis.provider;
        if (!targetId.isEmpty()) {
            evidence.targetId = targetId;
            evidence.verdict = SpeakerVerdict::Active;
            evidence.detail = QStringLiteral("associated by %1").arg(method);
            // Record the association on the segment so the deterministic
            // speaker-follow planner can consume it.
            result.segments[segmentIndex].targetId = targetId;
            perTarget.append(evidence);
        } else if (ambiguous) {
            evidence.verdict = SpeakerVerdict::Ambiguous;
            evidence.detail = QStringLiteral(
                "multiple visible people; speaker not attributed");
            result.evidence.append(evidence);
        } else {
            evidence.verdict = SpeakerVerdict::Unassociated;
            evidence.detail = QStringLiteral(
                "speech has no visible person; no target invented");
            result.evidence.append(evidence);
        }
    }

    // Keep the strongest Active evidence per target (deterministic).
    QList<SpeakerEvidence> strongest;
    for (const SpeakerEvidence &evidence : perTarget) {
        bool replaced = false;
        for (SpeakerEvidence &existing : strongest) {
            if (existing.targetId != evidence.targetId) {
                continue;
            }
            if (evidence.confidence > existing.confidence) {
                existing = evidence;
            }
            replaced = true;
            break;
        }
        if (!replaced) {
            strongest.append(evidence);
        }
    }
    result.evidence.append(strongest);

    std::stable_sort(result.evidence.begin(), result.evidence.end(),
                     [](const SpeakerEvidence &a, const SpeakerEvidence &b) {
                         if (a.targetId != b.targetId) {
                             return a.targetId < b.targetId;
                         }
                         if (a.startMs != b.startMs) {
                             return a.startMs < b.startMs;
                         }
                         return speakerVerdictToString(a.verdict)
                             < speakerVerdictToString(b.verdict);
                     });

    if (strongest.isEmpty()) {
        result.notes.append(QStringLiteral(
            "Speech detected, but no visible target could be associated."));
    } else {
        result.notes.append(QStringLiteral(
            "Associated speech with %1 visible target(s).")
                                .arg(strongest.size()));
    }
    return result;
}

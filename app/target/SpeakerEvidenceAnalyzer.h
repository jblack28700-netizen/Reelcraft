#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "target/SpeakerEvidenceProvider.h"
#include "target/SpeakerTargetAssociator.h"
#include "target/SpeakerTimeline.h"
#include "target/TargetTypes.h"

// SpeakerEvidenceAnalyzer is the optional audio/speaker orchestration layer. It
// runs a replaceable SpeakerEvidenceProvider, applies the deterministic timeline
// hysteresis, associates speakerIds with visible target tracks, and produces
// structured per-target evidence. It never mutates identity resolution and
// never invents a target: speech that maps to no visible person is reported
// Unassociated.
class SpeakerEvidenceAnalyzer
{
public:
    struct Config
    {
        SpeakerTimeline::Config timeline;
        SpeakerTargetAssociator::Config association;
    };

    struct Result
    {
        SpeakerAnalysis analysis;
        QList<SpeakerSegment> segments;
        QList<SpeakerEvidence> evidence;
        QStringList notes;
    };

    SpeakerEvidenceAnalyzer() = default;
    explicit SpeakerEvidenceAnalyzer(Config config);

    Config config() const;
    void setConfig(const Config &config);

    Result analyze(const QString &mediaPath, qint64 startMs, qint64 endMs,
                   const QList<TargetTrack> &tracks,
                   SpeakerEvidenceProvider *provider,
                   const SpeakerTargetAssociator &associator) const;

private:
    Config m_config;
};

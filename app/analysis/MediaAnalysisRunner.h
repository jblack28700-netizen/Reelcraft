#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>

#include "analysis/MediaAnalysis.h"
#include "core/MediaItem.h"
#include "media/MediaDurationProbe.h"
#include "target/TargetDetector.h"
#include "target/TargetResolver.h"
#include "target/TargetTypes.h"

// MediaAnalysisRunner (Objective 21) performs ONE whole-video analysis pass and
// produces one MediaAnalysis artifact.
//
// PERFORMANCE CONTRACT (this is the reason the class exists at all):
//   - exactly ONE persistent visual decoder process per analysis run, opened at
//     the recorded PERCEPTION resolution -- never the source resolution, and
//     never one process per sampled frame (Objective 20 removed that pattern
//     from the render path and it never belonged here);
//   - frames are decoded strictly sequentially and sampled on a configured
//     interval, so the decoder's work is bounded by the scope, not by the
//     number of capabilities;
//   - audio-only capabilities never open the visual decoder.
//
// The perception resolution is deliberately its own concept. It is NOT the
// viewer's 1024x512 display proxy (chosen for display latency) and NOT the
// source resolution (a single 4K frame costs seconds to decode here). Both the
// resolution and the sampling interval are persisted per layer, because an
// observation is only comparable with another made at the same settings.
//
// The runner composes existing pieces and invents no perception of its own:
// the target capability is the existing TargetResolver + SphericalTargetTracker
// over tangent views, and the technical capability is the existing ffprobe
// probe seam. Nothing here replaces the spherical geometry or the tracker.
class MediaAnalysisRunner
{
public:
    struct Config
    {
        // Equirectangular frame size the visual capabilities are computed at.
        // A 2:1 geometry keeps the spherical mapping valid.
        int perceptionWidth = 960;
        int perceptionHeight = 480;
        // Temporal step between analyzed samples.
        qint64 sampleIntervalMs = 1000;
        // Upper bound on samples in one run, so a long source cannot turn an
        // analysis pass into an unbounded job. Truncation is reported.
        int maxSamples = 240;
        // Reused unchanged from the existing resolver.
        TargetResolveConfig resolve;
        // What to look for. An empty label means "any detectable target".
        TargetQuery targetQuery;
        // Per-frame decode budget for the persistent stream.
        int decodeTimeoutMs = 30000;
    };

    // Replaceable seams. Every one of them is optional: a missing provider must
    // produce an explicitly Unavailable layer, never a silently empty one.
    struct Seams
    {
        // null => the ffprobe probe is used when it is available.
        MediaDurationProbe *probe = nullptr;
        // null => the targets capability is recorded as Unavailable.
        TargetDetector *detector = nullptr;
        // empty => the default ffmpeg executable is resolved.
        QString ffmpegExecutable;
    };

    struct Result
    {
        bool ok = false;
        MediaAnalysis analysis;
        QString error;
        int samplesRequested = 0;
        int samplesAnalyzed = 0;
        // One per analysis run. A value greater than 1 means the persistence
        // contract was violated, so it is surfaced rather than hidden.
        int decoderOpens = 0;
        qint64 decodedFrames = 0;
        bool scopeTruncated = false;
    };

    MediaAnalysisRunner() = default;
    explicit MediaAnalysisRunner(Config config);

    Config config() const { return m_config; }
    void setConfig(const Config &config) { m_config = config; }

    struct Request
    {
        MediaItem media;
        // Scope. endMs <= startMs means "the whole source", resolved from the
        // probed duration.
        qint64 startMs = 0;
        qint64 endMs = 0;
        // Invalid => the current UTC time. Injectable so tests stay deterministic.
        QDateTime createdUtc;
    };

    Result run(const Request &request, const Seams &seams) const;

    // The canonical description of an analysis specification. It is a pure
    // function of the configuration and the provider identities, so a caller can
    // recompute it later and ask whether a stored artifact is still current.
    QJsonObject specificationDescription(const Seams &seams) const;
    QString specificationHash(const Seams &seams) const;

    // Evenly spaced sample timestamps across [startMs, endMs], inclusive of both
    // ends. *truncated is set when maxSamples stopped the sequence before the
    // end of the scope, which is what makes a layer honestly Partial.
    static QList<qint64> deriveSampleTimestamps(qint64 startMs, qint64 endMs,
                                                qint64 intervalMs, int maxSamples,
                                                bool *truncated);

    // Target-track payload conversion for the "targets" layer. Exposed so the
    // persisted shape is testable without running an analysis.
    static QJsonObject targetTrackToJson(const TargetTrack &track);
    static bool targetTrackFromJson(const QJsonObject &object, TargetTrack *out,
                                    QString *error = nullptr);

private:
    Config m_config;
};

// The layer kind tag for a single target track observation.
QString targetTrackObservationKind();


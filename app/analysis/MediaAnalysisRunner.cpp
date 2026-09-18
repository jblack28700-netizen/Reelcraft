#include "analysis/MediaAnalysisRunner.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonValue>

#include <cmath>
#include <memory>

#include "media/FfmpegFrameSource.h"
#include "media/FfprobeDurationProbe.h"
#include "media/FrameExtractor.h"
#include "media/FrameSource.h"
#include "reframe/ReframeFrameProvider.h"

namespace {

// Sub-millisecond guard, matching the convention the render path uses when it
// decides whether a held frame still answers a request.
constexpr double kTimeEpsilonMs = 0.5;

QString observationKindMediaFacts()
{
    return QStringLiteral("media-facts");
}

// One persistent sequential decoder for a whole analysis run.
//
// A persistent stream is sequential while ReframeFrameProvider is random
// access. The driver requests strictly non-decreasing sample timestamps, so the
// reconciliation is a cursor: advance by whole source frames until the stream
// reaches the request, then hand over that frame. No re-open, no seek, and
// therefore no process per sample.
//
// The cursor starts at the requested scope start: the input seek (-ss before -i)
// delivers the first frame at or after that timestamp, which is what the cursor
// nominally represents. That is an approximation of at most one frame, it is
// deterministic, and it is the same one the Objective 20 streaming provider
// makes; it is recorded here rather than hidden.
class SequentialAnalysisFrameProvider : public ReframeFrameProvider
{
public:
    bool open(const QString &path, int width, int height, qint64 startMs,
              double frameStepMs, int timeoutMs, QString *error)
    {
        m_stream = std::make_unique<FfmpegFrameSource>();
        if (!m_stream->open(path, width, height, startMs, false)) {
            if (error) {
                *error = m_stream->errorString();
            }
            m_stream.reset();
            return false;
        }
        ++m_opens;
        m_stepMs = frameStepMs;
        m_nextFrameMs = static_cast<double>(startMs);
        m_timeoutMs = timeoutMs;
        return true;
    }

    bool frameAt(qint64 timeMs, QImage *outFrame, QString *error) override
    {
        if (error) {
            error->clear();
        }
        if (!outFrame) {
            ++m_failedSamples;
            if (error) {
                *error = QStringLiteral("Analysis frame output is null.");
            }
            return false;
        }
        if (!m_stream || !m_stream->isOpen()) {
            ++m_failedSamples;
            if (error) {
                *error = QStringLiteral("The analysis stream is not open.");
            }
            return false;
        }

        // Advance to the first stream frame at or after the request.
        while (m_nextFrameMs < static_cast<double>(timeMs) - kTimeEpsilonMs) {
            QImage discarded;
            if (!readOne(&discarded, error)) {
                ++m_failedSamples;
                return false;
            }
            m_nextFrameMs += m_stepMs;
        }

        QImage frame;
        if (!readOne(&frame, error)) {
            ++m_failedSamples;
            return false;
        }
        m_nextFrameMs += m_stepMs;
        *outFrame = frame;
        return true;
    }

    void close()
    {
        if (m_stream) {
            m_stream->close();
            m_stream.reset();
        }
    }

    int opens() const { return m_opens; }
    qint64 decoded() const { return m_decoded; }
    int failedSamples() const { return m_failedSamples; }

private:
    bool readOne(QImage *outFrame, QString *error)
    {
        FrameSource::ReadResult result = FrameSource::ReadResult::Error;
        QImage frame;
        if (!m_stream->readNextFrame(m_timeoutMs, &result, &frame)) {
            if (error) {
                *error = result == FrameSource::ReadResult::EndOfStream
                    ? QStringLiteral(
                          "The analysis stream reached the end of the media.")
                    : m_stream->errorString();
            }
            return false;
        }
        ++m_decoded;
        *outFrame = frame;
        return true;
    }

    std::unique_ptr<FfmpegFrameSource> m_stream;
    double m_stepMs = 0.0;
    double m_nextFrameMs = 0.0;
    int m_timeoutMs = 30000;
    int m_opens = 0;
    qint64 m_decoded = 0;
    int m_failedSamples = 0;
};

MediaSourceReference sourceReferenceFor(const MediaItem &media)
{
    MediaSourceReference source;
    source.mediaId = media.id();
    source.path = media.path();
    source.sizeBytes = media.sizeBytes();
    source.lastModifiedUtc = media.lastModifiedUtc();
    // contentSha256 is deliberately left empty: the cheap fingerprint is always
    // available, and hashing multi-gigabyte footage is not free.
    return source;
}

} // namespace

QString targetTrackObservationKind()
{
    return QStringLiteral("target-track");
}

MediaAnalysisRunner::MediaAnalysisRunner(Config config) : m_config(config) {}

QJsonObject MediaAnalysisRunner::specificationDescription(const Seams &seams) const
{
    QJsonObject specification;
    specification.insert(QStringLiteral("perceptionWidth"), m_config.perceptionWidth);
    specification.insert(QStringLiteral("perceptionHeight"), m_config.perceptionHeight);
    specification.insert(QStringLiteral("sampleIntervalMs"),
                         static_cast<double>(m_config.sampleIntervalMs));
    specification.insert(QStringLiteral("maxSamples"), m_config.maxSamples);

    QJsonArray capabilities;

    // Provider identities are part of the specification: swapping a model
    // changes what an observation means, so it must invalidate a stored artifact
    // rather than silently reuse it.
    FfprobeDurationProbe defaultProbe;
    QJsonObject technicalCapability;
    technicalCapability.insert(QStringLiteral("kind"),
                               MediaAnalysis::technicalLayerKind());
    technicalCapability.insert(
        QStringLiteral("provider"),
        seams.probe ? seams.probe->name() : defaultProbe.name());
    capabilities.append(technicalCapability);

    QJsonObject targetsCapability;
    targetsCapability.insert(QStringLiteral("kind"),
                             MediaAnalysis::targetsLayerKind());
    targetsCapability.insert(QStringLiteral("provider"),
                             seams.detector ? seams.detector->name()
                                            : QStringLiteral("none"));
    targetsCapability.insert(QStringLiteral("label"), m_config.targetQuery.label);
    targetsCapability.insert(
        QStringLiteral("minConfidence"),
        qMax(m_config.targetQuery.minConfidence, m_config.resolve.minConfidence));
    targetsCapability.insert(QStringLiteral("yawCount"),
                             m_config.resolve.viewPlan.yawCount);
    targetsCapability.insert(QStringLiteral("pitchCount"),
                             m_config.resolve.viewPlan.pitchCount);
    targetsCapability.insert(QStringLiteral("viewWidth"),
                             m_config.resolve.viewPlan.viewWidth);
    targetsCapability.insert(QStringLiteral("viewHeight"),
                             m_config.resolve.viewPlan.viewHeight);
    targetsCapability.insert(QStringLiteral("fieldOfViewDeg"),
                             m_config.resolve.viewPlan.fieldOfViewDeg);
    capabilities.append(targetsCapability);

    specification.insert(QStringLiteral("capabilities"), capabilities);
    return specification;
}

QString MediaAnalysisRunner::specificationHash(const Seams &seams) const
{
    return MediaAnalysis::specHashFor(specificationDescription(seams));
}

QList<qint64> MediaAnalysisRunner::deriveSampleTimestamps(qint64 startMs,
                                                          qint64 endMs,
                                                          qint64 intervalMs,
                                                          int maxSamples,
                                                          bool *truncated)
{
    if (truncated) {
        *truncated = false;
    }
    QList<qint64> timestamps;
    if (endMs <= startMs || intervalMs <= 0 || maxSamples <= 0) {
        return timestamps;
    }

    for (qint64 timeMs = startMs; timeMs <= endMs; timeMs += intervalMs) {
        if (timestamps.size() >= maxSamples) {
            // The budget stopped the sequence before the end of the declared
            // scope. That is what makes the layer honestly Partial rather than
            // silently claiming the whole span.
            if (truncated) {
                *truncated = true;
            }
            return timestamps;
        }
        timestamps.append(timeMs);
    }

    // Cover the end of the scope when it does not fall on the sampling grid.
    if (!timestamps.isEmpty() && timestamps.last() != endMs
        && timestamps.size() < maxSamples) {
        timestamps.append(endMs);
    }
    return timestamps;
}

MediaAnalysisRunner::Result MediaAnalysisRunner::run(const Request &request,
                                                     const Seams &seams) const
{
    Result result;

    if (!request.media.isValid()) {
        result.error = QStringLiteral("Media analysis requires a valid media record.");
        return result;
    }
    const QString sourcePath = request.media.path();
    const QFileInfo sourceInfo(sourcePath);
    if (sourcePath.isEmpty() || !sourceInfo.exists() || !sourceInfo.isFile()) {
        result.error = QStringLiteral("The media file does not exist: %1")
                           .arg(sourcePath.isEmpty()
                                    ? QStringLiteral("(empty path)")
                                    : sourcePath);
        return result;
    }

    MediaAnalysis analysis;
    analysis.setSource(sourceReferenceFor(request.media));
    analysis.setCreatedUtc(request.createdUtc.isValid()
                               ? request.createdUtc
                               : QDateTime::currentDateTimeUtc());
    analysis.setAnalysisSpecHash(specificationHash(seams));

    FfprobeDurationProbe localProbe;
    const bool usingDefaultProbe = seams.probe == nullptr;
    MediaDurationProbe *probe = usingDefaultProbe ? &localProbe : seams.probe;

    // --- Technical capability ------------------------------------------------
    // Deterministic, no model, and the only capability that is always attempted.
    qint64 durationMs = 0;
    double fps = 0.0;
    MediaDurationProbe::StreamSummary summary;
    QString durationError;
    QString frameRateError;
    QString summaryError;
    const bool hasDuration =
        probe->durationMs(sourcePath, &durationMs, &durationError);
    const bool hasFps = probe->frameRate(sourcePath, &fps, &frameRateError);
    const bool hasSummary =
        probe->streamSummary(sourcePath, &summary, &summaryError);

    MediaAnalysis::Layer technical;
    technical.kind = MediaAnalysis::technicalLayerKind();
    technical.provider.name = probe->name();

    const bool probeMissing = usingDefaultProbe
        && FfprobeDurationProbe::defaultExecutablePath().isEmpty();
    if (probeMissing) {
        technical.state = MediaAnalysis::LayerState::Unavailable;
        technical.error = QStringLiteral(
            "No media probe is available in this environment (ffprobe was not "
            "found).");
    } else if (!hasDuration && !hasFps && !hasSummary) {
        // The probe exists but could not describe the file: that is a failure,
        // not an unavailable capability, and it is reported as one.
        technical.state = MediaAnalysis::LayerState::Failed;
        technical.error = durationError.isEmpty()
            ? QStringLiteral("The media probe could not describe the source.")
            : durationError;
    } else {
        QJsonObject facts;
        facts.insert(QStringLiteral("kind"), observationKindMediaFacts());
        QJsonArray unavailable;

        if (hasDuration) {
            facts.insert(QStringLiteral("durationMs"),
                         static_cast<double>(durationMs));
        } else {
            unavailable.append(QStringLiteral("duration"));
        }
        if (hasFps && fps > 0.0) {
            facts.insert(QStringLiteral("frameRate"), fps);
        } else {
            unavailable.append(QStringLiteral("frameRate"));
        }
        if (hasSummary) {
            facts.insert(QStringLiteral("hasVideo"), summary.hasVideo);
            if (summary.hasVideo && summary.videoWidth > 0
                && summary.videoHeight > 0) {
                facts.insert(QStringLiteral("width"), summary.videoWidth);
                facts.insert(QStringLiteral("height"), summary.videoHeight);
                facts.insert(QStringLiteral("aspectRatio"),
                             static_cast<double>(summary.videoWidth)
                                 / static_cast<double>(summary.videoHeight));
            }
            facts.insert(QStringLiteral("hasAudio"), summary.hasAudio);
            if (summary.hasAudio) {
                if (summary.audioSampleRate > 0) {
                    facts.insert(QStringLiteral("audioSampleRate"),
                                 summary.audioSampleRate);
                }
                if (summary.audioChannels > 0) {
                    facts.insert(QStringLiteral("audioChannels"),
                                 summary.audioChannels);
                }
            }
        } else {
            unavailable.append(QStringLiteral("streams"));
        }

        const QString projection =
            MediaItem::projectionToString(request.media.projection());
        facts.insert(QStringLiteral("projection"),
                     projection.isEmpty() ? QStringLiteral("unknown")
                                          : projection);
        if (projection == QStringLiteral("equirectangular")) {
            // Observations are only interpretable relative to a frame
            // convention, so the convention is recorded with them.
            facts.insert(QStringLiteral("frameConvention"),
                         MediaAnalysis::equirectFrameConvention());
        }
        if (!unavailable.isEmpty()) {
            // Facts that could not be determined are named, never defaulted.
            facts.insert(QStringLiteral("unavailable"), unavailable);
        }
        technical.observations.append(facts);
        technical.state = MediaAnalysis::LayerState::Complete;
        if (hasDuration && durationMs > 0) {
            MediaAnalysis::TimeRange range;
            range.startMs = 0;
            range.endMs = durationMs;
            technical.coverage.append(range);
        }
    }
    analysis.setLayer(technical);

    // --- Analysis scope ------------------------------------------------------
    qint64 scopeStart = request.startMs;
    qint64 scopeEnd = 0;
    if (request.endMs > request.startMs) {
        scopeEnd = request.endMs;
        if (hasDuration && durationMs > 0 && scopeEnd > durationMs) {
            scopeEnd = durationMs;
        }
    } else if (hasDuration && durationMs > 0) {
        scopeEnd = durationMs;
    }

    // The probed duration is the END of the last frame, not a timestamp that
    // exists: for a 2 s 10 fps clip the frames are at 0..1900 ms, so sampling at
    // 2000 ms asks for a frame that was never encoded. When the frame rate is
    // known the scope is clamped to the last decodable timestamp, which keeps
    // coverage truthful instead of reporting a sample the stream cannot serve.
    if (hasFps && fps > 0.0 && scopeEnd > scopeStart) {
        const qint64 frameStepMs = static_cast<qint64>(std::llround(1000.0 / fps));
        if (frameStepMs > 0) {
            const qint64 lastUsableMs = scopeEnd - frameStepMs;
            if (lastUsableMs > scopeStart) {
                scopeEnd = lastUsableMs;
            }
        }
    }

    // --- Target capability ---------------------------------------------------
    MediaAnalysis::Layer targets;
    targets.kind = MediaAnalysis::targetsLayerKind();
    targets.spec.perceptionWidth = m_config.perceptionWidth;
    targets.spec.perceptionHeight = m_config.perceptionHeight;
    targets.spec.sampleIntervalMs = m_config.sampleIntervalMs;
    targets.minConfidence =
        qMax(m_config.targetQuery.minConfidence, m_config.resolve.minConfidence);

    const QString projection =
        MediaItem::projectionToString(request.media.projection());

    if (!seams.detector) {
        // A capability that cannot run here is UNAVAILABLE with a reason. It is
        // never an empty successful layer, because "no people found" and "no
        // detector configured" are completely different statements.
        targets.state = MediaAnalysis::LayerState::Unavailable;
        targets.error = QStringLiteral(
            "No target detector is configured for this environment, so the "
            "targets capability could not run.");
    } else if (projection == QStringLiteral("flat")) {
        targets.state = MediaAnalysis::LayerState::Unavailable;
        targets.error = QStringLiteral(
            "Target resolution requires a 360 equirectangular source; this media "
            "is declared flat.");
        targets.provider.name = seams.detector->name();
    } else if (scopeEnd <= scopeStart) {
        targets.state = MediaAnalysis::LayerState::Unavailable;
        targets.error = QStringLiteral(
            "The analysis scope could not be determined: the source duration is "
            "unknown and no explicit end time was supplied.");
        targets.provider.name = seams.detector->name();
    } else if (!hasFps || fps <= 0.0) {
        targets.state = MediaAnalysis::LayerState::Unavailable;
        targets.error = QStringLiteral(
            "The source frame rate is unknown, and sequential sampling cannot "
            "place samples on the source timeline without it.");
        targets.provider.name = seams.detector->name();
    } else {
        targets.provider.name = seams.detector->name();

        bool truncated = false;
        const QList<qint64> timestamps =
            deriveSampleTimestamps(scopeStart, scopeEnd,
                                   m_config.sampleIntervalMs, m_config.maxSamples,
                                   &truncated);
        result.samplesRequested = timestamps.size();
        result.scopeTruncated = truncated;

        const QString ffmpeg = seams.ffmpegExecutable.isEmpty()
            ? FrameExtractor::defaultExecutablePath()
            : seams.ffmpegExecutable;

        SequentialAnalysisFrameProvider provider;
        QString openError;
        if (!provider.open(sourcePath, m_config.perceptionWidth,
                           m_config.perceptionHeight, scopeStart,
                           1000.0 / fps, m_config.decodeTimeoutMs, &openError)) {
            targets.state = MediaAnalysis::LayerState::Failed;
            targets.error = QStringLiteral("The analysis stream could not be "
                                           "opened: %1")
                                .arg(openError.isEmpty()
                                         ? QStringLiteral("unknown error")
                                         : openError);
        } else {
            TargetResolver resolver(m_config.resolve);
            QList<TargetTrack> tracks;
            QString resolveError;
            const bool resolved =
                resolver.resolveSequence(&provider, timestamps,
                                         m_config.targetQuery, seams.detector,
                                         &tracks, &resolveError);
            result.samplesAnalyzed =
                timestamps.size() - provider.failedSamples();
            if (result.samplesAnalyzed < 0) {
                result.samplesAnalyzed = 0;
            }

            if (!resolved) {
                targets.state = MediaAnalysis::LayerState::Failed;
                targets.error = resolveError.isEmpty()
                    ? QStringLiteral("Target resolution failed.")
                    : resolveError;
            } else {
                for (const TargetTrack &track : tracks) {
                    targets.observations.append(targetTrackToJson(track));
                }
                if (!timestamps.isEmpty()) {
                    // Coverage is the span the sampling covers, not a claim that
                    // every frame in it was examined; the sampling interval is
                    // persisted in the layer spec so that difference is explicit.
                    MediaAnalysis::TimeRange range;
                    range.startMs = scopeStart;
                    range.endMs = qMin(scopeEnd,
                                       timestamps.last()
                                           + m_config.sampleIntervalMs);
                    if (range.endMs <= range.startMs) {
                        range.endMs = qMin(scopeEnd, range.startMs + 1);
                    }
                    if (range.isValid()) {
                        targets.coverage.append(range);
                    }
                }
                const bool everySampleDecoded = provider.failedSamples() == 0;
                targets.state = (truncated || !everySampleDecoded)
                    ? MediaAnalysis::LayerState::Partial
                    : MediaAnalysis::LayerState::Complete;
                if (!everySampleDecoded) {
                    targets.error = QStringLiteral(
                        "%1 sample(s) could not be decoded and were skipped.")
                                        .arg(provider.failedSamples());
                }
            }
        }

        result.decoderOpens = provider.opens();
        result.decodedFrames = provider.decoded();
        provider.close();
    }
    analysis.setLayer(targets);

    if (result.decoderOpens > 1) {
        // Should be impossible: the performance contract is one persistent
        // decoder per run, and a violation is surfaced rather than hidden.
        result.error = QStringLiteral(
            "The analysis run opened more than one decoder, which violates the "
            "single-persistent-decoder contract.");
        return result;
    }

    QString validationError;
    if (!analysis.isValid(&validationError)) {
        result.error = validationError;
        return result;
    }
    result.analysis = analysis;
    result.ok = true;
    return result;
}

QJsonObject MediaAnalysisRunner::targetTrackToJson(const TargetTrack &track)
{
    QJsonObject object;
    object.insert(QStringLiteral("kind"), targetTrackObservationKind());
    object.insert(QStringLiteral("id"), track.id());
    object.insert(QStringLiteral("label"), track.label());
    object.insert(QStringLiteral("active"), track.active());
    object.insert(QStringLiteral("missCount"), track.missCount());
    object.insert(QStringLiteral("meanConfidence"), track.meanConfidence());

    // Spherical, viewpoint-independent coordinates only. Provider/view-pixel
    // coordinates are never persisted: they are meaningless once the covering
    // views change.
    QJsonArray observations;
    for (const TargetObservation &observation : track.observations()) {
        QJsonObject entry;
        entry.insert(QStringLiteral("timeMs"),
                     static_cast<double>(observation.timeMs));
        entry.insert(QStringLiteral("frameIndex"), observation.frameIndex);
        if (!observation.targetId.isEmpty()) {
            entry.insert(QStringLiteral("targetId"), observation.targetId);
        }
        if (!observation.label.isEmpty()) {
            entry.insert(QStringLiteral("label"), observation.label);
        }
        entry.insert(QStringLiteral("confidence"), observation.confidence);
        entry.insert(QStringLiteral("yawDeg"), observation.yawDeg);
        entry.insert(QStringLiteral("pitchDeg"), observation.pitchDeg);
        entry.insert(QStringLiteral("yawRadiusDeg"), observation.yawRadiusDeg);
        entry.insert(QStringLiteral("pitchRadiusDeg"), observation.pitchRadiusDeg);
        if (!observation.source.isEmpty()) {
            entry.insert(QStringLiteral("source"), observation.source);
        }
        observations.append(entry);
    }
    object.insert(QStringLiteral("observations"), observations);
    return object;
}

bool MediaAnalysisRunner::targetTrackFromJson(const QJsonObject &object,
                                              TargetTrack *out, QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };
    if (!out) {
        return fail(QStringLiteral("Target track output is null."));
    }

    const QString id = object.value(QStringLiteral("id")).toString();
    const QString label = object.value(QStringLiteral("label")).toString();
    if (id.isEmpty()) {
        return fail(QStringLiteral("A stored target track has no id."));
    }

    TargetTrack track(id, label);
    track.setActive(object.value(QStringLiteral("active")).toBool(true));
    track.setMissCount(object.value(QStringLiteral("missCount")).toInt());

    const QJsonValue observationsValue =
        object.value(QStringLiteral("observations"));
    if (!observationsValue.isUndefined() && !observationsValue.isNull()) {
        if (!observationsValue.isArray()) {
            return fail(QStringLiteral("Stored target observations are not a list."));
        }
        for (const QJsonValue &value : observationsValue.toArray()) {
            if (!value.isObject()) {
                continue;
            }
            const QJsonObject entry = value.toObject();
            TargetObservation observation;
            observation.timeMs = static_cast<qint64>(
                entry.value(QStringLiteral("timeMs")).toDouble());
            observation.frameIndex =
                entry.value(QStringLiteral("frameIndex")).toInt(-1);
            observation.targetId =
                entry.value(QStringLiteral("targetId")).toString();
            observation.label = entry.value(QStringLiteral("label")).toString();
            observation.confidence =
                entry.value(QStringLiteral("confidence")).toDouble();
            observation.yawDeg = entry.value(QStringLiteral("yawDeg")).toDouble();
            observation.pitchDeg = entry.value(QStringLiteral("pitchDeg")).toDouble();
            observation.yawRadiusDeg =
                entry.value(QStringLiteral("yawRadiusDeg")).toDouble();
            observation.pitchRadiusDeg =
                entry.value(QStringLiteral("pitchRadiusDeg")).toDouble();
            observation.source = entry.value(QStringLiteral("source")).toString();
            track.append(observation);
        }
    }

    *out = track;
    return true;
}


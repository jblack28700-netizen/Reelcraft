#pragma once

#include <QString>
#include <QtGlobal>

// MediaDurationProbe is the replaceable media-engine seam that reports the
// duration of a media file (Objective 10). It exists so range-less 360 commands
// can default to the whole clip without the application knowing anything about
// codecs.
//
// The probe never decodes frames and never modifies the source media. A
// concrete implementation may invoke an external tool (ffprobe) or a future
// linked media library; the application owns no codec dependency and callers
// treat an unavailable probe as "duration unknown".
class MediaDurationProbe
{
public:
    virtual ~MediaDurationProbe() = default;

    // Human-readable backend identity (diagnostics).
    virtual QString name() const = 0;

    // Duration in milliseconds. Returns false with a deterministic error when
    // the probe is unavailable, the file is invalid/missing, the process fails
    // or times out, or the value cannot be parsed; *outDurationMs is left
    // unmodified on failure.
    virtual bool durationMs(const QString &filePath, qint64 *outDurationMs,
                            QString *error = nullptr) = 0;

    // Optional: the video frame rate, used to pace playback at the source rate.
    // The default implementation reports "unknown" so that existing probes and
    // test doubles remain valid without change; a probe that can answer overrides
    // it. Returns false (leaving *outFps unmodified) when unknown.
    virtual bool frameRate(const QString &filePath, double *outFps,
                           QString *error = nullptr)
    {
        Q_UNUSED(filePath);
        Q_UNUSED(outFps);
        if (error) {
            *error = QStringLiteral("Frame rate is not reported by this probe.");
        }
        return false;
    }

    // Optional: which elementary streams the file contains and at what geometry
    // (Objective 21). The media-analysis "technical" layer persists these facts;
    // nothing here is inferred or defaulted, because a fabricated resolution is
    // worse than an honestly missing one.
    //
    // Same additive-seam rule as frameRate() above: the default implementation
    // reports "unavailable" so existing probes and test doubles stay valid, and a
    // probe that can answer overrides it.
    struct StreamSummary
    {
        bool hasVideo = false;
        int videoWidth = 0;
        int videoHeight = 0;
        bool hasAudio = false;
        int audioSampleRate = 0;
        int audioChannels = 0;

        // A summary is usable when it says something definite about the file.
        bool isValid() const { return hasVideo || hasAudio; }
    };

    virtual bool streamSummary(const QString &filePath, StreamSummary *out,
                               QString *error = nullptr)
    {
        Q_UNUSED(filePath);
        Q_UNUSED(out);
        if (error) {
            *error = QStringLiteral(
                "Media stream summary is not reported by this probe.");
        }
        return false;
    }

protected:
    MediaDurationProbe() = default;
};

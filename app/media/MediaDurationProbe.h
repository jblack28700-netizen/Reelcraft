#pragma once

#include <QString>

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

protected:
    MediaDurationProbe() = default;
};

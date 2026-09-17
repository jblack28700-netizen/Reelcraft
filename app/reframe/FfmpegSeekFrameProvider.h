#pragma once

#include "reframe/ReframeFrameProvider.h"

// FfmpegSeekFrameProvider decodes individual equirectangular source frames
// with the existing FFmpeg-CLI single-frame seam (FrameExtractor). It adds no
// new dependency and never modifies the source media.
//
// This is the first production provider: simple and deterministic, at the cost
// of one bounded FFmpeg invocation per requested frame. Streaming/sequential
// decoding can replace it behind the same seam without changing the reframing
// engine.
class FfmpegSeekFrameProvider : public ReframeFrameProvider
{
public:
    FfmpegSeekFrameProvider(QString sourcePath, QString ffmpegExecutable);

    bool frameAt(qint64 timeMs, QImage *outFrame,
                 QString *error = nullptr) override;

private:
    QString m_sourcePath;
    QString m_ffmpegExecutable;
};

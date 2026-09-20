#pragma once

#include <QImage>
#include <QList>
#include <QString>
#include <QStringList>

#include <functional>

#include "reframe/ReframeFrameProvider.h"
#include "reframe/ReframePlan.h"

// ReframeRenderer deterministically executes a ReframePlan: for every output
// frame it evaluates the camera path, decodes the equirectangular source frame,
// and renders a flat camera view through EquirectView.
//
// The renderer is pure orchestration over replaceable seams (frame provider)
// and existing primitives (CameraPath, EquirectView). It never writes to the
// source media. Every frame is produced from (plan, requested timestamp), so
// rendering is reproducible.
class ReframeRenderer
{
public:
    // Receives one rendered flat frame. Return false to stop with an error.
    using FrameSink = std::function<bool(int frameIndex, qint64 timeMs,
                                         const QImage &flatFrame)>;

    // Renders every frame of the plan, invoking sink in increasing frame
    // index order. Returns false and sets *error on the first failure.
    // *outFrameCount receives the number of frames rendered.
    static bool render(const ReframePlan &plan,
                       ReframeFrameProvider *provider,
                       const FrameSink &sink,
                       int *outFrameCount = nullptr,
                       QString *error = nullptr);

    // Renders every frame to outputDirectory/frame_00000.png (deterministic
    // zero-padded names). Returns the written paths in order.
    static bool renderToPngSequence(const ReframePlan &plan,
                                    ReframeFrameProvider *provider,
                                    const QString &outputDirectory,
                                    QStringList *outPaths = nullptr,
                                    QString *error = nullptr);

    // The printf-style image pattern used by the PNG sequence.
    static QString frameFileNamePattern();

    // Encodes a rendered PNG sequence into a video with FFmpeg. Uses H.264
    // (yuv420p) and pads odd dimensions to the next even size. Returns false
    // with a deterministic error when FFmpeg is unavailable or fails.
    static bool encodeVideo(const QString &ffmpegExecutable,
                            const QString &inputPattern,
                            double fps,
                            const QString &outputPath,
                            QString *error = nullptr);

    // 360 Reframing Objective 28 — the audio that accompanies the rendered
    // picture.
    //
    // The spans are the plan's retained SOURCE ranges, in output order (the
    // plan's ordered segments when it has them, otherwise its single source
    // range). Each span's audio is trimmed out of the source, its timestamps are
    // reset, and the spans are concatenated, so the output audio corresponds
    // exactly to the retained picture and begins at output time zero. The
    // source media is only ever read; nothing is stream-copied.
    struct AudioSpec
    {
        // Source media the audio is taken from. Required.
        QString sourcePath;
        // Ordered retained source spans. Required and non-empty; every span must
        // be valid, because a fabricated span would be fabricated audio.
        QList<ReframePlan::TimeRange> spans;
        // Length of the rendered output timeline in milliseconds. A positive
        // value bounds the concatenated audio to the picture, so a retained span
        // whose exact frame count rounds down can never leave an audio-only
        // tail. Zero or less leaves the concatenated audio unbounded.
        qint64 outputDurationMs = 0;
        // Source audio format, preserved when known (both > 0). Zero means "let
        // FFmpeg use the source's own value", which is how an uncommon layout is
        // preserved rather than approximated.
        int sampleRate = 0;
        int channels = 0;
    };

    // Encodes the rendered PNG sequence AND the source audio over the retained
    // spans into one video. The picture is produced by the UNCHANGED
    // encodeVideo() path and then remuxed with -c:v copy, so the video stream of
    // an output carrying audio is byte-identical to the video-only output of the
    // same frames; only the container gains a stream. The audio is re-encoded
    // with fixed parameters (never copied). Returns false with a deterministic
    // error when FFmpeg is unavailable, the spec is unusable, or either pass
    // fails — an output whose audio could not be produced is an error, never a
    // silent success.
    static bool encodeVideoWithAudio(const QString &ffmpegExecutable,
                                     const QString &inputPattern,
                                     double fps,
                                     const QString &outputPath,
                                     const AudioSpec &audio,
                                     QString *error = nullptr);
};

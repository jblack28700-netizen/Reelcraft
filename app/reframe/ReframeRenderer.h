#pragma once

#include <QImage>
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
};

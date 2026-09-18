#pragma once

#include <QImage>
#include <QString>

#include <memory>

#include "media/FfmpegFrameSource.h"
#include "reframe/FfmpegSeekFrameProvider.h"
#include "reframe/ReframeFrameProvider.h"

// ReframeStreamFrameProvider (Objective 20) supplies equirectangular source
// frames to the deterministic renderer from a PERSISTENT FFmpeg decoding
// process instead of one FFmpeg process per rendered frame.
//
// The rendering contract is unchanged: frameAt(timeMs) still returns the source
// frame at (or immediately after) the requested absolute source timestamp, and
// the renderer still decides the times, the camera state and the output.
//
// RECONCILING RANDOM ACCESS WITH SEQUENTIAL DECODING
//
// ReframeFrameProvider is a random-access interface; a persistent stream is
// sequential. The renderer's requests are in fact monotonically non-decreasing
// (ReframePlan::frameTimeMs() walks the retained segments in order), which is
// what makes streaming possible at all. The reconciliation is:
//
//   * ANCHORING. A request that cannot be served sequentially -- the first
//     request, a backwards jump, a jump further ahead than kMaxSequentialSpanMs,
//     or a stream that failed or ended -- is served by (re)opening the stream at
//     that exact timestamp. The stream's first frame is the frame at or after
//     the anchor timestamp, which is exactly what a seek would have returned.
//     Anchoring therefore costs one process, not one process per frame.
//   * SEQUENCING. After an anchor the provider holds a nominal cursor: the
//     timestamp of the frame it currently holds. A later request advances the
//     stream by whole frames (cursor += 1000 / sourceFps per frame) until the
//     cursor reaches the request. Requests within a sub-millisecond epsilon of
//     the cursor are served from the held frame, which keeps repeated timestamps
//     and sub-frame rounding stable while never serving a frame that is
//     materially earlier than the request.
//   * GEOMETRY. Rawvideo carries no frame dimensions, so the FIRST anchor uses
//     one seeked decode to obtain the frame at the anchor timestamp and, at the
//     same time, the native source geometry. Later anchors reuse it.
//   * NO APPROXIMATION FOR SPEED. Streaming is used only when the source frame
//     rate is known and the renderer is replaying a contiguous forward window.
//     Whenever any of those conditions does not hold the provider falls back to
//     the positioned seek path, which preserves the previous behaviour exactly.
//     The worst case is therefore the old cost, never different output.
class ReframeStreamFrameProvider : public ReframeFrameProvider
{
public:
    // sourceFps <= 0 means "unknown": the provider then behaves exactly like the
    // seek provider and no streaming is attempted.
    ReframeStreamFrameProvider(QString sourcePath, QString ffmpegExecutable,
                               double sourceFps);
    ~ReframeStreamFrameProvider() override;

    bool frameAt(qint64 timeMs, QImage *outFrame,
                 QString *error = nullptr) override;

    // Diagnostics, for tests and for demonstrating process behaviour.
    // Persistent streams opened: one per anchor (plus none extra after the
    // geometry-discovering first anchor).
    int streamOpenCount() const { return m_streamOpens; }
    // Requests served by anchoring (a positioned open) rather than by advancing
    // an already open stream.
    int anchorCount() const { return m_anchors; }
    // Seeked single-frame decodes. One for the whole session (geometry discovery
    // at the first anchor), or one per request when the frame rate is unknown.
    int seekDecodeCount() const { return m_seekDecodes; }
    // Frames delivered by advancing the persistent stream.
    int streamedFrameCount() const { return m_streamedFrames; }
    // Native source geometry discovered at the first anchor (0 until then).
    int sourceWidth() const { return m_width; }
    int sourceHeight() const { return m_height; }

private:
    bool ensureGeometry(qint64 timeMs, QImage *outFrame, QString *error);
    // skipFirstFrame: the stream restarts at the anchor frame, which the seek
    // path has ALREADY delivered, so the first streamed frame must be dropped.
    bool openStreamAt(qint64 timeMs, bool skipFirstFrame);
    bool readStreamFrame(QImage *outFrame, QString *error);
    bool readStreamFrameRaw(QImage *outFrame, QString *error);
    void closeStream();
    void holdFrame(const QImage &frame, double nominalMs);
    bool fail(const QString &message, QString *error);
    // Nominal timestamp of the first source frame at or after timeMs, for
    // constant-rate media whose frame grid starts at zero.
    double snapToSourceGrid(double timeMs) const;

    QString m_sourcePath;
    QString m_ffmpegExecutable;
    double m_sourceFps = 0.0;
    double m_frameStepMs = 0.0;

    FfmpegSeekFrameProvider m_seek;
    std::unique_ptr<FfmpegFrameSource> m_stream;

    int m_width = 0;
    int m_height = 0;
    QImage m_currentFrame;
    bool m_hasCurrent = false;
    double m_cursorMs = 0.0;
    double m_anchorMs = 0.0;

    int m_streamOpens = 0;
    int m_anchors = 0;
    int m_seekDecodes = 0;
    int m_streamedFrames = 0;
    bool m_streamNeedsAlignment = false;

    // QImage format delivered by the positioned seek path (learned from the
    // first anchor frame). The persistent stream delivers raw RGB888, so streamed
    // frames are normalized to this format. Without it the renderer would see a
    // different pixel layout than before and the rendered output would change.
    QImage::Format m_frameFormat = QImage::Format_Invalid;
};


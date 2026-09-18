#include "reframe/ReframeStreamFrameProvider.h"

#include <cmath>
#include <utility>

namespace {

// A forward window of at most this many milliseconds is replayed from one open
// stream. Beyond it the provider anchors again, which bounds both the nominal
// drift between the stream cursor and the true source timestamps and the amount
// of decoding needed to reach a far-forward request. Anchoring remains an
// O(seek points) cost, never O(frames).
constexpr qint64 kMaxSequentialSpanMs = 3000;

constexpr int kStreamReadTimeoutMs = 10000;

} // namespace

ReframeStreamFrameProvider::ReframeStreamFrameProvider(QString sourcePath,
                                                       QString ffmpegExecutable,
                                                       double sourceFps)
    : m_sourcePath(std::move(sourcePath))
    , m_ffmpegExecutable(std::move(ffmpegExecutable))
    , m_sourceFps(std::isfinite(sourceFps) && sourceFps > 0.0 ? sourceFps : 0.0)
    , m_frameStepMs(m_sourceFps > 0.0 ? 1000.0 / m_sourceFps : 0.0)
    , m_seek(m_sourcePath, m_ffmpegExecutable)
{
}

ReframeStreamFrameProvider::~ReframeStreamFrameProvider()
{
    closeStream();
}

bool ReframeStreamFrameProvider::fail(const QString &message, QString *error)
{
    if (error) {
        *error = message;
    }
    return false;
}

void ReframeStreamFrameProvider::closeStream()
{
    if (m_stream) {
        m_stream->close();
        m_stream.reset();
    }
}

void ReframeStreamFrameProvider::holdFrame(const QImage &frame, double nominalMs)
{
    m_currentFrame = frame;
    m_cursorMs = nominalMs;
    m_hasCurrent = true;
}

bool ReframeStreamFrameProvider::openStreamAt(qint64 timeMs, bool skipFirstFrame)
{
    closeStream();
    if (m_sourceFps <= 0.0 || m_width <= 0 || m_height <= 0) {
        return false;
    }
    std::unique_ptr<FfmpegFrameSource> stream = std::make_unique<FfmpegFrameSource>();
    // Native geometry, an input seek at the anchor timestamp, and the aspect
    // left untouched: the renderer must see the same source pixels as before.
    if (!stream->open(m_sourcePath, m_width, m_height, timeMs, false)) {
        return false;
    }
    m_stream = std::move(stream);
    m_anchorMs = static_cast<double>(timeMs);
    m_streamNeedsAlignment = skipFirstFrame;
    ++m_streamOpens;
    return true;
}

bool ReframeStreamFrameProvider::readStreamFrame(QImage *outFrame, QString *error)
{
    if (m_streamNeedsAlignment) {
        // The anchor frame was delivered by the seek path; the stream restarts
        // at that same frame, so the duplicate is dropped exactly once.
        QImage duplicate;
        QString alignmentError;
        if (!readStreamFrameRaw(&duplicate, &alignmentError)) {
            return fail(alignmentError, error);
        }
        m_streamNeedsAlignment = false;
    }
    QImage frame;
    if (!readStreamFrameRaw(&frame, error)) {
        return false;
    }
    // Match the positioned seek path exactly, so the renderer sees the same
    // pixel layout it always saw: the persistent stream delivers raw RGB888
    // while the seek path delivers the PNG-decoded format.
    if (m_frameFormat != QImage::Format_Invalid && frame.format() != m_frameFormat) {
        const QImage converted = frame.convertToFormat(m_frameFormat);
        if (!converted.isNull()) {
            frame = converted;
        } else {
            // A paletted target cannot represent real decoded video colours.
            // Never deliver a null frame: fall back to an equivalent non-paletted
            // layout that preserves the colours.
            frame = frame.convertToFormat(QImage::Format_ARGB32);
        }
    }
    if (frame.isNull()) {
        return fail(QStringLiteral("The source stream produced an unusable frame."), error);
    }
    *outFrame = frame;
    return true;
}

bool ReframeStreamFrameProvider::readStreamFrameRaw(QImage *outFrame, QString *error)
{
    if (!m_stream || !m_stream->isOpen()) {
        return fail(QStringLiteral("The persistent source stream is not open."), error);
    }
    FrameSource::ReadResult result = FrameSource::ReadResult::Error;
    QImage frame;
    if (!m_stream->readNextFrame(kStreamReadTimeoutMs, &result, &frame)) {
        if (result == FrameSource::ReadResult::EndOfStream) {
            return fail(
                QStringLiteral("The source stream reached the end of the media."),
                error);
        }
        const QString detail = m_stream->errorString();
        return fail(QStringLiteral("The source stream failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("decode error")
                                              : detail),
                    error);
    }
    if (frame.isNull()) {
        return fail(QStringLiteral("The source stream produced an empty frame."), error);
    }
    *outFrame = frame;
    return true;
}

bool ReframeStreamFrameProvider::ensureGeometry(qint64 timeMs, QImage *outFrame,
                                                QString *error)
{
    // One seeked decode gives both the frame at the anchor timestamp and, since
    // rawvideo carries no dimensions, the native source geometry.
    QImage frame;
    QString seekError;
    ++m_seekDecodes;
    if (!m_seek.frameAt(timeMs, &frame, &seekError)) {
        return fail(seekError, error);
    }
    m_width = frame.width();
    m_height = frame.height();
    m_frameFormat = frame.format();
    *outFrame = frame;
    return true;
}

double ReframeStreamFrameProvider::snapToSourceGrid(double timeMs) const
{
    if (m_frameStepMs <= 0.0) {
        return timeMs;
    }
    // A positioned seek returns the first source frame at or after the request,
    // which for constant-rate media lies on the next grid point. Snapping the
    // cursor there keeps it aligned with the true frame timestamps, so a
    // fractionally misaligned anchor cannot shift every later selection by one
    // frame. The small negative epsilon stops exact grid values rounding up.
    const double index = std::ceil(timeMs / m_frameStepMs - 1e-9);
    return index * m_frameStepMs;
}

bool ReframeStreamFrameProvider::frameAt(qint64 timeMs, QImage *outFrame,
                                         QString *error)
{
    if (error) {
        error->clear();
    }
    if (!outFrame) {
        return fail(QStringLiteral("Frame provider output is null."), error);
    }
    if (timeMs < 0) {
        return fail(QStringLiteral("Frame provider time must not be negative."), error);
    }

    // A sub-millisecond guard ONLY. The selection rule is "the frame at or after"
    // the requested timestamp, so a frame earlier than the request must never be
    // served merely because it is nearby.
    constexpr double kCursorEpsilonMs = 0.5;
    const double requestedMs = static_cast<double>(timeMs);

    // Serve the held frame when the request maps onto it (identical or repeated
    // timestamps, and sub-frame rounding).
    // Sequential replay is used only while the request stays within the bounded
    // forward window from the current anchor. A far-forward jump anchors instead,
    // which keeps decode-ahead bounded and re-aligns the cursor.
    const bool canAdvanceSequentially =
        m_hasCurrent && requestedMs >= m_cursorMs - kCursorEpsilonMs
        && (requestedMs - m_anchorMs) <= kMaxSequentialSpanMs;
    if (canAdvanceSequentially) {
        while (m_cursorMs < requestedMs - kCursorEpsilonMs) {
            QImage frame;
            QString streamError;
            if (!readStreamFrame(&frame, &streamError)) {
                // The stream cannot reach the request: reposition below rather
                // than guess. (End of media is reported by the anchor.)
                m_hasCurrent = false;
                break;
            }
            holdFrame(frame, m_cursorMs + m_frameStepMs);
            ++m_streamedFrames;
        }
        if (m_hasCurrent && requestedMs <= m_cursorMs + kCursorEpsilonMs) {
            *outFrame = m_currentFrame;
            return true;
        }
    }

    // ANCHOR: the first request, a backwards jump, a jump beyond the sequential
    // window, an unknown frame rate, or a stream that failed or ended.
    ++m_anchors;
    const bool withinSequentialWindow =
        m_hasCurrent && (requestedMs - m_anchorMs) <= kMaxSequentialSpanMs;

    if (m_width > 0 && m_height > 0 && m_sourceFps > 0.0
        && !withinSequentialWindow) {
        if (openStreamAt(timeMs, false)) {
            QImage frame;
            QString streamError;
            if (readStreamFrame(&frame, &streamError)) {
                // The stream's first frame IS the frame at or after the anchor
                // timestamp: exactly what a positioned seek would have returned.
                holdFrame(frame, snapToSourceGrid(requestedMs));
                ++m_streamedFrames;
                *outFrame = frame;
                return true;
            }
            closeStream();
        }
    }

    closeStream();

    // Positioned seek: the geometry-discovery path on the first anchor, the
    // behaviour for every request when the frame rate is unknown, and the
    // fallback whenever streaming could not serve the request.
    QImage frame;
    QString annotationError;
    if (m_width <= 0 || m_height <= 0) {
        if (!ensureGeometry(timeMs, &frame, error)) {
            return false;
        }
    } else {
        ++m_seekDecodes;
        if (!m_seek.frameAt(timeMs, &frame, &annotationError)) {
            return fail(annotationError, error);
        }
    }
    holdFrame(frame, snapToSourceGrid(requestedMs));
    // Never leave a stale stream positioned elsewhere: reposition it to this
    // anchor, skipping the duplicate of the frame just delivered.
    if (m_sourceFps > 0.0) {
        openStreamAt(timeMs, true);
    }
    *outFrame = frame;
    return true;
}


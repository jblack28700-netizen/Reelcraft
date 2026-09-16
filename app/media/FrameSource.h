#pragma once

#include <QImage>
#include <QString>

// FrameSource is the replaceable, transport-agnostic media/frame seam
// (Phase 3 Objective 3). It delivers decoded frames to consumers as QImage
// values and reports a distinct ReadResult. Implementations own all media
// decoding details (e.g., the persistent FFmpeg subprocess); consumers never
// see transport, geometry, or pixel-format concerns here.
//
// Opening is intentionally NOT part of this abstract contract because the
// configuration needed to open a source is implementation-specific (for
// example, rawvideo requires caller-supplied frame geometry). Callers open a
// concrete implementation and hand the opened source to consumers such as
// FramePump.
//
// This seam is the boundary a future backend (e.g., linked FFmpeg or
// QtMultimedia) can implement without changing Application/viewer contracts.
class FrameSource
{
public:
    enum class ReadResult {
        Ok,          // one frame was decoded and returned
        EndOfStream, // no more frames are available; process ended normally
        Error,       // decoding failed; see errorString()
        Timeout      // no frame arrived within the bounded wait
    };

    virtual ~FrameSource() = default;

    // Requests one frame with a bounded wait. On Ok, *outFrame receives a
    // decoded image. Never blocks longer than timeoutMs for a single frame.
    virtual bool readNextFrame(int timeoutMs, ReadResult *result,
                               QImage *outFrame) = 0;

    // Stops decoding and releases resources. Safe to call at any time and
    // multiple times; never leaves a process running.
    virtual void close() = 0;

    virtual bool isOpen() const = 0;

    // Last deterministic error message (valid after Error results).
    virtual QString errorString() const = 0;

protected:
    FrameSource() = default;
};

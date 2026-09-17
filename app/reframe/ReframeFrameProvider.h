#pragma once

#include <QImage>
#include <QString>

// ReframeFrameProvider is the replaceable seam that supplies equirectangular
// source frames to the reframing engine. It deliberately knows nothing about
// the camera path, the output specification, or rendering.
//
// A provider must never modify the original media. Implementations may seek,
// cache, or stream however they like; the engine only requires deterministic
// behavior for the same requested timestamp. Tests inject an in-memory
// synthetic provider so the entire camera path and rendering pipeline can be
// verified without any decoder.
class ReframeFrameProvider
{
public:
    virtual ~ReframeFrameProvider() = default;

    // Decodes/returns the equirectangular source frame for the given absolute
    // source timestamp (milliseconds). Returns false with a deterministic
    // error on failure and leaves *outFrame unmodified.
    virtual bool frameAt(qint64 timeMs, QImage *outFrame,
                         QString *error = nullptr) = 0;

protected:
    ReframeFrameProvider() = default;
};

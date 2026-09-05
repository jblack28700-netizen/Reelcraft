#pragma once

#include <QImage>
#include <QString>

// FrameExtractor is a minimal, deterministic single-frame decoder seam for
// Objective 9 (approved: FFmpeg-CLI decode adapter).
//
// It invokes the external `ffmpeg` executable (discovered at runtime, never
// linked) via QProcess to decode ONE frame of a media file to a PNG byte
// stream, which is then parsed in-process into a QImage. The ffmpeg executable
// path is injectable (and honors the REELCRAFT_FFMPEG environment variable)
// so tests are deterministic and the adapter is isolated behind this seam: a
// future media engine can replace the implementation without changing its
// callers or the viewer contract.
//
// This is NOT playback/streaming/audio and defines no media-engine contract.
class FrameExtractor
{
public:
    // Resolves the ffmpeg executable: REELCRAFT_FFMPEG override, then
    // QStandardPaths::findExecutable("ffmpeg"). Empty when not found.
    static QString defaultExecutablePath();

    // True when an ffmpeg executable is resolvable (override or PATH).
    static bool isAvailable();

    // Decodes the first frame of filePath and stores it into *outImage.
    // Returns false (with a deterministic error, and no output mutation) when
    // ffmpeg is unavailable, the file is invalid/missing, the process fails or
    // times out, or no frame can be decoded. The media file is never modified.
    static bool extractFirstFrame(const QString &filePath,
                                  const QString &executablePath,
                                  QImage *outImage,
                                  QString *error = nullptr);

    // Decodes the frame at the requested seek position (seconds) of filePath.
    // This is a REQUESTED preview/seek position, not a guarantee of exact
    // presentation-timestamp or frame accuracy (seek may land on a keyframe).
    // Semantics and failure behavior match extractFirstFrame(); a negative or
    // non-finite time is rejected deterministically. The media file is never
    // modified. Beyond-end/beyond-available positions fail deterministically
    // with the current preview position left unchanged by callers.
    static bool extractFrameAt(const QString &filePath,
                               const QString &executablePath,
                               double seconds,
                               QImage *outImage,
                               QString *error = nullptr);

private:
    FrameExtractor() = delete;
};

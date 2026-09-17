#include "FfmpegSeekFrameProvider.h"

#include <QFileInfo>

#include "media/FrameExtractor.h"

FfmpegSeekFrameProvider::FfmpegSeekFrameProvider(QString sourcePath,
                                                 QString ffmpegExecutable)
    : m_sourcePath(std::move(sourcePath))
    , m_ffmpegExecutable(std::move(ffmpegExecutable))
{
}

bool FfmpegSeekFrameProvider::frameAt(qint64 timeMs, QImage *outFrame,
                                      QString *error)
{
    if (error) {
        error->clear();
    }
    if (!outFrame) {
        if (error) {
            *error = QStringLiteral("Frame provider output is null.");
        }
        return false;
    }
    if (timeMs < 0) {
        if (error) {
            *error = QStringLiteral("Frame provider time must not be negative.");
        }
        return false;
    }
    if (m_ffmpegExecutable.isEmpty()) {
        if (error) {
            *error = QStringLiteral("FFmpeg executable not found.");
        }
        return false;
    }
    const QFileInfo info(m_sourcePath);
    if (m_sourcePath.isEmpty() || !info.exists() || !info.isFile()) {
        if (error) {
            *error = QStringLiteral("Source media is not a readable file.");
        }
        return false;
    }

    return FrameExtractor::extractFrameAt(m_sourcePath, m_ffmpegExecutable,
                                          static_cast<double>(timeMs) / 1000.0,
                                          outFrame, error);
}

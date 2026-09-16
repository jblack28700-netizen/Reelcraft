#include "FfmpegFrameSource.h"

#include <QElapsedTimer>
#include <QFileInfo>

#include "FrameExtractor.h"

FfmpegFrameSource::FfmpegFrameSource()
{
    m_process.setProcessChannelMode(QProcess::SeparateChannels);
}

FfmpegFrameSource::~FfmpegFrameSource()
{
    close();
}

bool FfmpegFrameSource::open(const QString &filePath, int frameWidth, int frameHeight)
{
    close();
    m_error.clear();
    m_atEnd = false;

    if (frameWidth <= 0 || frameHeight <= 0) {
        setError(QStringLiteral(
            "Frame geometry required for rawvideo transport; geometry "
            "acquisition is deferred (ffprobe deferred)."));
        return false;
    }
    const QFileInfo info(filePath);
    if (filePath.isEmpty() || !info.exists()) {
        setError(QStringLiteral("Media file does not exist: %1").arg(filePath));
        return false;
    }
    if (info.isDir() || !info.isFile()) {
        setError(QStringLiteral("Path is not a media file: %1").arg(filePath));
        return false;
    }

    m_frameWidth = frameWidth;
    m_frameHeight = frameHeight;

    const QString executable = FrameExtractor::defaultExecutablePath();
    if (executable.isEmpty()) {
        setError(QStringLiteral("FFmpeg executable not found."));
        return false;
    }

    // Scale to the exact caller-supplied geometry so each raw rgb24 frame is
    // exactly frameWidth*frameHeight*3 bytes.
    m_process.start(executable,
                    {
                        QStringLiteral("-v"), QStringLiteral("error"),
                        QStringLiteral("-nostdin"),
                        QStringLiteral("-i"), filePath,
                        QStringLiteral("-vf"),
                        QStringLiteral("scale=%1:%2").arg(frameWidth).arg(frameHeight),
                        QStringLiteral("-an"),
                        QStringLiteral("-f"), QStringLiteral("rawvideo"),
                        QStringLiteral("-pix_fmt"), QStringLiteral("rgb24"),
                        QStringLiteral("-")
                    });
    if (!m_process.waitForStarted(10000)) {
        setError(QStringLiteral("FFmpeg process could not start."));
        return false;
    }
    m_process.closeWriteChannel();
    m_open = true;
    return true;
}

bool FfmpegFrameSource::readNextFrame(int timeoutMs, ReadResult *result,
                                      QImage *outFrame)
{
    if (result) {
        *result = ReadResult::Error;
    }
    if (!m_open) {
        setError(QStringLiteral("Frame source is not open."));
        return false;
    }
    if (m_atEnd) {
        if (result) {
            *result = ReadResult::EndOfStream;
        }
        return false;
    }

    const qint64 need =
        static_cast<qint64>(m_frameWidth) * m_frameHeight * 3;
    QElapsedTimer timer;
    timer.start();
    bool processStopped = false;

    while (m_pendingBytes.size() < need && timer.elapsed() < timeoutMs) {
        if (m_process.waitForReadyRead(150)) {
            m_pendingBytes += m_process.readAll();
        }
        if (m_process.state() == QProcess::NotRunning) {
            processStopped = true;
            break;
        }
    }

    if (m_pendingBytes.size() >= need) {
        const QImage image(reinterpret_cast<const uchar *>(m_pendingBytes.constData()),
                           m_frameWidth, m_frameHeight, m_frameWidth * 3,
                           QImage::Format_RGB888);
        *outFrame = image.copy();
        m_pendingBytes.remove(0, need);
        if (result) {
            *result = ReadResult::Ok;
        }
        return true;
    }

    if (processStopped || m_process.state() == QProcess::NotRunning) {
        if (m_process.exitStatus() == QProcess::NormalExit
            && m_process.exitCode() == 0) {
            m_atEnd = true;
            if (result) {
                *result = ReadResult::EndOfStream;
            }
            return false;
        }
        const QString detail =
            QString::fromUtf8(m_process.readAllStandardError()).trimmed().left(300);
        setError(QStringLiteral("FFmpeg failed (%1): %2")
                     .arg(m_process.exitCode())
                     .arg(detail.isEmpty() ? QStringLiteral("unknown error") : detail));
        if (result) {
            *result = ReadResult::Error;
        }
        return false;
    }

    if (result) {
        *result = ReadResult::Timeout;
    }
    return false;
}

void FfmpegFrameSource::close()
{
    stopProcess();
    m_pendingBytes.clear();
    m_atEnd = false;
    m_open = false;
    m_frameWidth = 0;
    m_frameHeight = 0;
}

bool FfmpegFrameSource::isOpen() const
{
    return m_open;
}

QString FfmpegFrameSource::errorString() const
{
    return m_error;
}

void FfmpegFrameSource::setError(const QString &message)
{
    m_error = message;
}

void FfmpegFrameSource::stopProcess()
{
    if (m_process.state() == QProcess::NotRunning) {
        return;
    }
    m_process.terminate();
    if (!m_process.waitForFinished(3000)) {
        m_process.kill();
        m_process.waitForFinished(3000);
    }
}

#include "FrameExtractor.h"

#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <cmath>

namespace {

constexpr int kProcessTimeoutMs = 15000;

QString trimmedErrorOutput(const QByteArray &data)
{
    const QString text = QString::fromUtf8(data).trimmed();
    return text.left(300);
}

} // namespace

QString FrameExtractor::defaultExecutablePath()
{
    const QByteArray overridePath = qgetenv("REELCRAFT_FFMPEG");
    if (!overridePath.isEmpty()) {
        return QString::fromLocal8Bit(overridePath);
    }
    return QStandardPaths::findExecutable(QStringLiteral("ffmpeg"));
}

bool FrameExtractor::isAvailable()
{
    return !defaultExecutablePath().isEmpty();
}

bool FrameExtractor::extractFirstFrame(const QString &filePath,
                                       const QString &executablePath,
                                       QImage *outImage,
                                       QString *error)
{
    return extractFrameAt(filePath, executablePath, 0.0, outImage, error);
}

bool FrameExtractor::extractFrameAt(const QString &filePath,
                                    const QString &executablePath,
                                    double seconds,
                                    QImage *outImage,
                                    QString *error)
{
    if (error) {
        error->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };

    if (!outImage) {
        return fail(QStringLiteral("Frame extraction failed: null output."));
    }
    if (!std::isfinite(seconds) || seconds < 0.0) {
        return fail(QStringLiteral("Frame extraction failed: invalid seek time."));
    }
    if (executablePath.isEmpty()) {
        return fail(QStringLiteral("Frame extraction unavailable: ffmpeg not found."));
    }
    const QFileInfo info(filePath);
    if (filePath.isEmpty() || !info.exists()) {
        return fail(QStringLiteral("Frame extraction failed: media file does not exist."));
    }
    if (info.isDir() || !info.isFile()) {
        return fail(QStringLiteral("Frame extraction failed: path is not a media file."));
    }

    const QString seekTime = QString::number(seconds, 'f', 3);

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(executablePath, {
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-nostdin"),
        QStringLiteral("-ss"), seekTime,
        QStringLiteral("-i"), filePath,
        QStringLiteral("-frames:v"), QStringLiteral("1"),
        QStringLiteral("-an"),
        QStringLiteral("-f"), QStringLiteral("image2"),
        QStringLiteral("-c:v"), QStringLiteral("png"),
        QStringLiteral("pipe:1")
    });
    if (!process.waitForStarted(kProcessTimeoutMs)) {
        return fail(QStringLiteral("Frame extraction failed: ffmpeg could not start."));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(kProcessTimeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        return fail(QStringLiteral("Frame extraction failed: ffmpeg timed out."));
    }

    const QByteArray frameBytes = process.readAllStandardOutput();
    const QByteArray errorBytes = process.readAllStandardError();
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0
        || frameBytes.isEmpty()) {
        const QString detail = trimmedErrorOutput(errorBytes);
        return fail(QStringLiteral("Frame extraction failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("no frame decoded")
                                              : detail));
    }

    QImage decoded = QImage::fromData(frameBytes, "PNG");
    if (decoded.isNull()) {
        return fail(QStringLiteral("Frame extraction failed: decoded frame is invalid."));
    }
    *outImage = decoded;
    return true;
}

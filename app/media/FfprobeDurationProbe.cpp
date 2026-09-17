#include "FfprobeDurationProbe.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <cmath>
#include <utility>

#include "media/FrameExtractor.h"

namespace {

constexpr int kProcessTimeoutMs = 15000;

QString trimmedErrorOutput(const QByteArray &data)
{
    const QString text = QString::fromUtf8(data).trimmed();
    return text.left(300);
}

} // namespace

FfprobeDurationProbe::FfprobeDurationProbe()
    : m_executablePath(defaultExecutablePath())
{
}

FfprobeDurationProbe::FfprobeDurationProbe(QString executablePath)
    : m_executablePath(std::move(executablePath))
{
}

QString FfprobeDurationProbe::defaultExecutablePath()
{
    const QByteArray overridePath = qgetenv("REELCRAFT_FFPROBE");
    if (!overridePath.isEmpty()) {
        return QString::fromLocal8Bit(overridePath);
    }

    // Prefer an ffprobe next to the resolved ffmpeg executable.
    const QString ffmpeg = FrameExtractor::defaultExecutablePath();
    if (!ffmpeg.isEmpty()) {
        const QFileInfo info(ffmpeg);
        if (info.completeBaseName().compare(QStringLiteral("ffmpeg"),
                                            Qt::CaseInsensitive)
            == 0) {
            const QString suffix =
                info.suffix().isEmpty() ? QString()
                                        : QStringLiteral(".") + info.suffix();
            const QString sibling =
                info.absoluteDir().filePath(QStringLiteral("ffprobe") + suffix);
            if (QFileInfo::exists(sibling)) {
                return sibling;
            }
        }
    }

    return QStandardPaths::findExecutable(QStringLiteral("ffprobe"));
}

bool FfprobeDurationProbe::parseDurationOutput(const QString &output,
                                               qint64 *outDurationMs,
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
    if (!outDurationMs) {
        return fail(QStringLiteral("Duration output is null."));
    }
    bool ok = false;
    const double seconds = output.trimmed().toDouble(&ok);
    if (!ok || !std::isfinite(seconds) || seconds <= 0.0) {
        return fail(QStringLiteral("Could not parse a positive media duration."));
    }
    *outDurationMs = static_cast<qint64>(std::llround(seconds * 1000.0));
    return true;
}

bool FfprobeDurationProbe::durationMs(const QString &filePath,
                                      qint64 *outDurationMs, QString *error)
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

    if (!outDurationMs) {
        return fail(QStringLiteral("Duration probe failed: null output."));
    }
    if (m_executablePath.isEmpty()) {
        return fail(QStringLiteral(
            "Duration probe unavailable: ffprobe not found."));
    }
    const QFileInfo info(filePath);
    if (filePath.isEmpty() || !info.exists()) {
        return fail(QStringLiteral(
            "Duration probe failed: media file does not exist."));
    }
    if (info.isDir() || !info.isFile()) {
        return fail(QStringLiteral(
            "Duration probe failed: path is not a media file."));
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(m_executablePath, {
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-show_entries"), QStringLiteral("format=duration"),
        QStringLiteral("-of"), QStringLiteral("default=nw=1:nk=1"),
        filePath
    });
    if (!process.waitForStarted(kProcessTimeoutMs)) {
        return fail(QStringLiteral(
            "Duration probe failed: ffprobe could not start."));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(kProcessTimeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        return fail(QStringLiteral("Duration probe failed: ffprobe timed out."));
    }

    const QByteArray standardOutput = process.readAllStandardOutput();
    const QByteArray errorOutput = process.readAllStandardError();
    if (process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0) {
        const QString detail = trimmedErrorOutput(errorOutput);
        return fail(QStringLiteral("Duration probe failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("ffprobe error")
                                              : detail));
    }

    return parseDurationOutput(QString::fromUtf8(standardOutput), outDurationMs,
                               error);
}

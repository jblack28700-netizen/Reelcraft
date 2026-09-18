#include "FfprobeDurationProbe.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

bool FfprobeDurationProbe::frameRate(const QString &filePath, double *outFps,
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

    if (!outFps) {
        return fail(QStringLiteral("Frame rate probe failed: null output."));
    }
    if (m_executablePath.isEmpty()) {
        return fail(QStringLiteral(
            "Frame rate probe unavailable: ffprobe not found."));
    }
    const QFileInfo info(filePath);
    if (filePath.isEmpty() || !info.exists()) {
        return fail(QStringLiteral(
            "Frame rate probe failed: media file does not exist."));
    }
    if (info.isDir() || !info.isFile()) {
        return fail(QStringLiteral(
            "Frame rate probe failed: path is not a media file."));
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(m_executablePath, {
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-select_streams"), QStringLiteral("v:0"),
        QStringLiteral("-show_entries"), QStringLiteral("stream=r_frame_rate"),
        QStringLiteral("-of"), QStringLiteral("default=nw=1:nk=1"),
        filePath
    });
    if (!process.waitForStarted(kProcessTimeoutMs)) {
        return fail(QStringLiteral(
            "Frame rate probe failed: ffprobe could not start."));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(kProcessTimeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        return fail(QStringLiteral(
            "Frame rate probe failed: ffprobe timed out."));
    }
    const QByteArray standardOutput = process.readAllStandardOutput();
    const QByteArray errorOutput = process.readAllStandardError();
    if (process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0) {
        const QString detail = trimmedErrorOutput(errorOutput);
        return fail(QStringLiteral("Frame rate probe failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("ffprobe error")
                                              : detail));
    }

    // ffprobe reports a rational, for example 30000/1001 or 25/1.
    const QString output = QString::fromUtf8(standardOutput).trimmed();
    const int slash = output.indexOf(QLatin1Char('/'));
    bool ok = false;
    double fps = 0.0;
    if (slash > 0) {
        const double numerator = output.left(slash).toDouble(&ok);
        if (!ok) {
            return fail(QStringLiteral(
                "Frame rate probe failed: unparsable frame rate."));
        }
        const double denominator = output.mid(slash + 1).toDouble(&ok);
        if (!ok || denominator <= 0.0) {
            return fail(QStringLiteral(
                "Frame rate probe failed: unparsable frame rate."));
        }
        fps = numerator / denominator;
    } else {
        fps = output.toDouble(&ok);
        if (!ok) {
            return fail(QStringLiteral(
                "Frame rate probe failed: unparsable frame rate."));
        }
    }
    if (!std::isfinite(fps) || fps <= 0.0) {
        return fail(QStringLiteral(
            "Frame rate probe failed: unusable frame rate."));
    }
    *outFps = fps;
    return true;
}

bool FfprobeDurationProbe::parseStreamSummary(const QByteArray &json,
                                              StreamSummary *outSummary,
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
    if (!outSummary) {
        return fail(QStringLiteral("Stream summary output is null."));
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return fail(QStringLiteral("Could not parse the media stream listing."));
    }
    const QJsonValue streamsValue =
        document.object().value(QStringLiteral("streams"));
    if (!streamsValue.isArray()) {
        return fail(QStringLiteral("The media stream listing has no streams."));
    }

    StreamSummary summary;
    for (const QJsonValue &value : streamsValue.toArray()) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject stream = value.toObject();
        const QString codecType =
            stream.value(QStringLiteral("codec_type")).toString();
        if (codecType == QStringLiteral("video") && !summary.hasVideo) {
            // Only the first video stream is described; a multi-video-stream
            // file is reported as "the first one" rather than merged.
            const int width = stream.value(QStringLiteral("width")).toInt();
            const int height = stream.value(QStringLiteral("height")).toInt();
            if (width > 0 && height > 0) {
                summary.hasVideo = true;
                summary.videoWidth = width;
                summary.videoHeight = height;
            }
        } else if (codecType == QStringLiteral("audio") && !summary.hasAudio) {
            summary.hasAudio = true;
            // ffprobe reports sample_rate as a STRING and channels as a number.
            const QString sampleRate =
                stream.value(QStringLiteral("sample_rate")).toString();
            if (!sampleRate.isEmpty()) {
                bool ok = false;
                const int parsed = sampleRate.toInt(&ok);
                if (ok && parsed > 0) {
                    summary.audioSampleRate = parsed;
                }
            }
            const int channels = stream.value(QStringLiteral("channels")).toInt();
            if (channels > 0) {
                summary.audioChannels = channels;
            }
        }
    }

    if (!summary.isValid()) {
        return fail(QStringLiteral(
            "The media stream listing describes no usable video or audio stream."));
    }
    *outSummary = summary;
    return true;
}

bool FfprobeDurationProbe::streamSummary(const QString &filePath,
                                         StreamSummary *outSummary, QString *error)
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

    if (!outSummary) {
        return fail(QStringLiteral("Stream summary probe failed: null output."));
    }
    if (m_executablePath.isEmpty()) {
        return fail(QStringLiteral(
            "Stream summary probe unavailable: ffprobe not found."));
    }
    const QFileInfo info(filePath);
    if (filePath.isEmpty() || !info.exists()) {
        return fail(QStringLiteral(
            "Stream summary probe failed: media file does not exist."));
    }
    if (info.isDir() || !info.isFile()) {
        return fail(QStringLiteral(
            "Stream summary probe failed: path is not a media file."));
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(m_executablePath, {
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-show_entries"),
        QStringLiteral("stream=codec_type,width,height,sample_rate,channels"),
        QStringLiteral("-of"), QStringLiteral("json"),
        filePath
    });
    if (!process.waitForStarted(kProcessTimeoutMs)) {
        return fail(QStringLiteral(
            "Stream summary probe failed: ffprobe could not start."));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(kProcessTimeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        return fail(QStringLiteral("Stream summary probe failed: ffprobe timed out."));
    }
    const QByteArray standardOutput = process.readAllStandardOutput();
    const QByteArray errorOutput = process.readAllStandardError();
    if (process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0) {
        const QString detail = trimmedErrorOutput(errorOutput);
        return fail(QStringLiteral("Stream summary probe failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("ffprobe error")
                                              : detail));
    }

    return parseStreamSummary(standardOutput, outSummary, error);
}


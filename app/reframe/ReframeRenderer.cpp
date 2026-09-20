#include "ReframeRenderer.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>

#include <cmath>

#include "reframe/CameraPath.h"
#include "viewer/EquirectView.h"

namespace {

constexpr int kEncodeTimeoutMs = 180000;

QString frameFileName(int index)
{
    return QStringLiteral("frame_%1.png").arg(index, 5, 10, QChar('0'));
}

// Objective 28: milliseconds as the seconds value an FFmpeg filter option
// expects. Fixed six decimals so the generated command is a pure function of the
// spec and therefore byte-comparable between runs.
QString secondsValue(qint64 timeMs)
{
    return QString::number(static_cast<double>(timeMs) / 1000.0, 'f', 6);
}

// Runs one FFmpeg invocation to completion under the encoder timeout. Returns
// false with a deterministic error when it cannot start, times out, or exits
// non-zero. Shared by the video-only encode and the audio-aware encode, so both
// paths fail identically.
bool runFfmpeg(const QString &ffmpegExecutable, const QStringList &arguments,
               QString *error)
{
    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(ffmpegExecutable, arguments);
    if (!process.waitForStarted(10000)) {
        if (error) {
            *error = QStringLiteral("FFmpeg encoder could not start.");
        }
        return false;
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(kEncodeTimeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        if (error) {
            *error = QStringLiteral("FFmpeg encoder timed out.");
        }
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0) {
        const QString detail =
            QString::fromUtf8(process.readAllStandardError()).trimmed().left(300);
        if (error) {
            *error = QStringLiteral("FFmpeg encoder failed: %1")
                         .arg(detail.isEmpty() ? QStringLiteral("unknown error")
                                               : detail);
        }
        return false;
    }
    return true;
}

// Objective 28: the audio filtergraph that maps the plan's retained SOURCE spans
// onto the OUTPUT timeline. One trim per span, timestamps reset at each trim, the
// spans concatenated in order, and the result bounded to the rendered picture
// when the caller knows its length.
QString audioFilterGraph(const QList<ReframePlan::TimeRange> &spans,
                         qint64 outputDurationMs)
{
    QString graph;
    QStringList parts;
    for (int i = 0; i < spans.size(); ++i) {
        const ReframePlan::TimeRange &span = spans.at(i);
        const QString label = QStringLiteral("span%1").arg(i);
        graph += QStringLiteral(
                     "[1:a]atrim=start=%1:end=%2,asetpts=PTS-STARTPTS[%3];")
                     .arg(secondsValue(span.startMs), secondsValue(span.endMs),
                          label);
        parts.append(QStringLiteral("[%1]").arg(label));
    }

    QString joined;
    if (spans.size() == 1) {
        joined = parts.first();
    } else {
        graph += QStringLiteral("%1concat=n=%2:v=0:a=1[joined];")
                     .arg(parts.join(QString()),
                          QString::number(spans.size()));
        joined = QStringLiteral("[joined]");
    }

    if (outputDurationMs > 0) {
        // The picture is the reference timeline: a span's exact output frame
        // count rounds DOWN (framesForRange uses floor), so the concatenated
        // audio can outlast the picture by less than one frame, and an
        // unbounded tail would extend the container past the last frame.
        graph += QStringLiteral("%1atrim=end=%2,asetpts=PTS-STARTPTS[aout]")
                     .arg(joined, secondsValue(outputDurationMs));
    } else {
        graph += QStringLiteral("%1anull[aout]").arg(joined);
    }
    return graph;
}

} // namespace

QString ReframeRenderer::frameFileNamePattern()
{
    return QStringLiteral("frame_%05d.png");
}

bool ReframeRenderer::render(const ReframePlan &plan,
                             ReframeFrameProvider *provider,
                             const FrameSink &sink,
                             int *outFrameCount,
                             QString *error)
{
    if (error) {
        error->clear();
    }
    if (outFrameCount) {
        *outFrameCount = 0;
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };

    QString validationError;
    if (!plan.isValid(&validationError)) {
        return fail(validationError);
    }
    if (!provider) {
        return fail(QStringLiteral("Reframe renderer has no frame provider."));
    }
    if (!sink) {
        return fail(QStringLiteral("Reframe renderer has no frame sink."));
    }

    const int count = plan.frameCount();
    const ReframePlan::OutputSpec output = plan.output();

    for (int index = 0; index < count; ++index) {
        const qint64 timeMs = plan.frameTimeMs(index);

        QImage sourceFrame;
        QString providerError;
        if (!provider->frameAt(timeMs, &sourceFrame, &providerError)) {
            return fail(QStringLiteral("Frame %1 (t=%2 ms): %3")
                            .arg(index)
                            .arg(timeMs)
                            .arg(providerError));
        }

        const CameraState camera = CameraPath::stateAt(plan, timeMs);
        QImage flatFrame;
        if (!EquirectView::render(sourceFrame, camera.yawDeg, camera.pitchDeg,
                                  camera.rollDeg, camera.fieldOfViewDeg,
                                  output.width, output.height, &flatFrame)) {
            return fail(QStringLiteral(
                "Frame %1 (t=%2 ms): equirectangular render failed.")
                            .arg(index)
                            .arg(timeMs));
        }

        if (!sink(index, timeMs, flatFrame)) {
            return fail(QStringLiteral("Frame %1: sink rejected the frame.")
                            .arg(index));
        }
    }

    if (outFrameCount) {
        *outFrameCount = count;
    }
    return true;
}

bool ReframeRenderer::renderToPngSequence(const ReframePlan &plan,
                                          ReframeFrameProvider *provider,
                                          const QString &outputDirectory,
                                          QStringList *outPaths,
                                          QString *error)
{
    if (error) {
        error->clear();
    }
    if (outPaths) {
        outPaths->clear();
    }
    const auto fail = [error](const QString &message) {
        if (error) {
            *error = message;
        }
        return false;
    };

    if (outputDirectory.isEmpty()) {
        return fail(QStringLiteral("Reframe output directory is empty."));
    }
    QDir directory;
    if (!directory.mkpath(outputDirectory)) {
        return fail(QStringLiteral("Could not create reframe output directory."));
    }

    QStringList written;
    const bool ok = render(
        plan, provider,
        [&outputDirectory, &written](int index, qint64, const QImage &frame) {
            const QString path =
                QDir(outputDirectory).filePath(frameFileName(index));
            if (!frame.save(path, "PNG")) {
                return false;
            }
            written.append(path);
            return true;
        },
        nullptr, error);

    if (!ok) {
        return false;
    }
    if (outPaths) {
        *outPaths = written;
    }
    return true;
}

bool ReframeRenderer::encodeVideo(const QString &ffmpegExecutable,
                                  const QString &inputPattern,
                                  double fps,
                                  const QString &outputPath,
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

    if (ffmpegExecutable.isEmpty()) {
        return fail(QStringLiteral("FFmpeg executable not found."));
    }
    if (inputPattern.isEmpty() || outputPath.isEmpty()) {
        return fail(QStringLiteral("Reframe encode input/output is empty."));
    }
    if (!std::isfinite(fps) || fps <= 0.0) {
        return fail(QStringLiteral("Reframe encode frame rate is invalid."));
    }

    return runFfmpeg(ffmpegExecutable, {
        QStringLiteral("-y"),
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-nostdin"),
        QStringLiteral("-framerate"), QString::number(fps, 'f', 6),
        QStringLiteral("-i"), inputPattern,
        QStringLiteral("-c:v"), QStringLiteral("libx264"),
        QStringLiteral("-pix_fmt"), QStringLiteral("yuv420p"),
        QStringLiteral("-vf"),
        QStringLiteral("pad=ceil(iw/2)*2:ceil(ih/2)*2"),
        outputPath
    }, error);
}

bool ReframeRenderer::encodeVideoWithAudio(const QString &ffmpegExecutable,
                                           const QString &inputPattern,
                                           double fps,
                                           const QString &outputPath,
                                           const AudioSpec &audio,
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

    if (ffmpegExecutable.isEmpty()) {
        return fail(QStringLiteral("FFmpeg executable not found."));
    }
    if (inputPattern.isEmpty() || outputPath.isEmpty()) {
        return fail(QStringLiteral("Reframe audio encode input/output is empty."));
    }
    if (!std::isfinite(fps) || fps <= 0.0) {
        return fail(QStringLiteral("Reframe audio encode frame rate is invalid."));
    }
    if (audio.sourcePath.isEmpty()) {
        return fail(QStringLiteral(
            "Reframe audio encode has no source media to take audio from."));
    }
    if (audio.spans.isEmpty()) {
        return fail(QStringLiteral(
            "Reframe audio encode has no retained source span."));
    }
    for (const ReframePlan::TimeRange &span : audio.spans) {
        if (!span.isValid()) {
            // Never render audio for a span that was not actually retained.
            return fail(QStringLiteral(
                "Reframe audio encode has an invalid retained source span."));
        }
    }

    // Pass 1: the picture, through the UNCHANGED video-only encoder, into a
    // temporary file. Routing the video through the existing path is what keeps
    // the video stream of an output that carries audio identical to the
    // video-only output of the same frames: the audio pass below remuxes that
    // stream with -c:v copy and never re-encodes it.
    QTemporaryDir pictureDirectory;
    if (!pictureDirectory.isValid()) {
        return fail(QStringLiteral(
            "Reframe audio encode could not create a temporary directory."));
    }
    const QString picturePath =
        pictureDirectory.filePath(QStringLiteral("picture_only.mp4"));
    if (!encodeVideo(ffmpegExecutable, inputPattern, fps, picturePath, error)) {
        return false;
    }

    // Pass 2: the picture plus the retained source audio. The audio is always
    // re-encoded with fixed parameters, never stream-copied: a copy cannot be
    // trimmed to an arbitrary source span and would make the output depend on
    // the source's own codec rather than on this command. The audio map is
    // REQUIRED (not optional): if the retained spans cannot produce an audio
    // stream the encode must fail rather than quietly write a silent file.
    QStringList arguments{
        QStringLiteral("-y"),
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-nostdin"),
        QStringLiteral("-i"), picturePath,
        QStringLiteral("-i"), audio.sourcePath,
        QStringLiteral("-filter_complex"),
        audioFilterGraph(audio.spans, audio.outputDurationMs),
        QStringLiteral("-map"), QStringLiteral("0:v:0"),
        QStringLiteral("-map"), QStringLiteral("[aout]"),
        QStringLiteral("-c:v"), QStringLiteral("copy"),
        QStringLiteral("-c:a"), QStringLiteral("aac"),
        QStringLiteral("-b:a"), QStringLiteral("192k")
    };
    // Preserve the source's sample rate, and its channel count for the layouts
    // that are unambiguous to re-encode; anything else is left to FFmpeg so an
    // uncommon layout is preserved by the muxer rather than approximated.
    if (audio.sampleRate > 0) {
        arguments << QStringLiteral("-ar") << QString::number(audio.sampleRate);
    }
    if (audio.channels == 1 || audio.channels == 2) {
        arguments << QStringLiteral("-ac") << QString::number(audio.channels);
    }
    arguments << outputPath;

    return runFfmpeg(ffmpegExecutable, arguments, error);
}

#include "ReframeRenderer.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include <cmath>

#include "reframe/CameraPath.h"
#include "viewer/EquirectView.h"

namespace {

constexpr int kEncodeTimeoutMs = 180000;

QString frameFileName(int index)
{
    return QStringLiteral("frame_%1.png").arg(index, 5, 10, QChar('0'));
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

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(ffmpegExecutable, {
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
    });
    if (!process.waitForStarted(10000)) {
        return fail(QStringLiteral("FFmpeg encoder could not start."));
    }
    process.closeWriteChannel();
    if (!process.waitForFinished(kEncodeTimeoutMs)) {
        process.kill();
        process.waitForFinished(2000);
        return fail(QStringLiteral("FFmpeg encoder timed out."));
    }

    if (process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0) {
        const QString detail =
            QString::fromUtf8(process.readAllStandardError()).trimmed().left(300);
        return fail(QStringLiteral("FFmpeg encoder failed: %1")
                        .arg(detail.isEmpty() ? QStringLiteral("unknown error")
                                              : detail));
    }
    return true;
}

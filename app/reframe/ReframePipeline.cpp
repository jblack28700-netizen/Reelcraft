#include "ReframePipeline.h"

#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

#include <cmath>
#include <memory>

#include "media/FfprobeDurationProbe.h"
#include "media/FrameExtractor.h"
#include "media/MediaDurationProbe.h"
#include "reframe/ReframePlanBuilder.h"
#include "reframe/ReframeRenderer.h"
#include "reframe/ReframeStreamFrameProvider.h"

ReframePipeline::Result ReframePipeline::run(const Request &request)
{
    Result result;
    result.outputPath = request.outputPath;

    if (request.sourcePath.isEmpty()
        || !QFileInfo::exists(request.sourcePath)) {
        result.error = QStringLiteral("Source media does not exist.");
        return result;
    }
    if (request.outputPath.isEmpty()) {
        result.error = QStringLiteral("Output path is empty.");
        return result;
    }

    const QString ffmpeg = FrameExtractor::defaultExecutablePath();
    if (ffmpeg.isEmpty()) {
        result.error = QStringLiteral(
            "FFmpeg is unavailable; 360 reframing requires it.");
        return result;
    }

    ReframeIntent intent = ReframeIntentParser::parse(request.instruction);
    result.intent = intent;

    const ReframeBuildResult built = ReframePlanBuilder::build(
        intent, request.resolvedTargets, request.defaultRange,
        request.defaultOutput);
    result.notes = built.notes;
    if (!built.ok) {
        result.error = built.error;
        return result;
    }

    ReframePlan plan = built.plan;
    plan.setSourceMediaId(request.sourceMediaId);
    result.plan = plan;

    const Result rendered =
        renderPlan(plan, request.sourcePath, request.outputPath, nullptr);
    result.ok = rendered.ok;
    result.error = rendered.error;
    result.frameCount = rendered.frameCount;
    result.renderedFramePaths = rendered.renderedFramePaths;
    // Objective 28: execution notes (for example why an output carries no
    // audio) belong to the caller's result, not only to the inner call.
    result.notes.append(rendered.notes);
    if (!rendered.outputPath.isEmpty()) {
        result.outputPath = rendered.outputPath;
    }
    return result;
}

ReframePipeline::Result ReframePipeline::renderPlan(
    const ReframePlan &plan, const QString &sourcePath, const QString &outputPath,
    ReframeFrameProvider *provider, MediaDurationProbe *probe)
{
    Result result;
    result.plan = plan;
    result.outputPath = outputPath;

    if (sourcePath.isEmpty() || !QFileInfo::exists(sourcePath)) {
        result.error = QStringLiteral("Source media does not exist.");
        return result;
    }
    if (outputPath.isEmpty()) {
        result.error = QStringLiteral("Output path is empty.");
        return result;
    }
    QString validationError;
    if (!plan.isValid(&validationError)) {
        result.error = validationError;
        return result;
    }

    const QString ffmpeg = FrameExtractor::defaultExecutablePath();
    if (ffmpeg.isEmpty()) {
        result.error = QStringLiteral(
            "FFmpeg is unavailable; 360 reframing requires it.");
        return result;
    }

    QTemporaryDir frameDirectory;
    if (!frameDirectory.isValid()) {
        result.error = QStringLiteral(
            "Could not create a temporary frame directory.");
        return result;
    }

    std::unique_ptr<ReframeFrameProvider> ownedProvider;
    ReframeFrameProvider *frameProvider = provider;
    if (!frameProvider) {
        // Objective 20: ONE persistent decoding process for the whole render,
        // instead of one FFmpeg process per output frame. The source frame rate
        // is probed once so the provider can replay frames sequentially; when it
        // is unknown, or when a request cannot be served sequentially (a
        // backwards jump, a far-forward jump, or a stream that ended), the
        // provider falls back to a positioned seek and the result is unchanged.
        double sourceFps = 0.0;
        {
            FfprobeDurationProbe rateProbe;
            QString rateError;
            rateProbe.frameRate(sourcePath, &sourceFps, &rateError);
        }
        ownedProvider = std::make_unique<ReframeStreamFrameProvider>(
            sourcePath, ffmpeg, sourceFps);
        frameProvider = ownedProvider.get();
    }

    QStringList framePaths;
    QString renderError;
    if (!ReframeRenderer::renderToPngSequence(plan, frameProvider,
                                              frameDirectory.path(),
                                              &framePaths, &renderError)) {
        result.error = renderError;
        return result;
    }
    result.renderedFramePaths = framePaths;
    result.frameCount = framePaths.size();

    const QString pattern = QDir(frameDirectory.path())
                                .filePath(ReframeRenderer::frameFileNamePattern());

    // Objective 28: the picture keeps the audio that belongs to it. The spans
    // are the plan's retained SOURCE ranges in output order — its ordered
    // segments when it has them (Objective 14), otherwise its single source
    // range. Nothing here decides anything editorial: the plan already chose
    // those spans, and this maps the audio of exactly those spans onto the
    // output timeline.
    QList<ReframePlan::TimeRange> audioSpans = plan.segments();
    if (audioSpans.isEmpty()) {
        audioSpans.append(plan.sourceRange());
    }

    // Whether the source HAS audio is a fact reported by the probe seam, never
    // an assumption. A source with no audio track must keep rendering exactly as
    // it did before; a probe that cannot answer must degrade to that same silent
    // output with the reason recorded rather than guessing.
    FfprobeDurationProbe ownedProbe;
    MediaDurationProbe *audioProbe = probe ? probe : &ownedProbe;
    MediaDurationProbe::StreamSummary summary;
    QString summaryError;
    if (!audioProbe->streamSummary(sourcePath, &summary, &summaryError)) {
        result.notes.append(
            QStringLiteral("Source audio could not be determined (%1); "
                           "rendering without audio.")
                .arg(summaryError.isEmpty() ? QStringLiteral("unavailable")
                                            : summaryError));
    }

    QString encodeError;
    if (summary.hasAudio) {
        ReframeRenderer::AudioSpec audio;
        audio.sourcePath = sourcePath;
        audio.spans = audioSpans;
        // The rendered picture is the reference timeline. A retained span's
        // exact output frame count rounds down, so the concatenated audio can
        // outlast the picture by less than one frame; bounding it keeps the
        // container ending on the last frame.
        audio.outputDurationMs = static_cast<qint64>(std::llround(
            static_cast<double>(plan.frameCount()) * 1000.0
            / plan.output().fps));
        audio.sampleRate = summary.audioSampleRate;
        audio.channels = summary.audioChannels;
        if (!ReframeRenderer::encodeVideoWithAudio(
                ffmpeg, pattern, plan.output().fps, outputPath, audio,
                &encodeError)) {
            result.error = encodeError;
            return result;
        }
        result.notes.append(
            QStringLiteral("Rendered output carries the source audio over %1 "
                           "retained source span(s).")
                .arg(audioSpans.size()));
    } else if (!ReframeRenderer::encodeVideo(ffmpeg, pattern, plan.output().fps,
                                             outputPath, &encodeError)) {
        result.error = encodeError;
        return result;
    }

    result.ok = true;
    result.outputPath = outputPath;
    return result;
}

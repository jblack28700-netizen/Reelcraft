#include "ReframePipeline.h"

#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

#include <memory>

#include "media/FfprobeDurationProbe.h"
#include "media/FrameExtractor.h"
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
    if (!rendered.outputPath.isEmpty()) {
        result.outputPath = rendered.outputPath;
    }
    return result;
}

ReframePipeline::Result ReframePipeline::renderPlan(
    const ReframePlan &plan, const QString &sourcePath, const QString &outputPath,
    ReframeFrameProvider *provider)
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
    QString encodeError;
    if (!ReframeRenderer::encodeVideo(ffmpeg, pattern, plan.output().fps,
                                      outputPath, &encodeError)) {
        result.error = encodeError;
        return result;
    }

    result.ok = true;
    result.outputPath = outputPath;
    return result;
}

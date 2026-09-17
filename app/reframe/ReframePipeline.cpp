#include "ReframePipeline.h"

#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

#include <memory>

#include "media/FrameExtractor.h"
#include "reframe/FfmpegSeekFrameProvider.h"
#include "reframe/ReframePlanBuilder.h"
#include "reframe/ReframeRenderer.h"

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
        ownedProvider = std::make_unique<FfmpegSeekFrameProvider>(sourcePath,
                                                                  ffmpeg);
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

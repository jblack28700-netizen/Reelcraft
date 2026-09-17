#include "ReframePipeline.h"

#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>

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

    QTemporaryDir frameDirectory;
    if (!frameDirectory.isValid()) {
        result.error = QStringLiteral(
            "Could not create a temporary frame directory.");
        return result;
    }

    FfmpegSeekFrameProvider provider(request.sourcePath, ffmpeg);
    QStringList framePaths;
    QString renderError;
    if (!ReframeRenderer::renderToPngSequence(plan, &provider,
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
                                      request.outputPath, &encodeError)) {
        result.error = encodeError;
        return result;
    }

    result.ok = true;
    result.outputPath = request.outputPath;
    return result;
}

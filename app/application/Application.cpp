#include "Application.h"

#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QThread>
#include <QTimer>

#include <cmath>
#include <memory>

#include "media/FfprobeDurationProbe.h"
#include "media/FfmpegFrameSource.h"
#include "media/FrameExtractor.h"
#include "viewer/ViewportState.h"

Application::Application(QObject *parent)
    : QObject(parent),
      m_viewportState(new ViewportState(this)),
      m_ownedDurationProbe(std::make_unique<FfprobeDurationProbe>())
{
    m_durationProbe = m_ownedDurationProbe.get();
    resetReframeCommandExecutor();
    resetReframePreviewDecoder();
    resetPlaybackSourceFactory();

    // The Application owns the playback event-loop driver; the Player does not.
    m_playbackTimer = new QTimer(this);
    connect(m_playbackTimer, &QTimer::timeout, this,
            &Application::tickReframeOutputPlayback);
}

Application::~Application()
{
    // Stop the driver and release the source deterministically without emitting
    // signals during destruction.
    if (m_playbackTimer) {
        m_playbackTimer->stop();
    }
    if (m_playbackPlayer) {
        disconnect(m_playbackPlayer.get(), nullptr, this, nullptr);
        m_playbackPlayer->stop();
        m_playbackPlayer.reset();
    }
    m_playbackPump.reset();
    if (m_playbackSource) {
        m_playbackSource->close();
        m_playbackSource.reset();
    }
}

void Application::initialize()
{
}

Project Application::currentProject() const
{
    return m_currentProject;
}

bool Application::hasProject() const
{
    return m_hasProject;
}

ViewportState *Application::viewportState() const
{
    return m_viewportState;
}

QList<MediaItem> Application::mediaItems() const
{
    return m_mediaItems;
}

QString Application::activeMediaId() const
{
    return m_activeMediaId;
}

const MediaItem *Application::activeMediaItem() const
{
    if (m_activeMediaId.isEmpty()) {
        return nullptr;
    }
    for (const MediaItem &item : m_mediaItems) {
        if (item.id() == m_activeMediaId) {
            return &item;
        }
    }
    return nullptr;
}

void Application::newProject()
{
    stopReframeOutputPlayback();
    m_currentProject = Project();
    m_hasProject = true;
    const bool hadActiveMedia = !m_activeMediaId.isEmpty();
    const bool hadPreviewTime = m_previewTimeSeconds != 0.0;
    const bool hadReframeOutputs = !m_reframeOutputs.isEmpty();
    const bool hadCreatorSelection = m_hasCreatorSelection;
    m_mediaItems.clear();
    m_activeMediaId.clear();
    m_previewTimeSeconds = 0.0;
    m_reframeOutputs.clear();
    m_hasCreatorSelection = false;
    m_creatorSelection = CreatorTargetSelection();
    resetViewport();
    emit mediaListChanged(m_mediaItems);
    if (hadCreatorSelection) {
        emit creatorSelectionChanged(false);
    }
    if (hadReframeOutputs) {
        emit reframeOutputsChanged(m_reframeOutputs);
    }
    if (hadActiveMedia) {
        emit activeMediaChanged(m_activeMediaId);
    }
    if (hadPreviewTime) {
        emit previewTimeChanged(m_previewTimeSeconds);
    }
    emit projectChanged(m_currentProject);
}

bool Application::saveProject(const QString &filePath)
{
    if (!m_hasProject) {
        emit backgroundCompleted(QStringLiteral("No project to save."));
        return false;
    }

    m_currentProject.setViewerState(m_viewportState ? m_viewportState->toJsonObject() : QJsonObject());
    m_currentProject.setMedia(mediaJson());
    m_currentProject.setActiveMediaId(m_activeMediaId);
    m_currentProject.setReframeOutputs(reframeOutputsJson());

    QString error;
    const bool ok = m_currentProject.save(filePath, &error);
    if (!ok) {
        emit backgroundCompleted(QStringLiteral("Save failed: %1").arg(error));
    }
    return ok;
}

bool Application::openProject(const QString &filePath)
{
    bool ok = false;
    QString error;
    Project loaded = Project::load(filePath, &ok, &error);

    if (!ok) {
        emit backgroundCompleted(QStringLiteral("Open failed: %1").arg(error));
        return false;
    }

    if (m_hasCreatorSelection) {
        m_hasCreatorSelection = false;
        m_creatorSelection = CreatorTargetSelection();
        emit creatorSelectionChanged(false);
    }
    stopReframeOutputPlayback();
    m_currentProject = loaded;
    m_hasProject = true;
    resetViewport();
    restoreMediaFromJson(m_currentProject.media());
    restoreReframeOutputsFromJson(m_currentProject.reframeOutputs());
    emit reframeOutputsChanged(m_reframeOutputs);

    if (m_viewportState && !m_currentProject.viewerState().isEmpty()) {
        QString viewerError;
        m_viewportState->readFromJsonObject(m_currentProject.viewerState(), &viewerError);
    }

    const int unavailable = unavailableMediaCount();
    if (unavailable > 0) {
        emit backgroundCompleted(QStringLiteral("Media references unavailable: %1 of %2.")
                                     .arg(unavailable)
                                     .arg(m_mediaItems.size()));
    }

    // The media list is normalized first; the active id is then restored only
    // when it resolves to a current, available media record.
    const bool hadPreviewTime = m_previewTimeSeconds != 0.0;
    m_previewTimeSeconds = 0.0;
    emit mediaListChanged(m_mediaItems);
    restoreActiveMediaFromProject(m_currentProject.activeMediaId());
    if (hadPreviewTime) {
        emit previewTimeChanged(m_previewTimeSeconds);
    }
    emit projectChanged(m_currentProject);
    return true;
}

bool Application::importMediaFile(const QString &filePath)
{
    if (!m_hasProject) {
        emit backgroundCompleted(QStringLiteral("No project to import media into."));
        return false;
    }

    QString error;
    const MediaItem item = MediaItem::createFromFilePath(filePath, &error);
    if (!item.isValid()) {
        emit backgroundCompleted(QStringLiteral("Import failed: %1").arg(error));
        return false;
    }

    for (const MediaItem &existing : m_mediaItems) {
        if (existing.path() == item.path()) {
            emit backgroundCompleted(
                QStringLiteral("Media already imported: %1").arg(item.fileName()));
            return true;
        }
    }

    m_mediaItems.append(item);
    emit backgroundCompleted(QStringLiteral("Imported media: %1").arg(item.fileName()));
    emit mediaListChanged(m_mediaItems);
    return true;
}

bool Application::removeMedia(const QString &mediaId)
{
    if (!m_hasProject) {
        emit backgroundCompleted(QStringLiteral("No project to remove media from."));
        return false;
    }

    for (int i = 0; i < m_mediaItems.size(); ++i) {
        if (m_mediaItems.at(i).id() == mediaId) {
            const QString fileName = m_mediaItems.at(i).fileName();
            const bool removedActive = (m_activeMediaId == mediaId);
            m_mediaItems.removeAt(i);
            if (removedActive) {
                m_activeMediaId.clear();
                if (m_previewTimeSeconds != 0.0) {
                    m_previewTimeSeconds = 0.0;
                }
            }
            emit backgroundCompleted(QStringLiteral("Removed media: %1").arg(fileName));
            emit mediaListChanged(m_mediaItems);
            if (removedActive) {
                emit activeMediaChanged(m_activeMediaId);
                emit previewTimeChanged(m_previewTimeSeconds);
            }
            return true;
        }
    }

    emit backgroundCompleted(QStringLiteral("Remove failed: media not found."));
    return false;
}

bool Application::setActiveMedia(const QString &mediaId)
{
    if (!m_hasProject) {
        emit backgroundCompleted(QStringLiteral("No project to select media in."));
        return false;
    }

    const MediaItem *item = nullptr;
    for (const MediaItem &candidate : m_mediaItems) {
        if (candidate.id() == mediaId) {
            item = &candidate;
            break;
        }
    }
    if (!item) {
        emit backgroundCompleted(QStringLiteral("Select failed: media not found."));
        return false;
    }
    if (!item->referenceExists()) {
        emit backgroundCompleted(QStringLiteral("Select failed: media is unavailable."));
        return false;
    }

    if (m_activeMediaId == mediaId) {
        emit backgroundCompleted(QStringLiteral("Media already active: %1").arg(item->fileName()));
        return true;
    }

    m_activeMediaId = mediaId;
    // Changing the active media starts time navigation from the beginning.
    if (m_previewTimeSeconds != 0.0) {
        m_previewTimeSeconds = 0.0;
    }
    emit backgroundCompleted(QStringLiteral("Active media: %1").arg(item->fileName()));
    emit activeMediaChanged(m_activeMediaId);
    emit previewTimeChanged(m_previewTimeSeconds);
    return true;
}

double Application::previewTimeSeconds() const
{
    return m_previewTimeSeconds;
}

bool Application::declareMediaProjection(const QString &mediaId, const QString &projectionValue)
{
    if (!m_hasProject) {
        emit backgroundCompleted(QStringLiteral("No project to declare media projection in."));
        return false;
    }

    const MediaItem::Projection projection =
        MediaItem::projectionFromString(projectionValue);
    if (projection == MediaItem::Projection::Unknown) {
        emit backgroundCompleted(QStringLiteral("Projection failed: unknown projection value."));
        return false;
    }

    for (int i = 0; i < m_mediaItems.size(); ++i) {
        if (m_mediaItems.at(i).id() == mediaId) {
            const QString fileName = m_mediaItems.at(i).fileName();
            m_mediaItems[i].setProjection(projection);
            emit backgroundCompleted(
                QStringLiteral("Media projection: %1 = %2")
                    .arg(fileName, MediaItem::projectionToString(projection)));
            emit mediaListChanged(m_mediaItems);
            return true;
        }
    }

    emit backgroundCompleted(QStringLiteral("Projection failed: media not found."));
    return false;
}

bool Application::previewActiveMediaFrame()
{
    return decodePreviewFrameAt(m_previewTimeSeconds);
}

bool Application::previewActiveMediaFrameAt(double seconds)
{
    return decodePreviewFrameAt(seconds < 0.0 ? 0.0 : seconds);
}

bool Application::stepActiveMediaPreview(double deltaSeconds)
{
    const double target = m_previewTimeSeconds + deltaSeconds;
    return decodePreviewFrameAt(target < 0.0 ? 0.0 : target);
}

bool Application::decodePreviewFrameAt(double targetSeconds)
{
    if (!m_hasProject) {
        emit backgroundCompleted(QStringLiteral("No project to preview media in."));
        return false;
    }
    const MediaItem *active = activeMediaItem();
    if (!active) {
        emit backgroundCompleted(QStringLiteral("No active media to preview."));
        return false;
    }
    if (!active->referenceExists()) {
        emit backgroundCompleted(QStringLiteral("Preview failed: active media is unavailable."));
        return false;
    }
    if (!FrameExtractor::isAvailable()) {
        emit backgroundCompleted(
            QStringLiteral("Preview failed: ffmpeg not found (frame extraction unavailable)."));
        return false;
    }

    QImage frame;
    QString error;
    if (!FrameExtractor::extractFrameAt(active->path(),
                                        FrameExtractor::defaultExecutablePath(),
                                        targetSeconds, &frame, &error)) {
        // Beyond-end / undecodable requests fail deterministically; the
        // current preview position is left unchanged.
        emit backgroundCompleted(QStringLiteral("Preview failed: %1").arg(error));
        return false;
    }

    if (m_previewTimeSeconds != targetSeconds) {
        m_previewTimeSeconds = targetSeconds;
        emit previewTimeChanged(m_previewTimeSeconds);
    }
    emit backgroundCompleted(QStringLiteral("Frame extracted: %1").arg(active->fileName()));
    emit framePreviewReady(frame);
    return true;
}

void Application::runBackgroundDemo()
{
    QtConcurrent::run([this]() {
        QThread::msleep(200);
        emit backgroundCompleted(QStringLiteral("Background task completed successfully."));
    });
}

void Application::resetViewport()
{
    if (!m_viewportState) {
        return;
    }

    m_viewportState->setYaw(0.0);
    m_viewportState->setPitch(0.0);
    m_viewportState->setRoll(0.0);
    m_viewportState->setFieldOfView(90.0);
}

void Application::adjustViewportYaw(double delta)
{
    if (m_viewportState) {
        m_viewportState->setYaw(m_viewportState->yaw() + delta);
    }
}

void Application::adjustViewportPitch(double delta)
{
    if (m_viewportState) {
        m_viewportState->setPitch(m_viewportState->pitch() + delta);
    }
}

void Application::adjustViewportRoll(double delta)
{
    if (m_viewportState) {
        m_viewportState->setRoll(m_viewportState->roll() + delta);
    }
}

void Application::adjustViewportFieldOfView(double delta)
{
    if (m_viewportState) {
        m_viewportState->setFieldOfView(m_viewportState->fieldOfView() + delta);
    }
}

QJsonArray Application::mediaJson() const
{
    QJsonArray array;
    for (const MediaItem &item : m_mediaItems) {
        array.append(item.toJsonObject());
    }
    return array;
}

void Application::restoreMediaFromJson(const QJsonArray &media)
{
    m_mediaItems.clear();
    QSet<QString> seenIds;
    for (const QJsonValue &value : media) {
        if (!value.isObject()) {
            continue;
        }
        MediaItem item;
        QString error;
        if (!item.readFromJsonObject(value.toObject(), &error)) {
            continue;
        }
        // Deterministic normalization: keep the first occurrence of each media
        // id and drop later duplicates, preserving overall order.
        if (seenIds.contains(item.id())) {
            continue;
        }
        seenIds.insert(item.id());
        m_mediaItems.append(item);
    }
}

void Application::restoreActiveMediaFromProject(const QString &persistedId)
{
    const QString previous = m_activeMediaId;
    m_activeMediaId.clear();
    if (!persistedId.isEmpty()) {
        for (const MediaItem &item : m_mediaItems) {
            if (item.id() == persistedId && item.referenceExists()) {
                m_activeMediaId = item.id();
                break;
            }
        }
    }
    if (m_activeMediaId != previous) {
        emit activeMediaChanged(m_activeMediaId);
    }
}

int Application::unavailableMediaCount() const
{
    int count = 0;
    for (const MediaItem &item : m_mediaItems) {
        if (!item.referenceExists()) {
            ++count;
        }
    }
    return count;
}

bool Application::hasUnavailableMedia() const
{
    return unavailableMediaCount() > 0;
}

bool Application::runReframeCommand(const QString &instruction, qint64 startMs,
                                    qint64 endMs)
{
    return runReframeCommandTo(instruction, startMs, endMs, QString());
}

bool Application::runReframeCommandTo(const QString &instruction, qint64 startMs,
                                      qint64 endMs, const QString &outputPath)
{
    ReframeCommandOutcome outcome;
    outcome.instruction = instruction.trimmed();
    outcome.startMs = startMs;
    outcome.endMs = endMs;
    outcome.outputWidth = m_reframeOutputWidth;
    outcome.outputHeight = m_reframeOutputHeight;
    outcome.outputFps = m_reframeOutputFps;

    // Every path (success and failure) records and emits the structured
    // outcome; nothing is silently substituted.
    const auto finish = [this, &outcome]() {
        m_lastReframeOutcome = outcome;
        // Only commands that reached an output target are recorded/persisted;
        // early state/validation failures are reported but not stored as
        // renders. The record is the same structured outcome, so failures carry
        // their error.
        if (!outcome.outputPath.isEmpty()) {
            m_reframeOutputs.append(outcome);
            emit reframeOutputsChanged(m_reframeOutputs);
        }
        emit reframeCommandFinished(m_lastReframeOutcome);
        return m_lastReframeOutcome.ok;
    };

    if (!m_hasProject) {
        outcome.error = QStringLiteral(
            "Open or create a project before running a reframe command.");
        return finish();
    }
    if (outcome.instruction.isEmpty()) {
        outcome.error = QStringLiteral("Enter a reframe command.");
        return finish();
    }
    const MediaItem *media = activeMediaItem();
    if (!media) {
        outcome.error = QStringLiteral(
            "Select an active media item before running a reframe command.");
        return finish();
    }
    if (!media->referenceExists()) {
        outcome.error = QStringLiteral(
            "The active media file is unavailable: %1").arg(media->path());
        return finish();
    }
    outcome.sourceMediaId = media->id();
    outcome.sourcePath = media->path();

    // A zero start/end range means "the whole clip". Resolve it through the
    // replaceable duration-probe seam; when the duration is unknown the runner
    // still honours an explicit command range, otherwise it errors honestly.
    qint64 effectiveStartMs = startMs;
    qint64 effectiveEndMs = endMs;
    if (startMs == 0 && endMs == 0 && m_durationProbe) {
        qint64 durationMs = 0;
        QString probeError;
        if (m_durationProbe->durationMs(media->path(), &durationMs, &probeError)
            && durationMs > 0) {
            effectiveEndMs = durationMs;
            outcome.notes.append(QStringLiteral("Using the whole clip (0..%1 ms).")
                                     .arg(durationMs));
        } else {
            outcome.notes.append(
                QStringLiteral("Could not determine the clip duration; specify "
                               "an explicit time range. (%1)")
                    .arg(probeError.isEmpty() ? QStringLiteral("duration unavailable")
                                              : probeError));
        }
    }
    outcome.startMs = effectiveStartMs;
    outcome.endMs = effectiveEndMs;

    QString resolvedOutput = outputPath.trimmed();
    if (resolvedOutput.isEmpty()) {
        resolvedOutput = defaultReframeOutputPath(*media);
    }
    const QFileInfo outputInfo(resolvedOutput);
    const QDir outputDir = outputInfo.absoluteDir();
    if (!outputDir.exists()) {
        outcome.error = QStringLiteral(
            "The output directory does not exist: %1")
                            .arg(outputDir.absolutePath());
        return finish();
    }
    if (outputInfo.absoluteFilePath()
        == QFileInfo(media->path()).absoluteFilePath()) {
        outcome.error = QStringLiteral(
            "The output path must differ from the source media path.");
        return finish();
    }
    outcome.outputPath = outputInfo.absoluteFilePath();

    // Delegate interpretation, resolution, planning, and execution to the
    // library-level composition boundary (no duplicated logic here).
    ReframeCommandRequest request;
    request.sourcePath = media->path();
    request.sourceMediaId = media->id();
    request.instruction = outcome.instruction;
    request.outputPath = outcome.outputPath;
    request.defaultRange = ReframePlan::TimeRange{ effectiveStartMs, effectiveEndMs };
    // Objective 14: the effective end is the known source upper bound used to
    // resolve a temporal edit (Remove/TargetDuration bounds checks).
    request.sourceDurationMs = effectiveEndMs;
    request.defaultOutput = ReframePlan::OutputSpec{
        m_reframeOutputWidth, m_reframeOutputHeight, m_reframeOutputFps };
    request.speakerProvider = m_speakerProvider;
    request.speakerBindings = m_speakerBindings;
    request.hasCreatorSelection = m_hasCreatorSelection;
    request.creatorSelection = m_creatorSelection;

    const ReframeCommandResult result =
        m_commandExecutor(request, m_targetDetector, m_commandFrameProvider);

    outcome.notes.append(result.notes);
    outcome.unresolvedReferences = result.unresolvedReferences;
    outcome.resolvedTargets = result.resolvedTargets;
    if (result.intent.hasTimeRange) {
        outcome.startMs = result.intent.startMs;
        outcome.endMs = result.intent.endMs;
    }
    // Objective 14: persist the ordered retained ranges when the command
    // performed a temporal edit.
    for (const ReframePlan::TimeRange &segment : result.plan.segments()) {
        outcome.temporalSegments.append(
            qMakePair(segment.startMs, segment.endMs));
    }
    if (!outcome.temporalSegments.isEmpty()) {
        qint64 rangeStart = outcome.temporalSegments.first().first;
        qint64 rangeEnd = outcome.temporalSegments.first().second;
        for (const QPair<qint64, qint64> &segment : outcome.temporalSegments) {
            rangeStart = qMin(rangeStart, segment.first);
            rangeEnd = qMax(rangeEnd, segment.second);
        }
        outcome.startMs = rangeStart;
        outcome.endMs = rangeEnd;
    }
    const ReframePlan::OutputSpec planOutput = result.plan.output();
    if (planOutput.isValid()) {
        outcome.outputWidth = planOutput.width;
        outcome.outputHeight = planOutput.height;
        outcome.outputFps = planOutput.fps;
    }

    if (!result.ok) {
        outcome.ok = false;
        outcome.error = result.error.isEmpty()
            ? QStringLiteral("The reframe command failed.")
            : result.error;
        return finish();
    }
    outcome.ok = true;
    outcome.frameCount = result.frameCount;
    if (!result.outputPath.isEmpty()) {
        outcome.outputPath = result.outputPath;
    }
    return finish();
}

const ReframeCommandOutcome &Application::lastReframeCommandOutcome() const
{
    return m_lastReframeOutcome;
}

void Application::setTargetDetector(TargetDetector *detector)
{
    m_targetDetector = detector;
}

TargetDetector *Application::targetDetector() const
{
    return m_targetDetector;
}

void Application::setCommandFrameProvider(ReframeFrameProvider *provider)
{
    m_commandFrameProvider = provider;
}

ReframeFrameProvider *Application::commandFrameProvider() const
{
    return m_commandFrameProvider;
}

void Application::setReframeCommandExecutor(
    const ReframeCommandExecutor &executor)
{
    if (executor) {
        m_commandExecutor = executor;
    }
}

void Application::resetReframeCommandExecutor()
{
    m_commandExecutor = [](const ReframeCommandRequest &request,
                           TargetDetector *detector,
                           ReframeFrameProvider *provider) {
        return ReframeCommandRunner::run(request, detector, provider);
    };
}

void Application::setReframeDefaultOutput(int width, int height, double fps)
{
    if (width > 0) {
        m_reframeOutputWidth = width;
    }
    if (height > 0) {
        m_reframeOutputHeight = height;
    }
    if (fps > 0.0) {
        m_reframeOutputFps = fps;
    }
}

QString Application::defaultReframeOutputPath(const MediaItem &media) const
{
    const QFileInfo info(media.path());
    return info.absoluteDir().filePath(
        info.completeBaseName() + QStringLiteral("_reframe.mp4"));
}

QList<ReframeCommandOutcome> Application::reframeOutputs() const
{
    return m_reframeOutputs;
}

void Application::setMediaDurationProbe(MediaDurationProbe *probe)
{
    m_durationProbe = probe ? probe : m_ownedDurationProbe.get();
}

MediaDurationProbe *Application::mediaDurationProbe() const
{
    return m_durationProbe;
}

void Application::setSpeakerEvidenceProvider(SpeakerEvidenceProvider *provider)
{
    m_speakerProvider = provider;
}

SpeakerEvidenceProvider *Application::speakerEvidenceProvider() const
{
    return m_speakerProvider;
}

void Application::setSpeakerBindings(
    const QList<QPair<QString, QString>> &bindings)
{
    m_speakerBindings = bindings;
}

QList<QPair<QString, QString>> Application::speakerBindings() const
{
    return m_speakerBindings;
}

bool Application::selectCreatorTargetFromViewport()
{
    if (!m_hasProject) {
        emit backgroundCompleted(QStringLiteral(
            "Open or create a project before selecting a creator target."));
        return false;
    }
    const MediaItem *media = activeMediaItem();
    if (!media) {
        emit backgroundCompleted(QStringLiteral(
            "Select an active media item before selecting a creator target."));
        return false;
    }
    CreatorTargetSelection selection;
    selection.identity = TargetIdentityRegistry::creatorIdentity();
    selection.timeMs = static_cast<qint64>(m_previewTimeSeconds * 1000.0);
    selection.yawDeg = m_viewportState ? m_viewportState->yaw() : 0.0;
    selection.pitchDeg = m_viewportState ? m_viewportState->pitch() : 0.0;
    selection.label = QStringLiteral("person");
    selection.evidence = QStringLiteral("creator selected the viewport center");
    m_creatorSelection = selection;
    m_hasCreatorSelection = true;
    emit creatorSelectionChanged(true);
    emit backgroundCompleted(
        QStringLiteral("Creator target 'me' set at yaw %1, pitch %2.")
            .arg(selection.yawDeg, 0, 'f', 1)
            .arg(selection.pitchDeg, 0, 'f', 1));
    return true;
}

void Application::clearCreatorSelection()
{
    if (!m_hasCreatorSelection) {
        return;
    }
    m_hasCreatorSelection = false;
    m_creatorSelection = CreatorTargetSelection();
    emit creatorSelectionChanged(false);
    emit backgroundCompleted(QStringLiteral("Creator target 'me' cleared."));
}

bool Application::hasCreatorSelection() const
{
    return m_hasCreatorSelection;
}

CreatorTargetSelection Application::creatorSelection() const
{
    return m_creatorSelection;
}

bool Application::previewReframeOutput(int index)
{
    if (index < 0 || index >= m_reframeOutputs.size()) {
        emit backgroundCompleted(
            QStringLiteral("Select a generated render to preview."));
        return false;
    }
    const ReframeCommandOutcome &record = m_reframeOutputs.at(index);
    if (record.outputPath.isEmpty() || !QFileInfo::exists(record.outputPath)) {
        emit backgroundCompleted(QStringLiteral(
            "The render output is unavailable: %1").arg(record.outputPath));
        return false;
    }
    QImage image;
    QString error;
    if (!m_previewDecoder
        || !m_previewDecoder(record.outputPath, &image, &error)
        || image.isNull()) {
        emit backgroundCompleted(
            QStringLiteral("Could not preview the render: %1")
                .arg(error.isEmpty() ? QStringLiteral("no frame decoded")
                                     : error));
        return false;
    }
    emit backgroundCompleted(
        QStringLiteral("Previewing render: %1").arg(record.outputPath));
    emit reframeOutputPreviewReady(image);
    return true;
}

void Application::setReframePreviewDecoder(const ReframePreviewDecoder &decoder)
{
    if (decoder) {
        m_previewDecoder = decoder;
    }
}

void Application::resetReframePreviewDecoder()
{
    m_previewDecoder = [](const QString &path, QImage *out, QString *error) {
        return FrameExtractor::extractFirstFrame(
            path, FrameExtractor::defaultExecutablePath(), out, error);
    };
}


QJsonArray Application::reframeOutputsJson() const
{
    QJsonArray array;
    for (const ReframeCommandOutcome &outcome : m_reframeOutputs) {
        array.append(outcome.toJsonObject());
    }
    return array;
}

void Application::restoreReframeOutputsFromJson(const QJsonArray &outputs)
{
    m_reframeOutputs.clear();
    for (const QJsonValue &value : outputs) {
        if (!value.isObject()) {
            continue;
        }
        ReframeCommandOutcome outcome;
        QString error;
        if (ReframeCommandOutcome::readFromJsonObject(value.toObject(), &outcome,
                                                      &error)) {
            m_reframeOutputs.append(outcome);
        }
    }
}

int Application::playbackIntervalMsForFps(double fps) const
{
    if (fps > 0.0) {
        const int interval = static_cast<int>(std::lround(1000.0 / fps));
        return interval > 0 ? interval : 1;
    }
    return 40; // matches the Player's default 25 fps pacing
}

bool Application::startReframeOutputPlayback(int index)
{
    if (index < 0 || index >= m_reframeOutputs.size()) {
        emit backgroundCompleted(
            QStringLiteral("Select a generated render to play."));
        return false;
    }
    const ReframeCommandOutcome &record = m_reframeOutputs.at(index);
    if (record.outputPath.isEmpty() || !QFileInfo::exists(record.outputPath)) {
        emit backgroundCompleted(QStringLiteral(
            "The render output is unavailable: %1").arg(record.outputPath));
        return false;
    }
    if (record.outputWidth <= 0 || record.outputHeight <= 0) {
        emit backgroundCompleted(QStringLiteral(
            "The render dimensions are unknown; cannot play it back."));
        return false;
    }

    // The same paused record resumes rather than restarting.
    if (m_playbackPlayer && m_playbackRecordIndex == index
        && m_playbackPlayer->isPaused()) {
        return resumeReframeOutputPlayback();
    }

    stopReframeOutputPlayback();

    QString error;
    std::unique_ptr<FrameSource> source = m_playbackSourceFactory(record, &error);
    if (!source || !source->isOpen()) {
        emit backgroundCompleted(
            QStringLiteral("Could not open the render for playback: %1")
                .arg(error.isEmpty() ? QStringLiteral("source unavailable")
                                     : error));
        return false;
    }
    m_playbackSource = std::move(source);
    m_playbackPump = std::make_unique<FramePump>();
    m_playbackPump->setSource(m_playbackSource.get());
    m_playbackPlayer = std::make_unique<Player>(
        m_playbackPump.get(), m_playbackClock, m_playbackPacing);
    m_playbackRecordIndex = index;

    connect(m_playbackPlayer.get(), &Player::framePresented, this,
            [this](const QImage &image, qint64, qint64) {
                emit reframePlaybackFrameReady(image);
            });
    connect(m_playbackPlayer.get(), &Player::stateChanged, this,
            [this](Player::State state) {
                if (state != Player::State::Playing && m_playbackTimer) {
                    m_playbackTimer->stop();
                }
                emit reframePlaybackStateChanged(state == Player::State::Playing);
            });
    connect(m_playbackPlayer.get(), &Player::positionChanged, this,
            [this](qint64 frameCount, qint64 positionMs) {
                emit reframePlaybackPositionChanged(frameCount, positionMs);
            });
    connect(m_playbackPlayer.get(), &Player::playbackEnded, this, [this]() {
        if (m_playbackTimer) {
            m_playbackTimer->stop();
        }
        emit reframePlaybackEnded();
    });
    connect(m_playbackPlayer.get(), &Player::errorOccurred, this,
            [this](const QString &message) {
                if (m_playbackTimer) {
                    m_playbackTimer->stop();
                }
                emit backgroundCompleted(
                    QStringLiteral("Playback failed: %1").arg(message));
            });

    const int intervalMs = playbackIntervalMsForFps(record.outputFps);
    m_playbackPlayer->setFrameIntervalMs(intervalMs);
    m_playbackPlayer->play();
    if (m_playbackTimer) {
        m_playbackTimer->start(qBound(10, intervalMs, 100));
    }
    return true;
}

bool Application::pauseReframeOutputPlayback()
{
    if (!m_playbackPlayer || !m_playbackPlayer->isPlaying()) {
        return false;
    }
    if (m_playbackTimer) {
        m_playbackTimer->stop();
    }
    m_playbackPlayer->pause();
    return true;
}

bool Application::resumeReframeOutputPlayback()
{
    if (!m_playbackPlayer || !m_playbackPlayer->isPaused()) {
        return false;
    }
    m_playbackPlayer->play();
    if (m_playbackTimer && m_playbackRecordIndex >= 0
        && m_playbackRecordIndex < m_reframeOutputs.size()) {
        const int intervalMs = playbackIntervalMsForFps(
            m_reframeOutputs.at(m_playbackRecordIndex).outputFps);
        m_playbackTimer->start(qBound(10, intervalMs, 100));
    }
    return true;
}

void Application::stopReframeOutputPlayback()
{
    const bool wasActive = m_playbackPlayer != nullptr;
    if (m_playbackTimer) {
        m_playbackTimer->stop();
    }
    if (m_playbackPlayer) {
        disconnect(m_playbackPlayer.get(), nullptr, this, nullptr);
        m_playbackPlayer->stop();
        m_playbackPlayer.reset();
    }
    m_playbackPump.reset();
    if (m_playbackSource) {
        m_playbackSource->close();
        m_playbackSource.reset();
    }
    m_playbackRecordIndex = -1;
    if (wasActive) {
        emit reframePlaybackStateChanged(false);
    }
}

int Application::tickReframeOutputPlayback()
{
    if (!m_playbackPlayer) {
        return 0;
    }
    return m_playbackPlayer->tick();
}

bool Application::isReframeOutputPlaybackActive() const
{
    return m_playbackPlayer != nullptr;
}

bool Application::isReframeOutputPlaying() const
{
    return m_playbackPlayer && m_playbackPlayer->isPlaying();
}

qint64 Application::reframeOutputPlaybackFrameCount() const
{
    return m_playbackPlayer ? m_playbackPlayer->frameCount() : 0;
}

qint64 Application::reframeOutputPlaybackPositionMs() const
{
    return m_playbackPlayer ? m_playbackPlayer->positionMs() : 0;
}

int Application::reframeOutputPlaybackRecordIndex() const
{
    return m_playbackRecordIndex;
}

void Application::setPlaybackSourceFactory(const PlaybackSourceFactory &factory)
{
    if (factory) {
        m_playbackSourceFactory = factory;
    }
}

void Application::resetPlaybackSourceFactory()
{
    m_playbackSourceFactory =
        [](const ReframeCommandOutcome &record, QString *error)
        -> std::unique_ptr<FrameSource> {
        std::unique_ptr<FfmpegFrameSource> source =
            std::make_unique<FfmpegFrameSource>();
        if (!source->open(record.outputPath, record.outputWidth,
                          record.outputHeight)) {
            if (error) {
                *error = source->errorString();
            }
            return nullptr;
        }
        return source;
    };
}

void Application::setPlaybackClock(Clock *clock)
{
    m_playbackClock = clock;
}

void Application::setPlaybackPacing(PacingPolicy *pacing)
{
    m_playbackPacing = pacing;
}

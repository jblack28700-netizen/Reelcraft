#include "Application.h"

#include <QtConcurrent/QtConcurrent>
#include <QSet>
#include <QThread>

#include "media/FrameExtractor.h"
#include "viewer/ViewportState.h"

Application::Application(QObject *parent)
    : QObject(parent),
      m_viewportState(new ViewportState(this))
{
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
    m_currentProject = Project();
    m_hasProject = true;
    const bool hadActiveMedia = !m_activeMediaId.isEmpty();
    const bool hadPreviewTime = m_previewTimeSeconds != 0.0;
    m_mediaItems.clear();
    m_activeMediaId.clear();
    m_previewTimeSeconds = 0.0;
    resetViewport();
    emit mediaListChanged(m_mediaItems);
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

    m_currentProject = loaded;
    m_hasProject = true;
    resetViewport();
    restoreMediaFromJson(m_currentProject.media());

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

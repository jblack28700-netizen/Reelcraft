#pragma once

#include <QObject>
#include <QList>

#include "core/MediaItem.h"
#include "core/Project.h"

class ViewportState;

class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);

    void initialize();

    Project currentProject() const;
    bool hasProject() const;
    ViewportState *viewportState() const;

    // The authoritative, deterministic media list (import order preserved).
    QList<MediaItem> mediaItems() const;

    // Point-in-time filesystem availability of the current media list. Media
    // records are never removed when their file becomes unavailable; they are
    // surfaced through these helpers instead.
    bool hasUnavailableMedia() const;
    int unavailableMediaCount() const;

    // The active/selected media ("viewer source" contract). The invariant is:
    // the active id is empty, or it resolves to exactly one current MediaItem
    // in the normalized list whose file was available at selection/restore
    // time. It is never a dangling reference.
    QString activeMediaId() const;
    const MediaItem *activeMediaItem() const;

public slots:
    void newProject();
    bool saveProject(const QString &filePath);
    bool openProject(const QString &filePath);
    void runBackgroundDemo();
    void resetViewport();
    void adjustViewportYaw(double delta);
    void adjustViewportPitch(double delta);
    void adjustViewportRoll(double delta);
    void adjustViewportFieldOfView(double delta);

    // Validates a real media file and records it in the current project.
    // Requires an active project; duplicates of the same canonical path are
    // idempotent. The original media file is never modified.
    bool importMediaFile(const QString &filePath);

    // Removes the media record with the given id from the current project.
    // Requires an active project and an existing media id. Only the record is
    // removed; the referenced file is never modified or deleted. If the active
    // media is removed, the active id is cleared.
    bool removeMedia(const QString &mediaId);

    // Sets the active/selected media to the record with the given id. Requires
    // an active project, an existing media id, and an available referenced
    // file. Never auto-selects on import; idempotent when already active.
    bool setActiveMedia(const QString &mediaId);

signals:
    void projectChanged(const Project &project);
    void backgroundCompleted(const QString &message);

    // Emitted whenever the authoritative media list changes: after a
    // successful import that appends a record, after a successful removal,
    // after a project is opened, and after a new project is created (empty).
    void mediaListChanged(const QList<MediaItem> &items);

    // Emitted whenever the active media changes (empty id = none active).
    void activeMediaChanged(const QString &mediaId);

private:
    QJsonArray mediaJson() const;
    void restoreMediaFromJson(const QJsonArray &media);
    // Restores the active id from a persisted value after the media list has
    // been normalized: the id is kept only when it resolves to a current,
    // available media record; otherwise it is cleared deterministically.
    void restoreActiveMediaFromProject(const QString &persistedId);

    Project m_currentProject;
    bool m_hasProject = false;
    ViewportState *m_viewportState = nullptr;
    QList<MediaItem> m_mediaItems;
    QString m_activeMediaId;
};

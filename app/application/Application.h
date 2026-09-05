#pragma once

#include <QObject>
#include <QImage>
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

    // Decodes a single preview frame from the active media (FFmpeg-CLI decode
    // adapter, Objective 9) and emits framePreviewReady() on success. Requires
    // an active project, an active media record, and an available file. The
    // media file is never modified. The frame is presented through the
    // existing viewer pixel path; no viewer code changes. Decodes at the
    // current preview time position.
    bool previewActiveMediaFrame();

    // Requests a preview frame at the given time position (seconds) of the
    // active media. Negative positions clamp to 0. Beyond-end/undecodable
    // requests fail deterministically and leave the current preview position
    // unchanged. The requested position is a seek/preview request, not a
    // guarantee of exact frame or presentation-timestamp accuracy.
    bool previewActiveMediaFrameAt(double seconds);

    // Steps the current preview position by deltaSeconds and requests a frame
    // there (floor of 0 when stepping below the start). Beyond-end steps fail
    // deterministically and leave the position unchanged.
    bool stepActiveMediaPreview(double deltaSeconds);

    // The current preview time position (seconds; session state only, not
    // persisted). Reset to 0 on new project, project open, active-media
    // change, and removal of the active media.
    double previewTimeSeconds() const;

signals:
    void projectChanged(const Project &project);
    void backgroundCompleted(const QString &message);

    // Emitted whenever the authoritative media list changes: after a
    // successful import that appends a record, after a successful removal,
    // after a project is opened, and after a new project is created (empty).
    void mediaListChanged(const QList<MediaItem> &items);

    // Emitted whenever the active media changes (empty id = none active).
    void activeMediaChanged(const QString &mediaId);

    // Emitted after previewActiveMediaFrame() decodes a frame successfully.
    void framePreviewReady(const QImage &image);

    // Emitted whenever the current preview time position changes.
    void previewTimeChanged(double seconds);

private:
    bool decodePreviewFrameAt(double targetSeconds);

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
    double m_previewTimeSeconds = 0.0;
};

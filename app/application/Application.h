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

signals:
    void projectChanged(const Project &project);
    void backgroundCompleted(const QString &message);

private:
    QJsonArray mediaJson() const;
    void restoreMediaFromJson(const QJsonArray &media);

    Project m_currentProject;
    bool m_hasProject = false;
    ViewportState *m_viewportState = nullptr;
    QList<MediaItem> m_mediaItems;
};

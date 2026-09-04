#pragma once

#include <QObject>

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

public slots:
    void newProject();
    bool saveProject(const QString &filePath);
    bool openProject(const QString &filePath);
    void runBackgroundDemo();
    void resetViewport();

signals:
    void projectChanged(const Project &project);
    void backgroundCompleted(const QString &message);

private:
    Project m_currentProject;
    bool m_hasProject = false;
    ViewportState *m_viewportState = nullptr;
};

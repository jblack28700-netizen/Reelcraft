#pragma once

#include <QMainWindow>

#include "core/Project.h"

class QLabel;
class QPushButton;
class QKeyEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    virtual QString chooseSaveFilePath();
    virtual QString chooseOpenFilePath();
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void showProject(const Project &project);
    void showStatus(const QString &message);
    void showYaw(double value);
    void showPitch(double value);
    void showRoll(double value);
    void showFieldOfView(double value);

signals:
    void newProjectRequested();
    void saveProjectRequested(const QString &filePath);
    void openProjectRequested(const QString &filePath);
    void backgroundDemoRequested();
    void resetViewportRequested();
    void viewportYawDeltaRequested(double delta);
    void viewportPitchDeltaRequested(double delta);
    void viewportRollDeltaRequested(double delta);
    void viewportFovDeltaRequested(double delta);

private:
    QLabel *m_projectLabel = nullptr;
    QLabel *m_statusLabel = nullptr;

    QLabel *m_yawLabel = nullptr;
    QLabel *m_pitchLabel = nullptr;
    QLabel *m_rollLabel = nullptr;
    QLabel *m_fieldOfViewLabel = nullptr;

    QPushButton *m_newProjectButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_openButton = nullptr;
    QPushButton *m_backgroundButton = nullptr;
    QPushButton *m_resetViewportButton = nullptr;
};

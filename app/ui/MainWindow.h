#pragma once

#include <QImage>
#include <QMainWindow>

#include "core/MediaItem.h"
#include "core/Project.h"

class QLabel;
class QListWidget;
class QPushButton;
class QKeyEvent;
class ViewerWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    // The embedded viewer presentation surface. UI code (e.g. main.cpp wiring)
    // uses this to supply the authoritative viewport state to the viewer.
    ViewerWidget *viewerWidget() const { return m_viewerWidget; }

protected:
    virtual QString chooseSaveFilePath();
    virtual QString chooseOpenFilePath();
    virtual QString chooseMediaFilePath();
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void showProject(const Project &project);
    void showStatus(const QString &message);
    void showYaw(double value);
    void showPitch(double value);
    void showRoll(double value);
    void showFieldOfView(double value);

    // Rebuilds the project media list display from the authoritative list.
    void showMediaList(const QList<MediaItem> &items);

    // Updates the active/selected media display (empty id = none active).
    void showActiveMedia(const QString &mediaId);

    // Presents a decoded preview frame through the viewer pixel path.
    void showFramePreview(const QImage &image);

signals:
    void newProjectRequested();
    void saveProjectRequested(const QString &filePath);
    void openProjectRequested(const QString &filePath);
    void importMediaRequested(const QString &filePath);
    void removeMediaRequested(const QString &mediaId);
    void setActiveRequested(const QString &mediaId);
    void previewFrameRequested();
    void backgroundDemoRequested();
    void resetViewportRequested();
    void viewportYawDeltaRequested(double delta);
    void viewportPitchDeltaRequested(double delta);
    void viewportRollDeltaRequested(double delta);
    void viewportFovDeltaRequested(double delta);

private:
    QLabel *m_projectLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_activeMediaLabel = nullptr;

    QLabel *m_yawLabel = nullptr;
    QLabel *m_pitchLabel = nullptr;
    QLabel *m_rollLabel = nullptr;
    QLabel *m_fieldOfViewLabel = nullptr;

    QPushButton *m_newProjectButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_openButton = nullptr;
    QPushButton *m_importButton = nullptr;
    QPushButton *m_removeMediaButton = nullptr;
    QPushButton *m_setActiveButton = nullptr;
    QPushButton *m_previewFrameButton = nullptr;
    QPushButton *m_backgroundButton = nullptr;
    QPushButton *m_resetViewportButton = nullptr;

    QListWidget *m_mediaListWidget = nullptr;

    ViewerWidget *m_viewerWidget = nullptr;

    // Active media id used to mark the active row in the list display.
    QString m_activeMediaIdText;
    void refreshActiveMarking();
};

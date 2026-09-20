#pragma once

#include <QImage>
#include <QMainWindow>

#include "application/DecisionProvenance.h"
#include "application/ReframeCommandOutcome.h"
#include "application/ReframePlanReview.h"
#include "core/MediaItem.h"
#include "core/Project.h"

class QLabel;
class QLineEdit;
class QDoubleSpinBox;
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

    // Updates the preview time readout (seconds).
    void showPreviewTime(double seconds);

    // Presents the structured result of a 360 reframe command (Objective 9).
    void showReframeCommandResult(const ReframeCommandOutcome &outcome);

    // Rebuilds the generated-render list from the authoritative records
    // (Objective 10).
    void showReframeOutputs(const QList<ReframeCommandOutcome> &outputs);

    // Updates the creator "me" selection readout (Objective 12).
    void showCreatorSelection(bool hasSelection, double yawDeg, double pitchDeg);

    // Presents a generated render's decoded frame flat (Objective 12): a
    // reframed output is a flat video, so no equirectangular camera transform is
    // applied.
    void showReframeOutputPreview(const QImage &image);

    // Shows whether the replaceable detector/speaker providers are configured
    // (Objective 12). The providers are external and optional.
    void showProviderStatus(bool hasDetector, bool hasSpeaker);

    // Presents the pending creator review of a prepared plan (Objective 34), or
    // the "nothing is waiting" state for an invalid review. The panel is a VIEW:
    // it displays what the plan will do and offers accept or reject, and it
    // cannot change the plan.
    void showReframeReview(const ReframePlanReview &review);

    // Presents the read-only "why" of the decision behind a render record
    // (Objective 35): how it was formed, what it revises, whether that parent is
    // present, the source status and the plan summary. Display only.
    //
    // revisionsOf carries the DERIVED supersession (Objective 36): the indices of
    // the held records that revise this one. Nothing is stored to produce it.
    void showDecisionProvenance(const DecisionProvenance &view, int index,
                                const QList<int> &revisionsOf);

    // Updates the rendered-result playback state readout (Objective 13).
    void showReframePlaybackState(bool playing);
    void showReframePlaybackPosition(qint64 frameCount, qint64 positionMs);

    // Objective 19: source-media playback presentation.
    void showSourcePlaybackState(bool playing);
    void showSourcePlaybackPosition(qint64 positionMs);

signals:
    void newProjectRequested();
    void saveProjectRequested(const QString &filePath);
    void openProjectRequested(const QString &filePath);
    void importMediaRequested(const QString &filePath);
    void removeMediaRequested(const QString &mediaId);
    void setActiveRequested(const QString &mediaId);
    void previewFrameRequested();
    void previewStepRequested(double deltaSeconds);
    void setMediaProjectionRequested(const QString &mediaId, const QString &projectionValue);
    void reframeCommandRequested(const QString &instruction, qint64 startMs,
                                 qint64 endMs);
    // Objective 34: "review the plan before committing to a render" and the two
    // decisions a creator can make about a reviewed plan.
    void reframeReviewRequested(const QString &instruction, qint64 startMs,
                                qint64 endMs);
    void acceptReframeReviewRequested();
    void rejectReframeReviewRequested();
    // Objective 35: revise a RENDERED record from a new instruction, and ask why
    // a recorded decision was made. Requests only; the application acts.
    void reviseReframeOutputRequested(int index, const QString &instruction);
    void describeDecisionRequested(int index);
    // Objective 40: widen the lens of the SELECTED rendered decision. The
    // application chooses the ladder step and refuses honestly; the button only
    // asks for "the next wider lens" of that record.
    void widenRenderLensRequested(int index);
    void selectCreatorTargetRequested();
    void clearCreatorTargetRequested();
    void previewReframeOutputRequested(int index);
    void playReframeOutputRequested(int index);
    void pauseReframeOutputPlaybackRequested();
    void stopReframeOutputPlaybackRequested();

    // Objective 19: source-media playback requests.
    void playSourceRequested();
    void pauseSourceRequested();
    void stopSourceRequested();
    void seekSourceRequested(qint64 positionMs);
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
    QPushButton *m_stepBackButton = nullptr;
    QPushButton *m_stepForwardButton = nullptr;
    QPushButton *m_markFlatButton = nullptr;
    QPushButton *m_markEquirectButton = nullptr;
    QPushButton *m_backgroundButton = nullptr;
    QPushButton *m_resetViewportButton = nullptr;

    QLabel *m_previewTimeLabel = nullptr;
    QListWidget *m_mediaListWidget = nullptr;

    // 360 reframe command interface (Objective 9).
    QLineEdit *m_commandEdit = nullptr;
    QDoubleSpinBox *m_commandStartSeconds = nullptr;
    QDoubleSpinBox *m_commandEndSeconds = nullptr;
    QPushButton *m_runCommandButton = nullptr;
    QLabel *m_commandResultLabel = nullptr;

    // Objective 34: creator review panel (inspect -> accept or reject).
    QPushButton *m_reviewButton = nullptr;
    QPushButton *m_acceptReviewButton = nullptr;
    QPushButton *m_rejectReviewButton = nullptr;
    QLabel *m_reviewSummaryLabel = nullptr;

    // Objective 35: record-level revision and the provenance readout.
    QLineEdit *m_revisionEdit = nullptr;
    QPushButton *m_reviseRenderButton = nullptr;
    QPushButton *m_provenanceButton = nullptr;
    QLabel *m_provenanceLabel = nullptr;
    // Objective 40: the lens-widening action (no narrowing control exists).
    QPushButton *m_widenLensButton = nullptr;
    QListWidget *m_reframeOutputsList = nullptr;

    // Objective 12: creator selection, render preview, provider status.
    QPushButton *m_selectCreatorButton = nullptr;
    QPushButton *m_clearCreatorButton = nullptr;
    QLabel *m_creatorSelectionLabel = nullptr;
    QPushButton *m_previewRenderButton = nullptr;
    QLabel *m_providersLabel = nullptr;

    // Objective 13: rendered-result playback controls.
    QPushButton *m_playRenderButton = nullptr;
    QPushButton *m_pauseRenderButton = nullptr;
    QPushButton *m_stopRenderButton = nullptr;
    QLabel *m_playbackPositionLabel = nullptr;

    // Objective 19: source-media playback controls.
    QPushButton *m_playSourceButton = nullptr;
    QPushButton *m_pauseSourceButton = nullptr;
    QPushButton *m_stopSourceButton = nullptr;
    QPushButton *m_seekSourceButton = nullptr;
    QDoubleSpinBox *m_sourceSeekSeconds = nullptr;
    QLabel *m_sourcePlaybackLabel = nullptr;

    ViewerWidget *m_viewerWidget = nullptr;

    // Active media id used to mark the active row in the list display.
    QString m_activeMediaIdText;
    void refreshActiveMarking();
    // Returns the viewer to the marker-scene presentation (clears any decoded
    // frame and resets flat mode) when the project/active-media context ends.
    void clearViewerSource();
};

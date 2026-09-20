#include "MainWindow.h"

#include <QFileDialog>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "ui/ViewerWidget.h"

namespace {

constexpr int kMediaIdRole = Qt::UserRole;
constexpr int kMediaNameRole = Qt::UserRole + 1;
constexpr int kMediaTagRole = Qt::UserRole + 2;
constexpr int kMediaProjectionRole = Qt::UserRole + 3;
const QString kActivePrefix = QStringLiteral("▶ ");

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Reelcraft");
    resize(1200, 800);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_projectLabel = new QLabel(QStringLiteral("No project"), central);
    m_statusLabel = new QLabel(QStringLiteral("Ready"), central);
    m_activeMediaLabel = new QLabel(QStringLiteral("Active media: None"), central);

    m_yawLabel = new QLabel(QStringLiteral("Yaw: 0.0"), central);
    m_pitchLabel = new QLabel(QStringLiteral("Pitch: 0.0"), central);
    m_rollLabel = new QLabel(QStringLiteral("Roll: 0.0"), central);
    m_fieldOfViewLabel = new QLabel(QStringLiteral("FOV: 90.0"), central);

    m_newProjectButton = new QPushButton(QStringLiteral("New Project"), central);
    m_saveButton = new QPushButton(QStringLiteral("Save Project"), central);
    m_openButton = new QPushButton(QStringLiteral("Open Project"), central);
    m_importButton = new QPushButton(QStringLiteral("Import Media"), central);
    m_removeMediaButton = new QPushButton(QStringLiteral("Remove Media"), central);
    m_setActiveButton = new QPushButton(QStringLiteral("Set Active"), central);
    m_previewFrameButton = new QPushButton(QStringLiteral("Preview Active Frame"), central);
    m_stepBackButton = new QPushButton(QStringLiteral("Step -1 s"), central);
    m_stepForwardButton = new QPushButton(QStringLiteral("Step +1 s"), central);
    m_markFlatButton = new QPushButton(QStringLiteral("Mark Flat"), central);
    m_markEquirectButton = new QPushButton(QStringLiteral("Mark Equirect"), central);
    m_backgroundButton = new QPushButton(QStringLiteral("Run Background Demo"), central);
    m_resetViewportButton = new QPushButton(QStringLiteral("Reset Viewport"), central);

    m_previewTimeLabel = new QLabel(QStringLiteral("Time: 0.0 s"), central);

    m_commandEdit = new QLineEdit(central);
    m_commandEdit->setPlaceholderText(
        QStringLiteral("e.g. From 00:00 to 00:10, follow person 1"));
    m_commandStartSeconds = new QDoubleSpinBox(central);
    m_commandStartSeconds->setRange(0.0, 36000.0);
    m_commandStartSeconds->setDecimals(2);
    m_commandStartSeconds->setPrefix(QStringLiteral("start s (0=whole): "));
    m_commandStartSeconds->setValue(0.0);
    m_commandEndSeconds = new QDoubleSpinBox(central);
    m_commandEndSeconds->setRange(0.0, 36000.0);
    m_commandEndSeconds->setDecimals(2);
    m_commandEndSeconds->setPrefix(QStringLiteral("end s (0=whole): "));
    m_commandEndSeconds->setValue(0.0);
    m_runCommandButton = new QPushButton(QStringLiteral("Run 360 Command"), central);
    m_commandResultLabel = new QLabel(QStringLiteral("No reframe command run."), central);

    // Objective 34: inspect the plan before committing to a render.
    m_reviewButton = new QPushButton(QStringLiteral("Review Plan"), central);
    m_acceptReviewButton = new QPushButton(QStringLiteral("Accept & Render"), central);
    m_rejectReviewButton = new QPushButton(QStringLiteral("Reject"), central);
    m_reviewSummaryLabel = new QLabel(
        QStringLiteral("No plan is waiting for review."), central);
    m_reviewSummaryLabel->setWordWrap(true);
    m_reviewSummaryLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // Objective 35: revise a render that already exists, and ask why it was made.
    m_revisionEdit = new QLineEdit(central);
    m_revisionEdit->setPlaceholderText(
        QStringLiteral("e.g. keep me centered and zoom in"));
    m_reviseRenderButton =
        new QPushButton(QStringLiteral("Revise Selected Render"), central);
    m_provenanceButton =
        new QPushButton(QStringLiteral("Why This Decision?"), central);
    m_provenanceLabel = new QLabel(
        QStringLiteral("Select a generated render for its decision provenance."),
        central);
    m_provenanceLabel->setWordWrap(true);
    m_provenanceLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // A plan must be reviewed before it can be accepted or rejected.
    m_acceptReviewButton->setEnabled(false);
    m_rejectReviewButton->setEnabled(false);
    m_reframeOutputsList = new QListWidget(central);
    m_reframeOutputsList->setMinimumHeight(60);
    m_previewRenderButton = new QPushButton(QStringLiteral("Preview Selected Render"), central);
    m_selectCreatorButton = new QPushButton(QStringLiteral("Select Center as Me"), central);
    m_clearCreatorButton = new QPushButton(QStringLiteral("Clear Me"), central);
    m_creatorSelectionLabel = new QLabel(QStringLiteral("Creator target 'me': none"), central);
    m_providersLabel = new QLabel(QStringLiteral("Providers: detector=no, speaker=no"), central);
    m_playRenderButton = new QPushButton(QStringLiteral("Play Render"), central);
    m_pauseRenderButton = new QPushButton(QStringLiteral("Pause Render"), central);
    m_stopRenderButton = new QPushButton(QStringLiteral("Stop Render"), central);
    m_playbackPositionLabel = new QLabel(QStringLiteral("Playback: stopped"), central);

    m_playSourceButton = new QPushButton(QStringLiteral("Play Source"), central);
    m_pauseSourceButton = new QPushButton(QStringLiteral("Pause Source"), central);
    m_stopSourceButton = new QPushButton(QStringLiteral("Stop Source"), central);
    m_sourceSeekSeconds = new QDoubleSpinBox(central);
    m_sourceSeekSeconds->setRange(0.0, 100000.0);
    m_sourceSeekSeconds->setDecimals(3);
    m_sourceSeekSeconds->setSingleStep(1.0);
    m_sourceSeekSeconds->setValue(0.0);
    m_seekSourceButton = new QPushButton(QStringLiteral("Seek Source"), central);
    m_sourcePlaybackLabel = new QLabel(QStringLiteral("Source: stopped"), central);

    m_mediaListWidget = new QListWidget(central);
    m_mediaListWidget->setObjectName("mediaListWidget");
    m_mediaListWidget->setMinimumHeight(80);
    m_mediaListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    m_projectLabel->setObjectName("projectLabel");
    m_statusLabel->setObjectName("statusLabel");
    m_activeMediaLabel->setObjectName("activeMediaLabel");
    m_yawLabel->setObjectName("yawLabel");
    m_pitchLabel->setObjectName("pitchLabel");
    m_rollLabel->setObjectName("rollLabel");
    m_fieldOfViewLabel->setObjectName("fieldOfViewLabel");
    m_newProjectButton->setObjectName("newProjectButton");
    m_saveButton->setObjectName("saveProjectButton");
    m_openButton->setObjectName("openProjectButton");
    m_importButton->setObjectName("importMediaButton");
    m_removeMediaButton->setObjectName("removeMediaButton");
    m_setActiveButton->setObjectName("setActiveButton");
    m_previewFrameButton->setObjectName("previewFrameButton");
    m_stepBackButton->setObjectName("stepBackButton");
    m_stepForwardButton->setObjectName("stepForwardButton");
    m_markFlatButton->setObjectName("markFlatButton");
    m_markEquirectButton->setObjectName("markEquirectButton");
    m_previewTimeLabel->setObjectName("previewTimeLabel");
    m_backgroundButton->setObjectName("backgroundDemoButton");
    m_resetViewportButton->setObjectName("resetViewportButton");
    m_commandEdit->setObjectName("reframeCommandEdit");
    m_commandStartSeconds->setObjectName("reframeStartSeconds");
    m_commandEndSeconds->setObjectName("reframeEndSeconds");
    m_runCommandButton->setObjectName("runReframeCommandButton");
    m_commandResultLabel->setObjectName("reframeResultLabel");
    m_reviewButton->setObjectName("reviewReframePlanButton");
    m_acceptReviewButton->setObjectName("acceptReframeReviewButton");
    m_rejectReviewButton->setObjectName("rejectReframeReviewButton");
    m_reviewSummaryLabel->setObjectName("reframeReviewSummaryLabel");
    m_revisionEdit->setObjectName("revisionInstructionEdit");
    m_reviseRenderButton->setObjectName("reviseRenderButton");
    m_provenanceButton->setObjectName("describeDecisionButton");
    m_provenanceLabel->setObjectName("decisionProvenanceLabel");
    m_reframeOutputsList->setObjectName("reframeOutputsList");
    m_previewRenderButton->setObjectName("previewRenderButton");
    m_selectCreatorButton->setObjectName("selectCreatorButton");
    m_clearCreatorButton->setObjectName("clearCreatorButton");
    m_creatorSelectionLabel->setObjectName("creatorSelectionLabel");
    m_providersLabel->setObjectName("providersLabel");
    m_playRenderButton->setObjectName("playRenderButton");
    m_pauseRenderButton->setObjectName("pauseRenderButton");
    m_stopRenderButton->setObjectName("stopRenderButton");
    m_playSourceButton->setObjectName("playSourceButton");
    m_pauseSourceButton->setObjectName("pauseSourceButton");
    m_stopSourceButton->setObjectName("stopSourceButton");
    m_seekSourceButton->setObjectName("seekSourceButton");
    m_sourceSeekSeconds->setObjectName("sourceSeekSeconds");
    m_sourcePlaybackLabel->setObjectName("sourcePlaybackLabel");
    m_playbackPositionLabel->setObjectName("playbackPositionLabel");

    m_viewerWidget = new ViewerWidget(central);
    m_viewerWidget->setObjectName("viewerWidget");
    m_viewerWidget->setMinimumHeight(160);

    layout->addWidget(m_projectLabel);
    layout->addWidget(m_viewerWidget);
    layout->addWidget(m_yawLabel);
    layout->addWidget(m_pitchLabel);
    layout->addWidget(m_rollLabel);
    layout->addWidget(m_fieldOfViewLabel);
    layout->addWidget(m_newProjectButton);
    layout->addWidget(m_saveButton);
    layout->addWidget(m_openButton);
    layout->addWidget(m_importButton);
    layout->addWidget(m_mediaListWidget);
    layout->addWidget(m_removeMediaButton);
    layout->addWidget(m_setActiveButton);
    layout->addWidget(m_previewFrameButton);
    layout->addWidget(m_stepBackButton);
    layout->addWidget(m_stepForwardButton);
    layout->addWidget(m_markFlatButton);
    layout->addWidget(m_markEquirectButton);
    layout->addWidget(m_previewTimeLabel);
    layout->addWidget(m_activeMediaLabel);
    layout->addWidget(m_backgroundButton);
    layout->addWidget(m_resetViewportButton);
    layout->addWidget(m_commandEdit);
    layout->addWidget(m_commandStartSeconds);
    layout->addWidget(m_commandEndSeconds);
    layout->addWidget(m_runCommandButton);
    layout->addWidget(m_commandResultLabel);
    layout->addWidget(m_reviewButton);
    layout->addWidget(m_reviewSummaryLabel);
    layout->addWidget(m_acceptReviewButton);
    layout->addWidget(m_rejectReviewButton);
    layout->addWidget(m_reframeOutputsList);
    layout->addWidget(m_previewRenderButton);
    layout->addWidget(m_revisionEdit);
    layout->addWidget(m_reviseRenderButton);
    layout->addWidget(m_provenanceButton);
    layout->addWidget(m_provenanceLabel);
    layout->addWidget(m_selectCreatorButton);
    layout->addWidget(m_clearCreatorButton);
    layout->addWidget(m_creatorSelectionLabel);
    layout->addWidget(m_providersLabel);
    layout->addWidget(m_playRenderButton);
    layout->addWidget(m_pauseRenderButton);
    layout->addWidget(m_stopRenderButton);
    layout->addWidget(m_playbackPositionLabel);
    layout->addWidget(m_playSourceButton);
    layout->addWidget(m_pauseSourceButton);
    layout->addWidget(m_stopSourceButton);
    layout->addWidget(m_sourceSeekSeconds);
    layout->addWidget(m_seekSourceButton);
    layout->addWidget(m_sourcePlaybackLabel);
    layout->addWidget(m_statusLabel);

    setCentralWidget(central);

    connect(m_newProjectButton, &QPushButton::clicked, this, &MainWindow::newProjectRequested);
    connect(m_resetViewportButton, &QPushButton::clicked, this, &MainWindow::resetViewportRequested);

    connect(m_saveButton, &QPushButton::clicked, this, [this]() {
        const QString filePath = chooseSaveFilePath();
        if (!filePath.isEmpty()) {
            emit saveProjectRequested(filePath);
        }
    });

    connect(m_openButton, &QPushButton::clicked, this, [this]() {
        const QString filePath = chooseOpenFilePath();
        if (!filePath.isEmpty()) {
            emit openProjectRequested(filePath);
        }
    });

    connect(m_importButton, &QPushButton::clicked, this, [this]() {
        const QString filePath = chooseMediaFilePath();
        if (!filePath.isEmpty()) {
            emit importMediaRequested(filePath);
        }
    });

    connect(m_removeMediaButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem *current = m_mediaListWidget->currentItem();
        if (!current) {
            m_statusLabel->setText(QStringLiteral("No media selected to remove."));
            return;
        }
        emit removeMediaRequested(current->data(kMediaIdRole).toString());
    });

    connect(m_setActiveButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem *current = m_mediaListWidget->currentItem();
        if (!current) {
            m_statusLabel->setText(QStringLiteral("No media selected to set active."));
            return;
        }
        emit setActiveRequested(current->data(kMediaIdRole).toString());
    });

    connect(m_previewFrameButton, &QPushButton::clicked, this, [this]() {
        emit previewFrameRequested();
    });

    connect(m_stepBackButton, &QPushButton::clicked, this, [this]() {
        emit previewStepRequested(-1.0);
    });

    connect(m_stepForwardButton, &QPushButton::clicked, this, [this]() {
        emit previewStepRequested(1.0);
    });

    auto markProjection = [this](const QString &projectionValue) {
        QListWidgetItem *current = m_mediaListWidget->currentItem();
        if (!current) {
            m_statusLabel->setText(QStringLiteral("No media selected to mark."));
            return;
        }
        emit setMediaProjectionRequested(current->data(kMediaIdRole).toString(),
                                         projectionValue);
    };
    connect(m_markFlatButton, &QPushButton::clicked, this,
            [this, markProjection]() { markProjection(QStringLiteral("flat")); });
    connect(m_markEquirectButton, &QPushButton::clicked, this,
            [this, markProjection]() {
                markProjection(QStringLiteral("equirectangular"));
            });

    connect(m_backgroundButton, &QPushButton::clicked, this, &MainWindow::backgroundDemoRequested);

    connect(m_runCommandButton, &QPushButton::clicked, this, [this]() {
        const qint64 startMs = static_cast<qint64>(
            qRound64(m_commandStartSeconds->value() * 1000.0));
        const qint64 endMs = static_cast<qint64>(
            qRound64(m_commandEndSeconds->value() * 1000.0));
        emit reframeCommandRequested(m_commandEdit->text(), startMs, endMs);
    });

    // Objective 34. Reviewing reuses the same instruction and range inputs as
    // running: a review is the decision stage of the command the creator typed.
    connect(m_reviewButton, &QPushButton::clicked, this, [this]() {
        const qint64 startMs = static_cast<qint64>(
            qRound64(m_commandStartSeconds->value() * 1000.0));
        const qint64 endMs = static_cast<qint64>(
            qRound64(m_commandEndSeconds->value() * 1000.0));
        emit reframeReviewRequested(m_commandEdit->text(), startMs, endMs);
    });
    connect(m_acceptReviewButton, &QPushButton::clicked, this,
            &MainWindow::acceptReframeReviewRequested);
    connect(m_rejectReviewButton, &QPushButton::clicked, this,
            &MainWindow::rejectReframeReviewRequested);

    connect(m_selectCreatorButton, &QPushButton::clicked, this,
            &MainWindow::selectCreatorTargetRequested);
    connect(m_clearCreatorButton, &QPushButton::clicked, this,
            &MainWindow::clearCreatorTargetRequested);
    connect(m_previewRenderButton, &QPushButton::clicked, this, [this]() {
        const int row = m_reframeOutputsList->currentRow();
        if (row < 0) {
            m_statusLabel->setText(
                QStringLiteral("No generated render selected."));
            return;
        }
        emit previewReframeOutputRequested(row);
    });

    connect(m_playRenderButton, &QPushButton::clicked, this, [this]() {
        const int row = m_reframeOutputsList->currentRow();
        if (row < 0) {
            m_statusLabel->setText(
                QStringLiteral("No generated render selected."));
            return;
        }
        emit playReframeOutputRequested(row);
    });

    // Objective 35: both actions act on the SELECTED record, and both refuse
    // locally when there is nothing to act on, so no request is emitted blind.
    connect(m_reviseRenderButton, &QPushButton::clicked, this, [this]() {
        const int row = m_reframeOutputsList->currentRow();
        if (row < 0) {
            m_statusLabel->setText(
                QStringLiteral("No generated render selected to revise."));
            return;
        }
        const QString instruction = m_revisionEdit->text().trimmed();
        if (instruction.isEmpty()) {
            m_statusLabel->setText(QStringLiteral("Enter a revised instruction."));
            return;
        }
        emit reviseReframeOutputRequested(row, instruction);
    });
    connect(m_provenanceButton, &QPushButton::clicked, this, [this]() {
        const int row = m_reframeOutputsList->currentRow();
        if (row < 0) {
            m_statusLabel->setText(
                QStringLiteral("No generated render selected."));
            return;
        }
        emit describeDecisionRequested(row);
    });
    connect(m_pauseRenderButton, &QPushButton::clicked, this,
            &MainWindow::pauseReframeOutputPlaybackRequested);
    connect(m_stopRenderButton, &QPushButton::clicked, this,
            &MainWindow::stopReframeOutputPlaybackRequested);
    connect(m_playSourceButton, &QPushButton::clicked, this,
            &MainWindow::playSourceRequested);
    connect(m_pauseSourceButton, &QPushButton::clicked, this,
            &MainWindow::pauseSourceRequested);
    connect(m_stopSourceButton, &QPushButton::clicked, this,
            &MainWindow::stopSourceRequested);
    connect(m_seekSourceButton, &QPushButton::clicked, this, [this]() {
        emit seekSourceRequested(
            static_cast<qint64>(qRound64(m_sourceSeekSeconds->value() * 1000.0)));
    });

    m_newProjectButton->installEventFilter(this);
    m_saveButton->installEventFilter(this);
    m_openButton->installEventFilter(this);
    m_importButton->installEventFilter(this);
    m_removeMediaButton->installEventFilter(this);
    m_setActiveButton->installEventFilter(this);
    m_previewFrameButton->installEventFilter(this);
    m_stepBackButton->installEventFilter(this);
    m_stepForwardButton->installEventFilter(this);
    m_markFlatButton->installEventFilter(this);
    m_markEquirectButton->installEventFilter(this);
    m_backgroundButton->installEventFilter(this);
    m_resetViewportButton->installEventFilter(this);
}

QString MainWindow::chooseSaveFilePath()
{
    return QFileDialog::getSaveFileName(
        this, QStringLiteral("Save Project"), QString(), QStringLiteral("Reelcraft Project (*.reel)"));
}

QString MainWindow::chooseOpenFilePath()
{
    return QFileDialog::getOpenFileName(
        this, QStringLiteral("Open Project"), QString(), QStringLiteral("Reelcraft Project (*.reel)"));
}

QString MainWindow::chooseMediaFilePath()
{
    // Media content is not decoded here; any existing regular file may be
    // selected and validated by the application layer.
    return QFileDialog::getOpenFileName(
        this, QStringLiteral("Import Media"), QString(), QStringLiteral("All files (*)"));
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Q:
        case Qt::Key_E:
        case Qt::Key_Plus:
        case Qt::Key_Equal:
        case Qt::Key_Minus:
        case Qt::Key_Underscore:
            keyPressEvent(keyEvent);
            return true;
        default:
            break;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showProject(const Project &project)
{
    m_projectLabel->setText(QStringLiteral("%1 (%2)")
                                .arg(project.name(), project.id()));
    // Project context changed (new/open): clear any stale decoded frame so the
    // viewer does not keep showing the previous project's media.
    clearViewerSource();
}

void MainWindow::showStatus(const QString &message)
{
    m_statusLabel->setText(message);
}

void MainWindow::showMediaList(const QList<MediaItem> &items)
{
    m_mediaListWidget->clear();
    for (const MediaItem &item : items) {
        auto *listItem = new QListWidgetItem(m_mediaListWidget);
        listItem->setData(kMediaIdRole, item.id());
        listItem->setData(kMediaNameRole, item.fileName());
        listItem->setData(kMediaTagRole, item.formatTag());
        listItem->setData(kMediaProjectionRole,
                          MediaItem::projectionToString(item.projection()));
    }
    refreshActiveMarking();
}

void MainWindow::showActiveMedia(const QString &mediaId)
{
    if (mediaId != m_activeMediaIdText) {
        // The active media context changed (including cleared): remove any
        // stale decoded frame from the previous active media.
        clearViewerSource();
    }
    m_activeMediaIdText = mediaId;
    if (mediaId.isEmpty()) {
        m_activeMediaLabel->setText(QStringLiteral("Active media: None"));
    } else {
        QString name = QStringLiteral("Unknown");
        for (int i = 0; i < m_mediaListWidget->count(); ++i) {
            QListWidgetItem *row = m_mediaListWidget->item(i);
            if (row && row->data(kMediaIdRole).toString() == mediaId) {
                name = row->data(kMediaNameRole).toString();
                break;
            }
        }
        m_activeMediaLabel->setText(QStringLiteral("Active media: %1").arg(name));
    }
    refreshActiveMarking();
}

void MainWindow::clearViewerSource()
{
    if (!m_viewerWidget) {
        return;
    }
    m_viewerWidget->setSourceImage(QImage());
    m_viewerWidget->setFlatSourceMode(false);
}

void MainWindow::showFramePreview(const QImage &image)
{
    if (!m_viewerWidget) {
        return;
    }
    // Route presentation by the active row's declared projection (Objective
    // 12): flat => flat (no camera transform); unknown/equirectangular =>
    // equirectangular pixel path.
    bool flat = false;
    for (int i = 0; i < m_mediaListWidget->count(); ++i) {
        QListWidgetItem *row = m_mediaListWidget->item(i);
        if (row && row->data(kMediaIdRole).toString() == m_activeMediaIdText
            && row->data(kMediaProjectionRole).toString() == QStringLiteral("flat")) {
            flat = true;
            break;
        }
    }
    m_viewerWidget->setFlatSourceMode(flat);
    m_viewerWidget->setSourceImage(image);
}

void MainWindow::showReframeCommandResult(const ReframeCommandOutcome &outcome)
{
    if (outcome.ok) {
        m_commandResultLabel->setText(
            QStringLiteral("Reframe command succeeded: %1 (%2 frame(s))")
                .arg(outcome.outputPath)
                .arg(outcome.frameCount));
    } else {
        m_commandResultLabel->setText(
            QStringLiteral("Reframe command failed: %1").arg(outcome.error));
    }
}

void MainWindow::showReframeReview(const ReframePlanReview &review)
{
    if (!m_reviewSummaryLabel) {
        return;
    }
    // The accept/reject controls are only meaningful while a plan is waiting.
    const bool pending = review.isValid();
    m_acceptReviewButton->setEnabled(pending);
    m_rejectReviewButton->setEnabled(pending);
    if (!pending) {
        m_reviewSummaryLabel->setText(
            QStringLiteral("No plan is waiting for review."));
        return;
    }
    // The summary is the review's own deterministic lines; the panel adds no
    // interpretation of its own.
    m_reviewSummaryLabel->setText(
        QStringLiteral("Reviewing a plan of %1 keyframe(s) (digest %2). Nothing "
                       "has been rendered yet.\n%3")
            .arg(review.keyframeCount)
            .arg(review.planDigest.left(12),
                 review.summaryLines().join(QStringLiteral("\n"))));
}

void MainWindow::showDecisionProvenance(const DecisionProvenance &view, int index)
{
    if (!m_provenanceLabel) {
        return;
    }
    if (!view.available) {
        m_provenanceLabel->setText(
            QStringLiteral("Render %1 — no decision to explain: %2")
                .arg(index)
                .arg(view.error.isEmpty()
                         ? QStringLiteral("this record carries no edit decision.")
                         : view.error));
        return;
    }
    QStringList lines;
    lines.append(QStringLiteral("Render %1 — how this decision was formed:").arg(index));
    lines.append(QStringLiteral("  origin: %1").arg(view.origin));
    lines.append(QStringLiteral("  instruction: %1").arg(view.instruction));
    if (view.hasParent) {
        lines.append(QStringLiteral("  revises: %1 (%2)")
                         .arg(view.parentDecisionHash.left(12),
                              view.parentResolved
                                  ? QStringLiteral("held by this project")
                                  : QStringLiteral("not held by this project")));
    } else {
        lines.append(QStringLiteral("  revises: nothing (an original decision)"));
    }
    lines.append(QStringLiteral("  source: %1%2")
                     .arg(view.sourceStatus,
                          view.sourceDetail.isEmpty()
                              ? QString()
                              : QStringLiteral(" — %1").arg(view.sourceDetail)));
    lines.append(QStringLiteral("  plan: %1 keyframe(s), %2 retained segment(s), "
                                "%3..%4 ms, %5x%6 at %7 fps, %8 frame(s)")
                     .arg(view.keyframeCount)
                     .arg(view.segmentCount)
                     .arg(view.planStartMs)
                     .arg(view.planEndMs)
                     .arg(view.outputWidth)
                     .arg(view.outputHeight)
                     .arg(view.outputFps)
                     .arg(view.planFrameCount));
    m_provenanceLabel->setText(lines.join(QStringLiteral("\n")));
}

void MainWindow::showReframeOutputs(const QList<ReframeCommandOutcome> &outputs)
{
    if (!m_reframeOutputsList) {
        return;
    }
    m_reframeOutputsList->clear();
    for (const ReframeCommandOutcome &outcome : outputs) {
        const QString status =
            outcome.ok ? QStringLiteral("ok") : QStringLiteral("failed");
        QString text = QStringLiteral("[%1] %2 -> %3")
                           .arg(status, outcome.instruction, outcome.outputPath);
        if (!outcome.ok && !outcome.error.isEmpty()) {
            text += QStringLiteral(" (%1)").arg(outcome.error);
        }
        m_reframeOutputsList->addItem(text);
    }
}

void MainWindow::showPreviewTime(double seconds)
{
    m_previewTimeLabel->setText(QStringLiteral("Time: %1 s").arg(seconds, 0, 'f', 1));
}

void MainWindow::refreshActiveMarking()
{
    for (int i = 0; i < m_mediaListWidget->count(); ++i) {
        QListWidgetItem *row = m_mediaListWidget->item(i);
        if (!row) {
            continue;
        }
        const QString id = row->data(kMediaIdRole).toString();
        const bool active = !m_activeMediaIdText.isEmpty() && id == m_activeMediaIdText;
        const QString text = QStringLiteral("%1%2  [%3]")
                                 .arg(active ? kActivePrefix : QString(),
                                      row->data(kMediaNameRole).toString(),
                                      row->data(kMediaTagRole).toString());
        row->setText(text);
    }
}

void MainWindow::showYaw(double value)
{
    m_yawLabel->setText(QStringLiteral("Yaw: %1").arg(value, 0, 'f', 2));
}

void MainWindow::showPitch(double value)
{
    m_pitchLabel->setText(QStringLiteral("Pitch: %1").arg(value, 0, 'f', 2));
}

void MainWindow::showRoll(double value)
{
    m_rollLabel->setText(QStringLiteral("Roll: %1").arg(value, 0, 'f', 2));
}

void MainWindow::showFieldOfView(double value)
{
    m_fieldOfViewLabel->setText(QStringLiteral("FOV: %1").arg(value, 0, 'f', 2));
}
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    constexpr double step = 5.0;

    switch (event->key()) {
    case Qt::Key_Left:
        emit viewportYawDeltaRequested(-step);
        break;
    case Qt::Key_Right:
        emit viewportYawDeltaRequested(step);
        break;
    case Qt::Key_Up:
        emit viewportPitchDeltaRequested(step);
        break;
    case Qt::Key_Down:
        emit viewportPitchDeltaRequested(-step);
        break;
    case Qt::Key_Q:
        emit viewportRollDeltaRequested(-step);
        break;
    case Qt::Key_E:
        emit viewportRollDeltaRequested(step);
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        emit viewportFovDeltaRequested(step);
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        emit viewportFovDeltaRequested(-step);
        break;
    default:
        QMainWindow::keyPressEvent(event);
        return;
    }

    event->accept();
}

void MainWindow::showCreatorSelection(bool hasSelection, double yawDeg,
                                      double pitchDeg)
{
    if (hasSelection) {
        m_creatorSelectionLabel->setText(
            QStringLiteral("Creator target 'me': yaw %1, pitch %2")
                .arg(yawDeg, 0, 'f', 1)
                .arg(pitchDeg, 0, 'f', 1));
    } else {
        m_creatorSelectionLabel->setText(
            QStringLiteral("Creator target 'me': none"));
    }
}

void MainWindow::showReframeOutputPreview(const QImage &image)
{
    if (!m_viewerWidget) {
        return;
    }
    // A reframed render is a flat video: present it flat, letterboxed, with no
    // equirectangular camera transform.
    m_viewerWidget->setFlatSourceMode(true);
    m_viewerWidget->setSourceImage(image);
}

void MainWindow::showProviderStatus(bool hasDetector, bool hasSpeaker)
{
    m_providersLabel->setText(
        QStringLiteral("Providers: detector=%1, speaker=%2")
            .arg(hasDetector ? QStringLiteral("configured")
                             : QStringLiteral("not configured"))
            .arg(hasSpeaker ? QStringLiteral("configured")
                            : QStringLiteral("not configured")));
}

void MainWindow::showReframePlaybackState(bool playing)
{
    m_playbackPositionLabel->setText(
        playing ? QStringLiteral("Playback: playing")
                : QStringLiteral("Playback: paused/stopped"));
}

void MainWindow::showReframePlaybackPosition(qint64 frameCount,
                                             qint64 positionMs)
{
    m_playbackPositionLabel->setText(
        QStringLiteral("Playback: frame %1, %2 ms")
            .arg(frameCount)
            .arg(positionMs));
}

void MainWindow::showSourcePlaybackState(bool playing)
{
    m_sourcePlaybackLabel->setText(
        playing ? QStringLiteral("Source: playing")
                : QStringLiteral("Source: paused/stopped"));
}

void MainWindow::showSourcePlaybackPosition(qint64 positionMs)
{
    m_sourcePlaybackLabel->setText(
        QStringLiteral("Source: %1 ms").arg(positionMs));
}

#include "MainWindow.h"

#include <QFileDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
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
    m_backgroundButton = new QPushButton(QStringLiteral("Run Background Demo"), central);
    m_resetViewportButton = new QPushButton(QStringLiteral("Reset Viewport"), central);

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
    m_backgroundButton->setObjectName("backgroundDemoButton");
    m_resetViewportButton->setObjectName("resetViewportButton");

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
    layout->addWidget(m_activeMediaLabel);
    layout->addWidget(m_backgroundButton);
    layout->addWidget(m_resetViewportButton);
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

    connect(m_backgroundButton, &QPushButton::clicked, this, &MainWindow::backgroundDemoRequested);

    m_newProjectButton->installEventFilter(this);
    m_saveButton->installEventFilter(this);
    m_openButton->installEventFilter(this);
    m_importButton->installEventFilter(this);
    m_removeMediaButton->installEventFilter(this);
    m_setActiveButton->installEventFilter(this);
    m_previewFrameButton->installEventFilter(this);
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
    }
    refreshActiveMarking();
}

void MainWindow::showActiveMedia(const QString &mediaId)
{
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

void MainWindow::showFramePreview(const QImage &image)
{
    if (m_viewerWidget) {
        m_viewerWidget->setSourceImage(image);
    }
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

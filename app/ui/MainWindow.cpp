#include "MainWindow.h"

#include <QFileDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "ui/ViewerWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Reelcraft");
    resize(1200, 800);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_projectLabel = new QLabel(QStringLiteral("No project"), central);
    m_statusLabel = new QLabel(QStringLiteral("Ready"), central);

    m_yawLabel = new QLabel(QStringLiteral("Yaw: 0.0"), central);
    m_pitchLabel = new QLabel(QStringLiteral("Pitch: 0.0"), central);
    m_rollLabel = new QLabel(QStringLiteral("Roll: 0.0"), central);
    m_fieldOfViewLabel = new QLabel(QStringLiteral("FOV: 90.0"), central);

    m_newProjectButton = new QPushButton(QStringLiteral("New Project"), central);
    m_saveButton = new QPushButton(QStringLiteral("Save Project"), central);
    m_openButton = new QPushButton(QStringLiteral("Open Project"), central);
    m_backgroundButton = new QPushButton(QStringLiteral("Run Background Demo"), central);
    m_resetViewportButton = new QPushButton(QStringLiteral("Reset Viewport"), central);

    m_projectLabel->setObjectName("projectLabel");
    m_statusLabel->setObjectName("statusLabel");
    m_yawLabel->setObjectName("yawLabel");
    m_pitchLabel->setObjectName("pitchLabel");
    m_rollLabel->setObjectName("rollLabel");
    m_fieldOfViewLabel->setObjectName("fieldOfViewLabel");
    m_newProjectButton->setObjectName("newProjectButton");
    m_saveButton->setObjectName("saveProjectButton");
    m_openButton->setObjectName("openProjectButton");
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

    connect(m_backgroundButton, &QPushButton::clicked, this, &MainWindow::backgroundDemoRequested);

    m_newProjectButton->installEventFilter(this);
    m_saveButton->installEventFilter(this);
    m_openButton->installEventFilter(this);
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

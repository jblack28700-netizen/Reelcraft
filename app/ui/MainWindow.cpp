#include "MainWindow.h"

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Reelcraft");
    resize(1200, 800);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_projectLabel = new QLabel(QStringLiteral("No project"), central);
    m_statusLabel = new QLabel(QStringLiteral("Ready"), central);

    m_newProjectButton = new QPushButton(QStringLiteral("New Project"), central);
    m_saveButton = new QPushButton(QStringLiteral("Save Project"), central);
    m_openButton = new QPushButton(QStringLiteral("Open Project"), central);
    m_backgroundButton = new QPushButton(QStringLiteral("Run Background Demo"), central);

    layout->addWidget(m_projectLabel);
    layout->addWidget(m_newProjectButton);
    layout->addWidget(m_saveButton);
    layout->addWidget(m_openButton);
    layout->addWidget(m_backgroundButton);
    layout->addWidget(m_statusLabel);

    setCentralWidget(central);

    connect(m_newProjectButton, &QPushButton::clicked, this, &MainWindow::newProjectRequested);

    connect(m_saveButton, &QPushButton::clicked, this, [this]() {
        const QString filePath = QFileDialog::getSaveFileName(
            this, QStringLiteral("Save Project"), QString(), QStringLiteral("Reelcraft Project (*.reel)"));
        if (!filePath.isEmpty()) {
            emit saveProjectRequested(filePath);
        }
    });

    connect(m_openButton, &QPushButton::clicked, this, [this]() {
        const QString filePath = QFileDialog::getOpenFileName(
            this, QStringLiteral("Open Project"), QString(), QStringLiteral("Reelcraft Project (*.reel)"));
        if (!filePath.isEmpty()) {
            emit openProjectRequested(filePath);
        }
    });

    connect(m_backgroundButton, &QPushButton::clicked, this, &MainWindow::backgroundDemoRequested);
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

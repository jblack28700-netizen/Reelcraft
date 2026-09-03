#pragma once

#include <QMainWindow>

#include "core/Project.h"

class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    virtual QString chooseSaveFilePath();
    virtual QString chooseOpenFilePath();

public slots:
    void showProject(const Project &project);
    void showStatus(const QString &message);

signals:
    void newProjectRequested();
    void saveProjectRequested(const QString &filePath);
    void openProjectRequested(const QString &filePath);
    void backgroundDemoRequested();

private:
    QLabel *m_projectLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_newProjectButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_openButton = nullptr;
    QPushButton *m_backgroundButton = nullptr;
};

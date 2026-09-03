#include <QApplication>

#include "application/Application.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Application application;
    application.initialize();

    MainWindow window;
    window.show();

    QObject::connect(&window, &MainWindow::newProjectRequested,
                     &application, &Application::newProject);
    QObject::connect(&window, &MainWindow::saveProjectRequested,
                     &application, &Application::saveProject);
    QObject::connect(&window, &MainWindow::openProjectRequested,
                     &application, &Application::openProject);
    QObject::connect(&window, &MainWindow::backgroundDemoRequested,
                     &application, &Application::runBackgroundDemo);

    QObject::connect(&application, &Application::projectChanged,
                     &window, &MainWindow::showProject);
    QObject::connect(&application, &Application::backgroundCompleted,
                     &window, &MainWindow::showStatus);

    return app.exec();
}

#include <QApplication>

#include "application/Application.h"
#include "ui/MainWindow.h"
#include "ui/ViewerWidget.h"
#include "viewer/ViewportState.h"

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
    QObject::connect(&window, &MainWindow::importMediaRequested,
                     &application, &Application::importMediaFile);
    QObject::connect(&window, &MainWindow::removeMediaRequested,
                     &application, &Application::removeMedia);
    QObject::connect(&window, &MainWindow::backgroundDemoRequested,
                     &application, &Application::runBackgroundDemo);
    QObject::connect(&window, &MainWindow::resetViewportRequested,
                     &application, &Application::resetViewport);

    QObject::connect(&application, &Application::projectChanged,
                     &window, &MainWindow::showProject);
    QObject::connect(&application, &Application::backgroundCompleted,
                     &window, &MainWindow::showStatus);
    QObject::connect(&application, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);

    ViewportState *viewport = application.viewportState();
    if (viewport) {
        // The viewer presentation follows the authoritative viewport state.
        window.viewerWidget()->setViewportState(viewport);

        QObject::connect(viewport, &ViewportState::yawChanged,
                         &window, &MainWindow::showYaw);
        QObject::connect(viewport, &ViewportState::pitchChanged,
                         &window, &MainWindow::showPitch);
        QObject::connect(viewport, &ViewportState::rollChanged,
                         &window, &MainWindow::showRoll);
        QObject::connect(viewport, &ViewportState::fieldOfViewChanged,
                         &window, &MainWindow::showFieldOfView);

        QObject::connect(&window, &MainWindow::viewportYawDeltaRequested,
                         &application, &Application::adjustViewportYaw);
        QObject::connect(&window, &MainWindow::viewportPitchDeltaRequested,
                         &application, &Application::adjustViewportPitch);
        QObject::connect(&window, &MainWindow::viewportRollDeltaRequested,
                         &application, &Application::adjustViewportRoll);
        QObject::connect(&window, &MainWindow::viewportFovDeltaRequested,
                         &application, &Application::adjustViewportFieldOfView);
    }

    return app.exec();
}

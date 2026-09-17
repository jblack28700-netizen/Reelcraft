#include <QApplication>

#include <memory>

#include "application/Application.h"
#include "target/ProcessSpeakerProvider.h"
#include "target/ProcessTargetDetector.h"
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
    QObject::connect(&window, &MainWindow::setActiveRequested,
                     &application, &Application::setActiveMedia);
    QObject::connect(&window, &MainWindow::previewFrameRequested,
                     &application, &Application::previewActiveMediaFrame);
    QObject::connect(&window, &MainWindow::previewStepRequested,
                     &application, &Application::stepActiveMediaPreview);
    QObject::connect(&window, &MainWindow::setMediaProjectionRequested,
                     &application, &Application::declareMediaProjection);
    QObject::connect(&window, &MainWindow::backgroundDemoRequested,
                     &application, &Application::runBackgroundDemo);
    QObject::connect(&window, &MainWindow::resetViewportRequested,
                     &application, &Application::resetViewport);

    QObject::connect(&application, &Application::projectChanged,
                     &window, &MainWindow::showProject);
    QObject::connect(&application, &Application::backgroundCompleted,
                     &window, &MainWindow::showStatus);
    QObject::connect(&window, &MainWindow::reframeCommandRequested,
                     &application, &Application::runReframeCommand);
    QObject::connect(&application, &Application::reframeCommandFinished,
                     &window, &MainWindow::showReframeCommandResult);
    QObject::connect(&application, &Application::reframeOutputsChanged,
                     &window, &MainWindow::showReframeOutputs);
    QObject::connect(&window, &MainWindow::selectCreatorTargetRequested,
                     &application, &Application::selectCreatorTargetFromViewport);
    QObject::connect(&window, &MainWindow::clearCreatorTargetRequested,
                     &application, &Application::clearCreatorSelection);
    QObject::connect(&window, &MainWindow::previewReframeOutputRequested,
                     &application, &Application::previewReframeOutput);
    QObject::connect(&application, &Application::reframeOutputPreviewReady,
                     &window, &MainWindow::showReframeOutputPreview);
    QObject::connect(&application, &Application::creatorSelectionChanged,
                     [&window, &application](bool hasSelection) {
                         window.showCreatorSelection(
                             hasSelection,
                             application.creatorSelection().yawDeg,
                             application.creatorSelection().pitchDeg);
                     });
    QObject::connect(&window, &MainWindow::playReframeOutputRequested,
                     &application, &Application::startReframeOutputPlayback);
    QObject::connect(&window, &MainWindow::pauseReframeOutputPlaybackRequested,
                     &application, &Application::pauseReframeOutputPlayback);
    QObject::connect(&window, &MainWindow::stopReframeOutputPlaybackRequested,
                     &application, &Application::stopReframeOutputPlayback);
    QObject::connect(&application, &Application::reframePlaybackFrameReady,
                     &window, &MainWindow::showReframeOutputPreview);
    QObject::connect(&application, &Application::reframePlaybackStateChanged,
                     &window, &MainWindow::showReframePlaybackState);
    QObject::connect(&application, &Application::reframePlaybackPositionChanged,
                     &window, &MainWindow::showReframePlaybackPosition);
    QObject::connect(&application, &Application::reframePlaybackEnded,
                     &window, [&window]() {
                         window.showReframePlaybackState(false);
                         window.showStatus(
                             QStringLiteral("Render playback ended."));
                     });
    QObject::connect(&application, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&application, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);
    QObject::connect(&application, &Application::framePreviewReady,
                     &window, &MainWindow::showFramePreview);
    QObject::connect(&application, &Application::previewTimeChanged,
                     &window, &MainWindow::showPreviewTime);

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

        // Pointer-based viewer orientation control (Objective 11).
        QObject::connect(window.viewerWidget(), &ViewerWidget::viewportYawDeltaRequested,
                         &application, &Application::adjustViewportYaw);
        QObject::connect(window.viewerWidget(), &ViewerWidget::viewportPitchDeltaRequested,
                         &application, &Application::adjustViewportPitch);
        QObject::connect(window.viewerWidget(), &ViewerWidget::viewportFovDeltaRequested,
                         &application, &Application::adjustViewportFieldOfView);
    }

    // Optional external target detector for subject-referencing commands
    // ("follow person 1", "follow me"). The model/runtime stay external and
    // replaceable; without it, subject commands report a clear detector error.
    std::unique_ptr<TargetDetector> commandDetector;
    const QString detectorPython =
        qEnvironmentVariable("REELCRAFT_TARGET_DETECTOR_PY");
    const QString detectorScript =
        qEnvironmentVariable("REELCRAFT_TARGET_DETECTOR_SCRIPT");
    const QString detectorModel =
        qEnvironmentVariable("REELCRAFT_TARGET_YOLOX_MODEL");
    if (!detectorPython.isEmpty() && !detectorScript.isEmpty()
        && !detectorModel.isEmpty()) {
        commandDetector = std::make_unique<ProcessTargetDetector>(
            detectorPython,
            QStringList{ detectorScript, QStringLiteral("--model"),
                         detectorModel });
        application.setTargetDetector(commandDetector.get());
    }

    // Optional external speaker/audio provider for speaker commands ("follow
    // the speaker"). The VAD model/runtime also stay external and replaceable.
    std::unique_ptr<SpeakerEvidenceProvider> speakerProvider;
    const QString speakerPython = qEnvironmentVariable("REELCRAFT_SPEAKER_PY");
    const QString speakerScript = qEnvironmentVariable("REELCRAFT_SPEAKER_SCRIPT");
    const QString sileroModel = qEnvironmentVariable("REELCRAFT_SILERO_MODEL");
    if (!speakerPython.isEmpty() && !speakerScript.isEmpty()
        && !sileroModel.isEmpty()) {
        speakerProvider = std::make_unique<ProcessSpeakerProvider>(
            speakerPython,
            QStringList{ speakerScript, QStringLiteral("--model"), sileroModel });
        application.setSpeakerEvidenceProvider(speakerProvider.get());
    }

    window.showProviderStatus(application.targetDetector() != nullptr,
                              application.speakerEvidenceProvider() != nullptr);

    return app.exec();
}

#include <QtTest>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QColor>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtMath>

#include "application/Application.h"
#include "core/MediaItem.h"
#include "core/Project.h"
#include "ui/MainWindow.h"
#include "ui/ViewerWidget.h"
#include "viewer/ViewerProjection.h"
#include "viewer/ViewerScene.h"
#include "viewer/ViewportState.h"

class TestMainWindow : public MainWindow
{
public:
    explicit TestMainWindow(QWidget *parent = nullptr) : MainWindow(parent) {}

protected:
    QString chooseSaveFilePath() override { return QStringLiteral("/tmp/reelcraft_test.reel"); }
    QString chooseOpenFilePath() override { return QStringLiteral("/tmp/reelcraft_test.reel"); }
    QString chooseMediaFilePath() override { return QStringLiteral("/tmp/reelcraft_media_test.bin"); }
};

class ProjectTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void newProjectHasValidDefaults();
    void saveAndLoadPreservesData();
    void loadMissingFileReturnsInvalid();
    void saveToInvalidPathFails();
    void applicationNewProjectEmitsSignal();
    void applicationOpenProjectEmitsSignal();
    void applicationBackgroundDemoEmitsSignal();
    void mainWindowButtonsEmitSignals();
    void mainWindowProjectLabelUpdates();
    void mainWindowStatusLabelUpdates();
    void applicationSaveInvalidPathEmitsError();
    void applicationOpenMissingFileEmitsError();
    void projectLifecycleDoesNotModifyOriginalMedia();
    void viewportStateDefaultsAreValid();
    void viewportStateYawNormalizes();
    void viewportStatePitchClamps();
    void viewportStateRollNormalizes();
    void viewportStateFieldOfViewClamps();
    void viewportStateSettersEmitSignalsOnlyOnChange();
    void applicationViewportStateIsValid();
    void applicationResetViewportRestoresDefaults();
    void mainWindowViewerLabelsUpdate();
    void mainWindowResetViewportButtonEmitsSignal();
    void applicationViewerStateUpdatesMainWindowLabels();
    void applicationResetViewportUpdatesMainWindowLabels();
    void mainWindowKeyboardEmitsViewportDeltas();
    void applicationKeyboardUpdatesViewportState();
    void newProjectResetsViewport();
    void openProjectResetsViewport();
    void viewportStateJsonRoundTrip();
    void viewportStateRejectsInvalidJson();
    void applicationSaveAndOpenPersistsViewportState();
    void restoredViewerStateUpdatesMainWindowLabels();
    void focusedButtonKeyPressEmitsViewportDelta();
    void invalidPersistedViewerStateFallsBackToDefaults();
    void applicationAdjustViewportSlots();
    void projectDefaultsHaveSchemaVersion();
    void legacyProjectWithoutVersionLoadsSchemaOne();
    void projectSchemaVersionRoundTrip();
    void futureProjectSchemaVersionIsRejected();
    void applicationOpenFutureSchemaProjectFailsSafely();
    void viewerSceneGroundTruthIsDeterministic();
    void viewerSceneMarkerRangesAreValid();
    void viewerWidgetPresentsDeterministicScene();
    void mainWindowContainsViewerSurface();
    void viewerWidgetRenderIsDeterministic();
    void viewerProjectionCentersFrontMarkerAtIdentity();
    void viewerProjectionYawAimsAtSideMarkers();
    void viewerProjectionPitchAimsAtPoleMarkers();
    void viewerProjectionPitchMirrorSymmetry();
    void viewerProjectionRollRotatesViewContent();
    void viewerProjectionLargerFieldOfViewBringsMarkersCloser();
    void viewerWidgetCameraFollowsApplicationViewportState();
    void mediaItemRecordsMetadataFromRealFile();
    void mediaItemRejectsInvalidPaths();
    void mediaItemIdIsDeterministic();
    void mediaItemJsonRoundTrip();
    void mediaItemRejectsInvalidJson();
    void applicationImportMediaRequiresActiveProject();
    void applicationImportMediaDeduplicatesAndOrders();
    void applicationImportMediaRejectsInvalidPath();
    void mediaReferencesPersistAndReopenDeterministically();
    void originalMediaUnchangedByImportAndLifecycle();
    void legacyProjectOpensWithEmptyMedia();
    void invalidPersistedMediaFallsBackSafely();
    void mainWindowImportButtonEmitsSignal();
    void reopenWithAvailableMediaIsSilentAndIdentical();
    void reopenAfterDeletingMediaFileFlagsUnavailable();
    void reopenAfterMovingMediaFileFlagsUnavailable();
    void duplicatePersistedMediaNormalizedOnOpenAndResave();
    void newProjectClearsUnavailableMediaState();
};

void ProjectTest::initTestCase()
{
    qRegisterMetaType<Project>("Project");
}

void ProjectTest::newProjectHasValidDefaults()
{
    Project project;
    QVERIFY(project.isValid());
    QVERIFY(!project.id().isEmpty());
    QCOMPARE(project.name(), QString("Untitled Project"));
    QVERIFY(project.created().isValid());
}

void ProjectTest::saveAndLoadPreservesData()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project project;
    project.setName(QStringLiteral("Demo Project"));

    const QString filePath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(project.save(filePath));

    bool ok = false;
    Project loaded = Project::load(filePath, &ok);
    QVERIFY(ok);
    QVERIFY(loaded.isValid());
    QCOMPARE(loaded.name(), QStringLiteral("Demo Project"));
    QCOMPARE(loaded.id(), project.id());
    QCOMPARE(loaded.created(), project.created());
}

void ProjectTest::loadMissingFileReturnsInvalid()
{
    bool ok = true;
    Project loaded = Project::load(QStringLiteral("/nonexistent/nope.reel"), &ok);
    QVERIFY(!ok);
    QVERIFY(!loaded.isValid());
}

void ProjectTest::saveToInvalidPathFails()
{
    Project project;
    QString error;
    QVERIFY(!project.save(QStringLiteral("/nonexistent_dir/nope.reel"), &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::applicationNewProjectEmitsSignal()
{
    Application app;
    QSignalSpy spy(&app, &Application::projectChanged);

    app.newProject();

    QCOMPARE(spy.count(), 1);
    QVERIFY(app.hasProject());
    QVERIFY(app.currentProject().isValid());
}

void ProjectTest::applicationOpenProjectEmitsSignal()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project original;
    original.setName(QStringLiteral("Reopen Me"));
    const QString filePath = tempDir.filePath(QStringLiteral("reopen.reel"));
    QVERIFY(original.save(filePath));

    Application app;
    QSignalSpy spy(&app, &Application::projectChanged);

    QVERIFY(app.openProject(filePath));
    QCOMPARE(spy.count(), 1);
    QVERIFY(app.hasProject());
    QCOMPARE(app.currentProject().name(), QStringLiteral("Reopen Me"));
    QCOMPARE(app.currentProject().id(), original.id());
}

void ProjectTest::applicationBackgroundDemoEmitsSignal()
{
    Application app;
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    app.runBackgroundDemo();

    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 1, 2000);
    QVERIFY(!app.hasProject());
}

void ProjectTest::mainWindowButtonsEmitSignals()
{
    TestMainWindow window;
    QSignalSpy newSpy(&window, &MainWindow::newProjectRequested);
    QSignalSpy saveSpy(&window, &MainWindow::saveProjectRequested);
    QSignalSpy openSpy(&window, &MainWindow::openProjectRequested);
    QSignalSpy bgSpy(&window, &MainWindow::backgroundDemoRequested);

    auto *newButton = window.findChild<QPushButton*>("newProjectButton");
    auto *saveButton = window.findChild<QPushButton*>("saveProjectButton");
    auto *openButton = window.findChild<QPushButton*>("openProjectButton");
    auto *bgButton = window.findChild<QPushButton*>("backgroundDemoButton");

    QVERIFY(newButton);
    QVERIFY(saveButton);
    QVERIFY(openButton);
    QVERIFY(bgButton);

    newButton->click();
    saveButton->click();
    openButton->click();
    bgButton->click();

    QCOMPARE(newSpy.count(), 1);
    QCOMPARE(saveSpy.count(), 1);
    QCOMPARE(openSpy.count(), 1);
    QCOMPARE(bgSpy.count(), 1);
}

void ProjectTest::mainWindowProjectLabelUpdates()
{
    TestMainWindow window;
    Project project;
    project.setName(QStringLiteral("Label Test"));

    window.showProject(project);

    auto *label = window.findChild<QLabel*>("projectLabel");
    QVERIFY(label);
    QVERIFY(label->text().contains(project.name()));
    QVERIFY(label->text().contains(project.id()));
}

void ProjectTest::mainWindowStatusLabelUpdates()
{
    TestMainWindow window;
    window.showStatus(QStringLiteral("Test Status"));

    auto *label = window.findChild<QLabel*>("statusLabel");
    QVERIFY(label);
    QCOMPARE(label->text(), QStringLiteral("Test Status"));
}

void ProjectTest::applicationSaveInvalidPathEmitsError()
{
    Application app;
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    app.newProject();
    spy.clear();

    QVERIFY(!app.saveProject(QStringLiteral("/nonexistent_dir/nope.reel")));
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Save failed")));
    QVERIFY(app.hasProject());
}

void ProjectTest::applicationOpenMissingFileEmitsError()
{
    Application app;
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.openProject(QStringLiteral("/nonexistent/nope.reel")));
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Open failed")));
    QVERIFY(!app.hasProject());
}


void ProjectTest::projectLifecycleDoesNotModifyOriginalMedia()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("source.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));

    const QByteArray originalBytes("FAKE_MEDIA_BYTES_0123456789_REELCRAFT_PHASE1");
    QCOMPARE(mediaFile.write(originalBytes), static_cast<qint64>(originalBytes.size()));
    mediaFile.close();

    const QByteArray originalHash =
        QCryptographicHash::hash(originalBytes, QCryptographicHash::Sha256);

    Application app;
    app.newProject();

    Project project = app.currentProject();
    project.setName(QStringLiteral("Media Safety"));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(project.save(projectPath));
    QVERIFY(app.openProject(projectPath));

    QFile reopened(mediaPath);
    QVERIFY(reopened.open(QIODevice::ReadOnly));
    const QByteArray afterBytes = reopened.readAll();
    reopened.close();

    QCOMPARE(afterBytes, originalBytes);
    QCOMPARE(QCryptographicHash::hash(afterBytes, QCryptographicHash::Sha256), originalHash);
}


void ProjectTest::viewportStateDefaultsAreValid()
{
    ViewportState state;

    QCOMPARE(state.yaw(), 0.0);
    QCOMPARE(state.pitch(), 0.0);
    QCOMPARE(state.roll(), 0.0);
    QCOMPARE(state.fieldOfView(), 90.0);
}

void ProjectTest::viewportStateYawNormalizes()
{
    ViewportState state;

    state.setYaw(190.0);
    QVERIFY(qAbs(state.yaw() - (-170.0)) < 0.000001);

    state.setYaw(-190.0);
    QVERIFY(qAbs(state.yaw() - 170.0) < 0.000001);
}

void ProjectTest::viewportStatePitchClamps()
{
    ViewportState state;

    state.setPitch(95.0);
    QCOMPARE(state.pitch(), 90.0);

    state.setPitch(-95.0);
    QCOMPARE(state.pitch(), -90.0);
}

void ProjectTest::viewportStateRollNormalizes()
{
    ViewportState state;

    state.setRoll(200.0);
    QVERIFY(qAbs(state.roll() - (-160.0)) < 0.000001);
}

void ProjectTest::viewportStateFieldOfViewClamps()
{
    ViewportState state;

    state.setFieldOfView(10.0);
    QCOMPARE(state.fieldOfView(), 20.0);

    state.setFieldOfView(170.0);
    QCOMPARE(state.fieldOfView(), 140.0);
}

void ProjectTest::viewportStateSettersEmitSignalsOnlyOnChange()
{
    ViewportState state;

    QSignalSpy yawSpy(&state, &ViewportState::yawChanged);
    QSignalSpy pitchSpy(&state, &ViewportState::pitchChanged);
    QSignalSpy rollSpy(&state, &ViewportState::rollChanged);
    QSignalSpy fovSpy(&state, &ViewportState::fieldOfViewChanged);

    state.setYaw(0.0);
    state.setPitch(0.0);
    state.setRoll(0.0);
    state.setFieldOfView(90.0);

    QCOMPARE(yawSpy.count(), 0);
    QCOMPARE(pitchSpy.count(), 0);
    QCOMPARE(rollSpy.count(), 0);
    QCOMPARE(fovSpy.count(), 0);

    state.setYaw(45.0);
    state.setPitch(-30.0);
    state.setRoll(10.0);
    state.setFieldOfView(75.0);

    QCOMPARE(yawSpy.count(), 1);
    QCOMPARE(pitchSpy.count(), 1);
    QCOMPARE(rollSpy.count(), 1);
    QCOMPARE(fovSpy.count(), 1);
}

void ProjectTest::applicationViewportStateIsValid()
{
    Application app;

    QVERIFY(app.viewportState() != nullptr);
    QCOMPARE(app.viewportState()->yaw(), 0.0);
    QCOMPARE(app.viewportState()->pitch(), 0.0);
    QCOMPARE(app.viewportState()->roll(), 0.0);
    QCOMPARE(app.viewportState()->fieldOfView(), 90.0);
}

void ProjectTest::applicationResetViewportRestoresDefaults()
{
    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state != nullptr);

    state->setYaw(30.0);
    state->setPitch(-45.0);
    state->setRoll(15.0);
    state->setFieldOfView(120.0);

    app.resetViewport();

    QCOMPARE(state->yaw(), 0.0);
    QCOMPARE(state->pitch(), 0.0);
    QCOMPARE(state->roll(), 0.0);
    QCOMPARE(state->fieldOfView(), 90.0);
}

void ProjectTest::mainWindowViewerLabelsUpdate()
{
    TestMainWindow window;

    window.showYaw(30.5);
    window.showPitch(-15.25);
    window.showRoll(45.0);
    window.showFieldOfView(110.0);

    auto *yaw = window.findChild<QLabel*>("yawLabel");
    auto *pitch = window.findChild<QLabel*>("pitchLabel");
    auto *roll = window.findChild<QLabel*>("rollLabel");
    auto *fov = window.findChild<QLabel*>("fieldOfViewLabel");

    QVERIFY(yaw);
    QVERIFY(pitch);
    QVERIFY(roll);
    QVERIFY(fov);

    QCOMPARE(yaw->text(), QStringLiteral("Yaw: 30.50"));
    QCOMPARE(pitch->text(), QStringLiteral("Pitch: -15.25"));
    QCOMPARE(roll->text(), QStringLiteral("Roll: 45.00"));
    QCOMPARE(fov->text(), QStringLiteral("FOV: 110.00"));
}

void ProjectTest::mainWindowResetViewportButtonEmitsSignal()
{
    TestMainWindow window;
    QSignalSpy spy(&window, &MainWindow::resetViewportRequested);

    auto *button = window.findChild<QPushButton*>("resetViewportButton");
    QVERIFY(button);

    button->click();
    QCOMPARE(spy.count(), 1);
}

void ProjectTest::applicationViewerStateUpdatesMainWindowLabels()
{
    Application app;
    TestMainWindow window;

    ViewportState *state = app.viewportState();
    QVERIFY(state);

    QObject::connect(state, &ViewportState::yawChanged, &window, &MainWindow::showYaw);
    QObject::connect(state, &ViewportState::pitchChanged, &window, &MainWindow::showPitch);
    QObject::connect(state, &ViewportState::rollChanged, &window, &MainWindow::showRoll);
    QObject::connect(state, &ViewportState::fieldOfViewChanged, &window, &MainWindow::showFieldOfView);

    state->setYaw(30.5);
    state->setPitch(-15.25);
    state->setRoll(45.0);
    state->setFieldOfView(110.0);

    auto *yaw = window.findChild<QLabel*>("yawLabel");
    auto *pitch = window.findChild<QLabel*>("pitchLabel");
    auto *roll = window.findChild<QLabel*>("rollLabel");
    auto *fov = window.findChild<QLabel*>("fieldOfViewLabel");

    QVERIFY(yaw);
    QVERIFY(pitch);
    QVERIFY(roll);
    QVERIFY(fov);

    QCOMPARE(yaw->text(), QStringLiteral("Yaw: 30.50"));
    QCOMPARE(pitch->text(), QStringLiteral("Pitch: -15.25"));
    QCOMPARE(roll->text(), QStringLiteral("Roll: 45.00"));
    QCOMPARE(fov->text(), QStringLiteral("FOV: 110.00"));
}

void ProjectTest::applicationResetViewportUpdatesMainWindowLabels()
{
    Application app;
    TestMainWindow window;

    ViewportState *state = app.viewportState();
    QVERIFY(state);

    QObject::connect(state, &ViewportState::yawChanged, &window, &MainWindow::showYaw);
    QObject::connect(state, &ViewportState::pitchChanged, &window, &MainWindow::showPitch);
    QObject::connect(state, &ViewportState::rollChanged, &window, &MainWindow::showRoll);
    QObject::connect(state, &ViewportState::fieldOfViewChanged, &window, &MainWindow::showFieldOfView);
    QObject::connect(&window, &MainWindow::resetViewportRequested, &app, &Application::resetViewport);

    state->setYaw(30.0);
    state->setPitch(-45.0);
    state->setRoll(15.0);
    state->setFieldOfView(120.0);

    auto *button = window.findChild<QPushButton*>("resetViewportButton");
    QVERIFY(button);
    button->click();

    auto *yaw = window.findChild<QLabel*>("yawLabel");
    auto *pitch = window.findChild<QLabel*>("pitchLabel");
    auto *roll = window.findChild<QLabel*>("rollLabel");
    auto *fov = window.findChild<QLabel*>("fieldOfViewLabel");

    QVERIFY(yaw);
    QVERIFY(pitch);
    QVERIFY(roll);
    QVERIFY(fov);

    QCOMPARE(yaw->text(), QStringLiteral("Yaw: 0.00"));
    QCOMPARE(pitch->text(), QStringLiteral("Pitch: 0.00"));
    QCOMPARE(roll->text(), QStringLiteral("Roll: 0.00"));
    QCOMPARE(fov->text(), QStringLiteral("FOV: 90.00"));
}

void ProjectTest::mainWindowKeyboardEmitsViewportDeltas()
{
    TestMainWindow window;
    QSignalSpy yawSpy(&window, &MainWindow::viewportYawDeltaRequested);
    QSignalSpy pitchSpy(&window, &MainWindow::viewportPitchDeltaRequested);
    QSignalSpy rollSpy(&window, &MainWindow::viewportRollDeltaRequested);
    QSignalSpy fovSpy(&window, &MainWindow::viewportFovDeltaRequested);

    QKeyEvent left(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent q(QEvent::KeyPress, Qt::Key_Q, Qt::NoModifier);
    QKeyEvent minus(QEvent::KeyPress, Qt::Key_Minus, Qt::NoModifier);

    QApplication::sendEvent(&window, &left);
    QApplication::sendEvent(&window, &up);
    QApplication::sendEvent(&window, &q);
    QApplication::sendEvent(&window, &minus);

    QCOMPARE(yawSpy.count(), 1);
    QCOMPARE(pitchSpy.count(), 1);
    QCOMPARE(rollSpy.count(), 1);
    QCOMPARE(fovSpy.count(), 1);
}

void ProjectTest::applicationKeyboardUpdatesViewportState()
{
    Application app;
    TestMainWindow window;

    ViewportState *state = app.viewportState();
    QVERIFY(state);

    QObject::connect(&window, &MainWindow::viewportYawDeltaRequested,
                     [state](double delta) { state->setYaw(state->yaw() + delta); });
    QObject::connect(&window, &MainWindow::viewportPitchDeltaRequested,
                     [state](double delta) { state->setPitch(state->pitch() + delta); });
    QObject::connect(&window, &MainWindow::viewportRollDeltaRequested,
                     [state](double delta) { state->setRoll(state->roll() + delta); });
    QObject::connect(&window, &MainWindow::viewportFovDeltaRequested,
                     [state](double delta) { state->setFieldOfView(state->fieldOfView() + delta); });

    QKeyEvent right(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent e(QEvent::KeyPress, Qt::Key_E, Qt::NoModifier);
    QKeyEvent plus(QEvent::KeyPress, Qt::Key_Plus, Qt::NoModifier);

    QApplication::sendEvent(&window, &right);
    QApplication::sendEvent(&window, &down);
    QApplication::sendEvent(&window, &e);
    QApplication::sendEvent(&window, &plus);

    QCOMPARE(state->yaw(), 5.0);
    QCOMPARE(state->pitch(), -5.0);
    QCOMPARE(state->roll(), 5.0);
    QCOMPARE(state->fieldOfView(), 95.0);
}

void ProjectTest::newProjectResetsViewport()
{
    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    state->setYaw(30.0);
    state->setPitch(-45.0);
    state->setRoll(15.0);
    state->setFieldOfView(120.0);

    app.newProject();

    QCOMPARE(state->yaw(), 0.0);
    QCOMPARE(state->pitch(), 0.0);
    QCOMPARE(state->roll(), 0.0);
    QCOMPARE(state->fieldOfView(), 90.0);
}

void ProjectTest::openProjectResetsViewport()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project project;
    project.setName(QStringLiteral("Open Reset"));
    const QString filePath = tempDir.filePath(QStringLiteral("open_reset.reel"));
    QVERIFY(project.save(filePath));

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    state->setYaw(25.0);
    state->setPitch(-40.0);
    state->setRoll(10.0);
    state->setFieldOfView(115.0);

    QVERIFY(app.openProject(filePath));

    QCOMPARE(state->yaw(), 0.0);
    QCOMPARE(state->pitch(), 0.0);
    QCOMPARE(state->roll(), 0.0);
    QCOMPARE(state->fieldOfView(), 90.0);
}

void ProjectTest::viewportStateJsonRoundTrip()
{
    ViewportState original;
    original.setYaw(30.0);
    original.setPitch(-15.0);
    original.setRoll(10.0);
    original.setFieldOfView(75.0);

    const QJsonObject object = original.toJsonObject();

    ViewportState restored;
    QString error;
    const bool ok = restored.readFromJsonObject(object, &error);

    QVERIFY(ok);
    QVERIFY(error.isEmpty());
    QCOMPARE(restored.yaw(), original.yaw());
    QCOMPARE(restored.pitch(), original.pitch());
    QCOMPARE(restored.roll(), original.roll());
    QCOMPARE(restored.fieldOfView(), original.fieldOfView());
}

void ProjectTest::viewportStateRejectsInvalidJson()
{
    ViewportState state;
    QString error;

    QJsonObject empty;
    QVERIFY(!state.readFromJsonObject(empty, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::applicationSaveAndOpenPersistsViewportState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    app.newProject();
    state->setYaw(33.0);
    state->setPitch(-12.0);
    state->setRoll(7.0);
    state->setFieldOfView(100.0);

    const QString filePath = tempDir.filePath(QStringLiteral("viewer_persist.reel"));
    QVERIFY(app.saveProject(filePath));

    Application reopened;
    ViewportState *restored = reopened.viewportState();
    QVERIFY(restored);

    QVERIFY(reopened.openProject(filePath));

    QCOMPARE(restored->yaw(), 33.0);
    QCOMPARE(restored->pitch(), -12.0);
    QCOMPARE(restored->roll(), 7.0);
    QCOMPARE(restored->fieldOfView(), 100.0);
}

void ProjectTest::restoredViewerStateUpdatesMainWindowLabels()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    app.newProject();
    state->setYaw(42.0);
    state->setPitch(-18.0);
    state->setRoll(9.0);
    state->setFieldOfView(105.0);

    const QString filePath = tempDir.filePath(QStringLiteral("viewer_ui_restore.reel"));
    QVERIFY(app.saveProject(filePath));

    Application reopened;
    ViewportState *restored = reopened.viewportState();
    QVERIFY(restored);

    TestMainWindow window;

    QObject::connect(restored, &ViewportState::yawChanged, &window, &MainWindow::showYaw);
    QObject::connect(restored, &ViewportState::pitchChanged, &window, &MainWindow::showPitch);
    QObject::connect(restored, &ViewportState::rollChanged, &window, &MainWindow::showRoll);
    QObject::connect(restored, &ViewportState::fieldOfViewChanged, &window, &MainWindow::showFieldOfView);

    QVERIFY(reopened.openProject(filePath));

    auto *yaw = window.findChild<QLabel*>("yawLabel");
    auto *pitch = window.findChild<QLabel*>("pitchLabel");
    auto *roll = window.findChild<QLabel*>("rollLabel");
    auto *fov = window.findChild<QLabel*>("fieldOfViewLabel");

    QVERIFY(yaw);
    QVERIFY(pitch);
    QVERIFY(roll);
    QVERIFY(fov);

    QCOMPARE(yaw->text(), QStringLiteral("Yaw: 42.00"));
    QCOMPARE(pitch->text(), QStringLiteral("Pitch: -18.00"));
    QCOMPARE(roll->text(), QStringLiteral("Roll: 9.00"));
    QCOMPARE(fov->text(), QStringLiteral("FOV: 105.00"));
}

void ProjectTest::focusedButtonKeyPressEmitsViewportDelta()
{
    TestMainWindow window;
    QSignalSpy yawSpy(&window, &MainWindow::viewportYawDeltaRequested);

    auto *button = window.findChild<QPushButton*>("newProjectButton");
    QVERIFY(button);

    QKeyEvent right(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(button, &right);

    QCOMPARE(yawSpy.count(), 1);
}

void ProjectTest::invalidPersistedViewerStateFallsBackToDefaults()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project project;
    project.setName(QStringLiteral("Invalid Viewer State"));

    QJsonObject invalidViewerState;
    invalidViewerState.insert(QStringLiteral("yaw"), QStringLiteral("not-a-number"));
    project.setViewerState(invalidViewerState);

    const QString filePath = tempDir.filePath(QStringLiteral("invalid_viewer.reel"));
    QVERIFY(project.save(filePath));

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    state->setYaw(12.0);
    state->setPitch(-8.0);
    state->setRoll(3.0);
    state->setFieldOfView(100.0);

    QVERIFY(app.openProject(filePath));

    QCOMPARE(state->yaw(), 0.0);
    QCOMPARE(state->pitch(), 0.0);
    QCOMPARE(state->roll(), 0.0);
    QCOMPARE(state->fieldOfView(), 90.0);
}

void ProjectTest::applicationAdjustViewportSlots()
{
    Application app;

    app.adjustViewportYaw(10.0);
    app.adjustViewportPitch(-5.0);
    app.adjustViewportRoll(7.0);
    app.adjustViewportFieldOfView(-10.0);

    QCOMPARE(app.viewportState()->yaw(), 10.0);
    QCOMPARE(app.viewportState()->pitch(), -5.0);
    QCOMPARE(app.viewportState()->roll(), 7.0);
    QCOMPARE(app.viewportState()->fieldOfView(), 80.0);
}

void ProjectTest::projectDefaultsHaveSchemaVersion()
{
    Project project;
    QCOMPARE(project.schemaVersion(), Project::CurrentSchemaVersion);
}

void ProjectTest::legacyProjectWithoutVersionLoadsSchemaOne()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject legacy;
    legacy.insert(QStringLiteral("id"), QStringLiteral("legacy-id"));
    legacy.insert(QStringLiteral("name"), QStringLiteral("Legacy Project"));
    legacy.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    const QString filePath = tempDir.filePath(QStringLiteral("legacy.reel"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(legacy).toJson(QJsonDocument::Compact));
    file.close();

    bool ok = false;
    Project loaded = Project::load(filePath, &ok);

    QVERIFY(ok);
    QCOMPARE(loaded.schemaVersion(), 1);
}

void ProjectTest::projectSchemaVersionRoundTrip()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Project project;
    project.setName(QStringLiteral("Schema Round Trip"));
    QCOMPARE(project.schemaVersion(), Project::CurrentSchemaVersion);

    const QString filePath = tempDir.filePath(QStringLiteral("schema_roundtrip.reel"));
    QVERIFY(project.save(filePath));

    bool ok = false;
    Project loaded = Project::load(filePath, &ok);

    QVERIFY(ok);
    QCOMPARE(loaded.schemaVersion(), Project::CurrentSchemaVersion);
}

void ProjectTest::futureProjectSchemaVersionIsRejected()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject future;
    future.insert(QStringLiteral("id"), QStringLiteral("future-id"));
    future.insert(QStringLiteral("name"), QStringLiteral("Future Project"));
    future.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    future.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion + 1);

    const QString filePath = tempDir.filePath(QStringLiteral("future_schema.reel"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(future).toJson(QJsonDocument::Compact));
    file.close();

    bool ok = true;
    QString error;
    Project loaded = Project::load(filePath, &ok, &error);

    QVERIFY(!ok);
    QVERIFY(!loaded.isValid());
    QVERIFY(error.contains(QStringLiteral("newer unsupported schema version")));
}

void ProjectTest::applicationOpenFutureSchemaProjectFailsSafely()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QJsonObject future;
    future.insert(QStringLiteral("id"), QStringLiteral("future-app-id"));
    future.insert(QStringLiteral("name"), QStringLiteral("Future App Project"));
    future.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    future.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion + 1);

    const QString filePath = tempDir.filePath(QStringLiteral("future_app_schema.reel"));
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(future).toJson(QJsonDocument::Compact));
    file.close();

    Application app;
    app.newProject();
    const QString originalName = app.currentProject().name();

    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.openProject(filePath));
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Open failed")));
    QVERIFY(app.hasProject());
    QCOMPARE(app.currentProject().name(), originalName);
}

void ProjectTest::viewerSceneGroundTruthIsDeterministic()
{
    const ViewerScene first = ViewerScene::createDeterministicTestScene();
    const ViewerScene second = ViewerScene::createDeterministicTestScene();

    QVERIFY(first.isValid());
    QCOMPARE(first.markerCount(), 10);
    QCOMPARE(first.markers().size(), second.markers().size());
    QCOMPARE(second.markers().size(), 10);

    for (int i = 0; i < first.markers().size(); ++i) {
        QCOMPARE(first.markers().at(i).label, second.markers().at(i).label);
        QVERIFY(qAbs(first.markers().at(i).yaw - second.markers().at(i).yaw) < 1e-9);
        QVERIFY(qAbs(first.markers().at(i).pitch - second.markers().at(i).pitch) < 1e-9);
    }

    const int front = first.markerIndex(QStringLiteral("FRONT"));
    QVERIFY(front >= 0);
    QVERIFY(qAbs(first.markers().at(front).yaw) < 1e-9);
    QVERIFY(qAbs(first.markers().at(front).pitch) < 1e-9);

    const int back = first.markerIndex(QStringLiteral("BACK"));
    QVERIFY(back >= 0);
    QVERIFY(qAbs(first.markers().at(back).yaw - (-180.0)) < 1e-9);
    QVERIFY(qAbs(first.markers().at(back).pitch) < 1e-9);

    const int up = first.markerIndex(QStringLiteral("UP"));
    QVERIFY(up >= 0);
    QVERIFY(qAbs(first.markers().at(up).pitch - 90.0) < 1e-9);

    const int rightUp = first.markerIndex(QStringLiteral("RIGHT_UP"));
    QVERIFY(rightUp >= 0);
    QVERIFY(qAbs(first.markers().at(rightUp).yaw - 90.0) < 1e-9);
    QVERIFY(qAbs(first.markers().at(rightUp).pitch - 45.0) < 1e-9);
}

void ProjectTest::viewerSceneMarkerRangesAreValid()
{
    const ViewerScene scene = ViewerScene::createDeterministicTestScene();
    QVERIFY(scene.isValid());

    QStringList seenLabels;
    for (const ViewerSceneMarker &marker : scene.markers()) {
        QVERIFY(!marker.label.isEmpty());
        QVERIFY(marker.yaw >= -180.0 && marker.yaw < 180.0);
        QVERIFY(marker.pitch >= -90.0 && marker.pitch <= 90.0);
        QVERIFY(!seenLabels.contains(marker.label));
        seenLabels.append(marker.label);
    }
}

void ProjectTest::viewerWidgetPresentsDeterministicScene()
{
    ViewerWidget widget;

    QVERIFY(widget.scene().isValid());
    QCOMPARE(widget.scene().markerCount(), 10);
    QVERIFY(widget.scene().markerIndex(QStringLiteral("FRONT")) >= 0);
    QVERIFY(widget.scene().markerIndex(QStringLiteral("BACK")) >= 0);
    QVERIFY(widget.scene().markerIndex(QStringLiteral("LEFT_DOWN")) >= 0);
}

void ProjectTest::mainWindowContainsViewerSurface()
{
    TestMainWindow window;

    auto *viewer = window.findChild<ViewerWidget *>(QStringLiteral("viewerWidget"));
    QVERIFY(viewer);
    QVERIFY(viewer->scene().isValid());
    QCOMPARE(viewer->scene().markerCount(), 10);
}

void ProjectTest::viewerWidgetRenderIsDeterministic()
{
    ViewerWidget widget;
    widget.resize(400, 200);

    const QImage image = widget.grab().toImage();
    QVERIFY(!image.isNull());
    QVERIFY(image.width() > 0);
    QVERIFY(image.height() > 0);

    const double scaleX = static_cast<double>(image.width()) / 400.0;
    const double scaleY = static_cast<double>(image.height()) / 200.0;

    auto expectPixel = [scaleX, scaleY](const QImage &img, double logicalX,
                                        double logicalY, int red, int green, int blue) {
        const QColor color =
            img.pixelColor(qRound(logicalX * scaleX), qRound(logicalY * scaleY));
        QCOMPARE(color.red(), red);
        QCOMPARE(color.green(), green);
        QCOMPARE(color.blue(), blue);
    };

    // Without a viewport state the camera is at identity defaults
    // (yaw/pitch/roll 0, FOV 90), so FRONT is centered.

    // Background samples away from markers, labels, and the center reticle.
    expectPixel(image, 20.0, 180.0, 18, 20, 24);
    expectPixel(image, 380.0, 30.0, 18, 20, 24);

    // FRONT marker (yaw 0, pitch 0) maps to the canvas center.
    expectPixel(image, 200.0, 100.0, 255, 213, 79);

    // Side markers are not visible at the identity camera view, so their
    // former equirectangular positions are plain background now.
    expectPixel(image, 100.0, 100.0, 18, 20, 24);
    expectPixel(image, 300.0, 100.0, 18, 20, 24);
}

void ProjectTest::viewerProjectionCentersFrontMarkerAtIdentity()
{
    double x = 0.0;
    double y = 0.0;
    const bool visible = ViewerProjection::project(
        0.0, 0.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y);

    QVERIFY(visible);
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);
}

void ProjectTest::viewerProjectionYawAimsAtSideMarkers()
{
    // At identity, side and back markers are not in front of the camera.
    double x = 0.0;
    double y = 0.0;
    QVERIFY(!ViewerProjection::project(90.0, 0.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(!ViewerProjection::project(-90.0, 0.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(!ViewerProjection::project(180.0, 0.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));

    // Turning the camera +90 degrees of yaw centers the RIGHT marker.
    QVERIFY(ViewerProjection::project(90.0, 0.0, 90.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);

    // Turning the camera -90 degrees of yaw centers the LEFT marker.
    QVERIFY(ViewerProjection::project(-90.0, 0.0, -90.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);
}

void ProjectTest::viewerProjectionPitchAimsAtPoleMarkers()
{
    double x = 0.0;
    double y = 0.0;

    // At identity the poles are overhead/underfoot, not in front.
    QVERIFY(!ViewerProjection::project(0.0, 90.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(!ViewerProjection::project(0.0, -90.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x, &y));

    // Tilting up +90 centers the UP marker; tilting down -90 centers DOWN.
    QVERIFY(ViewerProjection::project(0.0, 90.0, 0.0, 90.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);

    QVERIFY(ViewerProjection::project(0.0, -90.0, 0.0, -90.0, 0.0, 90.0, 400.0, 200.0, &x, &y));
    QVERIFY(qAbs(x - 200.0) < 1e-6);
    QVERIFY(qAbs(y - 100.0) < 1e-6);
}

void ProjectTest::viewerProjectionPitchMirrorSymmetry()
{
    double xAbove = 0.0;
    double yAbove = 0.0;
    double xBelow = 0.0;
    double yBelow = 0.0;

    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &xAbove, &yAbove));
    QVERIFY(ViewerProjection::project(0.0, -20.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &xBelow, &yBelow));

    QVERIFY(qAbs(xAbove - 200.0) < 1e-6);
    QVERIFY(qAbs(xBelow - 200.0) < 1e-6);
    QVERIFY(yAbove < 100.0);
    QVERIFY(yBelow > 100.0);
    QVERIFY(qAbs((yAbove + yBelow) / 2.0 - 100.0) < 1e-6);
    QVERIFY(qAbs((100.0 - yAbove) - (yBelow - 100.0)) < 1e-6);
}

void ProjectTest::viewerProjectionRollRotatesViewContent()
{
    // FRONT stays centered for any roll.
    for (double roll : { -90.0, -45.0, 0.0, 45.0, 90.0 }) {
        double x = 0.0;
        double y = 0.0;
        QVERIFY(ViewerProjection::project(0.0, 0.0, 0.0, 0.0, roll, 90.0, 400.0, 200.0, &x, &y));
        QVERIFY(qAbs(x - 200.0) < 1e-6);
        QVERIFY(qAbs(y - 100.0) < 1e-6);
    }

    // A marker above center rotates counter-clockwise by the roll angle:
    // roll +90 moves it to the left of center at the same height.
    double x0 = 0.0;
    double y0 = 0.0;
    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 0.0, 90.0, 400.0, 200.0, &x0, &y0));
    const double radius = 100.0 - y0;
    QVERIFY(radius > 0.0);

    double xLeft = 0.0;
    double yLeft = 0.0;
    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 90.0, 90.0, 400.0, 200.0, &xLeft, &yLeft));
    QVERIFY(qAbs(xLeft - (200.0 - radius)) < 1e-6);
    QVERIFY(qAbs(yLeft - 100.0) < 1e-6);

    double xRight = 0.0;
    double yRight = 0.0;
    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, -90.0, 90.0, 400.0, 200.0, &xRight, &yRight));
    QVERIFY(qAbs(xRight - (200.0 + radius)) < 1e-6);
    QVERIFY(qAbs(yRight - 100.0) < 1e-6);
}

void ProjectTest::viewerProjectionLargerFieldOfViewBringsMarkersCloser()
{
    double xNarrow = 0.0;
    double yNarrow = 0.0;
    double xWide = 0.0;
    double yWide = 0.0;

    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 0.0, 60.0, 400.0, 200.0, &xNarrow, &yNarrow));
    QVERIFY(ViewerProjection::project(0.0, 20.0, 0.0, 0.0, 0.0, 120.0, 400.0, 200.0, &xWide, &yWide));

    const double distanceNarrow = qAbs(yNarrow - 100.0);
    const double distanceWide = qAbs(yWide - 100.0);
    QVERIFY(distanceNarrow > distanceWide);
    QVERIFY(distanceWide > 0.0);
}

void ProjectTest::viewerWidgetCameraFollowsApplicationViewportState()
{
    Application app;
    TestMainWindow window;

    ViewportState *state = app.viewportState();
    QVERIFY(state);
    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);

    // Same wiring as main.cpp: the presentation follows the authoritative
    // application-owned viewport state.
    viewer->setViewportState(state);
    viewer->resize(400, 200);

    auto centerColor = [viewer]() {
        const QImage image = viewer->grab().toImage();
        return image.pixelColor(image.width() / 2, image.height() / 2);
    };
    auto expectCenter = [&centerColor](int red, int green, int blue) {
        const QColor color = centerColor();
        QCOMPARE(color.red(), red);
        QCOMPARE(color.green(), green);
        QCOMPARE(color.blue(), blue);
    };

    // Identity defaults: FRONT (amber) centered.
    expectCenter(255, 213, 79);

    // Application yaw adjustment +90 -> camera faces RIGHT (pink) marker.
    app.adjustViewportYaw(90.0);
    expectCenter(240, 98, 146);

    // Reset restores FRONT to center.
    app.resetViewport();
    expectCenter(255, 213, 79);
}

void ProjectTest::mediaItemRecordsMetadataFromRealFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("clip_x5.360.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    const QByteArray bytes(4096, 'A');
    QCOMPARE(mediaFile.write(bytes), static_cast<qint64>(bytes.size()));
    mediaFile.close();

    QString error;
    const MediaItem item = MediaItem::createFromFilePath(mediaPath, &error);
    QVERIFY(item.isValid());
    QVERIFY(error.isEmpty());

    const QString canonical = QFileInfo(mediaPath).canonicalFilePath();
    QVERIFY(!canonical.isEmpty());
    QCOMPARE(item.path(), canonical);
    QCOMPARE(item.fileName(), QStringLiteral("clip_x5.360.mp4"));
    QCOMPARE(item.formatTag(), QStringLiteral("mp4"));
    QCOMPARE(item.sizeBytes(), static_cast<qint64>(bytes.size()));
    QVERIFY(item.lastModifiedUtc().isValid());
    QVERIFY(!item.id().isEmpty());
    QVERIFY(item.referenceExists());
}

void ProjectTest::mediaItemRejectsInvalidPaths()
{
    QString error;

    // Empty path.
    QVERIFY(!MediaItem::createFromFilePath(QString(), &error).isValid());
    QVERIFY(!error.isEmpty());

    // Missing file.
    const MediaItem missing =
        MediaItem::createFromFilePath(QStringLiteral("/nonexistent/nope.mp4"), &error);
    QVERIFY(!missing.isValid());
    QVERIFY(error.contains(QStringLiteral("does not exist")));

    // Directory.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const MediaItem directory = MediaItem::createFromFilePath(tempDir.path(), &error);
    QVERIFY(!directory.isValid());
    QVERIFY(error.contains(QStringLiteral("directory")));
}

void ProjectTest::mediaItemIdIsDeterministic()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.bin"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.bin"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("MEDIA");
        file.close();
    }

    const MediaItem first = MediaItem::createFromFilePath(firstPath);
    const MediaItem firstAgain = MediaItem::createFromFilePath(firstPath);
    const MediaItem second = MediaItem::createFromFilePath(secondPath);

    QVERIFY(first.isValid());
    QVERIFY(second.isValid());
    QCOMPARE(first.id(), firstAgain.id());
    QVERIFY(first.id() != second.id());
}

void ProjectTest::mediaItemJsonRoundTrip()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("roundtrip.mov"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(128, 'B'));
    mediaFile.close();

    MediaItem original = MediaItem::createFromFilePath(mediaPath);
    QVERIFY(original.isValid());
    QJsonObject attributes;
    attributes.insert(QStringLiteral("note"), QStringLiteral("future metadata home"));
    original.setAttributes(attributes);

    const QJsonObject object = original.toJsonObject();

    MediaItem restored;
    QString error;
    QVERIFY(restored.readFromJsonObject(object, &error));
    QVERIFY(error.isEmpty());
    QVERIFY(restored.isValid());

    QCOMPARE(restored.id(), original.id());
    QCOMPARE(restored.path(), original.path());
    QCOMPARE(restored.fileName(), original.fileName());
    QCOMPARE(restored.formatTag(), original.formatTag());
    QCOMPARE(restored.sizeBytes(), original.sizeBytes());
    QCOMPARE(restored.lastModifiedUtc().toMSecsSinceEpoch(),
             original.lastModifiedUtc().toMSecsSinceEpoch());
    QCOMPARE(restored.attributes().value(QStringLiteral("note")).toString(),
             QStringLiteral("future metadata home"));
}

void ProjectTest::mediaItemRejectsInvalidJson()
{
    MediaItem item;
    QString error;

    QJsonObject empty;
    QVERIFY(!item.readFromJsonObject(empty, &error));
    QVERIFY(!error.isEmpty());

    QJsonObject partial;
    partial.insert(QStringLiteral("id"), QStringLiteral("abc"));
    QVERIFY(!item.readFromJsonObject(partial, &error));
    QVERIFY(!error.isEmpty());
}

void ProjectTest::applicationImportMediaRequiresActiveProject()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write("DATA");
    mediaFile.close();

    Application app;
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.importMediaFile(mediaPath));
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("No project")));

    app.newProject();
    spy.clear();
    QVERIFY(app.importMediaFile(mediaPath));
    QCOMPARE(app.mediaItems().size(), 1);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Imported media")));
}

void ProjectTest::applicationImportMediaDeduplicatesAndOrders()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.mp4"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(16, 'x'));
        file.close();
    }

    Application app;
    app.newProject();
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    // Re-importing the same canonical path is idempotent.
    QVERIFY(app.importMediaFile(firstPath));
    QCOMPARE(app.mediaItems().size(), 2);
    QVERIFY(spy.last().first().toString().contains(QStringLiteral("already imported")));

    // Import order is preserved deterministically.
    QCOMPARE(app.mediaItems().at(0).fileName(), QStringLiteral("first.mp4"));
    QCOMPARE(app.mediaItems().at(1).fileName(), QStringLiteral("second.mov"));
}

void ProjectTest::applicationImportMediaRejectsInvalidPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    Application app;
    app.newProject();
    QSignalSpy spy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.importMediaFile(QStringLiteral("/nonexistent/nope.mp4")));
    QCOMPARE(app.mediaItems().size(), 0);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Import failed")));

    spy.clear();
    QVERIFY(!app.importMediaFile(tempDir.path()));
    QCOMPARE(app.mediaItems().size(), 0);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("Import failed")));
}

void ProjectTest::mediaReferencesPersistAndReopenDeterministically()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.mp4"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(path.endsWith(QStringLiteral("mp4")) ? QByteArray(32, '1') : QByteArray(48, '2'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("media_project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QVERIFY(reopened.openProject(projectPath));

    const QList<MediaItem> items = reopened.mediaItems();
    QCOMPARE(items.size(), 2);

    const QList<MediaItem> originalItems = app.mediaItems();
    QCOMPARE(items.at(0).id(), originalItems.at(0).id());
    QCOMPARE(items.at(0).fileName(), QStringLiteral("first.mp4"));
    QCOMPARE(items.at(0).sizeBytes(), originalItems.at(0).sizeBytes());
    QCOMPARE(items.at(0).formatTag(), QStringLiteral("mp4"));
    QCOMPARE(items.at(1).id(), originalItems.at(1).id());
    QCOMPARE(items.at(1).fileName(), QStringLiteral("second.mov"));
    QCOMPARE(items.at(1).formatTag(), QStringLiteral("mov"));
}

void ProjectTest::originalMediaUnchangedByImportAndLifecycle()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("source.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    const QByteArray originalBytes("REAL_MEDIA_BYTES_0123456789_REELCRAFT_OBJ4");
    QCOMPARE(mediaFile.write(originalBytes), static_cast<qint64>(originalBytes.size()));
    mediaFile.close();

    const QByteArray originalHash =
        QCryptographicHash::hash(originalBytes, QCryptographicHash::Sha256);

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 1);

    QFile after(mediaPath);
    QVERIFY(after.open(QIODevice::ReadOnly));
    const QByteArray afterBytes = after.readAll();
    after.close();

    QCOMPARE(afterBytes, originalBytes);
    QCOMPARE(QCryptographicHash::hash(afterBytes, QCryptographicHash::Sha256), originalHash);
}

void ProjectTest::legacyProjectOpensWithEmptyMedia()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // A project file without any media section (legacy style).
    QJsonObject legacy;
    legacy.insert(QStringLiteral("id"), QStringLiteral("legacy-media-id"));
    legacy.insert(QStringLiteral("name"), QStringLiteral("Legacy No Media"));
    legacy.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    const QString projectPath = tempDir.filePath(QStringLiteral("legacy.reel"));
    QFile file(projectPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QJsonDocument(legacy).toJson(QJsonDocument::Compact));
    file.close();

    Application app;
    QVERIFY(app.openProject(projectPath));
    QVERIFY(app.mediaItems().isEmpty());
    QVERIFY(app.currentProject().media().isEmpty());
}

void ProjectTest::invalidPersistedMediaFallsBackSafely()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    auto writeProject = [&tempDir](const QString &name, const QJsonValue &mediaValue) {
        QJsonObject project;
        project.insert(QStringLiteral("id"), QStringLiteral("id-") + name);
        project.insert(QStringLiteral("name"), QStringLiteral("Project ") + name);
        project.insert(QStringLiteral("created"),
                       QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        project.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion);
        project.insert(QStringLiteral("media"), mediaValue);

        const QString projectPath = tempDir.filePath(name + QStringLiteral(".reel"));
        QFile file(projectPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return QString();
        }
        file.write(QJsonDocument(project).toJson(QJsonDocument::Compact));
        file.close();
        return projectPath;
    };

    // media present but not an array.
    const QString wrongTypePath = writeProject(QStringLiteral("wrongtype"), QJsonValue(QStringLiteral("nope")));
    QVERIFY(!wrongTypePath.isEmpty());
    Application appWrongType;
    QVERIFY(appWrongType.openProject(wrongTypePath));
    QVERIFY(appWrongType.mediaItems().isEmpty());

    // media array containing an invalid record.
    QJsonArray badArray;
    QJsonObject badRecord;
    badRecord.insert(QStringLiteral("id"), QStringLiteral("incomplete"));
    badArray.append(badRecord);
    const QString badRecordPath = writeProject(QStringLiteral("badrecord"), badArray);
    QVERIFY(!badRecordPath.isEmpty());
    Application appBadRecord;
    QVERIFY(appBadRecord.openProject(badRecordPath));
    QVERIFY(appBadRecord.mediaItems().isEmpty());
}

void ProjectTest::mainWindowImportButtonEmitsSignal()
{
    TestMainWindow window;
    QSignalSpy spy(&window, &MainWindow::importMediaRequested);

    auto *button = window.findChild<QPushButton *>(QStringLiteral("importMediaButton"));
    QVERIFY(button);

    button->click();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QStringLiteral("/tmp/reelcraft_media_test.bin"));
}

void ProjectTest::reopenWithAvailableMediaIsSilentAndIdentical()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("available.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(64, 'v'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QSignalSpy spy(&reopened, &Application::backgroundCompleted);
    QVERIFY(reopened.openProject(projectPath));

    QCOMPARE(reopened.mediaItems().size(), 1);
    QCOMPARE(reopened.mediaItems().at(0).id(), app.mediaItems().at(0).id());
    QVERIFY(!reopened.hasUnavailableMedia());
    QCOMPARE(reopened.unavailableMediaCount(), 0);
    QCOMPARE(spy.count(), 0);
}

void ProjectTest::reopenAfterDeletingMediaFileFlagsUnavailable()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("delete_me.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(32, 'd'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    // The referenced media disappears before reopen.
    QVERIFY(QFile::remove(mediaPath));

    Application reopened;
    QSignalSpy spy(&reopened, &Application::backgroundCompleted);
    QVERIFY(reopened.openProject(projectPath));

    // The record is retained but flagged unavailable.
    QCOMPARE(reopened.mediaItems().size(), 1);
    QCOMPARE(reopened.mediaItems().at(0).fileName(), QStringLiteral("delete_me.mp4"));
    QVERIFY(reopened.hasUnavailableMedia());
    QCOMPARE(reopened.unavailableMediaCount(), 1);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("unavailable")));
}

void ProjectTest::reopenAfterMovingMediaFileFlagsUnavailable()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString originalPath = tempDir.filePath(QStringLiteral("moved.mp4"));
    QFile mediaFile(originalPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(24, 'm'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(originalPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    // The file moves to a new path before reopen; the recorded reference
    // (path-based) no longer resolves.
    const QString movedPath = tempDir.filePath(QStringLiteral("moved_away.mp4"));
    QVERIFY(QFile::rename(originalPath, movedPath));

    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 1);
    QVERIFY(reopened.hasUnavailableMedia());
    QCOMPARE(reopened.unavailableMediaCount(), 1);
}

void ProjectTest::duplicatePersistedMediaNormalizedOnOpenAndResave()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("dup.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'p'));
    mediaFile.close();

    // A project file that lists the same media record twice.
    const MediaItem item = MediaItem::createFromFilePath(mediaPath);
    QVERIFY(item.isValid());
    QJsonArray duplicatedMedia;
    duplicatedMedia.append(item.toJsonObject());
    duplicatedMedia.append(item.toJsonObject());

    QJsonObject project;
    project.insert(QStringLiteral("id"), QStringLiteral("dup-project"));
    project.insert(QStringLiteral("name"), QStringLiteral("Duplicate Media"));
    project.insert(QStringLiteral("created"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    project.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion);
    project.insert(QStringLiteral("media"), duplicatedMedia);

    const QString projectPath = tempDir.filePath(QStringLiteral("dups.reel"));
    QFile projectFile(projectPath);
    QVERIFY(projectFile.open(QIODevice::WriteOnly | QIODevice::Text));
    projectFile.write(QJsonDocument(project).toJson(QJsonDocument::Compact));
    projectFile.close();

    Application app;
    QVERIFY(app.openProject(projectPath));
    QCOMPARE(app.mediaItems().size(), 1);

    // Re-saving normalizes the persisted section deterministically.
    const QString resavedPath = tempDir.filePath(QStringLiteral("dups_resaved.reel"));
    QVERIFY(app.saveProject(resavedPath));

    Application reopened;
    QVERIFY(reopened.openProject(resavedPath));
    QCOMPARE(reopened.mediaItems().size(), 1);
    QCOMPARE(reopened.mediaItems().at(0).id(), item.id());
    QVERIFY(!reopened.hasUnavailableMedia());
}

void ProjectTest::newProjectClearsUnavailableMediaState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("gone.mp4"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(8, 'g'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));
    QVERIFY(QFile::remove(mediaPath));

    Application appOpen;
    QVERIFY(appOpen.openProject(projectPath));
    QVERIFY(appOpen.hasUnavailableMedia());

    appOpen.newProject();
    QVERIFY(!appOpen.hasUnavailableMedia());
    QCOMPARE(appOpen.unavailableMediaCount(), 0);
    QVERIFY(appOpen.mediaItems().isEmpty());
}

QTEST_MAIN(ProjectTest)
#include "test_project.moc"

#include <QtTest>
#include <QFile>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "application/Application.h"
#include "core/Project.h"
#include "ui/MainWindow.h"
#include "viewer/ViewportState.h"

class TestMainWindow : public MainWindow
{
public:
    explicit TestMainWindow(QWidget *parent = nullptr) : MainWindow(parent) {}

protected:
    QString chooseSaveFilePath() override { return QStringLiteral("/tmp/reelcraft_test.reel"); }
    QString chooseOpenFilePath() override { return QStringLiteral("/tmp/reelcraft_test.reel"); }
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
QTEST_MAIN(ProjectTest)
#include "test_project.moc"

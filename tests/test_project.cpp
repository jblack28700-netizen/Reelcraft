#include <QtTest>
#include <QFile>
#include <QCryptographicHash>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "application/Application.h"
#include "core/Project.h"
#include "ui/MainWindow.h"

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

QTEST_MAIN(ProjectTest)
#include "test_project.moc"

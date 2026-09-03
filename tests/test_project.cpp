#include <QtTest>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "application/Application.h"
#include "core/Project.h"

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

QTEST_APPLESS_MAIN(ProjectTest)
#include "test_project.moc"

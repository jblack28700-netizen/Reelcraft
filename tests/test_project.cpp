#include <QtTest>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QColor>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QKeyEvent>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtMath>
#include <QWheelEvent>
#include <cmath>
#include <cstring>

#include "application/Application.h"
#include "core/MediaItem.h"
#include "core/Project.h"
#include "media/FfmpegFrameSource.h"
#include "media/FrameExtractor.h"
#include "media/FramePump.h"
#include "media/FrameSource.h"
#include "ui/MainWindow.h"
#include "ui/ViewerWidget.h"
#include "viewer/EquirectView.h"
#include "viewer/ViewerProjection.h"
#include "viewer/ViewerScene.h"
#include "viewer/ViewportState.h"

namespace {

// Deterministic synthetic equirectangular test pattern.
// Region colors are distinct so camera-view behavior is verifiable by color.
const QColor kPatternBackground(10, 10, 12);
const QColor kFrontColor(255, 213, 79);
const QColor kRightColor(240, 98, 146);
const QColor kLeftColor(129, 199, 132);
const QColor kUpColor(186, 104, 200);
const QColor kDownColor(255, 138, 101);
const QColor kUp20Color(0, 200, 200);
const QColor kUp30Color(255, 255, 255);

struct Region
{
    double yawDeg = 0.0;
    double pitchDeg = 0.0;
    double halfYawDeg = 0.0;
    double halfPitchDeg = 0.0;
    QColor color;
};

QColor patternColorAt(double yawDeg, double pitchDeg)
{
    // Pole bands (whole top/bottom rows).
    if (pitchDeg >= 86.0) {
        return kUpColor;
    }
    if (pitchDeg <= -86.0) {
        return kDownColor;
    }

    const Region regions[] = {
        { 0.0, 0.0, 6.0, 6.0, kFrontColor },
        { 90.0, 0.0, 6.0, 6.0, kRightColor },
        { -90.0, 0.0, 6.0, 6.0, kLeftColor },
        { 0.0, 20.0, 6.0, 4.0, kUp20Color },
        { 0.0, 30.0, 6.0, 4.0, kUp30Color },
    };
    for (const Region &region : regions) {
        if (qAbs(yawDeg - region.yawDeg) <= region.halfYawDeg
            && qAbs(pitchDeg - region.pitchDeg) <= region.halfPitchDeg) {
            return region.color;
        }
    }
    return kPatternBackground;
}

QImage buildTestPattern(int width = 720, int height = 360)
{
    QImage pattern(width, height, QImage::Format_ARGB32);
    for (int sy = 0; sy < height; ++sy) {
        const double pitch = 90.0 - (sy + 0.5) * 180.0 / height;
        for (int sx = 0; sx < width; ++sx) {
            const double yaw = (sx + 0.5) * 360.0 / width - 180.0;
            pattern.setPixel(sx, sy, patternColorAt(yaw, pitch).rgb());
        }
    }
    return pattern;
}

bool imagesIdentical(const QImage &a, const QImage &b)
{
    if (a.size() != b.size() || a.format() != b.format()) {
        return false;
    }
    if (a.isNull() || b.isNull()) {
        return a.isNull() && b.isNull();
    }
    return std::memcmp(a.constBits(), b.constBits(), a.sizeInBytes()) == 0;
}

// Counts pixels of a color in the four screen quadrants around the center.
struct QuadrantCounts
{
    int above = 0;
    int below = 0;
    int left = 0;
    int right = 0;
};

QuadrantCounts countQuadrants(const QImage &image, const QColor &color)
{
    QuadrantCounts counts;
    const int centerX = image.width() / 2;
    const int centerY = image.height() / 2;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y) != color) {
                continue;
            }
            if (y < centerY) {
                ++counts.above;
            } else if (y > centerY) {
                ++counts.below;
            }
            if (x < centerX) {
                ++counts.left;
            } else if (x > centerX) {
                ++counts.right;
            }
        }
    }
    return counts;
}

// Per-second solid-color frames (red, green, blue, magenta) encoded to a
// keyframe-every-frame H.264 clip at 1 fps so seeks are accurate. Lossy codec
// conversion is tolerated via channel-dominance assertions.
bool createSteppedVideo(const QString &directory, const QString &ffmpegPath,
                        QString *outVideoPath)
{
    const QColor frameColors[] = {
        QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 0, 255)
    };
    for (int i = 0; i < 4; ++i) {
        QImage frame(64, 32, QImage::Format_RGB32);
        frame.fill(frameColors[i]);
        if (!frame.save(directory + QStringLiteral("/p%1.png").arg(i), "PNG")) {
            return false;
        }
    }

    const QString videoPath = directory + QStringLiteral("/stepped.mp4");
    QProcess process;
    process.start(ffmpegPath, {
        QStringLiteral("-y"),
        QStringLiteral("-framerate"), QStringLiteral("1"),
        QStringLiteral("-i"), directory + QStringLiteral("/p%d.png"),
        QStringLiteral("-c:v"), QStringLiteral("libx264"),
        QStringLiteral("-pix_fmt"), QStringLiteral("yuv420p"),
        QStringLiteral("-g"), QStringLiteral("1"),
        QStringLiteral("-r"), QStringLiteral("1"),
        videoPath
    });
    const bool started = process.waitForStarted(10000);
    if (started) {
        process.waitForFinished(30000);
    }
    for (int i = 0; i < 4; ++i) {
        QFile::remove(directory + QStringLiteral("/p%1.png").arg(i));
    }
    if (!started || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return false;
    }
    *outVideoPath = videoPath;
    return true;
}

bool redDominant(const QColor &color)
{
    return color.red() > color.green() + 80 && color.red() > color.blue() + 80;
}

bool blueDominant(const QColor &color)
{
    return color.blue() > color.red() + 80 && color.blue() > color.green() + 80;
}

// Objective 15: deterministic equirectangular long-clip review fixture.
const QColor kReviewFrameColors[] = {
    QColor(230, 0, 0),   // red
    QColor(0, 230, 0),   // green
    QColor(0, 0, 230),   // blue
};
const QColor kReviewRightColor(230, 0, 230); // magenta: unique vs the cycle

bool reviewRedDominant(const QColor &color)
{
    return color.red() > 140 && color.green() < 110 && color.blue() < 110;
}
bool reviewGreenDominant(const QColor &color)
{
    return color.green() > 140 && color.red() < 110 && color.blue() < 110;
}
bool reviewBlueDominant(const QColor &color)
{
    return color.blue() > 140 && color.red() < 110 && color.green() < 110;
}
bool reviewMagentaDominant(const QColor &color)
{
    return color.red() > 140 && color.blue() > 140 && color.green() < 110;
}

QImage buildReviewFrame(int width, int height, int frameIndex)
{
    const QColor frontColor = kReviewFrameColors[frameIndex % 3];
    QImage frame(width, height, QImage::Format_ARGB32);
    frame.fill(QColor(10, 10, 12));
    for (int sy = 0; sy < height; ++sy) {
        const double pitch = 90.0 - (sy + 0.5) * 180.0 / height;
        for (int sx = 0; sx < width; ++sx) {
            const double yaw = (sx + 0.5) * 360.0 / width - 180.0;
            QColor color;
            if (qAbs(yaw) <= 10.0 && qAbs(pitch) <= 8.0) {
                color = frontColor;
            } else if (yaw >= 80.0 && yaw <= 100.0 && qAbs(pitch) <= 30.0) {
                color = kReviewRightColor;
            } else {
                continue;
            }
            frame.setPixel(sx, sy, color.rgb());
        }
    }
    return frame;
}

bool createEquirectReviewVideo(const QString &directory, const QString &ffmpegPath,
                               int frameCount, QString *outVideoPath)
{
    if (frameCount <= 0) {
        return false;
    }
    for (int i = 0; i < frameCount; ++i) {
        const QString name = QStringLiteral("/r_%1.png")
                                 .arg(i, 2, 10, QLatin1Char('0'));
        if (!buildReviewFrame(360, 180, i).save(directory + name, "PNG")) {
            return false;
        }
    }
    const QString videoPath = directory + QStringLiteral("/review_clip.mp4");
    QProcess process;
    process.start(ffmpegPath, {
        QStringLiteral("-y"),
        QStringLiteral("-framerate"), QStringLiteral("1"),
        QStringLiteral("-i"), directory + QStringLiteral("/r_%02d.png"),
        QStringLiteral("-c:v"), QStringLiteral("libx264"),
        QStringLiteral("-pix_fmt"), QStringLiteral("yuv420p"),
        QStringLiteral("-g"), QStringLiteral("1"),
        QStringLiteral("-r"), QStringLiteral("1"),
        videoPath
    });
    const bool started = process.waitForStarted(15000);
    if (started) {
        process.waitForFinished(60000);
    }
    for (int i = 0; i < frameCount; ++i) {
        QFile::remove(directory + QStringLiteral("/r_%1.png")
                                  .arg(i, 2, 10, QLatin1Char('0')));
    }
    if (!started || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return false;
    }
    *outVideoPath = videoPath;
    return true;
}

// --- Phase 3 Objective 2: TEMPORARY feasibility probe helpers (tests only) ---
// These deliberately do NOT introduce production infrastructure. They prove that
// a persistent FFmpeg subprocess can stream frames, hit EOF, and be killed/
// restarted cleanly in this environment. Transport details (rawvideo, pacing,
// buffering) are recorded as measurements only, NOT decisions.

bool createStreamProbeVideo(const QString &directory, const QString &ffmpegPath,
                            int frameCount, int framesPerSecond, QString *outVideoPath)
{
    const int width = 160;
    const int height = 80;
    if (frameCount <= 0 || framesPerSecond <= 0) {
        return false;
    }
    for (int i = 0; i < frameCount; ++i) {
        const QString name = QStringLiteral("/s_%1.png")
                                 .arg(i, 3, 10, QLatin1Char('0'));
        if (!buildReviewFrame(width, height, i).save(directory + name, "PNG")) {
            return false;
        }
    }
    const QString videoPath = directory + QStringLiteral("/stream_probe.mp4");
    QProcess process;
    process.start(ffmpegPath, {
        QStringLiteral("-y"),
        QStringLiteral("-framerate"), QString::number(framesPerSecond),
        QStringLiteral("-i"), directory + QStringLiteral("/s_%03d.png"),
        QStringLiteral("-c:v"), QStringLiteral("libx264"),
        QStringLiteral("-pix_fmt"), QStringLiteral("yuv420p"),
        QStringLiteral("-g"), QStringLiteral("1"),
        QStringLiteral("-r"), QString::number(framesPerSecond),
        videoPath
    });
    const bool started = process.waitForStarted(15000);
    if (started) {
        process.waitForFinished(60000);
    }
    for (int i = 0; i < frameCount; ++i) {
        QFile::remove(directory + QStringLiteral("/s_%1.png")
                                  .arg(i, 3, 10, QLatin1Char('0')));
    }
    if (!started || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return false;
    }
    *outVideoPath = videoPath;
    return true;
}

// Reads exactly one raw rgb24 frame (w*h*3 bytes) from the process stdout with a
// bounded total wait. Returns false on timeout/EOF without a full frame.
bool readRawFrame(QProcess &process, int width, int height, QByteArray &pending,
                  QImage *outImage, int timeoutMs)
{
    const qint64 need = static_cast<qint64>(width) * height * 3;
    QElapsedTimer timer;
    timer.start();
    while (pending.size() < need && timer.elapsed() < timeoutMs) {
        if (process.waitForReadyRead(250)) {
            pending += process.readAll();
        } else if (process.state() == QProcess::NotRunning) {
            break;
        }
    }
    if (pending.size() < need) {
        return false;
    }
    const QImage image(reinterpret_cast<const uchar *>(pending.constData()),
                       width, height, width * 3, QImage::Format_RGB888);
    *outImage = image.copy();
    pending.remove(0, need);
    return true;
}

bool classifyFrameColor(const QImage &frame, int expectedIndex)
{
    const QColor center = frame.pixelColor(frame.width() / 2, frame.height() / 2);
    switch (expectedIndex % 3) {
    case 0:
        return reviewRedDominant(center);
    case 1:
        return reviewGreenDominant(center);
    default:
        return reviewBlueDominant(center);
    }
}

// --- Phase 3 Objective 3: replaceable media-source test double (tests only) ---
// A deterministic in-memory FrameSource implementation. It proves the seam is
// genuinely replaceable: FramePump is driven exactly through the abstract
// contract with no ffmpeg subprocess, media file, or timing dependency, and
// every ReadResult branch can be produced on demand.
class FakeFrameSource : public FrameSource
{
public:
    enum class Mode { Frames, Error, Timeout };

    void appendFrame(const QImage &frame) { m_frames.append(frame); }
    void setMode(Mode mode) { m_mode = mode; }
    void setErrorText(const QString &text) { m_error = text; }
    int closeCount() const { return m_closeCount; }

    bool readNextFrame(int, ReadResult *result, QImage *outFrame) override
    {
        if (m_mode == Mode::Error) {
            if (result) {
                *result = ReadResult::Error;
            }
            return false;
        }
        if (m_mode == Mode::Timeout) {
            if (result) {
                *result = ReadResult::Timeout;
            }
            return false;
        }
        if (m_index >= m_frames.size()) {
            if (result) {
                *result = ReadResult::EndOfStream;
            }
            return false;
        }
        if (outFrame) {
            *outFrame = m_frames.at(m_index);
        }
        ++m_index;
        if (result) {
            *result = ReadResult::Ok;
        }
        return true;
    }
    void close() override
    {
        ++m_closeCount;
        m_open = false;
    }
    bool isOpen() const override { return m_open; }
    QString errorString() const override { return m_error; }

private:
    QList<QImage> m_frames;
    int m_index = 0;
    int m_closeCount = 0;
    bool m_open = true;
    Mode m_mode = Mode::Frames;
    QString m_error;
};

} // namespace

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
    void mediaImportEmitsMediaListChanged();
    void openProjectRestoresAndEmitsMediaListChanged();
    void newProjectEmitsEmptyMediaListChanged();
    void removeMediaRemovesMatchingRecordOnlyAndPersists();
    void removeMediaUnknownIdFailsDeterministically();
    void removeMediaRequiresActiveProject();
    void removeUnavailableMediaClearsUnavailableState();
    void mainWindowMediaListPopulatedAndRemoveWorks();
    void activeMediaRequiresProjectAndSetsState();
    void activeMediaSameIdAndUnknownIdBehavior();
    void activeMediaImportNeverAutoSelects();
    void activeMediaRemovalRules();
    void activeMediaClearedOnNewProject();
    void activeMediaRoundTripRestoresOnReopen();
    void activeMediaOpenClearsDanglingOrLegacy();
    void activeMediaUnavailableCannotBeActive();
    void mainWindowSetActiveAndLabelWork();
    void equirectViewIdentityCentersFront();
    void equirectViewYawCentersRightAndLeft();
    void equirectViewPitchReachesUpAndDown();
    void equirectViewPositiveRollRotatesContentCorrectly();
    void equirectViewFieldOfViewChangesCoverage();
    void equirectViewRejectsInvalidInput();
    void equirectViewDeterministicRepeatability();
    void viewerWidgetSourceImageRendersThroughCamera();
    void viewerWidgetClearingSourceRestoresSceneRendering();
    void equirectViewPerformanceSanity();
    void frameExtractorRejectsInvalidInput();
    void frameExtractorAvailabilityAndSingleFrameDecode();
    void frameExtractorDeterministicRepeatability();
    void applicationPreviewRequiresProjectAndActive();
    void applicationPreviewEmitsFramePreview();
    void mainWindowPreviewButtonEmitsSignal();
    void previewEndToEndShowsActiveFrameInViewer();
    void frameExtractorRejectsInvalidSeekTime();
    void frameExtractorExtractsFrameAtSeekTime();
    void frameExtractorSeekDeterministicRepeatability();
    void applicationTimeNavigationGuards();
    void applicationPreviewAtUpdatesPositionAndEmits();
    void applicationStepAdvancesAndClampsBelowZero();
    void applicationSeekBeyondEndFailsDeterministically();
    void applicationTimeResetsOnProjectActiveAndRemoval();
    void mainWindowStepButtonsAndTimeLabel();
    void viewerWidgetDragEmitsYawAndPitchDeltas();
    void viewerWidgetIgnoresNonLeftDragAndReleaseWithoutMove();
    void viewerWidgetWheelUpDecreasesFovWheelDownIncreases();
    void viewerWidgetDragUpdatesApplicationViewport();
    void mediaItemProjectionDefaultsUnknown();
    void mediaItemProjectionJsonRoundTrip();
    void applicationDeclareProjectionGuardsValidateAndEmit();
    void applicationDeclaredProjectionPersistsOnReopen();
    void viewerWidgetFlatModeLetterboxesAndCenters();
    void viewerWidgetFlatModeIgnoresCameraTransforms();
    void mainWindowProjectionButtonsEmitRequests();
    void previewRoutingHonorsDeclaredProjection();
    void viewerClearedOnNewProjectAfterPreview();
    void viewerClearedWhenActiveMediaChangesOrRemoved();
    void viewerSourcePersistsWhileContextStable();
    void equirectViewBilinearBlendsFourNeighbors();
    void equirectViewBilinearRobustAtSeamAndPoles();
    void equirectReviewFixtureFramesAreDistinctAndSeekable();
    void reviewPathEndToEndOnEquirectClip();
    void equirectReviewPerformanceInformational();
    void persistentStreamProbeDeliversFramesInOrder();
    void persistentStreamProbeErrorOnMissingFile();
    void persistentStreamProbeKillAndRestartLifecycle();
    void framePumpDeliversFramesAndEndFromReplaceableSource();
    void framePumpHandlesErrorTimeoutAndMissingSource();
    void fmpegFrameSourceStreamsFramesInOrder();
    void fmpegFrameSourceValidatesAndRestartsCleanly();
    void framePumpStreamsFfmpegSourceToEnd();
};

void ProjectTest::initTestCase()
{
    qRegisterMetaType<Project>("Project");
    qRegisterMetaType<MediaItem>("MediaItem");
    qRegisterMetaType<QList<MediaItem>>("QList<MediaItem>");
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

void ProjectTest::mediaImportEmitsMediaListChanged()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    auto createFile = [&tempDir](const QString &name) {
        const QString path = tempDir.filePath(name);
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(QByteArray(16, 'i'));
        file.close();
        return path;
    };

    Application app;
    app.newProject();
    QSignalSpy spy(&app, &Application::mediaListChanged);

    const QString firstPath = createFile(QStringLiteral("first.bin"));
    QVERIFY(app.importMediaFile(firstPath));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<QList<MediaItem>>().size(), 1);
    QCOMPARE(spy.first().first().value<QList<MediaItem>>().at(0).id(),
             app.mediaItems().at(0).id());

    // Duplicate import does not change the list, so no emission.
    QVERIFY(app.importMediaFile(firstPath));
    QCOMPARE(spy.count(), 1);

    // A second distinct import emits again with both records.
    const QString secondPath = createFile(QStringLiteral("second.mov"));
    QVERIFY(app.importMediaFile(secondPath));
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.last().first().value<QList<MediaItem>>().size(), 2);
}

void ProjectTest::openProjectRestoresAndEmitsMediaListChanged()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.mp4"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(32, 'o'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QSignalSpy spy(&reopened, &Application::mediaListChanged);
    QVERIFY(reopened.openProject(projectPath));

    QCOMPARE(spy.count(), 1);
    const QList<MediaItem> restored = spy.first().first().value<QList<MediaItem>>();
    QCOMPARE(restored.size(), 2);
    QCOMPARE(restored.at(0).fileName(), QStringLiteral("first.mp4"));
    QCOMPARE(restored.at(1).fileName(), QStringLiteral("second.mov"));
    QCOMPARE(restored.at(0).id(), app.mediaItems().at(0).id());
}

void ProjectTest::newProjectEmitsEmptyMediaListChanged()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("clear.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(8, 'c'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    QCOMPARE(app.mediaItems().size(), 1);

    QSignalSpy spy(&app, &Application::mediaListChanged);
    app.newProject();

    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().value<QList<MediaItem>>().isEmpty());
    QVERIFY(app.mediaItems().isEmpty());
}

void ProjectTest::removeMediaRemovesMatchingRecordOnlyAndPersists()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("keep.mp4"));
    const QString secondPath = tempDir.filePath(QStringLiteral("remove_me.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(24, 'r'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));
    QCOMPARE(app.mediaItems().size(), 2);

    const QString removedId = app.mediaItems().at(1).id();
    const QString keptName = app.mediaItems().at(0).fileName();

    QSignalSpy listSpy(&app, &Application::mediaListChanged);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(app.removeMedia(removedId));

    QCOMPARE(app.mediaItems().size(), 1);
    QCOMPARE(app.mediaItems().at(0).fileName(), keptName);
    QCOMPARE(listSpy.count(), 1);
    QCOMPARE(listSpy.first().first().value<QList<MediaItem>>().size(), 1);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("Removed media")));

    // Persistence reflects the removal.
    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));
    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 1);
    QCOMPARE(reopened.mediaItems().at(0).fileName(), keptName);
}

void ProjectTest::removeMediaUnknownIdFailsDeterministically()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("stays.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(8, 's'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    QSignalSpy listSpy(&app, &Application::mediaListChanged);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(!app.removeMedia(QStringLiteral("no-such-id")));

    QCOMPARE(app.mediaItems().size(), 1);
    QCOMPARE(listSpy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("Remove failed")));
}

void ProjectTest::removeMediaRequiresActiveProject()
{
    Application app;
    QSignalSpy listSpy(&app, &Application::mediaListChanged);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);

    QVERIFY(!app.removeMedia(QStringLiteral("any-id")));
    QCOMPARE(listSpy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("No project")));
}

void ProjectTest::removeUnavailableMediaClearsUnavailableState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("gone_soon.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'g'));
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
    QCOMPARE(appOpen.unavailableMediaCount(), 1);

    const QString mediaId = appOpen.mediaItems().at(0).id();
    QVERIFY(appOpen.removeMedia(mediaId));

    QVERIFY(appOpen.mediaItems().isEmpty());
    QVERIFY(!appOpen.hasUnavailableMedia());
    QCOMPARE(appOpen.unavailableMediaCount(), 0);
}

void ProjectTest::mainWindowMediaListPopulatedAndRemoveWorks()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'm'));
    mediaFile.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&window, &MainWindow::removeMediaRequested,
                     &app, &Application::removeMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    auto *list = window.findChild<QListWidget *>(QStringLiteral("mediaListWidget"));
    QVERIFY(list);
    QCOMPARE(list->count(), 1);
    QVERIFY(list->item(0)->text().contains(QStringLiteral("media.bin")));

    auto *removeButton = window.findChild<QPushButton *>(QStringLiteral("removeMediaButton"));
    QVERIFY(removeButton);

    // No selection: clicking Remove is a no-op with status feedback.
    QSignalSpy spy(&window, &MainWindow::removeMediaRequested);
    removeButton->click();
    QCOMPARE(spy.count(), 0);
    auto *statusLabel = window.findChild<QLabel *>(QStringLiteral("statusLabel"));
    QVERIFY(statusLabel);
    QVERIFY(statusLabel->text().contains(QStringLiteral("No media selected")));

    // Selecting the entry and removing emits the request with its media id.
    const QString mediaId = app.mediaItems().at(0).id();
    list->setCurrentRow(0);
    removeButton->click();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), mediaId);

    QVERIFY(app.mediaItems().isEmpty());
    QCOMPARE(list->count(), 0);
}

void ProjectTest::activeMediaRequiresProjectAndSetsState()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("act.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(12, 'a'));
    mediaFile.close();

    Application app;
    QSignalSpy spy(&app, &Application::activeMediaChanged);

    // No active project: selection fails safely.
    QVERIFY(!app.setActiveMedia(QStringLiteral("any")));
    QCOMPARE(spy.count(), 0);
    QVERIFY(app.activeMediaId().isEmpty());
    QVERIFY(app.activeMediaItem() == nullptr);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString id = app.mediaItems().at(0).id();

    QVERIFY(app.setActiveMedia(id));
    QCOMPARE(app.activeMediaId(), id);
    QVERIFY(app.activeMediaItem() != nullptr);
    QCOMPARE(app.activeMediaItem()->id(), id);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), id);
}

void ProjectTest::activeMediaSameIdAndUnknownIdBehavior()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("same.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(8, 's'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString id = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(id));

    // Same id again: no change, no activeMediaChanged emission.
    QSignalSpy spy(&app, &Application::activeMediaChanged);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(app.setActiveMedia(id));
    QCOMPARE(spy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("already active")));
    QCOMPARE(app.activeMediaId(), id);

    // Unknown id (including empty): fails, state unchanged, no emission.
    QVERIFY(!app.setActiveMedia(QStringLiteral("no-such-id")));
    QVERIFY(!app.setActiveMedia(QString()));
    QCOMPARE(app.activeMediaId(), id);
    QCOMPARE(spy.count(), 0);
}

void ProjectTest::activeMediaImportNeverAutoSelects()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    auto createFile = [&tempDir](const QString &name) {
        const QString path = tempDir.filePath(name);
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(QByteArray(8, 'n'));
        file.close();
        return path;
    };

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(createFile(QStringLiteral("a.bin"))));
    QVERIFY(app.importMediaFile(createFile(QStringLiteral("b.bin"))));

    // Imports never auto-select.
    QVERIFY(app.activeMediaId().isEmpty());
    QVERIFY(app.activeMediaItem() == nullptr);

    // Selecting then importing another file leaves the selection unchanged.
    const QString id = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(id));
    QVERIFY(app.importMediaFile(createFile(QStringLiteral("c.mov"))));
    QCOMPARE(app.activeMediaId(), id);
    QCOMPARE(app.mediaItems().size(), 3);
}

void ProjectTest::activeMediaRemovalRules()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.bin"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.bin"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(8, 'r'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    const QString activeId = app.mediaItems().at(0).id();
    const QString otherId = app.mediaItems().at(1).id();
    QVERIFY(app.setActiveMedia(activeId));

    QSignalSpy spy(&app, &Application::activeMediaChanged);

    // Removing a non-active record preserves the active id (no emission).
    QVERIFY(app.removeMedia(otherId));
    QCOMPARE(app.activeMediaId(), activeId);
    QCOMPARE(spy.count(), 0);

    // Removing the active record clears it with one emission.
    QVERIFY(app.removeMedia(activeId));
    QVERIFY(app.activeMediaId().isEmpty());
    QVERIFY(app.activeMediaItem() == nullptr);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().isEmpty());
}

void ProjectTest::activeMediaClearedOnNewProject()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("clear.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(6, 'c'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString id = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(id));

    QSignalSpy spy(&app, &Application::activeMediaChanged);
    app.newProject();

    QVERIFY(app.activeMediaId().isEmpty());
    QVERIFY(app.activeMediaItem() == nullptr);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().isEmpty());
}

void ProjectTest::activeMediaRoundTripRestoresOnReopen()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("first.bin"));
    const QString secondPath = tempDir.filePath(QStringLiteral("second.mov"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(16, 'p'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));

    const QString activeId = app.mediaItems().at(1).id();
    QVERIFY(app.setActiveMedia(activeId));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QSignalSpy spy(&reopened, &Application::activeMediaChanged);
    QVERIFY(reopened.openProject(projectPath));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(reopened.activeMediaId(), activeId);
    QVERIFY(reopened.activeMediaItem() != nullptr);
    QCOMPARE(reopened.activeMediaItem()->id(), activeId);
    QCOMPARE(reopened.activeMediaItem()->fileName(), QStringLiteral("second.mov"));
}

void ProjectTest::activeMediaOpenClearsDanglingOrLegacy()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'm'));
    mediaFile.close();
    const MediaItem item = MediaItem::createFromFilePath(mediaPath);
    QVERIFY(item.isValid());

    auto writeProject = [&tempDir](const QString &name, const QJsonArray &media,
                                   const QJsonValue &activeId) {
        QJsonObject project;
        project.insert(QStringLiteral("id"), QStringLiteral("id-") + name);
        project.insert(QStringLiteral("name"), QStringLiteral("Project ") + name);
        project.insert(QStringLiteral("created"),
                       QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        project.insert(QStringLiteral("schemaVersion"), Project::CurrentSchemaVersion);
        project.insert(QStringLiteral("media"), media);
        if (activeId.isString()) {
            project.insert(QStringLiteral("activeMediaId"), activeId);
        }
        const QString projectPath = tempDir.filePath(name + QStringLiteral(".reel"));
        QFile file(projectPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return QString();
        }
        file.write(QJsonDocument(project).toJson(QJsonDocument::Compact));
        file.close();
        return projectPath;
    };

    QJsonArray single;
    single.append(item.toJsonObject());

    // Dangling persisted active id: cleared deterministically.
    const QString danglingPath =
        writeProject(QStringLiteral("dangling"), single, QJsonValue(QStringLiteral("no-such-id")));
    QVERIFY(!danglingPath.isEmpty());
    Application appDangling;
    QVERIFY(appDangling.openProject(danglingPath));
    QCOMPARE(appDangling.mediaItems().size(), 1);
    QVERIFY(appDangling.activeMediaId().isEmpty());

    // Legacy project without the key: no active media.
    const QString legacyPath =
        writeProject(QStringLiteral("legacy"), single, QJsonValue());
    QVERIFY(!legacyPath.isEmpty());
    Application appLegacy;
    QVERIFY(appLegacy.openProject(legacyPath));
    QVERIFY(appLegacy.activeMediaId().isEmpty());

    // Duplicate media entries + valid active id: normalization yields one
    // record and the active id resolves to it.
    QJsonArray duplicated;
    duplicated.append(item.toJsonObject());
    duplicated.append(item.toJsonObject());
    const QString dupPath =
        writeProject(QStringLiteral("dups"), duplicated, QJsonValue(item.id()));
    QVERIFY(!dupPath.isEmpty());
    Application appDup;
    QSignalSpy spy(&appDup, &Application::activeMediaChanged);
    QVERIFY(appDup.openProject(dupPath));
    QCOMPARE(appDup.mediaItems().size(), 1);
    QCOMPARE(appDup.activeMediaId(), item.id());
    QCOMPARE(spy.count(), 1);
}

void ProjectTest::activeMediaUnavailableCannotBeActive()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("vanishes.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'v'));
    mediaFile.close();

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString id = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(id));

    const QString projectPath = tempDir.filePath(QStringLiteral("project.reel"));
    QVERIFY(app.saveProject(projectPath));
    QVERIFY(QFile::remove(mediaPath));

    // Reopen: the record is retained but unavailable; the persisted active id
    // must be cleared (never a dangling/unavailable active selection).
    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 1);
    QVERIFY(reopened.hasUnavailableMedia());
    QVERIFY(reopened.activeMediaId().isEmpty());
    QVERIFY(reopened.activeMediaItem() == nullptr);

    // Selecting an unavailable record is rejected.
    QSignalSpy messageSpy(&reopened, &Application::backgroundCompleted);
    QVERIFY(!reopened.setActiveMedia(id));
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("unavailable")));
    QVERIFY(reopened.activeMediaId().isEmpty());
}

void ProjectTest::mainWindowSetActiveAndLabelWork()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile mediaFile(mediaPath);
    QVERIFY(mediaFile.open(QIODevice::WriteOnly));
    mediaFile.write(QByteArray(16, 'm'));
    mediaFile.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);
    QObject::connect(&window, &MainWindow::setActiveRequested,
                     &app, &Application::setActiveMedia);
    QObject::connect(&window, &MainWindow::removeMediaRequested,
                     &app, &Application::removeMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));

    auto *setActiveButton = window.findChild<QPushButton *>(QStringLiteral("setActiveButton"));
    QVERIFY(setActiveButton);
    auto *activeLabel = window.findChild<QLabel *>(QStringLiteral("activeMediaLabel"));
    QVERIFY(activeLabel);
    QCOMPARE(activeLabel->text(), QStringLiteral("Active media: None"));

    // No selection: clicking Set Active is a no-op with status feedback.
    QSignalSpy spy(&window, &MainWindow::setActiveRequested);
    setActiveButton->click();
    QCOMPARE(spy.count(), 0);
    auto *statusLabel = window.findChild<QLabel *>(QStringLiteral("statusLabel"));
    QVERIFY(statusLabel);
    QVERIFY(statusLabel->text().contains(QStringLiteral("No media selected to set active")));

    // Selecting the row and setting active updates id, label, and row marking.
    auto *list = window.findChild<QListWidget *>(QStringLiteral("mediaListWidget"));
    QVERIFY(list);
    const QString mediaId = app.mediaItems().at(0).id();
    list->setCurrentRow(0);
    setActiveButton->click();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), mediaId);
    QCOMPARE(app.activeMediaId(), mediaId);
    QCOMPARE(activeLabel->text(), QStringLiteral("Active media: media.bin"));
    QVERIFY(list->item(0)->text().startsWith(QStringLiteral("▶ ")));

    // Removing the active media clears the label back to None.
    auto *removeButton = window.findChild<QPushButton *>(QStringLiteral("removeMediaButton"));
    QVERIFY(removeButton);
    list->setCurrentRow(0);
    removeButton->click();
    QVERIFY(app.activeMediaId().isEmpty());
    QCOMPARE(activeLabel->text(), QStringLiteral("Active media: None"));
    QCOMPARE(list->count(), 0);
}

namespace {

void expectColor(const QImage &image, int x, int y, const QColor &expected)
{
    const QColor color = image.pixelColor(x, y);
    QCOMPARE(color.red(), expected.red());
    QCOMPARE(color.green(), expected.green());
    QCOMPARE(color.blue(), expected.blue());
}

double nearestPatchDistance(const QImage &image, const QColor &color,
                            int centerX, int centerY)
{
    double best = -1.0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y) != color) {
                continue;
            }
            const double dx = x - centerX;
            const double dy = y - centerY;
            const double distance = std::sqrt(dx * dx + dy * dy);
            if (best < 0.0 || distance < best) {
                best = distance;
            }
        }
    }
    return best;
}

} // namespace

void ProjectTest::equirectViewIdentityCentersFront()
{
    const QImage pattern = buildTestPattern();
    QImage view;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 200, 100, &view));
    QCOMPARE(view.width(), 200);
    QCOMPARE(view.height(), 100);

    expectColor(view, 100, 50, kFrontColor);
    expectColor(view, 2, 2, kPatternBackground);
}

void ProjectTest::equirectViewYawCentersRightAndLeft()
{
    const QImage pattern = buildTestPattern();

    QImage viewRight;
    QVERIFY(EquirectView::render(pattern, 90.0, 0.0, 0.0, 90.0, 200, 100, &viewRight));
    expectColor(viewRight, 100, 50, kRightColor);

    QImage viewLeft;
    QVERIFY(EquirectView::render(pattern, -90.0, 0.0, 0.0, 90.0, 200, 100, &viewLeft));
    expectColor(viewLeft, 100, 50, kLeftColor);
}

void ProjectTest::equirectViewPitchReachesUpAndDown()
{
    const QImage pattern = buildTestPattern();

    QImage viewUp;
    QVERIFY(EquirectView::render(pattern, 0.0, 90.0, 0.0, 90.0, 200, 100, &viewUp));
    expectColor(viewUp, 100, 50, kUpColor);

    QImage viewDown;
    QVERIFY(EquirectView::render(pattern, 0.0, -90.0, 0.0, 90.0, 200, 100, &viewDown));
    expectColor(viewDown, 100, 50, kDownColor);
}

void ProjectTest::equirectViewPositiveRollRotatesContentCorrectly()
{
    const QImage pattern = buildTestPattern();

    // A stripe above FRONT (world yaw 0, pitch 20). At zero roll it must sit
    // above center; positive roll must move it to the left, negative roll to
    // the right (matching the ViewerProjection convention that positive roll
    // rotates projected content counter-clockwise on screen).
    QImage viewZero;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 200, 100, &viewZero));
    const QuadrantCounts zeroCounts = countQuadrants(viewZero, kUp20Color);
    QVERIFY(zeroCounts.above > 0);
    QVERIFY(zeroCounts.above > zeroCounts.below);

    QImage viewRolled;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 90.0, 90.0, 200, 100, &viewRolled));
    const QuadrantCounts rolledCounts = countQuadrants(viewRolled, kUp20Color);
    QVERIFY(rolledCounts.left > 0);
    QVERIFY(rolledCounts.left > rolledCounts.right);
    QVERIFY(rolledCounts.left > rolledCounts.above);

    QImage viewRolledBack;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, -90.0, 90.0, 200, 100, &viewRolledBack));
    const QuadrantCounts backCounts = countQuadrants(viewRolledBack, kUp20Color);
    QVERIFY(backCounts.right > 0);
    QVERIFY(backCounts.right > backCounts.left);
    QVERIFY(backCounts.right > backCounts.above);
}

void ProjectTest::equirectViewFieldOfViewChangesCoverage()
{
    const QImage pattern = buildTestPattern();

    QImage viewNarrow;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 60.0, 200, 100, &viewNarrow));
    QImage viewWide;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 120.0, 200, 100, &viewWide));

    const double distanceNarrow = nearestPatchDistance(viewNarrow, kUp20Color, 100, 50);
    const double distanceWide = nearestPatchDistance(viewWide, kUp20Color, 100, 50);
    QVERIFY(distanceNarrow > 0.0);
    QVERIFY(distanceWide > 0.0);
    // A larger vertical FOV brings the same direction closer to center.
    QVERIFY(distanceNarrow > distanceWide);
}

void ProjectTest::equirectViewRejectsInvalidInput()
{
    const QImage pattern = buildTestPattern();
    QImage out(4, 4, QImage::Format_ARGB32);
    out.fill(QColor(255, 0, 0));

    const QColor sentinel(255, 0, 0);
    auto expectUnchanged = [&out, &sentinel]() {
        expectColor(out, 0, 0, sentinel);
        expectColor(out, 3, 3, sentinel);
    };

    // Empty source.
    QVERIFY(!EquirectView::render(QImage(), 0.0, 0.0, 0.0, 90.0, 200, 100, &out));
    expectUnchanged();

    // Invalid output dimensions.
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 0, 100, &out));
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 200, -5, &out));
    expectUnchanged();

    // Invalid FOV (outside ViewportState clamp semantics [20, 140]).
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 10.0, 200, 100, &out));
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 150.0, 200, 100, &out));
    expectUnchanged();

    // Invalid camera values.
    QVERIFY(!EquirectView::render(pattern, 0.0, 91.0, 0.0, 90.0, 200, 100, &out));
    QVERIFY(!EquirectView::render(pattern, 0.0, -91.0, 0.0, 90.0, 200, 100, &out));
    QVERIFY(!EquirectView::render(pattern, qQNaN(), 0.0, 0.0, 90.0, 200, 100, &out));
    expectUnchanged();

    // Null output pointer.
    QVERIFY(!EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0, 200, 100, nullptr));
}

void ProjectTest::equirectViewDeterministicRepeatability()
{
    const QImage pattern = buildTestPattern();

    QImage first;
    QImage second;
    QVERIFY(EquirectView::render(pattern, 30.0, 15.0, 45.0, 95.0, 200, 100, &first));
    QVERIFY(EquirectView::render(pattern, 30.0, 15.0, 45.0, 95.0, 200, 100, &second));

    QVERIFY(imagesIdentical(first, second));
}

void ProjectTest::viewerWidgetSourceImageRendersThroughCamera()
{
    ViewportState state;
    ViewerWidget widget;
    widget.setViewportState(&state);
    widget.resize(200, 100);

    widget.setSourceImage(buildTestPattern());
    QVERIFY(widget.hasSourceImage());

    QImage view = widget.grab().toImage();
    expectColor(view, 100, 50, kFrontColor);

    // The presentation follows the authoritative ViewportState: +90 yaw
    // centers the RIGHT region.
    state.setYaw(90.0);
    view = widget.grab().toImage();
    expectColor(view, 100, 50, kRightColor);
}

void ProjectTest::viewerWidgetClearingSourceRestoresSceneRendering()
{
    ViewportState state;
    ViewerWidget widget;
    widget.setViewportState(&state);
    widget.resize(200, 100);

    widget.setSourceImage(buildTestPattern());
    QVERIFY(widget.hasSourceImage());
    state.setYaw(0.0);

    // Clearing the source returns to the deterministic marker scene.
    widget.setSourceImage(QImage());
    QVERIFY(!widget.hasSourceImage());

    QImage view = widget.grab().toImage();
    expectColor(view, 100, 50, QColor(255, 213, 79)); // FRONT marker dot
    expectColor(view, 10, 10, QColor(18, 20, 24));    // scene background
}

void ProjectTest::equirectViewPerformanceSanity()
{
    const QImage pattern = buildTestPattern();

    // Warm-up at the capped resolution.
    QImage warm;
    QVERIFY(EquirectView::render(pattern, 0.0, 0.0, 0.0, 90.0,
                                  EquirectView::MaxOutputWidth, 320, &warm));

    constexpr int kIterations = 5;
    QElapsedTimer timer;
    timer.start();
    QImage view;
    for (int i = 0; i < kIterations; ++i) {
        QVERIFY(EquirectView::render(pattern, 10.0, 5.0, 3.0, 90.0,
                                     EquirectView::MaxOutputWidth, 320, &view));
    }
    const double cappedMs = static_cast<double>(timer.nsecsElapsed()) / kIterations / 1e6;
    qInfo("EquirectView bilinear CPU render: %.2f ms/frame at %dx%d (max width %d)",
          cappedMs, EquirectView::MaxOutputWidth, 320, EquirectView::MaxOutputWidth);

    // Informational only (no gate): quantifies the cost of a future cap raise.
    timer.restart();
    for (int i = 0; i < kIterations; ++i) {
        QVERIFY(EquirectView::render(pattern, 10.0, 5.0, 3.0, 90.0, 1280, 640, &view));
    }
    const double wideMs = static_cast<double>(timer.nsecsElapsed()) / kIterations / 1e6;
    qInfo("EquirectView bilinear CPU render (informational): %.2f ms/frame at 1280x640",
          wideMs);

    // Generous sanity bound; this is a CPU-cost measurement, not a
    // performance regression gate.
    QVERIFY(cappedMs < 2000.0);
}

void ProjectTest::frameExtractorRejectsInvalidInput()
{
    // Deterministic failures that do not require an ffmpeg executable.
    QString error;
    QImage out;

    // Missing media file (checked before process start).
    QVERIFY(!FrameExtractor::extractFirstFrame(
        QStringLiteral("/nonexistent/nope.png"), QStringLiteral("/no/ffmpeg"), &out, &error));
    QVERIFY(error.contains(QStringLiteral("does not exist")));

    // Empty executable path is reported before file checks.
    error.clear();
    QVERIFY(!FrameExtractor::extractFirstFrame(
        QStringLiteral("/nonexistent/nope.png"), QString(), &out, &error));
    QVERIFY(error.contains(QStringLiteral("ffmpeg not found")));

    error.clear();
    QVERIFY(!FrameExtractor::extractFirstFrame(
        QStringLiteral("/nonexistent/nope.png"), QStringLiteral("/no/ffmpeg"), nullptr, &error));
    QVERIFY(error.contains(QStringLiteral("null output")));
}

void ProjectTest::frameExtractorAvailabilityAndSingleFrameDecode()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QColor fill(0, 0, 255);
    QImage source(64, 32, QImage::Format_RGB32);
    source.fill(fill);
    const QString mediaPath = tempDir.filePath(QStringLiteral("frame.png"));
    QVERIFY(source.save(mediaPath, "PNG"));

    QImage decoded;
    QString error;
    QVERIFY(FrameExtractor::extractFirstFrame(
        mediaPath, FrameExtractor::defaultExecutablePath(), &decoded, &error));
    QVERIFY(error.isEmpty());
    QVERIFY(!decoded.isNull());
    QCOMPARE(decoded.width(), 64);
    QCOMPARE(decoded.height(), 32);
    expectColor(decoded, 0, 0, fill);
    expectColor(decoded, 63, 31, fill);
}

void ProjectTest::frameExtractorDeterministicRepeatability()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QImage source(48, 24, QImage::Format_RGB32);
    source.fill(QColor(0, 200, 200));
    const QString mediaPath = tempDir.filePath(QStringLiteral("repeat.png"));
    QVERIFY(source.save(mediaPath, "PNG"));

    QImage first;
    QImage second;
    QVERIFY(FrameExtractor::extractFirstFrame(
        mediaPath, FrameExtractor::defaultExecutablePath(), &first));
    QVERIFY(FrameExtractor::extractFirstFrame(
        mediaPath, FrameExtractor::defaultExecutablePath(), &second));
    QVERIFY(imagesIdentical(first, second));
}

void ProjectTest::applicationPreviewRequiresProjectAndActive()
{
    Application app;
    QSignalSpy spy(&app, &Application::framePreviewReady);

    // No project.
    QVERIFY(!app.previewActiveMediaFrame());
    QCOMPARE(spy.count(), 0);

    // Project but no active media.
    app.newProject();
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(!app.previewActiveMediaFrame());
    QCOMPARE(spy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("No active media")));
}

void ProjectTest::applicationPreviewEmitsFramePreview()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("preview.png"));
    QImage source(80, 40, QImage::Format_RGB32);
    source.fill(QColor(255, 0, 0));
    QVERIFY(source.save(mediaPath, "PNG"));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    QSignalSpy spy(&app, &Application::framePreviewReady);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(app.previewActiveMediaFrame());

    QCOMPARE(spy.count(), 1);
    const QImage frame = spy.first().first().value<QImage>();
    QVERIFY(!frame.isNull());
    QCOMPARE(frame.width(), 80);
    QCOMPARE(frame.height(), 40);
    expectColor(frame, 40, 20, QColor(255, 0, 0));
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("Frame extracted")));
}

void ProjectTest::mainWindowPreviewButtonEmitsSignal()
{
    TestMainWindow window;
    QSignalSpy spy(&window, &MainWindow::previewFrameRequested);

    auto *button = window.findChild<QPushButton *>(QStringLiteral("previewFrameButton"));
    QVERIFY(button);
    button->click();
    QCOMPARE(spy.count(), 1);
}

void ProjectTest::previewEndToEndShowsActiveFrameInViewer()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // The deterministic equirectangular test pattern saved as a real PNG media
    // file: identity camera must center the FRONT region after decode.
    const QString mediaPath = tempDir.filePath(QStringLiteral("pattern.png"));
    QVERIFY(buildTestPattern().save(mediaPath, "PNG"));

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::framePreviewReady,
                     &window, &MainWindow::showFramePreview);
    QObject::connect(&window, &MainWindow::previewFrameRequested,
                     &app, &Application::previewActiveMediaFrame);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    viewer->resize(200, 100);

    auto *button = window.findChild<QPushButton *>(QStringLiteral("previewFrameButton"));
    QVERIFY(button);

    QSignalSpy spy(&app, &Application::framePreviewReady);
    button->click();

    // Localize: inspect the decoded frame before the viewer mapping.
    QCOMPARE(spy.count(), 1);
    const QImage decoded = spy.first().first().value<QImage>();
    QVERIFY(!decoded.isNull());
    expectColor(decoded, decoded.width() / 2, decoded.height() / 2, kFrontColor);

    QVERIFY(viewer->hasSourceImage());
    const QImage view = viewer->grab().toImage();
    // Identity camera centers FRONT; sample the true image center (the widget
    // sits inside a layout, so its actual geometry may exceed the requested
    // resize).
    expectColor(view, view.width() / 2, view.height() / 2, kFrontColor);
}

void ProjectTest::frameExtractorRejectsInvalidSeekTime()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("frame.png"));
    QImage source(16, 16, QImage::Format_RGB32);
    source.fill(QColor(255, 255, 255));
    QVERIFY(source.save(mediaPath, "PNG"));

    QString error;
    QImage out;
    QVERIFY(!FrameExtractor::extractFrameAt(
        mediaPath, QStringLiteral("/no/ffmpeg"), -5.0, &out, &error));
    QVERIFY(error.contains(QStringLiteral("invalid seek time")));

    error.clear();
    QVERIFY(!FrameExtractor::extractFrameAt(
        mediaPath, QStringLiteral("/no/ffmpeg"), qQNaN(), &out, &error));
    QVERIFY(error.contains(QStringLiteral("invalid seek time")));

    error.clear();
    QVERIFY(!FrameExtractor::extractFrameAt(
        mediaPath, QString(), 0.0, &out, &error));
    QVERIFY(error.contains(QStringLiteral("ffmpeg not found")));
}

void ProjectTest::frameExtractorExtractsFrameAtSeekTime()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    QImage atZero;
    QImage atTwo;
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 0.0, &atZero));
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 2.0, &atTwo));

    QVERIFY(!atZero.isNull());
    QVERIFY(!atTwo.isNull());
    QVERIFY(redDominant(atZero.pixelColor(atZero.width() / 2, atZero.height() / 2)));
    QVERIFY(blueDominant(atTwo.pixelColor(atTwo.width() / 2, atTwo.height() / 2)));
    QVERIFY(!imagesIdentical(atZero, atTwo));
}

void ProjectTest::frameExtractorSeekDeterministicRepeatability()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    QImage first;
    QImage second;
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 1.5, &first));
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 1.5, &second));
    QVERIFY(imagesIdentical(first, second));
}

void ProjectTest::applicationTimeNavigationGuards()
{
    Application app;
    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);
    QSignalSpy frameSpy(&app, &Application::framePreviewReady);

    // No project.
    QVERIFY(!app.previewActiveMediaFrameAt(1.0));
    QVERIFY(!app.stepActiveMediaPreview(1.0));
    QCOMPARE(timeSpy.count(), 0);
    QCOMPARE(frameSpy.count(), 0);

    // Project but no active media.
    app.newProject();
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    QVERIFY(!app.previewActiveMediaFrameAt(1.0));
    QVERIFY(!app.stepActiveMediaPreview(1.0));
    QCOMPARE(timeSpy.count(), 0);
    QCOMPARE(frameSpy.count(), 0);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("No active media")));
}

void ProjectTest::applicationPreviewAtUpdatesPositionAndEmits()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(videoPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);
    QSignalSpy frameSpy(&app, &Application::framePreviewReady);

    QVERIFY(app.previewActiveMediaFrameAt(2.0));
    QCOMPARE(app.previewTimeSeconds(), 2.0);
    QCOMPARE(timeSpy.count(), 1);
    QCOMPARE(timeSpy.first().first().toDouble(), 2.0);
    QCOMPARE(frameSpy.count(), 1);
    const QImage frame = frameSpy.first().first().value<QImage>();
    QVERIFY(!frame.isNull());
    QVERIFY(blueDominant(frame.pixelColor(frame.width() / 2, frame.height() / 2)));

    // Re-requesting the same position does not re-emit the time change.
    QVERIFY(app.previewActiveMediaFrameAt(2.0));
    QCOMPARE(timeSpy.count(), 1);
    QCOMPARE(frameSpy.count(), 2);
}

void ProjectTest::applicationStepAdvancesAndClampsBelowZero()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(videoPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);

    // +1 s from 0.
    QVERIFY(app.stepActiveMediaPreview(1.0));
    QCOMPARE(app.previewTimeSeconds(), 1.0);
    QCOMPARE(timeSpy.count(), 1);

    // Stepping below zero clamps to 0 and decodes there.
    QVERIFY(app.stepActiveMediaPreview(-3.0));
    QCOMPARE(app.previewTimeSeconds(), 0.0);
    QCOMPARE(timeSpy.count(), 2);
}

void ProjectTest::applicationSeekBeyondEndFailsDeterministically()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &videoPath));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(videoPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    QVERIFY(app.previewActiveMediaFrameAt(0.5));
    QCOMPARE(app.previewTimeSeconds(), 0.5);

    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);
    QSignalSpy frameSpy(&app, &Application::framePreviewReady);
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);
    timeSpy.clear();
    frameSpy.clear();
    messageSpy.clear();

    QVERIFY(!app.previewActiveMediaFrameAt(500.0));
    QCOMPARE(app.previewTimeSeconds(), 0.5); // unchanged
    QCOMPARE(timeSpy.count(), 0);
    QCOMPARE(frameSpy.count(), 0);
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("Preview failed")));
}

void ProjectTest::applicationTimeResetsOnProjectActiveAndRemoval()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString firstVideo;
    QVERIFY(createSteppedVideo(tempDir.path(), FrameExtractor::defaultExecutablePath(),
                               &firstVideo));
    const QString secondVideo = tempDir.filePath(QStringLiteral("second.mp4"));
    QVERIFY(QFile::copy(firstVideo, secondVideo));

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(firstVideo));
    const QString firstId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(firstId));
    QVERIFY(app.previewActiveMediaFrameAt(1.0));
    QCOMPARE(app.previewTimeSeconds(), 1.0);

    // New project resets the position.
    QSignalSpy timeSpy(&app, &Application::previewTimeChanged);
    app.newProject();
    QCOMPARE(app.previewTimeSeconds(), 0.0);
    QCOMPARE(timeSpy.count(), 1);

    // Changing the active media resets the position.
    QVERIFY(app.importMediaFile(firstVideo));
    QVERIFY(app.importMediaFile(secondVideo));
    const QString idA = app.mediaItems().at(0).id();
    const QString idB = app.mediaItems().at(1).id();
    QVERIFY(app.setActiveMedia(idA));
    QVERIFY(app.previewActiveMediaFrameAt(2.0));
    QCOMPARE(app.previewTimeSeconds(), 2.0);

    timeSpy.clear();
    QVERIFY(app.setActiveMedia(idB));
    QCOMPARE(app.previewTimeSeconds(), 0.0);
    QCOMPARE(timeSpy.count(), 1);

    // Removing the active media resets the position.
    QVERIFY(app.previewActiveMediaFrameAt(1.5));
    QCOMPARE(app.previewTimeSeconds(), 1.5);
    timeSpy.clear();
    QVERIFY(app.removeMedia(idB));
    QCOMPARE(app.previewTimeSeconds(), 0.0);
    QCOMPARE(timeSpy.count(), 1);
}

void ProjectTest::mainWindowStepButtonsAndTimeLabel()
{
    TestMainWindow window;
    QSignalSpy spy(&window, &MainWindow::previewStepRequested);

    auto *backButton = window.findChild<QPushButton *>(QStringLiteral("stepBackButton"));
    auto *forwardButton = window.findChild<QPushButton *>(QStringLiteral("stepForwardButton"));
    auto *timeLabel = window.findChild<QLabel *>(QStringLiteral("previewTimeLabel"));
    QVERIFY(backButton);
    QVERIFY(forwardButton);
    QVERIFY(timeLabel);
    QCOMPARE(timeLabel->text(), QStringLiteral("Time: 0.0 s"));

    backButton->click();
    forwardButton->click();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).first().toDouble(), -1.0);
    QCOMPARE(spy.at(1).first().toDouble(), 1.0);

    window.showPreviewTime(3.5);
    QCOMPARE(timeLabel->text(), QStringLiteral("Time: 3.5 s"));
}

namespace {

// Objective 11 pointer test helpers.
constexpr double kLookDegreesPerPixelTest = 0.25;
constexpr double kFovDegreesPerWheelStepTest = 5.0;

void sendMouseEvent(ViewerWidget &widget, QEvent::Type type, const QPointF &position,
                    Qt::MouseButton button, Qt::MouseButtons buttons,
                    Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    QMouseEvent event(type, position, button, buttons, modifiers);
    QApplication::sendEvent(&widget, &event);
}

void sendWheelEvent(ViewerWidget &widget, int angleDeltaY)
{
    QWheelEvent event(QPointF(10, 10), QPointF(10, 10), QPoint(0, 0),
                      QPoint(0, angleDeltaY), Qt::NoButton, Qt::NoModifier,
                      Qt::NoScrollPhase, false);
    QApplication::sendEvent(&widget, &event);
}

bool nearDouble(double actual, double expected)
{
    return qAbs(actual - expected) < 1e-9;
}

} // namespace

void ProjectTest::viewerWidgetDragEmitsYawAndPitchDeltas()
{
    ViewerWidget widget;
    widget.resize(400, 300);

    QSignalSpy yawSpy(&widget, &ViewerWidget::viewportYawDeltaRequested);
    QSignalSpy pitchSpy(&widget, &ViewerWidget::viewportPitchDeltaRequested);

    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(100, 100),
                   Qt::LeftButton, Qt::LeftButton);
    // Drag right by 80 px: yaw increases by 0.25 * 80 = 20.
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(180, 100),
                   Qt::NoButton, Qt::LeftButton);
    QCOMPARE(yawSpy.count(), 1);
    QVERIFY(nearDouble(yawSpy.first().first().toDouble(), 20.0));
    QCOMPARE(pitchSpy.count(), 0);

    // Drag up by 40 px: pitch increases by 0.25 * 40 = 10.
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(180, 60),
                   Qt::NoButton, Qt::LeftButton);
    QCOMPARE(pitchSpy.count(), 1);
    QVERIFY(nearDouble(pitchSpy.first().first().toDouble(), 10.0));

    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(180, 60),
                   Qt::LeftButton, Qt::NoButton);
    QCOMPARE(yawSpy.count(), 1);
    QCOMPARE(pitchSpy.count(), 1);
    Q_UNUSED(kLookDegreesPerPixelTest);
}

void ProjectTest::viewerWidgetIgnoresNonLeftDragAndReleaseWithoutMove()
{
    ViewerWidget widget;
    widget.resize(400, 300);

    QSignalSpy yawSpy(&widget, &ViewerWidget::viewportYawDeltaRequested);
    QSignalSpy pitchSpy(&widget, &ViewerWidget::viewportPitchDeltaRequested);

    // Right-button drag is ignored.
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(10, 10),
                   Qt::RightButton, Qt::RightButton);
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(60, 10),
                   Qt::NoButton, Qt::RightButton);
    QCOMPARE(yawSpy.count(), 0);
    QCOMPARE(pitchSpy.count(), 0);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(60, 10),
                   Qt::RightButton, Qt::NoButton);

    // Press and release without any move emits nothing.
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(20, 20),
                   Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(20, 20),
                   Qt::LeftButton, Qt::NoButton);
    QCOMPARE(yawSpy.count(), 0);
    QCOMPARE(pitchSpy.count(), 0);
}

void ProjectTest::viewerWidgetWheelUpDecreasesFovWheelDownIncreases()
{
    ViewerWidget widget;
    widget.resize(400, 300);

    QSignalSpy fovSpy(&widget, &ViewerWidget::viewportFovDeltaRequested);

    // Wheel up one step: FOV decreases by 5 (zoom in).
    sendWheelEvent(widget, 120);
    QCOMPARE(fovSpy.count(), 1);
    QVERIFY(nearDouble(fovSpy.first().first().toDouble(), -kFovDegreesPerWheelStepTest));

    // Wheel down one step: FOV increases by 5 (zoom out).
    sendWheelEvent(widget, -120);
    QCOMPARE(fovSpy.count(), 2);
    QVERIFY(nearDouble(fovSpy.last().first().toDouble(), kFovDegreesPerWheelStepTest));
}

void ProjectTest::viewerWidgetDragUpdatesApplicationViewport()
{
    ViewerWidget widget;
    widget.resize(400, 300);

    Application app;
    ViewportState *state = app.viewportState();
    QVERIFY(state);

    QObject::connect(&widget, &ViewerWidget::viewportYawDeltaRequested,
                     &app, &Application::adjustViewportYaw);
    QObject::connect(&widget, &ViewerWidget::viewportPitchDeltaRequested,
                     &app, &Application::adjustViewportPitch);
    QObject::connect(&widget, &ViewerWidget::viewportFovDeltaRequested,
                     &app, &Application::adjustViewportFieldOfView);

    // Drag right 200 px: yaw 0 -> 50.
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(100, 100),
                   Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(300, 100),
                   Qt::NoButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(300, 100),
                   Qt::LeftButton, Qt::NoButton);
    QVERIFY(nearDouble(state->yaw(), 50.0));

    // Drag up by 400 px on a taller widget: pitch 0 -> clamped at 90 (max).
    widget.resize(400, 2000);
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPointF(150, 1500),
                   Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseMove, QPointF(150, 300),
                   Qt::NoButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPointF(150, 300),
                   Qt::LeftButton, Qt::NoButton);
    QVERIFY(nearDouble(state->pitch(), 90.0));

    // Wheel up one step: FOV 90 -> 85.
    sendWheelEvent(widget, 120);
    QVERIFY(nearDouble(state->fieldOfView(), 85.0));
}

void ProjectTest::mediaItemProjectionDefaultsUnknown()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("proj.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("X");
    file.close();

    const MediaItem item = MediaItem::createFromFilePath(mediaPath);
    QVERIFY(item.isValid());
    QCOMPARE(item.projection(), MediaItem::Projection::Unknown);

    QCOMPARE(MediaItem::projectionToString(MediaItem::Projection::Unknown), QString());
    QCOMPARE(MediaItem::projectionToString(MediaItem::Projection::Equirectangular),
             QStringLiteral("equirectangular"));
    QCOMPARE(MediaItem::projectionToString(MediaItem::Projection::Flat),
             QStringLiteral("flat"));
    QCOMPARE(MediaItem::projectionFromString(QString()), MediaItem::Projection::Unknown);
    QCOMPARE(MediaItem::projectionFromString(QStringLiteral("garbage")),
             MediaItem::Projection::Unknown);
    QCOMPARE(MediaItem::projectionFromString(QStringLiteral("flat")),
             MediaItem::Projection::Flat);
    QCOMPARE(MediaItem::projectionFromString(QStringLiteral("equirectangular")),
             MediaItem::Projection::Equirectangular);
}

void ProjectTest::mediaItemProjectionJsonRoundTrip()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("proj.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("Y");
    file.close();

    // Flat round trip.
    MediaItem flatItem = MediaItem::createFromFilePath(mediaPath);
    flatItem.setProjection(MediaItem::Projection::Flat);
    const QJsonObject flatObject = flatItem.toJsonObject();
    QCOMPARE(flatObject.value(QStringLiteral("projection")).toString(),
             QStringLiteral("flat"));

    MediaItem flatRestored;
    QString error;
    QVERIFY(flatRestored.readFromJsonObject(flatObject, &error));
    QCOMPARE(flatRestored.projection(), MediaItem::Projection::Flat);

    // Equirectangular round trip.
    flatItem.setProjection(MediaItem::Projection::Equirectangular);
    const QJsonObject equirectObject = flatItem.toJsonObject();
    MediaItem equirectRestored;
    QVERIFY(equirectRestored.readFromJsonObject(equirectObject, &error));
    QCOMPARE(equirectRestored.projection(), MediaItem::Projection::Equirectangular);

    // Unknown is not serialized; absent/unrecognized values read as Unknown.
    flatItem.setProjection(MediaItem::Projection::Unknown);
    QVERIFY(!flatItem.toJsonObject().contains(QStringLiteral("projection")));

    QJsonObject withBadProjection = flatObject;
    withBadProjection.insert(QStringLiteral("projection"), QStringLiteral("fisheye"));
    MediaItem badRestored;
    QVERIFY(badRestored.readFromJsonObject(withBadProjection, &error));
    QCOMPARE(badRestored.projection(), MediaItem::Projection::Unknown);
}

void ProjectTest::applicationDeclareProjectionGuardsValidateAndEmit()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("decl.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("Z");
    file.close();

    Application app;
    QSignalSpy messageSpy(&app, &Application::backgroundCompleted);

    // No project.
    QVERIFY(!app.declareMediaProjection(QStringLiteral("x"), QStringLiteral("flat")));
    QCOMPARE(messageSpy.count(), 1);
    QVERIFY(messageSpy.first().first().toString().contains(QStringLiteral("No project")));

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();

    // Unknown media id.
    QVERIFY(!app.declareMediaProjection(QStringLiteral("no-such-id"),
                                        QStringLiteral("flat")));
    QVERIFY(messageSpy.last().first().toString().contains(QStringLiteral("not found")));

    // Invalid projection value.
    QVERIFY(!app.declareMediaProjection(mediaId, QStringLiteral("fisheye")));
    QVERIFY(messageSpy.last().first().toString().contains(QStringLiteral("unknown projection")));

    // Valid declarations update the record and emit the list change.
    QSignalSpy listSpy(&app, &Application::mediaListChanged);
    QVERIFY(app.declareMediaProjection(mediaId, QStringLiteral("flat")));
    QCOMPARE(app.mediaItems().at(0).projection(), MediaItem::Projection::Flat);
    QCOMPARE(listSpy.count(), 1);
    QVERIFY(messageSpy.last().first().toString().contains(QStringLiteral("projection")));

    QVERIFY(app.declareMediaProjection(mediaId, QStringLiteral("equirectangular")));
    QCOMPARE(app.mediaItems().at(0).projection(), MediaItem::Projection::Equirectangular);
    QCOMPARE(listSpy.count(), 2);
}

void ProjectTest::applicationDeclaredProjectionPersistsOnReopen()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString flatPath = tempDir.filePath(QStringLiteral("flat.bin"));
    const QString unknownPath = tempDir.filePath(QStringLiteral("unknown.mov"));
    for (const QString &path : { flatPath, unknownPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(8, 'p'));
        file.close();
    }

    Application app;
    app.newProject();
    QVERIFY(app.importMediaFile(flatPath));
    QVERIFY(app.importMediaFile(unknownPath));
    const QString flatId = app.mediaItems().at(0).id();
    QVERIFY(app.declareMediaProjection(flatId, QStringLiteral("flat")));

    const QString projectPath = tempDir.filePath(QStringLiteral("proj.reel"));
    QVERIFY(app.saveProject(projectPath));

    Application reopened;
    QVERIFY(reopened.openProject(projectPath));
    QCOMPARE(reopened.mediaItems().size(), 2);
    QCOMPARE(reopened.mediaItems().at(0).projection(), MediaItem::Projection::Flat);
    QCOMPARE(reopened.mediaItems().at(1).projection(), MediaItem::Projection::Unknown);
}

void ProjectTest::viewerWidgetFlatModeLetterboxesAndCenters()
{
    ViewerWidget widget;
    widget.resize(400, 200);

    // 4:3 solid source inside a 2:1 widget => vertical fit, side letterbox.
    QImage source(200, 150, QImage::Format_RGB32);
    source.fill(QColor(0, 255, 255));
    widget.setFlatSourceMode(true);
    widget.setSourceImage(source);
    QVERIFY(widget.isFlatSourceMode());

    const QImage view = widget.grab().toImage();
    expectColor(view, 200, 100, QColor(0, 255, 255)); // content center
    expectColor(view, 5, 100, QColor(18, 20, 24));    // left letterbox bar
    expectColor(view, 395, 100, QColor(18, 20, 24));  // right letterbox bar
}

void ProjectTest::viewerWidgetFlatModeIgnoresCameraTransforms()
{
    ViewportState state;
    ViewerWidget widget;
    widget.setViewportState(&state);
    widget.resize(300, 300);

    QImage source(150, 150, QImage::Format_RGB32);
    source.fill(QColor(255, 0, 255));
    widget.setFlatSourceMode(true);
    widget.setSourceImage(source);

    const QImage before = widget.grab().toImage();
    state.setYaw(90.0);
    state.setPitch(45.0);
    state.setFieldOfView(40.0);
    const QImage after = widget.grab().toImage();
    QVERIFY(imagesIdentical(before, after));
}

void ProjectTest::mainWindowProjectionButtonsEmitRequests()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QList<MediaItem> items;
    const QStringList names = { QStringLiteral("a.bin"), QStringLiteral("b.bin") };
    for (const QString &name : names) {
        const QString path = tempDir.filePath(name);
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("M");
        file.close();
        items.append(MediaItem::createFromFilePath(path));
    }

    TestMainWindow window;
    window.showMediaList(items);

    auto *list = window.findChild<QListWidget *>(QStringLiteral("mediaListWidget"));
    QVERIFY(list);
    QCOMPARE(list->count(), 2);

    auto *markFlat = window.findChild<QPushButton *>(QStringLiteral("markFlatButton"));
    auto *markEquirect = window.findChild<QPushButton *>(QStringLiteral("markEquirectButton"));
    QVERIFY(markFlat);
    QVERIFY(markEquirect);

    QSignalSpy spy(&window, &MainWindow::setMediaProjectionRequested);
    list->setCurrentRow(0);
    markFlat->click();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), items.at(0).id());
    QCOMPARE(spy.first().at(1).toString(), QStringLiteral("flat"));

    markEquirect->click();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.last().first().toString(), items.at(0).id());
    QCOMPARE(spy.last().at(1).toString(), QStringLiteral("equirectangular"));
}

void ProjectTest::previewRoutingHonorsDeclaredProjection()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString mediaPath = tempDir.filePath(QStringLiteral("media.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("P");
    file.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);

    // Flat solid frame presented in flat mode (no camera transform).
    QVERIFY(app.declareMediaProjection(mediaId, QStringLiteral("flat")));
    QImage flatFrame(200, 100, QImage::Format_RGB32);
    flatFrame.fill(QColor(0, 255, 255));
    window.showFramePreview(flatFrame);
    QVERIFY(viewer->isFlatSourceMode());
    QImage view = viewer->grab().toImage();
    expectColor(view, view.width() / 2, view.height() / 2, QColor(0, 255, 255));

    // Equirectangular routing: equirectangular pixel path (FRONT centered).
    QVERIFY(app.declareMediaProjection(mediaId, QStringLiteral("equirectangular")));
    window.showFramePreview(buildTestPattern());
    QVERIFY(!viewer->isFlatSourceMode());
    view = viewer->grab().toImage();
    expectColor(view, view.width() / 2, view.height() / 2, kFrontColor);
}

void ProjectTest::viewerClearedOnNewProjectAfterPreview()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("a.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("A");
    file.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::projectChanged,
                     &window, &MainWindow::showProject);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    window.showFramePreview(QImage(100, 50, QImage::Format_RGB32));
    QVERIFY(viewer->hasSourceImage());

    // Creating a new project must clear the stale frame (back to scene) and
    // reset flat mode.
    viewer->setFlatSourceMode(true);
    app.newProject();
    QVERIFY(!viewer->hasSourceImage());
    QVERIFY(!viewer->isFlatSourceMode());

    // The deterministic marker scene is visible again at identity.
    const QImage view = viewer->grab().toImage();
    expectColor(view, view.width() / 2, view.height() / 2, QColor(255, 213, 79));
}

void ProjectTest::viewerClearedWhenActiveMediaChangesOrRemoved()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString firstPath = tempDir.filePath(QStringLiteral("a.bin"));
    const QString secondPath = tempDir.filePath(QStringLiteral("b.bin"));
    for (const QString &path : { firstPath, secondPath }) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("M");
        file.close();
    }

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);
    QObject::connect(&app, &Application::projectChanged,
                     &window, &MainWindow::showProject);

    app.newProject();
    QVERIFY(app.importMediaFile(firstPath));
    QVERIFY(app.importMediaFile(secondPath));
    const QString idA = app.mediaItems().at(0).id();
    const QString idB = app.mediaItems().at(1).id();
    QVERIFY(app.setActiveMedia(idA));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    window.showFramePreview(QImage(50, 50, QImage::Format_RGB32));
    QVERIFY(viewer->hasSourceImage());

    // Switching to a different active media clears the stale frame.
    QVERIFY(app.setActiveMedia(idB));
    QVERIFY(!viewer->hasSourceImage());

    window.showFramePreview(QImage(60, 40, QImage::Format_RGB32));
    QVERIFY(viewer->hasSourceImage());

    // Removing the active media clears it as well.
    QVERIFY(app.removeMedia(idB));
    QVERIFY(!viewer->hasSourceImage());
}

void ProjectTest::viewerSourcePersistsWhileContextStable()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString mediaPath = tempDir.filePath(QStringLiteral("a.bin"));
    QFile file(mediaPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("S");
    file.close();

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);

    app.newProject();
    QVERIFY(app.importMediaFile(mediaPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    window.showFramePreview(QImage(80, 40, QImage::Format_RGB32));
    QVERIFY(viewer->hasSourceImage());

    // Re-announcing the same active id must not clear the presented frame.
    window.showActiveMedia(mediaId);
    QVERIFY(viewer->hasSourceImage());
}

void ProjectTest::equirectViewBilinearBlendsFourNeighbors()
{
    // 4x4 source with four distinct colors around texel coordinates
    // (1..2, 1..2); a 1x1 render whose center ray lands exactly at (1.5, 1.5)
    // must produce the quarter blend (bilinear), not any single neighbor
    // (nearest).
    QImage source(4, 4, QImage::Format_ARGB32);
    source.fill(QColor(0, 0, 0));
    source.setPixelColor(1, 1, QColor(200, 0, 0));
    source.setPixelColor(2, 1, QColor(0, 200, 0));
    source.setPixelColor(1, 2, QColor(0, 0, 200));
    source.setPixelColor(2, 2, QColor(100, 100, 100));

    QImage view;
    QVERIFY(EquirectView::render(source, -45.0, 22.5, 0.0, 90.0, 1, 1, &view));
    QCOMPARE(view.width(), 1);
    QCOMPARE(view.height(), 1);

    const QColor color = view.pixelColor(0, 0);
    QVERIFY(qAbs(color.red() - 75) <= 3);
    QVERIFY(qAbs(color.green() - 75) <= 3);
    QVERIFY(qAbs(color.blue() - 75) <= 3);
    // Bilinear, not nearest: must not equal any pure neighbor.
    QVERIFY(color != QColor(200, 0, 0));
    QVERIFY(color != QColor(0, 200, 0));
    QVERIFY(color != QColor(0, 0, 200));
}

void ProjectTest::equirectViewBilinearRobustAtSeamAndPoles()
{
    const QImage pattern = buildTestPattern();

    // Pole directions clamp vertically and stay exact on uniform rows.
    QImage viewUp;
    QVERIFY(EquirectView::render(pattern, 0.0, 90.0, 0.0, 90.0, 200, 100, &viewUp));
    expectColor(viewUp, 100, 50, kUpColor);

    QImage viewDown;
    QVERIFY(EquirectView::render(pattern, 0.0, -90.0, 0.0, 90.0, 200, 100, &viewDown));
    expectColor(viewDown, 100, 50, kDownColor);

    // The horizontal seam (+/-180 deg yaw) wraps deterministically without
    // artifacts; center is background (no marker region at yaw 180).
    QImage firstSeam;
    QImage secondSeam;
    QVERIFY(EquirectView::render(pattern, 180.0, 0.0, 0.0, 90.0, 200, 100, &firstSeam));
    QVERIFY(EquirectView::render(pattern, 180.0, 0.0, 0.0, 90.0, 200, 100, &secondSeam));
    QVERIFY(imagesIdentical(firstSeam, secondSeam));
    expectColor(firstSeam, 100, 50, kPatternBackground);
}

void ProjectTest::equirectReviewFixtureFramesAreDistinctAndSeekable()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createEquirectReviewVideo(tempDir.path(),
                                      FrameExtractor::defaultExecutablePath(),
                                      30, &videoPath));

    QImage atZero;
    QImage atOne;
    QImage atFive;
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 0.0, &atZero));
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 1.0, &atOne));
    QVERIFY(FrameExtractor::extractFrameAt(
        videoPath, FrameExtractor::defaultExecutablePath(), 5.0, &atFive));

    const QColor centerZero = atZero.pixelColor(atZero.width() / 2, atZero.height() / 2);
    const QColor centerOne = atOne.pixelColor(atOne.width() / 2, atOne.height() / 2);
    const QColor centerFive = atFive.pixelColor(atFive.width() / 2, atFive.height() / 2);

    QVERIFY(reviewRedDominant(centerZero));   // frame 0 = red
    QVERIFY(reviewGreenDominant(centerOne));  // frame 1 = green
    QVERIFY(reviewBlueDominant(centerFive));  // frame 5 % 3 == 2 = blue

    QVERIFY(!imagesIdentical(atZero, atOne));
}

void ProjectTest::reviewPathEndToEndOnEquirectClip()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createEquirectReviewVideo(tempDir.path(),
                                      FrameExtractor::defaultExecutablePath(),
                                      30, &videoPath));

    Application app;
    TestMainWindow window;
    QObject::connect(&app, &Application::mediaListChanged,
                     &window, &MainWindow::showMediaList);
    QObject::connect(&app, &Application::activeMediaChanged,
                     &window, &MainWindow::showActiveMedia);
    QObject::connect(&app, &Application::projectChanged,
                     &window, &MainWindow::showProject);
    QObject::connect(&app, &Application::framePreviewReady,
                     &window, &MainWindow::showFramePreview);
    QObject::connect(&app, &Application::previewTimeChanged,
                     &window, &MainWindow::showPreviewTime);
    QObject::connect(&window, &MainWindow::previewStepRequested,
                     &app, &Application::stepActiveMediaPreview);

    app.newProject();
    QVERIFY(app.importMediaFile(videoPath));
    const QString mediaId = app.mediaItems().at(0).id();
    QVERIFY(app.setActiveMedia(mediaId));

    ViewerWidget *viewer = window.viewerWidget();
    QVERIFY(viewer);
    viewer->setViewportState(app.viewportState());

    auto centerColor = [viewer]() {
        const QImage image = viewer->grab().toImage();
        return image.pixelColor(image.width() / 2, image.height() / 2);
    };

    // Preview at t=0: FRONT red.
    QVERIFY(app.previewActiveMediaFrameAt(0.0));
    QVERIFY(viewer->hasSourceImage());
    QVERIFY(reviewRedDominant(centerColor()));

    // Step +1 s: FRONT green.
    auto *stepForward = window.findChild<QPushButton *>(QStringLiteral("stepForwardButton"));
    QVERIFY(stepForward);
    stepForward->click();
    QVERIFY(reviewGreenDominant(centerColor()));
    QCOMPARE(app.previewTimeSeconds(), 1.0);

    // Look around while the frame is present: +90 yaw centers RIGHT (magenta).
    app.adjustViewportYaw(90.0);
    QVERIFY(reviewMagentaDominant(centerColor()));
    app.resetViewport();

    // Seek far forward: frame 5 % 3 == 2 => FRONT blue.
    QVERIFY(app.previewActiveMediaFrameAt(5.0));
    QVERIFY(reviewBlueDominant(centerColor()));
    QCOMPARE(app.previewTimeSeconds(), 5.0);
}

void ProjectTest::equirectReviewPerformanceInformational()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createEquirectReviewVideo(tempDir.path(),
                                      FrameExtractor::defaultExecutablePath(),
                                      30, &videoPath));

    // One-shot per-step decode cost on the real clip.
    QElapsedTimer timer;
    timer.start();
    QImage frame;
    for (int t = 0; t < 5; ++t) {
        QVERIFY(FrameExtractor::extractFrameAt(
            videoPath, FrameExtractor::defaultExecutablePath(),
            static_cast<double>(t), &frame));
    }
    const double decodeMs = static_cast<double>(timer.nsecsElapsed()) / 5 / 1e6;
    qInfo("Review path: single-frame decode ~%.1f ms/step on the equirect clip", decodeMs);

    // Per-paint equirect render cost on a decoded frame (640 cap).
    const QImage pattern = buildTestPattern();
    timer.restart();
    QImage view;
    for (int i = 0; i < 5; ++i) {
        QVERIFY(EquirectView::render(pattern, 10.0, 5.0, 3.0, 90.0,
                                     EquirectView::MaxOutputWidth, 320, &view));
    }
    const double renderMs = static_cast<double>(timer.nsecsElapsed()) / 5 / 1e6;
    qInfo("Review path: per-paint render ~%.1f ms at %dx%d",
          renderMs, EquirectView::MaxOutputWidth, 320);

    // Informational only; no timing gate.
}

void ProjectTest::persistentStreamProbeDeliversFramesInOrder()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    constexpr int kFrames = 25;
    constexpr int kFps = 5;
    constexpr int kWidth = 160;
    constexpr int kHeight = 80;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   kFrames, kFps, &videoPath));

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(FrameExtractor::defaultExecutablePath(), {
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-nostdin"),
        QStringLiteral("-i"), videoPath,
        QStringLiteral("-f"), QStringLiteral("rawvideo"),
        QStringLiteral("-pix_fmt"), QStringLiteral("rgb24"),
        QStringLiteral("-")
    });
    QVERIFY(process.waitForStarted(10000));

    QElapsedTimer clock;
    clock.start();
    QByteArray pending;
    QList<double> arrivalMs;
    QList<QImage> frames;
    constexpr int kReadTimeoutMs = 60000;
    while (frames.size() < kFrames && clock.elapsed() < kReadTimeoutMs) {
        QImage frame;
        if (!readRawFrame(process, kWidth, kHeight, pending, &frame, kReadTimeoutMs)) {
            break;
        }
        arrivalMs.append(clock.elapsed());
        frames.append(frame);
    }

    // Allow the process to finish and reach EOF within a bounded wait.
    if (process.state() != QProcess::NotRunning) {
        process.waitForFinished(15000);
    }
    const QByteArray errorOutput = process.readAllStandardError();

    const int delivered = frames.size();
    QVERIFY2(delivered >= kFrames, qPrintable(
        QStringLiteral("Expected %1 frames, delivered %2; stderr: %3")
            .arg(kFrames).arg(delivered)
            .arg(QString::fromUtf8(errorOutput).trimmed().left(200))));
    QVERIFY(process.exitStatus() == QProcess::NormalExit);
    QVERIFY(process.exitCode() == 0);

    // Content in order: frame i carries the i%3 color (red/green/blue cycle).
    for (int i = 0; i < delivered && i < 9; ++i) {
        QVERIFY2(classifyFrameColor(frames.at(i), i),
                 qPrintable(QStringLiteral("Frame %1 color mismatch").arg(i)));
    }

    const double elapsedSeconds = clock.elapsed() / 1000.0;
    const double deliveredFps = elapsedSeconds > 0.0 ? delivered / elapsedSeconds : 0.0;
    double firstArrivalMs = 0.0;
    if (arrivalMs.size() >= 2) {
        firstArrivalMs = arrivalMs.at(1) - arrivalMs.at(0);
    }
    qInfo("Persistent subprocess probe: %d/%d frames delivered, ~%.1f frames/s, "
          "median inter-frame latency ~%.1f ms (informational; rawvideo, unthrottled)",
          delivered, kFrames, deliveredFps, firstArrivalMs);

    // No timing gate; the above are evidence for the feasibility record.
}

void ProjectTest::persistentStreamProbeErrorOnMissingFile()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start(FrameExtractor::defaultExecutablePath(), {
        QStringLiteral("-v"), QStringLiteral("error"),
        QStringLiteral("-nostdin"),
        QStringLiteral("-i"), QStringLiteral("/nonexistent/probe.mp4"),
        QStringLiteral("-f"), QStringLiteral("rawvideo"),
        QStringLiteral("-pix_fmt"), QStringLiteral("rgb24"),
        QStringLiteral("-")
    });
    QVERIFY(process.waitForStarted(10000));
    process.waitForFinished(15000);
    QVERIFY(process.exitStatus() == QProcess::NormalExit);
    QVERIFY(process.exitCode() != 0);
    QVERIFY(!process.readAllStandardError().isEmpty());
}

void ProjectTest::persistentStreamProbeKillAndRestartLifecycle()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   25, 5, &videoPath));

    auto startStream = [&videoPath](QProcess &process) {
        process.setProcessChannelMode(QProcess::SeparateChannels);
        process.start(FrameExtractor::defaultExecutablePath(), {
            QStringLiteral("-v"), QStringLiteral("error"),
            QStringLiteral("-nostdin"),
            QStringLiteral("-i"), videoPath,
            QStringLiteral("-f"), QStringLiteral("rawvideo"),
            QStringLiteral("-pix_fmt"), QStringLiteral("rgb24"),
            QStringLiteral("-")
        });
        return process.waitForStarted(10000);
    };

    // First stream: read one frame, then kill cleanly.
    QProcess first;
    QVERIFY(startStream(first));
    QByteArray pending;
    QImage frame;
    QVERIFY(readRawFrame(first, 160, 80, pending, &frame, 30000));
    first.terminate();
    if (!first.waitForFinished(3000)) {
        first.kill();
        first.waitForFinished(3000);
    }
    QVERIFY(first.state() == QProcess::NotRunning);

    // Restart: a fresh process must stream from the beginning again.
    QProcess second;
    QVERIFY(startStream(second));
    pending.clear();
    QVERIFY(readRawFrame(second, 160, 80, pending, &frame, 30000));
    QVERIFY(classifyFrameColor(frame, 0)); // frame 0 = red
    second.terminate();
    if (!second.waitForFinished(3000)) {
        second.kill();
        second.waitForFinished(3000);
    }
    QVERIFY(second.state() == QProcess::NotRunning);
}

// --- Phase 3 Objective 3: replaceable media-source seam + deterministic frame pump ---

void ProjectTest::framePumpDeliversFramesAndEndFromReplaceableSource()
{
    // Replaceable-seam proof: an in-memory FrameSource drives FramePump with no
    // ffmpeg subprocess, media file, or timing dependency.
    FakeFrameSource source;
    QImage red(4, 3, QImage::Format_RGB32);
    red.fill(QColor(230, 0, 0));
    QImage green(4, 3, QImage::Format_RGB32);
    green.fill(QColor(0, 230, 0));
    source.appendFrame(red);
    source.appendFrame(green);

    FramePump pump;
    pump.setSource(&source);
    QVERIFY(pump.source() == static_cast<FrameSource *>(&source));

    QList<QImage> received;
    int ended = 0;
    int failed = 0;
    QObject::connect(&pump, &FramePump::frameReady,
                     [&received](const QImage &image) { received.append(image); });
    QObject::connect(&pump, &FramePump::streamEnded, [&ended]() { ++ended; });
    QObject::connect(&pump, &FramePump::streamFailed,
                     [&failed](const QString &) { ++failed; });

    FrameSource::ReadResult result = FrameSource::ReadResult::Error;
    QVERIFY(pump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Ok);
    QVERIFY(pump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Ok);
    QCOMPARE(static_cast<int>(received.size()), 2);
    QVERIFY(imagesIdentical(received.at(0), red));
    QVERIFY(imagesIdentical(received.at(1), green));

    // End of a finite source is reported exactly once and delivers no frame.
    QVERIFY(!pump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::EndOfStream);
    QCOMPARE(ended, 1);
    QCOMPARE(failed, 0);
    QCOMPARE(static_cast<int>(received.size()), 2);
}

void ProjectTest::framePumpHandlesErrorTimeoutAndMissingSource()
{
    // Error branch: the source error text is relayed through streamFailed.
    FakeFrameSource errorSource;
    errorSource.setMode(FakeFrameSource::Mode::Error);
    errorSource.setErrorText(QStringLiteral("synthetic decode failure"));
    FramePump errorPump;
    errorPump.setSource(&errorSource);
    int errorFailures = 0;
    QString failureMessage;
    QObject::connect(&errorPump, &FramePump::streamFailed,
                     [&errorFailures, &failureMessage](const QString &text) {
                         ++errorFailures;
                         failureMessage = text;
                     });
    FrameSource::ReadResult result = FrameSource::ReadResult::Ok;
    QVERIFY(!errorPump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Error);
    QCOMPARE(errorFailures, 1);
    QCOMPARE(failureMessage, QStringLiteral("synthetic decode failure"));

    // Timeout branch: no signal is emitted; the caller may advance again.
    FakeFrameSource timeoutSource;
    timeoutSource.setMode(FakeFrameSource::Mode::Timeout);
    FramePump timeoutPump;
    timeoutPump.setSource(&timeoutSource);
    int timeoutSignals = 0;
    QObject::connect(&timeoutPump, &FramePump::frameReady,
                     [&timeoutSignals](const QImage &) { ++timeoutSignals; });
    QObject::connect(&timeoutPump, &FramePump::streamEnded,
                     [&timeoutSignals]() { ++timeoutSignals; });
    QObject::connect(&timeoutPump, &FramePump::streamFailed,
                     [&timeoutSignals](const QString &) { ++timeoutSignals; });
    QVERIFY(!timeoutPump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Timeout);
    QCOMPARE(timeoutSignals, 0);

    // Missing source: deterministic failure, never a crash or a frame.
    FramePump emptyPump;
    int emptySignals = 0;
    QString emptyMessage;
    QObject::connect(&emptyPump, &FramePump::streamFailed,
                     [&emptySignals, &emptyMessage](const QString &text) {
                         ++emptySignals;
                         emptyMessage = text;
                     });
    QVERIFY(!emptyPump.advance(1000, &result));
    QVERIFY(result == FrameSource::ReadResult::Error);
    QCOMPARE(emptySignals, 1);
    QVERIFY(!emptyMessage.isEmpty());
}

void ProjectTest::fmpegFrameSourceStreamsFramesInOrder()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    constexpr int kFrames = 25;
    constexpr int kWidth = 160;
    constexpr int kHeight = 80;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   kFrames, 5, &videoPath));

    FfmpegFrameSource source;
    QVERIFY2(source.open(videoPath, kWidth, kHeight),
             qPrintable(source.errorString()));
    QVERIFY(source.isOpen());
    QVERIFY(source.errorString().isEmpty());

    for (int i = 0; i < kFrames; ++i) {
        FrameSource::ReadResult result = FrameSource::ReadResult::Error;
        QImage frame;
        QVERIFY2(source.readNextFrame(30000, &result, &frame),
                 qPrintable(QStringLiteral("frame %1 failed: %2")
                                .arg(i).arg(source.errorString())));
        QVERIFY(result == FrameSource::ReadResult::Ok);
        QCOMPARE(frame.size(), QSize(kWidth, kHeight));
        QVERIFY2(classifyFrameColor(frame, i),
                 qPrintable(QStringLiteral("frame %1 color mismatch").arg(i)));
    }

    // The stream reaches end-of-stream deterministically after the last frame.
    FrameSource::ReadResult endResult = FrameSource::ReadResult::Error;
    QImage extra;
    QVERIFY(!source.readNextFrame(30000, &endResult, &extra));
    QVERIFY(endResult == FrameSource::ReadResult::EndOfStream);
    QVERIFY(source.errorString().isEmpty());

    source.close();
    QVERIFY(!source.isOpen());
}

void ProjectTest::fmpegFrameSourceValidatesAndRestartsCleanly()
{
    FfmpegFrameSource source;
    FrameSource::ReadResult result = FrameSource::ReadResult::Error;
    QImage frame;

    // Rawvideo carries no geometry metadata and ffprobe discovery is deferred,
    // so opening without explicit geometry must fail deterministically.
    QVERIFY(!source.open(QStringLiteral("/nonexistent/clip.mp4"), 0, 0));
    QVERIFY(!source.isOpen());
    QVERIFY(!source.errorString().isEmpty());
    QVERIFY(!source.readNextFrame(100, &result, &frame));
    QVERIFY(result == FrameSource::ReadResult::Error);

    // Missing file with valid geometry also fails deterministically.
    QVERIFY(!source.open(QStringLiteral("/nonexistent/clip.mp4"), 160, 80));
    QVERIFY(!source.isOpen());
    QVERIFY(source.errorString().contains(QStringLiteral("does not exist")));

    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping stream restart portion.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   6, 5, &videoPath));

    QVERIFY2(source.open(videoPath, 160, 80), qPrintable(source.errorString()));
    QVERIFY(source.readNextFrame(30000, &result, &frame));
    QVERIFY(result == FrameSource::ReadResult::Ok);
    QVERIFY(classifyFrameColor(frame, 0));

    // close() releases the process and reading afterwards fails cleanly.
    source.close();
    QVERIFY(!source.isOpen());
    QVERIFY(!source.readNextFrame(100, &result, &frame));
    QVERIFY(result == FrameSource::ReadResult::Error);

    // The same instance can be reopened and streams from the start again.
    QVERIFY2(source.open(videoPath, 160, 80), qPrintable(source.errorString()));
    QVERIFY(source.readNextFrame(30000, &result, &frame));
    QVERIFY(result == FrameSource::ReadResult::Ok);
    QVERIFY(classifyFrameColor(frame, 0));
    source.close();
    QVERIFY(!source.isOpen());
}

void ProjectTest::framePumpStreamsFfmpegSourceToEnd()
{
    if (!FrameExtractor::isAvailable()) {
        QSKIP("ffmpeg not available; skipping decode-dependent test.");
    }

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString videoPath;
    constexpr int kFrames = 12;
    QVERIFY(createStreamProbeVideo(tempDir.path(),
                                   FrameExtractor::defaultExecutablePath(),
                                   kFrames, 5, &videoPath));

    FfmpegFrameSource source;
    QVERIFY2(source.open(videoPath, 160, 80), qPrintable(source.errorString()));

    FramePump pump;
    pump.setSource(&source);

    QList<QImage> frames;
    int ended = 0;
    int failed = 0;
    QObject::connect(&pump, &FramePump::frameReady,
                     [&frames](const QImage &image) { frames.append(image); });
    QObject::connect(&pump, &FramePump::streamEnded, [&ended]() { ++ended; });
    QObject::connect(&pump, &FramePump::streamFailed,
                     [&failed](const QString &) { ++failed; });

    FrameSource::ReadResult result = FrameSource::ReadResult::Error;
    for (int i = 0; i < kFrames; ++i) {
        QVERIFY2(pump.advance(30000, &result),
                 qPrintable(QStringLiteral("pump advance %1 failed: %2")
                                .arg(i).arg(source.errorString())));
        QVERIFY(result == FrameSource::ReadResult::Ok);
    }
    QCOMPARE(static_cast<int>(frames.size()), kFrames);
    for (int i = 0; i < kFrames; ++i) {
        QVERIFY2(classifyFrameColor(frames.at(i), i),
                 qPrintable(QStringLiteral("pumped frame %1 color mismatch").arg(i)));
    }

    QVERIFY(!pump.advance(30000, &result));
    QVERIFY(result == FrameSource::ReadResult::EndOfStream);
    QCOMPARE(ended, 1);
    QCOMPARE(failed, 0);

    source.close();
    QVERIFY(!source.isOpen());
}

QTEST_MAIN(ProjectTest)
#include "test_project.moc"
